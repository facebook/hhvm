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
     T_HASHBANG = 318,
     T_CHARACTER = 319,
     T_BAD_CHARACTER = 320,
     T_ENCAPSED_AND_WHITESPACE = 321,
     T_CONSTANT_ENCAPSED_STRING = 322,
     T_ECHO = 323,
     T_DO = 324,
     T_WHILE = 325,
     T_ENDWHILE = 326,
     T_FOR = 327,
     T_ENDFOR = 328,
     T_FOREACH = 329,
     T_ENDFOREACH = 330,
     T_DECLARE = 331,
     T_ENDDECLARE = 332,
     T_AS = 333,
     T_SWITCH = 334,
     T_ENDSWITCH = 335,
     T_CASE = 336,
     T_DEFAULT = 337,
     T_BREAK = 338,
     T_GOTO = 339,
     T_CONTINUE = 340,
     T_FUNCTION = 341,
     T_CONST = 342,
     T_RETURN = 343,
     T_TRY = 344,
     T_CATCH = 345,
     T_THROW = 346,
     T_USE = 347,
     T_GLOBAL = 348,
     T_PUBLIC = 349,
     T_PROTECTED = 350,
     T_PRIVATE = 351,
     T_FINAL = 352,
     T_ABSTRACT = 353,
     T_STATIC = 354,
     T_VAR = 355,
     T_UNSET = 356,
     T_ISSET = 357,
     T_EMPTY = 358,
     T_HALT_COMPILER = 359,
     T_CLASS = 360,
     T_INTERFACE = 361,
     T_EXTENDS = 362,
     T_IMPLEMENTS = 363,
     T_OBJECT_OPERATOR = 364,
     T_NULLSAFE_OBJECT_OPERATOR = 365,
     T_DOUBLE_ARROW = 366,
     T_LIST = 367,
     T_ARRAY = 368,
     T_CALLABLE = 369,
     T_CLASS_C = 370,
     T_METHOD_C = 371,
     T_FUNC_C = 372,
     T_LINE = 373,
     T_FILE = 374,
     T_COMMENT = 375,
     T_DOC_COMMENT = 376,
     T_OPEN_TAG = 377,
     T_OPEN_TAG_WITH_ECHO = 378,
     T_CLOSE_TAG = 379,
     T_WHITESPACE = 380,
     T_START_HEREDOC = 381,
     T_END_HEREDOC = 382,
     T_DOLLAR_OPEN_CURLY_BRACES = 383,
     T_CURLY_OPEN = 384,
     T_DOUBLE_COLON = 385,
     T_NAMESPACE = 386,
     T_NS_C = 387,
     T_DIR = 388,
     T_NS_SEPARATOR = 389,
     T_XHP_LABEL = 390,
     T_XHP_TEXT = 391,
     T_XHP_ATTRIBUTE = 392,
     T_XHP_CATEGORY = 393,
     T_XHP_CATEGORY_LABEL = 394,
     T_XHP_CHILDREN = 395,
     T_ENUM = 396,
     T_XHP_REQUIRED = 397,
     T_TRAIT = 398,
     T_ELLIPSIS = 399,
     T_INSTEADOF = 400,
     T_TRAIT_C = 401,
     T_HH_ERROR = 402,
     T_FINALLY = 403,
     T_XHP_TAG_LT = 404,
     T_XHP_TAG_GT = 405,
     T_TYPELIST_LT = 406,
     T_TYPELIST_GT = 407,
     T_UNRESOLVED_LT = 408,
     T_COLLECTION = 409,
     T_SHAPE = 410,
     T_VARRAY = 411,
     T_MIARRAY = 412,
     T_MSARRAY = 413,
     T_TYPE = 414,
     T_UNRESOLVED_TYPE = 415,
     T_NEWTYPE = 416,
     T_UNRESOLVED_NEWTYPE = 417,
     T_COMPILER_HALT_OFFSET = 418,
     T_ASYNC = 419,
     T_FROM = 420,
     T_WHERE = 421,
     T_JOIN = 422,
     T_IN = 423,
     T_ON = 424,
     T_EQUALS = 425,
     T_INTO = 426,
     T_LET = 427,
     T_ORDERBY = 428,
     T_ASCENDING = 429,
     T_DESCENDING = 430,
     T_SELECT = 431,
     T_GROUP = 432,
     T_BY = 433,
     T_LAMBDA_OP = 434,
     T_LAMBDA_CP = 435,
     T_UNRESOLVED_OP = 436
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
#line 882 "hphp.tab.cpp"

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
#define YYLAST   16717

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  276
/* YYNRULES -- Number of rules.  */
#define YYNRULES  935
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1753

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   436

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   209,     2,   206,    51,    35,   210,
     201,   202,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   203,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   208,    34,     2,   207,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   204,    33,   205,    54,     2,     2,     2,
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
     194,   195,   196,   197,   198,   199,   200
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
     308,   309,   319,   320,   331,   337,   338,   352,   353,   359,
     363,   367,   370,   373,   376,   379,   382,   385,   389,   392,
     395,   399,   402,   403,   408,   418,   419,   420,   425,   428,
     429,   431,   432,   434,   435,   445,   446,   457,   458,   470,
     471,   481,   482,   493,   494,   503,   504,   514,   515,   523,
     524,   533,   534,   542,   543,   552,   554,   556,   558,   560,
     562,   565,   569,   573,   576,   579,   580,   583,   584,   587,
     588,   590,   594,   596,   600,   603,   604,   606,   609,   614,
     616,   621,   623,   628,   630,   635,   637,   642,   646,   652,
     656,   661,   666,   672,   678,   683,   684,   686,   688,   693,
     694,   700,   701,   704,   705,   709,   710,   718,   727,   734,
     737,   743,   750,   755,   756,   761,   767,   775,   782,   789,
     797,   807,   816,   823,   831,   837,   840,   845,   851,   855,
     856,   860,   865,   872,   878,   884,   891,   900,   908,   911,
     912,   914,   917,   920,   924,   929,   934,   938,   940,   942,
     945,   950,   954,   960,   962,   966,   969,   970,   973,   977,
     980,   981,   982,   987,   988,   994,   997,   998,  1009,  1010,
    1022,  1026,  1030,  1034,  1039,  1044,  1048,  1054,  1057,  1060,
    1061,  1068,  1074,  1079,  1083,  1085,  1087,  1091,  1096,  1098,
    1100,  1102,  1104,  1109,  1111,  1113,  1117,  1120,  1121,  1124,
    1125,  1127,  1131,  1133,  1135,  1137,  1139,  1143,  1148,  1153,
    1158,  1160,  1162,  1165,  1168,  1171,  1175,  1179,  1181,  1183,
    1185,  1187,  1191,  1193,  1197,  1199,  1201,  1203,  1204,  1206,
    1209,  1211,  1213,  1215,  1217,  1219,  1221,  1223,  1225,  1226,
    1228,  1230,  1232,  1236,  1242,  1244,  1248,  1254,  1259,  1263,
    1267,  1270,  1272,  1274,  1278,  1282,  1284,  1286,  1287,  1289,
    1292,  1297,  1301,  1308,  1311,  1315,  1322,  1324,  1326,  1328,
    1330,  1332,  1339,  1343,  1348,  1355,  1359,  1363,  1367,  1371,
    1375,  1379,  1383,  1387,  1391,  1395,  1399,  1403,  1406,  1409,
    1412,  1415,  1419,  1423,  1427,  1431,  1435,  1439,  1443,  1447,
    1451,  1455,  1459,  1463,  1467,  1471,  1475,  1479,  1483,  1486,
    1489,  1492,  1495,  1499,  1503,  1507,  1511,  1515,  1519,  1523,
    1527,  1531,  1535,  1541,  1546,  1548,  1551,  1554,  1557,  1560,
    1563,  1566,  1569,  1572,  1575,  1577,  1579,  1581,  1583,  1585,
    1589,  1592,  1594,  1600,  1601,  1602,  1614,  1615,  1628,  1629,
    1633,  1634,  1639,  1640,  1647,  1648,  1656,  1659,  1662,  1667,
    1669,  1671,  1677,  1681,  1687,  1691,  1694,  1695,  1698,  1699,
    1704,  1709,  1713,  1718,  1723,  1728,  1733,  1738,  1743,  1748,
    1753,  1758,  1763,  1765,  1767,  1769,  1771,  1775,  1778,  1782,
    1787,  1790,  1794,  1796,  1799,  1801,  1804,  1806,  1808,  1810,
    1812,  1814,  1816,  1821,  1826,  1829,  1838,  1849,  1852,  1854,
    1858,  1860,  1863,  1865,  1867,  1869,  1871,  1874,  1879,  1883,
    1887,  1892,  1894,  1897,  1902,  1905,  1912,  1913,  1915,  1920,
    1921,  1924,  1925,  1927,  1929,  1933,  1935,  1939,  1941,  1943,
    1947,  1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,
    1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,
    1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,
    2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,
    2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,
    2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,
    2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,
    2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,
    2109,  2111,  2116,  2118,  2120,  2122,  2124,  2126,  2128,  2130,
    2132,  2135,  2137,  2138,  2139,  2141,  2143,  2147,  2148,  2150,
    2152,  2154,  2156,  2158,  2160,  2162,  2164,  2166,  2168,  2170,
    2172,  2174,  2178,  2181,  2183,  2185,  2190,  2194,  2199,  2201,
    2203,  2205,  2207,  2211,  2215,  2219,  2223,  2227,  2231,  2235,
    2239,  2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,
    2279,  2282,  2285,  2288,  2291,  2295,  2299,  2303,  2307,  2311,
    2315,  2319,  2323,  2329,  2334,  2338,  2342,  2346,  2348,  2350,
    2352,  2354,  2358,  2362,  2366,  2369,  2370,  2372,  2373,  2375,
    2376,  2382,  2386,  2390,  2392,  2394,  2396,  2398,  2400,  2404,
    2407,  2409,  2411,  2413,  2415,  2417,  2419,  2422,  2425,  2430,
    2434,  2439,  2442,  2443,  2449,  2453,  2457,  2459,  2463,  2465,
    2468,  2469,  2475,  2479,  2482,  2483,  2487,  2488,  2493,  2496,
    2497,  2501,  2505,  2507,  2508,  2510,  2512,  2514,  2518,  2520,
    2522,  2526,  2530,  2533,  2538,  2541,  2546,  2548,  2550,  2552,
    2554,  2556,  2560,  2566,  2570,  2575,  2580,  2584,  2586,  2588,
    2590,  2592,  2596,  2602,  2607,  2611,  2613,  2615,  2619,  2623,
    2625,  2627,  2635,  2645,  2653,  2660,  2669,  2671,  2674,  2679,
    2684,  2686,  2688,  2693,  2695,  2696,  2698,  2701,  2703,  2705,
    2709,  2715,  2719,  2723,  2724,  2726,  2730,  2736,  2740,  2743,
    2747,  2754,  2755,  2757,  2762,  2765,  2766,  2772,  2776,  2780,
    2782,  2789,  2794,  2799,  2802,  2805,  2806,  2812,  2816,  2820,
    2822,  2825,  2826,  2832,  2836,  2840,  2842,  2845,  2846,  2849,
    2850,  2856,  2860,  2864,  2866,  2869,  2870,  2873,  2874,  2880,
    2884,  2888,  2890,  2893,  2896,  2898,  2901,  2903,  2908,  2912,
    2916,  2923,  2927,  2929,  2931,  2933,  2938,  2943,  2948,  2953,
    2958,  2963,  2966,  2969,  2974,  2977,  2980,  2982,  2986,  2990,
    2994,  2995,  2998,  3004,  3011,  3013,  3016,  3018,  3023,  3027,
    3028,  3030,  3034,  3037,  3041,  3043,  3045,  3046,  3047,  3050,
    3055,  3058,  3065,  3070,  3072,  3074,  3075,  3079,  3085,  3089,
    3091,  3094,  3095,  3100,  3103,  3106,  3108,  3110,  3112,  3114,
    3119,  3126,  3128,  3137,  3144,  3146
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     212,     0,    -1,    -1,   213,   214,    -1,   214,   215,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   471,    -1,   123,   201,   202,   203,    -1,   150,   225,
     203,    -1,    -1,   150,   225,   204,   216,   214,   205,    -1,
      -1,   150,   204,   217,   214,   205,    -1,   111,   219,   203,
      -1,   111,   105,   220,   203,    -1,   111,   106,   221,   203,
      -1,   230,   203,    -1,    77,    -1,   156,    -1,   157,    -1,
     159,    -1,   161,    -1,   160,    -1,   185,    -1,   186,    -1,
     188,    -1,   187,    -1,   189,    -1,   190,    -1,   191,    -1,
     192,    -1,   193,    -1,   194,    -1,   195,    -1,   196,    -1,
     197,    -1,   219,     9,   222,    -1,   222,    -1,   223,     9,
     223,    -1,   223,    -1,   224,     9,   224,    -1,   224,    -1,
     225,    -1,   153,   225,    -1,   225,    97,   218,    -1,   153,
     225,    97,   218,    -1,   225,    -1,   153,   225,    -1,   225,
      97,   218,    -1,   153,   225,    97,   218,    -1,   225,    -1,
     153,   225,    -1,   225,    97,   218,    -1,   153,   225,    97,
     218,    -1,   218,    -1,   225,   153,   218,    -1,   225,    -1,
     150,   153,   225,    -1,   153,   225,    -1,   226,    -1,   226,
     474,    -1,   226,   474,    -1,   230,     9,   472,    14,   407,
      -1,   106,   472,    14,   407,    -1,   231,   232,    -1,    -1,
     233,    -1,   250,    -1,   257,    -1,   262,    -1,   204,   231,
     205,    -1,    70,   333,   233,   284,   286,    -1,    70,   333,
      30,   231,   285,   287,    73,   203,    -1,    -1,    89,   333,
     234,   278,    -1,    -1,    88,   235,   233,    89,   333,   203,
      -1,    -1,    91,   201,   335,   203,   335,   203,   335,   202,
     236,   276,    -1,    -1,    98,   333,   237,   281,    -1,   102,
     203,    -1,   102,   342,   203,    -1,   104,   203,    -1,   104,
     342,   203,    -1,   107,   203,    -1,   107,   342,   203,    -1,
      27,   102,   203,    -1,   112,   294,   203,    -1,   118,   296,
     203,    -1,    87,   334,   203,    -1,   120,   201,   468,   202,
     203,    -1,   203,    -1,    81,    -1,    82,    -1,    -1,    93,
     201,   342,    97,   275,   274,   202,   238,   277,    -1,    -1,
      93,   201,   342,    28,    97,   275,   274,   202,   239,   277,
      -1,    95,   201,   280,   202,   279,    -1,    -1,   108,   242,
     109,   201,   400,    79,   202,   204,   231,   205,   244,   240,
     247,    -1,    -1,   108,   242,   167,   241,   245,    -1,   110,
     342,   203,    -1,   103,   218,   203,    -1,   342,   203,    -1,
     336,   203,    -1,   337,   203,    -1,   338,   203,    -1,   339,
     203,    -1,   340,   203,    -1,   107,   339,   203,    -1,   341,
     203,    -1,   370,   203,    -1,   107,   369,   203,    -1,   218,
      30,    -1,    -1,   204,   243,   231,   205,    -1,   244,   109,
     201,   400,    79,   202,   204,   231,   205,    -1,    -1,    -1,
     204,   246,   231,   205,    -1,   167,   245,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   249,   248,   473,   251,   201,
     290,   202,   478,   322,    -1,    -1,   326,   249,   248,   473,
     252,   201,   290,   202,   478,   322,    -1,    -1,   427,   325,
     249,   248,   473,   253,   201,   290,   202,   478,   322,    -1,
      -1,   160,   218,   255,    30,   485,   470,   204,   297,   205,
      -1,    -1,   427,   160,   218,   256,    30,   485,   470,   204,
     297,   205,    -1,    -1,   268,   265,   258,   269,   270,   204,
     300,   205,    -1,    -1,   427,   268,   265,   259,   269,   270,
     204,   300,   205,    -1,    -1,   125,   266,   260,   271,   204,
     300,   205,    -1,    -1,   427,   125,   266,   261,   271,   204,
     300,   205,    -1,    -1,   162,   267,   263,   270,   204,   300,
     205,    -1,    -1,   427,   162,   267,   264,   270,   204,   300,
     205,    -1,   473,    -1,   154,    -1,   473,    -1,   473,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   400,    -1,    -1,
     127,   272,    -1,    -1,   126,   272,    -1,    -1,   400,    -1,
     272,     9,   400,    -1,   400,    -1,   273,     9,   400,    -1,
     130,   275,    -1,    -1,   435,    -1,    35,   435,    -1,   131,
     201,   449,   202,    -1,   233,    -1,    30,   231,    92,   203,
      -1,   233,    -1,    30,   231,    94,   203,    -1,   233,    -1,
      30,   231,    90,   203,    -1,   233,    -1,    30,   231,    96,
     203,    -1,   218,    14,   407,    -1,   280,     9,   218,    14,
     407,    -1,   204,   282,   205,    -1,   204,   203,   282,   205,
      -1,    30,   282,    99,   203,    -1,    30,   203,   282,    99,
     203,    -1,   282,   100,   342,   283,   231,    -1,   282,   101,
     283,   231,    -1,    -1,    30,    -1,   203,    -1,   284,    71,
     333,   233,    -1,    -1,   285,    71,   333,    30,   231,    -1,
      -1,    72,   233,    -1,    -1,    72,    30,   231,    -1,    -1,
     289,     9,   428,   328,   486,   163,    79,    -1,   289,     9,
     428,   328,   486,    35,   163,    79,    -1,   289,     9,   428,
     328,   486,   163,    -1,   289,   412,    -1,   428,   328,   486,
     163,    79,    -1,   428,   328,   486,    35,   163,    79,    -1,
     428,   328,   486,   163,    -1,    -1,   428,   328,   486,    79,
      -1,   428,   328,   486,    35,    79,    -1,   428,   328,   486,
      35,    79,    14,   407,    -1,   428,   328,   486,    79,    14,
     407,    -1,   289,     9,   428,   328,   486,    79,    -1,   289,
       9,   428,   328,   486,    35,    79,    -1,   289,     9,   428,
     328,   486,    35,    79,    14,   407,    -1,   289,     9,   428,
     328,   486,    79,    14,   407,    -1,   291,     9,   428,   486,
     163,    79,    -1,   291,     9,   428,   486,    35,   163,    79,
      -1,   291,     9,   428,   486,   163,    -1,   291,   412,    -1,
     428,   486,   163,    79,    -1,   428,   486,    35,   163,    79,
      -1,   428,   486,   163,    -1,    -1,   428,   486,    79,    -1,
     428,   486,    35,    79,    -1,   428,   486,    35,    79,    14,
     407,    -1,   428,   486,    79,    14,   407,    -1,   291,     9,
     428,   486,    79,    -1,   291,     9,   428,   486,    35,    79,
      -1,   291,     9,   428,   486,    35,    79,    14,   407,    -1,
     291,     9,   428,   486,    79,    14,   407,    -1,   293,   412,
      -1,    -1,   342,    -1,    35,   435,    -1,   163,   342,    -1,
     293,     9,   342,    -1,   293,     9,   163,   342,    -1,   293,
       9,    35,   435,    -1,   294,     9,   295,    -1,   295,    -1,
      79,    -1,   206,   435,    -1,   206,   204,   342,   205,    -1,
     296,     9,    79,    -1,   296,     9,    79,    14,   407,    -1,
      79,    -1,    79,    14,   407,    -1,   297,   298,    -1,    -1,
     299,   203,    -1,   472,    14,   407,    -1,   300,   301,    -1,
      -1,    -1,   324,   302,   330,   203,    -1,    -1,   326,   485,
     303,   330,   203,    -1,   331,   203,    -1,    -1,   325,   249,
     248,   473,   201,   304,   288,   202,   478,   323,    -1,    -1,
     427,   325,   249,   248,   473,   201,   305,   288,   202,   478,
     323,    -1,   156,   310,   203,    -1,   157,   316,   203,    -1,
     159,   318,   203,    -1,     4,   126,   400,   203,    -1,     4,
     127,   400,   203,    -1,   111,   273,   203,    -1,   111,   273,
     204,   306,   205,    -1,   306,   307,    -1,   306,   308,    -1,
      -1,   229,   149,   218,   164,   273,   203,    -1,   309,    97,
     325,   218,   203,    -1,   309,    97,   326,   203,    -1,   229,
     149,   218,    -1,   218,    -1,   311,    -1,   310,     9,   311,
      -1,   312,   397,   314,   315,    -1,   154,    -1,   132,    -1,
     400,    -1,   119,    -1,   160,   204,   313,   205,    -1,   133,
      -1,   406,    -1,   313,     9,   406,    -1,    14,   407,    -1,
      -1,    55,   161,    -1,    -1,   317,    -1,   316,     9,   317,
      -1,   158,    -1,   319,    -1,   218,    -1,   122,    -1,   201,
     320,   202,    -1,   201,   320,   202,    49,    -1,   201,   320,
     202,    29,    -1,   201,   320,   202,    46,    -1,   319,    -1,
     321,    -1,   321,    49,    -1,   321,    29,    -1,   321,    46,
      -1,   320,     9,   320,    -1,   320,    33,   320,    -1,   218,
      -1,   154,    -1,   158,    -1,   203,    -1,   204,   231,   205,
      -1,   203,    -1,   204,   231,   205,    -1,   326,    -1,   119,
      -1,   326,    -1,    -1,   327,    -1,   326,   327,    -1,   113,
      -1,   114,    -1,   115,    -1,   118,    -1,   117,    -1,   116,
      -1,   183,    -1,   329,    -1,    -1,   113,    -1,   114,    -1,
     115,    -1,   330,     9,    79,    -1,   330,     9,    79,    14,
     407,    -1,    79,    -1,    79,    14,   407,    -1,   331,     9,
     472,    14,   407,    -1,   106,   472,    14,   407,    -1,   201,
     332,   202,    -1,    68,   402,   405,    -1,    67,   342,    -1,
     389,    -1,   361,    -1,   201,   342,   202,    -1,   334,     9,
     342,    -1,   342,    -1,   334,    -1,    -1,    27,    -1,    27,
     342,    -1,    27,   342,   130,   342,    -1,   435,    14,   336,
      -1,   131,   201,   449,   202,    14,   336,    -1,    28,   342,
      -1,   435,    14,   339,    -1,   131,   201,   449,   202,    14,
     339,    -1,   343,    -1,   435,    -1,   332,    -1,   439,    -1,
     438,    -1,   131,   201,   449,   202,    14,   342,    -1,   435,
      14,   342,    -1,   435,    14,    35,   435,    -1,   435,    14,
      35,    68,   402,   405,    -1,   435,    26,   342,    -1,   435,
      25,   342,    -1,   435,    24,   342,    -1,   435,    23,   342,
      -1,   435,    22,   342,    -1,   435,    21,   342,    -1,   435,
      20,   342,    -1,   435,    19,   342,    -1,   435,    18,   342,
      -1,   435,    17,   342,    -1,   435,    16,   342,    -1,   435,
      15,   342,    -1,   435,    64,    -1,    64,   435,    -1,   435,
      63,    -1,    63,   435,    -1,   342,    31,   342,    -1,   342,
      32,   342,    -1,   342,    10,   342,    -1,   342,    12,   342,
      -1,   342,    11,   342,    -1,   342,    33,   342,    -1,   342,
      35,   342,    -1,   342,    34,   342,    -1,   342,    48,   342,
      -1,   342,    46,   342,    -1,   342,    47,   342,    -1,   342,
      49,   342,    -1,   342,    50,   342,    -1,   342,    65,   342,
      -1,   342,    51,   342,    -1,   342,    45,   342,    -1,   342,
      44,   342,    -1,    46,   342,    -1,    47,   342,    -1,    52,
     342,    -1,    54,   342,    -1,   342,    37,   342,    -1,   342,
      36,   342,    -1,   342,    39,   342,    -1,   342,    38,   342,
      -1,   342,    40,   342,    -1,   342,    43,   342,    -1,   342,
      41,   342,    -1,   342,    42,   342,    -1,   342,    53,   402,
      -1,   201,   343,   202,    -1,   342,    29,   342,    30,   342,
      -1,   342,    29,    30,   342,    -1,   467,    -1,    62,   342,
      -1,    61,   342,    -1,    60,   342,    -1,    59,   342,    -1,
      58,   342,    -1,    57,   342,    -1,    56,   342,    -1,    69,
     403,    -1,    55,   342,    -1,   409,    -1,   360,    -1,   359,
      -1,   362,    -1,   363,    -1,   207,   404,   207,    -1,    13,
     342,    -1,   367,    -1,   111,   201,   388,   412,   202,    -1,
      -1,    -1,   249,   248,   201,   346,   290,   202,   478,   344,
     204,   231,   205,    -1,    -1,   326,   249,   248,   201,   347,
     290,   202,   478,   344,   204,   231,   205,    -1,    -1,    79,
     349,   353,    -1,    -1,   183,    79,   350,   353,    -1,    -1,
     198,   351,   290,   199,   478,   353,    -1,    -1,   183,   198,
     352,   290,   199,   478,   353,    -1,     8,   342,    -1,     8,
     339,    -1,     8,   204,   231,   205,    -1,    86,    -1,   469,
      -1,   355,     9,   354,   130,   342,    -1,   354,   130,   342,
      -1,   356,     9,   354,   130,   407,    -1,   354,   130,   407,
      -1,   355,   411,    -1,    -1,   356,   411,    -1,    -1,   174,
     201,   357,   202,    -1,   132,   201,   450,   202,    -1,    66,
     450,   208,    -1,   400,   204,   452,   205,    -1,   176,   201,
     456,   202,    -1,   177,   201,   456,   202,    -1,   175,   201,
     457,   202,    -1,   176,   201,   460,   202,    -1,   177,   201,
     460,   202,    -1,   175,   201,   461,   202,    -1,   400,   204,
     454,   205,    -1,   367,    66,   445,   208,    -1,   368,    66,
     445,   208,    -1,   360,    -1,   469,    -1,   438,    -1,    86,
      -1,   201,   343,   202,    -1,   371,   372,    -1,   435,    14,
     369,    -1,   184,    79,   187,   342,    -1,   373,   384,    -1,
     373,   384,   387,    -1,   384,    -1,   384,   387,    -1,   374,
      -1,   373,   374,    -1,   375,    -1,   376,    -1,   377,    -1,
     378,    -1,   379,    -1,   380,    -1,   184,    79,   187,   342,
      -1,   191,    79,    14,   342,    -1,   185,   342,    -1,   186,
      79,   187,   342,   188,   342,   189,   342,    -1,   186,    79,
     187,   342,   188,   342,   189,   342,   190,    79,    -1,   192,
     381,    -1,   382,    -1,   381,     9,   382,    -1,   342,    -1,
     342,   383,    -1,   193,    -1,   194,    -1,   385,    -1,   386,
      -1,   195,   342,    -1,   196,   342,   197,   342,    -1,   190,
      79,   372,    -1,   388,     9,    79,    -1,   388,     9,    35,
      79,    -1,    79,    -1,    35,    79,    -1,   168,   154,   390,
     169,    -1,   392,    50,    -1,   392,   169,   393,   168,    50,
     391,    -1,    -1,   154,    -1,   392,   394,    14,   395,    -1,
      -1,   393,   396,    -1,    -1,   154,    -1,   155,    -1,   204,
     342,   205,    -1,   155,    -1,   204,   342,   205,    -1,   389,
      -1,   398,    -1,   397,    30,   398,    -1,   397,    47,   398,
      -1,   218,    -1,    69,    -1,   105,    -1,   106,    -1,   107,
      -1,    27,    -1,    28,    -1,   108,    -1,   109,    -1,   167,
      -1,   110,    -1,    70,    -1,    71,    -1,    73,    -1,    72,
      -1,    89,    -1,    90,    -1,    88,    -1,    91,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,    53,
      -1,    97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,
      -1,   102,    -1,   104,    -1,   103,    -1,    87,    -1,    13,
      -1,   124,    -1,   125,    -1,   126,    -1,   127,    -1,    68,
      -1,    67,    -1,   119,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   150,    -1,   111,    -1,   112,
      -1,   121,    -1,   122,    -1,   123,    -1,   118,    -1,   117,
      -1,   116,    -1,   115,    -1,   114,    -1,   113,    -1,   183,
      -1,   120,    -1,   131,    -1,   132,    -1,    10,    -1,    12,
      -1,    11,    -1,   134,    -1,   136,    -1,   135,    -1,   137,
      -1,   138,    -1,   152,    -1,   151,    -1,   182,    -1,   162,
      -1,   165,    -1,   164,    -1,   178,    -1,   180,    -1,   174,
      -1,   228,   201,   292,   202,    -1,   229,    -1,   154,    -1,
     400,    -1,   118,    -1,   443,    -1,   400,    -1,   118,    -1,
     447,    -1,   201,   202,    -1,   333,    -1,    -1,    -1,    85,
      -1,   464,    -1,   201,   292,   202,    -1,    -1,    74,    -1,
      75,    -1,    76,    -1,    86,    -1,   137,    -1,   138,    -1,
     152,    -1,   134,    -1,   165,    -1,   135,    -1,   136,    -1,
     151,    -1,   182,    -1,   145,    85,   146,    -1,   145,   146,
      -1,   406,    -1,   227,    -1,   132,   201,   410,   202,    -1,
      66,   410,   208,    -1,   174,   201,   358,   202,    -1,   408,
      -1,   366,    -1,   364,    -1,   365,    -1,   201,   407,   202,
      -1,   407,    31,   407,    -1,   407,    32,   407,    -1,   407,
      10,   407,    -1,   407,    12,   407,    -1,   407,    11,   407,
      -1,   407,    33,   407,    -1,   407,    35,   407,    -1,   407,
      34,   407,    -1,   407,    48,   407,    -1,   407,    46,   407,
      -1,   407,    47,   407,    -1,   407,    49,   407,    -1,   407,
      50,   407,    -1,   407,    51,   407,    -1,   407,    45,   407,
      -1,   407,    44,   407,    -1,   407,    65,   407,    -1,    52,
     407,    -1,    54,   407,    -1,    46,   407,    -1,    47,   407,
      -1,   407,    37,   407,    -1,   407,    36,   407,    -1,   407,
      39,   407,    -1,   407,    38,   407,    -1,   407,    40,   407,
      -1,   407,    43,   407,    -1,   407,    41,   407,    -1,   407,
      42,   407,    -1,   407,    29,   407,    30,   407,    -1,   407,
      29,    30,   407,    -1,   229,   149,   218,    -1,   154,   149,
     218,    -1,   229,   149,   124,    -1,   227,    -1,    78,    -1,
     469,    -1,   406,    -1,   209,   464,   209,    -1,   210,   464,
     210,    -1,   145,   464,   146,    -1,   413,   411,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   413,     9,   407,   130,
     407,    -1,   413,     9,   407,    -1,   407,   130,   407,    -1,
     407,    -1,    74,    -1,    75,    -1,    76,    -1,    86,    -1,
     145,    85,   146,    -1,   145,   146,    -1,    74,    -1,    75,
      -1,    76,    -1,   218,    -1,   414,    -1,   218,    -1,    46,
     415,    -1,    47,   415,    -1,   132,   201,   417,   202,    -1,
      66,   417,   208,    -1,   174,   201,   420,   202,    -1,   418,
     411,    -1,    -1,   418,     9,   416,   130,   416,    -1,   418,
       9,   416,    -1,   416,   130,   416,    -1,   416,    -1,   419,
       9,   416,    -1,   416,    -1,   421,   411,    -1,    -1,   421,
       9,   354,   130,   416,    -1,   354,   130,   416,    -1,   419,
     411,    -1,    -1,   201,   422,   202,    -1,    -1,   424,     9,
     218,   423,    -1,   218,   423,    -1,    -1,   426,   424,   411,
      -1,    45,   425,    44,    -1,   427,    -1,    -1,   128,    -1,
     129,    -1,   218,    -1,   204,   342,   205,    -1,   430,    -1,
     442,    -1,    66,   445,   208,    -1,   204,   342,   205,    -1,
     436,   432,    -1,   201,   332,   202,   432,    -1,   448,   432,
      -1,   201,   332,   202,   432,    -1,   442,    -1,   399,    -1,
     440,    -1,   441,    -1,   433,    -1,   435,   429,   431,    -1,
     201,   332,   202,   429,   431,    -1,   401,   149,   442,    -1,
     437,   201,   292,   202,    -1,   438,   201,   292,   202,    -1,
     201,   435,   202,    -1,   399,    -1,   440,    -1,   441,    -1,
     433,    -1,   435,   429,   430,    -1,   201,   332,   202,   429,
     430,    -1,   437,   201,   292,   202,    -1,   201,   435,   202,
      -1,   442,    -1,   433,    -1,   201,   435,   202,    -1,   201,
     439,   202,    -1,   345,    -1,   348,    -1,   435,   429,   431,
     474,   201,   292,   202,    -1,   201,   332,   202,   429,   431,
     474,   201,   292,   202,    -1,   401,   149,   218,   474,   201,
     292,   202,    -1,   401,   149,   442,   201,   292,   202,    -1,
     401,   149,   204,   342,   205,   201,   292,   202,    -1,   443,
      -1,   446,   443,    -1,   443,    66,   445,   208,    -1,   443,
     204,   342,   205,    -1,   444,    -1,    79,    -1,   206,   204,
     342,   205,    -1,   342,    -1,    -1,   206,    -1,   446,   206,
      -1,   442,    -1,   434,    -1,   447,   429,   431,    -1,   201,
     332,   202,   429,   431,    -1,   401,   149,   442,    -1,   201,
     435,   202,    -1,    -1,   434,    -1,   447,   429,   430,    -1,
     201,   332,   202,   429,   430,    -1,   201,   435,   202,    -1,
     449,     9,    -1,   449,     9,   435,    -1,   449,     9,   131,
     201,   449,   202,    -1,    -1,   435,    -1,   131,   201,   449,
     202,    -1,   451,   411,    -1,    -1,   451,     9,   342,   130,
     342,    -1,   451,     9,   342,    -1,   342,   130,   342,    -1,
     342,    -1,   451,     9,   342,   130,    35,   435,    -1,   451,
       9,    35,   435,    -1,   342,   130,    35,   435,    -1,    35,
     435,    -1,   453,   411,    -1,    -1,   453,     9,   342,   130,
     342,    -1,   453,     9,   342,    -1,   342,   130,   342,    -1,
     342,    -1,   455,   411,    -1,    -1,   455,     9,   407,   130,
     407,    -1,   455,     9,   407,    -1,   407,   130,   407,    -1,
     407,    -1,   458,   411,    -1,    -1,   459,   411,    -1,    -1,
     458,     9,   342,   130,   342,    -1,   342,   130,   342,    -1,
     459,     9,   342,    -1,   342,    -1,   462,   411,    -1,    -1,
     463,   411,    -1,    -1,   462,     9,   407,   130,   407,    -1,
     407,   130,   407,    -1,   463,     9,   407,    -1,   407,    -1,
     464,   465,    -1,   464,    85,    -1,   465,    -1,    85,   465,
      -1,    79,    -1,    79,    66,   466,   208,    -1,    79,   429,
     218,    -1,   147,   342,   205,    -1,   147,    78,    66,   342,
     208,   205,    -1,   148,   435,   205,    -1,   218,    -1,    80,
      -1,    79,    -1,   121,   201,   468,   202,    -1,   122,   201,
     435,   202,    -1,   122,   201,   343,   202,    -1,   122,   201,
     439,   202,    -1,   122,   201,   438,   202,    -1,   122,   201,
     332,   202,    -1,     7,   342,    -1,     6,   342,    -1,     5,
     201,   342,   202,    -1,     4,   342,    -1,     3,   342,    -1,
     435,    -1,   468,     9,   435,    -1,   401,   149,   218,    -1,
     401,   149,   124,    -1,    -1,    97,   485,    -1,   178,   473,
      14,   485,   203,    -1,   180,   473,   470,    14,   485,   203,
      -1,   218,    -1,   485,   218,    -1,   218,    -1,   218,   170,
     479,   171,    -1,   170,   476,   171,    -1,    -1,   485,    -1,
     475,     9,   485,    -1,   475,   411,    -1,   475,     9,   163,
      -1,   476,    -1,   163,    -1,    -1,    -1,    30,   485,    -1,
     479,     9,   480,   218,    -1,   480,   218,    -1,   479,     9,
     480,   218,    97,   485,    -1,   480,   218,    97,   485,    -1,
      46,    -1,    47,    -1,    -1,    86,   130,   485,    -1,   229,
     149,   218,   130,   485,    -1,   482,     9,   481,    -1,   481,
      -1,   482,   411,    -1,    -1,   174,   201,   483,   202,    -1,
      29,   485,    -1,    55,   485,    -1,   229,    -1,   132,    -1,
     133,    -1,   484,    -1,   132,   170,   485,   171,    -1,   132,
     170,   485,     9,   485,   171,    -1,   154,    -1,   201,   105,
     201,   477,   202,    30,   485,   202,    -1,   201,   485,     9,
     475,   411,   202,    -1,   485,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   737,   737,   737,   746,   748,   751,   752,   753,   754,
     755,   756,   757,   760,   762,   762,   764,   764,   766,   767,
     769,   771,   776,   777,   778,   779,   780,   781,   782,   783,
     784,   785,   786,   787,   788,   789,   790,   791,   792,   793,
     794,   798,   800,   804,   806,   810,   812,   816,   817,   818,
     819,   824,   825,   826,   827,   832,   833,   834,   835,   840,
     841,   845,   846,   848,   851,   857,   864,   871,   875,   881,
     883,   886,   887,   888,   889,   892,   893,   897,   902,   902,
     908,   908,   915,   914,   920,   920,   925,   926,   927,   928,
     929,   930,   931,   932,   933,   934,   935,   936,   937,   938,
     942,   940,   949,   947,   954,   962,   956,   966,   964,   968,
     969,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   991,   991,   996,  1002,  1006,  1006,  1014,  1015,
    1019,  1020,  1024,  1029,  1028,  1041,  1039,  1053,  1051,  1067,
    1066,  1075,  1073,  1085,  1084,  1103,  1101,  1120,  1119,  1128,
    1126,  1138,  1137,  1149,  1147,  1160,  1161,  1165,  1168,  1171,
    1172,  1173,  1176,  1177,  1180,  1182,  1185,  1186,  1189,  1190,
    1193,  1194,  1198,  1199,  1204,  1205,  1208,  1209,  1210,  1214,
    1215,  1219,  1220,  1224,  1225,  1229,  1230,  1235,  1236,  1241,
    1242,  1243,  1244,  1247,  1250,  1252,  1255,  1256,  1260,  1262,
    1265,  1268,  1271,  1272,  1275,  1276,  1280,  1286,  1292,  1299,
    1301,  1306,  1311,  1317,  1321,  1325,  1329,  1334,  1339,  1344,
    1349,  1355,  1364,  1369,  1374,  1380,  1382,  1386,  1390,  1395,
    1399,  1402,  1405,  1409,  1413,  1417,  1421,  1426,  1434,  1436,
    1439,  1440,  1441,  1442,  1444,  1446,  1451,  1452,  1455,  1456,
    1457,  1461,  1462,  1464,  1465,  1469,  1471,  1474,  1478,  1484,
    1486,  1489,  1489,  1493,  1492,  1496,  1500,  1498,  1513,  1510,
    1523,  1525,  1527,  1529,  1531,  1533,  1535,  1539,  1540,  1541,
    1544,  1550,  1553,  1559,  1562,  1567,  1569,  1574,  1579,  1583,
    1584,  1590,  1591,  1593,  1597,  1598,  1603,  1604,  1608,  1609,
    1613,  1615,  1621,  1626,  1627,  1629,  1633,  1634,  1635,  1636,
    1640,  1641,  1642,  1643,  1644,  1645,  1647,  1652,  1655,  1656,
    1660,  1661,  1665,  1666,  1669,  1670,  1673,  1674,  1677,  1678,
    1682,  1683,  1684,  1685,  1686,  1687,  1688,  1692,  1693,  1696,
    1697,  1698,  1701,  1703,  1705,  1706,  1709,  1711,  1716,  1717,
    1719,  1720,  1721,  1724,  1728,  1729,  1733,  1734,  1738,  1739,
    1740,  1744,  1748,  1753,  1757,  1761,  1766,  1767,  1768,  1769,
    1770,  1774,  1776,  1777,  1778,  1781,  1782,  1783,  1784,  1785,
    1786,  1787,  1788,  1789,  1790,  1791,  1792,  1793,  1794,  1795,
    1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,
    1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,
    1816,  1817,  1818,  1819,  1820,  1821,  1822,  1823,  1825,  1826,
    1828,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,
    1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,
    1849,  1850,  1854,  1858,  1863,  1862,  1877,  1875,  1892,  1892,
    1908,  1907,  1925,  1925,  1941,  1940,  1961,  1962,  1963,  1968,
    1970,  1974,  1978,  1984,  1988,  1994,  1996,  2000,  2002,  2006,
    2010,  2011,  2015,  2022,  2023,  2027,  2031,  2033,  2038,  2043,
    2050,  2052,  2057,  2058,  2059,  2060,  2062,  2066,  2070,  2074,
    2078,  2080,  2082,  2084,  2089,  2090,  2095,  2096,  2097,  2098,
    2099,  2100,  2104,  2108,  2112,  2116,  2121,  2126,  2130,  2131,
    2135,  2136,  2140,  2141,  2145,  2146,  2150,  2154,  2158,  2162,
    2163,  2164,  2165,  2169,  2175,  2184,  2197,  2198,  2201,  2204,
    2207,  2208,  2211,  2215,  2218,  2221,  2228,  2229,  2233,  2234,
    2236,  2240,  2241,  2242,  2243,  2244,  2245,  2246,  2247,  2248,
    2249,  2250,  2251,  2252,  2253,  2254,  2255,  2256,  2257,  2258,
    2259,  2260,  2261,  2262,  2263,  2264,  2265,  2266,  2267,  2268,
    2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,  2277,  2278,
    2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,  2287,  2288,
    2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,  2297,  2298,
    2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,  2307,  2308,
    2309,  2310,  2311,  2312,  2313,  2314,  2315,  2316,  2317,  2318,
    2319,  2323,  2328,  2329,  2332,  2333,  2334,  2338,  2339,  2340,
    2344,  2345,  2346,  2350,  2351,  2352,  2355,  2357,  2361,  2362,
    2363,  2364,  2366,  2367,  2368,  2369,  2370,  2371,  2372,  2373,
    2374,  2375,  2378,  2383,  2384,  2385,  2387,  2388,  2390,  2391,
    2392,  2393,  2394,  2395,  2397,  2399,  2401,  2403,  2405,  2406,
    2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,
    2417,  2418,  2419,  2420,  2421,  2423,  2425,  2427,  2429,  2430,
    2433,  2434,  2438,  2440,  2444,  2447,  2450,  2456,  2457,  2458,
    2459,  2460,  2461,  2462,  2467,  2469,  2473,  2474,  2477,  2478,
    2482,  2485,  2487,  2489,  2493,  2494,  2495,  2496,  2498,  2501,
    2505,  2506,  2507,  2508,  2511,  2512,  2513,  2514,  2515,  2517,
    2518,  2523,  2525,  2528,  2531,  2533,  2535,  2538,  2540,  2544,
    2546,  2549,  2552,  2558,  2560,  2563,  2564,  2569,  2572,  2576,
    2576,  2581,  2584,  2585,  2589,  2590,  2594,  2595,  2599,  2600,
    2604,  2605,  2609,  2610,  2615,  2617,  2622,  2623,  2624,  2625,
    2626,  2627,  2629,  2632,  2635,  2637,  2639,  2643,  2644,  2645,
    2646,  2647,  2649,  2652,  2654,  2658,  2659,  2660,  2664,  2668,
    2669,  2673,  2676,  2683,  2687,  2691,  2698,  2699,  2704,  2706,
    2707,  2710,  2711,  2714,  2715,  2719,  2720,  2724,  2725,  2726,
    2729,  2732,  2735,  2738,  2739,  2740,  2742,  2745,  2749,  2750,
    2751,  2753,  2754,  2755,  2759,  2761,  2764,  2766,  2767,  2768,
    2769,  2772,  2774,  2775,  2779,  2781,  2784,  2786,  2787,  2788,
    2792,  2794,  2797,  2800,  2802,  2804,  2808,  2810,  2813,  2815,
    2818,  2820,  2823,  2824,  2828,  2830,  2833,  2835,  2838,  2841,
    2845,  2847,  2851,  2852,  2854,  2855,  2861,  2862,  2864,  2866,
    2868,  2870,  2873,  2874,  2875,  2879,  2880,  2881,  2882,  2883,
    2884,  2885,  2886,  2887,  2888,  2889,  2893,  2894,  2898,  2900,
    2908,  2910,  2914,  2918,  2925,  2926,  2932,  2933,  2940,  2943,
    2947,  2950,  2955,  2960,  2962,  2963,  2964,  2968,  2969,  2973,
    2975,  2976,  2979,  2984,  2985,  2986,  2990,  2993,  3002,  3004,
    3008,  3011,  3014,  3022,  3025,  3028,  3029,  3032,  3035,  3036,
    3039,  3043,  3047,  3053,  3063,  3064
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
  "T_INLINE_HTML", "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
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
  "lambda_or_closure_with_parens", "lambda_or_closure",
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
     436,    40,    41,    59,   123,   125,    36,    96,    93,    34,
      39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   211,   213,   212,   214,   214,   215,   215,   215,   215,
     215,   215,   215,   215,   216,   215,   217,   215,   215,   215,
     215,   215,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   219,   219,   220,   220,   221,   221,   222,   222,   222,
     222,   223,   223,   223,   223,   224,   224,   224,   224,   225,
     225,   226,   226,   226,   227,   228,   229,   230,   230,   231,
     231,   232,   232,   232,   232,   233,   233,   233,   234,   233,
     235,   233,   236,   233,   237,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     238,   233,   239,   233,   233,   240,   233,   241,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   243,   242,   244,   244,   246,   245,   247,   247,
     248,   248,   249,   251,   250,   252,   250,   253,   250,   255,
     254,   256,   254,   258,   257,   259,   257,   260,   257,   261,
     257,   263,   262,   264,   262,   265,   265,   266,   267,   268,
     268,   268,   268,   268,   269,   269,   270,   270,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   275,   276,
     276,   277,   277,   278,   278,   279,   279,   280,   280,   281,
     281,   281,   281,   282,   282,   282,   283,   283,   284,   284,
     285,   285,   286,   286,   287,   287,   288,   288,   288,   288,
     288,   288,   288,   288,   289,   289,   289,   289,   289,   289,
     289,   289,   290,   290,   290,   290,   290,   290,   290,   290,
     291,   291,   291,   291,   291,   291,   291,   291,   292,   292,
     293,   293,   293,   293,   293,   293,   294,   294,   295,   295,
     295,   296,   296,   296,   296,   297,   297,   298,   299,   300,
     300,   302,   301,   303,   301,   301,   304,   301,   305,   301,
     301,   301,   301,   301,   301,   301,   301,   306,   306,   306,
     307,   308,   308,   309,   309,   310,   310,   311,   311,   312,
     312,   312,   312,   312,   313,   313,   314,   314,   315,   315,
     316,   316,   317,   318,   318,   318,   319,   319,   319,   319,
     320,   320,   320,   320,   320,   320,   320,   321,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   326,   326,
     327,   327,   327,   327,   327,   327,   327,   328,   328,   329,
     329,   329,   330,   330,   330,   330,   331,   331,   332,   332,
     332,   332,   332,   333,   334,   334,   335,   335,   336,   336,
     336,   337,   338,   339,   340,   341,   342,   342,   342,   342,
     342,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   344,   344,   346,   345,   347,   345,   349,   348,
     350,   348,   351,   348,   352,   348,   353,   353,   353,   354,
     354,   355,   355,   356,   356,   357,   357,   358,   358,   359,
     360,   360,   361,   362,   362,   363,   364,   364,   365,   366,
     367,   367,   368,   368,   368,   368,   368,   369,   370,   371,
     372,   372,   372,   372,   373,   373,   374,   374,   374,   374,
     374,   374,   375,   376,   377,   378,   379,   380,   381,   381,
     382,   382,   383,   383,   384,   384,   385,   386,   387,   388,
     388,   388,   388,   389,   390,   390,   391,   391,   392,   392,
     393,   393,   394,   395,   395,   396,   396,   396,   397,   397,
     397,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   399,   400,   400,   401,   401,   401,   402,   402,   402,
     403,   403,   403,   404,   404,   404,   405,   405,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   408,   408,   408,   409,   409,   409,
     409,   409,   409,   409,   410,   410,   411,   411,   412,   412,
     413,   413,   413,   413,   414,   414,   414,   414,   414,   414,
     415,   415,   415,   415,   416,   416,   416,   416,   416,   416,
     416,   417,   417,   418,   418,   418,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   424,   424,   426,
     425,   427,   428,   428,   429,   429,   430,   430,   431,   431,
     432,   432,   433,   433,   434,   434,   435,   435,   435,   435,
     435,   435,   435,   435,   435,   435,   435,   436,   436,   436,
     436,   436,   436,   436,   436,   437,   437,   437,   438,   439,
     439,   440,   440,   441,   441,   441,   442,   442,   443,   443,
     443,   444,   444,   445,   445,   446,   446,   447,   447,   447,
     447,   447,   447,   448,   448,   448,   448,   448,   449,   449,
     449,   449,   449,   449,   450,   450,   451,   451,   451,   451,
     451,   451,   451,   451,   452,   452,   453,   453,   453,   453,
     454,   454,   455,   455,   455,   455,   456,   456,   457,   457,
     458,   458,   459,   459,   460,   460,   461,   461,   462,   462,
     463,   463,   464,   464,   464,   464,   465,   465,   465,   465,
     465,   465,   466,   466,   466,   467,   467,   467,   467,   467,
     467,   467,   467,   467,   467,   467,   468,   468,   469,   469,
     470,   470,   471,   471,   472,   472,   473,   473,   474,   474,
     475,   475,   476,   477,   477,   477,   477,   478,   478,   479,
     479,   479,   479,   480,   480,   480,   481,   481,   482,   482,
     483,   483,   484,   485,   485,   485,   485,   485,   485,   485,
     485,   485,   485,   485,   486,   486
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
       2,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     0,     4,     9,     0,     0,     4,     2,     0,
       1,     0,     1,     0,     9,     0,    10,     0,    11,     0,
       9,     0,    10,     0,     8,     0,     9,     0,     7,     0,
       8,     0,     7,     0,     8,     1,     1,     1,     1,     1,
       2,     3,     3,     2,     2,     0,     2,     0,     2,     0,
       1,     3,     1,     3,     2,     0,     1,     2,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     5,     4,     0,     1,     1,     4,     0,
       5,     0,     2,     0,     3,     0,     7,     8,     6,     2,
       5,     6,     4,     0,     4,     5,     7,     6,     6,     7,
       9,     8,     6,     7,     5,     2,     4,     5,     3,     0,
       3,     4,     6,     5,     5,     6,     8,     7,     2,     0,
       1,     2,     2,     3,     4,     4,     3,     1,     1,     2,
       4,     3,     5,     1,     3,     2,     0,     2,     3,     2,
       0,     0,     4,     0,     5,     2,     0,    10,     0,    11,
       3,     3,     3,     4,     4,     3,     5,     2,     2,     0,
       6,     5,     4,     3,     1,     1,     3,     4,     1,     1,
       1,     1,     4,     1,     1,     3,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       2,     1,     1,     3,     3,     1,     1,     0,     1,     2,
       4,     3,     6,     2,     3,     6,     1,     1,     1,     1,
       1,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     3,
       2,     1,     5,     0,     0,    11,     0,    12,     0,     3,
       0,     4,     0,     6,     0,     7,     2,     2,     4,     1,
       1,     5,     3,     5,     3,     2,     0,     2,     0,     4,
       4,     3,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     1,     1,     1,     1,     3,     2,     3,     4,
       2,     3,     1,     2,     1,     2,     1,     1,     1,     1,
       1,     1,     4,     4,     2,     8,    10,     2,     1,     3,
       1,     2,     1,     1,     1,     1,     2,     4,     3,     3,
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
       1,     3,     2,     1,     1,     4,     3,     4,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     3,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     1,     2,     2,     4,     3,
       4,     2,     0,     5,     3,     3,     1,     3,     1,     2,
       0,     5,     3,     2,     0,     3,     0,     4,     2,     0,
       3,     3,     1,     0,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     4,     2,     4,     1,     1,     1,     1,
       1,     3,     5,     3,     4,     4,     3,     1,     1,     1,
       1,     3,     5,     4,     3,     1,     1,     3,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     3,
       5,     3,     3,     0,     1,     3,     5,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     0,     2,     0,
       5,     3,     3,     1,     2,     0,     2,     0,     5,     3,
       3,     1,     2,     2,     1,     2,     1,     4,     3,     3,
       6,     3,     1,     1,     1,     4,     4,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     1,     2,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     2,     4,
       2,     6,     4,     1,     1,     0,     3,     5,     3,     1,
       2,     0,     4,     2,     2,     1,     1,     1,     1,     4,
       6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   358,     0,   749,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   825,     0,
     813,   632,     0,   638,   639,   640,    22,   698,   801,    98,
      99,   641,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   330,
     331,   332,   335,   334,   333,     0,     0,     0,     0,   159,
       0,     0,     0,   645,   647,   648,   642,   643,     0,     0,
     649,   644,     0,   623,    23,    24,    25,    27,    26,     0,
     646,     0,     0,     0,     0,     0,     0,     0,   650,   336,
      28,    29,    31,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,   452,     0,    97,    70,   805,   633,     0,
       0,     4,    59,    61,    64,   697,     0,   622,     0,     6,
     131,     7,     9,     8,    10,     0,     0,   328,   368,     0,
       0,     0,     0,     0,     0,     0,   366,   789,   790,   436,
     435,   352,   437,   438,   441,     0,     0,   351,   767,   624,
       0,   700,   434,   327,   770,   367,     0,     0,   370,   369,
     768,   769,   766,   796,   800,     0,   424,   699,    11,   335,
     334,   333,     0,     0,    27,    59,   131,     0,   885,   367,
     884,     0,   882,   881,   440,     0,   359,   363,     0,     0,
     408,   409,   410,   411,   433,   431,   430,   429,   428,   427,
     426,   425,   801,   625,     0,   899,   624,     0,   390,     0,
     388,     0,   829,     0,   707,   350,   628,     0,   899,   627,
       0,   637,   808,   807,   629,     0,     0,   631,   432,     0,
       0,     0,     0,   355,     0,    78,   357,     0,     0,    84,
      86,     0,     0,    88,     0,     0,     0,   926,   927,   931,
       0,     0,    59,   925,     0,   928,     0,     0,    90,     0,
       0,     0,     0,   122,     0,     0,     0,     0,     0,     0,
      42,    47,   248,     0,     0,   247,     0,   163,     0,   160,
     253,     0,     0,     0,     0,     0,   896,   147,   157,   821,
     825,   866,     0,   652,     0,     0,     0,   864,     0,    16,
       0,    63,   139,   151,   158,   529,   466,   849,   847,   847,
       0,   890,   450,   454,   753,   368,     0,   366,   367,   369,
       0,     0,   634,     0,   635,     0,     0,     0,   121,     0,
       0,    66,   239,     0,    21,   130,     0,   156,   143,   155,
     333,   336,   131,   329,   112,   113,   114,   115,   116,   118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   813,     0,   111,   804,   804,
     119,   835,     0,     0,     0,     0,     0,     0,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   389,   387,   754,   755,     0,   804,     0,   762,
     239,   239,   804,     0,   806,   797,   821,     0,   131,     0,
       0,    92,     0,   751,   746,   707,     0,     0,     0,     0,
       0,   833,     0,   471,   706,   824,     0,     0,    66,     0,
     239,   349,     0,   764,   630,     0,    70,   199,     0,   449,
       0,    95,     0,     0,   356,     0,     0,     0,     0,     0,
      87,   110,    89,   923,   924,     0,   921,     0,     0,     0,
     895,     0,   117,    91,   120,     0,     0,     0,     0,     0,
       0,     0,   487,     0,   494,   496,   497,   498,   499,   500,
     501,   492,   514,   515,    70,     0,   107,   109,     0,     0,
      44,    51,     0,     0,    46,    55,    48,     0,    18,     0,
       0,   249,     0,    93,   162,   161,     0,     0,    94,   886,
       0,     0,   368,   366,   367,   370,   369,     0,   915,   169,
       0,   822,     0,     0,     0,     0,   651,   865,   698,     0,
       0,   863,   703,   862,    62,     5,    13,    14,     0,   167,
       0,     0,   459,     0,     0,   707,     0,     0,   626,   460,
     853,     0,   707,     0,     0,   707,     0,     0,     0,     0,
       0,   753,     0,   709,   752,   935,   348,   421,   776,   788,
      75,    69,    71,    72,    73,    74,   327,     0,   439,   701,
     702,    60,   707,     0,   900,     0,     0,     0,   709,   240,
       0,   444,   133,   165,     0,   393,   395,   394,     0,     0,
     391,   392,   396,   398,   397,   413,   412,   415,   414,   416,
     418,   419,   417,   407,   406,   400,   401,   399,   402,   403,
     405,   420,   404,   803,     0,     0,   839,     0,   707,   889,
       0,   888,   773,   796,   149,   141,   153,   145,   131,   358,
       0,   361,   364,   372,   488,   386,   385,   384,   383,   382,
     381,   380,   379,   378,   377,   376,   375,     0,   756,   758,
     771,   759,     0,     0,     0,     0,     0,     0,     0,     0,
     883,   360,   744,   748,   706,   750,     0,     0,   899,     0,
     828,     0,   827,     0,   812,   811,     0,   758,   809,   353,
     201,   203,    70,   457,   456,   354,     0,    70,   183,    79,
     357,     0,     0,     0,     0,     0,   195,   195,    85,     0,
       0,     0,   919,   707,     0,   906,     0,     0,     0,     0,
       0,   705,   641,     0,     0,   623,     0,     0,     0,     0,
       0,    64,   654,   622,   660,   661,   659,     0,   653,    68,
     658,     0,     0,   504,     0,     0,   510,   507,   508,   516,
       0,   495,   490,     0,   493,     0,     0,     0,    52,    19,
       0,     0,    56,    20,     0,     0,     0,    41,    49,     0,
     246,   254,   251,     0,     0,   875,   880,   877,   876,   879,
     878,    12,   913,   914,     0,     0,     0,     0,   821,   818,
       0,   470,   874,   873,   872,     0,   868,     0,   869,   871,
       0,     5,     0,     0,     0,   523,   524,   532,   531,     0,
       0,   706,   465,   469,     0,   475,   706,   848,     0,   473,
     706,   846,   474,     0,   891,     0,   451,     0,   907,   753,
     225,   934,     0,     0,   763,   802,   706,   902,   898,   241,
     242,   621,   708,   238,     0,   753,     0,     0,   167,   446,
     135,   423,     0,   480,   481,     0,   472,   706,   834,     0,
       0,   239,   169,     0,   167,   165,     0,   813,   373,     0,
       0,   760,   761,   774,   775,   798,   799,     0,     0,     0,
     732,   714,   715,   716,   717,     0,     0,     0,   725,   724,
     738,   707,     0,   746,   832,   831,     0,     0,   765,   636,
     205,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,   175,   176,   187,     0,    70,   185,   104,   195,     0,
     195,     0,     0,   929,     0,     0,   706,   920,   922,   905,
     707,   904,     0,   707,   682,   683,   680,   681,   713,     0,
     707,   705,     0,     0,   468,   857,   855,   855,     0,     0,
     841,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   489,     0,     0,
       0,   512,   513,   511,     0,     0,   491,     0,   123,     0,
     126,   108,     0,    43,    53,     0,    45,    57,    50,   250,
       0,   887,    96,   915,   897,   910,   168,   170,   260,     0,
       0,   819,     0,   867,     0,    17,     0,   890,   166,   260,
       0,     0,   462,     0,   888,   852,   851,     0,   892,     0,
     907,     0,     0,   935,     0,   230,   228,   758,   772,   901,
       0,     0,   243,    67,     0,   753,   164,     0,   753,     0,
     422,   838,   837,     0,   239,     0,     0,     0,     0,   167,
     137,   637,   757,   239,     0,   720,   721,   722,   723,   726,
     727,   736,     0,   707,   732,     0,   719,   740,   706,   743,
     745,   747,     0,   826,   758,   810,     0,     0,     0,     0,
     202,   458,    81,     0,   357,   175,   177,   821,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   189,     0,   916,
       0,   918,   706,     0,     0,     0,   656,   706,   704,     0,
     695,     0,   707,     0,   861,     0,   707,     0,     0,   707,
       0,   662,   696,   694,   845,     0,   707,   665,   667,   666,
       0,     0,   663,   664,   668,   670,   669,   685,   684,   687,
     686,   688,   690,   691,   689,   678,   677,   672,   673,   671,
     674,   675,   676,   679,   502,     0,   503,   509,   517,   518,
       0,    70,    54,    58,   252,     0,     0,     0,   327,   823,
     821,   362,   365,   371,     0,    15,     0,   327,   535,     0,
       0,   537,   530,   533,     0,   528,     0,     0,   893,     0,
     908,   453,     0,   231,     0,     0,   226,     0,   245,   244,
     907,     0,   260,     0,   753,     0,   239,     0,   794,   260,
     890,   260,     0,     0,   374,     0,     0,   729,   706,   731,
       0,   718,     0,     0,   707,   737,   830,     0,    70,     0,
     198,   184,     0,     0,     0,   174,   100,   188,     0,     0,
     191,     0,   196,   197,    70,   190,   930,     0,   903,     0,
     933,   712,   711,   655,     0,   706,   467,   657,   478,   706,
     856,     0,   476,   706,   854,   477,     0,   479,   706,   840,
     693,     0,     0,     0,     0,   909,   912,   171,     0,     0,
       0,   325,     0,     0,     0,   148,   259,   261,     0,   324,
       0,   327,     0,   870,   256,   152,   526,     0,     0,   461,
     850,   455,     0,   234,   224,     0,   227,   233,   239,   443,
     907,   327,   907,     0,   836,     0,   793,   327,     0,   327,
     260,   753,   791,   735,   734,   728,     0,   730,   706,   739,
      70,   204,    77,    82,   102,   178,     0,   186,   192,    70,
     194,   917,     0,     0,   464,     0,   860,   859,     0,   844,
     843,   692,     0,    70,   127,     0,     0,     0,     0,     0,
     172,   291,   289,   293,   623,    27,     0,   285,     0,   290,
     302,     0,   300,   305,     0,   304,     0,   303,     0,   131,
     263,     0,   265,     0,   820,     0,   527,   525,   536,   534,
     235,     0,     0,   222,   232,     0,     0,     0,     0,   144,
     443,   907,   795,   150,   256,   154,   327,     0,     0,   742,
       0,   200,     0,     0,    70,   181,   101,   193,   932,   710,
       0,     0,     0,     0,     0,   911,     0,     0,     0,     0,
     275,   279,     0,     0,   270,   587,   586,   583,   585,   584,
     604,   606,   605,   575,   546,   547,   565,   581,   580,   542,
     552,   553,   555,   554,   574,   558,   556,   557,   559,   560,
     561,   562,   563,   564,   566,   567,   568,   569,   570,   571,
     573,   572,   543,   544,   545,   548,   549,   551,   589,   590,
     599,   598,   597,   596,   595,   594,   582,   601,   591,   592,
     593,   576,   577,   578,   579,   602,   603,   607,   609,   608,
     610,   611,   588,   613,   612,   615,   617,   616,   550,   620,
     618,   619,   614,   600,   541,   297,   538,     0,   271,   318,
     319,   317,   310,     0,   311,   272,   344,     0,     0,     0,
       0,   131,   140,   255,     0,     0,     0,   223,   237,   792,
       0,    70,   320,    70,   134,     0,     0,     0,   146,   907,
     733,     0,    70,   179,    83,   103,     0,   463,   858,   842,
     505,   125,   273,   274,   347,   173,     0,     0,   294,   286,
       0,     0,     0,   299,   301,     0,     0,   306,   313,   314,
     312,     0,     0,   262,     0,     0,     0,     0,   257,     0,
     236,     0,   521,   709,     0,     0,    70,   136,   142,     0,
     741,     0,     0,     0,   105,   276,    59,     0,   277,   278,
       0,     0,   292,   296,   539,   540,     0,   287,   315,   316,
     308,   309,   307,   345,   342,   266,   264,   346,     0,   258,
     522,   708,     0,   445,   321,     0,   138,     0,   182,   506,
       0,   129,     0,   327,   295,   298,     0,   753,   268,     0,
     519,   442,   447,   180,     0,     0,   106,   283,     0,   326,
     343,     0,   709,   338,   753,   520,     0,   128,     0,     0,
     282,   907,   753,   209,   339,   340,   341,   935,   337,     0,
       0,     0,   281,     0,   338,     0,   907,     0,   280,   322,
      70,   267,   935,     0,   214,   212,     0,    70,     0,     0,
     215,     0,     0,   210,   269,     0,   323,     0,   218,   208,
       0,   211,   217,   124,   219,     0,     0,   206,   216,     0,
     207,   221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   121,   821,   555,   185,   279,   509,
     513,   280,   510,   514,   123,   124,   125,   126,   127,   128,
     330,   591,   592,   463,   244,  1442,   469,  1366,  1443,  1671,
     777,   274,   504,  1634,  1011,  1191,  1686,   346,   186,   593,
     866,  1069,  1243,   132,   558,   883,   594,   613,   885,   539,
     882,   595,   559,   884,   348,   297,   313,   135,   868,   824,
     807,  1026,  1389,  1119,   931,  1584,  1446,   719,   937,   468,
     728,   939,  1274,   711,   920,   923,  1108,  1691,  1692,   582,
     583,   607,   608,   284,   285,   291,  1415,  1563,  1564,  1198,
    1316,  1408,  1559,  1677,  1694,  1596,  1638,  1639,  1640,  1396,
    1397,  1398,  1597,  1603,  1647,  1401,  1402,  1406,  1552,  1553,
    1554,  1574,  1721,  1317,  1318,   187,   137,  1707,  1708,  1557,
    1320,   138,   237,   464,   465,   139,   140,   141,   142,   143,
     144,   145,   146,  1427,   147,   865,  1068,   148,   241,   580,
     324,   581,   459,   564,   565,  1142,   566,  1143,   149,   150,
     151,   152,   153,   754,   755,   756,   154,   155,   271,   156,
     272,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     767,   768,  1003,   501,   502,   503,   774,  1623,   157,   560,
    1417,   561,  1040,   829,  1215,  1212,  1545,  1546,   158,   159,
     160,   231,   238,   333,   451,   161,   958,   760,   162,   959,
     857,   850,   960,   909,  1089,  1091,  1092,  1093,   911,  1253,
    1254,   912,   693,   435,   198,   199,   596,   585,   416,   679,
     680,   854,   164,   232,   189,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   644,   175,   234,   235,   542,   223,
     224,   647,   648,  1155,  1156,   574,   571,   575,   572,  1148,
    1145,  1149,  1146,   306,   307,   815,   176,   530,   177,   579,
     178,  1565,   298,   341,   602,   603,   952,  1052,   804,   805,
     732,   733,   734,   265,   266,   852
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1464
static const yytype_int16 yypact[] =
{
   -1464,   138, -1464, -1464,  6381, 13867, 13867,   -53, 13867, 13867,
   13867, 11955, 13867, -1464, 13867, 13867, 13867, 13867, 13867, 13867,
   13867, 13867, 13867, 13867, 13867, 13867, 15344, 15344, 12163, 13867,
   15410,   -51,   -47, -1464, -1464, -1464, -1464, -1464,   150, -1464,
   -1464,   101, 13867, -1464,   -47,   151,   167,   173,   -47, 12329,
   13536, 12495, -1464, 14389, 10957,   152, 13867, 14647,   104, -1464,
   -1464, -1464,   248,    13,    32,   185,   194,   203,   232, -1464,
   13536,   240,   242, -1464, -1464, -1464, -1464, -1464,   326,  5507,
   -1464, -1464, 13536, -1464, -1464, -1464, -1464, 13536, -1464, 13536,
   -1464,   334,   262,   300,   319,   337, 13536, 13536, -1464,   -18,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, 13867, -1464, -1464,   289,   336,   339,
     339, -1464,   533,   424,   374, -1464,   378, -1464,    48, -1464,
     550, -1464, -1464, -1464, -1464, 16189,     7, -1464, -1464,   387,
     437,   438,   440,   442,   443,  3522, -1464, -1464, -1464, -1464,
     555, -1464, -1464, -1464,   583,   585,   451, -1464,    95,   448,
     507, -1464, -1464,   488,    76,  2144,   107,   456,    85, -1464,
     108,   112,   465,    83, -1464,   163, -1464,   601, -1464, -1464,
   -1464,   519,   471,   520, -1464, -1464,   550,     7, 16526,  2862,
   16526, 13867, 16526, 16526, 11148,   476, 15598, 11148,   639, 13536,
     620,   620,    99,   620,   620,   620,   620,   620,   620,   620,
     620,   620, -1464, -1464, 14924,   522, -1464,   540,   254,   493,
     254, 15344, 15642,   496,   692, -1464,   519, 14978,   522,   556,
     559,   509,   118, -1464,   254,   107, 12661, -1464, -1464, 13867,
    5713,   703,    62, 16526, 10541, -1464, 13867, 13867, 13536, -1464,
   -1464,  3596,   513, -1464,  3652, 14389, 14389,   548, -1464, -1464,
     518,  2446,   708, -1464,   711, -1464, 13536,   648, -1464,   523,
    3769,   525,   672, -1464,     1,  3899, 16204, 16249, 13536,    65,
   -1464,    17, -1464, 15119,    67, -1464,   611, -1464,   614, -1464,
     723,    69, 15344, 15344, 13867,   538,   573, -1464, -1464, 15204,
   12163,   341,   420, -1464, 14033, 15344,   366, -1464, 13536, -1464,
     303,   424, -1464, -1464, -1464, -1464,  4825, 13867, 13867, 13867,
     731,   649, -1464, -1464,    28,   545, 16526,   552,   613,   554,
    6589, 13867,   299,   551,   343,   299,   266,   261, -1464, 13536,
   14389,   560, 11165, 14389, -1464, -1464, 11998, -1464, -1464, -1464,
   -1464, -1464,   550, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   13867, 13867, 13867, 12869, 13867, 13867, 13867, 13867, 13867, 13867,
   13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867,
   13867, 13867, 13867, 13867, 13867, 15410, 13867, -1464, 13867, 13867,
   -1464, 13867,  3900, 13536, 13536, 13536, 16189,   652,   434, 10749,
   13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867, 13867,
   13867, 13867, -1464, -1464, -1464, -1464,  4733, 13867, 13867, -1464,
   11165, 11165, 13867, 13867,   289,   120, 15204,   561,   550, 13077,
    3957, -1464, 13867, -1464,   562,   755, 14924,   566,   225,   554,
    5309,   254, 13285, -1464, 13493, -1464,   568,   233, -1464,   219,
   11165, -1464,  4733, -1464, -1464,  4337, -1464, -1464, 11373, -1464,
   13867, -1464,   684,  9709,   766,   575, 16437,   762,    87,    29,
   -1464, -1464, -1464, -1464, -1464, 14389, 15188,   579,   774, 14743,
   -1464,   597, -1464, -1464, -1464,   706, 13867,   707,   713, 13867,
   13867, 13867, -1464,   672, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464,   603, -1464, -1464, -1464,   586, -1464, -1464, 13536,   591,
     786,   296, 13536,   595,   790,   309,   323, 16267, -1464, 13536,
   13867,   254,   104, -1464, -1464, -1464, 14743,   721, -1464,   254,
      88,    89,   606,   608,  1672,   156,   610,   600,   449,   687,
     616,   254,    90,   612, 15387, 13536, -1464, -1464,   754,  1939,
     -23, -1464, -1464, -1464,   424, -1464, -1464, -1464,   791,   700,
     662,   288, -1464,   289,   702,   824,   641,   697,   120, -1464,
   16526,   645,   832, 15698,   646,   842,   650, 14389, 14389,   836,
     703,    28,   654,   846, -1464, 14389,    60,   793,   113, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464,   635,  2070, -1464, -1464,
   -1464, -1464,   852,   694, -1464, 15344, 13867,   664,   862, 16526,
     858, -1464, -1464,   747, 12912, 11564, 16652, 11148, 13867, 16482,
    2973,  4098,  3998, 14615,  2386, 12847, 12847, 12847, 12847,  2726,
    2726,  2726,  2726,   789,   789,   647,   647,   647,    99,    99,
      99, -1464,   620, 16526,   666,   667, 15742,   674,   868, -1464,
   13867,   -54,   679,   120, -1464, -1464, -1464, -1464,   550, 13867,
   15065, -1464, -1464, 11148, -1464, 11148, 11148, 11148, 11148, 11148,
   11148, 11148, 11148, 11148, 11148, 11148, 11148, 13867, -1464,   121,
     -54, -1464,   675,  2214,   682,   691,   678,  2283,    91,   696,
   -1464, 16526, 14875, -1464, 13536, -1464,   545,    60,   522, 15344,
   16526, 15344, 15798,    60,   126, -1464,   699,   127, -1464, -1464,
    9501,    64, -1464, -1464, 16526, 16526,   -47, -1464, -1464, -1464,
   13867,   802, 14590, 14743, 13536,  9917,   701,   704, -1464,    55,
     773,   759, -1464,   902,   710,  5144, 14389, 14743, 14743, 14743,
   14743, 14743, -1464,   714,    23,   765,   715,   716,   718,   720,
   14743,   394, -1464,   780, -1464, -1464, -1464,   727, -1464, 16612,
   -1464, 13867,   746, 16526,   748,   920,  4531,   928, -1464, 16526,
    4471, -1464,   603,   860, -1464,  6797, 16126,   740,   382, -1464,
   16204, 13536,   383, -1464, 16249, 13536, 13536, -1464, -1464,  2524,
   -1464, 16612,   931, 15344,   744, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464,    68, 13536, 16126,   745, 15204, 15289,
     934, -1464, -1464, -1464, -1464,   742, -1464, 13867, -1464, -1464,
    5965, -1464, 14389, 16126,   749, -1464, -1464, -1464, -1464,   937,
   13867,  4825, -1464, -1464, 13120, -1464, 13867, -1464, 13867, -1464,
   13867, -1464, -1464,   752, -1464, 14389, -1464,   753,   926,    18,
   -1464, -1464,   106,  4733, -1464, -1464, 14389, -1464, -1464,   254,
   16526, -1464, 11581, -1464, 14743,    27,   763, 16126,   700, -1464,
   -1464, 14239, 13867, -1464, -1464, 13867, -1464, 13867, -1464,  2799,
     768, 11165,   687,   930,   700,   747, 13536, 15410,   254,  2924,
     769, -1464, -1464,   155, -1464, -1464, -1464,   949, 14786, 14786,
   14875, -1464, -1464, -1464, -1464,   771,   285,   772, -1464, -1464,
   -1464,   958,   775,   562,   254,   254, 13701,  4733, -1464, -1464,
     432,   -47, 10541, -1464,  7005,   776,  7213,   792, 14590, 15344,
     777,   844,   254, 16612,   982, -1464, -1464, -1464, -1464,    33,
   -1464,   247, 14389, -1464, 14389, 13536, 15188, -1464, -1464, -1464,
     988, -1464,   796,   852,   558,   558,   935,   935, 15942,   794,
     990, 14743,   855, 13536,  4825, 14743, 14743, 14743,  4393, 13328,
   14743, 14743, 14743, 14743, 14541, 14743, 14743, 14743, 14743, 14743,
   14743, 14743, 14743, 14743, 14743, 14743, 14743, 14743, 14743, 14743,
   14743, 14743, 14743, 14743, 14743, 14743, 14743, 16526, 13867, 13867,
   13867, -1464, -1464, -1464, 13867, 13867, -1464,   672, -1464,   924,
   -1464, -1464, 13536, -1464, -1464, 13536, -1464, -1464, -1464, -1464,
   14743,   254, -1464,   449, -1464,   907,   996, -1464, -1464,    92,
     808,   254, 11789, -1464,   165, -1464,  6173,   649,   996, -1464,
     418,   416, 16526,   880, -1464, 16526, 16526, 15842, -1464,   809,
     926, 14389,   703, 14389,   -10,   999,   936,   171,   -54, -1464,
   15344, 13867, 16526, 16612,   812,    27, -1464,   813,    27,   815,
   14239, 16526, 15898,   817, 11165,   820,   819, 14389,   821,   700,
   -1464,   509, -1464, 11165, 13867, -1464, -1464, -1464, -1464, -1464,
   -1464,   889,   816,  1017, 14875,   881, -1464,  4825, 14875, -1464,
   -1464, -1464, 15344, 16526,   172, -1464,   -47,   998,   956, 10541,
   -1464, -1464, -1464,   828, 13867,   844,   254, 15204, 14590,   831,
   14743,  7421,   482,   833, 13867,    20,   249, -1464,   863, -1464,
     908, -1464, 14340,  1007,   837, 14743, -1464, 14743, -1464,   845,
   -1464,   913,  1037,   849, 16612,   850,  1039, 15998,   851,  1045,
     854, -1464, -1464, -1464, 16042,   853,  1048,  5901,  3210,  3422,
   14743, 16570,  4427,  4636, 14411, 14723, 15111, 15163, 15163, 15163,
   15163,   941,   941,   941,   941,   841,   841,   558,   558,   558,
     935,   935,   935,   935, 16526,  5402, 16526, -1464, 16526, -1464,
     857, -1464, -1464, -1464, 16612, 13536, 14389, 16126,   413, -1464,
   15204, -1464, -1464, 11148,   856, -1464,   859,   962, -1464,   313,
   13867, -1464, -1464, -1464, 13867, -1464, 13867, 13867, -1464,   703,
   -1464, -1464,   292,  1046,   983, 14743, -1464,   864,   254, 16526,
     926,   865, -1464,   870,    27, 13867, 11165,   882, -1464, -1464,
     649, -1464,   866,   885, -1464,   886, 14875, -1464, 14875, -1464,
     891, -1464,   952,   893,  1055, -1464,   254,  1053, -1464,   884,
   -1464, -1464,   894,   896,    93, -1464, -1464, 16612,   888,   897,
   -1464,  3323, -1464, -1464, -1464, -1464, -1464, 14389, -1464, 14389,
   -1464, 16612, 16098, -1464, 14743,  4825, -1464, -1464, -1464, 14743,
   -1464, 14743, -1464, 14743, -1464, -1464, 14743, -1464, 14743, -1464,
    5145, 14743, 13867,   895,  7629,  1004, -1464, -1464,   390, 14389,
   16126, -1464, 16145,   945, 14443, -1464, -1464, -1464,   652,  5461,
      70,   434,    94, -1464, -1464, -1464,   954,  3061,  3123, 16526,
   16526, -1464,   236,  1095,  1034, 14743, -1464, 16612, 11165,  1011,
     926,  1107,   926,   922, 16526,   925, -1464,  1132,   911,  1655,
   -1464,    27, -1464, -1464,  1000, -1464, 14875, -1464,  4825, -1464,
   -1464,  9501, -1464, -1464, -1464, -1464, 10125, -1464, -1464, -1464,
    9501, -1464,   932, 14743, 16612,  1008, 16612, 16612, 16142, 16612,
   16198,  5145,  5014, -1464, -1464, 14389, 16126, 16126,  1114,    53,
   -1464, -1464, -1464, -1464,    75,   933,    78, -1464, 14241, -1464,
   -1464,    79, -1464, -1464,  1842, -1464,   938, -1464,  1060,   550,
   -1464, 14389, -1464,   652, -1464,  2324, -1464, -1464, -1464, -1464,
    1126,  1063, 14743, -1464, 16612,   944,   942,   943,   306, -1464,
    1011,   926, -1464, -1464, -1464, -1464,  1748,   946, 14875, -1464,
    1019,  9501, 10333, 10125, -1464, -1464, -1464,  9501, -1464, 16612,
   14743, 14743, 14743, 13867,  7837, -1464,   947,   948, 14743, 16126,
   -1464, -1464,  1339, 16145, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464,   116, -1464,   945, -1464, -1464,
   -1464, -1464, -1464,    86,   110, -1464,  1139,    80, 13536,  1060,
    1140,   550, -1464, -1464,   953,  1141, 14743, -1464, 16612, -1464,
     344, -1464, -1464, -1464, -1464,   957,   306,  5069, -1464,   926,
   -1464, 14875, -1464, -1464, -1464, -1464,  8045, 16612, 16612, 16612,
    4727, -1464, -1464, -1464, 16612, -1464,  3325,    47, -1464, -1464,
   14743, 14241, 14241,  1102, -1464,  1842,  1842,   565, -1464, -1464,
   -1464, 14743,  1079, -1464,   959,    81, 14743, 13536, -1464, 14743,
   16612,  1083, -1464,  1155,  8253,  8461, -1464, -1464, -1464,   306,
   -1464,  8669,   963,  1086,  1059, -1464,  1072,  1021, -1464, -1464,
    1074,  1339, -1464, 16612, -1464, -1464,  1012, -1464,  1142, -1464,
   -1464, -1464, -1464, 16612,  1158, -1464, -1464, 16612,   973, 16612,
   -1464,   446,   974, -1464, -1464,  8877, -1464,   977, -1464, -1464,
     980,  1016, 13536,   434, -1464, -1464, 14743,    72, -1464,  1109,
   -1464, -1464, -1464, -1464, 16126,   740, -1464,  1022, 13536,   441,
   16612,   987,  1181,   474,    72, -1464,  1113, -1464, 16126,   991,
   -1464,   926,    73, -1464, -1464, -1464, -1464, 14389, -1464,   993,
     994,    82, -1464,   389,   474,   301,   926,   995, -1464, -1464,
   -1464, -1464, 14389,   253,  1184,  1121,   389, -1464,  9085,   302,
    1188,  1124, 14743, -1464, -1464,  9293, -1464,   276,  1190,  1128,
   14743, -1464, 16612, -1464,  1191,  1129, 14743, -1464, 16612, 14743,
   -1464, 16612, 16612
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1464, -1464, -1464,  -516, -1464, -1464, -1464,    -4, -1464, -1464,
   -1464,   695,   429,   427,    25,  1080,  3863, -1464,  2454, -1464,
    -389, -1464,     5, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464,  -470, -1464, -1464,  -180,     4,     6,
   -1464, -1464, -1464, -1464, -1464, -1464,     8, -1464, -1464, -1464,
   -1464,     9, -1464, -1464,   823,   834,   822,  -130,   331,  -816,
     346,   407,  -467,   117,  -858, -1464,  -210, -1464, -1464, -1464,
   -1464,  -679,   -37, -1464, -1464, -1464, -1464,  -459, -1464,  -547,
   -1464,  -392, -1464, -1464,   717, -1464,  -194, -1464, -1464,  -986,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
    -227, -1464, -1464, -1464, -1464, -1464,  -306, -1464,   -62, -1006,
   -1464, -1463,  -473, -1464,  -162,    -1,  -132,  -460, -1464,  -304,
   -1464,   -70,   -21,  1214,  -685,  -368, -1464, -1464,   -39, -1464,
   -1464,  3873,   -60,  -172, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464,  -554,  -793, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,   861, -1464,
   -1464,   255, -1464,   778, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464,   257, -1464,   779, -1464, -1464,   495, -1464,   229, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464,  -976, -1464,  1928,
     370,  -353, -1464, -1464,   184,  3671,  4388, -1464, -1464,   312,
    -188,  -603, -1464, -1464,   371,  -672,   180, -1464, -1464, -1464,
   -1464, -1464,   363, -1464, -1464, -1464,    61,  -832,  -192,  -415,
    -409,  -141, -1464, -1464,    14, -1464, -1464,  1492,   -69, -1464,
   -1464,    45,  -173, -1464,  -262, -1464, -1464, -1464,  -410,   978,
   -1464, -1464, -1464, -1464, -1464,   960, -1464, -1464, -1464,   310,
   -1464, -1464, -1464,   541,   397, -1464, -1464,   989,  -309,  -990,
   -1464,   -34,   -75,  -198,  -120,   546, -1464,  -999, -1464,   260,
     338, -1464, -1464, -1464,  -175, -1029
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -900
static const yytype_int16 yytable[] =
{
     122,   397,   425,   136,   353,   863,   427,   569,   130,   129,
     131,   240,   133,   134,   314,   269,   688,  1053,   165,   264,
     910,   320,   321,   245,  1222,   419,   846,   249,   684,   685,
     448,   661,   641,   396,   847,   927,   445,   707,  1043,   820,
     218,   220,   452,   708,   325,   329,   252,  1206,   941,   262,
    1272,  1219,  1067,  1207,   327,   353,  1641,   343,   706,   726,
     349,   322,  1459,    13,   942,   163,   296,   710,  1078,  1223,
    1115,   460,    13,    13,   517,   233,   522,  1023,   527,  1411,
     473,   474,   281,   312,  -288,   296,   478,  1463,  1547,  1612,
    1612,  1459,   296,   296,   453,  1605,   724,   793,   793,   809,
     809,   809,   809,   809,   310,   414,   415,   311,   962,   545,
     505,   290,    52,  1627,   519,   775,   340,    13,    13,  1606,
      59,    60,    61,   179,   180,   350,   417,   645,   328,   288,
    1600,   296,  1123,  1124,  1125,   921,   922,   289,     3,  1608,
     352,  1054,  -780,   568,   437,   439,  1601,  -899,   191,   422,
     236,  -484,   385,  1224,   239,   682,  1609,   446,  -448,  1610,
     686,  -777,   398,  1602,   386,   604,  1666,  -485,   506,   303,
     339,  1141,   614,   417,  -778,   360,   361,   362,  -779,  -784,
     323,  -625,   819,   282,  -814,  1055,   422,  -781,   414,   415,
     351,   428,  -817,  -815,   363,   434,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,  -708,   385,   653,
    -708,  -783,  -484,  1273,   532,   536,   943,  -229,   438,  -229,
     386,  1339,  -626,   727,   533,   441,   122,  -782,  -816,  1024,
     122,   447,   212,   653,   467,   457,  1341,   695,   689,   462,
    1348,   344,  1642,  1347,   165,  1349,  1460,  1461,   165,  1122,
    1265,  1126,   480,  1242,   418,   461,   353,   653,   518,  1056,
     523,   612,   528,  1412,  -213,  -708,   653,  -786,  -288,   653,
    -780,  1464,  1548,  1613,  1656,  1718,   421,   423,  1607,   725,
     794,   795,   810,   897,  1199,  1365,  1414,   521,   212,  -777,
     729,   511,   515,   516,  1252,  1036,   529,   529,   534,   610,
     283,   418,  -778,   541,  -787,  1420,  -779,  -784,  1064,   550,
     314,   349,  -814,   924,   423,  -781,   122,  1332,   926,   136,
    -817,  -815,  1730,   554,   130,   601,  1723,  1737,   826,   262,
     301,  1428,   296,  1430,   165,   301,   551,  1124,  1125,  1124,
    1125,   551,   246,   414,   415,  1744,   273,   421,   799,  -783,
     662,   414,   415,  1326,  1436,   286,   696,   439,   247,   424,
    1095,  1333,   287,  1204,   248,  -782,  -816,   832,   301,  1621,
    1724,  1738,   414,   415,   837,   584,   292,   841,   651,   296,
     655,   296,   296,   781,   853,   293,   217,   217,  1029,  1421,
     230,   658,   843,   844,   294,   301,   785,   544,   304,   305,
     851,   302,   678,   304,   305,   301,  1731,  1308,   301,   713,
     786,   332,   301,  1622,   335,   117,  1255,   588,   551,  1262,
     233,  1096,  1576,   295,   397,   704,   698,   652,  1057,  1745,
     541,   299,   827,   300,  1058,   301,   304,   305,   678,   339,
     438,   551,  1127,   880,  1275,  1334,   339,   828,    13,   122,
     878,   681,   339,   316,  1725,  1739,   396,   315,   718,   414,
     415,   600,   303,   304,   305,   599,   339,   165,   886,  1012,
    1015,  1679,   890,   304,   305,   652,   304,   305,   315,  1075,
     304,   305,  1375,   331,   705,   802,   803,   681,  1221,   301,
     880,   317,  1104,  1106,  1107,   853,   556,   557,  1105,  1572,
    1573,   917,   552,   304,   305,   788,  1386,  1387,  1231,  1309,
     318,  1233,   569,  -899,  1310,  1680,    59,    60,    61,   179,
     180,   350,  1311,   778,  1081,   339,   339,   782,   319,   870,
     814,   816,   281,  -899,   340,   947,  1121,    59,    60,    61,
     179,   180,   350,   448,    59,    60,    61,   179,   180,   350,
     604,   604,   918,   338,   340,  1440,   546,   304,   305,  1312,
    1313,  1213,  1314,  1208,  1353,  -899,  1354,   339,  -899,   342,
    1629,  1269,  1124,  1125,   217,   345,  1209,  1704,  1705,  1706,
     354,   217,  1719,  1720,  1650,   398,   351,   217,  -899,  1648,
    1649,    59,    60,    61,    62,    63,   350,   993,   994,   995,
     296,  1651,    69,   393,  1652,   950,   953,   351,  1315,   859,
    1214,  -482,  1210,   996,   351,  1644,  1645,   429,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   409,   410,   411,
     355,   356,   584,   357,  1700,   358,   359,  1037,   394,   388,
     395,   389,   391,   217,   390,   569,   392,   420,   568,   334,
     336,   337,   217,   217,  1201,  1331,  -785,  -483,  -625,   217,
    1049,   351,   426,   308,   888,   217,   412,   413,  1715,   431,
     653,  1059,  1237,   433,  1439,   386,   567,  1343,   908,   440,
     913,  1245,   340,  1729,   421,   925,   382,   383,   384,   547,
     385,   444,  1713,   553,   443,  -624,   122,  1264,   449,   136,
     450,   458,   386,   914,   130,   915,   471,  1726,   475,   476,
     934,   122,  -894,  1099,   165,   479,   482,   481,   484,   547,
     936,   553,   547,   553,   553,   524,   932,   526,   525,   165,
     537,   414,   415,   538,   653,   577,   578,   586,    59,    60,
      61,    62,    63,   350,   587,   230,   589,    52,   598,    69,
     393,   -65,   611,   692,   694,  1134,  1580,  1128,   697,  1129,
     703,   122,  1138,   716,   136,   460,   723,  1014,   720,   130,
     735,  1017,  1018,   736,   761,   762,   764,   776,   569,   165,
    1322,   568,   765,   773,   779,   780,   217,   395,   783,   784,
     792,  1025,  1304,   801,  1437,   511,   217,  1021,   796,   515,
     797,  1080,   800,   806,   811,   588,   122,   808,   351,   136,
     817,   822,   541,  1031,   130,   129,   131,   823,   133,   134,
    1044,   825,   830,   831,   165,   379,   380,   381,   382,   383,
     384,   836,   385,   833,  1345,  1693,   834,   835,   839,   678,
     845,   840,   842,   848,   386,   849,   485,   486,   487,  -486,
    1227,   856,  1693,   488,   489,   858,   861,   490,   491,  1361,
    1714,   862,   864,   867,   873,   874,  1220,   877,   851,   876,
     881,   163,   296,   891,   893,  1370,   895,   990,   991,   992,
     993,   994,   995,   894,  1088,  1088,   908,   869,   681,   928,
    1109,   919,  1240,   944,   938,  1249,   996,   940,   945,  1630,
     584,   946,   948,   678,   963,   961,   964,   965,   122,   966,
     122,   967,   122,   136,   568,   136,   584,  1110,   130,   969,
     130,   970,   233,   998,  1000,   999,   165,  1004,   165,  1007,
     165,  1130,   932,  1116,  1010,  1020,  1425,  1022,  1032,  1028,
    1033,  1041,  1050,  1039,  1286,  1048,  1051,  1059,  1290,  1140,
    1077,  1294,   681,  1084,  1065,  1153,  1308,  1098,  1299,  1074,
    1083,  1441,  1094,  1097,  1118,   217,   569,  1100,  1117,  1112,
    1447,  -900,  -900,  -900,  -900,   988,   989,   990,   991,   992,
     993,   994,   995,  1202,  1454,  1114,  1120,  1132,  1133,  1137,
     996,   546,  1136,  1190,  1196,  1197,   996,    13,  1192,  1200,
    1216,  1193,  1218,  1225,  1230,  1226,  1234,  1232,  1236,  1246,
    1662,  1306,  1238,  1239,  1247,  1241,  1248,  1251,  1258,  1259,
     217,  1261,   122,  1266,  1276,   136,  1270,  1279,  1277,  1280,
     130,   129,   131,  1284,   133,   134,  1285,  1283,  1289,   569,
     165,  1287,  1288,  1292,  1293,  1586,  1295,  1298,  1297,  1303,
    1335,  1323,  1336,  1324,  1358,  1338,  1359,  1340,  1309,   217,
    1350,   217,  1342,  1310,  1228,    59,    60,    61,   179,   180,
     350,  1311,  1356,  1360,  1346,  1257,  1351,  1362,  1352,  1703,
     908,  1367,   217,  1355,   908,  1357,  1363,   163,  1364,  1383,
    1368,  1385,  1371,  1400,  1372,   122,   215,   215,  1416,  1422,
     228,  1308,   568,  1423,  1260,  1434,  1256,   122,  1312,  1313,
     136,  1314,  1426,   165,  1431,   130,   584,  1432,  1458,   584,
    1438,   541,   932,   228,  1448,   165,  1308,  1462,  1450,  1556,
    1566,  1555,  1567,  1570,  1410,   351,  1569,  1571,  1579,  1581,
    1592,  1593,    13,  1611,  1616,  1619,  1618,  1646,  1654,  1413,
    1655,  1626,  1660,   217,  1661,  1669,  1668,  1325,  1670,  -284,
    1672,  1673,  1676,  1675,  1678,  1606,  1681,    13,   217,   217,
    1683,  1684,  1624,  1685,  1625,   568,  1698,   353,  1695,  1701,
    1702,  1305,  1710,  1631,  1712,  1716,  1717,  1319,  1732,  1727,
    1733,   567,  1740,  1741,  1746,  1749,  1319,  1747,  1750,  1013,
    1455,  1016,   787,  1309,   541,  1697,  1079,   656,  1310,   657,
      59,    60,    61,   179,   180,   350,  1311,   654,  1076,  1558,
    1038,  1711,  1263,  1585,  1369,  1709,  1599,  1665,  1309,   790,
    1577,  1604,   908,  1310,   908,    59,    60,    61,   179,   180,
     350,  1311,  1407,  1734,  1722,  1615,   242,   230,  1575,  1321,
     664,  1187,  1189,  1312,  1313,  1244,  1314,  1006,  1321,  1211,
    1090,   771,   772,  1139,  1250,  1388,  1101,  1150,   543,   576,
       0,   951,   531,  1195,  1131,     0,     0,     0,  1312,  1313,
     351,  1314,     0,     0,   215,   584,     0,     0,   217,   217,
     122,   215,     0,   136,     0,   262,     0,   215,   130,     0,
    1405,     0,  1429,     0,     0,   351,     0,     0,   165,     0,
     398,     0,  1409,     0,     0,     0,     0,     0,     0,     0,
       0,  1728,     0,     0,   567,   228,   228,  1433,  1735,     0,
    1319,   228,     0,     0,     0,     0,  1319,     0,  1319,     0,
       0,     0,   908,     0,     0,     0,     0,   122,     0,     0,
     136,     0,   122,   215,     0,   130,   122,     0,     0,   136,
       0,  1445,   215,   215,   130,   165,     0,  1560,     0,   215,
     165,  1617,     0,     0,   165,   215,     0,     0,     0,     0,
       0,     0,     0,     0,  1544,     0,   228,     0,     0,     0,
    1551,     0,  1321,     0,     0,     0,     0,   262,  1321,     0,
    1321,   262,   584,    33,    34,    35,     0,  1561,     0,     0,
     228,     0,     0,   228,     0,   742,     0,     0,     0,     0,
     217,     0,     0,     0,   908,  1319,     0,   122,   122,   122,
     136,     0,     0,   122,     0,   130,   136,  1583,  1445,     0,
     122,   130,     0,   136,     0,   165,   165,   165,   130,     0,
       0,   165,     0,     0,     0,   228,     0,   567,   165,     0,
       0,     0,   217,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,  1614,   744,     0,     0,   217,   217,     0,
      80,    81,     0,     0,     0,     0,     0,  1321,     0,     0,
       0,     0,     0,     0,    90,     0,   215,     0,     0,     0,
       0,  1688,     0,     0,     0,     0,   215,     0,   219,   219,
       0,    98,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   851,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1658,     0,     0,     0,     0,   851,     0,     0,
       0,     0,     0,     0,   296,   228,   228,   353,     0,   751,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,     0,   262,     0,     0,     0,   908,     0,     0,
       0,     0,   122,     0,     0,   136,     0,     0,     0,     0,
     130,     0,  1636,     0,     0,     0,     0,  1544,  1544,     0,
     165,  1551,  1551,     0,     0,     0,   751,     0,     0,     0,
       0,     0,     0,   296,     0,     0,     0,     0,     0,     0,
     122,   122,     0,   136,   136,     0,     0,   122,   130,   130,
     136,     0,     0,     0,     0,   130,     0,     0,   165,   165,
       0,     0,     0,     0,     0,   165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   567,     0,   228,   228,  1308,
       0,   122,     0,     0,   136,   228,     0,     0,  1687,   130,
       0,     0,  1689,     0,     0,     0,     0,     0,     0,   165,
       0,     0,     0,     0,  1699,   215,   429,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   411,     0,
      13,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,   122,     0,     0,   136,   567,     0,
       0,   122,   130,     0,   136,   412,   413,     0,   584,   130,
     215,     0,   165,     0,     0,     0,     0,     0,     0,   165,
       0,     0,  1308,     0,     0,   584,     0,     0,     0,     0,
       0,  1309,     0,   584,     0,     0,  1310,     0,    59,    60,
      61,   179,   180,   350,  1311,   219,     0,     0,     0,   215,
       0,   215,     0,     0,   219,   219,   535,     0,     0,     0,
       0,   219,     0,    13,     0,     0,     0,   219,     0,     0,
     414,   415,   215,   751,     0,     0,     0,     0,     0,     0,
       0,  1312,  1313,     0,  1314,   228,   228,   751,   751,   751,
     751,   751,     0,     0,     0,     0,     0,     0,     0,     0,
     751,     0,     0,     0,     0,     0,     0,     0,   351,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1309,     0,   228,     0,     0,  1310,
    1435,    59,    60,    61,   179,   180,   350,  1311,     0,     0,
       0,     0,     0,   215,   798,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   228,     0,   215,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,   228,  1312,  1313,     0,  1314,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,   219,    36,
       0,     0,     0,     0,     0,   228,     0,     0,   219,     0,
       0,   351,     0,     0,     0,     0,   228,     0,     0,     0,
       0,     0,     0,     0,   751,     0,     0,   228,     0,   360,
     361,   362,     0,  1578,   216,   216,     0,     0,   229,     0,
       0,     0,     0,     0,     0,     0,     0,   228,   363,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,     0,     0,  1549,     0,    84,    85,
    1550,    86,   184,    88,   386,     0,     0,     0,   215,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,     0,   228,     0,   228,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,   751,     0,  1404,   228,   751,   751,   751,     0,     0,
     751,   751,   751,   751,   751,   751,   751,   751,   751,   751,
     751,   751,   751,   751,   751,   751,   751,   751,   751,   751,
     751,   751,   751,   751,   751,   751,   751,     0,     0,     0,
     360,   361,   362,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   219,     0,   363,
     751,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,     0,   385,     0,     0,     0,     0,     0,     0,
       0,   228,     0,   228,     0,   386,     0,     0,     0,     0,
     215,     0,     0,     0,   818,     0,     0,     0,     0,   216,
       0,     0,   219,     0,     0,     0,     0,   228,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,   410,
     411,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,   219,     0,     0,     0,   215,   215,     0,
     751,     0,     0,     0,     0,     0,     0,   412,   413,     0,
       0,   216,   228,     0,   219,   751,     0,   751,     0,     0,
     216,   216,     0,     0,   360,   361,   362,   216,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
     751,     0,     0,   363,   216,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,     0,     0,
       0,     0,   414,   415,     0,   855,   228,   228,     0,   386,
     215,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,     0,   360,   361,   362,     0,     0,     0,     0,
     219,   219,     0,     0,     0,   751,     0,     0,     0,     0,
       0,     0,   363,   229,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,     0,   385,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   386,     0,
       0,     0,     0,   255,   216,     0,     0,   228,     0,   228,
       0,     0,     0,     0,   751,   228,     0,     0,     0,   751,
       0,   751,     0,   751,     0,     0,   751,     0,   751,   256,
       0,   751,     0,     0,     0,     0,     0,     0,     0,   228,
     228,     0,   228,     0,     0,     0,     0,     0,     0,   228,
       0,    36,     0,     0,     0,     0,     0,   757,     0,     0,
       0,     0,     0,     0,     0,   751,     0,     0,     0,   892,
     219,   219,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   228,   385,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   386,     0,   751,   757,     0,   257,   258,     0,     0,
       0,     0,     0,     0,     0,   228,   228,   228,     0,     0,
       0,     0,     0,     0,   183,   255,     0,    82,   259,     0,
      84,    85,     0,    86,   184,    88,     0,     0,   896,     0,
       0,   228,     0,     0,     0,   228,     0,     0,   260,     0,
       0,   256,   751,     0,     0,     0,     0,   263,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,    36,     0,   261,     0,     0,     0,  1562,
     751,   751,   751,   216,   360,   361,   362,     0,   751,   228,
       0,     0,     0,   228,     0,     0,     0,     0,     0,     0,
       0,   477,   219,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,   257,   258,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   386,
       0,     0,     0,     0,   219,     0,   183,     0,     0,    82,
     259,     0,    84,    85,     0,    86,   184,    88,     0,   219,
     219,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,   216,     0,   216,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   751,   261,     0,     0,
     216,   757,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,   757,   757,   757,   757,   757,
       0,     0,     0,     0,     0,     0,   228,     0,   757,     0,
     751,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   751,   219,     0,     0,     0,   751,     0,     0,   751,
       0,     0,     0,     0,  1009,     0,     0,     0,     0,   263,
     263,     0,     0,     0,     0,   263,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     0,  1019,
       0,     0,     0,     0,  1027,     0,   216,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1027,     0,     0,     0,     0,   751,     0,     0,   216,
       0,     0,     0,     0,   228,     0,  -900,  -900,  -900,  -900,
     377,   378,   379,   380,   381,   382,   383,   384,   228,   385,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,   386,   757,     0,   263,  1066,     0,   263,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,     0,   360,
     361,   362,   751,     0,     0,   229,     0,     0,     0,     0,
     751,     0,     0,     0,     0,     0,   751,     0,   363,   751,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,     0,     0,   216,   216,     0,     0,
       0,     0,     0,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   429,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   757,
       0,     0,   216,   757,   757,   757,     0,     0,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   412,   413,     0,     0,   263,
     731,     0,     0,   753,   360,   361,   362,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   757,     0,
       0,     0,     0,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,     0,     0,
     753,     0,     0,     0,     0,     0,     0,     0,   216,   386,
     414,   415,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1073,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   216,   385,     0,     0,     0,
     216,   263,   263,     0,     0,     0,     0,     0,   386,   263,
       0,     0,     0,     0,     0,   216,   216,     0,   757,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   757,     0,   757,     0,     0,     0,     0,
       0,   360,   361,   362,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   757,     0,
     363,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,     0,   385,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1307,   386,     0,   216,  1082,
       0,     0,     0,   360,   361,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   363,   757,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,     0,   385,   753,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   386,   263,
     263,   753,   753,   753,   753,   753,     0,     0,     0,     0,
       0,     0,     0,     0,   753,     0,     0,     0,     0,     0,
       0,     0,   757,   216,     0,     0,     0,   757,     0,   757,
       0,   757,   973,     0,   757,     0,   757,     0,     0,   757,
       0,     0,     0,     0,     0,     0,     0,     0,  1390,   974,
    1399,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,     0,   757,     0,     0,  1418,     0,     0,     0,
       0,     0,     0,     0,     0,   996,   263,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   263,
       0,   757,     0,     0,     0,     0,     0,     0,     0,     0,
     263,     0,     0,     0,  1456,  1457,     0,     0,   753,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1419,     0,
       0,     0,     0,   360,   361,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     757,     0,   363,  1272,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,     0,   385,     0,   757,   757,
     757,     0,     0,     0,     0,     0,   757,  1595,   386,     0,
       0,  1399,     0,     0,     0,     0,   263,     0,   263,     0,
     731,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   753,     0,     0,     0,   753,
     753,   753,     0,     0,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   974,     0,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   753,   183,     0,     0,    82,     0,
       0,    84,    85,     0,    86,   184,    88,   996,     0,     0,
       0,     0,     0,     0,   757,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,     0,   263,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,     0,  1273,     0,   757,     0,
    1635,   263,   360,   361,   362,     0,     0,     0,     0,   757,
       0,     0,     0,     0,   757,     0,     0,   757,     0,     0,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   753,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   263,   386,     0,   753,
       0,   753,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   757,     0,   360,   361,   362,     0,
       0,     0,  1696,     0,   753,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   363,  1390,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,     0,   385,
     263,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     757,   386,   360,   361,   362,     0,     0,     0,   757,     0,
       0,     0,     0,     0,   757,     0,     0,   757,     0,   753,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,     0,
       0,   263,     0,   263,     0,     0,     0,     0,   753,     0,
       0,     0,     0,   753,     0,   753,     0,   753,     0,     0,
     753,     0,   753,     0,     0,   753,     0,     0,     0,     0,
       0,     0,     0,   263,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   263,     0,     0,     0,     0,     0,   360,
     361,   362,     0,     0,     0,     0,     0,     0,     0,   753,
       0,     0,     0,     0,     0,     0,     0,     0,   363,   470,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,     0,     0,     0,   753,     0,     0,
       0,     0,     0,     0,   386,     0,     0,     0,     0,   263,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   472,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,     0,     0,     0,   263,
       0,     0,     0,     0,     0,     0,   753,     0,   188,   190,
       0,   192,   193,   194,   196,   197,     0,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,     0,
       0,   222,   225,     0,   753,   753,   753,     0,     0,   360,
     361,   362,   753,     0,     0,   243,     0,     0,     0,     0,
       0,     0,   251,     0,   254,     0,     0,   270,   363,   275,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   386,     0,     0,   360,   361,   362,
       0,     0,   483,     0,     0,     0,     0,    36,     0,   212,
       0,     0,     0,     0,     0,     0,   363,   326,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     753,     0,   386,     0,   649,     0,     0,     0,     0,     0,
       0,   263,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
    1637,   385,     0,     0,   753,     0,    84,    85,     0,    86,
     184,    88,     0,   386,   430,   753,     0,     0,     0,     0,
     753,     0,     0,   753,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
       0,     0,   507,     0,   650,     0,   117,     0,     0,   455,
       0,     0,   455,     0,     0,     0,     0,     0,     0,   243,
     466,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     753,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     758,   385,     0,     0,     0,     0,     0,     0,     0,   690,
       0,   263,     0,   386,     0,     0,     0,   326,     0,     0,
       0,     0,     0,   222,     0,     0,   263,   549,     0,     0,
       0,     0,     0,     0,     0,     0,   753,     0,     0,     0,
     570,   573,   573,     0,   753,     0,     0,   758,     0,     0,
     753,     0,     0,   753,   597,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   609,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   615,   616,   617,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   628,   629,   630,   631,   632,
     633,   634,   635,   636,   637,   638,   639,   640,     0,   642,
       0,   643,   643,     0,   646,     0,     0,     0,     0,     0,
       0,     0,   663,   665,   666,   667,   668,   669,   670,   671,
     672,   673,   674,   675,   676,     0,     0,     0,     0,     0,
     643,   683,     0,   609,   609,   643,   687,     0,     0,     0,
       0,     0,   663,     0,     0,   691,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   700,     0,   702,     0,     0,
       0,     0,     0,   609,     0,     0,     0,     0,     0,     0,
       0,   714,     0,   715,     0,     0,     0,     0,     0,     0,
       0,     0,   752,     0,     0,     0,     0,   360,   361,   362,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   763,
       0,     0,   766,   769,   770,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   752,
     385,     0,     0,   789,   758,     0,     0,     0,     0,     0,
       0,     0,   386,   971,   972,   973,     0,     0,   758,   758,
     758,   758,   758,     0,     0,     0,     0,     0,     0,     0,
       0,   758,   974,     0,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   996,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   860,
       0,   360,   361,   362,     0,     0,     0,     0,     0,     0,
       0,   871,   996,     0,     0,     0,     0,     0,     0,     0,
     363,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   879,   385,     0,     0,     0,     0,     0,
       0,     0,   196,     0,     0,   758,   386,     0,     0,   709,
       0,   360,   361,   362,     0,     0,     0,     0,     0,     0,
     889,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     363,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,     0,   385,     0,   752,     0,     0,     0,
       0,     0,     0,   243,     0,  1151,   386,     0,     0,     0,
     752,   752,   752,   752,   752,     0,     0,     0,     0,     0,
       0,     0,     0,   752,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   758,     0,   997,     0,   758,   758,   758,     0,
       0,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,   758,  1005,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,     0,     0,
    1034,   758,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   996,     0,  1042,     0,     0,     0,     0,     0,  1045,
       0,  1046,     0,  1047,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1001,  1002,     0,   752,     0,     0,
       0,     0,     0,     0,     0,  1062,     0,   360,   361,   362,
       0,     0,     0,     0,     0,  1070,     0,     0,  1071,     0,
    1072,     0,     0,     0,   609,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,     0,     0,     0,     0,     0,     0,     0,  1103,
       0,   758,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   758,     0,   758,     0,
      36,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   752,     0,     0,     0,   752,   752,
     752,   758,     0,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
       0,     0,     0,     0,     0,     0,     0,   759,     0,     0,
       0,  1184,  1185,  1186,     0,     0,     0,   766,  1188,     0,
       0,     0,     0,   752,     0,     0,     0,     0,     0,    84,
      85,     0,    86,   184,    88,     0,   758,     0,     0,     0,
       0,     0,    36,     0,   212,  1203,     0,     0,     0,     0,
       0,   562,     0,     0,   791,     0,     0,  1633,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,  1229,     0,     0,   677,     0,   117,
       0,     0,     0,   213,     0,     0,     0,   609,     0,     0,
       0,     0,     0,     0,     0,   758,   609,  1203,     0,     0,
     758,     0,   758,     0,   758,     0,     0,   758,     0,   758,
       0,     0,   758,     0,     0,   183,     0,     0,    82,    83,
       0,    84,    85,   752,    86,   184,    88,   243,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1271,   752,     0,
     752,     0,     0,     0,     0,     0,   758,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   752,   360,   361,   362,     0,     0,     0,
       0,   563,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   363,   758,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   386,
       0,     0,     0,  1327,     0,     0,     0,  1328,   752,  1329,
    1330,     0,     0,   758,     0,     0,     0,     0,   255,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1344,   609,
       0,   933,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   758,   758,   758,   256,   954,   955,   956,   957,   758,
       0,     0,     0,  1598,     0,     0,     0,     0,   968,     0,
       0,     0,     0,     0,     0,     0,    36,   752,     0,     0,
       0,     0,   752,     0,   752,     0,   752,     0,     0,   752,
       0,   752,     0,     0,   752,     0,     0,     0,     0,     0,
       0,     0,     0,   255,     0,  1382,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,     0,   752,   256,
       0,   257,   258,  1453,     0,     0,     0,     0,     0,     0,
     996,   609,     0,     0,     0,     0,     0,     0,     0,   183,
       0,    36,    82,   259,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,   752,   758,     0,     0,
       0,     0,     0,   260,     0,     0,     0,     0,     0,     0,
       0,     0,  1063,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     261,   758,     0,     0,  1628,     0,   257,   258,     0,     0,
       0,     0,   758,     0,     0,   752,     0,   758,     0,     0,
     758,     0,     0,     0,   183,     0,     0,    82,   259,     0,
      84,    85,     0,    86,   184,    88,     0,   949,     0,     0,
       0,     0,  1674,   752,   752,   752,     0,     0,   260,     0,
       0,   752,     0,     0,     0,     0,  1590,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,   261,     0,   758,     0,     0,
       0,     0,     0,  1144,  1147,  1147,     0,     0,  1154,  1157,
    1158,  1159,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,     0,    36,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   758,     0,     0,     0,     0,  1194,     0,
       0,   758,   360,   361,   362,     0,     0,   758,     0,     0,
     758,     0,     0,     0,     0,     0,     0,     0,     0,   752,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,   752,     0,    84,    85,   386,    86,   184,
      88,     0,     0,     0,   752,     0,     0,     0,     0,   752,
       0,     0,   752,     0,     0,     0,     0,     0,     0,     0,
     255,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,  1267,     0,
       0,     0,     0,   650,     0,   117,   256,     0,     0,     0,
       0,     0,     0,  1281,     0,  1282,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,   752,
       0,     0,     0,     0,     0,     0,     0,     0,  1300,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -326,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,   179,   180,   350,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
    1302,     0,     0,   257,   258,   752,     0,     0,     0,     0,
       0,     0,     0,   752,     0,     0,     0,     0,     0,   752,
       0,   183,   752,  1337,    82,   259,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   260,     0,     0,     0,     0,
       0,     0,     0,     0,   351,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     308,     0,   261,    84,    85,     0,    86,   184,    88,     0,
       0,     0,  1374,     0,     0,     0,     0,  1376,     0,  1377,
       0,  1378,     0,     0,  1379,     0,  1380,     0,     0,  1381,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   309,     0,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,  1424,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   456,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,  1449,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
    1568,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,   179,
     180,    64,     0,    65,    66,    67,     0,     0,  1587,  1588,
    1589,     0,     0,     0,    71,    72,  1594,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   972,   973,   114,     0,   115,   116,     0,   117,
     118,     0,   119,   120,     0,     0,     0,     0,     0,     0,
     974,     0,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,     0,  1620,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   996,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1643,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,  1653,
       0,     0,     0,     0,  1657,     0,     0,  1659,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,  1690,     0,     0,    49,    50,    51,
      52,    53,    54,    55,     0,    56,    57,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,    79,    80,    81,    82,    83,
    1742,    84,    85,     0,    86,    87,    88,    89,  1748,     0,
      90,     0,     0,    91,  1751,     0,     0,  1752,     0,    92,
      93,    94,    95,    96,     0,    97,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   115,   116,
    1035,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,    53,
      54,    55,     0,    56,    57,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,    79,    80,    81,    82,    83,     0,    84,
      85,     0,    86,    87,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,     0,    97,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,   115,   116,  1205,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
       0,     0,     0,    49,    50,    51,    52,    53,    54,    55,
       0,    56,    57,    58,    59,    60,    61,    62,    63,    64,
       0,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,    71,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,    79,    80,    81,    82,    83,     0,    84,    85,     0,
      86,    87,    88,    89,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
       0,    97,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   115,   116,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,     0,     0,
       0,    49,    50,    51,    52,     0,    54,    55,     0,    56,
       0,    58,    59,    60,    61,    62,    63,    64,     0,    65,
      66,    67,     0,    69,    70,     0,     0,     0,     0,     0,
      71,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,    89,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   115,   116,   590,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     115,   116,  1008,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,     0,     0,     0,    49,    50,    51,
      52,     0,    54,    55,     0,    56,     0,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,     0,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,    89,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   115,   116,
    1111,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,  1113,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,   115,   116,     0,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,  1268,     0,    48,
       0,     0,     0,    49,    50,    51,    52,     0,    54,    55,
       0,    56,     0,    58,    59,    60,    61,    62,    63,    64,
       0,    65,    66,    67,     0,    69,    70,     0,     0,     0,
       0,     0,    71,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,    89,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   115,   116,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,     0,     0,
       0,    49,    50,    51,    52,     0,    54,    55,     0,    56,
       0,    58,    59,    60,    61,    62,    63,    64,     0,    65,
      66,    67,     0,    69,    70,     0,     0,     0,     0,     0,
      71,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,    89,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   115,   116,  1384,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     115,   116,  1591,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,  1632,
      47,     0,     0,    48,     0,     0,     0,    49,    50,    51,
      52,     0,    54,    55,     0,    56,     0,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,     0,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,    89,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   115,   116,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,   115,   116,  1663,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
       0,     0,     0,    49,    50,    51,    52,     0,    54,    55,
       0,    56,     0,    58,    59,    60,    61,    62,    63,    64,
       0,    65,    66,    67,     0,    69,    70,     0,     0,     0,
       0,     0,    71,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,    89,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   115,   116,  1664,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,  1667,    46,     0,    47,     0,     0,    48,     0,     0,
       0,    49,    50,    51,    52,     0,    54,    55,     0,    56,
       0,    58,    59,    60,    61,    62,    63,    64,     0,    65,
      66,    67,     0,    69,    70,     0,     0,     0,     0,     0,
      71,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,    89,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   115,   116,     0,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     115,   116,  1682,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,     0,     0,     0,    49,    50,    51,
      52,     0,    54,    55,     0,    56,     0,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,     0,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,    89,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   115,   116,
    1736,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,   115,   116,  1743,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
       0,     0,     0,    49,    50,    51,    52,     0,    54,    55,
       0,    56,     0,    58,    59,    60,    61,    62,    63,    64,
       0,    65,    66,    67,     0,    69,    70,     0,     0,     0,
       0,     0,    71,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,    89,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   115,   116,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,   717,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,     0,     0,
       0,    49,    50,    51,    52,     0,    54,    55,     0,    56,
       0,    58,    59,    60,    61,   179,   180,    64,     0,    65,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
      71,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   115,   116,     0,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   935,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,   179,   180,    64,     0,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     115,   116,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,  1444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,     0,     0,     0,    49,    50,    51,
      52,     0,    54,    55,     0,    56,     0,    58,    59,    60,
      61,   179,   180,    64,     0,    65,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   115,   116,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1582,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,   179,
     180,    64,     0,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,   115,   116,     0,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
       0,     0,     0,    49,    50,    51,    52,     0,    54,    55,
       0,    56,     0,    58,    59,    60,    61,   179,   180,    64,
       0,    65,    66,    67,     0,     0,     0,     0,     0,     0,
       0,     0,    71,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   115,   116,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   659,    12,     0,     0,
       0,     0,     0,     0,   660,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,   179,   180,   181,     0,     0,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,   267,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,     0,     0,     0,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   179,   180,   181,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,   267,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     268,     0,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,   363,    10,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     605,   385,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,   386,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,   179,   180,   181,     0,     0,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,   606,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,     0,     0,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   181,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,   361,   362,   712,     0,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,   363,    10,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,  1060,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,   386,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,   179,   180,   181,
       0,     0,    66,    67,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,  1061,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,     0,     0,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   659,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,   179,   180,   181,     0,     0,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,    91,     5,     6,
       7,     8,     9,    92,    93,    94,    95,     0,    10,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,     0,     0,     0,   117,   118,     0,   119,   120,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   195,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,   179,   180,   181,     0,    36,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,    84,    85,   114,    86,   184,    88,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   221,   611,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   181,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     5,     6,     7,     8,     9,    92,    93,    94,
      95,     0,    10,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,     0,     0,     0,   117,
     118,     0,   119,   120,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,   179,   180,   181,     0,     0,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,    91,     5,     6,
       7,     8,     9,    92,    93,    94,    95,     0,    10,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   250,     0,     0,   117,   118,     0,   119,   120,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,   179,   180,   181,     0,     0,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
      90,     0,     0,    91,     5,     6,     7,     8,     9,    92,
      93,    94,    95,     0,    10,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,   253,     0,
       0,   117,   118,     0,   119,   120,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,   179,   180,   181,
       0,     0,    66,    67,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,     0,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,   454,     0,     0,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  -900,  -900,  -900,  -900,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   618,
     385,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   386,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,   179,   180,   181,     0,    36,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,   183,
      80,    81,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    84,    85,
     114,    86,   184,    88,     0,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   660,   869,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   179,   180,   181,     0,    36,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,    90,     0,   649,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    84,    85,   114,    86,
     184,    88,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     699,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,   179,   180,   181,     0,    36,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,   183,    80,    81,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
      90,     0,  1152,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,    84,    85,   114,    86,   184,    88,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   701,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   181,     0,    36,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,    84,    85,   114,    86,   184,    88,     0,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,  1102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,   179,   180,   181,
       0,     0,    66,    67,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,     0,    91,
       5,     6,     7,     8,     9,    92,    93,    94,    95,     0,
      10,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,     0,     0,     0,   117,   118,     0,
     119,   120,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   179,   180,   181,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,    90,     0,     0,    91,     5,     6,     7,     8,
       9,    92,    93,    94,    95,     0,    10,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
       0,     0,     0,   117,   118,     0,   119,   120,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,   548,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   181,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   183,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,     0,     0,     0,   117,
     118,     0,   119,   120,  1465,  1466,  1467,  1468,  1469,     0,
       0,  1470,  1471,  1472,  1473,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1474,  1475,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,  1476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   386,     0,     0,     0,  1477,  1478,
    1479,  1480,  1481,  1482,  1483,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1484,  1485,
    1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,
    1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,
    1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,  1515,
    1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,   255,
       0,     0,  1525,  1526,     0,  1527,  1528,  1529,  1530,  1531,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1532,  1533,  1534,     0,   256,     0,    84,    85,     0,
      86,   184,    88,  1535,     0,  1536,  1537,     0,  1538,     0,
       0,     0,     0,     0,     0,  1539,     0,    36,   255,  1540,
       0,  1541,     0,  1542,  1543,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,     0,     0,   256,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,     0,     0,     0,    36,     0,     0,     0,
       0,     0,   257,   258,     0,     0,   996,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,   259,     0,    84,    85,     0,    86,
     184,    88,     0,  1278,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,     0,     0,     0,     0,     0,
      36,   257,   258,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,   183,
       0,   261,    82,   259,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,  1403,     0,     0,     0,     0,
       0,  1160,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   737,   738,     0,
     261,     0,     0,   739,     0,   740,     0,     0,     0,    84,
      85,     0,    86,   184,    88,     0,     0,   741,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,     0,
       0,     0,     0,     0,     0,   929,     0,   742,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,  1404,     0,     0,     0,     0,     0,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,    36,   385,   212,
       0,     0,     0,   743,     0,    73,    74,    75,    76,    77,
     386,     0,     0,     0,     0,     0,   744,     0,     0,     0,
       0,   183,    80,    81,    82,   745,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,   213,     0,
       0,     0,     0,     0,     0,   746,   747,   748,   749,     0,
       0,   930,     0,    98,    36,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     183,     0,   750,    82,    83,     0,    84,    85,     0,    86,
     184,    88,   276,   277,     0,     0,     0,     0,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   996,   737,
     738,   214,     0,     0,     0,   739,   117,   740,     0,     0,
     278,     0,     0,    84,    85,     0,    86,   184,    88,   741,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,   742,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1085,  1086,  1087,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   743,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,   744,     0,
       0,     0,     0,   183,    80,    81,    82,   745,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    90,     0,
       0,     0,     0,     0,     0,     0,     0,   746,   747,   748,
     749,   898,   899,     0,     0,    98,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   900,    84,    85,   750,    86,   184,    88,     0,   901,
     902,   903,    36,     0,     0,     0,     0,     0,     0,     0,
       0,   904,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,     0,     0,     0,
       0,    29,    30,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,    38,     0,     0,     0,   905,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     906,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,    84,    85,     0,    86,   184,    88,    59,    60,    61,
     179,   180,   181,     0,     0,    29,    30,     0,     0,   907,
       0,     0,     0,     0,     0,    36,     0,   212,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   183,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,     0,
       0,     0,    91,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   436,     0,     0,   183,     0,
     117,    82,    83,   887,    84,    85,     0,    86,   184,    88,
       0,     0,    36,     0,   212,     0,    91,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   996,     0,     0,   436,
       0,     0,     0,   213,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   212,  -900,
    -900,  -900,  -900,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   183,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,   996,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,    36,   214,     0,     0,   183,
       0,   117,    82,    83,   730,    84,    85,     0,    86,   184,
      88,    36,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     214,     0,   213,   520,     0,   117,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   540,     0,     0,   183,     0,
       0,    82,     0,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,     0,   183,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   184,    88,    36,     0,   212,     0,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,   214,     0,   213,     0,     0,
     117,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1030,    36,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,    36,     0,   812,   813,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    36,     0,   212,
     214,     0,     0,     0,   183,   117,     0,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,    84,    85,   214,    86,   184,    88,     0,
     117,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,    83,     0,    84,    85,     0,    86,
     184,    88,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   360,   361,
     362,   227,     0,     0,     0,     0,   117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   363,     0,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   360,   361,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   363,   432,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   363,   442,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   360,   361,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   363,   838,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   363,   875,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   360,   361,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   363,   916,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   971,   972,   973,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   974,  1217,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   996,   971,   972,
     973,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   974,  1235,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
       0,     0,   971,   972,   973,     0,     0,     0,     0,     0,
       0,     0,     0,   996,     0,     0,     0,     0,     0,     0,
       0,   974,  1135,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   996,   971,   972,
     973,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   974,  1291,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
       0,     0,   971,   972,   973,     0,     0,     0,     0,     0,
       0,     0,     0,   996,     0,     0,     0,     0,     0,     0,
       0,   974,  1296,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,   996,   971,   972,
     973,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,   974,  1373,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   996,  1391,     0,    36,     0,     0,     0,
       0,     0,  1451,     0,     0,     0,   183,  1392,  1393,    82,
      83,    36,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    82,  1394,
       0,    84,    85,     0,    86,  1395,    88,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,    36,     0,  1452,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   347,    36,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,     0,   508,     0,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   512,     0,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     278,     0,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   360,   361,   362,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   721,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,   386,     0,     0,     0,     0,     0,     0,     0,
       0,   363,   872,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   722,   385,   360,   361,   362,     0,
       0,     0,     0,     0,     0,     0,     0,   386,     0,     0,
       0,     0,     0,     0,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,     0,   385,
     971,   972,   973,     0,     0,     0,     0,     0,     0,     0,
       0,   386,     0,     0,     0,     0,     0,     0,     0,   974,
    1301,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   971,   972,   973,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   996,     0,     0,     0,     0,
       0,   974,     0,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   362,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   996,     0,     0,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386
};

static const yytype_int16 yycheck[] =
{
       4,   163,   175,     4,   136,   608,   186,   316,     4,     4,
       4,    32,     4,     4,    89,    54,   426,   849,     4,    53,
     692,    96,    97,    44,  1053,   166,   580,    48,   420,   421,
     228,   399,   385,   163,   581,   720,   224,   452,   831,   555,
      26,    27,   234,   452,   114,   114,    50,  1037,   727,    53,
      30,  1050,   868,  1039,   114,   187,     9,     9,   450,    30,
     135,    79,     9,    45,     9,     4,    70,   456,   884,    79,
     928,     9,    45,    45,     9,    30,     9,     9,     9,     9,
     255,   256,    57,    87,     9,    89,   261,     9,     9,     9,
       9,     9,    96,    97,   235,     9,     9,     9,     9,     9,
       9,     9,     9,     9,    79,   128,   129,    82,    85,   301,
     109,    79,   105,  1576,    97,   504,   170,    45,    45,    33,
     113,   114,   115,   116,   117,   118,    66,   389,   114,   116,
      14,   135,    99,   100,   101,    71,    72,   124,     0,    29,
     136,    35,    66,   316,   214,   214,    30,   201,   201,    66,
     201,    66,    53,   163,   201,   417,    46,   227,     8,    49,
     422,    66,   163,    47,    65,   340,  1629,    66,   167,   146,
     153,   964,   352,    66,    66,    10,    11,    12,    66,    66,
     198,   149,   205,    79,    66,    79,    66,    66,   128,   129,
     183,   187,    66,    66,    29,   199,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   199,    53,   392,
     202,    66,    66,   203,   294,   294,   171,   199,   214,   202,
      65,  1230,   149,   204,   294,   221,   240,    66,    66,   171,
     244,   227,    79,   416,   248,   240,  1232,   435,   428,   244,
    1240,   203,   205,  1239,   240,  1241,   203,   204,   244,   938,
    1118,   940,   266,  1079,   204,   203,   398,   440,   203,   163,
     203,   346,   203,   203,   202,   202,   449,   201,   203,   452,
     204,   203,   203,   203,   203,   203,   201,   204,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   283,    79,   204,
     475,   276,   277,   278,  1097,   821,   292,   293,   294,   343,
     206,   204,   204,   299,   201,    79,   204,   204,   865,   305,
     395,   396,   204,   712,   204,   204,   330,    35,   717,   330,
     204,   204,    79,   308,   330,   339,    35,    35,    50,   343,
      79,  1340,   346,  1342,   330,    79,    85,   100,   101,   100,
     101,    85,   201,   128,   129,    79,   204,   201,   202,   204,
     399,   128,   129,    50,  1350,   117,   436,   436,   201,   206,
      85,    79,   124,   208,   201,   204,   204,   565,    79,    35,
      79,    79,   128,   129,   572,   324,   201,   575,   392,   393,
     394,   395,   396,    97,   586,   201,    26,    27,   808,   163,
      30,   397,   577,   578,   201,    79,    97,    66,   147,   148,
     585,    85,   416,   147,   148,    79,   163,     4,    79,   458,
      97,    85,    79,    79,    85,   206,  1098,   202,    85,  1114,
     385,   146,  1431,   201,   596,   202,   440,   392,   853,   163,
     426,   201,   154,   201,   853,    79,   147,   148,   452,   153,
     436,    85,   205,   651,   205,   163,   153,   169,    45,   463,
     648,   416,   153,   201,   163,   163,   596,   154,   463,   128,
     129,   210,   146,   147,   148,   209,   153,   463,   658,    97,
      97,    35,   680,   147,   148,   440,   147,   148,   154,   881,
     147,   148,  1285,   204,   449,    46,    47,   452,  1052,    79,
     698,   201,   917,    71,    72,   697,   203,   204,   917,   203,
     204,   703,   146,   147,   148,   519,   126,   127,  1065,   106,
     201,  1068,   831,   149,   111,    79,   113,   114,   115,   116,
     117,   118,   119,   508,   887,   153,   153,   512,   201,   614,
     544,   545,   517,   149,   170,   733,   935,   113,   114,   115,
     116,   117,   118,   751,   113,   114,   115,   116,   117,   118,
     735,   736,   703,    30,   170,  1358,   146,   147,   148,   156,
     157,   155,   159,   155,  1246,   201,  1248,   153,   204,   201,
    1579,    99,   100,   101,   214,    35,   168,   113,   114,   115,
     203,   221,   203,   204,    29,   596,   183,   227,   204,  1605,
    1606,   113,   114,   115,   116,   117,   118,    49,    50,    51,
     614,    46,   124,   125,    49,   735,   736,   183,   205,   605,
     204,    66,   204,    65,   183,  1601,  1602,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     203,   203,   581,   203,   203,   203,   203,   822,   160,    66,
     162,    66,   204,   283,   203,   964,   149,   201,   831,   118,
     119,   120,   292,   293,  1032,  1219,   201,    66,   149,   299,
     845,   183,   201,   153,   660,   305,    63,    64,  1707,   203,
     853,   856,  1074,    44,  1356,    65,   316,  1234,   692,   149,
     694,  1083,   170,  1722,   201,   716,    49,    50,    51,   302,
      53,     9,  1701,   306,   208,   149,   710,  1117,   149,   710,
     201,     8,    65,   699,   710,   701,   203,  1716,   170,   201,
     724,   725,    14,   911,   710,    14,   203,    79,   203,   332,
     725,   334,   335,   336,   337,   124,   722,    14,   124,   725,
     202,   128,   129,   170,   917,    14,    97,   202,   113,   114,
     115,   116,   117,   118,   202,   385,   202,   105,   207,   124,
     125,   201,   201,   201,     9,   953,  1438,   942,   202,   944,
     202,   775,   960,    89,   775,     9,    14,   781,   203,   775,
     201,   785,   786,     9,   187,    79,    79,   201,  1097,   775,
    1200,   964,    79,   190,   203,     9,   426,   162,   203,     9,
      79,   805,  1191,   203,  1351,   780,   436,   793,   202,   784,
     202,   886,   202,   126,   202,   202,   820,   201,   183,   820,
      66,    30,   808,   809,   820,   820,   820,   127,   820,   820,
     834,   169,   130,     9,   820,    46,    47,    48,    49,    50,
      51,     9,    53,   202,  1236,  1677,   149,   202,   202,   853,
      14,     9,   202,   199,    65,     9,   184,   185,   186,    66,
    1058,     9,  1694,   191,   192,   171,   202,   195,   196,  1258,
    1702,     9,    14,   126,   208,   208,  1051,     9,  1053,   205,
     201,   820,   886,   208,   202,  1274,   208,    46,    47,    48,
      49,    50,    51,   202,   898,   899,   900,   201,   853,    97,
     921,   202,  1077,   130,   203,  1093,    65,   203,   149,  1581,
     849,     9,   202,   917,   149,   201,   201,   201,   922,   201,
     924,   201,   926,   924,  1097,   926,   865,   922,   924,   149,
     926,   204,   887,   187,    14,   187,   922,     9,   924,    79,
     926,   945,   928,   929,   204,    14,  1338,   203,    14,   204,
     208,    14,   199,   204,  1142,   203,    30,  1132,  1146,   963,
      30,  1149,   917,    14,   201,   969,     4,     9,  1156,   201,
     201,  1360,   201,   201,   130,   605,  1285,   202,   201,   203,
    1369,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1032,  1383,   203,    14,     9,   202,     9,
      65,   146,   208,    79,    97,     9,    65,    45,  1012,   201,
     130,  1015,   203,    14,   202,    79,   201,   204,   201,   130,
    1623,  1196,   202,   204,   208,   204,     9,   146,    30,    73,
     660,   203,  1036,   202,   171,  1036,   203,    30,   130,   202,
    1036,  1036,  1036,   130,  1036,  1036,     9,   202,     9,  1358,
    1036,   202,   202,   202,     9,  1444,   202,     9,   205,   202,
      14,   205,    79,   204,     9,   201,  1254,   202,   106,   699,
     204,   701,   202,   111,  1060,   113,   114,   115,   116,   117,
     118,   119,   130,    30,   202,  1106,   201,   203,   202,  1692,
    1094,   203,   722,   202,  1098,   202,   202,  1036,   202,   204,
     203,    97,  1277,   158,  1279,  1109,    26,    27,   154,    14,
      30,     4,  1285,    79,  1109,   204,  1102,  1121,   156,   157,
    1121,   159,   111,  1109,   202,  1121,  1065,   202,    14,  1068,
     130,  1117,  1118,    53,   202,  1121,     4,   204,   130,    79,
      14,   203,    79,   201,  1319,   183,   202,   204,   202,   130,
     203,   203,    45,    14,    14,    14,   203,    55,    79,  1321,
     201,   204,    79,   793,     9,    79,   203,   205,   109,    97,
     149,    97,    14,   161,   201,    33,   202,    45,   808,   809,
     203,   201,  1571,   167,  1573,  1358,   164,  1319,    79,   202,
       9,  1195,    79,  1582,   203,   202,   202,  1198,    14,   204,
      79,   831,    14,    79,    14,    14,  1207,    79,    79,   780,
    1385,   784,   517,   106,  1200,  1685,   885,   395,   111,   396,
     113,   114,   115,   116,   117,   118,   119,   393,   882,  1409,
     823,  1698,  1115,  1443,  1271,  1694,  1463,  1626,   106,   522,
    1434,  1547,  1246,   111,  1248,   113,   114,   115,   116,   117,
     118,   119,  1314,  1726,  1714,  1559,    42,   887,  1430,  1198,
     399,  1004,  1007,   156,   157,  1081,   159,   772,  1207,  1040,
     899,   493,   493,   961,  1094,  1309,   913,   967,   300,   319,
      -1,   735,   293,  1023,   946,    -1,    -1,    -1,   156,   157,
     183,   159,    -1,    -1,   214,  1234,    -1,    -1,   928,   929,
    1304,   221,    -1,  1304,    -1,  1309,    -1,   227,  1304,    -1,
    1314,    -1,   205,    -1,    -1,   183,    -1,    -1,  1304,    -1,
    1321,    -1,  1318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1720,    -1,    -1,   964,   255,   256,   205,  1727,    -1,
    1341,   261,    -1,    -1,    -1,    -1,  1347,    -1,  1349,    -1,
      -1,    -1,  1356,    -1,    -1,    -1,    -1,  1361,    -1,    -1,
    1361,    -1,  1366,   283,    -1,  1361,  1370,    -1,    -1,  1370,
      -1,  1366,   292,   293,  1370,  1361,    -1,  1411,    -1,   299,
    1366,  1561,    -1,    -1,  1370,   305,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1398,    -1,   316,    -1,    -1,    -1,
    1404,    -1,  1341,    -1,    -1,    -1,    -1,  1411,  1347,    -1,
    1349,  1415,  1351,    74,    75,    76,    -1,  1413,    -1,    -1,
     340,    -1,    -1,   343,    -1,    86,    -1,    -1,    -1,    -1,
    1060,    -1,    -1,    -1,  1438,  1436,    -1,  1441,  1442,  1443,
    1441,    -1,    -1,  1447,    -1,  1441,  1447,  1442,  1443,    -1,
    1454,  1447,    -1,  1454,    -1,  1441,  1442,  1443,  1454,    -1,
      -1,  1447,    -1,    -1,    -1,   385,    -1,  1097,  1454,    -1,
      -1,    -1,  1102,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,  1558,   145,    -1,    -1,  1117,  1118,    -1,
     151,   152,    -1,    -1,    -1,    -1,    -1,  1436,    -1,    -1,
      -1,    -1,    -1,    -1,   165,    -1,   426,    -1,    -1,    -1,
      -1,  1673,    -1,    -1,    -1,    -1,   436,    -1,    26,    27,
      -1,   182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1707,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1617,    -1,    -1,    -1,    -1,  1722,    -1,    -1,
      -1,    -1,    -1,    -1,  1558,   475,   476,  1689,    -1,   479,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1200,    -1,    -1,  1577,    -1,    -1,    -1,  1581,    -1,    -1,
      -1,    -1,  1586,    -1,    -1,  1586,    -1,    -1,    -1,    -1,
    1586,    -1,  1596,    -1,    -1,    -1,    -1,  1601,  1602,    -1,
    1586,  1605,  1606,    -1,    -1,    -1,   526,    -1,    -1,    -1,
      -1,    -1,    -1,  1617,    -1,    -1,    -1,    -1,    -1,    -1,
    1624,  1625,    -1,  1624,  1625,    -1,    -1,  1631,  1624,  1625,
    1631,    -1,    -1,    -1,    -1,  1631,    -1,    -1,  1624,  1625,
      -1,    -1,    -1,    -1,    -1,  1631,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1285,    -1,   577,   578,     4,
      -1,  1665,    -1,    -1,  1665,   585,    -1,    -1,  1672,  1665,
      -1,    -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,  1665,
      -1,    -1,    -1,    -1,  1688,   605,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      45,    -1,    -1,    -1,    -1,    -1,   214,    -1,    -1,    -1,
      -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,    -1,   227,
      -1,    -1,    -1,    -1,  1728,    -1,    -1,  1728,  1358,    -1,
      -1,  1735,  1728,    -1,  1735,    63,    64,    -1,  1677,  1735,
     660,    -1,  1728,    -1,    -1,    -1,    -1,    -1,    -1,  1735,
      -1,    -1,     4,    -1,    -1,  1694,    -1,    -1,    -1,    -1,
      -1,   106,    -1,  1702,    -1,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   283,    -1,    -1,    -1,   699,
      -1,   701,    -1,    -1,   292,   293,   294,    -1,    -1,    -1,
      -1,   299,    -1,    45,    -1,    -1,    -1,   305,    -1,    -1,
     128,   129,   722,   723,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   156,   157,    -1,   159,   735,   736,   737,   738,   739,
     740,   741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     750,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,   776,    -1,    -1,   111,
     205,   113,   114,   115,   116,   117,   118,   119,    -1,    -1,
      -1,    -1,    -1,   793,   202,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   806,    -1,   808,   809,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   822,   823,   156,   157,    -1,   159,    -1,    -1,
      -1,   831,    -1,    -1,    -1,    -1,    -1,    -1,   426,    77,
      -1,    -1,    -1,    -1,    -1,   845,    -1,    -1,   436,    -1,
      -1,   183,    -1,    -1,    -1,    -1,   856,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   864,    -1,    -1,   867,    -1,    10,
      11,    12,    -1,   205,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,   154,    -1,   156,   157,
     158,   159,   160,   161,    65,    -1,    -1,    -1,   928,   929,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   942,    -1,   944,    -1,   946,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,   961,    -1,   201,   964,   965,   966,   967,    -1,    -1,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   605,    -1,    29,
    1020,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1051,    -1,  1053,    -1,    65,    -1,    -1,    -1,    -1,
    1060,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,   221,
      -1,    -1,   660,    -1,    -1,    -1,    -1,  1077,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,  1097,    -1,    -1,
      -1,    -1,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   699,    -1,   701,    -1,    -1,    -1,  1117,  1118,    -1,
    1120,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,
      -1,   283,  1132,    -1,   722,  1135,    -1,  1137,    -1,    -1,
     292,   293,    -1,    -1,    10,    11,    12,   299,    -1,    -1,
      -1,    -1,    -1,   305,    -1,    -1,    -1,    -1,    -1,    -1,
    1160,    -1,    -1,    29,   316,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,   128,   129,    -1,   205,  1196,  1197,    -1,    65,
    1200,    -1,    -1,    -1,    -1,   793,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
     808,   809,    -1,    -1,    -1,  1225,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   385,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    29,   426,    -1,    -1,  1277,    -1,  1279,
      -1,    -1,    -1,    -1,  1284,  1285,    -1,    -1,    -1,  1289,
      -1,  1291,    -1,  1293,    -1,    -1,  1296,    -1,  1298,    55,
      -1,  1301,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1309,
    1310,    -1,  1312,    -1,    -1,    -1,    -1,    -1,    -1,  1319,
      -1,    77,    -1,    -1,    -1,    -1,    -1,   479,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1335,    -1,    -1,    -1,   205,
     928,   929,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1358,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,  1373,   526,    -1,   132,   133,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1385,  1386,  1387,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    29,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,   205,    -1,
      -1,  1411,    -1,    -1,    -1,  1415,    -1,    -1,   174,    -1,
      -1,    55,  1422,    -1,    -1,    -1,    -1,    53,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    77,    -1,   201,    -1,    -1,    -1,   205,
    1450,  1451,  1452,   605,    10,    11,    12,    -1,  1458,  1459,
      -1,    -1,    -1,  1463,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,  1060,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   132,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   660,    65,
      -1,    -1,    -1,    -1,  1102,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,  1117,
    1118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,   699,    -1,   701,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,  1566,   201,    -1,    -1,
     722,   723,    -1,    -1,    -1,    -1,    -1,  1577,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   737,   738,   739,   740,   741,
      -1,    -1,    -1,    -1,    -1,    -1,  1596,    -1,   750,    -1,
    1600,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1611,  1200,    -1,    -1,    -1,  1616,    -1,    -1,  1619,
      -1,    -1,    -1,    -1,   776,    -1,    -1,    -1,    -1,   255,
     256,    -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,    -1,
      -1,   793,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,   806,    -1,   808,   809,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   823,    -1,    -1,    -1,    -1,  1676,    -1,    -1,   831,
      -1,    -1,    -1,    -1,  1684,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1698,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1707,    -1,    -1,
      -1,    65,   864,    -1,   340,   867,    -1,   343,    -1,    -1,
      -1,    -1,  1722,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,  1732,    -1,    -1,   887,    -1,    -1,    -1,    -1,
    1740,    -1,    -1,    -1,    -1,    -1,  1746,    -1,    29,  1749,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,   928,   929,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   961,
      -1,    -1,   964,   965,   966,   967,    -1,    -1,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   996,    63,    64,    -1,    -1,   475,
     476,    -1,    -1,   479,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1020,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
     526,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1060,    65,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   205,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1097,    53,    -1,    -1,    -1,
    1102,   577,   578,    -1,    -1,    -1,    -1,    -1,    65,   585,
      -1,    -1,    -1,    -1,    -1,  1117,  1118,    -1,  1120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1135,    -1,  1137,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1160,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1197,    65,    -1,  1200,   205,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,  1225,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,   723,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,   735,
     736,   737,   738,   739,   740,   741,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   750,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1284,  1285,    -1,    -1,    -1,  1289,    -1,  1291,
      -1,  1293,    12,    -1,  1296,    -1,  1298,    -1,    -1,  1301,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1310,    29,
    1312,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,  1335,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   822,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1358,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   845,
      -1,  1373,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     856,    -1,    -1,    -1,  1386,  1387,    -1,    -1,   864,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1422,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,  1450,  1451,
    1452,    -1,    -1,    -1,    -1,    -1,  1458,  1459,    65,    -1,
      -1,  1463,    -1,    -1,    -1,    -1,   942,    -1,   944,    -1,
     946,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   961,    -1,    -1,    -1,   965,
     966,   967,    -1,    -1,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1020,   150,    -1,    -1,   153,    -1,
      -1,   156,   157,    -1,   159,   160,   161,    65,    -1,    -1,
      -1,    -1,    -1,    -1,  1566,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1051,    -1,  1053,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    -1,   203,    -1,  1600,    -1,
     205,  1077,    10,    11,    12,    -1,    -1,    -1,    -1,  1611,
      -1,    -1,    -1,    -1,  1616,    -1,    -1,  1619,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1120,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1132,    65,    -1,  1135,
      -1,  1137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1676,    -1,    10,    11,    12,    -1,
      -1,    -1,  1684,    -1,  1160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,  1698,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
    1196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1732,    65,    10,    11,    12,    -1,    -1,    -1,  1740,    -1,
      -1,    -1,    -1,    -1,  1746,    -1,    -1,  1749,    -1,  1225,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,  1277,    -1,  1279,    -1,    -1,    -1,    -1,  1284,    -1,
      -1,    -1,    -1,  1289,    -1,  1291,    -1,  1293,    -1,    -1,
    1296,    -1,  1298,    -1,    -1,  1301,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1309,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1319,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1335,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   203,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,  1373,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,  1385,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1411,    -1,    -1,    -1,  1415,
      -1,    -1,    -1,    -1,    -1,    -1,  1422,    -1,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    28,    29,    -1,  1450,  1451,  1452,    -1,    -1,    10,
      11,    12,  1458,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    51,    -1,    -1,    54,    29,    56,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    10,    11,    12,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   114,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1566,    -1,    65,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,  1577,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1596,    53,    -1,    -1,  1600,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    65,   191,  1611,    -1,    -1,    -1,    -1,
    1616,    -1,    -1,  1619,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
      -1,    -1,   203,    -1,   204,    -1,   206,    -1,    -1,   236,
      -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,   246,
     247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1676,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     479,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,  1707,    -1,    65,    -1,    -1,    -1,   294,    -1,    -1,
      -1,    -1,    -1,   300,    -1,    -1,  1722,   304,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1732,    -1,    -1,    -1,
     317,   318,   319,    -1,  1740,    -1,    -1,   526,    -1,    -1,
    1746,    -1,    -1,  1749,   331,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   342,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,    -1,   386,
      -1,   388,   389,    -1,   391,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,    -1,    -1,    -1,    -1,    -1,
     417,   418,    -1,   420,   421,   422,   423,    -1,    -1,    -1,
      -1,    -1,   429,    -1,    -1,   432,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   442,    -1,   444,    -1,    -1,
      -1,    -1,    -1,   450,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   458,    -1,   460,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   479,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   486,
      -1,    -1,   489,   490,   491,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   526,
      53,    -1,    -1,   520,   723,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,   737,   738,
     739,   740,   741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   750,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   606,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   618,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   650,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   659,    -1,    -1,   864,    65,    -1,    -1,   202,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
     677,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,   723,    -1,    -1,    -1,
      -1,    -1,    -1,   720,    -1,   202,    65,    -1,    -1,    -1,
     737,   738,   739,   740,   741,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   750,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   961,    -1,   761,    -1,   965,   966,   967,    -1,
      -1,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   197,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
     817,  1020,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,   830,    -1,    -1,    -1,    -1,    -1,   836,
      -1,   838,    -1,   840,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,   194,    -1,   864,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   862,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   872,    -1,    -1,   875,    -1,
     877,    -1,    -1,    -1,   881,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   916,
      -1,  1120,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1135,    -1,  1137,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   961,    -1,    -1,    -1,   965,   966,
     967,  1160,    -1,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   479,    -1,    -1,
      -1,   998,   999,  1000,    -1,    -1,    -1,  1004,  1005,    -1,
      -1,    -1,    -1,  1020,    -1,    -1,    -1,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,  1225,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,  1032,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,   526,    -1,    -1,   190,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,  1061,    -1,    -1,   204,    -1,   206,
      -1,    -1,    -1,   118,    -1,    -1,    -1,  1074,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1284,  1083,  1084,    -1,    -1,
    1289,    -1,  1291,    -1,  1293,    -1,    -1,  1296,    -1,  1298,
      -1,    -1,  1301,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,  1120,   159,   160,   161,  1114,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1124,  1135,    -1,
    1137,    -1,    -1,    -1,    -1,    -1,  1335,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,  1160,    10,    11,    12,    -1,    -1,    -1,
      -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1373,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,  1210,    -1,    -1,    -1,  1214,  1225,  1216,
    1217,    -1,    -1,  1422,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1235,  1236,
      -1,   723,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1450,  1451,  1452,    55,   737,   738,   739,   740,  1458,
      -1,    -1,    -1,  1462,    -1,    -1,    -1,    -1,   750,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,  1284,    -1,    -1,
      -1,    -1,  1289,    -1,  1291,    -1,  1293,    -1,    -1,  1296,
      -1,  1298,    -1,    -1,  1301,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,  1302,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,  1335,    55,
      -1,   132,   133,   189,    -1,    -1,    -1,    -1,    -1,    -1,
      65,  1338,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
      -1,    77,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,  1373,  1566,    -1,    -1,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   864,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,  1600,    -1,    -1,   205,    -1,   132,   133,    -1,    -1,
      -1,    -1,  1611,    -1,    -1,  1422,    -1,  1616,    -1,    -1,
    1619,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,   163,    -1,    -1,
      -1,    -1,  1641,  1450,  1451,  1452,    -1,    -1,   174,    -1,
      -1,  1458,    -1,    -1,    -1,    -1,  1453,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,   201,    -1,  1676,    -1,    -1,
      -1,    -1,    -1,   965,   966,   967,    -1,    -1,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   996,    -1,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1732,    -1,    -1,    -1,    -1,  1020,    -1,
      -1,  1740,    10,    11,    12,    -1,    -1,  1746,    -1,    -1,
    1749,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1566,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1600,    -1,   156,   157,    65,   159,   160,
     161,    -1,    -1,    -1,  1611,    -1,    -1,    -1,    -1,  1616,
      -1,    -1,  1619,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,  1120,    -1,
      -1,    -1,    -1,   204,    -1,   206,    55,    -1,    -1,    -1,
      -1,    -1,    -1,  1135,    -1,  1137,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,  1676,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
     188,    -1,    -1,   132,   133,  1732,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1740,    -1,    -1,    -1,    -1,    -1,  1746,
      -1,   150,  1749,  1225,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
     153,    -1,   201,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,  1284,    -1,    -1,    -1,    -1,  1289,    -1,  1291,
      -1,  1293,    -1,    -1,  1296,    -1,  1298,    -1,    -1,  1301,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,  1335,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,  1373,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
    1422,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,    -1,  1450,  1451,
    1452,    -1,    -1,    -1,   131,   132,  1458,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    11,    12,   201,    -1,   203,   204,    -1,   206,
     207,    -1,   209,   210,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,  1566,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1600,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,  1611,
      -1,    -1,    -1,    -1,  1616,    -1,    -1,  1619,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,  1676,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
    1732,   156,   157,    -1,   159,   160,   161,   162,  1740,    -1,
     165,    -1,    -1,   168,  1746,    -1,    -1,  1749,    -1,   174,
     175,   176,   177,   178,    -1,   180,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,   204,
     205,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,   178,    -1,   180,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,   204,   205,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,   102,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,   178,
      -1,   180,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,   203,   204,   205,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
     203,   204,   205,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,    -1,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   162,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,   204,
     205,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,   204,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    96,    -1,    98,
      -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,   203,   204,   205,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
     203,   204,   205,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    94,
      95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,    -1,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   162,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,   204,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,   204,   205,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,   204,   205,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    92,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,   203,   204,    -1,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
     203,   204,   205,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,    -1,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   162,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,   204,
     205,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,   204,   205,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,   203,   204,    -1,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
     203,   204,    -1,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,    -1,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,   204,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,   204,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
     203,    -1,    -1,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      35,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,   163,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,    -1,    -1,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    11,    12,   204,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    29,    13,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    35,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,   163,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,   168,     3,     4,
       5,     6,     7,   174,   175,   176,   177,    -1,    13,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   206,   207,    -1,   209,   210,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    77,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   156,   157,   201,   159,   160,   161,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,    35,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,     3,     4,     5,     6,     7,   174,   175,   176,
     177,    -1,    13,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
     207,    -1,   209,   210,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,   168,     3,     4,
       5,     6,     7,   174,   175,   176,   177,    -1,    13,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,    -1,    -1,
     201,    -1,   203,    -1,    -1,   206,   207,    -1,   209,   210,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     165,    -1,    -1,   168,     3,     4,     5,     6,     7,   174,
     175,   176,   177,    -1,    13,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,   203,    -1,
      -1,   206,   207,    -1,   209,   210,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,   202,    -1,    -1,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    30,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,    -1,    77,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   156,   157,
     201,   159,   160,   161,    -1,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    35,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    77,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,   124,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   156,   157,   201,   159,
     160,   161,    -1,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    77,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     165,    -1,   124,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   156,   157,   201,   159,   160,   161,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    77,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   156,   157,   201,   159,   160,   161,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,
       3,     4,     5,     6,     7,   174,   175,   176,   177,    -1,
      13,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   207,    -1,
     209,   210,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,     3,     4,     5,     6,
       7,   174,   175,   176,   177,    -1,    13,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
      -1,    -1,    -1,   206,   207,    -1,   209,   210,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    29,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,   152,    -1,    55,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    77,    29,   178,
      -1,   180,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,    -1,    -1,    55,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
      77,   132,   133,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,   150,
      -1,   201,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    46,    47,    -1,
     201,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    86,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    77,    53,    79,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
      65,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,   118,    -1,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,   131,    -1,   182,    77,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
     150,    -1,   201,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   105,   106,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    65,    46,
      47,   201,    -1,    -1,    -1,    52,   206,    54,    -1,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   161,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    46,    47,    -1,    -1,   182,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    66,   156,   157,   201,   159,   160,   161,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,   156,   157,    -1,   159,   160,   161,   113,   114,   115,
     116,   117,   118,    -1,    -1,    67,    68,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,    -1,   150,    -1,
     206,   153,   154,    68,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    77,    -1,    79,    -1,   168,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    65,    -1,    -1,   201,
      -1,    -1,    -1,   118,   206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    77,   201,    -1,    -1,   150,
      -1,   206,   153,   154,    86,   156,   157,    -1,   159,   160,
     161,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    -1,   118,   204,    -1,   206,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    77,    -1,    79,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,   201,    -1,   118,    -1,    -1,
     206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,    -1,    77,    -1,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    77,    -1,    79,
     201,    -1,    -1,    -1,   150,   206,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,   156,   157,   201,   159,   160,   161,    -1,
     206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    10,    11,
      12,   201,    -1,    -1,    -1,    -1,   206,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   130,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   130,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   130,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   130,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   130,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   119,    -1,    77,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,   150,   132,   133,   153,
     154,    77,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    77,    -1,   130,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   154,    77,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   153,    -1,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    10,    11,    12,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    28,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    97,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   212,   213,     0,   214,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      82,    86,    87,    88,    89,    91,    93,    95,    98,   102,
     103,   104,   105,   106,   107,   108,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   120,   121,   122,   123,   124,
     125,   131,   132,   134,   135,   136,   137,   138,   145,   150,
     151,   152,   153,   154,   156,   157,   159,   160,   161,   162,
     165,   168,   174,   175,   176,   177,   178,   180,   182,   183,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   201,   203,   204,   206,   207,   209,
     210,   215,   218,   225,   226,   227,   228,   229,   230,   233,
     249,   250,   254,   257,   262,   268,   326,   327,   332,   336,
     337,   338,   339,   340,   341,   342,   343,   345,   348,   359,
     360,   361,   362,   363,   367,   368,   370,   389,   399,   400,
     401,   406,   409,   427,   433,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   446,   467,   469,   471,   116,
     117,   118,   131,   150,   160,   218,   249,   326,   342,   435,
     342,   201,   342,   342,   342,   102,   342,   342,   425,   426,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,    79,   118,   201,   226,   400,   401,   435,   438,
     435,    35,   342,   450,   451,   342,   118,   201,   226,   400,
     401,   402,   434,   442,   447,   448,   201,   333,   403,   201,
     333,   349,   334,   342,   235,   333,   201,   201,   201,   333,
     203,   342,   218,   203,   342,    29,    55,   132,   133,   154,
     174,   201,   218,   229,   472,   484,   485,   184,   203,   339,
     342,   369,   371,   204,   242,   342,   105,   106,   153,   219,
     222,   225,    79,   206,   294,   295,   117,   124,   116,   124,
      79,   296,   201,   201,   201,   201,   218,   266,   473,   201,
     201,    79,    85,   146,   147,   148,   464,   465,   153,   204,
     225,   225,   218,   267,   473,   154,   201,   201,   201,   201,
     473,   473,    79,   198,   351,   332,   342,   343,   435,   439,
     231,   204,    85,   404,   464,    85,   464,   464,    30,   153,
     170,   474,   201,     9,   203,    35,   248,   154,   265,   473,
     118,   183,   249,   327,   203,   203,   203,   203,   203,   203,
      10,    11,    12,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    53,    65,   203,    66,    66,
     203,   204,   149,   125,   160,   162,   268,   325,   326,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    63,    64,   128,   129,   429,    66,   204,   432,
     201,   201,    66,   204,   206,   443,   201,   248,   249,    14,
     342,   203,   130,    44,   218,   424,   201,   332,   435,   439,
     149,   435,   130,   208,     9,   411,   332,   435,   474,   149,
     201,   405,   429,   432,   202,   342,    30,   233,     8,   353,
       9,   203,   233,   234,   334,   335,   342,   218,   280,   237,
     203,   203,   203,   485,   485,   170,   201,   105,   485,    14,
     218,    79,   203,   203,   203,   184,   185,   186,   191,   192,
     195,   196,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   384,   385,   386,   243,   109,   167,   203,   153,   220,
     223,   225,   153,   221,   224,   225,   225,     9,   203,    97,
     204,   435,     9,   203,   124,   124,    14,     9,   203,   435,
     468,   468,   332,   343,   435,   438,   439,   202,   170,   260,
     131,   435,   449,   450,    66,   429,   146,   465,    78,   342,
     435,    85,   146,   465,   225,   217,   203,   204,   255,   263,
     390,   392,    86,   206,   354,   355,   357,   401,   443,   469,
     342,   457,   459,   342,   456,   458,   456,    14,    97,   470,
     350,   352,   290,   291,   427,   428,   202,   202,   202,   202,
     205,   232,   233,   250,   257,   262,   427,   342,   207,   209,
     210,   218,   475,   476,   485,    35,   163,   292,   293,   342,
     472,   201,   473,   258,   248,   342,   342,   342,    30,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   402,   342,   342,   445,   445,   342,   452,   453,   124,
     204,   218,   442,   443,   266,   218,   267,   265,   249,    27,
      35,   336,   339,   342,   369,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   204,   218,   430,
     431,   442,   445,   342,   292,   292,   445,   342,   449,   248,
     202,   342,   201,   423,     9,   411,   332,   202,   218,    35,
     342,    35,   342,   202,   202,   442,   292,   430,   431,   202,
     231,   284,   204,   339,   342,   342,    89,    30,   233,   278,
     203,    28,    97,    14,     9,   202,    30,   204,   281,   485,
      86,   229,   481,   482,   483,   201,     9,    46,    47,    52,
      54,    66,    86,   132,   145,   154,   174,   175,   176,   177,
     201,   226,   227,   229,   364,   365,   366,   400,   406,   407,
     408,   187,    79,   342,    79,    79,   342,   381,   382,   342,
     342,   374,   384,   190,   387,   231,   201,   241,   225,   203,
       9,    97,   225,   203,     9,    97,    97,   222,   218,   342,
     295,   407,    79,     9,   202,   202,   202,   202,   202,   202,
     202,   203,    46,    47,   479,   480,   126,   271,   201,     9,
     202,   202,    79,    80,   218,   466,   218,    66,   205,   205,
     214,   216,    30,   127,   270,   169,    50,   154,   169,   394,
     130,     9,   411,   202,   149,   202,     9,   411,   130,   202,
       9,   411,   202,   485,   485,    14,   353,   290,   199,     9,
     412,   485,   486,   429,   432,   205,     9,   411,   171,   435,
     342,   202,     9,   412,    14,   346,   251,   126,   269,   201,
     473,   342,    30,   208,   208,   130,   205,     9,   411,   342,
     474,   201,   261,   256,   264,   259,   248,    68,   435,   342,
     474,   208,   205,   202,   202,   208,   205,   202,    46,    47,
      66,    74,    75,    76,    86,   132,   145,   174,   218,   414,
     416,   419,   422,   218,   435,   435,   130,   429,   432,   202,
     285,    71,    72,   286,   231,   333,   231,   335,    97,    35,
     131,   275,   435,   407,   218,    30,   233,   279,   203,   282,
     203,   282,     9,   171,   130,   149,     9,   411,   202,   163,
     475,   476,   477,   475,   407,   407,   407,   407,   407,   410,
     413,   201,    85,   149,   201,   201,   201,   201,   407,   149,
     204,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    65,   342,   187,   187,
      14,   193,   194,   383,     9,   197,   387,    79,   205,   400,
     204,   245,    97,   223,   218,    97,   224,   218,   218,   205,
      14,   435,   203,     9,   171,   218,   272,   400,   204,   449,
     131,   435,    14,   208,   342,   205,   214,   485,   272,   204,
     393,    14,   342,   354,   218,   342,   342,   342,   203,   485,
     199,    30,   478,   428,    35,    79,   163,   430,   431,   485,
      35,   163,   342,   407,   290,   201,   400,   270,   347,   252,
     342,   342,   342,   205,   201,   292,   271,    30,   270,   269,
     473,   402,   205,   201,    14,    74,    75,    76,   218,   415,
     415,   416,   417,   418,   201,    85,   146,   201,     9,   411,
     202,   423,    35,   342,   430,   431,    71,    72,   287,   333,
     233,   205,   203,    90,   203,   275,   435,   201,   130,   274,
      14,   231,   282,    99,   100,   101,   282,   205,   485,   485,
     218,   481,     9,   202,   411,   130,   208,     9,   411,   410,
     218,   354,   356,   358,   407,   461,   463,   407,   460,   462,
     460,   202,   124,   218,   407,   454,   455,   407,   407,   407,
      30,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   342,   342,   342,   382,   342,   372,
      79,   246,   218,   218,   407,   480,    97,     9,   300,   202,
     201,   336,   339,   342,   208,   205,   470,   300,   155,   168,
     204,   389,   396,   155,   204,   395,   130,   130,   203,   478,
     485,   353,   486,    79,   163,    14,    79,   474,   435,   342,
     202,   290,   204,   290,   201,   130,   201,   292,   202,   204,
     485,   204,   270,   253,   405,   292,   130,   208,     9,   411,
     417,   146,   354,   420,   421,   416,   435,   333,    30,    73,
     233,   203,   335,   274,   449,   275,   202,   407,    96,    99,
     203,   342,    30,   203,   283,   205,   171,   130,   163,    30,
     202,   407,   407,   202,   130,     9,   411,   202,   202,     9,
     411,   130,   202,     9,   411,   202,   130,   205,     9,   411,
     407,    30,   188,   202,   231,   218,   485,   400,     4,   106,
     111,   119,   156,   157,   159,   205,   301,   324,   325,   326,
     331,   427,   449,   205,   204,   205,    50,   342,   342,   342,
     342,   353,    35,    79,   163,    14,    79,   407,   201,   478,
     202,   300,   202,   290,   342,   292,   202,   300,   470,   300,
     204,   201,   202,   416,   416,   202,   130,   202,     9,   411,
      30,   231,   203,   202,   202,   202,   238,   203,   203,   283,
     231,   485,   485,   130,   407,   354,   407,   407,   407,   407,
     407,   407,   342,   204,   205,    97,   126,   127,   472,   273,
     400,   119,   132,   133,   154,   160,   310,   311,   312,   400,
     158,   316,   317,   122,   201,   218,   318,   319,   302,   249,
     485,     9,   203,   325,   202,   297,   154,   391,   205,   205,
      79,   163,    14,    79,   407,   292,   111,   344,   478,   205,
     478,   202,   202,   205,   204,   205,   300,   290,   130,   416,
     354,   231,   236,   239,    30,   233,   277,   231,   202,   407,
     130,   130,   130,   189,   231,   485,   400,   400,    14,     9,
     203,   204,   204,     9,   203,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    53,    67,    68,    69,
      70,    71,    72,    73,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   131,   132,   134,   135,   136,
     137,   138,   150,   151,   152,   162,   164,   165,   167,   174,
     178,   180,   182,   183,   218,   397,   398,     9,   203,   154,
     158,   218,   319,   320,   321,   203,    79,   330,   248,   303,
     472,   249,   205,   298,   299,   472,    14,    79,   407,   202,
     201,   204,   203,   204,   322,   344,   478,   297,   205,   202,
     416,   130,    30,   233,   276,   277,   231,   407,   407,   407,
     342,   205,   203,   203,   407,   400,   306,   313,   406,   311,
      14,    30,    47,   314,   317,     9,    33,   202,    29,    46,
      49,    14,     9,   203,   473,   330,    14,   248,   203,    14,
     407,    35,    79,   388,   231,   231,   204,   322,   205,   478,
     416,   231,    94,   190,   244,   205,   218,   229,   307,   308,
     309,     9,   205,   407,   398,   398,    55,   315,   320,   320,
      29,    46,    49,   407,    79,   201,   203,   407,   473,   407,
      79,     9,   412,   205,   205,   231,   322,    92,   203,    79,
     109,   240,   149,    97,   406,   161,    14,   304,   201,    35,
      79,   202,   205,   203,   201,   167,   247,   218,   325,   326,
     407,   288,   289,   428,   305,    79,   400,   245,   164,   218,
     203,   202,     9,   412,   113,   114,   115,   328,   329,   288,
      79,   273,   203,   478,   428,   486,   202,   202,   203,   203,
     204,   323,   328,    35,    79,   163,   478,   204,   231,   486,
      79,   163,    14,    79,   323,   231,   205,    35,    79,   163,
      14,    79,   407,   205,    79,   163,    14,    79,   407,    14,
      79,   407,   407
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
#line 737 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 858 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { (yyval).reset();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
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

  case 145:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
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

  case 147:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval).reset();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { (yyval).reset();;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { (yyval).reset();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval).reset();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval).reset();;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval).reset();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval).reset();;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval).reset();;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval).reset();;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { Token v; Token w; Token x;
                                         (yyvsp[(1) - (4)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (4)]), nullptr, (yyvsp[(1) - (4)]));
                                         _p->finishStatement((yyvsp[(4) - (4)]), (yyvsp[(4) - (4)])); (yyvsp[(4) - (4)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (4)]),
                                                            v,(yyvsp[(2) - (4)]),w,(yyvsp[(4) - (4)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { Token u; Token v;
                                         (yyvsp[(1) - (7)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (7)]), nullptr, (yyvsp[(1) - (7)]));
                                         _p->finishStatement((yyvsp[(7) - (7)]), (yyvsp[(7) - (7)])); (yyvsp[(7) - (7)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (7)]),
                                                            u,(yyvsp[(4) - (7)]),v,(yyvsp[(7) - (7)]),(yyvsp[(6) - (7)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
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

  case 525:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
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

  case 526:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval).reset();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval).reset();;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval).reset();;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval).reset();;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval).reset();;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset();;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval).reset();;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval).reset();;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval)++;;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval).reset();;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
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
    { _p->onEmptyCollection((yyval));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    {;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    {;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    {;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    {;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13679 "hphp.tab.cpp"
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
#line 3067 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

