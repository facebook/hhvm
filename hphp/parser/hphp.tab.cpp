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
#include "folly/Conv.h"
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
  t.setText(folly::to<std::string>(num));
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
      "Syntax only allowed in Hack files (<?hh) or with -v "
        "Eval.EnableHipHopSyntax=true",
      _p);
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
#line 646 "hphp.tab.cpp"

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
     T_NULLSAFE_OBJECT_OPERATOR = 364,
     T_DOUBLE_ARROW = 365,
     T_LIST = 366,
     T_ARRAY = 367,
     T_CALLABLE = 368,
     T_CLASS_C = 369,
     T_METHOD_C = 370,
     T_FUNC_C = 371,
     T_LINE = 372,
     T_FILE = 373,
     T_COMMENT = 374,
     T_DOC_COMMENT = 375,
     T_OPEN_TAG = 376,
     T_OPEN_TAG_WITH_ECHO = 377,
     T_CLOSE_TAG = 378,
     T_WHITESPACE = 379,
     T_START_HEREDOC = 380,
     T_END_HEREDOC = 381,
     T_DOLLAR_OPEN_CURLY_BRACES = 382,
     T_CURLY_OPEN = 383,
     T_DOUBLE_COLON = 384,
     T_NAMESPACE = 385,
     T_NS_C = 386,
     T_DIR = 387,
     T_NS_SEPARATOR = 388,
     T_XHP_LABEL = 389,
     T_XHP_TEXT = 390,
     T_XHP_ATTRIBUTE = 391,
     T_XHP_CATEGORY = 392,
     T_XHP_CATEGORY_LABEL = 393,
     T_XHP_CHILDREN = 394,
     T_ENUM = 395,
     T_XHP_REQUIRED = 396,
     T_TRAIT = 397,
     T_ELLIPSIS = 398,
     T_INSTEADOF = 399,
     T_TRAIT_C = 400,
     T_HH_ERROR = 401,
     T_FINALLY = 402,
     T_XHP_TAG_LT = 403,
     T_XHP_TAG_GT = 404,
     T_TYPELIST_LT = 405,
     T_TYPELIST_GT = 406,
     T_UNRESOLVED_LT = 407,
     T_COLLECTION = 408,
     T_SHAPE = 409,
     T_VARRAY = 410,
     T_MIARRAY = 411,
     T_MSARRAY = 412,
     T_TYPE = 413,
     T_UNRESOLVED_TYPE = 414,
     T_NEWTYPE = 415,
     T_UNRESOLVED_NEWTYPE = 416,
     T_COMPILER_HALT_OFFSET = 417,
     T_ASYNC = 418,
     T_FROM = 419,
     T_WHERE = 420,
     T_JOIN = 421,
     T_IN = 422,
     T_ON = 423,
     T_EQUALS = 424,
     T_INTO = 425,
     T_LET = 426,
     T_ORDERBY = 427,
     T_ASCENDING = 428,
     T_DESCENDING = 429,
     T_SELECT = 430,
     T_GROUP = 431,
     T_BY = 432,
     T_LAMBDA_OP = 433,
     T_LAMBDA_CP = 434,
     T_UNRESOLVED_OP = 435
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
#line 881 "hphp.tab.cpp"

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
#define YYLAST   16406

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  274
/* YYNRULES -- Number of rules.  */
#define YYNRULES  927
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1739

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   435

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   208,     2,   205,    51,    35,   209,
     200,   201,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   202,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   207,    34,     2,   206,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   203,    33,   204,    54,     2,     2,     2,
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
     194,   195,   196,   197,   198,   199
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
     563,   567,   571,   574,   577,   578,   581,   582,   585,   586,
     588,   592,   594,   598,   601,   602,   604,   607,   612,   614,
     619,   621,   626,   628,   633,   635,   640,   644,   650,   654,
     659,   664,   670,   676,   681,   682,   684,   686,   691,   692,
     698,   699,   702,   703,   707,   708,   716,   725,   732,   735,
     741,   748,   753,   754,   759,   765,   773,   780,   787,   795,
     805,   814,   821,   829,   835,   838,   843,   849,   853,   854,
     858,   863,   870,   876,   882,   889,   898,   906,   909,   910,
     912,   915,   918,   922,   927,   932,   936,   938,   940,   943,
     948,   952,   958,   960,   964,   967,   968,   971,   975,   978,
     979,   980,   985,   986,   992,   995,   996,  1007,  1008,  1020,
    1024,  1028,  1032,  1037,  1042,  1046,  1052,  1055,  1058,  1059,
    1066,  1072,  1077,  1081,  1083,  1085,  1089,  1094,  1096,  1098,
    1100,  1102,  1107,  1109,  1111,  1115,  1118,  1119,  1122,  1123,
    1125,  1129,  1131,  1133,  1135,  1137,  1141,  1146,  1151,  1156,
    1158,  1160,  1163,  1166,  1169,  1173,  1177,  1179,  1181,  1183,
    1185,  1189,  1191,  1195,  1197,  1199,  1201,  1202,  1204,  1207,
    1209,  1211,  1213,  1215,  1217,  1219,  1221,  1223,  1224,  1226,
    1228,  1230,  1234,  1240,  1242,  1246,  1252,  1257,  1261,  1265,
    1268,  1270,  1272,  1276,  1280,  1282,  1284,  1285,  1287,  1290,
    1295,  1299,  1306,  1309,  1313,  1320,  1322,  1324,  1326,  1333,
    1337,  1342,  1349,  1353,  1357,  1361,  1365,  1369,  1373,  1377,
    1381,  1385,  1389,  1393,  1397,  1400,  1403,  1406,  1409,  1413,
    1417,  1421,  1425,  1429,  1433,  1437,  1441,  1445,  1449,  1453,
    1457,  1461,  1465,  1469,  1473,  1477,  1480,  1483,  1486,  1489,
    1493,  1497,  1501,  1505,  1509,  1513,  1517,  1521,  1525,  1529,
    1535,  1540,  1542,  1545,  1548,  1551,  1554,  1557,  1560,  1563,
    1566,  1569,  1571,  1573,  1575,  1577,  1579,  1583,  1586,  1588,
    1590,  1592,  1598,  1599,  1600,  1612,  1613,  1626,  1627,  1631,
    1632,  1637,  1638,  1645,  1646,  1654,  1657,  1660,  1665,  1667,
    1669,  1675,  1679,  1685,  1689,  1692,  1693,  1696,  1697,  1702,
    1707,  1711,  1716,  1721,  1726,  1731,  1736,  1741,  1746,  1751,
    1756,  1761,  1763,  1765,  1767,  1771,  1774,  1778,  1783,  1786,
    1790,  1792,  1795,  1797,  1800,  1802,  1804,  1806,  1808,  1810,
    1812,  1817,  1822,  1825,  1834,  1845,  1848,  1850,  1854,  1856,
    1859,  1861,  1863,  1865,  1867,  1870,  1875,  1879,  1883,  1888,
    1890,  1893,  1898,  1901,  1908,  1909,  1911,  1916,  1917,  1920,
    1921,  1923,  1925,  1929,  1931,  1935,  1937,  1939,  1943,  1947,
    1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,
    1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,
    1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,
    2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,
    2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,
    2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,
    2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,
    2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,
    2112,  2114,  2116,  2118,  2120,  2122,  2124,  2126,  2128,  2131,
    2133,  2134,  2135,  2137,  2139,  2143,  2144,  2146,  2148,  2150,
    2152,  2154,  2156,  2158,  2160,  2162,  2164,  2166,  2168,  2170,
    2174,  2177,  2179,  2181,  2186,  2190,  2195,  2197,  2199,  2201,
    2203,  2207,  2211,  2215,  2219,  2223,  2227,  2231,  2235,  2239,
    2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,  2278,
    2281,  2284,  2287,  2291,  2295,  2299,  2303,  2307,  2311,  2315,
    2319,  2325,  2330,  2334,  2338,  2342,  2344,  2346,  2348,  2350,
    2354,  2358,  2362,  2365,  2366,  2368,  2369,  2371,  2372,  2378,
    2382,  2386,  2388,  2390,  2392,  2394,  2396,  2400,  2403,  2405,
    2407,  2409,  2411,  2413,  2415,  2418,  2421,  2426,  2430,  2435,
    2438,  2439,  2445,  2449,  2453,  2455,  2459,  2461,  2464,  2465,
    2471,  2475,  2478,  2479,  2483,  2484,  2489,  2492,  2493,  2497,
    2501,  2503,  2504,  2506,  2508,  2510,  2514,  2516,  2518,  2522,
    2526,  2529,  2534,  2537,  2542,  2544,  2546,  2548,  2550,  2552,
    2556,  2562,  2566,  2571,  2575,  2577,  2579,  2581,  2583,  2587,
    2593,  2598,  2602,  2604,  2606,  2610,  2618,  2628,  2636,  2643,
    2652,  2654,  2657,  2662,  2667,  2669,  2671,  2676,  2678,  2679,
    2681,  2684,  2686,  2688,  2692,  2698,  2702,  2706,  2707,  2709,
    2713,  2719,  2723,  2726,  2730,  2737,  2738,  2740,  2745,  2748,
    2749,  2755,  2759,  2763,  2765,  2772,  2777,  2782,  2785,  2788,
    2789,  2795,  2799,  2803,  2805,  2808,  2809,  2815,  2819,  2823,
    2825,  2828,  2829,  2832,  2833,  2839,  2843,  2847,  2849,  2852,
    2853,  2856,  2857,  2863,  2867,  2871,  2873,  2876,  2879,  2881,
    2884,  2886,  2891,  2895,  2899,  2906,  2910,  2912,  2914,  2916,
    2921,  2926,  2931,  2936,  2939,  2942,  2947,  2950,  2953,  2955,
    2959,  2963,  2967,  2968,  2971,  2977,  2984,  2986,  2989,  2991,
    2996,  3000,  3001,  3003,  3007,  3010,  3014,  3016,  3018,  3019,
    3020,  3023,  3028,  3031,  3038,  3043,  3045,  3047,  3048,  3052,
    3058,  3062,  3064,  3067,  3068,  3073,  3076,  3079,  3081,  3083,
    3085,  3087,  3092,  3099,  3101,  3110,  3117,  3119
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   232,    -1,   249,    -1,   256,    -1,   253,    -1,   261,
      -1,   468,    -1,   122,   200,   201,   202,    -1,   149,   224,
     202,    -1,    -1,   149,   224,   203,   215,   213,   204,    -1,
      -1,   149,   203,   216,   213,   204,    -1,   110,   218,   202,
      -1,   110,   104,   219,   202,    -1,   110,   105,   220,   202,
      -1,   229,   202,    -1,    77,    -1,   155,    -1,   156,    -1,
     158,    -1,   160,    -1,   159,    -1,   184,    -1,   185,    -1,
     187,    -1,   186,    -1,   188,    -1,   189,    -1,   190,    -1,
     191,    -1,   192,    -1,   193,    -1,   194,    -1,   195,    -1,
     196,    -1,   218,     9,   221,    -1,   221,    -1,   222,     9,
     222,    -1,   222,    -1,   223,     9,   223,    -1,   223,    -1,
     224,    -1,   152,   224,    -1,   224,    96,   217,    -1,   152,
     224,    96,   217,    -1,   224,    -1,   152,   224,    -1,   224,
      96,   217,    -1,   152,   224,    96,   217,    -1,   224,    -1,
     152,   224,    -1,   224,    96,   217,    -1,   152,   224,    96,
     217,    -1,   217,    -1,   224,   152,   217,    -1,   224,    -1,
     149,   152,   224,    -1,   152,   224,    -1,   225,    -1,   225,
     471,    -1,   225,   471,    -1,   229,     9,   469,    14,   406,
      -1,   105,   469,    14,   406,    -1,   230,   231,    -1,    -1,
     232,    -1,   249,    -1,   256,    -1,   261,    -1,   203,   230,
     204,    -1,    70,   332,   232,   283,   285,    -1,    70,   332,
      30,   230,   284,   286,    73,   202,    -1,    -1,    88,   332,
     233,   277,    -1,    -1,    87,   234,   232,    88,   332,   202,
      -1,    -1,    90,   200,   334,   202,   334,   202,   334,   201,
     235,   275,    -1,    -1,    97,   332,   236,   280,    -1,   101,
     202,    -1,   101,   341,   202,    -1,   103,   202,    -1,   103,
     341,   202,    -1,   106,   202,    -1,   106,   341,   202,    -1,
      27,   101,   202,    -1,   111,   293,   202,    -1,   117,   295,
     202,    -1,    86,   333,   202,    -1,   119,   200,   465,   201,
     202,    -1,   202,    -1,    81,    -1,    -1,    92,   200,   341,
      96,   274,   273,   201,   237,   276,    -1,    -1,    92,   200,
     341,    28,    96,   274,   273,   201,   238,   276,    -1,    94,
     200,   279,   201,   278,    -1,    -1,   107,   241,   108,   200,
     399,    79,   201,   203,   230,   204,   243,   239,   246,    -1,
      -1,   107,   241,   166,   240,   244,    -1,   109,   341,   202,
      -1,   102,   217,   202,    -1,   341,   202,    -1,   335,   202,
      -1,   336,   202,    -1,   337,   202,    -1,   338,   202,    -1,
     339,   202,    -1,   106,   338,   202,    -1,   340,   202,    -1,
     369,   202,    -1,   106,   368,   202,    -1,   217,    30,    -1,
      -1,   203,   242,   230,   204,    -1,   243,   108,   200,   399,
      79,   201,   203,   230,   204,    -1,    -1,    -1,   203,   245,
     230,   204,    -1,   166,   244,    -1,    -1,    35,    -1,    -1,
     104,    -1,    -1,   248,   247,   470,   250,   200,   289,   201,
     475,   321,    -1,    -1,   325,   248,   247,   470,   251,   200,
     289,   201,   475,   321,    -1,    -1,   426,   324,   248,   247,
     470,   252,   200,   289,   201,   475,   321,    -1,    -1,   159,
     217,   254,    30,   482,   467,   203,   296,   204,    -1,    -1,
     426,   159,   217,   255,    30,   482,   467,   203,   296,   204,
      -1,    -1,   267,   264,   257,   268,   269,   203,   299,   204,
      -1,    -1,   426,   267,   264,   258,   268,   269,   203,   299,
     204,    -1,    -1,   124,   265,   259,   270,   203,   299,   204,
      -1,    -1,   426,   124,   265,   260,   270,   203,   299,   204,
      -1,    -1,   161,   266,   262,   269,   203,   299,   204,    -1,
      -1,   426,   161,   266,   263,   269,   203,   299,   204,    -1,
     470,    -1,   153,    -1,   470,    -1,   470,    -1,   123,    -1,
     116,   123,    -1,   116,   115,   123,    -1,   115,   116,   123,
      -1,   115,   123,    -1,   125,   399,    -1,    -1,   126,   271,
      -1,    -1,   125,   271,    -1,    -1,   399,    -1,   271,     9,
     399,    -1,   399,    -1,   272,     9,   399,    -1,   129,   274,
      -1,    -1,   434,    -1,    35,   434,    -1,   130,   200,   446,
     201,    -1,   232,    -1,    30,   230,    91,   202,    -1,   232,
      -1,    30,   230,    93,   202,    -1,   232,    -1,    30,   230,
      89,   202,    -1,   232,    -1,    30,   230,    95,   202,    -1,
     217,    14,   406,    -1,   279,     9,   217,    14,   406,    -1,
     203,   281,   204,    -1,   203,   202,   281,   204,    -1,    30,
     281,    98,   202,    -1,    30,   202,   281,    98,   202,    -1,
     281,    99,   341,   282,   230,    -1,   281,   100,   282,   230,
      -1,    -1,    30,    -1,   202,    -1,   283,    71,   332,   232,
      -1,    -1,   284,    71,   332,    30,   230,    -1,    -1,    72,
     232,    -1,    -1,    72,    30,   230,    -1,    -1,   288,     9,
     427,   327,   483,   162,    79,    -1,   288,     9,   427,   327,
     483,    35,   162,    79,    -1,   288,     9,   427,   327,   483,
     162,    -1,   288,   411,    -1,   427,   327,   483,   162,    79,
      -1,   427,   327,   483,    35,   162,    79,    -1,   427,   327,
     483,   162,    -1,    -1,   427,   327,   483,    79,    -1,   427,
     327,   483,    35,    79,    -1,   427,   327,   483,    35,    79,
      14,   406,    -1,   427,   327,   483,    79,    14,   406,    -1,
     288,     9,   427,   327,   483,    79,    -1,   288,     9,   427,
     327,   483,    35,    79,    -1,   288,     9,   427,   327,   483,
      35,    79,    14,   406,    -1,   288,     9,   427,   327,   483,
      79,    14,   406,    -1,   290,     9,   427,   483,   162,    79,
      -1,   290,     9,   427,   483,    35,   162,    79,    -1,   290,
       9,   427,   483,   162,    -1,   290,   411,    -1,   427,   483,
     162,    79,    -1,   427,   483,    35,   162,    79,    -1,   427,
     483,   162,    -1,    -1,   427,   483,    79,    -1,   427,   483,
      35,    79,    -1,   427,   483,    35,    79,    14,   406,    -1,
     427,   483,    79,    14,   406,    -1,   290,     9,   427,   483,
      79,    -1,   290,     9,   427,   483,    35,    79,    -1,   290,
       9,   427,   483,    35,    79,    14,   406,    -1,   290,     9,
     427,   483,    79,    14,   406,    -1,   292,   411,    -1,    -1,
     341,    -1,    35,   434,    -1,   162,   341,    -1,   292,     9,
     341,    -1,   292,     9,   162,   341,    -1,   292,     9,    35,
     434,    -1,   293,     9,   294,    -1,   294,    -1,    79,    -1,
     205,   434,    -1,   205,   203,   341,   204,    -1,   295,     9,
      79,    -1,   295,     9,    79,    14,   406,    -1,    79,    -1,
      79,    14,   406,    -1,   296,   297,    -1,    -1,   298,   202,
      -1,   469,    14,   406,    -1,   299,   300,    -1,    -1,    -1,
     323,   301,   329,   202,    -1,    -1,   325,   482,   302,   329,
     202,    -1,   330,   202,    -1,    -1,   324,   248,   247,   470,
     200,   303,   287,   201,   475,   322,    -1,    -1,   426,   324,
     248,   247,   470,   200,   304,   287,   201,   475,   322,    -1,
     155,   309,   202,    -1,   156,   315,   202,    -1,   158,   317,
     202,    -1,     4,   125,   399,   202,    -1,     4,   126,   399,
     202,    -1,   110,   272,   202,    -1,   110,   272,   203,   305,
     204,    -1,   305,   306,    -1,   305,   307,    -1,    -1,   228,
     148,   217,   163,   272,   202,    -1,   308,    96,   324,   217,
     202,    -1,   308,    96,   325,   202,    -1,   228,   148,   217,
      -1,   217,    -1,   310,    -1,   309,     9,   310,    -1,   311,
     396,   313,   314,    -1,   153,    -1,   131,    -1,   399,    -1,
     118,    -1,   159,   203,   312,   204,    -1,   132,    -1,   405,
      -1,   312,     9,   405,    -1,    14,   406,    -1,    -1,    55,
     160,    -1,    -1,   316,    -1,   315,     9,   316,    -1,   157,
      -1,   318,    -1,   217,    -1,   121,    -1,   200,   319,   201,
      -1,   200,   319,   201,    49,    -1,   200,   319,   201,    29,
      -1,   200,   319,   201,    46,    -1,   318,    -1,   320,    -1,
     320,    49,    -1,   320,    29,    -1,   320,    46,    -1,   319,
       9,   319,    -1,   319,    33,   319,    -1,   217,    -1,   153,
      -1,   157,    -1,   202,    -1,   203,   230,   204,    -1,   202,
      -1,   203,   230,   204,    -1,   325,    -1,   118,    -1,   325,
      -1,    -1,   326,    -1,   325,   326,    -1,   112,    -1,   113,
      -1,   114,    -1,   117,    -1,   116,    -1,   115,    -1,   182,
      -1,   328,    -1,    -1,   112,    -1,   113,    -1,   114,    -1,
     329,     9,    79,    -1,   329,     9,    79,    14,   406,    -1,
      79,    -1,    79,    14,   406,    -1,   330,     9,   469,    14,
     406,    -1,   105,   469,    14,   406,    -1,   200,   331,   201,
      -1,    68,   401,   404,    -1,    67,   341,    -1,   388,    -1,
     360,    -1,   200,   341,   201,    -1,   333,     9,   341,    -1,
     341,    -1,   333,    -1,    -1,    27,    -1,    27,   341,    -1,
      27,   341,   129,   341,    -1,   434,    14,   335,    -1,   130,
     200,   446,   201,    14,   335,    -1,    28,   341,    -1,   434,
      14,   338,    -1,   130,   200,   446,   201,    14,   338,    -1,
     342,    -1,   434,    -1,   331,    -1,   130,   200,   446,   201,
      14,   341,    -1,   434,    14,   341,    -1,   434,    14,    35,
     434,    -1,   434,    14,    35,    68,   401,   404,    -1,   434,
      26,   341,    -1,   434,    25,   341,    -1,   434,    24,   341,
      -1,   434,    23,   341,    -1,   434,    22,   341,    -1,   434,
      21,   341,    -1,   434,    20,   341,    -1,   434,    19,   341,
      -1,   434,    18,   341,    -1,   434,    17,   341,    -1,   434,
      16,   341,    -1,   434,    15,   341,    -1,   434,    64,    -1,
      64,   434,    -1,   434,    63,    -1,    63,   434,    -1,   341,
      31,   341,    -1,   341,    32,   341,    -1,   341,    10,   341,
      -1,   341,    12,   341,    -1,   341,    11,   341,    -1,   341,
      33,   341,    -1,   341,    35,   341,    -1,   341,    34,   341,
      -1,   341,    48,   341,    -1,   341,    46,   341,    -1,   341,
      47,   341,    -1,   341,    49,   341,    -1,   341,    50,   341,
      -1,   341,    65,   341,    -1,   341,    51,   341,    -1,   341,
      45,   341,    -1,   341,    44,   341,    -1,    46,   341,    -1,
      47,   341,    -1,    52,   341,    -1,    54,   341,    -1,   341,
      37,   341,    -1,   341,    36,   341,    -1,   341,    39,   341,
      -1,   341,    38,   341,    -1,   341,    40,   341,    -1,   341,
      43,   341,    -1,   341,    41,   341,    -1,   341,    42,   341,
      -1,   341,    53,   401,    -1,   200,   342,   201,    -1,   341,
      29,   341,    30,   341,    -1,   341,    29,    30,   341,    -1,
     464,    -1,    62,   341,    -1,    61,   341,    -1,    60,   341,
      -1,    59,   341,    -1,    58,   341,    -1,    57,   341,    -1,
      56,   341,    -1,    69,   402,    -1,    55,   341,    -1,   408,
      -1,   359,    -1,   358,    -1,   361,    -1,   362,    -1,   206,
     403,   206,    -1,    13,   341,    -1,   344,    -1,   347,    -1,
     366,    -1,   110,   200,   387,   411,   201,    -1,    -1,    -1,
     248,   247,   200,   345,   289,   201,   475,   343,   203,   230,
     204,    -1,    -1,   325,   248,   247,   200,   346,   289,   201,
     475,   343,   203,   230,   204,    -1,    -1,    79,   348,   352,
      -1,    -1,   182,    79,   349,   352,    -1,    -1,   197,   350,
     289,   198,   475,   352,    -1,    -1,   182,   197,   351,   289,
     198,   475,   352,    -1,     8,   341,    -1,     8,   338,    -1,
       8,   203,   230,   204,    -1,    85,    -1,   466,    -1,   354,
       9,   353,   129,   341,    -1,   353,   129,   341,    -1,   355,
       9,   353,   129,   406,    -1,   353,   129,   406,    -1,   354,
     410,    -1,    -1,   355,   410,    -1,    -1,   173,   200,   356,
     201,    -1,   131,   200,   447,   201,    -1,    66,   447,   207,
      -1,   399,   203,   449,   204,    -1,   175,   200,   453,   201,
      -1,   176,   200,   453,   201,    -1,   174,   200,   454,   201,
      -1,   175,   200,   457,   201,    -1,   176,   200,   457,   201,
      -1,   174,   200,   458,   201,    -1,   399,   203,   451,   204,
      -1,   366,    66,   442,   207,    -1,   367,    66,   442,   207,
      -1,   359,    -1,   466,    -1,    85,    -1,   200,   342,   201,
      -1,   370,   371,    -1,   434,    14,   368,    -1,   183,    79,
     186,   341,    -1,   372,   383,    -1,   372,   383,   386,    -1,
     383,    -1,   383,   386,    -1,   373,    -1,   372,   373,    -1,
     374,    -1,   375,    -1,   376,    -1,   377,    -1,   378,    -1,
     379,    -1,   183,    79,   186,   341,    -1,   190,    79,    14,
     341,    -1,   184,   341,    -1,   185,    79,   186,   341,   187,
     341,   188,   341,    -1,   185,    79,   186,   341,   187,   341,
     188,   341,   189,    79,    -1,   191,   380,    -1,   381,    -1,
     380,     9,   381,    -1,   341,    -1,   341,   382,    -1,   192,
      -1,   193,    -1,   384,    -1,   385,    -1,   194,   341,    -1,
     195,   341,   196,   341,    -1,   189,    79,   371,    -1,   387,
       9,    79,    -1,   387,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   167,   153,   389,   168,    -1,   391,    50,
      -1,   391,   168,   392,   167,    50,   390,    -1,    -1,   153,
      -1,   391,   393,    14,   394,    -1,    -1,   392,   395,    -1,
      -1,   153,    -1,   154,    -1,   203,   341,   204,    -1,   154,
      -1,   203,   341,   204,    -1,   388,    -1,   397,    -1,   396,
      30,   397,    -1,   396,    47,   397,    -1,   217,    -1,    69,
      -1,   104,    -1,   105,    -1,   106,    -1,    27,    -1,    28,
      -1,   107,    -1,   108,    -1,   166,    -1,   109,    -1,    70,
      -1,    71,    -1,    73,    -1,    72,    -1,    88,    -1,    89,
      -1,    87,    -1,    90,    -1,    91,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    53,    -1,    96,    -1,    97,
      -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,   103,
      -1,   102,    -1,    86,    -1,    13,    -1,   123,    -1,   124,
      -1,   125,    -1,   126,    -1,    68,    -1,    67,    -1,   118,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   149,    -1,   110,    -1,   111,    -1,   120,    -1,   121,
      -1,   122,    -1,   117,    -1,   116,    -1,   115,    -1,   114,
      -1,   113,    -1,   112,    -1,   182,    -1,   119,    -1,   130,
      -1,   131,    -1,    10,    -1,    12,    -1,    11,    -1,   133,
      -1,   135,    -1,   134,    -1,   136,    -1,   137,    -1,   151,
      -1,   150,    -1,   181,    -1,   161,    -1,   164,    -1,   163,
      -1,   177,    -1,   179,    -1,   173,    -1,   227,   200,   291,
     201,    -1,   228,    -1,   153,    -1,   399,    -1,   117,    -1,
     440,    -1,   399,    -1,   117,    -1,   444,    -1,   200,   201,
      -1,   332,    -1,    -1,    -1,    84,    -1,   461,    -1,   200,
     291,   201,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   136,    -1,   137,    -1,   151,    -1,   133,    -1,
     164,    -1,   134,    -1,   135,    -1,   150,    -1,   181,    -1,
     144,    84,   145,    -1,   144,   145,    -1,   405,    -1,   226,
      -1,   131,   200,   409,   201,    -1,    66,   409,   207,    -1,
     173,   200,   357,   201,    -1,   407,    -1,   365,    -1,   363,
      -1,   364,    -1,   200,   406,   201,    -1,   406,    31,   406,
      -1,   406,    32,   406,    -1,   406,    10,   406,    -1,   406,
      12,   406,    -1,   406,    11,   406,    -1,   406,    33,   406,
      -1,   406,    35,   406,    -1,   406,    34,   406,    -1,   406,
      48,   406,    -1,   406,    46,   406,    -1,   406,    47,   406,
      -1,   406,    49,   406,    -1,   406,    50,   406,    -1,   406,
      51,   406,    -1,   406,    45,   406,    -1,   406,    44,   406,
      -1,   406,    65,   406,    -1,    52,   406,    -1,    54,   406,
      -1,    46,   406,    -1,    47,   406,    -1,   406,    37,   406,
      -1,   406,    36,   406,    -1,   406,    39,   406,    -1,   406,
      38,   406,    -1,   406,    40,   406,    -1,   406,    43,   406,
      -1,   406,    41,   406,    -1,   406,    42,   406,    -1,   406,
      29,   406,    30,   406,    -1,   406,    29,    30,   406,    -1,
     228,   148,   217,    -1,   153,   148,   217,    -1,   228,   148,
     123,    -1,   226,    -1,    78,    -1,   466,    -1,   405,    -1,
     208,   461,   208,    -1,   209,   461,   209,    -1,   144,   461,
     145,    -1,   412,   410,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   412,     9,   406,   129,   406,    -1,   412,     9,
     406,    -1,   406,   129,   406,    -1,   406,    -1,    74,    -1,
      75,    -1,    76,    -1,    85,    -1,   144,    84,   145,    -1,
     144,   145,    -1,    74,    -1,    75,    -1,    76,    -1,   217,
      -1,   413,    -1,   217,    -1,    46,   414,    -1,    47,   414,
      -1,   131,   200,   416,   201,    -1,    66,   416,   207,    -1,
     173,   200,   419,   201,    -1,   417,   410,    -1,    -1,   417,
       9,   415,   129,   415,    -1,   417,     9,   415,    -1,   415,
     129,   415,    -1,   415,    -1,   418,     9,   415,    -1,   415,
      -1,   420,   410,    -1,    -1,   420,     9,   353,   129,   415,
      -1,   353,   129,   415,    -1,   418,   410,    -1,    -1,   200,
     421,   201,    -1,    -1,   423,     9,   217,   422,    -1,   217,
     422,    -1,    -1,   425,   423,   410,    -1,    45,   424,    44,
      -1,   426,    -1,    -1,   127,    -1,   128,    -1,   217,    -1,
     203,   341,   204,    -1,   429,    -1,   439,    -1,    66,   442,
     207,    -1,   203,   341,   204,    -1,   435,   431,    -1,   200,
     331,   201,   431,    -1,   445,   431,    -1,   200,   331,   201,
     431,    -1,   439,    -1,   398,    -1,   437,    -1,   438,    -1,
     432,    -1,   434,   428,   430,    -1,   200,   331,   201,   428,
     430,    -1,   400,   148,   439,    -1,   436,   200,   291,   201,
      -1,   200,   434,   201,    -1,   398,    -1,   437,    -1,   438,
      -1,   432,    -1,   434,   428,   429,    -1,   200,   331,   201,
     428,   429,    -1,   436,   200,   291,   201,    -1,   200,   434,
     201,    -1,   439,    -1,   432,    -1,   200,   434,   201,    -1,
     434,   428,   430,   471,   200,   291,   201,    -1,   200,   331,
     201,   428,   430,   471,   200,   291,   201,    -1,   400,   148,
     217,   471,   200,   291,   201,    -1,   400,   148,   439,   200,
     291,   201,    -1,   400,   148,   203,   341,   204,   200,   291,
     201,    -1,   440,    -1,   443,   440,    -1,   440,    66,   442,
     207,    -1,   440,   203,   341,   204,    -1,   441,    -1,    79,
      -1,   205,   203,   341,   204,    -1,   341,    -1,    -1,   205,
      -1,   443,   205,    -1,   439,    -1,   433,    -1,   444,   428,
     430,    -1,   200,   331,   201,   428,   430,    -1,   400,   148,
     439,    -1,   200,   434,   201,    -1,    -1,   433,    -1,   444,
     428,   429,    -1,   200,   331,   201,   428,   429,    -1,   200,
     434,   201,    -1,   446,     9,    -1,   446,     9,   434,    -1,
     446,     9,   130,   200,   446,   201,    -1,    -1,   434,    -1,
     130,   200,   446,   201,    -1,   448,   410,    -1,    -1,   448,
       9,   341,   129,   341,    -1,   448,     9,   341,    -1,   341,
     129,   341,    -1,   341,    -1,   448,     9,   341,   129,    35,
     434,    -1,   448,     9,    35,   434,    -1,   341,   129,    35,
     434,    -1,    35,   434,    -1,   450,   410,    -1,    -1,   450,
       9,   341,   129,   341,    -1,   450,     9,   341,    -1,   341,
     129,   341,    -1,   341,    -1,   452,   410,    -1,    -1,   452,
       9,   406,   129,   406,    -1,   452,     9,   406,    -1,   406,
     129,   406,    -1,   406,    -1,   455,   410,    -1,    -1,   456,
     410,    -1,    -1,   455,     9,   341,   129,   341,    -1,   341,
     129,   341,    -1,   456,     9,   341,    -1,   341,    -1,   459,
     410,    -1,    -1,   460,   410,    -1,    -1,   459,     9,   406,
     129,   406,    -1,   406,   129,   406,    -1,   460,     9,   406,
      -1,   406,    -1,   461,   462,    -1,   461,    84,    -1,   462,
      -1,    84,   462,    -1,    79,    -1,    79,    66,   463,   207,
      -1,    79,   428,   217,    -1,   146,   341,   204,    -1,   146,
      78,    66,   341,   207,   204,    -1,   147,   434,   204,    -1,
     217,    -1,    80,    -1,    79,    -1,   120,   200,   465,   201,
      -1,   121,   200,   434,   201,    -1,   121,   200,   342,   201,
      -1,   121,   200,   331,   201,    -1,     7,   341,    -1,     6,
     341,    -1,     5,   200,   341,   201,    -1,     4,   341,    -1,
       3,   341,    -1,   434,    -1,   465,     9,   434,    -1,   400,
     148,   217,    -1,   400,   148,   123,    -1,    -1,    96,   482,
      -1,   177,   470,    14,   482,   202,    -1,   179,   470,   467,
      14,   482,   202,    -1,   217,    -1,   482,   217,    -1,   217,
      -1,   217,   169,   476,   170,    -1,   169,   473,   170,    -1,
      -1,   482,    -1,   472,     9,   482,    -1,   472,   410,    -1,
     472,     9,   162,    -1,   473,    -1,   162,    -1,    -1,    -1,
      30,   482,    -1,   476,     9,   477,   217,    -1,   477,   217,
      -1,   476,     9,   477,   217,    96,   482,    -1,   477,   217,
      96,   482,    -1,    46,    -1,    47,    -1,    -1,    85,   129,
     482,    -1,   228,   148,   217,   129,   482,    -1,   479,     9,
     478,    -1,   478,    -1,   479,   410,    -1,    -1,   173,   200,
     480,   201,    -1,    29,   482,    -1,    55,   482,    -1,   228,
      -1,   131,    -1,   132,    -1,   481,    -1,   131,   169,   482,
     170,    -1,   131,   169,   482,     9,   482,   170,    -1,   153,
      -1,   200,   104,   200,   474,   201,    30,   482,   201,    -1,
     200,   482,     9,   472,   410,   201,    -1,   482,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   736,   736,   736,   745,   747,   750,   751,   752,   753,
     754,   755,   756,   759,   761,   761,   763,   763,   765,   766,
     768,   770,   775,   776,   777,   778,   779,   780,   781,   782,
     783,   784,   785,   786,   787,   788,   789,   790,   791,   792,
     793,   797,   799,   803,   805,   809,   811,   815,   816,   817,
     818,   823,   824,   825,   826,   831,   832,   833,   834,   839,
     840,   844,   845,   847,   850,   856,   863,   870,   874,   880,
     882,   885,   886,   887,   888,   891,   892,   896,   901,   901,
     907,   907,   914,   913,   919,   919,   924,   925,   926,   927,
     928,   929,   930,   931,   932,   933,   934,   935,   936,   939,
     937,   946,   944,   951,   959,   953,   963,   961,   965,   966,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   988,   988,   993,   999,  1003,  1003,  1011,  1012,  1016,
    1017,  1021,  1026,  1025,  1038,  1036,  1050,  1048,  1064,  1063,
    1072,  1070,  1082,  1081,  1100,  1098,  1117,  1116,  1125,  1123,
    1135,  1134,  1146,  1144,  1157,  1158,  1162,  1165,  1168,  1169,
    1170,  1173,  1174,  1177,  1179,  1182,  1183,  1186,  1187,  1190,
    1191,  1195,  1196,  1201,  1202,  1205,  1206,  1207,  1211,  1212,
    1216,  1217,  1221,  1222,  1226,  1227,  1232,  1233,  1238,  1239,
    1240,  1241,  1244,  1247,  1249,  1252,  1253,  1257,  1259,  1262,
    1265,  1268,  1269,  1272,  1273,  1277,  1283,  1289,  1296,  1298,
    1303,  1308,  1314,  1318,  1322,  1326,  1331,  1336,  1341,  1346,
    1352,  1361,  1366,  1371,  1377,  1379,  1383,  1387,  1392,  1396,
    1399,  1402,  1406,  1410,  1414,  1418,  1423,  1431,  1433,  1436,
    1437,  1438,  1439,  1441,  1443,  1448,  1449,  1452,  1453,  1454,
    1458,  1459,  1461,  1462,  1466,  1468,  1471,  1475,  1481,  1483,
    1486,  1486,  1490,  1489,  1493,  1497,  1495,  1510,  1507,  1520,
    1522,  1524,  1526,  1528,  1530,  1532,  1536,  1537,  1538,  1541,
    1547,  1550,  1556,  1559,  1564,  1566,  1571,  1576,  1580,  1581,
    1587,  1588,  1590,  1594,  1595,  1600,  1601,  1605,  1606,  1610,
    1612,  1618,  1623,  1624,  1626,  1630,  1631,  1632,  1633,  1637,
    1638,  1639,  1640,  1641,  1642,  1644,  1649,  1652,  1653,  1657,
    1658,  1662,  1663,  1666,  1667,  1670,  1671,  1674,  1675,  1679,
    1680,  1681,  1682,  1683,  1684,  1685,  1689,  1690,  1693,  1694,
    1695,  1698,  1700,  1702,  1703,  1706,  1708,  1713,  1714,  1716,
    1717,  1718,  1721,  1725,  1726,  1730,  1731,  1735,  1736,  1737,
    1741,  1745,  1750,  1754,  1758,  1763,  1764,  1765,  1768,  1770,
    1771,  1772,  1775,  1776,  1777,  1778,  1779,  1780,  1781,  1782,
    1783,  1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,
    1793,  1794,  1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,
    1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,
    1813,  1814,  1815,  1816,  1817,  1819,  1820,  1822,  1824,  1825,
    1826,  1827,  1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,
    1836,  1837,  1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,
    1846,  1850,  1854,  1859,  1858,  1873,  1871,  1888,  1888,  1904,
    1903,  1921,  1921,  1937,  1936,  1957,  1958,  1959,  1964,  1966,
    1970,  1974,  1980,  1984,  1990,  1992,  1996,  1998,  2002,  2006,
    2007,  2011,  2018,  2019,  2023,  2027,  2029,  2034,  2039,  2046,
    2048,  2053,  2054,  2055,  2057,  2061,  2065,  2069,  2073,  2075,
    2077,  2079,  2084,  2085,  2090,  2091,  2092,  2093,  2094,  2095,
    2099,  2103,  2107,  2111,  2116,  2121,  2125,  2126,  2130,  2131,
    2135,  2136,  2140,  2141,  2145,  2149,  2153,  2157,  2158,  2159,
    2160,  2164,  2170,  2179,  2192,  2193,  2196,  2199,  2202,  2203,
    2206,  2210,  2213,  2216,  2223,  2224,  2228,  2229,  2231,  2235,
    2236,  2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,
    2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,
    2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,
    2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,
    2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,
    2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,
    2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2318,
    2323,  2324,  2327,  2328,  2329,  2333,  2334,  2335,  2339,  2340,
    2341,  2345,  2346,  2347,  2350,  2352,  2356,  2357,  2358,  2359,
    2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,  2370,
    2373,  2378,  2379,  2380,  2382,  2383,  2385,  2386,  2387,  2388,
    2389,  2390,  2392,  2394,  2396,  2398,  2400,  2401,  2402,  2403,
    2404,  2405,  2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,
    2414,  2415,  2416,  2418,  2420,  2422,  2424,  2425,  2428,  2429,
    2433,  2435,  2439,  2442,  2445,  2451,  2452,  2453,  2454,  2455,
    2456,  2457,  2462,  2464,  2468,  2469,  2472,  2473,  2477,  2480,
    2482,  2484,  2488,  2489,  2490,  2491,  2493,  2496,  2500,  2501,
    2502,  2503,  2506,  2507,  2508,  2509,  2510,  2512,  2513,  2518,
    2520,  2523,  2526,  2528,  2530,  2533,  2535,  2539,  2541,  2544,
    2547,  2553,  2555,  2558,  2559,  2564,  2567,  2571,  2571,  2576,
    2579,  2580,  2584,  2585,  2589,  2590,  2594,  2595,  2599,  2600,
    2604,  2605,  2610,  2612,  2617,  2618,  2619,  2620,  2621,  2622,
    2624,  2627,  2630,  2632,  2636,  2637,  2638,  2639,  2640,  2642,
    2645,  2647,  2651,  2652,  2653,  2657,  2660,  2667,  2671,  2675,
    2682,  2683,  2688,  2690,  2691,  2694,  2695,  2698,  2699,  2703,
    2704,  2708,  2709,  2710,  2713,  2716,  2719,  2722,  2723,  2724,
    2726,  2729,  2733,  2734,  2735,  2737,  2738,  2739,  2743,  2745,
    2748,  2750,  2751,  2752,  2753,  2756,  2758,  2759,  2763,  2765,
    2768,  2770,  2771,  2772,  2776,  2778,  2781,  2784,  2786,  2788,
    2792,  2794,  2797,  2799,  2802,  2804,  2807,  2808,  2812,  2814,
    2817,  2819,  2822,  2825,  2829,  2831,  2835,  2836,  2838,  2839,
    2845,  2846,  2848,  2850,  2852,  2854,  2857,  2858,  2859,  2863,
    2864,  2865,  2866,  2867,  2868,  2869,  2870,  2871,  2875,  2876,
    2880,  2882,  2890,  2892,  2896,  2900,  2907,  2908,  2914,  2915,
    2922,  2925,  2929,  2932,  2937,  2942,  2944,  2945,  2946,  2950,
    2951,  2955,  2957,  2958,  2961,  2966,  2967,  2968,  2972,  2975,
    2984,  2986,  2990,  2993,  2996,  3004,  3007,  3010,  3011,  3014,
    3017,  3018,  3021,  3025,  3029,  3035,  3045,  3046
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
  "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR", "T_DOUBLE_ARROW",
  "T_LIST", "T_ARRAY", "T_CALLABLE", "T_CLASS_C", "T_METHOD_C", "T_FUNC_C",
  "T_LINE", "T_FILE", "T_COMMENT", "T_DOC_COMMENT", "T_OPEN_TAG",
  "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG", "T_WHITESPACE", "T_START_HEREDOC",
  "T_END_HEREDOC", "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN",
  "T_DOUBLE_COLON", "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE",
  "T_VARRAY", "T_MIARRAY", "T_MSARRAY", "T_TYPE", "T_UNRESOLVED_TYPE",
  "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE", "T_COMPILER_HALT_OFFSET", "T_ASYNC",
  "T_FROM", "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO",
  "T_LET", "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT",
  "T_GROUP", "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP",
  "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''",
  "$accept", "start", "$@1", "top_statement_list", "top_statement", "$@2",
  "$@3", "ident", "use_declarations", "use_fn_declarations",
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
  "$@30", "lambda_expression", "$@31", "$@32", "$@33", "$@34",
  "lambda_body", "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "map_array_literal", "varray_literal",
  "static_map_array_literal", "static_varray_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base", "query_expr",
  "query_assign_expr", "query_head", "query_body", "query_body_clauses",
  "query_body_clause", "from_clause", "let_clause", "where_clause",
  "join_clause", "join_into_clause", "orderby_clause", "orderings",
  "ordering", "ordering_direction", "select_or_group_clause",
  "select_clause", "group_clause", "query_continuation",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
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
  "user_attribute_list", "$@35", "non_empty_user_attributes",
  "optional_user_attributes", "object_operator",
  "object_member_name_no_variables", "object_member_name", "array_access",
  "dimmable_variable_access", "dimmable_variable_no_calls_access",
  "variable", "dimmable_variable", "callable_variable",
  "object_method_call", "class_method_call", "variable_no_objects",
  "reference_variable", "compound_variable", "dim_offset",
  "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "map_array_init", "varray_init",
  "non_empty_map_array_init", "non_empty_varray_init",
  "static_map_array_init", "static_varray_init",
  "non_empty_static_map_array_init", "non_empty_static_varray_init",
  "encaps_list", "encaps_var", "encaps_var_offset", "internal_functions",
  "variable_list", "class_constant", "hh_opt_constraint",
  "hh_type_alias_statement", "hh_name_with_type", "hh_name_with_typevar",
  "hh_typeargs_opt", "hh_non_empty_type_list", "hh_type_list",
  "hh_func_type_list", "hh_opt_return_type", "hh_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
      40,    41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   210,   212,   211,   213,   213,   214,   214,   214,   214,
     214,   214,   214,   214,   215,   214,   216,   214,   214,   214,
     214,   214,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   218,   218,   219,   219,   220,   220,   221,   221,   221,
     221,   222,   222,   222,   222,   223,   223,   223,   223,   224,
     224,   225,   225,   225,   226,   227,   228,   229,   229,   230,
     230,   231,   231,   231,   231,   232,   232,   232,   233,   232,
     234,   232,   235,   232,   236,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   237,
     232,   238,   232,   232,   239,   232,   240,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   242,   241,   243,   243,   245,   244,   246,   246,   247,
     247,   248,   250,   249,   251,   249,   252,   249,   254,   253,
     255,   253,   257,   256,   258,   256,   259,   256,   260,   256,
     262,   261,   263,   261,   264,   264,   265,   266,   267,   267,
     267,   267,   267,   268,   268,   269,   269,   270,   270,   271,
     271,   272,   272,   273,   273,   274,   274,   274,   275,   275,
     276,   276,   277,   277,   278,   278,   279,   279,   280,   280,
     280,   280,   281,   281,   281,   282,   282,   283,   283,   284,
     284,   285,   285,   286,   286,   287,   287,   287,   287,   287,
     287,   287,   287,   288,   288,   288,   288,   288,   288,   288,
     288,   289,   289,   289,   289,   289,   289,   289,   289,   290,
     290,   290,   290,   290,   290,   290,   290,   291,   291,   292,
     292,   292,   292,   292,   292,   293,   293,   294,   294,   294,
     295,   295,   295,   295,   296,   296,   297,   298,   299,   299,
     301,   300,   302,   300,   300,   303,   300,   304,   300,   300,
     300,   300,   300,   300,   300,   300,   305,   305,   305,   306,
     307,   307,   308,   308,   309,   309,   310,   310,   311,   311,
     311,   311,   311,   312,   312,   313,   313,   314,   314,   315,
     315,   316,   317,   317,   317,   318,   318,   318,   318,   319,
     319,   319,   319,   319,   319,   319,   320,   320,   320,   321,
     321,   322,   322,   323,   323,   324,   324,   325,   325,   326,
     326,   326,   326,   326,   326,   326,   327,   327,   328,   328,
     328,   329,   329,   329,   329,   330,   330,   331,   331,   331,
     331,   331,   332,   333,   333,   334,   334,   335,   335,   335,
     336,   337,   338,   339,   340,   341,   341,   341,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   343,   343,   345,   344,   346,   344,   348,   347,   349,
     347,   350,   347,   351,   347,   352,   352,   352,   353,   353,
     354,   354,   355,   355,   356,   356,   357,   357,   358,   359,
     359,   360,   361,   361,   362,   363,   363,   364,   365,   366,
     366,   367,   367,   367,   367,   368,   369,   370,   371,   371,
     371,   371,   372,   372,   373,   373,   373,   373,   373,   373,
     374,   375,   376,   377,   378,   379,   380,   380,   381,   381,
     382,   382,   383,   383,   384,   385,   386,   387,   387,   387,
     387,   388,   389,   389,   390,   390,   391,   391,   392,   392,
     393,   394,   394,   395,   395,   395,   396,   396,   396,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   398,
     399,   399,   400,   400,   400,   401,   401,   401,   402,   402,
     402,   403,   403,   403,   404,   404,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   407,   407,   407,   408,   408,   408,   408,   408,
     408,   408,   409,   409,   410,   410,   411,   411,   412,   412,
     412,   412,   413,   413,   413,   413,   413,   413,   414,   414,
     414,   414,   415,   415,   415,   415,   415,   415,   415,   416,
     416,   417,   417,   417,   417,   418,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   425,   424,   426,
     427,   427,   428,   428,   429,   429,   430,   430,   431,   431,
     432,   432,   433,   433,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   435,   435,   435,   435,   435,   435,
     435,   435,   436,   436,   436,   437,   437,   438,   438,   438,
     439,   439,   440,   440,   440,   441,   441,   442,   442,   443,
     443,   444,   444,   444,   444,   444,   444,   445,   445,   445,
     445,   445,   446,   446,   446,   446,   446,   446,   447,   447,
     448,   448,   448,   448,   448,   448,   448,   448,   449,   449,
     450,   450,   450,   450,   451,   451,   452,   452,   452,   452,
     453,   453,   454,   454,   455,   455,   456,   456,   457,   457,
     458,   458,   459,   459,   460,   460,   461,   461,   461,   461,
     462,   462,   462,   462,   462,   462,   463,   463,   463,   464,
     464,   464,   464,   464,   464,   464,   464,   464,   465,   465,
     466,   466,   467,   467,   468,   468,   469,   469,   470,   470,
     471,   471,   472,   472,   473,   474,   474,   474,   474,   475,
     475,   476,   476,   476,   476,   477,   477,   477,   478,   478,
     479,   479,   480,   480,   481,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   483,   483
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
       3,     3,     2,     2,     0,     2,     0,     2,     0,     1,
       3,     1,     3,     2,     0,     1,     2,     4,     1,     4,
       1,     4,     1,     4,     1,     4,     3,     5,     3,     4,
       4,     5,     5,     4,     0,     1,     1,     4,     0,     5,
       0,     2,     0,     3,     0,     7,     8,     6,     2,     5,
       6,     4,     0,     4,     5,     7,     6,     6,     7,     9,
       8,     6,     7,     5,     2,     4,     5,     3,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     2,     3,     4,     4,     3,     1,     1,     2,     4,
       3,     5,     1,     3,     2,     0,     2,     3,     2,     0,
       0,     4,     0,     5,     2,     0,    10,     0,    11,     3,
       3,     3,     4,     4,     3,     5,     2,     2,     0,     6,
       5,     4,     3,     1,     1,     3,     4,     1,     1,     1,
       1,     4,     1,     1,     3,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     2,
       1,     1,     3,     3,     1,     1,     0,     1,     2,     4,
       3,     6,     2,     3,     6,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     5,     0,     0,    11,     0,    12,     0,     3,     0,
       4,     0,     6,     0,     7,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     1,     1,     1,     3,     2,     3,     4,     2,     3,
       1,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       4,     4,     2,     8,    10,     2,     1,     3,     1,     2,
       1,     1,     1,     1,     2,     4,     3,     3,     4,     1,
       2,     4,     2,     6,     0,     1,     4,     0,     2,     0,
       1,     1,     3,     1,     3,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     4,     3,     4,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     3,     3,     3,     1,     1,     1,     1,     3,
       3,     3,     2,     0,     1,     0,     1,     0,     5,     3,
       3,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     2,     2,     4,     3,     4,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     3,     1,     1,     3,     3,
       2,     4,     2,     4,     1,     1,     1,     1,     1,     3,
       5,     3,     4,     3,     1,     1,     1,     1,     3,     5,
       4,     3,     1,     1,     3,     7,     9,     7,     6,     8,
       1,     2,     4,     4,     1,     1,     4,     1,     0,     1,
       2,     1,     1,     3,     5,     3,     3,     0,     1,     3,
       5,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     0,     2,     0,     5,     3,     3,     1,     2,     0,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     4,     2,     6,     4,     1,     1,     0,     3,     5,
       3,     1,     2,     0,     4,     2,     2,     1,     1,     1,
       1,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   357,     0,   747,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   819,     0,
     807,   630,     0,   636,   637,   638,    22,   696,   795,    98,
     639,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   329,   330,
     331,   334,   333,   332,     0,     0,     0,     0,   158,     0,
       0,     0,   643,   645,   646,   640,   641,     0,     0,   647,
     642,     0,   621,    23,    24,    25,    27,    26,     0,   644,
       0,     0,     0,     0,     0,     0,     0,   648,   335,    28,
      29,    31,    30,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   451,     0,    97,    70,   799,   631,     0,     0,
       4,    59,    61,    64,   695,     0,   620,     0,     6,   130,
       7,     9,     8,    10,     0,     0,   327,   367,     0,     0,
       0,     0,     0,     0,     0,   365,   438,   439,   433,   432,
     351,   434,   435,   440,     0,     0,   350,   765,   622,     0,
     698,   431,   326,   768,   366,     0,     0,   766,   767,   764,
     790,   794,     0,   421,   697,    11,   334,   333,   332,     0,
       0,    27,    59,   130,     0,   877,   366,   876,     0,   874,
     873,   437,     0,   358,   362,     0,     0,   405,   406,   407,
     408,   430,   428,   427,   426,   425,   424,   423,   422,   795,
     623,     0,   891,   622,     0,   387,   385,     0,   823,     0,
     705,   349,   626,     0,   891,   625,     0,   635,   802,   801,
     627,     0,     0,   629,   429,     0,     0,     0,     0,   354,
       0,    78,   356,     0,     0,    84,    86,     0,     0,    88,
       0,     0,     0,   918,   919,   923,     0,     0,    59,   917,
       0,   920,     0,     0,    90,     0,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,    42,    47,   247,     0,
       0,   246,     0,   162,     0,   159,   252,     0,     0,     0,
       0,     0,   888,   146,   156,   815,   819,   860,     0,   650,
       0,     0,     0,   858,     0,    16,     0,    63,   138,   150,
     157,   527,   465,   843,   841,   841,     0,   882,   449,   453,
     751,   367,     0,   365,   366,     0,     0,   632,     0,   633,
       0,     0,     0,   120,     0,     0,    66,   238,     0,    21,
     129,     0,   155,   142,   154,   332,   335,   130,   328,   111,
     112,   113,   114,   115,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     807,     0,   110,   798,   798,   118,   829,     0,     0,     0,
       0,     0,     0,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   384,   752,
     753,     0,   798,     0,   760,   238,   798,     0,   800,   791,
     815,     0,   130,     0,     0,    92,     0,   749,   744,   705,
       0,     0,     0,     0,   827,     0,   470,   704,   818,     0,
       0,    66,     0,   238,   348,     0,   762,   628,     0,    70,
     198,     0,   448,     0,    95,     0,     0,   355,     0,     0,
       0,     0,     0,    87,   109,    89,   915,   916,     0,   913,
       0,     0,     0,   887,     0,   116,    91,   119,     0,     0,
       0,     0,     0,     0,     0,   485,     0,   492,   494,   495,
     496,   497,   498,   499,   490,   512,   513,    70,     0,   106,
     108,     0,     0,    44,    51,     0,     0,    46,    55,    48,
       0,    18,     0,     0,   248,     0,    93,   161,   160,     0,
       0,    94,   878,     0,     0,   367,   365,   366,     0,   907,
     168,     0,   816,     0,     0,     0,     0,   649,   859,   696,
       0,     0,   857,   701,   856,    62,     5,    13,    14,     0,
     166,     0,     0,   458,     0,     0,   705,     0,     0,   624,
     459,   847,     0,   705,     0,     0,   705,     0,     0,     0,
       0,     0,   751,     0,   707,   750,   927,   347,   418,   773,
      75,    69,    71,    72,    73,    74,   326,     0,   436,   699,
     700,    60,   705,     0,   892,     0,     0,     0,   707,   239,
       0,   443,   132,   164,     0,   390,   392,   391,     0,     0,
     388,   389,   393,   395,   394,   410,   409,   412,   411,   413,
     415,   416,   414,   404,   403,   397,   398,   396,   399,   400,
     402,   417,   401,   797,     0,     0,   833,     0,   705,   881,
       0,   880,   771,   790,   148,   140,   152,   144,   130,   357,
       0,   360,   363,   369,   486,   383,   382,   381,   380,   379,
     378,   377,   376,   375,   374,   373,   372,     0,   754,   756,
     769,   757,     0,     0,     0,     0,     0,     0,     0,   875,
     359,   742,   746,   704,   748,     0,     0,   891,     0,   822,
       0,   821,     0,   806,   805,     0,   756,   803,   352,   200,
     202,    70,   456,   455,   353,     0,    70,   182,    79,   356,
       0,     0,     0,     0,     0,   194,   194,    85,     0,     0,
       0,   911,   705,     0,   898,     0,     0,     0,     0,     0,
     703,   639,     0,     0,   621,     0,     0,     0,     0,     0,
      64,   652,   620,   658,   659,   657,     0,   651,    68,   656,
       0,     0,   502,     0,     0,   508,   505,   506,   514,     0,
     493,   488,     0,   491,     0,     0,     0,    52,    19,     0,
       0,    56,    20,     0,     0,     0,    41,    49,     0,   245,
     253,   250,     0,     0,   869,   872,   871,   870,    12,   905,
     906,     0,     0,     0,     0,   815,   812,     0,   469,   868,
     867,   866,     0,   862,     0,   863,   865,     0,     5,     0,
       0,     0,   521,   522,   530,   529,     0,     0,   704,   464,
     468,     0,   474,   704,   842,     0,   472,   704,   840,   473,
       0,   883,     0,   450,     0,   899,   751,   224,   926,     0,
       0,   761,   796,   704,   894,   890,   240,   241,   619,   706,
     237,     0,   751,     0,     0,   166,   445,   134,   420,     0,
     479,   480,     0,   471,   704,   828,     0,     0,   238,   168,
       0,   166,   164,     0,   807,   370,     0,     0,   758,   759,
     772,   792,   793,     0,     0,     0,   730,   712,   713,   714,
     715,     0,     0,     0,   723,   722,   736,   705,     0,   744,
     826,   825,     0,     0,   763,   634,   204,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,   174,   175,   186,
       0,    70,   184,   103,   194,     0,   194,     0,     0,   921,
       0,     0,   704,   912,   914,   897,   705,   896,     0,   705,
     680,   681,   678,   679,   711,     0,   705,   703,     0,     0,
     467,   851,   849,   849,     0,     0,   835,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   487,     0,     0,     0,   510,   511,   509,
       0,     0,   489,     0,   122,     0,   125,   107,     0,    43,
      53,     0,    45,    57,    50,   249,     0,   879,    96,   907,
     889,   902,   167,   169,   259,     0,     0,   813,     0,   861,
       0,    17,     0,   882,   165,   259,     0,     0,   461,     0,
     880,   846,   845,     0,   884,     0,   899,     0,     0,   927,
       0,   229,   227,   756,   770,   893,     0,     0,   242,    67,
       0,   751,   163,     0,   751,     0,   419,   832,   831,     0,
     238,     0,     0,     0,     0,   166,   136,   635,   755,   238,
       0,   718,   719,   720,   721,   724,   725,   734,     0,   705,
     730,     0,   717,   738,   704,   741,   743,   745,     0,   820,
     756,   804,     0,     0,     0,     0,   201,   457,    81,     0,
     356,   174,   176,   815,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   188,     0,   908,     0,   910,   704,     0,
       0,     0,   654,   704,   702,     0,   693,     0,   705,     0,
     855,     0,   705,     0,     0,   705,     0,   660,   694,   692,
     839,     0,   705,   663,   665,   664,     0,     0,   661,   662,
     666,   668,   667,   683,   682,   685,   684,   686,   688,   689,
     687,   676,   675,   670,   671,   669,   672,   673,   674,   677,
     500,     0,   501,   507,   515,   516,     0,    70,    54,    58,
     251,     0,     0,     0,   326,   817,   815,   361,   364,   368,
       0,    15,     0,   326,   533,     0,     0,   535,   528,   531,
       0,   526,     0,     0,   885,     0,   900,   452,     0,   230,
       0,     0,   225,     0,   244,   243,   899,     0,   259,     0,
     751,     0,   238,     0,   788,   259,   882,   259,     0,     0,
     371,     0,     0,   727,   704,   729,     0,   716,     0,     0,
     705,   735,   824,     0,    70,     0,   197,   183,     0,     0,
       0,   173,    99,   187,     0,     0,   190,     0,   195,   196,
      70,   189,   922,     0,   895,     0,   925,   710,   709,   653,
       0,   704,   466,   655,   477,   704,   850,     0,   475,   704,
     848,   476,     0,   478,   704,   834,   691,     0,     0,     0,
       0,   901,   904,   170,     0,     0,     0,   324,     0,     0,
       0,   147,   258,   260,     0,   323,     0,   326,     0,   864,
     255,   151,   524,     0,     0,   460,   844,   454,     0,   233,
     223,     0,   226,   232,   238,   442,   899,   326,   899,     0,
     830,     0,   787,   326,     0,   326,   259,   751,   785,   733,
     732,   726,     0,   728,   704,   737,    70,   203,    77,    82,
     101,   177,     0,   185,   191,    70,   193,   909,     0,     0,
     463,     0,   854,   853,     0,   838,   837,   690,     0,    70,
     126,     0,     0,     0,     0,     0,   171,   290,   288,   292,
     621,    27,     0,   284,     0,   289,   301,     0,   299,   304,
       0,   303,     0,   302,     0,   130,   262,     0,   264,     0,
     814,     0,   525,   523,   534,   532,   234,     0,     0,   221,
     231,     0,     0,     0,     0,   143,   442,   899,   789,   149,
     255,   153,   326,     0,     0,   740,     0,   199,     0,     0,
      70,   180,   100,   192,   924,   708,     0,     0,     0,     0,
       0,   903,     0,     0,     0,     0,   274,   278,     0,     0,
     269,   585,   584,   581,   583,   582,   602,   604,   603,   573,
     544,   545,   563,   579,   578,   540,   550,   551,   553,   552,
     572,   556,   554,   555,   557,   558,   559,   560,   561,   562,
     564,   565,   566,   567,   568,   569,   571,   570,   541,   542,
     543,   546,   547,   549,   587,   588,   597,   596,   595,   594,
     593,   592,   580,   599,   589,   590,   591,   574,   575,   576,
     577,   600,   601,   605,   607,   606,   608,   609,   586,   611,
     610,   613,   615,   614,   548,   618,   616,   617,   612,   598,
     539,   296,   536,     0,   270,   317,   318,   316,   309,     0,
     310,   271,   343,     0,     0,     0,     0,   130,   139,   254,
       0,     0,     0,   222,   236,   786,     0,    70,   319,    70,
     133,     0,     0,     0,   145,   899,   731,     0,    70,   178,
      83,   102,     0,   462,   852,   836,   503,   124,   272,   273,
     346,   172,     0,     0,   293,   285,     0,     0,     0,   298,
     300,     0,     0,   305,   312,   313,   311,     0,     0,   261,
       0,     0,     0,     0,   256,     0,   235,     0,   519,   707,
       0,     0,    70,   135,   141,     0,   739,     0,     0,     0,
     104,   275,    59,     0,   276,   277,     0,     0,   291,   295,
     537,   538,     0,   286,   314,   315,   307,   308,   306,   344,
     341,   265,   263,   345,     0,   257,   520,   706,     0,   444,
     320,     0,   137,     0,   181,   504,     0,   128,     0,   326,
     294,   297,     0,   751,   267,     0,   517,   441,   446,   179,
       0,     0,   105,   282,     0,   325,   342,     0,   707,   337,
     751,   518,     0,   127,     0,     0,   281,   899,   751,   208,
     338,   339,   340,   927,   336,     0,     0,     0,   280,     0,
     337,     0,   899,     0,   279,   321,    70,   266,   927,     0,
     213,   211,     0,    70,     0,     0,   214,     0,     0,   209,
     268,     0,   322,     0,   217,   207,     0,   210,   216,   123,
     218,     0,     0,   205,   215,     0,   206,   220,   219
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   808,   546,   182,   275,   502,
     506,   276,   503,   507,   122,   123,   124,   125,   126,   127,
     325,   581,   582,   456,   240,  1428,   462,  1352,  1429,  1657,
     766,   270,   497,  1620,   997,  1177,  1672,   341,   183,   583,
     853,  1055,  1229,   131,   549,   870,   584,   603,   872,   530,
     869,   585,   550,   871,   343,   293,   309,   134,   855,   811,
     794,  1012,  1375,  1105,   917,  1570,  1432,   708,   923,   461,
     717,   925,  1260,   700,   906,   909,  1094,  1677,  1678,   573,
     574,   597,   598,   280,   281,   287,  1401,  1549,  1550,  1184,
    1302,  1394,  1545,  1663,  1680,  1582,  1624,  1625,  1626,  1382,
    1383,  1384,  1583,  1589,  1633,  1387,  1388,  1392,  1538,  1539,
    1540,  1560,  1707,  1303,  1304,   184,   136,  1693,  1694,  1543,
    1306,   137,   233,   457,   458,   138,   139,   140,   141,   142,
     143,   144,   145,  1413,   146,   852,  1054,   147,   237,   571,
     320,   572,   452,   555,   556,  1128,   557,  1129,   148,   149,
     150,   151,   152,   743,   744,   745,   153,   154,   267,   155,
     268,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     756,   757,   989,   494,   495,   496,   763,  1609,   156,   551,
    1403,   552,  1026,   816,  1201,  1198,  1531,  1532,   157,   158,
     159,   227,   234,   328,   444,   160,   944,   749,   161,   945,
     844,   837,   946,   895,  1075,  1077,  1078,  1079,   897,  1239,
    1240,   898,   682,   429,   195,   196,   586,   576,   411,   669,
     670,   841,   163,   228,   186,   165,   166,   167,   168,   169,
     170,   171,   634,   172,   230,   231,   533,   219,   220,   637,
     638,  1141,  1142,   565,   562,   566,   563,  1134,  1131,  1135,
    1132,   302,   303,   802,   173,   523,   174,   570,   175,  1551,
     294,   336,   592,   593,   938,  1038,   791,   792,   721,   722,
     723,   261,   262,   839
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1430
static const yytype_int16 yypact[] =
{
   -1430,   137, -1430, -1430,  6149, 13805, 13805,   -30, 13805, 13805,
   13805, 11903, 13805, -1430, 13805, 13805, 13805, 13805, 13805, 13805,
   13805, 13805, 13805, 13805, 13805, 13805, 14748, 14748, 12110, 13805,
   14979,   -28,    -5, -1430, -1430, -1430, -1430, -1430,   145, -1430,
     115, 13805, -1430,    -5,    19,    24,   146,    -5, 12275, 15907,
   12440, -1430, 14594, 10910,    10, 13805,  5347,    37, -1430, -1430,
   -1430,    26,   297,    36,   171,   178,   195,   210, -1430, 15907,
     230,   243, -1430, -1430, -1430, -1430, -1430,   281,  1861, -1430,
   -1430, 15907, -1430, -1430, -1430, -1430, 15907, -1430, 15907, -1430,
     282,   257,   263,   290,   292, 15907, 15907, -1430,   278, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, 13805, -1430, -1430,   314,   332,   335,   335,
   -1430,   192,    77,     8, -1430,   299, -1430,    61, -1430,   378,
   -1430, -1430, -1430, -1430, 14613,   389, -1430, -1430,   324,   327,
     333,   339,   341,   350,  3767, -1430, -1430, -1430, -1430,   493,
   -1430, -1430, -1430,   497,   517,   379, -1430,    44,   383,   443,
   -1430, -1430,   509,    20,  2194,    82,   411,   116,   123,   418,
     179, -1430,   108, -1430,   565, -1430, -1430, -1430,   487,   440,
     503, -1430, -1430,   378,   389, 16164,  2276, 16164, 13805, 16164,
   16164, 11100,   459, 15250, 11100,   619, 15907,   602,   602,    92,
     602,   602,   602,   602,   602,   602,   602,   602,   602, -1430,
   -1430,  3459,   500, -1430,   523,    31,    31, 14748, 15294,   467,
     669, -1430,   487,  3459,   500,   532,   538,   492,   125, -1430,
      31,    82, 12605, -1430, -1430, 13805,  9461,   689,    67, 16164,
   10496, -1430, 13805, 13805, 15907, -1430, -1430,  3905,   505, -1430,
    3962, 14594, 14594,   542, -1430, -1430,   512,  1910,   700, -1430,
     702, -1430, 15907,   640, -1430,   522,  4363,   524,   591, -1430,
      60,  4419, 15001, 15770, 15907,    69, -1430,    -6, -1430,  4149,
      71, -1430,   604, -1430,   609, -1430,   719,    72, 14748, 14748,
   13805,   533,   567, -1430, -1430,  4562, 12110,   296,   408, -1430,
   13970, 14748,   313, -1430, 15907, -1430,   316,    77, -1430, -1430,
   -1430, -1430, 15063, 13805, 13805, 13805,   723,   648, -1430, -1430,
      29,   545, 16164,   546,  1492,  6356, 13805,   319,   548,   430,
     319,   275,    68, -1430, 15907, 14594,   551, 11117, 14594, -1430,
   -1430,  2991, -1430, -1430, -1430, -1430, -1430,   378, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, 13805, 13805, 13805, 12812, 13805,
   13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805,
   13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805, 13805,
   14979, 13805, -1430, 13805, 13805, -1430, 13805,  2516, 15907, 15907,
   15907, 14613,   651,   334, 10703, 13805, 13805, 13805, 13805, 13805,
   13805, 13805, 13805, 13805, 13805, 13805, 13805, -1430, -1430, -1430,
   -1430,   457, 13805, 13805, -1430, 11117, 13805, 13805,   314,   128,
    4562,   556,   378, 13019,  4465, -1430, 13805, -1430,   558,   750,
    3459,   560,   -16,  1994,    31, 13226, -1430, 13433, -1430,   561,
      16, -1430,   213, 11117, -1430,   457, -1430, -1430,  5018, -1430,
   -1430, 11324, -1430, 13805, -1430,   676,  9668,   756,   571, 16076,
     753,    89,    41, -1430, -1430, -1430, -1430, -1430, 14594, 14770,
     569,   770, 14836, -1430,   594, -1430, -1430, -1430,   704, 13805,
     713,   716, 13805, 13805, 13805, -1430,   591, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430,   608, -1430, -1430, -1430,   598, -1430,
   -1430, 15907,   597,   791,    21, 15907,   599,   793,    27,    54,
   15784, -1430, 15907, 13805,    31,    37, -1430, -1430, -1430, 14836,
     726, -1430,    31,    93,    94,   606,   611,  1727,   612,   320,
     690,   620,    31,    97,   621,  5331, 15907, -1430, -1430,   757,
    1785,    -7, -1430, -1430, -1430,    77, -1430, -1430, -1430,   795,
     701,   661,    25, -1430,   314,   709,   830,   639,   694,   128,
   -1430, 16164,   643,   836, 15351,   646,   842,   652, 14594, 14594,
     838,   689,    29,   656,   846, -1430, 14594,    13,   790,   117,
   -1430, -1430, -1430, -1430, -1430, -1430,   626,  2348, -1430, -1430,
   -1430, -1430,   848,   693, -1430, 14748, 13805,   658,   851, 16164,
     853, -1430, -1430,   740,  4686, 11514, 13002, 11100, 13805, 16120,
   13623,  4612, 14340, 16006, 12790, 16311, 16311, 16311, 16311,  1213,
    1213,  1213,  1213,   784,   784,   634,   634,   634,    92,    92,
      92, -1430,   602, 16164,   663,   664, 15395,   668,   859, -1430,
   13805,   -35,   673,   128, -1430, -1430, -1430, -1430,   378, 13805,
    3670, -1430, -1430, 11100, -1430, 11100, 11100, 11100, 11100, 11100,
   11100, 11100, 11100, 11100, 11100, 11100, 11100, 13805, -1430,   130,
     -35, -1430,   667,  2580,   674,   671,  2941,   100,   679, -1430,
   16164, 14931, -1430, 15907, -1430,   545,    13,   500, 14748, 16164,
   14748, 15452,    13,   132, -1430,   675,   133, -1430, -1430,  9254,
     382, -1430, -1430, 16164, 16164,    -5, -1430, -1430, -1430, 13805,
     787,  3940, 14836, 15907,  9875,   683,   684, -1430,    58,   758,
     741, -1430,   879,   691, 14448, 14594, 14836, 14836, 14836, 14836,
   14836, -1430,   695,    15,   743,   697,   698,   703,   707, 14836,
       7, -1430,   745, -1430, -1430, -1430,   699, -1430, 16250, -1430,
   13805,   708, 16164,   714,   887,  5356,   901, -1430, 16164,  5292,
   -1430,   608,   833, -1430,  6563,  5540,   710,   247, -1430, 15001,
   15907,   293, -1430, 15770, 15907, 15907, -1430, -1430,  2986, -1430,
   16250,   904, 14748,   717, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430,    64, 15907,  5540,   718,  4562,  4814,   906, -1430, -1430,
   -1430, -1430,   725, -1430, 13805, -1430, -1430,  5735, -1430, 14594,
    5540,   720, -1430, -1430, -1430, -1430,   919, 13805, 15063, -1430,
   -1430, 15827, -1430, 13805, -1430, 13805, -1430, 13805, -1430, -1430,
     732, -1430, 14594, -1430,   737,   908,   124, -1430, -1430,    59,
     457, -1430, -1430, 14594, -1430, -1430,    31, 16164, -1430, 11531,
   -1430, 14836,    32,   736,  5540,   701, -1430, -1430, 13209, 13805,
   -1430, -1430, 13805, -1430, 13805, -1430,  3074,   739, 11117,   690,
     910,   701,   740, 15907, 14979,    31,  3303,   742, -1430, -1430,
     135, -1430, -1430,   927,  3445,  3445, 14931, -1430, -1430, -1430,
   -1430,   744,    17,   746, -1430, -1430, -1430,   934,   747,   558,
      31,    31, 13640,   457, -1430, -1430,   451,    -5, 10496, -1430,
    6770,   748,  6977,   751,  3940, 14748,   749,   823,    31, 16250,
     940, -1430, -1430, -1430, -1430,   495, -1430,     5, 14594, -1430,
   14594, 15907, 14770, -1430, -1430, -1430,   946, -1430,   755,   848,
     680,   680,   892,   892, 15597,   752,   949, 14836,   815, 15907,
   15063, 14836, 14836, 14836,  2452, 15844, 14836, 14836, 14836, 14836,
   14685, 14836, 14836, 14836, 14836, 14836, 14836, 14836, 14836, 14836,
   14836, 14836, 14836, 14836, 14836, 14836, 14836, 14836, 14836, 14836,
   14836, 14836, 14836, 16164, 13805, 13805, 13805, -1430, -1430, -1430,
   13805, 13805, -1430,   591, -1430,   882, -1430, -1430, 15907, -1430,
   -1430, 15907, -1430, -1430, -1430, -1430, 14836,    31, -1430,   320,
   -1430,   866,   955, -1430, -1430,   104,   765,    31, 11738, -1430,
    1728, -1430,  5942,   648,   955, -1430,     0,   -29, 16164,   839,
   -1430, 16164, 16164, 15496, -1430,   769,   908, 14594,   689, 14594,
      28,   953,   895,   138,   -35, -1430, 14748, 13805, 16164, 16250,
     774,    32, -1430,   773,    32,   777, 13209, 16164, 15553,   779,
   11117,   780,   781, 14594,   782,   701, -1430,   492, -1430, 11117,
   13805, -1430, -1430, -1430, -1430, -1430, -1430,   854,   783,   978,
   14931,   843, -1430, 15063, 14931, -1430, -1430, -1430, 14748, 16164,
     139, -1430,    -5,   959,   920, 10496, -1430, -1430, -1430,   817,
   13805,   823,    31,  4562,  3940,   794, 14836,  7184,   499,   818,
   13805,    33,   269, -1430,   822, -1430,   893, -1430, 14514,   993,
     824, 14836, -1430, 14836, -1430,   826, -1430,   900,  1015,   831,
   16250,   834,  1028, 15654,   837,  1030,   844, -1430, -1430, -1430,
   15696,   845,  1031, 12093,  5549,  4788, 14836, 16208,  3225, 14173,
   15003, 16281,  4004, 16341, 16341, 16341, 16341,   965,   965,   965,
     965,   555,   555,   680,   680,   680,   892,   892,   892,   892,
   16164,  5637, 16164, -1430, 16164, -1430,   847, -1430, -1430, -1430,
   16250, 15907, 14594,  5540,   235, -1430,  4562, -1430, -1430, 11100,
     852, -1430,   855,   432, -1430,    47, 13805, -1430, -1430, -1430,
   13805, -1430, 13805, 13805, -1430,   689, -1430, -1430,   309,  1032,
     980, 14836, -1430,   857,    31, 16164,   908,   861, -1430,   864,
      32, 13805, 11117,   869, -1430, -1430,   648, -1430,   868,   867,
   -1430,   871, 14931, -1430, 14931, -1430,   872, -1430,   945,   874,
    1068, -1430,    31,  1049, -1430,   880, -1430, -1430,   884,   888,
     109, -1430, -1430, 16250,   881,   885, -1430,  1872, -1430, -1430,
   -1430, -1430, -1430, 14594, -1430, 14594, -1430, 16250, 15753, -1430,
   14836, 15063, -1430, -1430, -1430, 14836, -1430, 14836, -1430, 14836,
   -1430, -1430, 14836, -1430, 14836, -1430, 13416, 14836, 13805,   878,
    7391,   992, -1430, -1430,   255, 14594,  5540, -1430,  5095,   936,
    2601, -1430, -1430, -1430,   651, 14368,    74,   334,   110, -1430,
   -1430, -1430,   941,  3452,  3663, 16164, 16164, -1430,    45,  1081,
    1017, 14836, -1430, 16250, 11117,   987,   908,   452,   908,   898,
   16164,   899, -1430,   886,   902,  1039, -1430,    32, -1430, -1430,
     972, -1430, 14931, -1430, 15063, -1430, -1430,  9254, -1430, -1430,
   -1430, -1430, 10082, -1430, -1430, -1430,  9254, -1430,   903, 14836,
   16250,   981, 16250, 16250, 15795, 16250, 15852, 13416,  5591, -1430,
   -1430, 14594,  5540,  5540,  1093,    46, -1430, -1430, -1430, -1430,
      76,   905,    79, -1430, 14177, -1430, -1430,    80, -1430, -1430,
    2388, -1430,   917, -1430,  1041,   378, -1430, 14594, -1430,   651,
   -1430,  3226, -1430, -1430, -1430, -1430,  1108,  1045, 14836, -1430,
   16250,   925,   928,   924,   372, -1430,   987,   908, -1430, -1430,
   -1430, -1430,  1094,   930, 14931, -1430,  1003,  9254, 10289, 10082,
   -1430, -1430, -1430,  9254, -1430, 16250, 14836, 14836, 14836, 13805,
    7598, -1430,   931,   933, 14836,  5540, -1430, -1430,   979,  5095,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430,   526, -1430,   936, -1430, -1430, -1430, -1430, -1430,    99,
     484, -1430,  1122,    84, 15907,  1041,  1123,   378, -1430, -1430,
     938,  1124, 14836, -1430, 16250, -1430,    96, -1430, -1430, -1430,
   -1430,   939,   372, 14275, -1430,   908, -1430, 14931, -1430, -1430,
   -1430, -1430,  7805, 16250, 16250, 16250,  5432, -1430, -1430, -1430,
   16250, -1430,  1125,    55, -1430, -1430, 14836, 14177, 14177,  1086,
   -1430,  2388,  2388,   543, -1430, -1430, -1430, 14836,  1066, -1430,
     947,    86, 14836, 15907, -1430, 14836, 16250,  1069, -1430,  1141,
    8012,  8219, -1430, -1430, -1430,   372, -1430,  8426,   957,  1082,
    1054, -1430,  1067,  1016, -1430, -1430,  1070,   979, -1430, 16250,
   -1430, -1430,  1005, -1430,  1134, -1430, -1430, -1430, -1430, 16250,
    1154, -1430, -1430, 16250,   969, 16250, -1430,   101,   970, -1430,
   -1430,  8633, -1430,   968, -1430, -1430,   973,  1009, 15907,   334,
   -1430, -1430, 14836,    42, -1430,  1099, -1430, -1430, -1430, -1430,
    5540,   710, -1430,  1018, 15907,   260, 16250,   983,  1170,   515,
      42, -1430,  1101, -1430,  5540,   984, -1430,   908,    83, -1430,
   -1430, -1430, -1430, 14594, -1430,   988,   989,    87, -1430,   376,
     515,   326,   908,   990, -1430, -1430, -1430, -1430, 14594,    50,
    1168,  1135,   376, -1430,  8840,   329,  1187,  1136, 14836, -1430,
   -1430,  9047, -1430,    56,  1189,  1137, 14836, -1430, 16250, -1430,
    1203,  1139, 14836, -1430, 16250, 14836, -1430, 16250, 16250
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1430, -1430, -1430,  -492, -1430, -1430, -1430,    -4, -1430, -1430,
   -1430,   712,   454,   447,   -21,  1161,  3834, -1430,  2476, -1430,
    -383, -1430,    39, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430,  -447, -1430, -1430,  -176,     4,     6,
   -1430, -1430, -1430, -1430, -1430, -1430,     9, -1430, -1430, -1430,
   -1430,    11, -1430, -1430,   835,   849,   841,  -125,   353,  -809,
     358,   423,  -450,   134,  -842, -1430,  -193, -1430, -1430, -1430,
   -1430,  -660,   -18, -1430, -1430, -1430, -1430,  -442, -1430,  -544,
   -1430,  -394, -1430, -1430,   727, -1430,  -180, -1430, -1430,  -957,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,
    -208, -1430, -1430, -1430, -1430, -1430,  -289, -1430,   -55,  -933,
   -1430, -1429,  -466, -1430,  -161,    -1,  -133,  -453, -1430,  -294,
   -1430,   -60,   -23,  1224,  -662,  -364, -1430, -1430,   -36, -1430,
   -1430,  3877,   -52,  -149, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430,  -540,  -786, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430, -1430,   876, -1430,
   -1430,   280, -1430,   785, -1430, -1430, -1430, -1430, -1430, -1430,
   -1430,   285, -1430,   796, -1430, -1430,   518, -1430,   261, -1430,
   -1430, -1430, -1430, -1430, -1430, -1430, -1430,  -887, -1430,  1958,
    1242,  -347, -1430, -1430,   221,  3638,  4171, -1430, -1430,   343,
    -162,  -592, -1430, -1430,   407,  -652,   215, -1430, -1430, -1430,
   -1430, -1430,   394, -1430, -1430, -1430,    12,  -822,  -171,  -409,
    -403,  -131, -1430, -1430,    14, -1430, -1430, -1430, -1430,    22,
    -146, -1430,   112, -1430, -1430, -1430,  -395,  1001, -1430, -1430,
   -1430, -1430, -1430,   985, -1430, -1430, -1430,   346, -1430, -1430,
   -1430,   520,   391, -1430, -1430,  1012,  -293,  -954, -1430,   -47,
     -84,  -201,   -19,   578, -1430, -1014, -1430,   294,   373, -1430,
   -1430, -1430,  -213, -1012
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -892
static const yytype_int16 yytable[] =
{
     121,   392,   348,   135,   310,   260,   850,   421,   129,   236,
     130,   316,   317,   132,  1039,   133,   162,   265,   164,   560,
     241,   674,  1205,   441,   245,   677,   419,  1208,   834,   896,
     651,   833,  1029,   631,   414,   277,   696,   391,   466,   467,
     215,   216,   697,   128,   471,   248,  1053,   913,   258,   695,
     344,   348,   229,   321,   807,  1445,   927,   306,   438,   445,
     307,   323,  1064,  1258,  1627,   292,   699,   928,  1193,  1192,
     338,   715,  1101,  1009,    13,   813,   453,    13,   510,   412,
     515,   520,   308,  1397,   292,  -287,  -777,    13,  1449,  1533,
     512,   292,   292,  1598,  1040,  1598,  1445,  1312,   713,   948,
     446,  1081,   782,   782,  1110,  1111,   796,  1209,  1591,   796,
    -774,   409,   410,   796,   764,   286,   278,   770,   796,   796,
     409,   410,   594,   774,  1406,  1199,   536,   324,    13,  1716,
     292,  1607,  1592,  1613,   335,  1730,  1665,     3,  1041,   347,
     409,   410,   282,   409,   410,   380,   334,   297,   412,   283,
     775,   431,   542,  -447,  1194,  -891,  -891,   381,   409,   410,
     299,   393,  1082,   439,  1127,  -891,   559,  1195,   498,    13,
     188,   604,   232,   334,  1200,  1608,   335,   335,   814,   334,
    1666,  -483,  -775,  -781,  -623,   579,  1652,   209,   422,  -776,
    1210,  -808,   428,   815,   416,   235,  -778,   806,  -811,  -809,
     311,  -780,  1325,  1196,  -779,  -810,   334,  1407,  -891,  1113,
    -891,  -891,  1717,   269,   300,   301,   413,   693,  1731,   242,
    -783,  1042,   333,  -777,   243,   432,   499,  -228,   929,   334,
     525,   434,   121,  -228,  1010,  1259,   121,   440,   526,  1294,
     460,   643,   279,  -212,   716,   416,   678,  -774,  1446,  1447,
     164,   504,   508,   509,   164,   718,  1228,   602,   473,  1628,
     348,  1327,  1251,   339,  1108,   643,  1112,   684,  1333,   454,
    1335,   511,  1334,   516,   521,   450,  1398,   590,  -287,   455,
      13,  1450,  1534,   545,  -706,   413,  1599,   643,  1642,  1704,
     714,   600,   209,   514,   783,   784,   643,  1238,   797,   643,
    1593,   883,   522,   522,   527,  1185,   310,   344,  1050,   532,
    1351,  1400,  1414,   418,  1416,   541,  1022,  -784,   910,  -775,
    -781,   121,  -706,   912,   135,  -706,  -776,  -624,  -808,   129,
     591,   417,   575,  -778,   258,  -811,  -809,   292,  -780,   164,
    1295,  -779,  -810,   998,  1318,  1296,   244,    58,    59,    60,
     176,   177,   345,  1297,   297,   830,   831,   318,   652,   542,
     297,  1709,   535,   838,  1723,   298,   789,   790,  1110,  1111,
     685,   288,    58,    59,    60,   176,   177,   345,   289,  1422,
    1372,  1373,   417,   641,   292,   645,   292,   292,  1319,  1001,
    1298,  1299,   297,  1300,   819,   290,   648,   542,   297,   334,
    1015,   824,   229,  1562,   828,  1710,   840,   668,  1724,   642,
     291,   297,   284,   340,   297,   702,   327,   346,   116,   330,
     285,   300,   301,   409,   410,   392,   299,   300,   301,   687,
     295,  1043,  1241,   671,   532,   311,  1294,  1044,  1248,  1301,
     867,   668,   346,   296,   432,   334,    58,    59,    60,   176,
     177,   345,   121,   907,   908,   642,  1294,   312,   543,   300,
     301,   391,  1686,   313,   694,   300,   301,   671,   334,   877,
     164,  1320,   873,  1261,  1061,   319,   865,    13,   300,   301,
     767,   300,   301,   589,   771,  1361,   867,   297,  1711,   277,
     314,  1725,   315,    51,  1090,   707,   635,    13,  1207,   337,
    1091,    58,    59,    60,   176,   177,   345,  1217,   777,   297,
    1219,   594,   594,  1594,   542,   840,   346,   326,   547,   548,
     857,   903,  1092,  1093,   672,   560,   349,  1067,   675,   350,
    1595,   801,   803,  1596,    36,   351,   209,  1295,  1107,   441,
    1586,   352,  1296,   353,    58,    59,    60,   176,   177,   345,
    1297,  1615,   354,   537,   300,   301,  1587,  1295,  1426,  -481,
     933,   904,  1296,   383,    58,    59,    60,   176,   177,   345,
    1297,   346,  1636,  1588,  1558,  1559,   300,   301,  1705,  1706,
    1339,   385,  1340,   384,   575,   393,   386,  1298,  1299,  1637,
    1300,   387,  1638,  1109,  1110,  1111,  1023,  1255,  1110,  1111,
     292,   976,   977,   978,   979,   980,   981,  1298,  1299,   846,
    1300,   415,    83,    84,   346,    85,   181,    87,  -782,  1035,
     982,    58,    59,    60,    61,    62,   345,  1690,  1691,  1692,
    1045,  -482,    68,   388,   346,  -623,  1311,   329,   331,   332,
     420,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,  1187,   304,  1415,   560,  1634,  1635,
     667,   425,   116,   427,   875,  1317,  1223,   381,   389,   335,
     390,   433,   559,  1699,   436,  1231,  1329,   894,   437,   899,
    -622,  1701,   911,   377,   378,   379,   442,   380,  1712,   538,
    1425,   346,   443,   544,   643,   121,  1715,   451,   135,   381,
    1630,  1631,   900,   129,   901,   936,   939,   464,  1250,   920,
     121,   468,   469,   164,  -886,  1114,   472,  1115,   538,   474,
     544,   538,   544,   544,   475,   918,   477,   517,   164,   979,
     980,   981,   518,   519,   528,  1085,   529,   568,    58,    59,
      60,    61,    62,   345,   569,   982,   577,   578,   504,    68,
     388,   -65,   508,   922,   588,    51,   601,   643,   681,   683,
     121,   686,   692,   135,   705,   453,  1000,   712,   129,   724,
    1003,  1004,  1566,   709,   478,   479,   480,  1120,   164,   725,
     750,   481,   482,   751,  1124,   483,   484,   390,  1011,  1066,
     560,  1308,   753,  1423,  1290,   754,  1007,   762,   765,   768,
     769,   772,   773,   121,   559,   781,   135,   785,   346,   532,
    1017,   129,   786,   130,   788,   793,   132,  1030,   133,   162,
     795,   164,   798,   804,  1206,   809,   838,   810,  1331,   812,
     374,   375,   376,   377,   378,   379,   668,   380,   817,   818,
     820,  1679,   821,  1213,   822,   823,   128,   826,   575,   381,
    1226,   827,   832,   829,   835,   836,  -484,   843,  1679,   848,
     849,  1347,   671,   845,   575,   854,  1700,   851,   864,   292,
     860,   861,   863,   868,   878,   880,   905,  1356,   881,   856,
    1074,  1074,   894,   914,  1095,   924,   926,   930,   932,   931,
    1294,   949,   934,   955,   984,   947,   229,   950,   951,   668,
     985,   986,   956,   952,   121,  1045,   121,   953,   121,   135,
     990,   135,   993,   996,   129,  1616,   129,  1235,  1006,  1008,
    1018,  1014,   164,  1025,   164,   671,   164,  1116,   918,  1102,
    1411,    13,  1019,  1027,  1034,  1036,  1051,   559,  1037,  1060,
    1063,  1070,  1069,  1084,  1080,  1126,  1083,  1096,  1086,  1103,
    1098,  1139,  1104,  1100,  1106,  1118,  1119,   982,  1123,  1122,
     537,  1176,  1182,  1427,  1183,  1186,  1272,  1211,  1202,  1292,
    1276,  1204,  1433,  1280,  1212,  1216,  1218,  1220,   560,  1222,
    1285,  1224,  1188,  1232,  1225,  1227,  1440,  1234,  1237,  1244,
    1233,  1295,  1262,  1245,  1178,  1252,  1296,  1179,    58,    59,
      60,   176,   177,   345,  1297,  -892,  -892,  -892,  -892,   974,
     975,   976,   977,   978,   979,   980,   981,  1648,   121,  1247,
    1256,   135,  1263,  1265,  1271,  1266,   129,  1269,   130,  1270,
     982,   132,  1273,   133,   162,  1274,   164,  1275,  1278,  1279,
    1284,  1298,  1299,  1294,  1300,  1281,  1321,  1572,  1289,  1283,
    1357,   560,  1358,    33,    34,    35,  1309,  1324,  1310,  1322,
    1214,   128,  1326,   575,   731,  1328,   575,  1337,   346,  1243,
    1332,  1336,  1338,  1341,  1342,  1343,   894,  1344,  1345,  1346,
     894,  1369,  1348,  1353,    13,  1349,  1689,  1354,  1371,  1350,
    1419,   121,  1396,  1386,  1402,  1408,  1409,  1412,  1294,  1417,
    1418,  1424,  1242,   121,  1434,  1420,   135,  1444,  1448,   164,
    1436,   129,    72,    73,    74,    75,    76,   532,   918,  1541,
    1542,   164,  1552,   733,  1553,   559,  1555,  1557,  1556,    79,
      80,  1565,  1567,  1578,  1246,  1579,  1597,  1602,  1605,    13,
    1604,  1632,  1612,    89,  1295,  1640,  1399,  1641,  1646,  1296,
    1647,    58,    59,    60,   176,   177,   345,  1297,  1441,  1654,
      97,  1655,  1656,  -283,  1658,  1661,  1659,  1592,  1662,  1664,
    1669,  1667,   348,  1670,  1610,  1671,  1611,  1291,  1681,  1688,
    1696,  1684,  1718,  1305,  1687,  1617,  1698,   212,   212,  1702,
    1703,   224,  1305,  1713,  1298,  1299,  1307,  1300,   559,  1295,
     532,  1726,    36,  1732,  1296,  1307,    58,    59,    60,   176,
     177,   345,  1297,   224,  1719,  1727,  1733,  1735,  1736,  1544,
    1002,   346,   776,   999,  1683,  1065,   647,  1062,   894,  1651,
     894,   646,   575,  1024,  1697,  1249,  1571,   644,  1695,  1355,
    1563,  1585,   779,  1421,  1590,  1393,  1720,  1708,  1374,  1298,
    1299,  1601,  1300,  -892,  -892,  -892,  -892,   372,   373,   374,
     375,   376,   377,   378,   379,   238,   380,  1561,   214,   214,
     654,   760,   226,  1175,   180,  1173,   346,    81,   381,   992,
      83,    84,   761,    85,   181,    87,   121,  1197,  1230,   135,
    1125,   258,  1076,  1087,   129,  1236,  1391,   534,  1564,  1136,
     567,   524,   937,  1181,   164,  1117,   393,     0,  1395,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,  1714,     0,     0,  1305,     0,     0,  1621,
    1721,     0,  1305,     0,  1305,     0,     0,     0,   894,  1307,
       0,     0,     0,   121,     0,  1307,   135,  1307,   121,   575,
    1546,   129,   121,     0,     0,   135,     0,     0,     0,     0,
     129,   164,     0,     0,     0,     0,   164,     0,     0,     0,
     164,  1603,   212,     0,     0,     0,     0,     0,   212,     0,
    1530,     0,     0,     0,   212,     0,  1537,     0,     0,     0,
       0,  1431,     0,   258,     0,     0,     0,   258,     0,     0,
       0,     0,     0,  1547,     0,     0,     0,     0,     0,     0,
       0,     0,   224,   224,     0,     0,     0,     0,   224,     0,
     894,  1305,     0,   121,   121,   121,   135,     0,     0,   121,
       0,   129,   135,     0,  1307,     0,   121,   129,     0,   135,
     212,   164,   164,   164,   129,     0,     0,   164,     0,   212,
     212,     0,     0,   214,   164,     0,   212,     0,     0,   214,
    1600,     0,   212,     0,     0,   214,     0,  1569,  1431,     0,
       0,     0,     0,   224,     0,     0,     0,     0,     0,     0,
     838,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   838,   224,     0,  1674,   224,
       0,     0,     0,     0,     0,     0,   423,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,  1644,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
     214,   214,     0,     0,     0,     0,     0,   214,     0,     0,
     292,   224,   348,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   558,   407,   408,     0,     0,   258,
       0,     0,     0,   894,     0,     0,     0,     0,   121,     0,
       0,   135,     0,     0,     0,     0,   129,     0,  1622,     0,
       0,   212,     0,  1530,  1530,     0,   164,  1537,  1537,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,   292,
       0,     0,     0,     0,     0,     0,   121,   121,     0,   135,
     135,     0,     0,   121,   129,   129,   135,     0,     0,   409,
     410,   129,   226,     0,   164,   164,     0,     0,     0,   224,
     224,   164,     0,   740,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   121,     0,     0,
     135,     0,     0,     0,  1673,   129,     0,     0,  1675,     0,
       0,     0,   214,     0,     0,   164,     0,     0,     0,     0,
    1685,     0,   214,     0,     0,   575,     0,     0,     0,     0,
     740,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   575,   579,     0,     0,     0,     0,     0,     0,
     575,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     121,     0,     0,   135,     0,     0,     0,   121,   129,     0,
     135,     0,     0,     0,     0,   129,     0,     0,   164,   224,
     224,     0,     0,     0,     0,   164,     0,   224,   355,   356,
     357,   423,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,     0,     0,   212,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,     0,     0,     0,     0,     0,     0,     0,
     407,   408,     0,   381,     0,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   212,     0,     0,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   214,   380,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
     381,   212,     0,     0,   409,   410,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   212,   740,     0,     0,     0,     0,     0,     0,
       0,     0,   355,   356,   357,   224,   224,   740,   740,   740,
     740,   740,   214,     0,     0,     0,     0,     0,     0,     0,
     740,   358,  1258,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,   224,     0,   787,     0,
     214,     0,   214,     0,     0,  1190,     0,   381,    36,   251,
       0,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,   224,     0,   212,   212,     0,     0,
       0,     0,     0,     0,     0,   252,     0,     0,     0,     0,
     224,   224,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,   213,   213,     0,    36,   225,   805,
       0,     0,     0,   224,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,   740,   304,   470,   224,    83,    84,     0,    85,
     181,    87,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,     0,   214,   214,     0,
       0,   253,   254,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   180,
     558,     0,    81,   255,   305,    83,    84,     0,    85,   181,
      87,    36,     0,   209,  1259,   212,   212,     0,     0,     0,
       0,     0,     0,   256,     0,     0,     0,     0,     0,   224,
       0,   224,     0,   224,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   740,     0,
     257,   224,   740,   740,   740,     0,   226,   740,   740,   740,
     740,   740,   740,   740,   740,   740,   740,   740,   740,   740,
     740,   740,   740,   740,   740,   740,   740,   740,   740,   740,
     740,   740,   740,   740,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   181,    87,     0,   214,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   740,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   558,     0,     0,     0,     0,   640,   224,   116,
     224,     0,     0,     0,     0,     0,     0,   212,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,   224,     0,   213,   213,     0,   212,
       0,     0,     0,   213,     0,     0,     0,   407,   408,   213,
       0,     0,     0,     0,   212,   212,     0,   740,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,   740,     0,   740,     0,     0,     0,   214,     0,
     423,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,     0,     0,     0,     0,   740,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   409,   410,     0,     0,   558,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,   225,   407,
     408,     0,     0,   224,   224,   214,   214,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   355,   356,
     357,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   740,     0,     0,     0,     0,   358,   213,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,   409,   410,     0,     0,     0,     0,     0,
       0,     0,     0,   381,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,   224,     0,   214,     0,
     746,   740,   224,     0,     0,     0,   740,     0,   740,     0,
     740,     0,     0,   740,     0,   740,     0,     0,   740,     0,
       0,     0,     0,     0,     0,     0,   224,   224,     0,   224,
       0,     0,   957,   958,   959,    36,   224,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   746,     0,     0,
       0,   960,   740,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,     0,   224,     0,     0,     0,     0,
       0,     0,     0,   558,     0,     0,     0,   982,     0,     0,
     740,     0,     0,     0,     0,     0,     0,     0,   259,     0,
       0,     0,   224,   224,   224,     0,     0,     0,     0,     0,
       0,  1535,     0,    83,    84,  1536,    85,   181,    87,     0,
       0,     0,   842,   213,     0,     0,     0,     0,   224,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,   740,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   558,     0,  1390,     0,
     355,   356,   357,    36,     0,   209,     0,   740,   740,   740,
       0,     0,     0,     0,     0,   740,   224,     0,   213,   358,
     224,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,     0,   380,     0,     0,     0,     0,     0,   639,
       0,     0,     0,     0,     0,   381,   213,     0,   213,     0,
       0,     0,     0,  1137,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
     746,    83,    84,     0,    85,   181,    87,     0,    36,     0,
       0,     0,     0,     0,   746,   746,   746,   746,   746,     0,
       0,     0,     0,     0,     0,     0,     0,   746,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   740,     0,     0,     0,     0,     0,   640,
       0,   116,  1389,   995,   224,     0,     0,   259,   259,     0,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
     213,     0,     0,   224,     0,     0,     0,   740,     0,     0,
       0,  1013,     0,   213,   213,     0,    83,    84,   740,    85,
     181,    87,     0,   740,     0,     0,   740,     0,  1013,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,   879,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,  1390,     0,     0,     0,     0,     0,     0,     0,   746,
       0,   259,  1052,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,   740,     0,     0,     0,     0,     0,     0,
       0,   224,   225,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,   213,   213,     0,     0,     0,     0,     0,   740,
       0,     0,     0,     0,     0,     0,     0,   740,     0,     0,
       0,     0,     0,   740,     0,     0,   740,     0,     0,     0,
       0,     0,     0,     0,     0,   746,     0,     0,   213,   746,
     746,   746,     0,     0,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   746,   746,   746,   746,   746,   746,
     746,     0,     0,     0,   259,   720,     0,     0,   742,     0,
       0,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   746,     0,     0,     0,     0,     0,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,   742,   355,   356,   357,     0,
       0,     0,     0,     0,   213,     0,   381,     0,     0,     0,
       0,     0,     0,     0,     0,   358,     0,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
       0,   213,     0,     0,   259,   259,   213,     0,     0,     0,
       0,   381,   259,     0,     0,     0,     0,     0,     0,     0,
       0,   213,   213,     0,   746,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   746,
       0,   746,     0,     0,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   358,   746,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,  1293,     0,     0,   213,   882,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   746,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   742,     0,
    1005,   601,     0,     0,     0,     0,     0,     0,     0,     0,
     259,   259,   742,   742,   742,   742,   742,     0,     0,     0,
       0,     0,     0,     0,     0,   742,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   746,   213,
       0,     0,     0,   746,     0,   746,     0,   746,     0,     0,
     746,     0,   746,     0,     0,   746,     0,     0,     0,     0,
       0,     0,     0,     0,  1376,   251,  1385,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,     0,  1059,   746,
       0,   252,     0,     0,     0,   259,     0,     0,     0,     0,
     982,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,    36,     0,     0,     0,     0,   259,     0,
       0,     0,     0,   355,   356,   357,     0,   746,     0,   259,
       0,     0,     0,     0,     0,     0,     0,   742,     0,     0,
    1442,  1443,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,   253,   254,     0,
       0,     0,     0,     0,     0,     0,   746,     0,   381,     0,
       0,     0,     0,     0,     0,   180,     0,     0,    81,   255,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,     0,     0,   746,   746,   746,     0,     0,   256,
       0,     0,   746,  1581,   259,     0,   259,  1385,   720,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   742,     0,     0,   257,   742,   742,   742,
    1548,     0,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,     0,
       0,     0,   355,   356,   357,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   358,   742,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,     0,  1068,     0,     0,
     746,     0,     0,   259,     0,   259,     0,   381,     0,  1071,
    1072,  1073,    36,     0,     0,     0,    29,    30,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   209,   259,
       0,     0,     0,     0,   746,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   746,     0,     0,     0,     0,
     746,     0,     0,   746,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,   742,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,   742,     0,   742,
      83,    84,     0,    85,   181,    87,     0,     0,   180,     0,
       0,    81,    82,     0,    83,    84,     0,    85,   181,    87,
     746,     0,   742,     0,     0,     0,    90,     0,  1682,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,  1376,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,  1404,     0,   259,   430,
       0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,   355,   356,   357,   746,     0,     0,     0,
       0,     0,     0,     0,   746,     0,     0,   742,     0,     0,
     746,     0,   358,   746,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   381,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   874,   259,
       0,   259,     0,     0,     0,     0,   742,    36,     0,   209,
       0,   742,     0,   742,     0,   742,     0,     0,   742,     0,
     742,     0,     0,   742,     0,     0,     0,     0,     0,     0,
       0,   259,     0,     0,     0,     0,     0,   355,   356,   357,
       0,   259,     0,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,   358,   742,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   180,
     380,     0,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,   381,     0,     0,   742,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1405,     0,     0,
     211,     0,     0,   259,     0,   116,     0,   259,     0,     0,
       0,     0,   185,   187,   742,   189,   190,   191,   193,   194,
       0,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,     0,     0,   218,   221,     0,     0,     0,
       0,     0,   742,   742,   742,   355,   356,   357,   239,     0,
     742,     0,     0,     0,     0,   247,     0,   250,     0,     0,
     266,     0,   271,     0,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   382,
     381,     0,   355,   356,   357,   915,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     322,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,     0,    36,     0,   209,
       0,     0,     0,     0,     0,     0,     0,   381,   742,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   259,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,     0,   210,  1623,     0,
       0,     0,   742,     0,     0,   424,     0,     0,     0,   982,
     916,     0,     0,   742,     0,     0,     0,     0,   742,     0,
       0,   742,     0,     0,     0,     0,     0,     0,     0,   180,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,     0,   463,     0,   448,
     747,     0,   448,     0,     0,     0,     0,     0,     0,   239,
     459,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   742,     0,
     211,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   747,     0,     0,
       0,     0,     0,     0,   465,     0,     0,   322,     0,   259,
       0,     0,     0,   218,     0,     0,     0,   540,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
     561,   564,   564,     0,   742,     0,     0,     0,     0,     0,
       0,     0,   742,   587,     0,     0,     0,     0,   742,     0,
       0,   742,     0,     0,   599,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   209,     0,
       0,     0,   605,   606,   607,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   620,   621,   622,   623,
     624,   625,   626,   627,   628,   629,   630,     0,   632,     0,
     633,   633,     0,   636,     0,     0,   210,     0,     0,     0,
       0,   653,   655,   656,   657,   658,   659,   660,   661,   662,
     663,   664,   665,   666,     0,     0,     0,     0,     0,   633,
     673,     0,   599,   633,   676,     0,     0,     0,   180,     0,
     653,    81,    82,   680,    83,    84,   741,    85,   181,    87,
       0,     0,   689,     0,   691,     0,     0,     0,     0,     0,
     599,     0,     0,     0,     0,     0,     0,     0,   703,     0,
     704,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   211,
     747,     0,   513,   741,   116,     0,   752,     0,     0,   755,
     758,   759,     0,     0,   747,   747,   747,   747,   747,     0,
       0,     0,     0,   355,   356,   357,     0,   747,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     778,     0,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   381,   355,
     356,   357,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,     0,   380,   847,     0,   355,   356,   357,     0,     0,
       0,     0,     0,     0,   381,   858,     0,     0,     0,   747,
       0,     0,     0,     0,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   866,   380,     0,
       0,     0,     0,     0,     0,     0,   193,     0,     0,     0,
     381,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   876,     0,   741,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     741,   741,   741,   741,   741,   476,     0,     0,     0,     0,
       0,     0,     0,   741,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   747,   239,     0,     0,   747,
     747,   747,     0,     0,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   500,     0,     0,     0,     0,     0,   983,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   209,     0,   748,   747,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,   679,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   381,     0,   210,
       0,  1020,     0,     0,     0,   741,     0,     0,     0,     0,
     780,     0,   531,     0,  1028,     0,     0,     0,     0,     0,
    1031,     0,  1032,     0,  1033,     0,     0,     0,     0,     0,
       0,   180,     0,     0,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,  1048,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1056,     0,     0,  1057,
       0,  1058,     0,     0,   747,   599,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   747,
       0,   747,   211,    36,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1089,
       0,   741,     0,     0,   747,   741,   741,   741,     0,     0,
     741,   741,   741,   741,   741,   741,   741,   741,   741,   741,
     741,   741,   741,   741,   741,   741,   741,   741,   741,   741,
     741,   741,   741,   741,   741,   741,   741,   960,     0,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     741,    83,    84,     0,    85,   181,    87,     0,     0,   747,
       0,     0,     0,   982,     0,     0,     0,     0,     0,     0,
       0,  1170,  1171,  1172,     0,     0,     0,   755,  1174,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   919,     0,     0,   856,     0,     0,     0,
       0,    36,     0,   209,     0,  1189,     0,   940,   941,   942,
     943,     0,     0,     0,     0,     0,     0,     0,   747,     0,
     954,     0,     0,   747,     0,   747,     0,   747,     0,     0,
     747,     0,   747,     0,  1215,   747,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,   599,     0,     0,
     741,     0,     0,     0,  1016,     0,   599,  1189,     0,     0,
       0,     0,     0,     0,     0,   741,     0,   741,     0,   747,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,   239,     0,     0,
     741,     0,     0,     0,     0,     0,     0,  1257,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   747,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   211,     0,     0,     0,     0,   116,
       0,     0,  1049,     0,     0,     0,     0,     0,   355,   356,
     357,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   741,   747,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,  1313,   747,   747,   747,  1314,     0,  1315,
    1316,     0,   747,   381,     0,     0,  1584,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1330,   599,
       0,     0,     0,     0,   741,     0,     0,     0,     0,   741,
       0,   741,     0,   741,     0,     0,   741,     0,   741,     0,
       0,   741,  1130,  1133,  1133,     0,     0,  1140,  1143,  1144,
    1145,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,     0,   741,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1368,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,  1180,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     747,     0,     0,   741,     0,     0,     0,     0,     0,     0,
       0,   599,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1377,     0,     0,     0,     0,     0,   698,
       0,     0,     0,     0,   747,     0,  1378,  1379,     0,     0,
       0,     0,     0,     0,     0,   747,     0,     0,     0,     0,
     747,     0,   741,   747,   180,     0,     0,    81,  1380,     0,
      83,    84,     0,    85,  1381,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1660,     0,     0,     0,     0,
     741,   741,   741,     0,     0,     0,     0,  1253,   741,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,  1267,     0,  1268,     0,     0,     0,     0,     0,
     747,     0,   355,   356,   357,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1576,  1286,     0,     0,
       0,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   747,   381,     0,     0,
       0,     0,     0,     0,   747,     0,   355,   356,   357,     0,
     747,     0,     0,   747,     0,     0,     0,     0,     0,     0,
       0,     0,  1323,     0,     0,   358,   741,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,    36,   380,
     799,   800,     0,     0,     0,     0,     0,     0,     0,     0,
     741,   381,     0,     0,    36,     0,     0,     0,     0,     0,
       0,   741,     0,     0,     0,     0,   741,     0,     0,   741,
       0,  1360,   355,   356,   357,     0,  1362,     0,  1363,     0,
    1364,   272,   273,  1365,     0,  1366,     0,     0,  1367,     0,
       0,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,    83,    84,   991,    85,
     181,    87,  1410,     0,     0,     0,   741,   381,     0,   274,
       0,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
    1435,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,   987,   988,
       0,     0,   741,     0,     0,     0,     0,     0,     0,     0,
     741,   959,     0,     0,     0,     0,   741,     0,     0,   741,
       0,     0,     0,     0,     0,     0,     0,     0,   960,  1554,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   355,   356,   357,     0,     0,     0,  1573,  1574,  1575,
       0,     0,     0,     0,   982,  1580,     0,    36,     0,     0,
     358,  1619,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,     0,     0,   355,   356,   357,
       0,     0,     0,     0,     0,     0,   381,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   358,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   180,
     380,     0,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,   381,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1606,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,  1629,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,  1639,     0,
       0,     0,     0,  1643,     0,     0,  1645,     0,     0,  1439,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,  1288,    44,     0,    45,     0,    46,
       0,     0,    47,  1676,     0,     0,    48,    49,    50,    51,
      52,    53,    54,     0,    55,    56,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,    67,    68,    69,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,    78,    79,    80,    81,    82,  1728,
      83,    84,     0,    85,    86,    87,    88,  1734,     0,    89,
       0,     0,    90,  1737,     0,     0,  1738,     0,    91,    92,
      93,    94,    95,     0,    96,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1021,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,    78,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,    95,
       0,    96,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1191,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,    52,    53,    54,     0,    55,    56,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,    67,    68,    69,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,    78,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,    95,     0,    96,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
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
      69,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
     580,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,     0,    68,    69,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,   994,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
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
      65,    66,     0,    68,    69,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1097,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,  1099,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,  1254,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    68,    69,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
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
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1370,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1577,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,  1618,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1649,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1650,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,  1653,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,     0,    68,    69,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1668,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
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
      65,    66,     0,    68,    69,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1722,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1729,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    68,    69,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   449,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   176,   177,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   706,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,   176,   177,    63,     0,    64,    65,    66,
       0,     0,     0,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   921,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     176,   177,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1430,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   176,   177,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,  1568,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,   176,   177,    63,     0,    64,    65,
      66,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
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
      60,   176,   177,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     649,    12,     0,     0,     0,     0,     0,     0,   650,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,   263,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,   263,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   264,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   358,
      10,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   595,   380,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   381,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   176,   177,   178,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   179,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,   596,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   176,
     177,   178,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,   356,   357,   701,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,   358,    10,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,  1046,   380,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,   381,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   176,   177,   178,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
       0,   179,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,  1047,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   649,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   178,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   179,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,    94,     0,    10,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   192,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,   958,   959,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   960,    10,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   982,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,    94,     0,    10,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,    94,     0,    10,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   246,     0,     0,
     116,   117,     0,   118,   119,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,    94,     0,    10,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   249,     0,     0,   116,   117,     0,   118,   119,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,   447,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   608,   380,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   357,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   358,    10,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   650,   380,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   381,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   176,   177,   178,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   179,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   688,   380,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   381,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   690,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   982,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,  1088,   380,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   381,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,    94,     0,    10,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,    94,     0,    10,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,   539,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
    1451,  1452,  1453,  1454,  1455,     0,     0,  1456,  1457,  1458,
    1459,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1460,  1461,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,     0,     0,     0,     0,     0,
    1462,     0,     0,     0,     0,     0,     0,     0,   982,     0,
       0,     0,     0,     0,  1463,  1464,  1465,  1466,  1467,  1468,
    1469,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,  1470,  1471,  1472,  1473,  1474,  1475,  1476,
    1477,  1478,  1479,  1480,  1481,  1482,  1483,  1484,  1485,  1486,
    1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,  1496,
    1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,  1506,
    1507,  1508,  1509,  1510,   251,     0,     0,  1511,  1512,     0,
    1513,  1514,  1515,  1516,  1517,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1518,  1519,  1520,     0,
     252,     0,    83,    84,     0,    85,   181,    87,  1521,     0,
    1522,  1523,     0,  1524,     0,     0,     0,     0,     0,     0,
    1525,     0,    36,     0,  1526,     0,  1527,     0,  1528,  1529,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,     0,   380,     0,     0,     0,   251,     0,     0,
       0,     0,     0,     0,     0,   381,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   252,   180,     0,     0,    81,   255,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,   256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,  -325,     0,     0,   257,     0,   251,     0,  1614,
      58,    59,    60,   176,   177,   345,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
     254,     0,     0,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,     0,     0,
      81,   255,     0,    83,    84,    36,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   256,     0,   251,     0,     0,     0,     0,     0,     0,
     346,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   257,   252,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
     254,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,   180,     0,     0,
      81,   255,     0,    83,    84,     0,    85,   181,    87,     0,
     935,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   256,     0,   251,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   253,   254,     0,   257,   252,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   180,     0,     0,    81,   255,     0,    83,
      84,    36,    85,   181,    87,     0,  1264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   256,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   257,  1146,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   253,   254,     0,     0,     0,
       0,   726,   727,     0,     0,     0,     0,   728,     0,   729,
       0,     0,     0,   180,     0,     0,    81,   255,     0,    83,
      84,   730,    85,   181,    87,     0,     0,     0,     0,    33,
      34,    35,    36,     0,     0,     0,   342,   256,    83,    84,
     731,    85,   181,    87,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   257,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,     0,     0,     0,   732,     0,    72,    73,
      74,    75,    76,     0,     0,    36,     0,   209,     0,   733,
       0,     0,     0,     0,   180,    79,    80,    81,   734,     0,
      83,    84,     0,    85,   181,    87,     0,    36,     0,    89,
       0,     0,     0,     0,     0,   719,     0,     0,   735,   736,
     737,   738,     0,     0,     0,   210,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   726,   727,     0,   739,     0,     0,   728,     0,
     729,     0,     0,     0,     0,     0,     0,   180,     0,     0,
      81,    82,   730,    83,    84,     0,    85,   181,    87,     0,
      33,    34,    35,    36,     0,     0,     0,     0,     0,   180,
       0,   731,    81,     0,     0,    83,    84,     0,    85,   181,
      87,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   211,     0,
       0,     0,     0,   116,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   732,     0,    72,
      73,    74,    75,    76,     0,     0,     0,   884,   885,     0,
     733,     0,     0,     0,     0,   180,    79,    80,    81,   734,
       0,    83,    84,     0,    85,   181,    87,   886,     0,     0,
      89,     0,     0,     0,     0,   887,   888,   889,    36,   735,
     736,   737,   738,     0,     0,     0,   890,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   739,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,     0,    36,     0,   209,     0,
       0,     0,   891,     0,     0,     0,     0,     0,   982,     0,
       0,     0,     0,     0,     0,   892,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,     0,   893,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   180,     0,
       0,    81,    82,     0,    83,    84,     0,    85,   181,    87,
      36,     0,   209,     0,     0,     0,     0,     0,   553,     0,
       0,     0,     0,   501,     0,     0,    83,    84,     0,    85,
     181,    87,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   223,
     210,     0,     0,     0,   116,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   180,     0,     0,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     355,   356,   357,     0,     0,     0,     0,     0,   554,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   358,
       0,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,     0,   380,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,     0,     0,
       0,     0,     0,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   426,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,   381,     0,     0,     0,
       0,     0,     0,   435,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     381,     0,   355,   356,   357,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     825,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,   355,   356,   357,     0,
       0,     0,     0,     0,     0,     0,     0,   381,     0,     0,
       0,     0,     0,     0,   862,   358,     0,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,     0,   355,   356,   357,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   902,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,   957,   958,   959,
       0,     0,     0,     0,     0,     0,     0,     0,   381,     0,
       0,     0,     0,     0,     0,  1203,   960,     0,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   982,     0,   957,   958,   959,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1221,   960,     0,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   957,   958,   959,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   982,
       0,     0,     0,     0,     0,   960,  1121,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   982,     0,   957,   958,   959,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   960,  1277,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   957,   958,   959,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   982,     0,
       0,     0,     0,     0,   960,  1282,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     982,    36,   957,   958,   959,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   960,  1359,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   982,     0,     0,
       0,    36,   505,     0,  1437,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,   274,     0,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,     0,     0,
     639,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1138,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,  1438,    83,    84,    36,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
       0,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,   381,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   355,   356,   357,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   710,   358,     0,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
     355,   356,   357,     0,     0,     0,     0,     0,     0,     0,
       0,   381,     0,     0,     0,     0,     0,     0,     0,   358,
     859,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   711,   380,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,     0,     0,
       0,     0,     0,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,   957,   958,
     959,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,     0,     0,     0,     0,     0,     0,   960,  1287,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     957,   958,   959,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   982,     0,     0,     0,     0,     0,   960,
       0,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   982,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   982,  -892,  -892,  -892,
    -892,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   381,  -892,  -892,  -892,
    -892,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   982
};

static const yytype_int16 yycheck[] =
{
       4,   162,   135,     4,    88,    52,   598,   183,     4,    32,
       4,    95,    96,     4,   836,     4,     4,    53,     4,   312,
      43,   415,  1036,   224,    47,   420,   172,  1039,   572,   681,
     394,   571,   818,   380,   165,    56,   445,   162,   251,   252,
      26,    27,   445,     4,   257,    49,   855,   709,    52,   443,
     134,   184,    30,   113,   546,     9,   716,    78,   220,   230,
      81,   113,   871,    30,     9,    69,   449,     9,  1025,  1023,
       9,    30,   914,     9,    45,    50,     9,    45,     9,    66,
       9,     9,    86,     9,    88,     9,    66,    45,     9,     9,
      96,    95,    96,     9,    35,     9,     9,    50,     9,    84,
     231,    84,     9,     9,    99,   100,     9,    79,     9,     9,
      66,   127,   128,     9,   497,    79,    79,    96,     9,     9,
     127,   128,   335,    96,    79,   154,   297,   113,    45,    79,
     134,    35,    33,  1562,   169,    79,    35,     0,    79,   135,
     127,   128,   116,   127,   128,    53,   152,    79,    66,   123,
      96,   211,    84,     8,   154,   148,   148,    65,   127,   128,
     145,   162,   145,   223,   950,   200,   312,   167,   108,    45,
     200,   347,   200,   152,   203,    79,   169,   169,   153,   152,
      79,    66,    66,    66,   148,   201,  1615,    79,   184,    66,
     162,    66,   196,   168,    66,   200,    66,   204,    66,    66,
     153,    66,  1216,   203,    66,    66,   152,   162,   200,   204,
     203,   203,   162,   203,   146,   147,   203,   201,   162,   200,
     200,   162,    30,   203,   200,   211,   166,   198,   170,   152,
     290,   217,   236,   201,   170,   202,   240,   223,   290,     4,
     244,   387,   205,   201,   203,    66,   422,   203,   202,   203,
     236,   272,   273,   274,   240,   468,  1065,   341,   262,   204,
     393,  1218,  1104,   202,   924,   411,   926,   429,  1225,   202,
    1227,   202,  1226,   202,   202,   236,   202,   209,   202,   240,
      45,   202,   202,   304,   201,   203,   202,   433,   202,   202,
     201,   338,    79,   279,   201,   201,   442,  1083,   201,   445,
     201,   201,   288,   289,   290,   201,   390,   391,   852,   295,
     201,   201,  1326,   205,  1328,   301,   808,   200,   701,   203,
     203,   325,   198,   706,   325,   201,   203,   148,   203,   325,
     334,   203,   320,   203,   338,   203,   203,   341,   203,   325,
     105,   203,   203,    96,    35,   110,   200,   112,   113,   114,
     115,   116,   117,   118,    79,   568,   569,    79,   394,    84,
      79,    35,    66,   576,    35,    84,    46,    47,    99,   100,
     430,   200,   112,   113,   114,   115,   116,   117,   200,  1336,
     125,   126,   203,   387,   388,   389,   390,   391,    79,    96,
     155,   156,    79,   158,   556,   200,   392,    84,    79,   152,
     795,   563,   380,  1417,   566,    79,   577,   411,    79,   387,
     200,    79,   115,    35,    79,   451,    84,   182,   205,    84,
     123,   146,   147,   127,   128,   586,   145,   146,   147,   433,
     200,   840,  1084,   411,   420,   153,     4,   840,  1100,   204,
     641,   445,   182,   200,   430,   152,   112,   113,   114,   115,
     116,   117,   456,    71,    72,   433,     4,   200,   145,   146,
     147,   586,   202,   200,   442,   146,   147,   445,   152,   670,
     456,   162,   648,   204,   868,   197,   638,    45,   146,   147,
     501,   146,   147,   208,   505,  1271,   687,    79,   162,   510,
     200,   162,   200,   104,   903,   456,   384,    45,  1038,   200,
     903,   112,   113,   114,   115,   116,   117,  1051,   512,    79,
    1054,   724,   725,    29,    84,   686,   182,   203,   202,   203,
     604,   692,    71,    72,   412,   818,   202,   874,   416,   202,
      46,   535,   536,    49,    77,   202,    79,   105,   921,   740,
      14,   202,   110,   202,   112,   113,   114,   115,   116,   117,
     118,  1565,   202,   145,   146,   147,    30,   105,  1344,    66,
     722,   692,   110,    66,   112,   113,   114,   115,   116,   117,
     118,   182,    29,    47,   202,   203,   146,   147,   202,   203,
    1232,   202,  1234,    66,   572,   586,   203,   155,   156,    46,
     158,   148,    49,    98,    99,   100,   809,    98,    99,   100,
     604,    46,    47,    48,    49,    50,    51,   155,   156,   595,
     158,   200,   155,   156,   182,   158,   159,   160,   200,   832,
      65,   112,   113,   114,   115,   116,   117,   112,   113,   114,
     843,    66,   123,   124,   182,   148,   204,   117,   118,   119,
     200,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,  1018,   152,   204,   950,  1591,  1592,
     203,   202,   205,    44,   650,  1205,  1060,    65,   159,   169,
     161,   148,   818,  1687,   207,  1069,  1220,   681,     9,   683,
     148,  1693,   705,    49,    50,    51,   148,    53,  1702,   298,
    1342,   182,   200,   302,   840,   699,  1708,     8,   699,    65,
    1587,  1588,   688,   699,   690,   724,   725,   202,  1103,   713,
     714,   169,   200,   699,    14,   928,    14,   930,   327,    79,
     329,   330,   331,   332,   202,   711,   202,   123,   714,    49,
      50,    51,   123,    14,   201,   897,   169,    14,   112,   113,
     114,   115,   116,   117,    96,    65,   201,   201,   769,   123,
     124,   200,   773,   714,   206,   104,   200,   903,   200,     9,
     764,   201,   201,   764,    88,     9,   770,    14,   764,   200,
     774,   775,  1424,   202,   183,   184,   185,   939,   764,     9,
     186,   190,   191,    79,   946,   194,   195,   161,   792,   873,
    1083,  1186,    79,  1337,  1177,    79,   782,   189,   200,   202,
       9,   202,     9,   807,   950,    79,   807,   201,   182,   795,
     796,   807,   201,   807,   202,   125,   807,   821,   807,   807,
     200,   807,   201,    66,  1037,    30,  1039,   126,  1222,   168,
      46,    47,    48,    49,    50,    51,   840,    53,   129,     9,
     201,  1663,   148,  1044,   201,     9,   807,   201,   836,    65,
    1063,     9,    14,   201,   198,     9,    66,     9,  1680,   201,
       9,  1244,   840,   170,   852,   125,  1688,    14,     9,   873,
     207,   207,   204,   200,   207,   201,   201,  1260,   207,   200,
     884,   885,   886,    96,   907,   202,   202,   129,     9,   148,
       4,   148,   201,   148,   186,   200,   874,   200,   200,   903,
     186,    14,   203,   200,   908,  1118,   910,   200,   912,   910,
       9,   912,    79,   203,   910,  1567,   912,  1079,    14,   202,
      14,   203,   908,   203,   910,   903,   912,   931,   914,   915,
    1324,    45,   207,    14,   202,   198,   200,  1083,    30,   200,
      30,    14,   200,     9,   200,   949,   200,   908,   201,   200,
     202,   955,   129,   202,    14,     9,   201,    65,     9,   207,
     145,    79,    96,  1346,     9,   200,  1128,    14,   129,  1182,
    1132,   202,  1355,  1135,    79,   201,   203,   200,  1271,   200,
    1142,   201,  1018,   129,   203,   203,  1369,     9,   145,    30,
     207,   105,   170,    73,   998,   201,   110,  1001,   112,   113,
     114,   115,   116,   117,   118,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1609,  1022,   202,
     202,  1022,   129,    30,     9,   201,  1022,   201,  1022,   129,
      65,  1022,   201,  1022,  1022,   201,  1022,     9,   201,     9,
       9,   155,   156,     4,   158,   201,    14,  1430,   201,   204,
    1263,  1344,  1265,    74,    75,    76,   204,   200,   203,    79,
    1046,  1022,   201,  1051,    85,   201,  1054,   200,   182,  1092,
     201,   203,   201,   201,   129,   201,  1080,     9,  1240,    30,
    1084,   203,   202,   202,    45,   201,  1678,   202,    96,   201,
     204,  1095,  1305,   157,   153,    14,    79,   110,     4,   201,
     201,   129,  1088,  1107,   201,   203,  1107,    14,   203,  1095,
     129,  1107,   133,   134,   135,   136,   137,  1103,  1104,   202,
      79,  1107,    14,   144,    79,  1271,   201,   203,   200,   150,
     151,   201,   129,   202,  1095,   202,    14,    14,    14,    45,
     202,    55,   203,   164,   105,    79,  1307,   200,    79,   110,
       9,   112,   113,   114,   115,   116,   117,   118,  1371,   202,
     181,    79,   108,    96,   148,   160,    96,    33,    14,   200,
     202,   201,  1305,   200,  1557,   166,  1559,  1181,    79,     9,
      79,   163,    14,  1184,   201,  1568,   202,    26,    27,   201,
     201,    30,  1193,   203,   155,   156,  1184,   158,  1344,   105,
    1186,    14,    77,    14,   110,  1193,   112,   113,   114,   115,
     116,   117,   118,    52,    79,    79,    79,    14,    79,  1395,
     773,   182,   510,   769,  1671,   872,   391,   869,  1232,  1612,
    1234,   390,  1220,   810,  1684,  1101,  1429,   388,  1680,  1257,
    1420,  1449,   515,   204,  1533,  1300,  1712,  1700,  1295,   155,
     156,  1545,   158,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    41,    53,  1416,    26,    27,
     394,   486,    30,   993,   149,   990,   182,   152,    65,   761,
     155,   156,   486,   158,   159,   160,  1290,  1026,  1067,  1290,
     947,  1295,   885,   899,  1290,  1080,  1300,   296,   204,   953,
     315,   289,   724,  1009,  1290,   932,  1307,    -1,  1304,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,  1706,    -1,    -1,  1327,    -1,    -1,   204,
    1713,    -1,  1333,    -1,  1335,    -1,    -1,    -1,  1342,  1327,
      -1,    -1,    -1,  1347,    -1,  1333,  1347,  1335,  1352,  1337,
    1397,  1347,  1356,    -1,    -1,  1356,    -1,    -1,    -1,    -1,
    1356,  1347,    -1,    -1,    -1,    -1,  1352,    -1,    -1,    -1,
    1356,  1547,   211,    -1,    -1,    -1,    -1,    -1,   217,    -1,
    1384,    -1,    -1,    -1,   223,    -1,  1390,    -1,    -1,    -1,
      -1,  1352,    -1,  1397,    -1,    -1,    -1,  1401,    -1,    -1,
      -1,    -1,    -1,  1399,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   251,   252,    -1,    -1,    -1,    -1,   257,    -1,
    1424,  1422,    -1,  1427,  1428,  1429,  1427,    -1,    -1,  1433,
      -1,  1427,  1433,    -1,  1422,    -1,  1440,  1433,    -1,  1440,
     279,  1427,  1428,  1429,  1440,    -1,    -1,  1433,    -1,   288,
     289,    -1,    -1,   211,  1440,    -1,   295,    -1,    -1,   217,
    1544,    -1,   301,    -1,    -1,   223,    -1,  1428,  1429,    -1,
      -1,    -1,    -1,   312,    -1,    -1,    -1,    -1,    -1,    -1,
    1693,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1708,   335,    -1,  1659,   338,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1603,
      -1,   279,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     288,   289,    -1,    -1,    -1,    -1,    -1,   295,    -1,    -1,
    1544,   380,  1675,   301,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   312,    63,    64,    -1,    -1,  1563,
      -1,    -1,    -1,  1567,    -1,    -1,    -1,    -1,  1572,    -1,
      -1,  1572,    -1,    -1,    -1,    -1,  1572,    -1,  1582,    -1,
      -1,   420,    -1,  1587,  1588,    -1,  1572,  1591,  1592,    -1,
      -1,   430,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1603,
      -1,    -1,    -1,    -1,    -1,    -1,  1610,  1611,    -1,  1610,
    1611,    -1,    -1,  1617,  1610,  1611,  1617,    -1,    -1,   127,
     128,  1617,   380,    -1,  1610,  1611,    -1,    -1,    -1,   468,
     469,  1617,    -1,   472,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1651,    -1,    -1,
    1651,    -1,    -1,    -1,  1658,  1651,    -1,    -1,  1659,    -1,
      -1,    -1,   420,    -1,    -1,  1651,    -1,    -1,    -1,    -1,
    1674,    -1,   430,    -1,    -1,  1663,    -1,    -1,    -1,    -1,
     519,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1680,   201,    -1,    -1,    -1,    -1,    -1,    -1,
    1688,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1714,    -1,    -1,  1714,    -1,    -1,    -1,  1721,  1714,    -1,
    1721,    -1,    -1,    -1,    -1,  1721,    -1,    -1,  1714,   568,
     569,    -1,    -1,    -1,    -1,  1721,    -1,   576,    10,    11,
      12,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,   595,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    -1,    65,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   650,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   595,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   688,
      65,   690,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   711,   712,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,   724,   725,   726,   727,   728,
     729,   730,   650,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     739,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,   765,    -1,   201,    -1,
     688,    -1,   690,    -1,    -1,   207,    -1,    65,    77,    29,
      -1,    -1,    -1,   782,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   711,   793,    -1,   795,   796,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
     809,   810,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   818,
      -1,    -1,    -1,    -1,    26,    27,    -1,    77,    30,   204,
      -1,    -1,    -1,   832,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   843,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   851,   152,   104,   854,   155,   156,    -1,   158,
     159,   160,    -1,    -1,   782,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   874,    -1,   795,   796,    -1,
      -1,   131,   132,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,   149,
     818,    -1,   152,   153,   203,   155,   156,    -1,   158,   159,
     160,    77,    -1,    79,   202,   914,   915,    -1,    -1,    -1,
      -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,   928,
      -1,   930,    -1,   932,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,   947,    -1,
     200,   950,   951,   952,   953,    -1,   874,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,    -1,    -1,    -1,    -1,    -1,   155,
     156,    -1,   158,   159,   160,    -1,   914,   915,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1006,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   217,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,   950,    -1,    -1,    -1,    -1,   203,  1037,   205,
    1039,    -1,    -1,    -1,    -1,    -1,    -1,  1046,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,  1063,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   279,    -1,    -1,
      -1,    -1,    -1,    -1,  1083,    -1,   288,   289,    -1,  1088,
      -1,    -1,    -1,   295,    -1,    -1,    -1,    63,    64,   301,
      -1,    -1,    -1,    -1,  1103,  1104,    -1,  1106,    -1,    -1,
     312,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1118,
      -1,    -1,  1121,    -1,  1123,    -1,    -1,    -1,  1046,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,  1146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,    -1,    -1,  1083,    -1,    -1,    -1,    -1,
    1088,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   380,    63,
      64,    -1,    -1,  1182,  1183,  1103,  1104,  1186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1211,    -1,    -1,    -1,    -1,    29,   420,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1263,    -1,  1265,    -1,  1186,    -1,
     472,  1270,  1271,    -1,    -1,    -1,  1275,    -1,  1277,    -1,
    1279,    -1,    -1,  1282,    -1,  1284,    -1,    -1,  1287,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1295,  1296,    -1,  1298,
      -1,    -1,    10,    11,    12,    77,  1305,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   519,    -1,    -1,
      -1,    29,  1321,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,  1344,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1271,    -1,    -1,    -1,    65,    -1,    -1,
    1359,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,  1371,  1372,  1373,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,   155,   156,   157,   158,   159,   160,    -1,
      -1,    -1,   204,   595,    -1,    -1,    -1,    -1,  1397,    -1,
      -1,    -1,  1401,    -1,    -1,    -1,    -1,    -1,    -1,  1408,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,  1344,    -1,   200,    -1,
      10,    11,    12,    77,    -1,    79,    -1,  1436,  1437,  1438,
      -1,    -1,    -1,    -1,    -1,  1444,  1445,    -1,   650,    29,
    1449,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,   123,
      -1,    -1,    -1,    -1,    -1,    65,   688,    -1,   690,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   711,
     712,   155,   156,    -1,   158,   159,   160,    -1,    77,    -1,
      -1,    -1,    -1,    -1,   726,   727,   728,   729,   730,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   739,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,  1552,    -1,    -1,    -1,    -1,    -1,   203,
      -1,   205,   121,   765,  1563,    -1,    -1,   251,   252,    -1,
      -1,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
     782,    -1,    -1,  1582,    -1,    -1,    -1,  1586,    -1,    -1,
      -1,   793,    -1,   795,   796,    -1,   155,   156,  1597,   158,
     159,   160,    -1,  1602,    -1,    -1,  1605,    -1,   810,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   818,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   204,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   851,
      -1,   335,   854,    -1,   338,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1662,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1670,   874,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1684,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1693,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1708,
      -1,    -1,   914,   915,    -1,    -1,    -1,    -1,    -1,  1718,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1726,    -1,    -1,
      -1,    -1,    -1,  1732,    -1,    -1,  1735,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   947,    -1,    -1,   950,   951,
     952,   953,    -1,    -1,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,    -1,    -1,    -1,   468,   469,    -1,    -1,   472,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,   519,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1046,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,  1083,    -1,    -1,   568,   569,  1088,    -1,    -1,    -1,
      -1,    65,   576,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1103,  1104,    -1,  1106,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,
      -1,  1123,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1146,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,  1183,    -1,    -1,  1186,   204,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1211,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   712,    -1,
     204,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     724,   725,   726,   727,   728,   729,   730,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   739,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1270,  1271,
      -1,    -1,    -1,  1275,    -1,  1277,    -1,  1279,    -1,    -1,
    1282,    -1,  1284,    -1,    -1,  1287,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1296,    29,  1298,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,   204,  1321,
      -1,    55,    -1,    -1,    -1,   809,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1344,    77,    -1,    -1,    -1,    -1,   832,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,  1359,    -1,   843,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   851,    -1,    -1,
    1372,  1373,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1408,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1436,  1437,  1438,    -1,    -1,   173,
      -1,    -1,  1444,  1445,   928,    -1,   930,  1449,   932,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   947,    -1,    -1,   200,   951,   952,   953,
     204,    -1,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1006,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,   204,    -1,    -1,
    1552,    -1,    -1,  1037,    -1,  1039,    -1,    65,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,    67,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,  1063,
      -1,    -1,    -1,    -1,  1586,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1597,    -1,    -1,    -1,    -1,
    1602,    -1,    -1,  1605,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,    -1,
      -1,    -1,  1106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1118,    -1,    -1,  1121,    -1,  1123,
     155,   156,    -1,   158,   159,   160,    -1,    -1,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
    1662,    -1,  1146,    -1,    -1,    -1,   167,    -1,  1670,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,  1684,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   204,    -1,  1182,   200,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,  1718,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1726,    -1,    -1,  1211,    -1,    -1,
    1732,    -1,    29,  1735,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,  1263,
      -1,  1265,    -1,    -1,    -1,    -1,  1270,    77,    -1,    79,
      -1,  1275,    -1,  1277,    -1,  1279,    -1,    -1,  1282,    -1,
    1284,    -1,    -1,  1287,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1295,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1305,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1321,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   149,
      53,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    65,    -1,    -1,  1359,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1371,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   204,    -1,    -1,
     200,    -1,    -1,  1397,    -1,   205,    -1,  1401,    -1,    -1,
      -1,    -1,     5,     6,  1408,     8,     9,    10,    11,    12,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,  1436,  1437,  1438,    10,    11,    12,    41,    -1,
    1444,    -1,    -1,    -1,    -1,    48,    -1,    50,    -1,    -1,
      53,    -1,    55,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      65,    -1,    10,    11,    12,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,  1552,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1563,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,   117,  1582,    -1,
      -1,    -1,  1586,    -1,    -1,   188,    -1,    -1,    -1,    65,
     130,    -1,    -1,  1597,    -1,    -1,    -1,    -1,  1602,    -1,
      -1,  1605,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,   232,
     472,    -1,   235,    -1,    -1,    -1,    -1,    -1,    -1,   242,
     243,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,  1662,    -1,
     200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   519,    -1,    -1,
      -1,    -1,    -1,    -1,   202,    -1,    -1,   290,    -1,  1693,
      -1,    -1,    -1,   296,    -1,    -1,    -1,   300,    -1,    -1,
      -1,    -1,    -1,    -1,  1708,    -1,    -1,    -1,    -1,    -1,
     313,   314,   315,    -1,  1718,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1726,   326,    -1,    -1,    -1,    -1,  1732,    -1,
      -1,  1735,    -1,    -1,   337,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,
      -1,    -1,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,    -1,   381,    -1,
     383,   384,    -1,   386,    -1,    -1,   117,    -1,    -1,    -1,
      -1,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,    -1,    -1,    -1,    -1,    -1,   412,
     413,    -1,   415,   416,   417,    -1,    -1,    -1,   149,    -1,
     423,   152,   153,   426,   155,   156,   472,   158,   159,   160,
      -1,    -1,   435,    -1,   437,    -1,    -1,    -1,    -1,    -1,
     443,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   451,    -1,
     453,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
     712,    -1,   203,   519,   205,    -1,   479,    -1,    -1,   482,
     483,   484,    -1,    -1,   726,   727,   728,   729,   730,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,   739,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     513,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,   596,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    65,   608,    -1,    -1,    -1,   851,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   640,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   649,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   667,    -1,   712,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     726,   727,   728,   729,   730,   202,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   739,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   947,   709,    -1,    -1,   951,
     952,   953,    -1,    -1,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   202,    -1,    -1,    -1,    -1,    -1,   750,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,    -1,   472,  1006,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,   201,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   117,
      -1,   804,    -1,    -1,    -1,   851,    -1,    -1,    -1,    -1,
     519,    -1,   130,    -1,   817,    -1,    -1,    -1,    -1,    -1,
     823,    -1,   825,    -1,   827,    -1,    -1,    -1,    -1,    -1,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,   849,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   859,    -1,    -1,   862,
      -1,   864,    -1,    -1,  1106,   868,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,  1121,
      -1,  1123,   200,    77,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,
      -1,   947,    -1,    -1,  1146,   951,   952,   953,    -1,    -1,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1006,   155,   156,    -1,   158,   159,   160,    -1,    -1,  1211,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   984,   985,   986,    -1,    -1,    -1,   990,   991,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   712,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    77,    -1,    79,    -1,  1018,    -1,   726,   727,   728,
     729,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1270,    -1,
     739,    -1,    -1,  1275,    -1,  1277,    -1,  1279,    -1,    -1,
    1282,    -1,  1284,    -1,  1047,  1287,    -1,    -1,    -1,    -1,
      -1,   117,    -1,    -1,    -1,    -1,    -1,  1060,    -1,    -1,
    1106,    -1,    -1,    -1,   130,    -1,  1069,  1070,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1121,    -1,  1123,    -1,  1321,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,  1100,    -1,    -1,
    1146,    -1,    -1,    -1,    -1,    -1,    -1,  1110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1359,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,   851,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1211,  1408,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,  1196,  1436,  1437,  1438,  1200,    -1,  1202,
    1203,    -1,  1444,    65,    -1,    -1,  1448,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1221,  1222,
      -1,    -1,    -1,    -1,  1270,    -1,    -1,    -1,    -1,  1275,
      -1,  1277,    -1,  1279,    -1,    -1,  1282,    -1,  1284,    -1,
      -1,  1287,   951,   952,   953,    -1,    -1,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,    -1,  1321,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1288,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,  1006,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1552,    -1,    -1,  1359,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1324,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,  1586,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1597,    -1,    -1,    -1,    -1,
    1602,    -1,  1408,  1605,   149,    -1,    -1,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1627,    -1,    -1,    -1,    -1,
    1436,  1437,  1438,    -1,    -1,    -1,    -1,  1106,  1444,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,  1121,    -1,  1123,    -1,    -1,    -1,    -1,    -1,
    1662,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1439,  1146,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1718,    65,    -1,    -1,
      -1,    -1,    -1,    -1,  1726,    -1,    10,    11,    12,    -1,
    1732,    -1,    -1,  1735,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1211,    -1,    -1,    29,  1552,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    77,    53,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1586,    65,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,  1597,    -1,    -1,    -1,    -1,  1602,    -1,    -1,  1605,
      -1,  1270,    10,    11,    12,    -1,  1275,    -1,  1277,    -1,
    1279,   104,   105,  1282,    -1,  1284,    -1,    -1,  1287,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,   155,   156,   196,   158,
     159,   160,  1321,    -1,    -1,    -1,  1662,    65,    -1,   152,
      -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
    1359,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,    -1,   192,   193,
      -1,    -1,  1718,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1726,    12,    -1,    -1,    -1,    -1,  1732,    -1,    -1,  1735,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1408,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    10,    11,    12,    -1,    -1,    -1,  1436,  1437,  1438,
      -1,    -1,    -1,    -1,    65,  1444,    -1,    77,    -1,    -1,
      29,   189,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   149,
      53,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1552,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1586,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,  1597,    -1,
      -1,    -1,    -1,  1602,    -1,    -1,  1605,    -1,    -1,   188,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,   187,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,  1662,    -1,    -1,   101,   102,   103,   104,
     105,   106,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,  1718,
     155,   156,    -1,   158,   159,   160,   161,  1726,    -1,   164,
      -1,    -1,   167,  1732,    -1,    -1,  1735,    -1,   173,   174,
     175,   176,   177,    -1,   179,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,   204,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,   177,
      -1,   179,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,   105,   106,   107,    -1,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
     161,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,   177,    -1,   179,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,    -1,   205,   206,    -1,   208,   209,     3,
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
     124,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,   161,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
     204,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
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
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,   161,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,    -1,   155,   156,    -1,   158,   159,   160,   161,    -1,
      -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    95,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,   161,    -1,    -1,   164,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
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
      -1,   130,   131,    -1,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,   161,    -1,    -1,   164,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,   203,   204,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,   161,
      -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,   204,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    93,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,   161,    -1,    -1,   164,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
     161,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,    -1,   208,   209,     3,
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
     124,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,   161,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
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
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,   161,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,    -1,   155,   156,    -1,   158,   159,   160,   161,    -1,
      -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,   204,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,   161,    -1,    -1,   164,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
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
      -1,   130,   131,    -1,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,    -1,   205,   206,    -1,   208,   209,     3,     4,
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
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,    -1,   205,   206,    -1,   208,   209,     3,
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
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
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
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,    -1,    -1,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,    -1,   155,   156,    -1,   158,   159,   160,    -1,   162,
      -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    11,    12,   203,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    29,    13,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    35,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,   164,    -1,    -1,   167,     3,     4,     5,     6,
       7,   173,   174,   175,   176,    -1,    13,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    11,    12,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    29,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,
       5,     6,     7,   173,   174,   175,   176,    -1,    13,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,     3,     4,     5,     6,     7,   173,   174,
     175,   176,    -1,    13,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,    -1,    -1,
     205,   206,    -1,   208,   209,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,
       5,     6,     7,   173,   174,   175,   176,    -1,    13,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,    -1,    -1,   205,   206,    -1,   208,   209,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,   201,    -1,    -1,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    30,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    12,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,
       5,     6,     7,   173,   174,   175,   176,    -1,    13,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,     3,     4,     5,     6,     7,   173,   174,
     175,   176,    -1,    13,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,
     205,   206,    -1,   208,   209,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    29,    -1,    -1,   130,   131,    -1,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,    -1,
      55,    -1,   155,   156,    -1,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
     173,    -1,    77,    -1,   177,    -1,   179,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,   149,    -1,    -1,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   104,    -1,    -1,   200,    -1,    29,    -1,   204,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,    -1,   155,   156,    77,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   173,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   173,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   131,   132,    -1,   200,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    77,   158,   159,   160,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    66,   158,   159,   160,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,   153,   173,   155,   156,
      85,   158,   159,   160,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    77,    -1,    79,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    77,    -1,   164,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,    -1,   117,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    46,    47,    -1,   200,    -1,    -1,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,    66,   155,   156,    -1,   158,   159,   160,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    85,   152,    -1,    -1,   155,   156,    -1,   158,   159,
     160,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    46,    47,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    66,    -1,    -1,
     164,    -1,    -1,    -1,    -1,    74,    75,    76,    77,   173,
     174,   175,   176,    -1,    -1,    -1,    85,   181,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,   200,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    77,    -1,    79,    -1,
      -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,
     159,   160,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
     117,    -1,    -1,    -1,   205,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,   129,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,   129,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   129,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    29,   129,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   129,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    29,   129,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    77,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   129,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    77,   152,    -1,   129,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,
     123,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   123,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   129,   155,   156,    77,   158,   159,   160,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    28,    29,    -1,    31,    32,    33,
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
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   211,   212,     0,   213,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      85,    86,    87,    88,    90,    92,    94,    97,   101,   102,
     103,   104,   105,   106,   107,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   119,   120,   121,   122,   123,   124,
     130,   131,   133,   134,   135,   136,   137,   144,   149,   150,
     151,   152,   153,   155,   156,   158,   159,   160,   161,   164,
     167,   173,   174,   175,   176,   177,   179,   181,   182,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   200,   202,   203,   205,   206,   208,   209,
     214,   217,   224,   225,   226,   227,   228,   229,   232,   248,
     249,   253,   256,   261,   267,   325,   326,   331,   335,   336,
     337,   338,   339,   340,   341,   342,   344,   347,   358,   359,
     360,   361,   362,   366,   367,   369,   388,   398,   399,   400,
     405,   408,   426,   432,   434,   435,   436,   437,   438,   439,
     440,   441,   443,   464,   466,   468,   115,   116,   117,   130,
     149,   159,   217,   248,   325,   341,   434,   341,   200,   341,
     341,   341,   101,   341,   341,   424,   425,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,    79,
     117,   200,   225,   399,   400,   434,   434,    35,   341,   447,
     448,   341,   117,   200,   225,   399,   400,   401,   433,   439,
     444,   445,   200,   332,   402,   200,   332,   348,   333,   341,
     234,   332,   200,   200,   200,   332,   202,   341,   217,   202,
     341,    29,    55,   131,   132,   153,   173,   200,   217,   228,
     469,   481,   482,   183,   202,   338,   341,   368,   370,   203,
     241,   341,   104,   105,   152,   218,   221,   224,    79,   205,
     293,   294,   116,   123,   115,   123,    79,   295,   200,   200,
     200,   200,   217,   265,   470,   200,   200,    79,    84,   145,
     146,   147,   461,   462,   152,   203,   224,   224,   217,   266,
     470,   153,   200,   200,   200,   200,   470,   470,    79,   197,
     350,   331,   341,   342,   434,   230,   203,    84,   403,   461,
      84,   461,   461,    30,   152,   169,   471,   200,     9,   202,
      35,   247,   153,   264,   470,   117,   182,   248,   326,   202,
     202,   202,   202,   202,   202,    10,    11,    12,    29,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      53,    65,   202,    66,    66,   202,   203,   148,   124,   159,
     161,   267,   324,   325,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    63,    64,   127,
     128,   428,    66,   203,   431,   200,    66,   203,   205,   440,
     200,   247,   248,    14,   341,   202,   129,    44,   217,   423,
     200,   331,   434,   148,   434,   129,   207,     9,   410,   331,
     434,   471,   148,   200,   404,   428,   431,   201,   341,    30,
     232,     8,   352,     9,   202,   232,   233,   333,   334,   341,
     217,   279,   236,   202,   202,   202,   482,   482,   169,   200,
     104,   482,    14,   217,    79,   202,   202,   202,   183,   184,
     185,   190,   191,   194,   195,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   383,   384,   385,   242,   108,   166,
     202,   152,   219,   222,   224,   152,   220,   223,   224,   224,
       9,   202,    96,   203,   434,     9,   202,   123,   123,    14,
       9,   202,   434,   465,   465,   331,   342,   434,   201,   169,
     259,   130,   434,   446,   447,    66,   428,   145,   462,    78,
     341,   434,    84,   145,   462,   224,   216,   202,   203,   254,
     262,   389,   391,    85,   205,   353,   354,   356,   400,   440,
     466,   341,   454,   456,   341,   453,   455,   453,    14,    96,
     467,   349,   351,   289,   290,   426,   427,   201,   201,   201,
     204,   231,   232,   249,   256,   261,   426,   341,   206,   208,
     209,   217,   472,   473,   482,    35,   162,   291,   292,   341,
     469,   200,   470,   257,   247,   341,   341,   341,    30,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   401,   341,   341,   442,   442,   341,   449,   450,   123,
     203,   217,   439,   440,   265,   217,   266,   264,   248,    27,
      35,   335,   338,   341,   368,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   203,   217,   429,
     430,   439,   442,   341,   291,   442,   341,   446,   247,   201,
     341,   200,   422,     9,   410,   331,   201,   217,    35,   341,
      35,   341,   201,   201,   439,   291,   429,   430,   201,   230,
     283,   203,   338,   341,   341,    88,    30,   232,   277,   202,
      28,    96,    14,     9,   201,    30,   203,   280,   482,    85,
     228,   478,   479,   480,   200,     9,    46,    47,    52,    54,
      66,    85,   131,   144,   153,   173,   174,   175,   176,   200,
     225,   226,   228,   363,   364,   365,   399,   405,   406,   407,
     186,    79,   341,    79,    79,   341,   380,   381,   341,   341,
     373,   383,   189,   386,   230,   200,   240,   224,   202,     9,
      96,   224,   202,     9,    96,    96,   221,   217,   341,   294,
     406,    79,     9,   201,   201,   201,   201,   201,   202,    46,
      47,   476,   477,   125,   270,   200,     9,   201,   201,    79,
      80,   217,   463,   217,    66,   204,   204,   213,   215,    30,
     126,   269,   168,    50,   153,   168,   393,   129,     9,   410,
     201,   148,   201,     9,   410,   129,   201,     9,   410,   201,
     482,   482,    14,   352,   289,   198,     9,   411,   482,   483,
     428,   431,   204,     9,   410,   170,   434,   341,   201,     9,
     411,    14,   345,   250,   125,   268,   200,   470,   341,    30,
     207,   207,   129,   204,     9,   410,   341,   471,   200,   260,
     255,   263,   258,   247,    68,   434,   341,   471,   207,   204,
     201,   207,   204,   201,    46,    47,    66,    74,    75,    76,
      85,   131,   144,   173,   217,   413,   415,   418,   421,   217,
     434,   434,   129,   428,   431,   201,   284,    71,    72,   285,
     230,   332,   230,   334,    96,    35,   130,   274,   434,   406,
     217,    30,   232,   278,   202,   281,   202,   281,     9,   170,
     129,   148,     9,   410,   201,   162,   472,   473,   474,   472,
     406,   406,   406,   406,   406,   409,   412,   200,    84,   148,
     200,   200,   200,   200,   406,   148,   203,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   341,   186,   186,    14,   192,   193,   382,
       9,   196,   386,    79,   204,   399,   203,   244,    96,   222,
     217,    96,   223,   217,   217,   204,    14,   434,   202,     9,
     170,   217,   271,   399,   203,   446,   130,   434,    14,   207,
     341,   204,   213,   482,   271,   203,   392,    14,   341,   353,
     217,   341,   341,   341,   202,   482,   198,    30,   475,   427,
      35,    79,   162,   429,   430,   482,    35,   162,   341,   406,
     289,   200,   399,   269,   346,   251,   341,   341,   341,   204,
     200,   291,   270,    30,   269,   268,   470,   401,   204,   200,
      14,    74,    75,    76,   217,   414,   414,   415,   416,   417,
     200,    84,   145,   200,     9,   410,   201,   422,    35,   341,
     429,   430,    71,    72,   286,   332,   232,   204,   202,    89,
     202,   274,   434,   200,   129,   273,    14,   230,   281,    98,
      99,   100,   281,   204,   482,   482,   217,   478,     9,   201,
     410,   129,   207,     9,   410,   409,   217,   353,   355,   357,
     406,   458,   460,   406,   457,   459,   457,   201,   123,   217,
     406,   451,   452,   406,   406,   406,    30,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     341,   341,   341,   381,   341,   371,    79,   245,   217,   217,
     406,   477,    96,     9,   299,   201,   200,   335,   338,   341,
     207,   204,   467,   299,   154,   167,   203,   388,   395,   154,
     203,   394,   129,   129,   202,   475,   482,   352,   483,    79,
     162,    14,    79,   471,   434,   341,   201,   289,   203,   289,
     200,   129,   200,   291,   201,   203,   482,   203,   269,   252,
     404,   291,   129,   207,     9,   410,   416,   145,   353,   419,
     420,   415,   434,   332,    30,    73,   232,   202,   334,   273,
     446,   274,   201,   406,    95,    98,   202,   341,    30,   202,
     282,   204,   170,   129,   162,    30,   201,   406,   406,   201,
     129,     9,   410,   201,   201,     9,   410,   129,   201,     9,
     410,   201,   129,   204,     9,   410,   406,    30,   187,   201,
     230,   217,   482,   399,     4,   105,   110,   118,   155,   156,
     158,   204,   300,   323,   324,   325,   330,   426,   446,   204,
     203,   204,    50,   341,   341,   341,   341,   352,    35,    79,
     162,    14,    79,   406,   200,   475,   201,   299,   201,   289,
     341,   291,   201,   299,   467,   299,   203,   200,   201,   415,
     415,   201,   129,   201,     9,   410,    30,   230,   202,   201,
     201,   201,   237,   202,   202,   282,   230,   482,   482,   129,
     406,   353,   406,   406,   406,   406,   406,   406,   341,   203,
     204,    96,   125,   126,   469,   272,   399,   118,   131,   132,
     153,   159,   309,   310,   311,   399,   157,   315,   316,   121,
     200,   217,   317,   318,   301,   248,   482,     9,   202,   324,
     201,   296,   153,   390,   204,   204,    79,   162,    14,    79,
     406,   291,   110,   343,   475,   204,   475,   201,   201,   204,
     203,   204,   299,   289,   129,   415,   353,   230,   235,   238,
      30,   232,   276,   230,   201,   406,   129,   129,   129,   188,
     230,   482,   399,   399,    14,     9,   202,   203,   203,     9,
     202,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    53,    67,    68,    69,    70,    71,    72,    73,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   130,   131,   133,   134,   135,   136,   137,   149,   150,
     151,   161,   163,   164,   166,   173,   177,   179,   181,   182,
     217,   396,   397,     9,   202,   153,   157,   217,   318,   319,
     320,   202,    79,   329,   247,   302,   469,   248,   204,   297,
     298,   469,    14,    79,   406,   201,   200,   203,   202,   203,
     321,   343,   475,   296,   204,   201,   415,   129,    30,   232,
     275,   276,   230,   406,   406,   406,   341,   204,   202,   202,
     406,   399,   305,   312,   405,   310,    14,    30,    47,   313,
     316,     9,    33,   201,    29,    46,    49,    14,     9,   202,
     470,   329,    14,   247,   202,    14,   406,    35,    79,   387,
     230,   230,   203,   321,   204,   475,   415,   230,    93,   189,
     243,   204,   217,   228,   306,   307,   308,     9,   204,   406,
     397,   397,    55,   314,   319,   319,    29,    46,    49,   406,
      79,   200,   202,   406,   470,   406,    79,     9,   411,   204,
     204,   230,   321,    91,   202,    79,   108,   239,   148,    96,
     405,   160,    14,   303,   200,    35,    79,   201,   204,   202,
     200,   166,   246,   217,   324,   325,   406,   287,   288,   427,
     304,    79,   399,   244,   163,   217,   202,   201,     9,   411,
     112,   113,   114,   327,   328,   287,    79,   272,   202,   475,
     427,   483,   201,   201,   202,   202,   203,   322,   327,    35,
      79,   162,   475,   203,   230,   483,    79,   162,    14,    79,
     322,   230,   204,    35,    79,   162,    14,    79,   406,   204,
      79,   162,    14,    79,   406,    14,    79,   406,   406
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
#line 736 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 857 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset();;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { (yyval).reset();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
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
#line 1100 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
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
#line 1117 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval).reset();;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval).reset();;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval).reset();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { (yyval).reset();;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval).reset();;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval).reset();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval).reset();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval).reset();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval).reset();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval).reset();;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { (yyval).reset();;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval).reset();;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { Token v; Token w;
                                         (yyvsp[(1) - (4)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (4)]), nullptr, (yyvsp[(1) - (4)]));
                                         _p->finishStatement((yyvsp[(4) - (4)]), (yyvsp[(4) - (4)])); (yyvsp[(4) - (4)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (4)]),
                                                            v,(yyvsp[(2) - (4)]),w,(yyvsp[(4) - (4)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { Token u; Token v;
                                         (yyvsp[(1) - (7)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (7)]), nullptr, (yyvsp[(1) - (7)]));
                                         _p->finishStatement((yyvsp[(7) - (7)]), (yyvsp[(7) - (7)])); (yyvsp[(7) - (7)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (7)]),
                                                            u,(yyvsp[(4) - (7)]),v,(yyvsp[(7) - (7)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
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

  case 523:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
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

  case 524:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval).reset();;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval).reset();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval).reset();;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval).reset();;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval).reset();;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval).reset();;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset();;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval).reset();;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval).reset();;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval)++;;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval).reset();;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    {;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    {;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    {;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    {;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13544 "hphp.tab.cpp"
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
#line 3049 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

