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
#define YYLAST   16404

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  274
/* YYNRULES -- Number of rules.  */
#define YYNRULES  920
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1727

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
     563,   566,   569,   570,   573,   574,   577,   578,   580,   584,
     586,   590,   593,   594,   596,   599,   604,   606,   611,   613,
     618,   620,   625,   627,   632,   636,   642,   646,   651,   656,
     662,   668,   673,   674,   676,   678,   683,   684,   690,   691,
     694,   695,   699,   700,   708,   715,   718,   724,   729,   730,
     735,   741,   749,   756,   763,   771,   781,   790,   797,   803,
     806,   811,   815,   816,   820,   825,   832,   838,   844,   851,
     860,   868,   871,   872,   874,   877,   880,   884,   889,   894,
     898,   900,   902,   905,   910,   914,   920,   922,   926,   929,
     930,   933,   937,   940,   941,   942,   947,   948,   954,   957,
     958,   969,   970,   982,   986,   990,   994,   999,  1004,  1008,
    1014,  1017,  1020,  1021,  1028,  1034,  1039,  1043,  1045,  1047,
    1051,  1056,  1058,  1060,  1062,  1064,  1069,  1071,  1073,  1077,
    1080,  1081,  1084,  1085,  1087,  1091,  1093,  1095,  1097,  1099,
    1103,  1108,  1113,  1118,  1120,  1122,  1125,  1128,  1131,  1135,
    1139,  1141,  1143,  1145,  1147,  1151,  1153,  1157,  1159,  1161,
    1163,  1164,  1166,  1169,  1171,  1173,  1175,  1177,  1179,  1181,
    1183,  1185,  1186,  1188,  1190,  1192,  1196,  1202,  1204,  1208,
    1214,  1219,  1223,  1227,  1230,  1232,  1234,  1238,  1242,  1244,
    1246,  1247,  1250,  1255,  1259,  1266,  1269,  1273,  1280,  1282,
    1284,  1286,  1293,  1297,  1302,  1309,  1313,  1317,  1321,  1325,
    1329,  1333,  1337,  1341,  1345,  1349,  1353,  1357,  1360,  1363,
    1366,  1369,  1373,  1377,  1381,  1385,  1389,  1393,  1397,  1401,
    1405,  1409,  1413,  1417,  1421,  1425,  1429,  1433,  1437,  1440,
    1443,  1446,  1449,  1453,  1457,  1461,  1465,  1469,  1473,  1477,
    1481,  1485,  1489,  1495,  1500,  1502,  1505,  1508,  1511,  1514,
    1517,  1520,  1523,  1526,  1529,  1531,  1533,  1535,  1537,  1539,
    1543,  1546,  1548,  1550,  1552,  1558,  1559,  1560,  1572,  1573,
    1586,  1587,  1591,  1592,  1597,  1598,  1605,  1606,  1614,  1617,
    1620,  1625,  1627,  1629,  1635,  1639,  1645,  1649,  1652,  1653,
    1656,  1657,  1662,  1667,  1671,  1676,  1681,  1686,  1691,  1696,
    1701,  1706,  1711,  1716,  1721,  1723,  1725,  1727,  1731,  1734,
    1738,  1743,  1746,  1750,  1752,  1755,  1757,  1760,  1762,  1764,
    1766,  1768,  1770,  1772,  1777,  1782,  1785,  1794,  1805,  1808,
    1810,  1814,  1816,  1819,  1821,  1823,  1825,  1827,  1830,  1835,
    1839,  1843,  1848,  1850,  1853,  1858,  1861,  1868,  1869,  1871,
    1876,  1877,  1880,  1881,  1883,  1885,  1889,  1891,  1895,  1897,
    1899,  1903,  1907,  1909,  1911,  1913,  1915,  1917,  1919,  1921,
    1923,  1925,  1927,  1929,  1931,  1933,  1935,  1937,  1939,  1941,
    1943,  1945,  1947,  1949,  1951,  1953,  1955,  1957,  1959,  1961,
    1963,  1965,  1967,  1969,  1971,  1973,  1975,  1977,  1979,  1981,
    1983,  1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,
    2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,
    2023,  2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,
    2043,  2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,
    2063,  2065,  2067,  2072,  2074,  2076,  2078,  2080,  2082,  2084,
    2086,  2088,  2091,  2093,  2094,  2095,  2097,  2099,  2103,  2104,
    2106,  2108,  2110,  2112,  2114,  2116,  2118,  2120,  2122,  2124,
    2126,  2128,  2130,  2134,  2137,  2139,  2141,  2146,  2150,  2155,
    2157,  2159,  2161,  2163,  2167,  2171,  2175,  2179,  2183,  2187,
    2191,  2195,  2199,  2203,  2207,  2211,  2215,  2219,  2223,  2227,
    2231,  2235,  2238,  2241,  2244,  2247,  2251,  2255,  2259,  2263,
    2267,  2271,  2275,  2279,  2285,  2290,  2294,  2298,  2302,  2304,
    2306,  2308,  2310,  2314,  2318,  2322,  2325,  2326,  2328,  2329,
    2331,  2332,  2338,  2342,  2346,  2348,  2350,  2352,  2354,  2356,
    2360,  2363,  2365,  2367,  2369,  2371,  2373,  2375,  2378,  2381,
    2386,  2390,  2395,  2398,  2399,  2405,  2409,  2413,  2415,  2419,
    2421,  2424,  2425,  2431,  2435,  2438,  2439,  2443,  2444,  2449,
    2452,  2453,  2457,  2461,  2463,  2464,  2466,  2468,  2470,  2474,
    2476,  2478,  2482,  2486,  2489,  2494,  2497,  2502,  2504,  2506,
    2508,  2510,  2512,  2516,  2522,  2526,  2531,  2535,  2537,  2539,
    2541,  2543,  2547,  2553,  2558,  2562,  2564,  2566,  2570,  2578,
    2588,  2596,  2603,  2612,  2614,  2617,  2622,  2627,  2629,  2631,
    2636,  2638,  2639,  2641,  2644,  2646,  2648,  2652,  2658,  2662,
    2666,  2667,  2669,  2673,  2679,  2683,  2686,  2690,  2697,  2698,
    2700,  2705,  2708,  2709,  2715,  2719,  2723,  2725,  2732,  2737,
    2742,  2745,  2748,  2749,  2755,  2759,  2763,  2765,  2768,  2769,
    2775,  2779,  2783,  2785,  2788,  2789,  2792,  2793,  2799,  2803,
    2807,  2809,  2812,  2813,  2816,  2817,  2823,  2827,  2831,  2833,
    2836,  2839,  2841,  2844,  2846,  2851,  2855,  2859,  2866,  2870,
    2872,  2874,  2876,  2881,  2886,  2891,  2896,  2899,  2902,  2907,
    2910,  2913,  2915,  2919,  2923,  2927,  2928,  2931,  2937,  2944,
    2946,  2949,  2951,  2956,  2960,  2961,  2963,  2967,  2970,  2974,
    2976,  2978,  2979,  2980,  2983,  2988,  2991,  2998,  3003,  3005,
    3007,  3008,  3012,  3018,  3022,  3024,  3027,  3028,  3033,  3036,
    3039,  3041,  3043,  3045,  3047,  3052,  3059,  3061,  3070,  3077,
    3079
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
     116,   123,    -1,   115,   123,    -1,   125,   399,    -1,    -1,
     126,   271,    -1,    -1,   125,   271,    -1,    -1,   399,    -1,
     271,     9,   399,    -1,   399,    -1,   272,     9,   399,    -1,
     129,   274,    -1,    -1,   434,    -1,    35,   434,    -1,   130,
     200,   446,   201,    -1,   232,    -1,    30,   230,    91,   202,
      -1,   232,    -1,    30,   230,    93,   202,    -1,   232,    -1,
      30,   230,    89,   202,    -1,   232,    -1,    30,   230,    95,
     202,    -1,   217,    14,   406,    -1,   279,     9,   217,    14,
     406,    -1,   203,   281,   204,    -1,   203,   202,   281,   204,
      -1,    30,   281,    98,   202,    -1,    30,   202,   281,    98,
     202,    -1,   281,    99,   341,   282,   230,    -1,   281,   100,
     282,   230,    -1,    -1,    30,    -1,   202,    -1,   283,    71,
     332,   232,    -1,    -1,   284,    71,   332,    30,   230,    -1,
      -1,    72,   232,    -1,    -1,    72,    30,   230,    -1,    -1,
     288,     9,   427,   327,   483,   162,    79,    -1,   288,     9,
     427,   327,   483,   162,    -1,   288,   411,    -1,   427,   327,
     483,   162,    79,    -1,   427,   327,   483,   162,    -1,    -1,
     427,   327,   483,    79,    -1,   427,   327,   483,    35,    79,
      -1,   427,   327,   483,    35,    79,    14,   406,    -1,   427,
     327,   483,    79,    14,   406,    -1,   288,     9,   427,   327,
     483,    79,    -1,   288,     9,   427,   327,   483,    35,    79,
      -1,   288,     9,   427,   327,   483,    35,    79,    14,   406,
      -1,   288,     9,   427,   327,   483,    79,    14,   406,    -1,
     290,     9,   427,   483,   162,    79,    -1,   290,     9,   427,
     483,   162,    -1,   290,   411,    -1,   427,   483,   162,    79,
      -1,   427,   483,   162,    -1,    -1,   427,   483,    79,    -1,
     427,   483,    35,    79,    -1,   427,   483,    35,    79,    14,
     406,    -1,   427,   483,    79,    14,   406,    -1,   290,     9,
     427,   483,    79,    -1,   290,     9,   427,   483,    35,    79,
      -1,   290,     9,   427,   483,    35,    79,    14,   406,    -1,
     290,     9,   427,   483,    79,    14,   406,    -1,   292,   411,
      -1,    -1,   341,    -1,    35,   434,    -1,   162,   341,    -1,
     292,     9,   341,    -1,   292,     9,   162,   341,    -1,   292,
       9,    35,   434,    -1,   293,     9,   294,    -1,   294,    -1,
      79,    -1,   205,   434,    -1,   205,   203,   341,   204,    -1,
     295,     9,    79,    -1,   295,     9,    79,    14,   406,    -1,
      79,    -1,    79,    14,   406,    -1,   296,   297,    -1,    -1,
     298,   202,    -1,   469,    14,   406,    -1,   299,   300,    -1,
      -1,    -1,   323,   301,   329,   202,    -1,    -1,   325,   482,
     302,   329,   202,    -1,   330,   202,    -1,    -1,   324,   248,
     247,   470,   200,   303,   287,   201,   475,   322,    -1,    -1,
     426,   324,   248,   247,   470,   200,   304,   287,   201,   475,
     322,    -1,   155,   309,   202,    -1,   156,   315,   202,    -1,
     158,   317,   202,    -1,     4,   125,   399,   202,    -1,     4,
     126,   399,   202,    -1,   110,   272,   202,    -1,   110,   272,
     203,   305,   204,    -1,   305,   306,    -1,   305,   307,    -1,
      -1,   228,   148,   217,   163,   272,   202,    -1,   308,    96,
     324,   217,   202,    -1,   308,    96,   325,   202,    -1,   228,
     148,   217,    -1,   217,    -1,   310,    -1,   309,     9,   310,
      -1,   311,   396,   313,   314,    -1,   153,    -1,   131,    -1,
     399,    -1,   118,    -1,   159,   203,   312,   204,    -1,   132,
      -1,   405,    -1,   312,     9,   405,    -1,    14,   406,    -1,
      -1,    55,   160,    -1,    -1,   316,    -1,   315,     9,   316,
      -1,   157,    -1,   318,    -1,   217,    -1,   121,    -1,   200,
     319,   201,    -1,   200,   319,   201,    49,    -1,   200,   319,
     201,    29,    -1,   200,   319,   201,    46,    -1,   318,    -1,
     320,    -1,   320,    49,    -1,   320,    29,    -1,   320,    46,
      -1,   319,     9,   319,    -1,   319,    33,   319,    -1,   217,
      -1,   153,    -1,   157,    -1,   202,    -1,   203,   230,   204,
      -1,   202,    -1,   203,   230,   204,    -1,   325,    -1,   118,
      -1,   325,    -1,    -1,   326,    -1,   325,   326,    -1,   112,
      -1,   113,    -1,   114,    -1,   117,    -1,   116,    -1,   115,
      -1,   182,    -1,   328,    -1,    -1,   112,    -1,   113,    -1,
     114,    -1,   329,     9,    79,    -1,   329,     9,    79,    14,
     406,    -1,    79,    -1,    79,    14,   406,    -1,   330,     9,
     469,    14,   406,    -1,   105,   469,    14,   406,    -1,   200,
     331,   201,    -1,    68,   401,   404,    -1,    67,   341,    -1,
     388,    -1,   360,    -1,   200,   341,   201,    -1,   333,     9,
     341,    -1,   341,    -1,   333,    -1,    -1,    27,   341,    -1,
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
    1170,  1173,  1175,  1178,  1179,  1182,  1183,  1186,  1187,  1191,
    1192,  1197,  1198,  1201,  1202,  1203,  1207,  1208,  1212,  1213,
    1217,  1218,  1222,  1223,  1228,  1229,  1234,  1235,  1236,  1237,
    1240,  1243,  1245,  1248,  1249,  1253,  1255,  1258,  1261,  1264,
    1265,  1268,  1269,  1273,  1279,  1286,  1288,  1293,  1299,  1303,
    1307,  1311,  1316,  1321,  1326,  1331,  1337,  1346,  1351,  1357,
    1359,  1363,  1368,  1372,  1375,  1378,  1382,  1386,  1390,  1394,
    1399,  1407,  1409,  1412,  1413,  1414,  1415,  1417,  1419,  1424,
    1425,  1428,  1429,  1430,  1434,  1435,  1437,  1438,  1442,  1444,
    1447,  1451,  1457,  1459,  1462,  1462,  1466,  1465,  1469,  1473,
    1471,  1486,  1483,  1496,  1498,  1500,  1502,  1504,  1506,  1508,
    1512,  1513,  1514,  1517,  1523,  1526,  1532,  1535,  1540,  1542,
    1547,  1552,  1556,  1557,  1563,  1564,  1566,  1570,  1571,  1576,
    1577,  1581,  1582,  1586,  1588,  1594,  1599,  1600,  1602,  1606,
    1607,  1608,  1609,  1613,  1614,  1615,  1616,  1617,  1618,  1620,
    1625,  1628,  1629,  1633,  1634,  1638,  1639,  1642,  1643,  1646,
    1647,  1650,  1651,  1655,  1656,  1657,  1658,  1659,  1660,  1661,
    1665,  1666,  1669,  1670,  1671,  1674,  1676,  1678,  1679,  1682,
    1684,  1689,  1690,  1692,  1693,  1694,  1697,  1701,  1702,  1706,
    1707,  1711,  1712,  1716,  1720,  1725,  1729,  1733,  1738,  1739,
    1740,  1743,  1745,  1746,  1747,  1750,  1751,  1752,  1753,  1754,
    1755,  1756,  1757,  1758,  1759,  1760,  1761,  1762,  1763,  1764,
    1765,  1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,  1774,
    1775,  1776,  1777,  1778,  1779,  1780,  1781,  1782,  1783,  1784,
    1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,  1794,  1795,
    1797,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1821,  1825,  1829,  1834,  1833,  1848,  1846,
    1863,  1863,  1879,  1878,  1896,  1896,  1912,  1911,  1932,  1933,
    1934,  1939,  1941,  1945,  1949,  1955,  1959,  1965,  1967,  1971,
    1973,  1977,  1981,  1982,  1986,  1993,  1994,  1998,  2002,  2004,
    2009,  2014,  2021,  2023,  2028,  2029,  2030,  2032,  2036,  2040,
    2044,  2048,  2050,  2052,  2054,  2059,  2060,  2065,  2066,  2067,
    2068,  2069,  2070,  2074,  2078,  2082,  2086,  2091,  2096,  2100,
    2101,  2105,  2106,  2110,  2111,  2115,  2116,  2120,  2124,  2128,
    2132,  2133,  2134,  2135,  2139,  2145,  2154,  2167,  2168,  2171,
    2174,  2177,  2178,  2181,  2185,  2188,  2191,  2198,  2199,  2203,
    2204,  2206,  2210,  2211,  2212,  2213,  2214,  2215,  2216,  2217,
    2218,  2219,  2220,  2221,  2222,  2223,  2224,  2225,  2226,  2227,
    2228,  2229,  2230,  2231,  2232,  2233,  2234,  2235,  2236,  2237,
    2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,  2246,  2247,
    2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,  2256,  2257,
    2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,  2266,  2267,
    2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,  2277,
    2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,  2287,
    2288,  2289,  2293,  2298,  2299,  2302,  2303,  2304,  2308,  2309,
    2310,  2314,  2315,  2316,  2320,  2321,  2322,  2325,  2327,  2331,
    2332,  2333,  2334,  2336,  2337,  2338,  2339,  2340,  2341,  2342,
    2343,  2344,  2345,  2348,  2353,  2354,  2355,  2357,  2358,  2360,
    2361,  2362,  2363,  2364,  2365,  2367,  2369,  2371,  2373,  2375,
    2376,  2377,  2378,  2379,  2380,  2381,  2382,  2383,  2384,  2385,
    2386,  2387,  2388,  2389,  2390,  2391,  2393,  2395,  2397,  2399,
    2400,  2403,  2404,  2408,  2410,  2414,  2417,  2420,  2426,  2427,
    2428,  2429,  2430,  2431,  2432,  2437,  2439,  2443,  2444,  2447,
    2448,  2452,  2455,  2457,  2459,  2463,  2464,  2465,  2466,  2468,
    2471,  2475,  2476,  2477,  2478,  2481,  2482,  2483,  2484,  2485,
    2487,  2488,  2493,  2495,  2498,  2501,  2503,  2505,  2508,  2510,
    2514,  2516,  2519,  2522,  2528,  2530,  2533,  2534,  2539,  2542,
    2546,  2546,  2551,  2554,  2555,  2559,  2560,  2564,  2565,  2569,
    2570,  2574,  2575,  2579,  2580,  2585,  2587,  2592,  2593,  2594,
    2595,  2596,  2597,  2599,  2602,  2605,  2607,  2611,  2612,  2613,
    2614,  2615,  2617,  2620,  2622,  2626,  2627,  2628,  2632,  2635,
    2642,  2646,  2650,  2657,  2658,  2663,  2665,  2666,  2669,  2670,
    2673,  2674,  2678,  2679,  2683,  2684,  2685,  2688,  2691,  2694,
    2697,  2698,  2699,  2701,  2704,  2708,  2709,  2710,  2712,  2713,
    2714,  2718,  2720,  2723,  2725,  2726,  2727,  2728,  2731,  2733,
    2734,  2738,  2740,  2743,  2745,  2746,  2747,  2751,  2753,  2756,
    2759,  2761,  2763,  2767,  2769,  2772,  2774,  2777,  2779,  2782,
    2783,  2787,  2789,  2792,  2794,  2797,  2800,  2804,  2806,  2810,
    2811,  2813,  2814,  2820,  2821,  2823,  2825,  2827,  2829,  2832,
    2833,  2834,  2838,  2839,  2840,  2841,  2842,  2843,  2844,  2845,
    2846,  2850,  2851,  2855,  2857,  2865,  2867,  2871,  2875,  2882,
    2883,  2889,  2890,  2897,  2900,  2904,  2907,  2912,  2917,  2919,
    2920,  2921,  2925,  2926,  2930,  2932,  2933,  2936,  2941,  2942,
    2943,  2947,  2950,  2959,  2961,  2965,  2968,  2971,  2979,  2982,
    2985,  2986,  2989,  2992,  2993,  2996,  3000,  3004,  3010,  3020,
    3021
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
     267,   268,   268,   269,   269,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   274,   275,   275,   276,   276,
     277,   277,   278,   278,   279,   279,   280,   280,   280,   280,
     281,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     285,   286,   286,   287,   287,   287,   287,   287,   287,   288,
     288,   288,   288,   288,   288,   288,   288,   289,   289,   289,
     289,   289,   289,   290,   290,   290,   290,   290,   290,   290,
     290,   291,   291,   292,   292,   292,   292,   292,   292,   293,
     293,   294,   294,   294,   295,   295,   295,   295,   296,   296,
     297,   298,   299,   299,   301,   300,   302,   300,   300,   303,
     300,   304,   300,   300,   300,   300,   300,   300,   300,   300,
     305,   305,   305,   306,   307,   307,   308,   308,   309,   309,
     310,   310,   311,   311,   311,   311,   311,   312,   312,   313,
     313,   314,   314,   315,   315,   316,   317,   317,   317,   318,
     318,   318,   318,   319,   319,   319,   319,   319,   319,   319,
     320,   320,   320,   321,   321,   322,   322,   323,   323,   324,
     324,   325,   325,   326,   326,   326,   326,   326,   326,   326,
     327,   327,   328,   328,   328,   329,   329,   329,   329,   330,
     330,   331,   331,   331,   331,   331,   332,   333,   333,   334,
     334,   335,   335,   336,   337,   338,   339,   340,   341,   341,
     341,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   343,   343,   345,   344,   346,   344,
     348,   347,   349,   347,   350,   347,   351,   347,   352,   352,
     352,   353,   353,   354,   354,   355,   355,   356,   356,   357,
     357,   358,   359,   359,   360,   361,   361,   362,   363,   363,
     364,   365,   366,   366,   367,   367,   367,   367,   368,   369,
     370,   371,   371,   371,   371,   372,   372,   373,   373,   373,
     373,   373,   373,   374,   375,   376,   377,   378,   379,   380,
     380,   381,   381,   382,   382,   383,   383,   384,   385,   386,
     387,   387,   387,   387,   388,   389,   389,   390,   390,   391,
     391,   392,   392,   393,   394,   394,   395,   395,   395,   396,
     396,   396,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   398,   399,   399,   400,   400,   400,   401,   401,
     401,   402,   402,   402,   403,   403,   403,   404,   404,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   407,   407,   407,   408,   408,
     408,   408,   408,   408,   408,   409,   409,   410,   410,   411,
     411,   412,   412,   412,   412,   413,   413,   413,   413,   413,
     413,   414,   414,   414,   414,   415,   415,   415,   415,   415,
     415,   415,   416,   416,   417,   417,   417,   417,   418,   418,
     419,   419,   420,   420,   421,   421,   422,   422,   423,   423,
     425,   424,   426,   427,   427,   428,   428,   429,   429,   430,
     430,   431,   431,   432,   432,   433,   433,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   435,   435,   435,
     435,   435,   435,   435,   435,   436,   436,   436,   437,   437,
     438,   438,   438,   439,   439,   440,   440,   440,   441,   441,
     442,   442,   443,   443,   444,   444,   444,   444,   444,   444,
     445,   445,   445,   445,   445,   446,   446,   446,   446,   446,
     446,   447,   447,   448,   448,   448,   448,   448,   448,   448,
     448,   449,   449,   450,   450,   450,   450,   451,   451,   452,
     452,   452,   452,   453,   453,   454,   454,   455,   455,   456,
     456,   457,   457,   458,   458,   459,   459,   460,   460,   461,
     461,   461,   461,   462,   462,   462,   462,   462,   462,   463,
     463,   463,   464,   464,   464,   464,   464,   464,   464,   464,
     464,   465,   465,   466,   466,   467,   467,   468,   468,   469,
     469,   470,   470,   471,   471,   472,   472,   473,   474,   474,
     474,   474,   475,   475,   476,   476,   476,   476,   477,   477,
     477,   478,   478,   479,   479,   480,   480,   481,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   482,   482,   483,
     483
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
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     7,     6,     2,     5,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     5,     2,
       4,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     1,     1,     1,     4,     1,     1,     3,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     2,     1,     1,     3,     3,     1,     1,
       0,     2,     4,     3,     6,     2,     3,     6,     1,     1,
       1,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     5,     0,     0,    11,     0,    12,
       0,     3,     0,     4,     0,     6,     0,     7,     2,     2,
       4,     1,     1,     5,     3,     5,     3,     2,     0,     2,
       0,     4,     4,     3,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     1,     1,     1,     3,     2,     3,
       4,     2,     3,     1,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     4,     4,     2,     8,    10,     2,     1,
       3,     1,     2,     1,     1,     1,     1,     2,     4,     3,
       3,     4,     1,     2,     4,     2,     6,     0,     1,     4,
       0,     2,     0,     1,     1,     3,     1,     3,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       3,     4,     2,     0,     5,     3,     3,     1,     3,     1,
       2,     0,     5,     3,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     4,     2,     4,     1,     1,     1,
       1,     1,     3,     5,     3,     4,     3,     1,     1,     1,
       1,     3,     5,     4,     3,     1,     1,     3,     7,     9,
       7,     6,     8,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     1,     3,     5,     3,     3,
       0,     1,     3,     5,     3,     2,     3,     6,     0,     1,
       4,     2,     0,     5,     3,     3,     1,     6,     4,     4,
       2,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     0,     2,     0,     5,     3,     3,
       1,     2,     0,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     1,
       2,     1,     4,     3,     0,     1,     3,     2,     3,     1,
       1,     0,     0,     2,     4,     2,     6,     4,     1,     1,
       0,     3,     5,     3,     1,     2,     0,     4,     2,     2,
       1,     1,     1,     1,     4,     6,     1,     8,     6,     1,
       0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   740,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   812,     0,
     800,   623,     0,   629,   630,   631,    22,   689,   788,    98,
     632,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   323,   324,
     325,   328,   327,   326,     0,     0,     0,     0,   158,     0,
       0,     0,   636,   638,   639,   633,   634,     0,     0,   640,
     635,     0,   614,    23,    24,    25,    27,    26,     0,   637,
       0,     0,     0,     0,     0,     0,     0,   641,   329,    28,
      29,    31,    30,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   444,     0,    97,    70,   792,   624,     0,     0,
       4,    59,    61,    64,   688,     0,   613,     0,     6,   130,
       7,     9,     8,    10,     0,     0,   321,   360,     0,     0,
       0,     0,     0,     0,     0,   358,   431,   432,   426,   425,
     345,   427,   428,   433,     0,     0,   344,   758,   615,     0,
     691,   424,   320,   761,   359,     0,     0,   759,   760,   757,
     783,   787,     0,   414,   690,    11,   328,   327,   326,     0,
       0,    27,    59,   130,     0,   870,   359,   869,     0,   867,
     866,   430,     0,   351,   355,     0,     0,   398,   399,   400,
     401,   423,   421,   420,   419,   418,   417,   416,   415,   788,
     616,     0,   884,   615,     0,   380,   378,     0,   816,     0,
     698,   343,   619,     0,   884,   618,     0,   628,   795,   794,
     620,     0,     0,   622,   422,     0,     0,     0,     0,   348,
       0,    78,   350,     0,     0,    84,    86,     0,     0,    88,
       0,     0,     0,   911,   912,   916,     0,     0,    59,   910,
       0,   913,     0,     0,    90,     0,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,    42,    47,   241,     0,
       0,   240,   160,   159,   246,     0,     0,     0,     0,     0,
     881,   146,   156,   808,   812,   853,     0,   643,     0,     0,
       0,   851,     0,    16,     0,    63,   138,   150,   157,   520,
     458,   836,   834,   834,     0,   875,   442,   446,   744,   360,
       0,   358,   359,     0,     0,   625,     0,   626,     0,     0,
       0,   120,     0,     0,    66,   232,     0,    21,   129,     0,
     155,   142,   154,   326,   329,   130,   322,   111,   112,   113,
     114,   115,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   800,     0,
     110,   791,   791,   118,   822,     0,     0,     0,     0,     0,
       0,   319,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,   377,   745,   746,     0,
     791,     0,   753,   232,   791,     0,   793,   784,   808,     0,
     130,     0,     0,    92,     0,   742,   737,   698,     0,     0,
       0,     0,   820,     0,   463,   697,   811,     0,     0,    66,
       0,   232,   342,     0,   755,   621,     0,    70,   196,     0,
     441,     0,    95,     0,     0,   349,     0,     0,     0,     0,
       0,    87,   109,    89,   908,   909,     0,   906,     0,     0,
       0,   880,     0,   116,    91,   119,     0,     0,     0,     0,
       0,     0,     0,   478,     0,   485,   487,   488,   489,   490,
     491,   492,   483,   505,   506,    70,     0,   106,   108,     0,
       0,    44,    51,     0,     0,    46,    55,    48,     0,    18,
       0,     0,   242,     0,    93,     0,     0,    94,   871,     0,
       0,   360,   358,   359,     0,   900,   166,     0,   809,     0,
       0,     0,     0,   642,   852,   689,     0,     0,   850,   694,
     849,    62,     5,    13,    14,     0,   164,     0,     0,   451,
       0,     0,   698,     0,     0,   617,   452,   840,     0,   698,
       0,     0,   698,     0,     0,     0,     0,     0,   744,     0,
     700,   743,   920,   341,   411,   766,    75,    69,    71,    72,
      73,    74,   320,     0,   429,   692,   693,    60,   698,     0,
     885,     0,     0,     0,   700,   233,     0,   436,   132,   162,
       0,   383,   385,   384,     0,     0,   381,   382,   386,   388,
     387,   403,   402,   405,   404,   406,   408,   409,   407,   397,
     396,   390,   391,   389,   392,   393,   395,   410,   394,   790,
       0,     0,   826,     0,   698,   874,     0,   873,   764,   783,
     148,   140,   152,   144,   130,     0,     0,   353,   356,   362,
     479,   376,   375,   374,   373,   372,   371,   370,   369,   368,
     367,   366,   365,     0,   747,   749,   762,   750,     0,     0,
       0,     0,     0,     0,     0,   868,   352,   735,   739,   697,
     741,     0,     0,   884,     0,   815,     0,   814,     0,   799,
     798,     0,   749,   796,   346,   198,   200,    70,   449,   448,
     347,     0,    70,   180,    79,   350,     0,     0,     0,     0,
       0,   192,   192,    85,     0,     0,     0,   904,   698,     0,
     891,     0,     0,     0,     0,     0,   696,   632,     0,     0,
     614,     0,     0,     0,     0,     0,    64,   645,   613,   651,
     652,   650,     0,   644,    68,   649,     0,     0,   495,     0,
       0,   501,   498,   499,   507,     0,   486,   481,     0,   484,
       0,     0,     0,    52,    19,     0,     0,    56,    20,     0,
       0,     0,    41,    49,     0,   239,   247,   244,     0,     0,
     862,   865,   864,   863,    12,   898,   899,     0,     0,     0,
       0,   808,   805,     0,   462,   861,   860,   859,     0,   855,
       0,   856,   858,     0,     5,     0,     0,     0,   514,   515,
     523,   522,     0,     0,   697,   457,   461,     0,   467,   697,
     835,     0,   465,   697,   833,   466,     0,   876,     0,   443,
       0,   892,   744,   219,   919,     0,     0,   754,   789,   697,
     887,   883,   234,   235,   612,   699,   231,     0,   744,     0,
       0,   164,   438,   134,   413,     0,   472,   473,     0,   464,
     697,   821,     0,     0,   232,   166,     0,   164,   162,     0,
     800,   363,     0,     0,   751,   752,   765,   785,   786,     0,
       0,     0,   723,   705,   706,   707,   708,     0,     0,     0,
     716,   715,   729,   698,     0,   737,   819,   818,     0,     0,
     756,   627,   202,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   184,     0,    70,   182,   103,
     192,     0,   192,     0,     0,   914,     0,     0,   697,   905,
     907,   890,   698,   889,     0,   698,   673,   674,   671,   672,
     704,     0,   698,   696,     0,     0,   460,   844,   842,   842,
       0,     0,   828,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,   503,   504,   502,     0,     0,   482,     0,
     122,     0,   125,   107,     0,    43,    53,     0,    45,    57,
      50,   243,     0,   872,    96,   900,   882,   895,   165,   167,
     253,     0,     0,   806,     0,   854,     0,    17,     0,   875,
     163,   253,     0,     0,   454,     0,   873,   839,   838,     0,
     877,     0,   892,     0,     0,   920,     0,   223,   221,   749,
     763,   886,     0,     0,   236,    67,     0,   744,   161,     0,
     744,     0,   412,   825,   824,     0,   232,     0,     0,     0,
       0,   164,   136,   628,   748,   232,     0,   711,   712,   713,
     714,   717,   718,   727,     0,   698,   723,     0,   710,   731,
     697,   734,   736,   738,     0,   813,   749,   797,     0,     0,
       0,     0,   199,   450,    81,     0,   350,   172,   174,   808,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,   901,     0,   903,   697,     0,     0,     0,   647,   697,
     695,     0,   686,     0,   698,     0,   848,     0,   698,     0,
       0,   698,     0,   653,   687,   685,   832,     0,   698,   656,
     658,   657,     0,     0,   654,   655,   659,   661,   660,   676,
     675,   678,   677,   679,   681,   682,   680,   669,   668,   663,
     664,   662,   665,   666,   667,   670,   493,     0,   494,   500,
     508,   509,     0,    70,    54,    58,   245,     0,     0,     0,
     320,   810,   808,   354,   357,   361,     0,    15,     0,   320,
     526,     0,     0,   528,   521,   524,     0,   519,     0,     0,
     878,     0,   893,   445,     0,   224,     0,   220,     0,   238,
     237,   892,     0,   253,     0,   744,     0,   232,     0,   781,
     253,   875,   253,     0,     0,   364,     0,     0,   720,   697,
     722,     0,   709,     0,     0,   698,   728,   817,     0,    70,
       0,   195,   181,     0,     0,     0,   171,    99,   185,     0,
       0,   188,     0,   193,   194,    70,   187,   915,     0,   888,
       0,   918,   703,   702,   646,     0,   697,   459,   648,   470,
     697,   843,     0,   468,   697,   841,   469,     0,   471,   697,
     827,   684,     0,     0,     0,     0,   894,   897,   168,     0,
       0,     0,   318,     0,     0,     0,   147,   252,   254,     0,
     317,     0,   320,     0,   857,   249,   151,   517,     0,     0,
     453,   837,   447,     0,   227,   218,     0,   226,   232,   435,
     892,   320,   892,     0,   823,     0,   780,   320,     0,   320,
     253,   744,   778,   726,   725,   719,     0,   721,   697,   730,
      70,   201,    77,    82,   101,   175,     0,   183,   189,    70,
     191,   902,     0,     0,   456,     0,   847,   846,     0,   831,
     830,   683,     0,    70,   126,     0,     0,     0,     0,     0,
     169,   284,   282,   286,   614,    27,     0,   278,     0,   283,
     295,     0,   293,   298,     0,   297,     0,   296,     0,   130,
     256,     0,   258,     0,   807,     0,   518,   516,   527,   525,
     228,     0,   217,   225,     0,     0,     0,     0,   143,   435,
     892,   782,   149,   249,   153,   320,     0,     0,   733,     0,
     197,     0,     0,    70,   178,   100,   190,   917,   701,     0,
       0,     0,     0,     0,   896,     0,     0,     0,     0,   268,
     272,     0,     0,   263,   578,   577,   574,   576,   575,   595,
     597,   596,   566,   537,   538,   556,   572,   571,   533,   543,
     544,   546,   545,   565,   549,   547,   548,   550,   551,   552,
     553,   554,   555,   557,   558,   559,   560,   561,   562,   564,
     563,   534,   535,   536,   539,   540,   542,   580,   581,   590,
     589,   588,   587,   586,   585,   573,   592,   582,   583,   584,
     567,   568,   569,   570,   593,   594,   598,   600,   599,   601,
     602,   579,   604,   603,   606,   608,   607,   541,   611,   609,
     610,   605,   591,   532,   290,   529,     0,   264,   311,   312,
     310,   303,     0,   304,   265,   337,     0,     0,     0,     0,
     130,   139,   248,     0,     0,     0,   230,   779,     0,    70,
     313,    70,   133,     0,     0,     0,   145,   892,   724,     0,
      70,   176,    83,   102,     0,   455,   845,   829,   496,   124,
     266,   267,   340,   170,     0,     0,   287,   279,     0,     0,
       0,   292,   294,     0,     0,   299,   306,   307,   305,     0,
       0,   255,     0,     0,     0,     0,   250,     0,   229,     0,
     512,   700,     0,     0,    70,   135,   141,     0,   732,     0,
       0,     0,   104,   269,    59,     0,   270,   271,     0,     0,
     285,   289,   530,   531,     0,   280,   308,   309,   301,   302,
     300,   338,   335,   259,   257,   339,     0,   251,   513,   699,
       0,   437,   314,     0,   137,     0,   179,   497,     0,   128,
       0,   320,   288,   291,     0,   744,   261,     0,   510,   434,
     439,   177,     0,     0,   105,   276,     0,   319,   336,     0,
     700,   331,   744,   511,     0,   127,     0,     0,   275,   892,
     744,   205,   332,   333,   334,   920,   330,     0,     0,     0,
     274,     0,   331,     0,   892,     0,   273,   315,    70,   260,
     920,     0,   209,   207,     0,    70,     0,     0,   210,     0,
     206,   262,     0,   316,     0,   213,   204,     0,   212,   123,
     214,     0,   203,   211,     0,   216,   215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   804,   542,   182,   275,   500,
     504,   276,   501,   505,   122,   123,   124,   125,   126,   127,
     323,   577,   578,   454,   240,  1421,   460,  1346,  1422,  1649,
     762,   270,   495,  1612,   993,  1173,  1664,   339,   183,   579,
     849,  1051,  1224,   131,   545,   866,   580,   599,   868,   526,
     865,   581,   546,   867,   341,   291,   307,   134,   851,   807,
     790,  1008,  1369,  1101,   913,  1562,  1425,   704,   919,   459,
     713,   921,  1255,   696,   902,   905,  1090,  1669,  1670,   569,
     570,   593,   594,   280,   281,   285,  1395,  1542,  1543,  1180,
    1297,  1388,  1538,  1655,  1672,  1574,  1616,  1617,  1618,  1376,
    1377,  1378,  1575,  1581,  1625,  1381,  1382,  1386,  1531,  1532,
    1533,  1552,  1699,  1298,  1299,   184,   136,  1685,  1686,  1536,
    1301,   137,   233,   455,   456,   138,   139,   140,   141,   142,
     143,   144,   145,  1406,   146,   848,  1050,   147,   237,   567,
     318,   568,   450,   551,   552,  1124,   553,  1125,   148,   149,
     150,   151,   152,   739,   740,   741,   153,   154,   267,   155,
     268,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     752,   753,   985,   492,   493,   494,   759,  1601,   156,   547,
    1397,   548,  1022,   812,  1197,  1194,  1524,  1525,   157,   158,
     159,   227,   234,   326,   442,   160,   940,   745,   161,   941,
     840,   833,   942,   891,  1071,  1073,  1074,  1075,   893,  1234,
    1235,   894,   678,   427,   195,   196,   582,   572,   409,   665,
     666,   837,   163,   228,   186,   165,   166,   167,   168,   169,
     170,   171,   630,   172,   230,   231,   529,   219,   220,   633,
     634,  1137,  1138,   561,   558,   562,   559,  1130,  1127,  1131,
    1128,   300,   301,   798,   173,   519,   174,   566,   175,  1544,
     292,   334,   588,   589,   934,  1034,   787,   788,   717,   718,
     719,   261,   262,   835
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1417
static const yytype_int16 yypact[] =
{
   -1417,   132, -1417, -1417,  6119, 13568, 13568,   -66, 13568, 13568,
   13568, 11666, 13568, -1417, 13568, 13568, 13568, 13568, 13568, 13568,
   13568, 13568, 13568, 13568, 13568, 13568, 14241, 14241, 11873, 13568,
   14720,   -59,   -55, -1417, -1417, -1417, -1417, -1417,   152, -1417,
      96, 13568, -1417,   -55,     7,    10,    75,   -55, 12038, 15948,
   12203, -1417, 14181, 10673,     2, 13568, 15654,    28, -1417, -1417,
   -1417,   202,   206,   225,   149,   159,   162,   166, -1417, 15948,
     189,   200, -1417, -1417, -1417, -1417, -1417,   324,  3231, -1417,
   -1417, 15948, -1417, -1417, -1417, -1417, 15948, -1417, 15948, -1417,
     251,   207,   223,   226,   238, 15948, 15948, -1417,    21, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, 13568, -1417, -1417,   228,   317,   330,   330,
   -1417,   413,   293,   338, -1417,   247, -1417,    53, -1417,   414,
   -1417, -1417, -1417, -1417, 15741,   417, -1417, -1417,   270,   280,
     283,   287,   291,   297,  2969, -1417, -1417, -1417, -1417,   408,
   -1417, -1417, -1417,   429,   473,   370, -1417,    80,   350,   426,
   -1417, -1417,   630,    23,   603,   107,   384,   109,   121,   394,
      69, -1417,   111, -1417,   516, -1417, -1417, -1417,   449,   398,
     452, -1417, -1417,   414,   417, 16223,  2094, 16223, 13568, 16223,
   16223,  3329,   401, 15060,  3329,   562, 15948,   544,   544,   300,
     544,   544,   544,   544,   544,   544,   544,   544,   544, -1417,
   -1417,  1768,   442, -1417,   468,    -3,    -3, 14241, 15104,   423,
     623, -1417,   449,  1768,   442,   487,   488,   440,   123, -1417,
      -3,   107, 12368, -1417, -1417, 13568,  5455,   629,    62, 16223,
   10259, -1417, 13568, 13568, 15948, -1417, -1417,  3201,   436, -1417,
    3429, 14181, 14181,   472, -1417, -1417,   444, 14108,   631, -1417,
     635, -1417, 15948,   572, -1417,   453,  3503,   455,   755, -1417,
       6,  3559, 15790, 15803, 15948,    63, -1417,    33, -1417,  2207,
      64, -1417, -1417, -1417,   640,    65, 14241, 14241, 13568,   457,
     492, -1417, -1417, 14582, 11873,   242,    19, -1417, 13733, 14241,
     351, -1417, 15948, -1417,   195,   293, -1417, -1417, -1417, -1417,
   14804, 13568, 13568, 13568,   648,   567, -1417, -1417,    73,   463,
   16223,   467,  1502,  6326, 13568,   369,   465,   362,   369,   272,
     309, -1417, 15948, 14181,   469, 10880, 14181, -1417, -1417, 14786,
   -1417, -1417, -1417, -1417, -1417,   414, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, 13568, 13568, 13568, 12575, 13568, 13568, 13568,
   13568, 13568, 13568, 13568, 13568, 13568, 13568, 13568, 13568, 13568,
   13568, 13568, 13568, 13568, 13568, 13568, 13568, 13568, 14720, 13568,
   -1417, 13568, 13568, -1417, 13568,  3804, 15948, 15948, 15948, 15741,
     568,   451, 10466, 13568, 13568, 13568, 13568, 13568, 13568, 13568,
   13568, 13568, 13568, 13568, 13568, -1417, -1417, -1417, -1417,  4467,
   13568, 13568, -1417, 10880, 13568, 13568,   228,   128, 14582,   476,
     414, 12782,  3675, -1417, 13568, -1417,   482,   678,  1768,   489,
      -8,  4789,    -3, 12989, -1417, 13196, -1417,   500,    -6, -1417,
     150, 10880, -1417,  4467, -1417, -1417,  3820, -1417, -1417, 11087,
   -1417, 13568, -1417,   600,  9431,   680,   491, 16135,   683,    48,
      31, -1417, -1417, -1417, -1417, -1417, 14181, 15586,   503,   693,
   14533, -1417,   518, -1417, -1417, -1417,   632, 13568,   633,   634,
   13568, 13568, 13568, -1417,   755, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417,   519, -1417, -1417, -1417,   514, -1417, -1417, 15948,
     515,   701,    46, 15948,   520,   707,    61,   222, 15861, -1417,
   15948, 13568,    -3,    28, -1417, 14533,   639, -1417,    -3,    67,
      85,   522,   524,  2070,   526,   345,   595,   532,    -3,    87,
     525, 15728, 15948, -1417, -1417,   667,  1728,   -18, -1417, -1417,
   -1417,   293, -1417, -1417, -1417,   704,   609,   569,   227, -1417,
     228,   607,   730,   539,   601,   128, -1417, 16223,   547,   741,
   15161,   551,   746,   556, 14181, 14181,   744,   629,    73,   563,
     751, -1417, 14181,    40,   697,   130, -1417, -1417, -1417, -1417,
   -1417, -1417,   770,  1865, -1417, -1417, -1417, -1417,   756,   599,
   -1417, 14241, 13568,   570,   761, 16223,   758, -1417, -1417,   650,
   14846, 11277,  3009,  3329, 13568, 16179,  4859, 13386,  3901, 14607,
   12553, 16339, 16339, 16339, 16339,  1421,  1421,  1421,  1421,   772,
     772,   542,   542,   542,   300,   300,   300, -1417,   544, 16223,
     566,   573, 15205,   575,   773, -1417, 13568,     0,   581,   128,
   -1417, -1417, -1417, -1417,   414, 13568,  4043, -1417, -1417,  3329,
   -1417,  3329,  3329,  3329,  3329,  3329,  3329,  3329,  3329,  3329,
    3329,  3329,  3329, 13568, -1417,   131,     0, -1417,   576,  1995,
     584,   580,  2141,    88,   596, -1417, 16223,  5202, -1417, 15948,
   -1417,   463,    40,   442, 14241, 16223, 14241, 15262,    40,   135,
   -1417,   594,   136, -1417, -1417,  9224,   382, -1417, -1417, 16223,
   16223,   -55, -1417, -1417, -1417, 13568,   702, 14381, 14533, 15948,
    9638,   608,   622, -1417,    57,   668,   653, -1417,   794,   610,
    4885, 14181, 14533, 14533, 14533, 14533, 14533, -1417,   627,    29,
     661,   628,   641,   642,   644, 14533,     8, -1417,   682, -1417,
   -1417, -1417,   643, -1417, 16309, -1417, 13568,   645, 16223,   647,
     833,  4349,   839, -1417, 16223,  4305, -1417,   519,   780, -1417,
    6533, 15670,   646,   254, -1417, 15790, 15948,   261, -1417, 15803,
   15948, 15948, -1417, -1417,  2205, -1417, 16309,   847, 14241,   662,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417,    58, 15948, 15670,
     663, 14582, 14666,   853, -1417, -1417, -1417, -1417,   665, -1417,
   13568, -1417, -1417,  5705, -1417, 14181, 15670,   666, -1417, -1417,
   -1417, -1417,   856, 13568, 14804, -1417, -1417, 15877, -1417, 13568,
   -1417, 13568, -1417, 13568, -1417, -1417,   671, -1417, 14181, -1417,
     677,   849,    18, -1417, -1417,    44,  4467, -1417, -1417, 14181,
   -1417, -1417,    -3, 16223, -1417, 11294, -1417, 14533,    83,   688,
   15670,   609, -1417, -1417, 12765, 13568, -1417, -1417, 13568, -1417,
   13568, -1417,  2435,   689, 10880,   595,   850,   609,   650, 15948,
   14720,    -3,  2479,   696, -1417, -1417,   137, -1417, -1417,   883,
   14892, 14892,  5202, -1417, -1417, -1417, -1417,   698,   231,   703,
   -1417, -1417, -1417,   892,   705,   482,    -3,    -3, 13403,  4467,
   -1417, -1417,   386,   -55, 10259, -1417,  6740,   706,  6947,   709,
   14381, 14241,   713,   785,    -3, 16309,   901, -1417, -1417, -1417,
   -1417,    54, -1417,    16, 14181, -1417, 14181, 15948, 15586, -1417,
   -1417, -1417,   907, -1417,   720,   756,   510,   510,   861,   861,
   15407,   721,   920, 14533,   789, 15948, 14804, 14533, 14533, 14533,
    3868, 15935, 14533, 14533, 14533, 14533, 14332, 14533, 14533, 14533,
   14533, 14533, 14533, 14533, 14533, 14533, 14533, 14533, 14533, 14533,
   14533, 14533, 14533, 14533, 14533, 14533, 14533, 14533, 14533, 16223,
   13568, 13568, 13568, -1417, -1417, -1417, 13568, 13568, -1417,   755,
   -1417,   851, -1417, -1417, 15948, -1417, -1417, 15948, -1417, -1417,
   -1417, -1417, 14533,    -3, -1417,   345, -1417,   840,   928, -1417,
   -1417,    90,   742,    -3, 11501, -1417,  1641, -1417,  5912,   567,
     928, -1417,   262,   209, 16223,   814, -1417, 16223, 16223, 15306,
   -1417,   752,   849, 14181,   629, 14181,   869,   939,   876,   138,
       0, -1417, 14241, 13568, 16223, 16309,   759,    83, -1417,   753,
      83,   757, 12765, 16223, 15363,   762, 10880,   760,   771, 14181,
     774,   609, -1417,   440, -1417, 10880, 13568, -1417, -1417, -1417,
   -1417, -1417, -1417,   836,   766,   966,  5202,   831, -1417, 14804,
    5202, -1417, -1417, -1417, 14241, 16223,   143, -1417,   -55,   948,
     906, 10259, -1417, -1417, -1417,   781, 13568,   785,    -3, 14582,
   14381,   784, 14533,  7154,   502,   786, 13568,    45,    59, -1417,
     817, -1417,   862, -1417,  5252,   964,   797, 14533, -1417, 14533,
   -1417,   807, -1417,   866,   998,   810, 16309,   815,  1006, 15464,
     818,  1011,   820, -1417, -1417, -1417, 15506,   821,  1015,  5642,
   11856, 10863, 14533, 16267, 13179,  2512, 13935, 14513, 14205, 14714,
   14714, 14714, 14714,  1776,  1776,  1776,  1776,   806,   806,   510,
     510,   510,   861,   861,   861,   861, 16223,  5178, 16223, -1417,
   16223, -1417,   825, -1417, -1417, -1417, 16309, 15948, 14181, 15670,
      66, -1417, 14582, -1417, -1417,  3329,   823, -1417,   830,   430,
   -1417,    55, 13568, -1417, -1417, -1417, 13568, -1417, 13568, 13568,
   -1417,   629, -1417, -1417,    68,  1014, 14533, -1417,   834,    -3,
   16223,   849,   837, -1417,   838,    83, 13568, 10880,   846, -1417,
   -1417,   567, -1417,   845,   841, -1417,   858,  5202, -1417,  5202,
   -1417,   863, -1417,   908,   864,  1031, -1417,    -3,  1021, -1417,
     852, -1417, -1417,   867,   870,    93, -1417, -1417, 16309,   855,
     860, -1417,  2875, -1417, -1417, -1417, -1417, -1417, 14181, -1417,
   14181, -1417, 16309, 15563, -1417, 14533, 14804, -1417, -1417, -1417,
   14533, -1417, 14533, -1417, 14533, -1417, -1417, 14533, -1417, 14533,
   -1417, 12972, 14533, 13568,   871,  7361,   956, -1417, -1417,   335,
   14181, 15670, -1417, 15608,   898, 14743, -1417, -1417, -1417,   568,
   14038,    71,   451,    95, -1417, -1417, -1417,   917,  2742,  2831,
   16223, 16223, -1417,   994,  1063,   999, 14533, 16309, 10880,   969,
     849,   854,   849,   879, 16223,   880, -1417,   887,   881,  1100,
   -1417,    83, -1417, -1417,   954, -1417,  5202, -1417, 14804, -1417,
   -1417,  9224, -1417, -1417, -1417, -1417,  9845, -1417, -1417, -1417,
    9224, -1417,   884, 14533, 16309,   957, 16309, 16309, 15605, 16309,
   15662, 12972,  5122, -1417, -1417, 14181, 15670, 15670,  1074,    50,
   -1417, -1417, -1417, -1417,    72,   891,    74, -1417, 13940, -1417,
   -1417,    76, -1417, -1417, 14435, -1417,   888, -1417,  1016,   414,
   -1417, 14181, -1417,   568, -1417,  3125, -1417, -1417, -1417, -1417,
    1083, 14533, -1417, 16309,   899,   909,   903,   276, -1417,   969,
     849, -1417, -1417, -1417, -1417,  1294,   900,  5202, -1417,   979,
    9224, 10052,  9845, -1417, -1417, -1417,  9224, -1417, 16309, 14533,
   14533, 14533, 13568,  7568, -1417,   910,   913, 14533, 15670, -1417,
   -1417,  1047, 15608, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417,   103, -1417,   898, -1417, -1417, -1417,
   -1417, -1417,    60,   438, -1417,  1096,    77, 15948,  1016,  1097,
     414, -1417, -1417,   914,  1104, 14533, 16309, -1417,   285, -1417,
   -1417, -1417, -1417,   916,   276,  4372, -1417,   849, -1417,  5202,
   -1417, -1417, -1417, -1417,  7775, 16309, 16309, 16309,  4757, -1417,
   -1417, -1417, 16309, -1417,  4302,    47, -1417, -1417, 14533, 13940,
   13940,  1065, -1417, 14435, 14435,   508, -1417, -1417, -1417, 14533,
    1045, -1417,   925,    78, 14533, 15948, -1417, 14533, 16309,  1048,
   -1417,  1117,  7982,  8189, -1417, -1417, -1417,   276, -1417,  8396,
     926,  1050,  1022, -1417,  1035,   985, -1417, -1417,  1038,  1047,
   -1417, 16309, -1417, -1417,   975, -1417,  1103, -1417, -1417, -1417,
   -1417, 16309,  1123, -1417, -1417, 16309,   940, 16309, -1417,   342,
     938, -1417, -1417,  8603, -1417,   941, -1417, -1417,   947,   982,
   15948,   451, -1417, -1417, 14533,    86, -1417,  1062, -1417, -1417,
   -1417, -1417, 15670,   646, -1417,   986, 15948,   388, 16309,   949,
    1144,   437,    86, -1417,  1075, -1417, 15670,   955, -1417,   849,
      98, -1417, -1417, -1417, -1417, 14181, -1417,   958,   959,    79,
   -1417,   323,   437,   120,   849,   953, -1417, -1417, -1417, -1417,
   14181,  1079,  1147,  1084,   323, -1417,  8810,   282,  1148, 14533,
   -1417, -1417,  9017, -1417,  1085,  1151,  1087, 14533, 16309, -1417,
    1153, 14533, -1417, 16309, 14533, 16309, 16309
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1417, -1417, -1417,  -495, -1417, -1417, -1417,    -4, -1417, -1417,
   -1417,   664,   405,   406,    70,  1023,  3748, -1417,  2379, -1417,
    -405, -1417,     5, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417,  -487, -1417, -1417,  -182,     4,    11,
   -1417, -1417, -1417, -1417, -1417, -1417,    12, -1417, -1417, -1417,
   -1417,    13, -1417, -1417,   788,   792,   798,  -130,   319,  -799,
     320,   383,  -486,    97,  -802, -1417,  -230, -1417, -1417, -1417,
   -1417,  -663,   -57, -1417, -1417, -1417, -1417,  -479, -1417,  -537,
   -1417,  -391, -1417, -1417,   687, -1417,  -212, -1417, -1417,  -978,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,
    -240, -1417, -1417, -1417, -1417, -1417,  -323, -1417,   -91, -1064,
   -1417, -1416,  -498, -1417,  -160,    -1,  -131,  -484, -1417,  -329,
   -1417,   -74,   -22,  1178,  -671,  -364, -1417, -1417,   -34, -1417,
   -1417,  3773,   -49,  -189, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417,  -553,  -776, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1417,   829, -1417,
   -1417,   235, -1417,   743, -1417, -1417, -1417, -1417, -1417, -1417,
   -1417,   240, -1417,   745, -1417, -1417,   475, -1417,   211, -1417,
   -1417, -1417, -1417, -1417, -1417, -1417, -1417, -1024, -1417,  1857,
     360,  -348, -1417, -1417,   172,  3333,  4150, -1417, -1417,   294,
    -142,  -588, -1417, -1417,   355,  -640,   163, -1417, -1417, -1417,
   -1417, -1417,   343, -1417, -1417, -1417,    42,  -812,  -194,  -408,
    -385,  -136, -1417, -1417,    14, -1417, -1417, -1417, -1417,    -7,
     -95, -1417,  -270, -1417, -1417, -1417,  -392,   950, -1417, -1417,
   -1417, -1417, -1417,   929, -1417, -1417, -1417,   292, -1417, -1417,
   -1417,   496,   356, -1417, -1417,   960,  -303,  -975, -1417,   -41,
     -83,  -200,  -141,   528, -1417,  -999, -1417,   245,   325, -1417,
   -1417, -1417,  -197, -1008
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -885
static const yytype_int16 yytable[] =
{
     121,   419,   390,   135,   346,   308,   846,   556,   129,   128,
     236,   260,   314,   315,   829,   130,   132,   133,   164,   265,
    1035,   241,   670,   229,   439,   245,   673,  1204,   647,   412,
     627,   830,   389,  1201,   909,   692,   443,   892,  1025,   319,
     215,   216,   695,  1189,  1188,   248,   162,   803,   258,   923,
     691,   342,  1049,   346,   464,   465,  1619,   709,   693,  1438,
     469,   711,   336,    13,   321,   290,   924,  1005,  1060,  1583,
    1289,   451,   508,   513,   516,  1253,   778,   417,   436,  1036,
    1391,  -281,   306,  1442,   290,  1526,  1590,  1590,  1438,  -770,
     760,   290,   290,  1584,   778,   444,   792,   792,   295,   792,
     316,   532,   792,  1313,   792,  1307,   410,   278,  1097,   407,
     408,    13,   631,   944,   496,  1106,  1107,  1578,    13,   407,
     408,   407,   408,  1037,   407,   408,   277,   322,    13,   510,
     290,    13,     3,  1579,   188,   414,   590,   429,  1605,   345,
     668,   232,   766,    13,   671,   235,  -767,  1314,   304,   437,
    1580,   305,  1105,  1106,  1107,  1701,  -884,   770,  1106,  1107,
    -440,   391,  -476,   600,   533,   298,   299,   407,   408,   333,
    1123,  1290,   497,   410,   297,  -768,  1291,   333,    58,    59,
      60,   176,   177,   343,  1292,   332,   802,  -769,   420,  -801,
     209,  1644,   426,   575,   414,   689,  -774,  -771,   332,  1702,
    -884,  -804,  -802,  -773,  -772,   269,  1038,   242,   309,  -803,
     243,  -884,  1319,   332,   521,   555,  -699,  -617,   317,  -699,
    1109,  1293,  1294,  -776,  1295,   430,  -770,   925,  1006,   209,
    1315,   432,   121,   279,   712,  1321,   121,   438,   674,   522,
     458,   448,  1327,   411,  1329,   453,  1328,  1254,   344,   710,
     164,  1620,  1439,  1440,   164,   337,   598,  1104,   471,  1108,
     346,  1585,  1223,  1256,   452,   509,   514,   517,   779,   714,
    1296,  -222,   415,  1392,  -281,   244,  1443,   809,  1527,  1591,
    1634,  1696,  1703,  -767,  -222,   680,   780,  -208,   793,   879,
     639,  1181,   906,   512,  1345,   596,  1394,   908,  1246,  -699,
     518,   518,   523,  1233,   284,   308,   342,   528,   531,  1018,
     411,  1046,  -768,   537,   639,  1077,   416,  1714,   771,   121,
    1599,  1407,   135,  1409,  -769,   282,  -801,   129,   587,   283,
    -777,   415,   258,  -774,  -771,   290,   639,   164,  -804,  -802,
    -773,  -772,   502,   506,   507,   639,  -803,   332,   639,   286,
     994,   295,  1415,   378,   681,   116,   538,   997,   648,   287,
     571,  1715,   288,  1195,  1600,   379,   289,   826,   827,   407,
     408,   229,   541,  -616,   332,   834,  1078,  1657,   638,   836,
     810,   637,   290,   641,   290,   290,   214,   214,   295,   293,
     226,   785,   786,   538,   644,   811,   295,   543,   544,  1011,
     294,   325,   667,   295,   309,   664,   332,   310,   296,   295,
     815,  1554,  1196,   332,   328,   698,  1190,   820,   298,   299,
     824,  1658,   390,   311,   638,  1243,   312,   683,  1039,  1191,
     295,   324,   528,   690,  1289,   538,   667,   863,   313,   664,
    1236,   295,   430,   331,  1716,   332,   538,   335,   295,   338,
     121,  1040,   389,   903,   904,   298,   299,  1088,  1089,   703,
    1366,  1367,   869,   298,   299,  1192,   873,  1586,   164,   297,
     298,   299,   347,  1057,  -474,    13,   298,   299,  1550,  1551,
     585,  1203,   348,   863,  1587,   349,  -884,  1588,   836,   350,
    1355,  1086,   861,   351,   899,   381,   539,   298,   299,   352,
      58,    59,    60,   176,   177,   343,   773,   333,   298,   299,
    1212,   556,  1103,  1214,  1087,   298,   299,   853,   586,  1626,
    1627,    51,  1063,   590,   590,  1697,  1698,   797,   799,    58,
      59,    60,   176,   177,   343,  1290,   439,  1628,  -884,   382,
    1291,  -884,    58,    59,    60,   176,   177,   343,  1292,  1682,
    1683,  1684,   900,   384,  1629,  1622,  1623,  1630,  1607,   975,
     976,   977,  1419,    58,    59,    60,   176,   177,   343,   763,
     344,   214,   383,   767,   385,   978,   929,   214,   277,   932,
     935,   391,  -475,   214,   413,  1293,  1294,  1333,  1295,  1334,
    1678,   375,   376,   377,  -775,   378,   290,  -616,   418,   344,
    1250,  1106,  1107,   423,   302,   842,   425,   379,  1019,   379,
     571,   333,   344,   327,   329,   330,   431,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     434,  1031,   435,   344,  1306,  -615,   440,   449,   462,   214,
     441,   466,  1041,   556,   467,  -879,   214,   214,  1312,   470,
    1183,   472,   534,   214,   515,   473,   540,   475,   524,   214,
     871,   525,   564,   565,   573,  1218,   405,   406,   574,   -65,
     554,   584,    51,   890,  1226,   895,   597,  1693,  1323,   907,
    1691,   534,   677,   540,   534,   540,   540,   679,   701,   451,
     682,   121,  1707,   705,   135,  1704,  1418,   708,   896,   129,
     897,   688,   721,   720,   746,   916,   121,  1245,   758,   164,
     765,   747,   749,   750,   761,   918,   769,   764,   777,   555,
     789,   914,   768,   781,   164,   782,   794,  1110,   784,  1111,
     407,   408,   791,   800,   805,   806,   813,   808,   226,   814,
     816,   639,    58,    59,    60,    61,    62,   343,   818,   817,
     819,  1081,   822,    68,   386,   823,   121,   825,   828,   135,
     832,   831,   996,  -477,   129,   839,   999,  1000,  1285,   841,
     845,   844,   847,   856,   164,   850,   556,  1558,   214,   859,
     857,   864,   860,   874,  1007,   876,  1062,   877,   214,   387,
    1303,   388,  1003,  1116,  1416,   901,   852,   926,   910,   121,
    1120,   927,   135,   928,   639,   528,  1013,   129,   128,   945,
     920,   930,   344,  1026,   130,   132,   133,   164,   372,   373,
     374,   375,   376,   377,   922,   378,  1325,   943,   946,   667,
     951,   980,   664,   981,  1341,   502,  1202,   379,   834,   506,
    1208,   947,   948,  1671,   949,   162,   952,   982,   986,   992,
    1350,   555,   972,   973,   974,   975,   976,   977,  1289,   989,
    1671,  1002,  1221,   229,  1004,   290,  1010,  1014,  1692,  1021,
    1023,   978,  1015,  1030,   571,  1032,  1070,  1070,   890,  1033,
    1059,  1091,    58,    59,    60,    61,    62,   343,  1047,  1056,
     571,  1289,   667,    68,   386,   664,  1065,  1066,  1076,    13,
     121,  1080,   121,  1079,   121,   135,  1082,   135,  1094,  1092,
     129,  1096,   129,  1099,  1100,  1102,  1114,  1041,   164,  1608,
     164,  1115,   164,  1112,   914,  1098,   978,  1404,  1118,  1119,
    1172,   388,    13,  1230,   533,  1420,  1178,  1179,   476,   477,
     478,  1122,  1182,  1198,  1426,   479,   480,  1135,  1205,   481,
     482,   214,   344,  1206,  1200,  1207,  1213,  1215,  1433,  1290,
    1211,  1219,  1217,   556,  1291,  1227,    58,    59,    60,   176,
     177,   343,  1292,  1228,  1220,  1229,  1232,  1222,  1239,  1240,
    1184,  1287,  1267,  1242,   555,  1247,  1271,  1257,  1251,  1275,
    1174,  1258,  1290,  1175,  1260,  1265,  1280,  1291,  1261,    58,
      59,    60,   176,   177,   343,  1292,   214,  1266,  1264,  1293,
    1294,  1268,  1295,  1640,   121,  1270,  1269,   135,  1564,  1273,
    1274,  1276,   129,   128,  1279,  1278,  1284,  1304,  1316,   130,
     132,   133,   164,  1305,  1318,   556,   344,  1336,  1320,  1322,
    1338,  1331,  1293,  1294,   214,  1295,   214,  1326,  1330,   212,
     212,  1340,  1365,   224,  1342,  1380,  1209,  1347,  1408,  1332,
     162,  1351,  1348,  1352,  1335,  1337,  1238,   214,  1343,   344,
    1396,  1344,   890,  1400,  1363,   224,   890,  1401,  1402,  1405,
    1410,  1411,  1681,  1417,  1413,  1427,  1429,   121,  1437,   571,
    1534,  1412,   571,  1339,  1441,  1535,  1241,  1545,  1237,   121,
    1547,  1557,   135,  1390,  1289,   164,  1549,   129,  1559,  1548,
    1589,  1594,  1570,   528,   914,  1571,  1596,   164,  1597,  1604,
    1624,    33,    34,    35,  1632,  1633,  1639,  1638,  1646,  1647,
    1648,  -277,   727,  1650,  1651,  1653,  1584,  1654,   214,  1659,
    1656,  1673,  1393,  1661,  1602,    13,  1603,  1662,  1663,  1676,
    1679,   214,   214,  1680,  1688,  1609,  1705,  1690,  1708,  1694,
    1695,  1709,  1717,  1710,  1720,  1721,  1722,  1724,  1434,   346,
     995,   555,   772,  1286,   554,   998,  1675,   643,   640,  1300,
      72,    73,    74,    75,    76,  1058,   642,  1061,  1300,  1020,
    1689,   729,  1563,  1687,  1244,  1349,   528,    79,    80,  1643,
     775,  1555,  1577,  1582,  1387,  1290,  1711,  1537,  1700,  1593,
    1291,    89,    58,    59,    60,   176,   177,   343,  1292,   238,
    1553,   650,  1302,   890,  1171,   890,  1169,   756,    97,   757,
     226,  1302,   988,  1193,   212,  1225,  1072,  1121,  1083,  1231,
     212,  1132,   563,   555,   530,     0,   212,   520,   933,  1368,
    1177,     0,     0,  1113,     0,  1293,  1294,   571,  1295,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,   214,     0,     0,   224,   224,     0,     0,     0,     0,
     224,   121,   344,     0,   135,     0,   258,     0,     0,   129,
       0,  1385,     0,  1706,     0,     0,     0,     0,  1289,   164,
    1712,   391,   212,  1389,  1414,     0,   554,     0,     0,   212,
     212,     0,     0,     0,     0,     0,   212,     0,     0,     0,
    1300,     0,   212,     0,     0,     0,  1300,     0,  1300,     0,
       0,     0,   890,   224,     0,     0,     0,   121,     0,    13,
     135,     0,   121,     0,     0,   129,   121,     0,     0,   135,
    1539,  1424,     0,     0,   129,   164,   224,     0,  1595,   224,
     164,     0,     0,  1302,   164,     0,     0,     0,     0,  1302,
       0,  1302,     0,   571,  1523,     0,     0,     0,     0,     0,
    1530,     0,     0,     0,     0,     0,     0,   258,     0,     0,
       0,   258,     0,     0,     0,     0,     0,  1540,     0,  1290,
       0,   224,   214,     0,  1291,     0,    58,    59,    60,   176,
     177,   343,  1292,   890,  1300,     0,   121,   121,   121,   135,
       0,     0,   121,     0,   129,   135,  1561,  1424,     0,   121,
     129,     0,   135,     0,   164,   164,   164,   129,     0,   554,
     164,   212,     0,     0,   214,     0,     0,   164,     0,  1293,
    1294,   212,  1295,     0,  1592,     0,     0,  1302,     0,   214,
     214,  -885,  -885,  -885,  -885,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,   344,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,   834,   224,
     224,  1666,     0,   736,     0,     0,     0,     0,  1556,     0,
       0,     0,     0,   834,     0,     0,     0,     0,     0,     0,
       0,     0,  1636,     0,     0,     0,   421,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,     0,
       0,     0,     0,   290,     0,     0,   346,     0,   736,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
       0,   258,     0,     0,     0,   890,     0,     0,     0,     0,
     121,     0,     0,   135,     0,   405,   406,     0,   129,     0,
    1614,     0,     0,     0,     0,  1523,  1523,     0,   164,  1530,
    1530,     0,     0,     0,     0,     0,     0,   224,   224,     0,
       0,   290,     0,     0,     0,   224,     0,     0,   121,   121,
       0,   135,   135,     0,     0,   121,   129,   129,   135,     0,
       0,     0,     0,   129,   212,     0,   164,   164,     0,     0,
       0,     0,     0,   164,     0,     0,   554,     0,     0,   407,
     408,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,   135,     0,     0,     0,  1665,   129,     0,     0,
    1667,   353,   354,   355,     0,     0,     0,   164,     0,     0,
       0,     0,  1677,     0,     0,     0,     0,     0,     0,   212,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,   571,   554,     0,
       0,     0,   121,   575,     0,   135,   379,   212,   121,   212,
     129,   135,     0,     0,   571,     0,   129,     0,     0,     0,
     164,     0,   571,     0,     0,     0,   164,     0,     0,     0,
     212,   736,     0,     0,     0,     0,     0,     0,   353,   354,
     355,     0,     0,   224,   224,   736,   736,   736,   736,   736,
       0,     0,     0,     0,     0,     0,     0,   356,   736,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,   212,   212,  -885,  -885,  -885,  -885,
     970,   971,   972,   973,   974,   975,   976,   977,   224,   224,
       0,     0,     0,     0,     0,    29,    30,   224,     0,     0,
       0,   978,     0,     0,     0,    36,     0,   209,  1186,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,     0,
     736,     0,     0,   224,     0,   353,   354,   355,     0,     0,
       0,     0,     0,   213,   213,   210,     0,   225,     0,     0,
       0,     0,     0,   224,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   180,   378,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
     379,     0,   801,   212,   212,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   224,     0,   224,
       0,   224,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   736,     0,   428,   224,
     736,   736,   736,   116,     0,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,   736,
     736,   736,     0,     0,     0,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,   736,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,     0,     0,     0,     0,     0,   224,     0,   224,     0,
     379,     0,     0,     0,     0,   212,     0,     0,     0,   838,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,   224,     0,   421,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   212,   421,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,     0,   212,   212,     0,   736,     0,     0,     0,     0,
       0,     0,     0,   405,   406,     0,   213,   224,     0,     0,
     736,     0,   736,   213,   213,     0,     0,     0,     0,     0,
     213,   353,   354,   355,     0,     0,   213,   405,   406,     0,
       0,     0,     0,     0,     0,   736,     0,   213,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,   407,   408,   875,
       0,   224,   224,     0,     0,   212,   379,     0,     0,     0,
       0,     0,     0,     0,     0,   353,   354,   355,     0,     0,
       0,   407,   408,     0,     0,     0,     0,     0,     0,   736,
       0,     0,     0,     0,   356,   225,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     379,   783,     0,     0,     0,   213,     0,     0,     0,     0,
       0,   224,     0,   224,    36,     0,   209,     0,   736,   224,
       0,     0,     0,   736,     0,   736,     0,   736,     0,     0,
     736,     0,   736,     0,     0,   736,     0,     0,     0,     0,
       0,     0,     0,   224,   224,     0,   224,     0,     0,     0,
       0,     0,     0,   224,   210,     0,     0,   742,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   736,
       0,     0,     0,     0,     0,   878,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   180,     0,     0,    81,
      82,   224,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,   742,     0,     0,     0,   736,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,   224,
     224,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   211,     0,  1001,
     511,     0,   116,     0,   224,     0,     0,     0,   224,     0,
       0,     0,     0,     0,   736,     0,     0,     0,     0,     0,
       0,   259,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   353,   354,   355,   213,     0,
       0,     0,   736,   736,   736,     0,     0,     0,     0,     0,
     736,   224,     0,     0,   356,   224,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,   213,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,   213,   379,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   213,   742,     0,     0,   736,     0,
       0,     0,     0,     0,     0,     0,     0,   978,   224,   742,
     742,   742,   742,   742,     0,     0,     0,     0,     0,     0,
       0,     0,   742,     0,     0,     0,     0,   224,     0,     0,
       0,   736,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   736,     0,     0,     0,     0,   736,   991,     0,
     736,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     259,   259,     0,     0,     0,   213,   259,     0,     0,  1055,
       0,     0,     0,     0,     0,     0,  1009,     0,   213,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1009,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,   736,     0,     0,
       0,     0,     0,  1064,     0,   224,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,   742,     0,     0,  1048,   224,     0,
       0,     0,   259,     0,     0,   259,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,   225,     0,     0,
       0,     0,   736,     0,     0,     0,     0,     0,     0,     0,
     736,     0,     0,     0,   736,     0,     0,   736,     0,     0,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   213,     0,
       0,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,     0,     0,     0,
     742,     0,     0,   213,   742,   742,   742,   379,     0,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,   742,   742,     0,     0,     0,     0,
       0,   353,   354,   355,     0,   259,   716,     0,     0,   738,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   742,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,   353,   354,   355,     0,     0,
       0,     0,     0,     0,   738,     0,   379,     0,     0,   213,
       0,     0,     0,     0,   356,  1253,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
     379,   213,     0,   259,   259,     0,  1398,     0,     0,     0,
       0,   259,     0,     0,     0,     0,   213,   213,     0,   742,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   742,     0,   742,     0,     0,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   356,   742,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   355,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,  1399,  1288,     0,   356,   213,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,   742,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,  1254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   738,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   259,
     259,   738,   738,   738,   738,   738,     0,     0,     0,     0,
       0,     0,     0,     0,   738,     0,     0,     0,     0,     0,
       0,     0,   742,   213,     0,     0,     0,   742,     0,   742,
       0,   742,     0,     0,   742,     0,   742,     0,     0,   742,
       0,     0,     0,     0,     0,     0,     0,     0,  1370,     0,
    1379,     0,     0,     0,   251,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   380,     0,   742,     0,     0,     0,     0,     0,     0,
     252,     0,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,   259,     0,     0,
     742,   353,   354,   355,     0,     0,     0,     0,   259,     0,
       0,     0,     0,  1435,  1436,     0,   738,     0,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,   253,   254,   742,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,   180,     0,     0,    81,   255,     0,
      83,    84,     0,    85,   181,    87,   742,   742,   742,     0,
       0,     0,     0,     0,   742,  1573,     0,     0,   256,  1379,
       0,     0,     0,   259,     0,   259,     0,   716,    36,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   738,     0,     0,   257,   738,   738,   738,  1541,
       0,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   738,   378,   302,     0,     0,    83,    84,     0,    85,
     181,    87,     0,     0,   379,     0,     0,     0,     0,     0,
       0,     0,   742,   461,     0,     0,     0,     0,     0,     0,
       0,     0,   259,     0,   259,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,     0,     0,     0,   303,   742,     0,     0,   259,   353,
     354,   355,     0,     0,     0,     0,   742,     0,     0,     0,
       0,   742,     0,     0,   742,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   738,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   259,   379,     0,   738,     0,   738,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   742,     0,   353,   354,   355,     0,     0,     0,  1674,
       0,   738,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   356,  1370,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   259,     0,     0,
       0,     0,     0,     0,     0,     0,   742,     0,   379,   353,
     354,   355,     0,     0,   742,     0,     0,     0,   742,     0,
       0,   742,     0,     0,     0,   738,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,   259,     0,   259,
       0,     0,     0,     0,   738,     0,     0,     0,     0,   738,
       0,   738,     0,   738,     0,     0,   738,     0,   738,     0,
       0,   738,     0,     0,     0,     0,     0,     0,     0,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   259,
       0,     0,     0,     0,     0,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,   738,     0,     0,     0,     0,
       0,     0,     0,     0,   356,   474,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,     0,   738,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,     0,   259,     0,     0,     0,   185,   187,
     738,   189,   190,   191,   193,   194,     0,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,     0,
       0,   218,   221,   743,     0,     0,     0,     0,   738,   738,
     738,     0,     0,     0,   239,     0,   738,     0,     0,     0,
       0,   247,     0,   250,     0,     0,   266,     0,   271,     0,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   743,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,   675,     0,   953,   954,
     955,    36,     0,   209,     0,   379,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   956,     0,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
       0,     0,     0,     0,   738,     0,     0,   635,     0,     0,
       0,     0,     0,   978,   259,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,  1615,   378,     0,     0,   738,     0,    83,
      84,   422,    85,   181,    87,     0,   379,     0,   738,     0,
       0,     0,     0,   738,     0,     0,   738,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,     0,   446,     0,   636,   446,   116,
       0,     0,     0,     0,     0,   239,   457,     0,     0,     0,
       0,   694,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   738,     0,     0,     0,     0,     0,     0,
       0,   743,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   743,   743,   743,   743,   743,
       0,   320,     0,     0,   259,     0,     0,   218,   743,  1133,
       0,   536,     0,     0,     0,     0,     0,     0,     0,   259,
       0,     0,     0,     0,   557,   560,   560,     0,   738,     0,
       0,     0,     0,     0,     0,     0,   738,   583,     0,     0,
     738,     0,     0,   738,     0,     0,     0,     0,   595,     0,
       0,   870,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   209,     0,     0,     0,   601,   602,   603,   605,
     606,   607,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,     0,   628,     0,   629,   629,     0,   632,     0,     0,
     210,     0,     0,     0,     0,   649,   651,   652,   653,   654,
     655,   656,   657,   658,   659,   660,   661,   662,     0,     0,
     743,     0,     0,   629,   669,     0,   595,   629,   672,     0,
       0,     0,   180,     0,   649,    81,    82,   676,    83,    84,
       0,    85,   181,    87,     0,     0,   685,     0,   687,     0,
       0,     0,     0,     0,   595,     0,     0,     0,   737,     0,
       0,     0,   699,     0,   700,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   211,     0,     0,     0,     0,   116,     0,
     748,     0,     0,   751,   754,   755,     0,     0,     0,     0,
       0,     0,     0,   737,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   743,     0,     0,     0,
     743,   743,   743,     0,   774,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,     0,     0,     0,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,   743,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,   843,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,   854,   356,    36,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   251,   378,     0,     0,     0,     0,     0,     0,   862,
       0,     0,     0,     0,   379,     0,     0,     0,   193,     0,
       0,     0,     0,     0,     0,     0,     0,   252,     0,     0,
       0,     0,     0,     0,     0,   743,   872,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
     743,   180,   743,     0,    81,     0,   737,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,     0,
     737,   737,   737,   737,   737,   743,     0,     0,   239,     0,
       0,     0,     0,   737,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   987,     0,   253,   254,     0,  1613,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   979,
       0,   180,     0,     0,    81,   255,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,   743,
       0,   983,   984,     0,    36,   256,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   257,  1016,     0,     0,  1606,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1024,     0,     0,     0,
       0,     0,  1027,     0,  1028,   737,  1029,     0,   743,     0,
       0,     0,     0,   743,     0,   743,     0,   743,     0,     0,
     743,     0,   743,     0,     0,   743,     0,     0,  1044,     0,
     744,     0,    83,    84,     0,    85,   181,    87,  1052,     0,
       0,  1053,     0,  1054,     0,     0,     0,   595,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   743,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,   776,     0,     0,     0,     0,
     663,  1085,   116,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   743,     0,     0,     0,
       0,   737,     0,     0,     0,   737,   737,   737,     0,     0,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,     0,     0,     0,
       0,     0,     0,     0,   743,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     737,     0,     0,  1166,  1167,  1168,     0,     0,     0,   751,
    1170,     0,   743,   743,   743,     0,     0,   353,   354,   355,
     743,     0,     0,     0,  1576,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,  1185,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,  1210,     0,     0,     0,
       0,     0,   379,     0,     0,     0,     0,     0,     0,   595,
       0,     0,     0,     0,     0,     0,     0,     0,   595,  1185,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     737,     0,     0,     0,     0,     0,     0,     0,   915,     0,
       0,     0,     0,     0,     0,   737,    36,   737,   209,   239,
       0,     0,   936,   937,   938,   939,     0,     0,   743,  1252,
       0,     0,     0,     0,     0,   950,     0,     0,     0,     0,
     737,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   743,   378,     0,   251,     0,     0,     0,     0,     0,
       0,     0,   743,     0,   379,     0,     0,   743,     0,     0,
     743,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     252,     0,     0,     0,    83,    84,  1611,    85,   181,    87,
       0,     0,  1652,     0,   737,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,  1308,     0,     0,     0,  1309,
       0,  1310,  1311,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   743,     0,  1324,
     595,     0,   636,     0,   116,     0,     0,  1045,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   737,     0,     0,   253,   254,   737,     0,
     737,     0,   737,     0,     0,   737,     0,   737,     0,     0,
     737,     0,     0,     0,   180,     0,     0,    81,   255,     0,
      83,    84,   743,    85,   181,    87,     0,   931,     0,     0,
     743,     0,     0,     0,   743,     0,  1362,   743,   256,     0,
       0,     0,     0,     0,   737,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   257,     0,     0,     0,     0,
       0,   595,     0,     0,     0,     0,     0,  1126,  1129,  1129,
       0,   737,  1136,  1139,  1140,  1141,  1143,  1144,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,     0,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   737,
       0,   356,  1176,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,   737,   737,   737,
       0,     0,     0,     0,     0,   737,     0,   379,   353,   354,
     355,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1568,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,   880,   881,
       0,     0,  1248,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1262,   882,  1263,
       0,     0,     0,     0,     0,     0,   883,   884,   885,    36,
       0,   251,     0,     0,     0,     0,     0,   886,     0,     0,
       0,     0,  1281,   737,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   252,     0,     0,
    1432,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   737,     0,     0,    36,
       0,     0,     0,   887,     0,     0,     0,   737,     0,     0,
       0,     0,   737,     0,     0,   737,   888,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1317,    83,    84,     0,
      85,   181,    87,     0,     0,  1283,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   889,     0,     0,     0,     0,
       0,     0,     0,   253,   254,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   180,   737,     0,    81,   255,     0,    83,    84,     0,
      85,   181,    87,     0,  1259,  1354,     0,     0,     0,     0,
    1356,     0,  1357,     0,  1358,   256,     0,  1359,     0,  1360,
       0,     0,  1361,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   257,     0,     0,     0,     0,   737,     5,     6,
       7,     8,     9,     0,     0,   737,  1403,     0,    10,   737,
       0,     0,   737,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   447,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,  1428,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,  1546,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     176,   177,    63,     0,    64,    65,    66,     0,     0,  1565,
    1566,  1567,     0,     0,     0,    70,    71,  1572,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   954,   955,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     0,     0,     0,     0,     0,
       0,   956,     0,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,     0,  1598,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   978,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1621,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,  1631,
       0,     0,     0,     0,  1635,     0,     0,  1637,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,  1668,     0,    48,    49,    50,    51,
      52,    53,    54,     0,    55,    56,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,    67,    68,    69,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,    78,    79,    80,    81,    82,  1718,
      83,    84,     0,    85,    86,    87,    88,  1723,     0,    89,
       0,  1725,    90,     0,  1726,     0,     0,     0,    91,    92,
      93,    94,    95,     0,    96,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1017,
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
       0,     0,   113,     0,   114,   115,  1187,   116,   117,     0,
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
     576,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
     112,     0,     0,   113,     0,   114,   115,   990,   116,   117,
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
     113,     0,   114,   115,  1093,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,  1095,    44,     0,    45,
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
      41,    42,    43,     0,    44,     0,    45,     0,    46,  1249,
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
       0,   113,     0,   114,   115,  1364,   116,   117,     0,   118,
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
     114,   115,  1569,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,  1610,    46,
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
       0,     0,   113,     0,   114,   115,  1641,   116,   117,     0,
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
       0,   114,   115,  1642,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,  1645,    45,     0,
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
     112,     0,     0,   113,     0,   114,   115,  1660,   116,   117,
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
     113,     0,   114,   115,  1713,   116,   117,     0,   118,   119,
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
     115,  1719,   116,   117,     0,   118,   119,     5,     6,     7,
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
       0,   702,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,   917,     0,
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
       0,     0,    11,    12,     0,  1423,     0,     0,     0,     0,
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
      12,     0,  1560,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
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
       0,     0,     0,   645,    12,     0,     0,     0,     0,     0,
       0,   646,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
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
      92,    93,    94,     0,     0,     0,     0,    97,    98,   263,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
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
     112,     0,     0,   113,     0,   264,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   956,    10,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   591,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   978,     0,
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
      87,     0,   592,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,   354,   355,
     697,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   356,    10,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,  1042,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   379,     0,     0,     0,    16,     0,    17,    18,
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
      84,     0,    85,   181,    87,     0,  1043,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   645,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
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
     181,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,    94,     0,    10,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   192,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   955,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   956,    10,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   978,     0,     0,     0,    16,     0,    17,    18,    19,
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
      90,     5,     6,     7,     8,     9,    91,    92,    93,    94,
       0,    10,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     0,    14,    15,     0,     0,     0,     0,
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
     246,     0,     0,   116,   117,     0,   118,   119,     0,    14,
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
      90,     5,     6,     7,     8,     9,    91,    92,    93,    94,
       0,    10,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   249,     0,     0,   116,   117,
       0,   118,   119,     0,    14,    15,     0,     0,     0,     0,
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
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,   445,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   604,   378,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
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
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   646,   378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     379,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   684,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   978,     0,     0,
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
       0,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   686,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   978,     0,     0,     0,    16,     0,
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
       9,     0,     0,     0,     0,     0,    10,     0,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,  1084,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   379,     0,     0,     0,    16,     0,    17,    18,    19,
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
      90,     5,     6,     7,     8,     9,    91,    92,    93,    94,
       0,    10,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     0,    14,    15,     0,     0,     0,     0,
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
      36,   535,    38,     0,     0,     0,     0,     0,    40,     0,
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
       0,   118,   119,  1444,  1445,  1446,  1447,  1448,     0,     0,
    1449,  1450,  1451,  1452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1453,  1454,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,     0,     0,     0,
       0,     0,     0,  1455,     0,     0,     0,     0,     0,     0,
     978,     0,     0,     0,     0,     0,     0,  1456,  1457,  1458,
    1459,  1460,  1461,  1462,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,  1463,  1464,  1465,  1466,
    1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,  1476,
    1477,  1478,  1479,  1480,  1481,  1482,  1483,  1484,  1485,  1486,
    1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,  1496,
    1497,  1498,  1499,  1500,  1501,  1502,  1503,   251,     0,     0,
    1504,  1505,     0,  1506,  1507,  1508,  1509,  1510,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1511,
    1512,  1513,     0,   252,     0,    83,    84,     0,    85,   181,
      87,  1514,     0,  1515,  1516,     0,  1517,     0,     0,     0,
       0,     0,     0,  1518,     0,    36,     0,  1519,     0,  1520,
       0,  1521,  1522,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   251,     0,     0,
       0,     0,  -319,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   343,     0,     0,     0,     0,
       0,     0,     0,   252,     0,     0,     0,     0,     0,   253,
     254,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   180,     0,     0,
      81,   255,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     251,   256,   468,     0,     0,     0,     0,     0,     0,     0,
     344,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   252,     0,   257,   253,
     254,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   180,    36,     0,
      81,   255,     0,    83,    84,     0,    85,   181,    87,     0,
     978,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   256,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   257,     0,
       0,     0,   253,   254,     0,     0,     0,     0,    36,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     180,     0,     0,    81,   255,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   256,     0,     0,     0,   210,     0,
       0,     0,  1142,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   722,   723,
       0,   257,     0,     0,   724,     0,   725,     0,     0,     0,
     180,     0,     0,    81,    82,     0,    83,    84,   726,    85,
     181,    87,     0,     0,     0,     0,    33,    34,    35,    36,
       0,     0,     0,     0,     0,     0,   911,   727,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   211,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
     209,     0,     0,   728,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,   729,     0,     0,     0,
       0,   180,    79,    80,    81,   730,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,   210,     0,
       0,     0,     0,     0,     0,   731,   732,   733,   734,     0,
       0,   912,    36,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     180,     0,   735,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   978,   722,
     723,   211,     0,     0,     0,   724,   116,   725,  1528,     0,
      83,    84,  1529,    85,   181,    87,     0,     0,     0,   726,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,   727,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,  1384,     0,     0,     0,     0,
       0,     0,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,    36,
     378,   209,     0,     0,   728,     0,    72,    73,    74,    75,
      76,     0,   379,     0,     0,     0,     0,   729,     0,     0,
       0,     0,   180,    79,    80,    81,   730,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,   210,
       0,     0,     0,     0,     0,     0,   731,   732,   733,   734,
       0,     0,   527,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   180,     0,   735,    81,    82,     0,    83,    84,     0,
      85,   181,    87,    36,     0,   209,     0,     0,     0,     0,
    -885,  -885,  -885,  -885,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   978,
       0,     0,   211,   210,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,  1012,    36,     0,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   180,     0,     0,    81,    82,
      36,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,  1383,     0,   211,     0,     0,   180,
       0,   116,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    36,     0,   209,     0,     0,     0,     0,     0,   549,
       0,     0,     0,     0,     0,     0,     0,     0,    83,    84,
       0,    85,   181,    87,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     223,   210,     0,    36,     0,   116,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    83,    84,  1384,    85,   181,    87,     0,     0,     0,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,  1067,  1068,  1069,    36,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   597,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    83,    84,     0,    85,   181,    87,     0,     0,   550,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   852,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     353,   354,   355,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,   353,   354,   355,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,   356,     0,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   379,
       0,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   424,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,   433,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     821,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,     0,   379,     0,     0,
       0,     0,     0,     0,   858,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   898,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   953,   954,   955,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,  1199,   956,     0,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   978,     0,   953,   954,   955,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1216,   956,     0,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   953,   954,   955,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   978,
       0,     0,     0,     0,     0,   956,  1117,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   978,     0,   953,   954,   955,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   956,  1272,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   953,   954,   955,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   978,     0,
       0,     0,     0,     0,   956,  1277,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
     978,   715,   953,   954,   955,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,   956,  1353,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1371,   978,     0,     0,
       0,    36,     0,     0,  1430,   180,     0,     0,    81,  1372,
    1373,    83,    84,     0,    85,   181,    87,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,   272,   273,
      81,  1374,     0,    83,    84,     0,    85,  1375,    87,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,     0,     0,     0,     0,
       0,  1431,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,   274,   795,   796,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    36,   180,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,   340,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,     0,
       0,     0,   499,     0,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    36,   503,     0,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     635,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,   274,     0,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,  1134,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   706,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,   356,   855,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   707,   378,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
       0,     0,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   953,   954,   955,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,     0,   956,  1282,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   953,
     954,   955,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   978,     0,     0,     0,     0,     0,   956,     0,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   978,  -885,  -885,  -885,  -885,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379
};

static const yytype_int16 yycheck[] =
{
       4,   183,   162,     4,   135,    88,   594,   310,     4,     4,
      32,    52,    95,    96,   567,     4,     4,     4,     4,    53,
     832,    43,   413,    30,   224,    47,   418,  1035,   392,   165,
     378,   568,   162,  1032,   705,   443,   230,   677,   814,   113,
      26,    27,   447,  1021,  1019,    49,     4,   542,    52,   712,
     441,   134,   851,   184,   251,   252,     9,     9,   443,     9,
     257,    30,     9,    45,   113,    69,     9,     9,   867,     9,
       4,     9,     9,     9,     9,    30,     9,   172,   220,    35,
       9,     9,    86,     9,    88,     9,     9,     9,     9,    66,
     495,    95,    96,    33,     9,   231,     9,     9,    79,     9,
      79,   295,     9,    35,     9,    50,    66,    79,   910,   127,
     128,    45,   382,    84,   108,    99,   100,    14,    45,   127,
     128,   127,   128,    79,   127,   128,    56,   113,    45,    96,
     134,    45,     0,    30,   200,    66,   333,   211,  1554,   135,
     410,   200,    96,    45,   414,   200,    66,    79,    78,   223,
      47,    81,    98,    99,   100,    35,   148,    96,    99,   100,
       8,   162,    66,   345,   145,   146,   147,   127,   128,   169,
     946,   105,   166,    66,   145,    66,   110,   169,   112,   113,
     114,   115,   116,   117,   118,   152,   204,    66,   184,    66,
      79,  1607,   196,   201,    66,   201,    66,    66,   152,    79,
     200,    66,    66,    66,    66,   203,   162,   200,   153,    66,
     200,   203,  1211,   152,   288,   310,   198,   148,   197,   201,
     204,   155,   156,   200,   158,   211,   203,   170,   170,    79,
     162,   217,   236,   205,   203,  1213,   240,   223,   420,   288,
     244,   236,  1220,   203,  1222,   240,  1221,   202,   182,   201,
     236,   204,   202,   203,   240,   202,   339,   920,   262,   922,
     391,   201,  1061,   204,   202,   202,   202,   202,   201,   466,
     204,   198,   203,   202,   202,   200,   202,    50,   202,   202,
     202,   202,   162,   203,   201,   427,   201,   201,   201,   201,
     385,   201,   697,   279,   201,   336,   201,   702,  1100,   201,
     286,   287,   288,  1079,    79,   388,   389,   293,    66,   804,
     203,   848,   203,   299,   409,    84,   205,    35,    96,   323,
      35,  1320,   323,  1322,   203,   123,   203,   323,   332,   123,
     200,   203,   336,   203,   203,   339,   431,   323,   203,   203,
     203,   203,   272,   273,   274,   440,   203,   152,   443,   200,
      96,    79,  1330,    53,   428,   205,    84,    96,   392,   200,
     318,    79,   200,   154,    79,    65,   200,   564,   565,   127,
     128,   378,   302,   148,   152,   572,   145,    35,   385,   573,
     153,   385,   386,   387,   388,   389,    26,    27,    79,   200,
      30,    46,    47,    84,   390,   168,    79,   202,   203,   791,
     200,    84,   409,    79,   153,   409,   152,   200,    84,    79,
     552,  1410,   203,   152,    84,   449,   154,   559,   146,   147,
     562,    79,   582,   200,   431,  1096,   200,   431,   836,   167,
      79,   203,   418,   440,     4,    84,   443,   637,   200,   443,
    1080,    79,   428,    30,   162,   152,    84,   200,    79,    35,
     454,   836,   582,    71,    72,   146,   147,    71,    72,   454,
     125,   126,   644,   146,   147,   203,   666,    29,   454,   145,
     146,   147,   202,   864,    66,    45,   146,   147,   202,   203,
     208,  1034,   202,   683,    46,   202,   148,    49,   682,   202,
    1266,   899,   634,   202,   688,    66,   145,   146,   147,   202,
     112,   113,   114,   115,   116,   117,   510,   169,   146,   147,
    1047,   814,   917,  1050,   899,   146,   147,   600,   209,  1583,
    1584,   104,   870,   720,   721,   202,   203,   531,   532,   112,
     113,   114,   115,   116,   117,   105,   736,    29,   200,    66,
     110,   203,   112,   113,   114,   115,   116,   117,   118,   112,
     113,   114,   688,   203,    46,  1579,  1580,    49,  1557,    49,
      50,    51,  1338,   112,   113,   114,   115,   116,   117,   499,
     182,   211,   202,   503,   148,    65,   718,   217,   508,   720,
     721,   582,    66,   223,   200,   155,   156,  1227,   158,  1229,
     202,    49,    50,    51,   200,    53,   600,   148,   200,   182,
      98,    99,   100,   202,   152,   591,    44,    65,   805,    65,
     568,   169,   182,   117,   118,   119,   148,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     207,   828,     9,   182,   204,   148,   148,     8,   202,   279,
     200,   169,   839,   946,   200,    14,   286,   287,  1201,    14,
    1014,    79,   296,   293,    14,   202,   300,   202,   201,   299,
     646,   169,    14,    96,   201,  1056,    63,    64,   201,   200,
     310,   206,   104,   677,  1065,   679,   200,  1685,  1215,   701,
    1679,   325,   200,   327,   328,   329,   330,     9,    88,     9,
     201,   695,  1700,   202,   695,  1694,  1336,    14,   684,   695,
     686,   201,     9,   200,   186,   709,   710,  1099,   189,   695,
       9,    79,    79,    79,   200,   710,     9,   202,    79,   814,
     125,   707,   202,   201,   710,   201,   201,   924,   202,   926,
     127,   128,   200,    66,    30,   126,   129,   168,   378,     9,
     201,   836,   112,   113,   114,   115,   116,   117,   201,   148,
       9,   893,   201,   123,   124,     9,   760,   201,    14,   760,
       9,   198,   766,    66,   760,     9,   770,   771,  1173,   170,
       9,   201,    14,   207,   760,   125,  1079,  1417,   418,   204,
     207,   200,     9,   207,   788,   201,   869,   207,   428,   159,
    1182,   161,   778,   935,  1331,   201,   200,   129,    96,   803,
     942,   148,   803,     9,   899,   791,   792,   803,   803,   148,
     202,   201,   182,   817,   803,   803,   803,   803,    46,    47,
      48,    49,    50,    51,   202,    53,  1217,   200,   200,   836,
     148,   186,   836,   186,  1239,   765,  1033,    65,  1035,   769,
    1040,   200,   200,  1655,   200,   803,   203,    14,     9,   203,
    1255,   946,    46,    47,    48,    49,    50,    51,     4,    79,
    1672,    14,  1059,   870,   202,   869,   203,    14,  1680,   203,
      14,    65,   207,   202,   832,   198,   880,   881,   882,    30,
      30,   903,   112,   113,   114,   115,   116,   117,   200,   200,
     848,     4,   899,   123,   124,   899,   200,    14,   200,    45,
     904,     9,   906,   200,   908,   906,   201,   908,   202,   904,
     906,   202,   908,   200,   129,    14,     9,  1114,   904,  1559,
     906,   201,   908,   927,   910,   911,    65,  1318,   207,     9,
      79,   161,    45,  1075,   145,  1340,    96,     9,   183,   184,
     185,   945,   200,   129,  1349,   190,   191,   951,    79,   194,
     195,   591,   182,    14,   202,    79,   203,   200,  1363,   105,
     201,   201,   200,  1266,   110,   129,   112,   113,   114,   115,
     116,   117,   118,   207,   203,     9,   145,   203,    30,    73,
    1014,  1178,  1124,   202,  1079,   201,  1128,   170,   202,  1131,
     994,   129,   105,   997,    30,   129,  1138,   110,   201,   112,
     113,   114,   115,   116,   117,   118,   646,     9,   201,   155,
     156,   201,   158,  1601,  1018,     9,   201,  1018,  1423,   201,
       9,   201,  1018,  1018,     9,   204,   201,   204,    14,  1018,
    1018,  1018,  1018,   203,   200,  1338,   182,   129,   201,   201,
       9,   200,   155,   156,   684,   158,   686,   201,   203,    26,
      27,    30,    96,    30,   202,   157,  1042,   202,   204,   201,
    1018,  1258,   202,  1260,   201,   201,  1088,   707,   201,   182,
     153,   201,  1076,    79,   203,    52,  1080,    14,    79,   110,
     201,   201,  1670,   129,   203,   201,   129,  1091,    14,  1047,
     202,   204,  1050,  1235,   203,    79,  1091,    14,  1084,  1103,
     201,   201,  1103,  1300,     4,  1091,   203,  1103,   129,   200,
      14,    14,   202,  1099,  1100,   202,   202,  1103,    14,   203,
      55,    74,    75,    76,    79,   200,     9,    79,   202,    79,
     108,    96,    85,   148,    96,   160,    33,    14,   778,   201,
     200,    79,  1302,   202,  1549,    45,  1551,   200,   166,   163,
     201,   791,   792,     9,    79,  1560,   203,   202,    79,   201,
     201,    14,    14,    79,    79,    14,    79,    14,  1365,  1300,
     765,  1266,   508,  1177,   814,   769,  1663,   389,   386,  1180,
     133,   134,   135,   136,   137,   865,   388,   868,  1189,   806,
    1676,   144,  1422,  1672,  1097,  1252,  1182,   150,   151,  1604,
     513,  1413,  1442,  1526,  1295,   105,  1704,  1389,  1692,  1538,
     110,   164,   112,   113,   114,   115,   116,   117,   118,    41,
    1409,   392,  1180,  1227,   989,  1229,   986,   484,   181,   484,
     870,  1189,   757,  1022,   211,  1063,   881,   943,   895,  1076,
     217,   949,   313,  1338,   294,    -1,   223,   287,   720,  1290,
    1005,    -1,    -1,   928,    -1,   155,   156,  1215,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     910,   911,    -1,    -1,   251,   252,    -1,    -1,    -1,    -1,
     257,  1285,   182,    -1,  1285,    -1,  1290,    -1,    -1,  1285,
      -1,  1295,    -1,  1698,    -1,    -1,    -1,    -1,     4,  1285,
    1705,  1302,   279,  1299,   204,    -1,   946,    -1,    -1,   286,
     287,    -1,    -1,    -1,    -1,    -1,   293,    -1,    -1,    -1,
    1321,    -1,   299,    -1,    -1,    -1,  1327,    -1,  1329,    -1,
      -1,    -1,  1336,   310,    -1,    -1,    -1,  1341,    -1,    45,
    1341,    -1,  1346,    -1,    -1,  1341,  1350,    -1,    -1,  1350,
    1391,  1346,    -1,    -1,  1350,  1341,   333,    -1,  1540,   336,
    1346,    -1,    -1,  1321,  1350,    -1,    -1,    -1,    -1,  1327,
      -1,  1329,    -1,  1331,  1378,    -1,    -1,    -1,    -1,    -1,
    1384,    -1,    -1,    -1,    -1,    -1,    -1,  1391,    -1,    -1,
      -1,  1395,    -1,    -1,    -1,    -1,    -1,  1393,    -1,   105,
      -1,   378,  1042,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,  1417,  1415,    -1,  1420,  1421,  1422,  1420,
      -1,    -1,  1426,    -1,  1420,  1426,  1421,  1422,    -1,  1433,
    1426,    -1,  1433,    -1,  1420,  1421,  1422,  1433,    -1,  1079,
    1426,   418,    -1,    -1,  1084,    -1,    -1,  1433,    -1,   155,
     156,   428,   158,    -1,  1537,    -1,    -1,  1415,    -1,  1099,
    1100,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,   182,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,  1685,   466,
     467,  1651,    -1,   470,    -1,    -1,    -1,    -1,   204,    -1,
      -1,    -1,    -1,  1700,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1595,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,  1537,    -1,    -1,  1667,    -1,   515,    -1,
      -1,    -1,  1182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1555,    -1,    -1,    -1,  1559,    -1,    -1,    -1,    -1,
    1564,    -1,    -1,  1564,    -1,    63,    64,    -1,  1564,    -1,
    1574,    -1,    -1,    -1,    -1,  1579,  1580,    -1,  1564,  1583,
    1584,    -1,    -1,    -1,    -1,    -1,    -1,   564,   565,    -1,
      -1,  1595,    -1,    -1,    -1,   572,    -1,    -1,  1602,  1603,
      -1,  1602,  1603,    -1,    -1,  1609,  1602,  1603,  1609,    -1,
      -1,    -1,    -1,  1609,   591,    -1,  1602,  1603,    -1,    -1,
      -1,    -1,    -1,  1609,    -1,    -1,  1266,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1643,
      -1,    -1,  1643,    -1,    -1,    -1,  1650,  1643,    -1,    -1,
    1651,    10,    11,    12,    -1,    -1,    -1,  1643,    -1,    -1,
      -1,    -1,  1666,    -1,    -1,    -1,    -1,    -1,    -1,   646,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,  1655,  1338,    -1,
      -1,    -1,  1706,   201,    -1,  1706,    65,   684,  1712,   686,
    1706,  1712,    -1,    -1,  1672,    -1,  1712,    -1,    -1,    -1,
    1706,    -1,  1680,    -1,    -1,    -1,  1712,    -1,    -1,    -1,
     707,   708,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   720,   721,   722,   723,   724,   725,   726,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   735,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,   761,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   778,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   789,    -1,   791,   792,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   805,   806,
      -1,    -1,    -1,    -1,    -1,    67,    68,   814,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    77,    -1,    79,   207,    -1,
      -1,   828,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   839,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     847,    -1,    -1,   850,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    26,    27,   117,    -1,    30,    -1,    -1,
      -1,    -1,    -1,   870,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   149,    53,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      65,    -1,   204,   910,   911,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   924,    -1,   926,
      -1,   928,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,   943,    -1,   200,   946,
     947,   948,   949,   205,    -1,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1002,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1033,    -1,  1035,    -1,
      65,    -1,    -1,    -1,    -1,  1042,    -1,    -1,    -1,   204,
      -1,    -1,    -1,    -1,   217,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1059,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,  1079,    -1,    -1,    -1,    -1,  1084,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1099,  1100,    -1,  1102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    -1,   279,  1114,    -1,    -1,
    1117,    -1,  1119,   286,   287,    -1,    -1,    -1,    -1,    -1,
     293,    10,    11,    12,    -1,    -1,   299,    63,    64,    -1,
      -1,    -1,    -1,    -1,    -1,  1142,    -1,   310,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,   127,   128,   204,
      -1,  1178,  1179,    -1,    -1,  1182,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,  1206,
      -1,    -1,    -1,    -1,    29,   378,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,   201,    -1,    -1,    -1,   418,    -1,    -1,    -1,    -1,
      -1,  1258,    -1,  1260,    77,    -1,    79,    -1,  1265,  1266,
      -1,    -1,    -1,  1270,    -1,  1272,    -1,  1274,    -1,    -1,
    1277,    -1,  1279,    -1,    -1,  1282,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1290,  1291,    -1,  1293,    -1,    -1,    -1,
      -1,    -1,    -1,  1300,   117,    -1,    -1,   470,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1316,
      -1,    -1,    -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,
     153,  1338,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,    -1,   515,    -1,    -1,    -1,  1353,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1365,  1366,
    1367,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,   204,
     203,    -1,   205,    -1,  1391,    -1,    -1,    -1,  1395,    -1,
      -1,    -1,    -1,    -1,  1401,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,   591,    -1,
      -1,    -1,  1429,  1430,  1431,    -1,    -1,    -1,    -1,    -1,
    1437,  1438,    -1,    -1,    29,  1442,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,   646,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   684,    -1,   686,    65,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   707,   708,    -1,    -1,  1545,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,  1555,   722,
     723,   724,   725,   726,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   735,    -1,    -1,    -1,    -1,  1574,    -1,    -1,
      -1,  1578,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1589,    -1,    -1,    -1,    -1,  1594,   761,    -1,
    1597,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     251,   252,    -1,    -1,    -1,   778,   257,    -1,    -1,   204,
      -1,    -1,    -1,    -1,    -1,    -1,   789,    -1,   791,   792,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   806,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   814,    -1,    -1,    -1,    -1,    -1,  1654,    -1,    -1,
      -1,    -1,    -1,   204,    -1,  1662,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1676,
      -1,    -1,    -1,    -1,   847,    -1,    -1,   850,  1685,    -1,
      -1,    -1,   333,    -1,    -1,   336,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1700,    -1,    -1,    -1,   870,    -1,    -1,
      -1,    -1,  1709,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1717,    -1,    -1,    -1,  1721,    -1,    -1,  1724,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   910,   911,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
     943,    -1,    -1,   946,   947,   948,   949,    65,    -1,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,   466,   467,    -1,    -1,   470,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1002,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,   515,    -1,    65,    -1,    -1,  1042,
      -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1079,    -1,    -1,    -1,
      65,  1084,    -1,   564,   565,    -1,   204,    -1,    -1,    -1,
      -1,   572,    -1,    -1,    -1,    -1,  1099,  1100,    -1,  1102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1117,    -1,  1119,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1142,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    12,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,   204,  1179,    -1,    29,  1182,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,  1206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   708,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   720,
     721,   722,   723,   724,   725,   726,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   735,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1265,  1266,    -1,    -1,    -1,  1270,    -1,  1272,
      -1,  1274,    -1,    -1,  1277,    -1,  1279,    -1,    -1,  1282,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1291,    -1,
    1293,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    -1,  1316,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,   805,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1338,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,   828,    -1,    -1,
    1353,    10,    11,    12,    -1,    -1,    -1,    -1,   839,    -1,
      -1,    -1,    -1,  1366,  1367,    -1,   847,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,   131,   132,  1401,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,  1429,  1430,  1431,    -1,
      -1,    -1,    -1,    -1,  1437,  1438,    -1,    -1,   173,  1442,
      -1,    -1,    -1,   924,    -1,   926,    -1,   928,    77,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   943,    -1,    -1,   200,   947,   948,   949,   204,
      -1,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1002,    53,   152,    -1,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1545,   202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1033,    -1,  1035,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,   203,  1578,    -1,    -1,  1059,    10,
      11,    12,    -1,    -1,    -1,    -1,  1589,    -1,    -1,    -1,
      -1,  1594,    -1,    -1,  1597,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1102,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1114,    65,    -1,  1117,    -1,  1119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1654,    -1,    10,    11,    12,    -1,    -1,    -1,  1662,
      -1,  1142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,  1676,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,  1178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1709,    -1,    65,    10,
      11,    12,    -1,    -1,  1717,    -1,    -1,    -1,  1721,    -1,
      -1,  1724,    -1,    -1,    -1,  1206,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,  1258,    -1,  1260,
      -1,    -1,    -1,    -1,  1265,    -1,    -1,    -1,    -1,  1270,
      -1,  1272,    -1,  1274,    -1,    -1,  1277,    -1,  1279,    -1,
      -1,  1282,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1290,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1300,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1316,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   202,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,  1353,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,  1365,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1391,    -1,    -1,    -1,  1395,    -1,    -1,    -1,     5,     6,
    1401,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    28,    29,   470,    -1,    -1,    -1,    -1,  1429,  1430,
    1431,    -1,    -1,    -1,    41,    -1,  1437,    -1,    -1,    -1,
      -1,    48,    -1,    50,    -1,    -1,    53,    -1,    55,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   515,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,   201,    -1,    10,    11,
      12,    77,    -1,    79,    -1,    65,   113,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,  1545,    -1,    -1,   123,    -1,    -1,
      -1,    -1,    -1,    65,  1555,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1574,    53,    -1,    -1,  1578,    -1,   155,
     156,   188,   158,   159,   160,    -1,    65,    -1,  1589,    -1,
      -1,    -1,    -1,  1594,    -1,    -1,  1597,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,    -1,   232,    -1,   203,   235,   205,
      -1,    -1,    -1,    -1,    -1,   242,   243,    -1,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1654,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   708,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   722,   723,   724,   725,   726,
      -1,   288,    -1,    -1,  1685,    -1,    -1,   294,   735,   201,
      -1,   298,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1700,
      -1,    -1,    -1,    -1,   311,   312,   313,    -1,  1709,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1717,   324,    -1,    -1,
    1721,    -1,    -1,  1724,    -1,    -1,    -1,    -1,   335,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,    -1,   379,    -1,   381,   382,    -1,   384,    -1,    -1,
     117,    -1,    -1,    -1,    -1,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,    -1,    -1,
     847,    -1,    -1,   410,   411,    -1,   413,   414,   415,    -1,
      -1,    -1,   149,    -1,   421,   152,   153,   424,   155,   156,
      -1,   158,   159,   160,    -1,    -1,   433,    -1,   435,    -1,
      -1,    -1,    -1,    -1,   441,    -1,    -1,    -1,   470,    -1,
      -1,    -1,   449,    -1,   451,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,
     477,    -1,    -1,   480,   481,   482,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   515,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   943,    -1,    -1,    -1,
     947,   948,   949,    -1,   511,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1002,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,   592,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   604,    29,    77,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    29,    53,    -1,    -1,    -1,    -1,    -1,    -1,   636,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   645,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1102,   663,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
    1117,   149,  1119,    -1,   152,    -1,   708,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     722,   723,   724,   725,   726,  1142,    -1,    -1,   705,    -1,
      -1,    -1,    -1,   735,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   196,    -1,   131,   132,    -1,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   746,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,  1206,
      -1,   192,   193,    -1,    77,   173,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,   800,    -1,    -1,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   813,    -1,    -1,    -1,
      -1,    -1,   819,    -1,   821,   847,   823,    -1,  1265,    -1,
      -1,    -1,    -1,  1270,    -1,  1272,    -1,  1274,    -1,    -1,
    1277,    -1,  1279,    -1,    -1,  1282,    -1,    -1,   845,    -1,
     470,    -1,   155,   156,    -1,   158,   159,   160,   855,    -1,
      -1,   858,    -1,   860,    -1,    -1,    -1,   864,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1316,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,   515,    -1,    -1,    -1,    -1,
     203,   898,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1353,    -1,    -1,    -1,
      -1,   943,    -1,    -1,    -1,   947,   948,   949,    -1,    -1,
     952,   953,   954,   955,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1401,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1002,    -1,    -1,   980,   981,   982,    -1,    -1,    -1,   986,
     987,    -1,  1429,  1430,  1431,    -1,    -1,    10,    11,    12,
    1437,    -1,    -1,    -1,  1441,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1014,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,  1056,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1065,  1066,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   708,    -1,
      -1,    -1,    -1,    -1,    -1,  1117,    77,  1119,    79,  1096,
      -1,    -1,   722,   723,   724,   725,    -1,    -1,  1545,  1106,
      -1,    -1,    -1,    -1,    -1,   735,    -1,    -1,    -1,    -1,
    1142,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1578,    53,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1589,    -1,    65,    -1,    -1,  1594,    -1,    -1,
    1597,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,   155,   156,   189,   158,   159,   160,
      -1,    -1,  1619,    -1,  1206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,  1192,    -1,    -1,    -1,  1196,
      -1,  1198,  1199,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,  1654,    -1,  1216,
    1217,    -1,   203,    -1,   205,    -1,    -1,   847,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1265,    -1,    -1,   131,   132,  1270,    -1,
    1272,    -1,  1274,    -1,    -1,  1277,    -1,  1279,    -1,    -1,
    1282,    -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,
     155,   156,  1709,   158,   159,   160,    -1,   162,    -1,    -1,
    1717,    -1,    -1,    -1,  1721,    -1,  1283,  1724,   173,    -1,
      -1,    -1,    -1,    -1,  1316,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,  1318,    -1,    -1,    -1,    -1,    -1,   947,   948,   949,
      -1,  1353,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1401,
      -1,    29,  1002,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,  1429,  1430,  1431,
      -1,    -1,    -1,    -1,    -1,  1437,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1432,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1117,    66,  1119,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,  1142,  1545,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1578,    -1,    -1,    77,
      -1,    -1,    -1,   131,    -1,    -1,    -1,  1589,    -1,    -1,
      -1,    -1,  1594,    -1,    -1,  1597,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1206,   155,   156,    -1,
     158,   159,   160,    -1,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   149,  1654,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,   162,  1265,    -1,    -1,    -1,    -1,
    1270,    -1,  1272,    -1,  1274,   173,    -1,  1277,    -1,  1279,
      -1,    -1,  1282,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,  1709,     3,     4,
       5,     6,     7,    -1,    -1,  1717,  1316,    -1,    13,  1721,
      -1,    -1,  1724,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,  1353,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,  1401,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,    -1,  1429,
    1430,  1431,    -1,    -1,    -1,   130,   131,  1437,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    11,    12,   200,    -1,   202,   203,    -1,
     205,   206,    -1,   208,   209,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,  1545,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1578,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,  1589,
      -1,    -1,    -1,    -1,  1594,    -1,    -1,  1597,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    -1,  1654,    -1,   101,   102,   103,   104,
     105,   106,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,  1709,
     155,   156,    -1,   158,   159,   160,   161,  1717,    -1,   164,
      -1,  1721,   167,    -1,  1724,    -1,    -1,    -1,   173,   174,
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
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
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
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
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
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     197,    -1,    -1,   200,    -1,   202,    -1,    -1,   205,   206,
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
     160,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
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
     153,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    11,    12,
     203,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
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
     156,    -1,   158,   159,   160,    -1,   162,    -1,   164,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
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
     159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,     3,
       4,     5,     6,     7,   173,   174,   175,   176,    -1,    13,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,
     209,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,    12,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,
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
     167,     3,     4,     5,     6,     7,   173,   174,   175,   176,
      -1,    13,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
     202,    -1,    -1,   205,   206,    -1,   208,   209,    -1,    46,
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
     167,     3,     4,     5,     6,     7,   173,   174,   175,   176,
      -1,    13,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,    -1,    -1,   205,   206,
      -1,   208,   209,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,   201,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    30,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
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
     195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
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
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,
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
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
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
     167,     3,     4,     5,     6,     7,   173,   174,   175,   176,
      -1,    13,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    29,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,
     150,   151,    -1,    55,    -1,   155,   156,    -1,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,   173,    -1,    77,    -1,   177,    -1,   179,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    29,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,   149,    -1,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   173,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    55,    -1,   200,   131,
     132,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   149,    77,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,   131,   132,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,   117,    -1,
      -1,    -1,    30,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    46,    47,
      -1,   200,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,
     149,    -1,    -1,   152,   153,    -1,   155,   156,    66,   158,
     159,   160,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,   164,    -1,   117,    -1,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
      -1,   130,    77,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
     149,    -1,   200,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    65,    46,
      47,   200,    -1,    -1,    -1,    52,   205,    54,   153,    -1,
     155,   156,   157,   158,   159,   160,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    77,
      53,    79,    -1,    -1,   131,    -1,   133,   134,   135,   136,
     137,    -1,    65,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,   130,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,   149,    -1,   200,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    77,    -1,    79,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    65,
      -1,    -1,   200,   117,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,   153,
      77,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    77,   121,    -1,   200,    -1,    -1,   149,
      -1,   205,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   155,   156,
      -1,   158,   159,   160,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
     200,   117,    -1,    77,    -1,   205,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,   155,   156,   200,   158,   159,   160,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    74,    75,    76,    77,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,   200,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   155,   156,    -1,   158,   159,   160,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,   200,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    29,
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
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    85,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    29,   129,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    65,    -1,    -1,
      -1,    77,    -1,    -1,   129,   149,    -1,    -1,   152,   131,
     132,   155,   156,    -1,   158,   159,   160,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,   104,   105,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    77,   152,    79,    80,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    77,   149,
      -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    77,    -1,
      -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    77,   152,    -1,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,   152,    -1,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   123,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    96,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65
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
     293,   294,   123,   123,    79,   295,   200,   200,   200,   200,
     217,   265,   470,   200,   200,    79,    84,   145,   146,   147,
     461,   462,   152,   203,   224,   224,   217,   266,   470,   153,
     200,   200,   200,   200,   470,   470,    79,   197,   350,   331,
     341,   342,   434,   230,   203,    84,   403,   461,    84,   461,
     461,    30,   152,   169,   471,   200,     9,   202,    35,   247,
     153,   264,   470,   117,   182,   248,   326,   202,   202,   202,
     202,   202,   202,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    53,    65,
     202,    66,    66,   202,   203,   148,   124,   159,   161,   267,
     324,   325,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    63,    64,   127,   128,   428,
      66,   203,   431,   200,    66,   203,   205,   440,   200,   247,
     248,    14,   341,   202,   129,    44,   217,   423,   200,   331,
     434,   148,   434,   129,   207,     9,   410,   331,   434,   471,
     148,   200,   404,   428,   431,   201,   341,    30,   232,     8,
     352,     9,   202,   232,   233,   333,   334,   341,   217,   279,
     236,   202,   202,   202,   482,   482,   169,   200,   104,   482,
      14,   217,    79,   202,   202,   202,   183,   184,   185,   190,
     191,   194,   195,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   383,   384,   385,   242,   108,   166,   202,   152,
     219,   222,   224,   152,   220,   223,   224,   224,     9,   202,
      96,   203,   434,     9,   202,    14,     9,   202,   434,   465,
     465,   331,   342,   434,   201,   169,   259,   130,   434,   446,
     447,    66,   428,   145,   462,    78,   341,   434,    84,   145,
     462,   224,   216,   202,   203,   254,   262,   389,   391,    85,
     205,   353,   354,   356,   400,   440,   466,   341,   454,   456,
     341,   453,   455,   453,    14,    96,   467,   349,   351,   289,
     290,   426,   427,   201,   201,   201,   204,   231,   232,   249,
     256,   261,   426,   341,   206,   208,   209,   217,   472,   473,
     482,    35,   162,   291,   292,   341,   469,   200,   470,   257,
     247,   341,   341,   341,    30,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   401,   341,   341,
     442,   442,   341,   449,   450,   123,   203,   217,   439,   440,
     265,   217,   266,   264,   248,    27,    35,   335,   338,   341,
     368,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   203,   217,   429,   430,   439,   442,   341,
     291,   442,   341,   446,   247,   201,   341,   200,   422,     9,
     410,   331,   201,   217,    35,   341,    35,   341,   201,   201,
     439,   291,   429,   430,   201,   230,   283,   203,   338,   341,
     341,    88,    30,   232,   277,   202,    28,    96,    14,     9,
     201,    30,   203,   280,   482,    85,   228,   478,   479,   480,
     200,     9,    46,    47,    52,    54,    66,    85,   131,   144,
     153,   173,   174,   175,   176,   200,   225,   226,   228,   363,
     364,   365,   399,   405,   406,   407,   186,    79,   341,    79,
      79,   341,   380,   381,   341,   341,   373,   383,   189,   386,
     230,   200,   240,   224,   202,     9,    96,   224,   202,     9,
      96,    96,   221,   217,   341,   294,   406,    79,     9,   201,
     201,   201,   201,   201,   202,    46,    47,   476,   477,   125,
     270,   200,     9,   201,   201,    79,    80,   217,   463,   217,
      66,   204,   204,   213,   215,    30,   126,   269,   168,    50,
     153,   168,   393,   129,     9,   410,   201,   148,   201,     9,
     410,   129,   201,     9,   410,   201,   482,   482,    14,   352,
     289,   198,     9,   411,   482,   483,   428,   431,   204,     9,
     410,   170,   434,   341,   201,     9,   411,    14,   345,   250,
     125,   268,   200,   470,   341,    30,   207,   207,   129,   204,
       9,   410,   341,   471,   200,   260,   255,   263,   258,   247,
      68,   434,   341,   471,   207,   204,   201,   207,   204,   201,
      46,    47,    66,    74,    75,    76,    85,   131,   144,   173,
     217,   413,   415,   418,   421,   217,   434,   434,   129,   428,
     431,   201,   284,    71,    72,   285,   230,   332,   230,   334,
      96,    35,   130,   274,   434,   406,   217,    30,   232,   278,
     202,   281,   202,   281,     9,   170,   129,   148,     9,   410,
     201,   162,   472,   473,   474,   472,   406,   406,   406,   406,
     406,   409,   412,   200,    84,   148,   200,   200,   200,   200,
     406,   148,   203,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    65,   341,
     186,   186,    14,   192,   193,   382,     9,   196,   386,    79,
     204,   399,   203,   244,    96,   222,   217,    96,   223,   217,
     217,   204,    14,   434,   202,     9,   170,   217,   271,   399,
     203,   446,   130,   434,    14,   207,   341,   204,   213,   482,
     271,   203,   392,    14,   341,   353,   217,   341,   341,   341,
     202,   482,   198,    30,   475,   427,    35,    79,   162,   429,
     430,   482,    35,   162,   341,   406,   289,   200,   399,   269,
     346,   251,   341,   341,   341,   204,   200,   291,   270,    30,
     269,   268,   470,   401,   204,   200,    14,    74,    75,    76,
     217,   414,   414,   415,   416,   417,   200,    84,   145,   200,
       9,   410,   201,   422,    35,   341,   429,   430,    71,    72,
     286,   332,   232,   204,   202,    89,   202,   274,   434,   200,
     129,   273,    14,   230,   281,    98,    99,   100,   281,   204,
     482,   482,   217,   478,     9,   201,   410,   129,   207,     9,
     410,   409,   217,   353,   355,   357,   406,   458,   460,   406,
     457,   459,   457,   201,   123,   217,   406,   451,   452,   406,
     406,   406,    30,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   341,   341,   341,   381,
     341,   371,    79,   245,   217,   217,   406,   477,    96,     9,
     299,   201,   200,   335,   338,   341,   207,   204,   467,   299,
     154,   167,   203,   388,   395,   154,   203,   394,   129,   129,
     202,   475,   482,   352,   483,    79,    14,    79,   471,   434,
     341,   201,   289,   203,   289,   200,   129,   200,   291,   201,
     203,   482,   203,   269,   252,   404,   291,   129,   207,     9,
     410,   416,   145,   353,   419,   420,   415,   434,   332,    30,
      73,   232,   202,   334,   273,   446,   274,   201,   406,    95,
      98,   202,   341,    30,   202,   282,   204,   170,   129,   162,
      30,   201,   406,   406,   201,   129,     9,   410,   201,   201,
       9,   410,   129,   201,     9,   410,   201,   129,   204,     9,
     410,   406,    30,   187,   201,   230,   217,   482,   399,     4,
     105,   110,   118,   155,   156,   158,   204,   300,   323,   324,
     325,   330,   426,   446,   204,   203,   204,    50,   341,   341,
     341,   341,   352,    35,    79,   162,    14,   406,   200,   475,
     201,   299,   201,   289,   341,   291,   201,   299,   467,   299,
     203,   200,   201,   415,   415,   201,   129,   201,     9,   410,
      30,   230,   202,   201,   201,   201,   237,   202,   202,   282,
     230,   482,   482,   129,   406,   353,   406,   406,   406,   406,
     406,   406,   341,   203,   204,    96,   125,   126,   469,   272,
     399,   118,   131,   132,   153,   159,   309,   310,   311,   399,
     157,   315,   316,   121,   200,   217,   317,   318,   301,   248,
     482,     9,   202,   324,   201,   296,   153,   390,   204,   204,
      79,    14,    79,   406,   291,   110,   343,   475,   204,   475,
     201,   201,   204,   203,   204,   299,   289,   129,   415,   353,
     230,   235,   238,    30,   232,   276,   230,   201,   406,   129,
     129,   129,   188,   230,   482,   399,   399,    14,     9,   202,
     203,   203,     9,   202,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    53,    67,    68,    69,    70,
      71,    72,    73,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   130,   131,   133,   134,   135,   136,
     137,   149,   150,   151,   161,   163,   164,   166,   173,   177,
     179,   181,   182,   217,   396,   397,     9,   202,   153,   157,
     217,   318,   319,   320,   202,    79,   329,   247,   302,   469,
     248,   204,   297,   298,   469,    14,   406,   201,   200,   203,
     202,   203,   321,   343,   475,   296,   204,   201,   415,   129,
      30,   232,   275,   276,   230,   406,   406,   406,   341,   204,
     202,   202,   406,   399,   305,   312,   405,   310,    14,    30,
      47,   313,   316,     9,    33,   201,    29,    46,    49,    14,
       9,   202,   470,   329,    14,   247,   202,    14,   406,    35,
      79,   387,   230,   230,   203,   321,   204,   475,   415,   230,
      93,   189,   243,   204,   217,   228,   306,   307,   308,     9,
     204,   406,   397,   397,    55,   314,   319,   319,    29,    46,
      49,   406,    79,   200,   202,   406,   470,   406,    79,     9,
     411,   204,   204,   230,   321,    91,   202,    79,   108,   239,
     148,    96,   405,   160,    14,   303,   200,    35,    79,   201,
     204,   202,   200,   166,   246,   217,   324,   325,   406,   287,
     288,   427,   304,    79,   399,   244,   163,   217,   202,   201,
       9,   411,   112,   113,   114,   327,   328,   287,    79,   272,
     202,   475,   427,   483,   201,   201,   202,   202,   203,   322,
     327,    35,    79,   162,   475,   203,   230,   483,    79,    14,
      79,   322,   230,   204,    35,    79,   162,    14,   406,   204,
      79,    14,    79,   406,    14,   406,   406
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
    { (yyval) = T_ABSTRACT;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval).reset();;}
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
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval).reset();;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyval).reset();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval).reset();;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval).reset();;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
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

  case 444:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
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

  case 448:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
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

  case 516:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
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

  case 517:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval).reset();;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval).reset();;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval).reset();;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval).reset();;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval).reset();;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval).reset();;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval).reset();;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { (yyval).reset();;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval)++;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval).reset();;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    {;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    {;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    {;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    {;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13482 "hphp.tab.cpp"
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
#line 3024 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

