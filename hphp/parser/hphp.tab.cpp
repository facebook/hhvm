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
#define YYLAST   16684

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  276
/* YYNRULES -- Number of rules.  */
#define YYNRULES  934
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1752

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
    1295,  1299,  1306,  1309,  1313,  1320,  1322,  1324,  1326,  1328,
    1330,  1337,  1341,  1346,  1353,  1357,  1361,  1365,  1369,  1373,
    1377,  1381,  1385,  1389,  1393,  1397,  1401,  1404,  1407,  1410,
    1413,  1417,  1421,  1425,  1429,  1433,  1437,  1441,  1445,  1449,
    1453,  1457,  1461,  1465,  1469,  1473,  1477,  1481,  1484,  1487,
    1490,  1493,  1497,  1501,  1505,  1509,  1513,  1517,  1521,  1525,
    1529,  1533,  1539,  1544,  1546,  1549,  1552,  1555,  1558,  1561,
    1564,  1567,  1570,  1573,  1575,  1577,  1579,  1581,  1583,  1587,
    1590,  1592,  1598,  1599,  1600,  1612,  1613,  1626,  1627,  1631,
    1632,  1637,  1638,  1645,  1646,  1654,  1657,  1660,  1665,  1667,
    1669,  1675,  1679,  1685,  1689,  1692,  1693,  1696,  1697,  1702,
    1707,  1711,  1716,  1721,  1726,  1731,  1736,  1741,  1746,  1751,
    1756,  1761,  1763,  1765,  1767,  1769,  1773,  1776,  1780,  1785,
    1788,  1792,  1794,  1797,  1799,  1802,  1804,  1806,  1808,  1810,
    1812,  1814,  1819,  1824,  1827,  1836,  1847,  1850,  1852,  1856,
    1858,  1861,  1863,  1865,  1867,  1869,  1872,  1877,  1881,  1885,
    1890,  1892,  1895,  1900,  1903,  1910,  1911,  1913,  1918,  1919,
    1922,  1923,  1925,  1927,  1931,  1933,  1937,  1939,  1941,  1945,
    1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,
    1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,
    1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,
    2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,
    2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,
    2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,
    2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,
    2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,
    2109,  2114,  2116,  2118,  2120,  2122,  2124,  2126,  2128,  2130,
    2133,  2135,  2136,  2137,  2139,  2141,  2145,  2146,  2148,  2150,
    2152,  2154,  2156,  2158,  2160,  2162,  2164,  2166,  2168,  2170,
    2172,  2176,  2179,  2181,  2183,  2188,  2192,  2197,  2199,  2201,
    2203,  2205,  2209,  2213,  2217,  2221,  2225,  2229,  2233,  2237,
    2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,  2277,
    2280,  2283,  2286,  2289,  2293,  2297,  2301,  2305,  2309,  2313,
    2317,  2321,  2327,  2332,  2336,  2340,  2344,  2346,  2348,  2350,
    2352,  2356,  2360,  2364,  2367,  2368,  2370,  2371,  2373,  2374,
    2380,  2384,  2388,  2390,  2392,  2394,  2396,  2398,  2402,  2405,
    2407,  2409,  2411,  2413,  2415,  2417,  2420,  2423,  2428,  2432,
    2437,  2440,  2441,  2447,  2451,  2455,  2457,  2461,  2463,  2466,
    2467,  2473,  2477,  2480,  2481,  2485,  2486,  2491,  2494,  2495,
    2499,  2503,  2505,  2506,  2508,  2510,  2512,  2516,  2518,  2520,
    2524,  2528,  2531,  2536,  2539,  2544,  2546,  2548,  2550,  2552,
    2554,  2558,  2564,  2568,  2573,  2578,  2582,  2584,  2586,  2588,
    2590,  2594,  2600,  2605,  2609,  2611,  2613,  2617,  2621,  2623,
    2625,  2633,  2643,  2651,  2658,  2667,  2669,  2672,  2677,  2682,
    2684,  2686,  2691,  2693,  2694,  2696,  2699,  2701,  2703,  2707,
    2713,  2717,  2721,  2722,  2724,  2728,  2734,  2738,  2741,  2745,
    2752,  2753,  2755,  2760,  2763,  2764,  2770,  2774,  2778,  2780,
    2787,  2792,  2797,  2800,  2803,  2804,  2810,  2814,  2818,  2820,
    2823,  2824,  2830,  2834,  2838,  2840,  2843,  2844,  2847,  2848,
    2854,  2858,  2862,  2864,  2867,  2868,  2871,  2872,  2878,  2882,
    2886,  2888,  2891,  2894,  2896,  2899,  2901,  2906,  2910,  2914,
    2921,  2925,  2927,  2929,  2931,  2936,  2941,  2946,  2951,  2956,
    2961,  2964,  2967,  2972,  2975,  2978,  2980,  2984,  2988,  2992,
    2993,  2996,  3002,  3009,  3011,  3014,  3016,  3021,  3025,  3026,
    3028,  3032,  3035,  3039,  3041,  3043,  3044,  3045,  3048,  3053,
    3056,  3063,  3068,  3070,  3072,  3073,  3077,  3083,  3087,  3089,
    3092,  3093,  3098,  3101,  3104,  3106,  3108,  3110,  3112,  3117,
    3124,  3126,  3135,  3142,  3144
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   232,    -1,   249,    -1,   256,    -1,   253,    -1,   261,
      -1,   470,    -1,   122,   200,   201,   202,    -1,   149,   224,
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
     473,    -1,   225,   473,    -1,   229,     9,   471,    14,   406,
      -1,   105,   471,    14,   406,    -1,   230,   231,    -1,    -1,
     232,    -1,   249,    -1,   256,    -1,   261,    -1,   203,   230,
     204,    -1,    70,   332,   232,   283,   285,    -1,    70,   332,
      30,   230,   284,   286,    73,   202,    -1,    -1,    88,   332,
     233,   277,    -1,    -1,    87,   234,   232,    88,   332,   202,
      -1,    -1,    90,   200,   334,   202,   334,   202,   334,   201,
     235,   275,    -1,    -1,    97,   332,   236,   280,    -1,   101,
     202,    -1,   101,   341,   202,    -1,   103,   202,    -1,   103,
     341,   202,    -1,   106,   202,    -1,   106,   341,   202,    -1,
      27,   101,   202,    -1,   111,   293,   202,    -1,   117,   295,
     202,    -1,    86,   333,   202,    -1,   119,   200,   467,   201,
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
     104,    -1,    -1,   248,   247,   472,   250,   200,   289,   201,
     477,   321,    -1,    -1,   325,   248,   247,   472,   251,   200,
     289,   201,   477,   321,    -1,    -1,   426,   324,   248,   247,
     472,   252,   200,   289,   201,   477,   321,    -1,    -1,   159,
     217,   254,    30,   484,   469,   203,   296,   204,    -1,    -1,
     426,   159,   217,   255,    30,   484,   469,   203,   296,   204,
      -1,    -1,   267,   264,   257,   268,   269,   203,   299,   204,
      -1,    -1,   426,   267,   264,   258,   268,   269,   203,   299,
     204,    -1,    -1,   124,   265,   259,   270,   203,   299,   204,
      -1,    -1,   426,   124,   265,   260,   270,   203,   299,   204,
      -1,    -1,   161,   266,   262,   269,   203,   299,   204,    -1,
      -1,   426,   161,   266,   263,   269,   203,   299,   204,    -1,
     472,    -1,   153,    -1,   472,    -1,   472,    -1,   123,    -1,
     116,   123,    -1,   116,   115,   123,    -1,   115,   116,   123,
      -1,   115,   123,    -1,   125,   399,    -1,    -1,   126,   271,
      -1,    -1,   125,   271,    -1,    -1,   399,    -1,   271,     9,
     399,    -1,   399,    -1,   272,     9,   399,    -1,   129,   274,
      -1,    -1,   434,    -1,    35,   434,    -1,   130,   200,   448,
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
     427,   327,   485,   162,    79,    -1,   288,     9,   427,   327,
     485,    35,   162,    79,    -1,   288,     9,   427,   327,   485,
     162,    -1,   288,   411,    -1,   427,   327,   485,   162,    79,
      -1,   427,   327,   485,    35,   162,    79,    -1,   427,   327,
     485,   162,    -1,    -1,   427,   327,   485,    79,    -1,   427,
     327,   485,    35,    79,    -1,   427,   327,   485,    35,    79,
      14,   406,    -1,   427,   327,   485,    79,    14,   406,    -1,
     288,     9,   427,   327,   485,    79,    -1,   288,     9,   427,
     327,   485,    35,    79,    -1,   288,     9,   427,   327,   485,
      35,    79,    14,   406,    -1,   288,     9,   427,   327,   485,
      79,    14,   406,    -1,   290,     9,   427,   485,   162,    79,
      -1,   290,     9,   427,   485,    35,   162,    79,    -1,   290,
       9,   427,   485,   162,    -1,   290,   411,    -1,   427,   485,
     162,    79,    -1,   427,   485,    35,   162,    79,    -1,   427,
     485,   162,    -1,    -1,   427,   485,    79,    -1,   427,   485,
      35,    79,    -1,   427,   485,    35,    79,    14,   406,    -1,
     427,   485,    79,    14,   406,    -1,   290,     9,   427,   485,
      79,    -1,   290,     9,   427,   485,    35,    79,    -1,   290,
       9,   427,   485,    35,    79,    14,   406,    -1,   290,     9,
     427,   485,    79,    14,   406,    -1,   292,   411,    -1,    -1,
     341,    -1,    35,   434,    -1,   162,   341,    -1,   292,     9,
     341,    -1,   292,     9,   162,   341,    -1,   292,     9,    35,
     434,    -1,   293,     9,   294,    -1,   294,    -1,    79,    -1,
     205,   434,    -1,   205,   203,   341,   204,    -1,   295,     9,
      79,    -1,   295,     9,    79,    14,   406,    -1,    79,    -1,
      79,    14,   406,    -1,   296,   297,    -1,    -1,   298,   202,
      -1,   471,    14,   406,    -1,   299,   300,    -1,    -1,    -1,
     323,   301,   329,   202,    -1,    -1,   325,   484,   302,   329,
     202,    -1,   330,   202,    -1,    -1,   324,   248,   247,   472,
     200,   303,   287,   201,   477,   322,    -1,    -1,   426,   324,
     248,   247,   472,   200,   304,   287,   201,   477,   322,    -1,
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
      79,    -1,    79,    14,   406,    -1,   330,     9,   471,    14,
     406,    -1,   105,   471,    14,   406,    -1,   200,   331,   201,
      -1,    68,   401,   404,    -1,    67,   341,    -1,   388,    -1,
     360,    -1,   200,   341,   201,    -1,   333,     9,   341,    -1,
     341,    -1,   333,    -1,    -1,    27,    -1,    27,   341,    -1,
      27,   341,   129,   341,    -1,   434,    14,   335,    -1,   130,
     200,   448,   201,    14,   335,    -1,    28,   341,    -1,   434,
      14,   338,    -1,   130,   200,   448,   201,    14,   338,    -1,
     342,    -1,   434,    -1,   331,    -1,   438,    -1,   437,    -1,
     130,   200,   448,   201,    14,   341,    -1,   434,    14,   341,
      -1,   434,    14,    35,   434,    -1,   434,    14,    35,    68,
     401,   404,    -1,   434,    26,   341,    -1,   434,    25,   341,
      -1,   434,    24,   341,    -1,   434,    23,   341,    -1,   434,
      22,   341,    -1,   434,    21,   341,    -1,   434,    20,   341,
      -1,   434,    19,   341,    -1,   434,    18,   341,    -1,   434,
      17,   341,    -1,   434,    16,   341,    -1,   434,    15,   341,
      -1,   434,    64,    -1,    64,   434,    -1,   434,    63,    -1,
      63,   434,    -1,   341,    31,   341,    -1,   341,    32,   341,
      -1,   341,    10,   341,    -1,   341,    12,   341,    -1,   341,
      11,   341,    -1,   341,    33,   341,    -1,   341,    35,   341,
      -1,   341,    34,   341,    -1,   341,    48,   341,    -1,   341,
      46,   341,    -1,   341,    47,   341,    -1,   341,    49,   341,
      -1,   341,    50,   341,    -1,   341,    65,   341,    -1,   341,
      51,   341,    -1,   341,    45,   341,    -1,   341,    44,   341,
      -1,    46,   341,    -1,    47,   341,    -1,    52,   341,    -1,
      54,   341,    -1,   341,    37,   341,    -1,   341,    36,   341,
      -1,   341,    39,   341,    -1,   341,    38,   341,    -1,   341,
      40,   341,    -1,   341,    43,   341,    -1,   341,    41,   341,
      -1,   341,    42,   341,    -1,   341,    53,   401,    -1,   200,
     342,   201,    -1,   341,    29,   341,    30,   341,    -1,   341,
      29,    30,   341,    -1,   466,    -1,    62,   341,    -1,    61,
     341,    -1,    60,   341,    -1,    59,   341,    -1,    58,   341,
      -1,    57,   341,    -1,    56,   341,    -1,    69,   402,    -1,
      55,   341,    -1,   408,    -1,   359,    -1,   358,    -1,   361,
      -1,   362,    -1,   206,   403,   206,    -1,    13,   341,    -1,
     366,    -1,   110,   200,   387,   411,   201,    -1,    -1,    -1,
     248,   247,   200,   345,   289,   201,   477,   343,   203,   230,
     204,    -1,    -1,   325,   248,   247,   200,   346,   289,   201,
     477,   343,   203,   230,   204,    -1,    -1,    79,   348,   352,
      -1,    -1,   182,    79,   349,   352,    -1,    -1,   197,   350,
     289,   198,   477,   352,    -1,    -1,   182,   197,   351,   289,
     198,   477,   352,    -1,     8,   341,    -1,     8,   338,    -1,
       8,   203,   230,   204,    -1,    85,    -1,   468,    -1,   354,
       9,   353,   129,   341,    -1,   353,   129,   341,    -1,   355,
       9,   353,   129,   406,    -1,   353,   129,   406,    -1,   354,
     410,    -1,    -1,   355,   410,    -1,    -1,   173,   200,   356,
     201,    -1,   131,   200,   449,   201,    -1,    66,   449,   207,
      -1,   399,   203,   451,   204,    -1,   175,   200,   455,   201,
      -1,   176,   200,   455,   201,    -1,   174,   200,   456,   201,
      -1,   175,   200,   459,   201,    -1,   176,   200,   459,   201,
      -1,   174,   200,   460,   201,    -1,   399,   203,   453,   204,
      -1,   366,    66,   444,   207,    -1,   367,    66,   444,   207,
      -1,   359,    -1,   468,    -1,   437,    -1,    85,    -1,   200,
     342,   201,    -1,   370,   371,    -1,   434,    14,   368,    -1,
     183,    79,   186,   341,    -1,   372,   383,    -1,   372,   383,
     386,    -1,   383,    -1,   383,   386,    -1,   373,    -1,   372,
     373,    -1,   374,    -1,   375,    -1,   376,    -1,   377,    -1,
     378,    -1,   379,    -1,   183,    79,   186,   341,    -1,   190,
      79,    14,   341,    -1,   184,   341,    -1,   185,    79,   186,
     341,   187,   341,   188,   341,    -1,   185,    79,   186,   341,
     187,   341,   188,   341,   189,    79,    -1,   191,   380,    -1,
     381,    -1,   380,     9,   381,    -1,   341,    -1,   341,   382,
      -1,   192,    -1,   193,    -1,   384,    -1,   385,    -1,   194,
     341,    -1,   195,   341,   196,   341,    -1,   189,    79,   371,
      -1,   387,     9,    79,    -1,   387,     9,    35,    79,    -1,
      79,    -1,    35,    79,    -1,   167,   153,   389,   168,    -1,
     391,    50,    -1,   391,   168,   392,   167,    50,   390,    -1,
      -1,   153,    -1,   391,   393,    14,   394,    -1,    -1,   392,
     395,    -1,    -1,   153,    -1,   154,    -1,   203,   341,   204,
      -1,   154,    -1,   203,   341,   204,    -1,   388,    -1,   397,
      -1,   396,    30,   397,    -1,   396,    47,   397,    -1,   217,
      -1,    69,    -1,   104,    -1,   105,    -1,   106,    -1,    27,
      -1,    28,    -1,   107,    -1,   108,    -1,   166,    -1,   109,
      -1,    70,    -1,    71,    -1,    73,    -1,    72,    -1,    88,
      -1,    89,    -1,    87,    -1,    90,    -1,    91,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    53,    -1,    96,
      -1,    97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,
      -1,   103,    -1,   102,    -1,    86,    -1,    13,    -1,   123,
      -1,   124,    -1,   125,    -1,   126,    -1,    68,    -1,    67,
      -1,   118,    -1,     5,    -1,     7,    -1,     6,    -1,     4,
      -1,     3,    -1,   149,    -1,   110,    -1,   111,    -1,   120,
      -1,   121,    -1,   122,    -1,   117,    -1,   116,    -1,   115,
      -1,   114,    -1,   113,    -1,   112,    -1,   182,    -1,   119,
      -1,   130,    -1,   131,    -1,    10,    -1,    12,    -1,    11,
      -1,   133,    -1,   135,    -1,   134,    -1,   136,    -1,   137,
      -1,   151,    -1,   150,    -1,   181,    -1,   161,    -1,   164,
      -1,   163,    -1,   177,    -1,   179,    -1,   173,    -1,   227,
     200,   291,   201,    -1,   228,    -1,   153,    -1,   399,    -1,
     117,    -1,   442,    -1,   399,    -1,   117,    -1,   446,    -1,
     200,   201,    -1,   332,    -1,    -1,    -1,    84,    -1,   463,
      -1,   200,   291,   201,    -1,    -1,    74,    -1,    75,    -1,
      76,    -1,    85,    -1,   136,    -1,   137,    -1,   151,    -1,
     133,    -1,   164,    -1,   134,    -1,   135,    -1,   150,    -1,
     181,    -1,   144,    84,   145,    -1,   144,   145,    -1,   405,
      -1,   226,    -1,   131,   200,   409,   201,    -1,    66,   409,
     207,    -1,   173,   200,   357,   201,    -1,   407,    -1,   365,
      -1,   363,    -1,   364,    -1,   200,   406,   201,    -1,   406,
      31,   406,    -1,   406,    32,   406,    -1,   406,    10,   406,
      -1,   406,    12,   406,    -1,   406,    11,   406,    -1,   406,
      33,   406,    -1,   406,    35,   406,    -1,   406,    34,   406,
      -1,   406,    48,   406,    -1,   406,    46,   406,    -1,   406,
      47,   406,    -1,   406,    49,   406,    -1,   406,    50,   406,
      -1,   406,    51,   406,    -1,   406,    45,   406,    -1,   406,
      44,   406,    -1,   406,    65,   406,    -1,    52,   406,    -1,
      54,   406,    -1,    46,   406,    -1,    47,   406,    -1,   406,
      37,   406,    -1,   406,    36,   406,    -1,   406,    39,   406,
      -1,   406,    38,   406,    -1,   406,    40,   406,    -1,   406,
      43,   406,    -1,   406,    41,   406,    -1,   406,    42,   406,
      -1,   406,    29,   406,    30,   406,    -1,   406,    29,    30,
     406,    -1,   228,   148,   217,    -1,   153,   148,   217,    -1,
     228,   148,   123,    -1,   226,    -1,    78,    -1,   468,    -1,
     405,    -1,   208,   463,   208,    -1,   209,   463,   209,    -1,
     144,   463,   145,    -1,   412,   410,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   412,     9,   406,   129,   406,    -1,
     412,     9,   406,    -1,   406,   129,   406,    -1,   406,    -1,
      74,    -1,    75,    -1,    76,    -1,    85,    -1,   144,    84,
     145,    -1,   144,   145,    -1,    74,    -1,    75,    -1,    76,
      -1,   217,    -1,   413,    -1,   217,    -1,    46,   414,    -1,
      47,   414,    -1,   131,   200,   416,   201,    -1,    66,   416,
     207,    -1,   173,   200,   419,   201,    -1,   417,   410,    -1,
      -1,   417,     9,   415,   129,   415,    -1,   417,     9,   415,
      -1,   415,   129,   415,    -1,   415,    -1,   418,     9,   415,
      -1,   415,    -1,   420,   410,    -1,    -1,   420,     9,   353,
     129,   415,    -1,   353,   129,   415,    -1,   418,   410,    -1,
      -1,   200,   421,   201,    -1,    -1,   423,     9,   217,   422,
      -1,   217,   422,    -1,    -1,   425,   423,   410,    -1,    45,
     424,    44,    -1,   426,    -1,    -1,   127,    -1,   128,    -1,
     217,    -1,   203,   341,   204,    -1,   429,    -1,   441,    -1,
      66,   444,   207,    -1,   203,   341,   204,    -1,   435,   431,
      -1,   200,   331,   201,   431,    -1,   447,   431,    -1,   200,
     331,   201,   431,    -1,   441,    -1,   398,    -1,   439,    -1,
     440,    -1,   432,    -1,   434,   428,   430,    -1,   200,   331,
     201,   428,   430,    -1,   400,   148,   441,    -1,   436,   200,
     291,   201,    -1,   437,   200,   291,   201,    -1,   200,   434,
     201,    -1,   398,    -1,   439,    -1,   440,    -1,   432,    -1,
     434,   428,   429,    -1,   200,   331,   201,   428,   429,    -1,
     436,   200,   291,   201,    -1,   200,   434,   201,    -1,   441,
      -1,   432,    -1,   200,   434,   201,    -1,   200,   438,   201,
      -1,   344,    -1,   347,    -1,   434,   428,   430,   473,   200,
     291,   201,    -1,   200,   331,   201,   428,   430,   473,   200,
     291,   201,    -1,   400,   148,   217,   473,   200,   291,   201,
      -1,   400,   148,   441,   200,   291,   201,    -1,   400,   148,
     203,   341,   204,   200,   291,   201,    -1,   442,    -1,   445,
     442,    -1,   442,    66,   444,   207,    -1,   442,   203,   341,
     204,    -1,   443,    -1,    79,    -1,   205,   203,   341,   204,
      -1,   341,    -1,    -1,   205,    -1,   445,   205,    -1,   441,
      -1,   433,    -1,   446,   428,   430,    -1,   200,   331,   201,
     428,   430,    -1,   400,   148,   441,    -1,   200,   434,   201,
      -1,    -1,   433,    -1,   446,   428,   429,    -1,   200,   331,
     201,   428,   429,    -1,   200,   434,   201,    -1,   448,     9,
      -1,   448,     9,   434,    -1,   448,     9,   130,   200,   448,
     201,    -1,    -1,   434,    -1,   130,   200,   448,   201,    -1,
     450,   410,    -1,    -1,   450,     9,   341,   129,   341,    -1,
     450,     9,   341,    -1,   341,   129,   341,    -1,   341,    -1,
     450,     9,   341,   129,    35,   434,    -1,   450,     9,    35,
     434,    -1,   341,   129,    35,   434,    -1,    35,   434,    -1,
     452,   410,    -1,    -1,   452,     9,   341,   129,   341,    -1,
     452,     9,   341,    -1,   341,   129,   341,    -1,   341,    -1,
     454,   410,    -1,    -1,   454,     9,   406,   129,   406,    -1,
     454,     9,   406,    -1,   406,   129,   406,    -1,   406,    -1,
     457,   410,    -1,    -1,   458,   410,    -1,    -1,   457,     9,
     341,   129,   341,    -1,   341,   129,   341,    -1,   458,     9,
     341,    -1,   341,    -1,   461,   410,    -1,    -1,   462,   410,
      -1,    -1,   461,     9,   406,   129,   406,    -1,   406,   129,
     406,    -1,   462,     9,   406,    -1,   406,    -1,   463,   464,
      -1,   463,    84,    -1,   464,    -1,    84,   464,    -1,    79,
      -1,    79,    66,   465,   207,    -1,    79,   428,   217,    -1,
     146,   341,   204,    -1,   146,    78,    66,   341,   207,   204,
      -1,   147,   434,   204,    -1,   217,    -1,    80,    -1,    79,
      -1,   120,   200,   467,   201,    -1,   121,   200,   434,   201,
      -1,   121,   200,   342,   201,    -1,   121,   200,   438,   201,
      -1,   121,   200,   437,   201,    -1,   121,   200,   331,   201,
      -1,     7,   341,    -1,     6,   341,    -1,     5,   200,   341,
     201,    -1,     4,   341,    -1,     3,   341,    -1,   434,    -1,
     467,     9,   434,    -1,   400,   148,   217,    -1,   400,   148,
     123,    -1,    -1,    96,   484,    -1,   177,   472,    14,   484,
     202,    -1,   179,   472,   469,    14,   484,   202,    -1,   217,
      -1,   484,   217,    -1,   217,    -1,   217,   169,   478,   170,
      -1,   169,   475,   170,    -1,    -1,   484,    -1,   474,     9,
     484,    -1,   474,   410,    -1,   474,     9,   162,    -1,   475,
      -1,   162,    -1,    -1,    -1,    30,   484,    -1,   478,     9,
     479,   217,    -1,   479,   217,    -1,   478,     9,   479,   217,
      96,   484,    -1,   479,   217,    96,   484,    -1,    46,    -1,
      47,    -1,    -1,    85,   129,   484,    -1,   228,   148,   217,
     129,   484,    -1,   481,     9,   480,    -1,   480,    -1,   481,
     410,    -1,    -1,   173,   200,   482,   201,    -1,    29,   484,
      -1,    55,   484,    -1,   228,    -1,   131,    -1,   132,    -1,
     483,    -1,   131,   169,   484,   170,    -1,   131,   169,   484,
       9,   484,   170,    -1,   153,    -1,   200,   104,   200,   476,
     201,    30,   484,   201,    -1,   200,   484,     9,   474,   410,
     201,    -1,   484,    -1,    -1
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
    1741,  1745,  1750,  1754,  1758,  1763,  1764,  1765,  1766,  1767,
    1771,  1773,  1774,  1775,  1778,  1779,  1780,  1781,  1782,  1783,
    1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,  1793,
    1794,  1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,
    1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1819,  1820,  1822,  1823,  1825,
    1827,  1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,
    1837,  1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,
    1847,  1851,  1855,  1860,  1859,  1874,  1872,  1889,  1889,  1905,
    1904,  1922,  1922,  1938,  1937,  1958,  1959,  1960,  1965,  1967,
    1971,  1975,  1981,  1985,  1991,  1993,  1997,  1999,  2003,  2007,
    2008,  2012,  2019,  2020,  2024,  2028,  2030,  2035,  2040,  2047,
    2049,  2054,  2055,  2056,  2057,  2059,  2063,  2067,  2071,  2075,
    2077,  2079,  2081,  2086,  2087,  2092,  2093,  2094,  2095,  2096,
    2097,  2101,  2105,  2109,  2113,  2118,  2123,  2127,  2128,  2132,
    2133,  2137,  2138,  2142,  2143,  2147,  2151,  2155,  2159,  2160,
    2161,  2162,  2166,  2172,  2181,  2194,  2195,  2198,  2201,  2204,
    2205,  2208,  2212,  2215,  2218,  2225,  2226,  2230,  2231,  2233,
    2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,  2246,
    2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,  2256,
    2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,  2316,
    2320,  2325,  2326,  2329,  2330,  2331,  2335,  2336,  2337,  2341,
    2342,  2343,  2347,  2348,  2349,  2352,  2354,  2358,  2359,  2360,
    2361,  2363,  2364,  2365,  2366,  2367,  2368,  2369,  2370,  2371,
    2372,  2375,  2380,  2381,  2382,  2384,  2385,  2387,  2388,  2389,
    2390,  2391,  2392,  2394,  2396,  2398,  2400,  2402,  2403,  2404,
    2405,  2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,
    2415,  2416,  2417,  2418,  2420,  2422,  2424,  2426,  2427,  2430,
    2431,  2435,  2437,  2441,  2444,  2447,  2453,  2454,  2455,  2456,
    2457,  2458,  2459,  2464,  2466,  2470,  2471,  2474,  2475,  2479,
    2482,  2484,  2486,  2490,  2491,  2492,  2493,  2495,  2498,  2502,
    2503,  2504,  2505,  2508,  2509,  2510,  2511,  2512,  2514,  2515,
    2520,  2522,  2525,  2528,  2530,  2532,  2535,  2537,  2541,  2543,
    2546,  2549,  2555,  2557,  2560,  2561,  2566,  2569,  2573,  2573,
    2578,  2581,  2582,  2586,  2587,  2591,  2592,  2596,  2597,  2601,
    2602,  2606,  2607,  2612,  2614,  2619,  2620,  2621,  2622,  2623,
    2624,  2626,  2629,  2632,  2634,  2636,  2640,  2641,  2642,  2643,
    2644,  2646,  2649,  2651,  2655,  2656,  2657,  2661,  2665,  2666,
    2670,  2673,  2680,  2684,  2688,  2695,  2696,  2701,  2703,  2704,
    2707,  2708,  2711,  2712,  2716,  2717,  2721,  2722,  2723,  2726,
    2729,  2732,  2735,  2736,  2737,  2739,  2742,  2746,  2747,  2748,
    2750,  2751,  2752,  2756,  2758,  2761,  2763,  2764,  2765,  2766,
    2769,  2771,  2772,  2776,  2778,  2781,  2783,  2784,  2785,  2789,
    2791,  2794,  2797,  2799,  2801,  2805,  2807,  2810,  2812,  2815,
    2817,  2820,  2821,  2825,  2827,  2830,  2832,  2835,  2838,  2842,
    2844,  2848,  2849,  2851,  2852,  2858,  2859,  2861,  2863,  2865,
    2867,  2870,  2871,  2872,  2876,  2877,  2878,  2879,  2880,  2881,
    2882,  2883,  2884,  2885,  2886,  2890,  2891,  2895,  2897,  2905,
    2907,  2911,  2915,  2922,  2923,  2929,  2930,  2937,  2940,  2944,
    2947,  2952,  2957,  2959,  2960,  2961,  2965,  2966,  2970,  2972,
    2973,  2976,  2981,  2982,  2983,  2987,  2990,  2999,  3001,  3005,
    3008,  3011,  3019,  3022,  3025,  3026,  3029,  3032,  3033,  3036,
    3040,  3044,  3050,  3060,  3061
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
     336,   337,   338,   339,   340,   341,   341,   341,   341,   341,
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
     366,   367,   367,   367,   367,   367,   368,   369,   370,   371,
     371,   371,   371,   372,   372,   373,   373,   373,   373,   373,
     373,   374,   375,   376,   377,   378,   379,   380,   380,   381,
     381,   382,   382,   383,   383,   384,   385,   386,   387,   387,
     387,   387,   388,   389,   389,   390,   390,   391,   391,   392,
     392,   393,   394,   394,   395,   395,   395,   396,   396,   396,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     398,   399,   399,   400,   400,   400,   401,   401,   401,   402,
     402,   402,   403,   403,   403,   404,   404,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   407,   407,   407,   408,   408,   408,   408,
     408,   408,   408,   409,   409,   410,   410,   411,   411,   412,
     412,   412,   412,   413,   413,   413,   413,   413,   413,   414,
     414,   414,   414,   415,   415,   415,   415,   415,   415,   415,
     416,   416,   417,   417,   417,   417,   418,   418,   419,   419,
     420,   420,   421,   421,   422,   422,   423,   423,   425,   424,
     426,   427,   427,   428,   428,   429,   429,   430,   430,   431,
     431,   432,   432,   433,   433,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   435,   435,   435,   435,
     435,   435,   435,   435,   436,   436,   436,   437,   438,   438,
     439,   439,   440,   440,   440,   441,   441,   442,   442,   442,
     443,   443,   444,   444,   445,   445,   446,   446,   446,   446,
     446,   446,   447,   447,   447,   447,   447,   448,   448,   448,
     448,   448,   448,   449,   449,   450,   450,   450,   450,   450,
     450,   450,   450,   451,   451,   452,   452,   452,   452,   453,
     453,   454,   454,   454,   454,   455,   455,   456,   456,   457,
     457,   458,   458,   459,   459,   460,   460,   461,   461,   462,
     462,   463,   463,   463,   463,   464,   464,   464,   464,   464,
     464,   465,   465,   465,   466,   466,   466,   466,   466,   466,
     466,   466,   466,   466,   466,   467,   467,   468,   468,   469,
     469,   470,   470,   471,   471,   472,   472,   473,   473,   474,
     474,   475,   476,   476,   476,   476,   477,   477,   478,   478,
     478,   478,   479,   479,   479,   480,   480,   481,   481,   482,
     482,   483,   484,   484,   484,   484,   484,   484,   484,   484,
     484,   484,   484,   485,   485
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
       3,     6,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    11,     0,    12,     0,     3,     0,
       4,     0,     6,     0,     7,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     1,     1,     1,     1,     3,     2,     3,     4,     2,
       3,     1,     2,     1,     2,     1,     1,     1,     1,     1,
       1,     4,     4,     2,     8,    10,     2,     1,     3,     1,
       2,     1,     1,     1,     1,     2,     4,     3,     3,     4,
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     4,     3,     4,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     3,     4,
       2,     0,     5,     3,     3,     1,     3,     1,     2,     0,
       5,     3,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       3,     5,     3,     4,     4,     3,     1,     1,     1,     1,
       3,     5,     4,     3,     1,     1,     3,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     3,     5,
       3,     3,     0,     1,     3,     5,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     0,     2,     0,     5,
       3,     3,     1,     2,     0,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     3,     0,
       2,     5,     6,     1,     2,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     2,     4,     2,
       6,     4,     1,     1,     0,     3,     5,     3,     1,     2,
       0,     4,     2,     2,     1,     1,     1,     1,     4,     6,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   357,     0,   748,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   824,     0,
     812,   631,     0,   637,   638,   639,    22,   697,   800,    98,
     640,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   329,   330,
     331,   334,   333,   332,     0,     0,     0,     0,   158,     0,
       0,     0,   644,   646,   647,   641,   642,     0,     0,   648,
     643,     0,   622,    23,    24,    25,    27,    26,     0,   645,
       0,     0,     0,     0,     0,     0,     0,   649,   335,    28,
      29,    31,    30,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   451,     0,    97,    70,   804,   632,     0,     0,
       4,    59,    61,    64,   696,     0,   621,     0,     6,   130,
       7,     9,     8,    10,     0,     0,   327,   367,     0,     0,
       0,     0,     0,     0,     0,   365,   788,   789,   435,   434,
     351,   436,   437,   440,     0,     0,   350,   766,   623,     0,
     699,   433,   326,   769,   366,     0,     0,   369,   368,   767,
     768,   765,   795,   799,     0,   423,   698,    11,   334,   333,
     332,     0,     0,    27,    59,   130,     0,   884,   366,   883,
       0,   881,   880,   439,     0,   358,   362,     0,     0,   407,
     408,   409,   410,   432,   430,   429,   428,   427,   426,   425,
     424,   800,   624,     0,   898,   623,     0,   389,     0,   387,
       0,   828,     0,   706,   349,   627,     0,   898,   626,     0,
     636,   807,   806,   628,     0,     0,   630,   431,     0,     0,
       0,     0,   354,     0,    78,   356,     0,     0,    84,    86,
       0,     0,    88,     0,     0,     0,   925,   926,   930,     0,
       0,    59,   924,     0,   927,     0,     0,    90,     0,     0,
       0,     0,   121,     0,     0,     0,     0,     0,     0,    42,
      47,   247,     0,     0,   246,     0,   162,     0,   159,   252,
       0,     0,     0,     0,     0,   895,   146,   156,   820,   824,
     865,     0,   651,     0,     0,     0,   863,     0,    16,     0,
      63,   138,   150,   157,   528,   465,   848,   846,   846,     0,
     889,   449,   453,   752,   367,     0,   365,   366,   368,     0,
       0,   633,     0,   634,     0,     0,     0,   120,     0,     0,
      66,   238,     0,    21,   129,     0,   155,   142,   154,   332,
     335,   130,   328,   111,   112,   113,   114,   115,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   812,     0,   110,   803,   803,   118,
     834,     0,     0,     0,     0,     0,     0,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   388,   386,   753,   754,     0,   803,     0,   761,   238,
     238,   803,     0,   805,   796,   820,     0,   130,     0,     0,
      92,     0,   750,   745,   706,     0,     0,     0,     0,     0,
     832,     0,   470,   705,   823,     0,     0,    66,     0,   238,
     348,     0,   763,   629,     0,    70,   198,     0,   448,     0,
      95,     0,     0,   355,     0,     0,     0,     0,     0,    87,
     109,    89,   922,   923,     0,   920,     0,     0,     0,   894,
       0,   116,    91,   119,     0,     0,     0,     0,     0,     0,
       0,   486,     0,   493,   495,   496,   497,   498,   499,   500,
     491,   513,   514,    70,     0,   106,   108,     0,     0,    44,
      51,     0,     0,    46,    55,    48,     0,    18,     0,     0,
     248,     0,    93,   161,   160,     0,     0,    94,   885,     0,
       0,   367,   365,   366,   369,   368,     0,   914,   168,     0,
     821,     0,     0,     0,     0,   650,   864,   697,     0,     0,
     862,   702,   861,    62,     5,    13,    14,     0,   166,     0,
       0,   458,     0,     0,   706,     0,     0,   625,   459,   852,
       0,   706,     0,     0,   706,     0,     0,     0,     0,     0,
     752,     0,   708,   751,   934,   347,   420,   775,   787,    75,
      69,    71,    72,    73,    74,   326,     0,   438,   700,   701,
      60,   706,     0,   899,     0,     0,     0,   708,   239,     0,
     443,   132,   164,     0,   392,   394,   393,     0,     0,   390,
     391,   395,   397,   396,   412,   411,   414,   413,   415,   417,
     418,   416,   406,   405,   399,   400,   398,   401,   402,   404,
     419,   403,   802,     0,     0,   838,     0,   706,   888,     0,
     887,   772,   795,   148,   140,   152,   144,   130,   357,     0,
     360,   363,   371,   487,   385,   384,   383,   382,   381,   380,
     379,   378,   377,   376,   375,   374,     0,   755,   757,   770,
     758,     0,     0,     0,     0,     0,     0,     0,     0,   882,
     359,   743,   747,   705,   749,     0,     0,   898,     0,   827,
       0,   826,     0,   811,   810,     0,   757,   808,   352,   200,
     202,    70,   456,   455,   353,     0,    70,   182,    79,   356,
       0,     0,     0,     0,     0,   194,   194,    85,     0,     0,
       0,   918,   706,     0,   905,     0,     0,     0,     0,     0,
     704,   640,     0,     0,   622,     0,     0,     0,     0,     0,
      64,   653,   621,   659,   660,   658,     0,   652,    68,   657,
       0,     0,   503,     0,     0,   509,   506,   507,   515,     0,
     494,   489,     0,   492,     0,     0,     0,    52,    19,     0,
       0,    56,    20,     0,     0,     0,    41,    49,     0,   245,
     253,   250,     0,     0,   874,   879,   876,   875,   878,   877,
      12,   912,   913,     0,     0,     0,     0,   820,   817,     0,
     469,   873,   872,   871,     0,   867,     0,   868,   870,     0,
       5,     0,     0,     0,   522,   523,   531,   530,     0,     0,
     705,   464,   468,     0,   474,   705,   847,     0,   472,   705,
     845,   473,     0,   890,     0,   450,     0,   906,   752,   224,
     933,     0,     0,   762,   801,   705,   901,   897,   240,   241,
     620,   707,   237,     0,   752,     0,     0,   166,   445,   134,
     422,     0,   479,   480,     0,   471,   705,   833,     0,     0,
     238,   168,     0,   166,   164,     0,   812,   372,     0,     0,
     759,   760,   773,   774,   797,   798,     0,     0,     0,   731,
     713,   714,   715,   716,     0,     0,     0,   724,   723,   737,
     706,     0,   745,   831,   830,     0,     0,   764,   635,   204,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
     174,   175,   186,     0,    70,   184,   103,   194,     0,   194,
       0,     0,   928,     0,     0,   705,   919,   921,   904,   706,
     903,     0,   706,   681,   682,   679,   680,   712,     0,   706,
     704,     0,     0,   467,   856,   854,   854,     0,     0,   840,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   488,     0,     0,     0,
     511,   512,   510,     0,     0,   490,     0,   122,     0,   125,
     107,     0,    43,    53,     0,    45,    57,    50,   249,     0,
     886,    96,   914,   896,   909,   167,   169,   259,     0,     0,
     818,     0,   866,     0,    17,     0,   889,   165,   259,     0,
       0,   461,     0,   887,   851,   850,     0,   891,     0,   906,
       0,     0,   934,     0,   229,   227,   757,   771,   900,     0,
       0,   242,    67,     0,   752,   163,     0,   752,     0,   421,
     837,   836,     0,   238,     0,     0,     0,     0,   166,   136,
     636,   756,   238,     0,   719,   720,   721,   722,   725,   726,
     735,     0,   706,   731,     0,   718,   739,   705,   742,   744,
     746,     0,   825,   757,   809,     0,     0,     0,     0,   201,
     457,    81,     0,   356,   174,   176,   820,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   188,     0,   915,     0,
     917,   705,     0,     0,     0,   655,   705,   703,     0,   694,
       0,   706,     0,   860,     0,   706,     0,     0,   706,     0,
     661,   695,   693,   844,     0,   706,   664,   666,   665,     0,
       0,   662,   663,   667,   669,   668,   684,   683,   686,   685,
     687,   689,   690,   688,   677,   676,   671,   672,   670,   673,
     674,   675,   678,   501,     0,   502,   508,   516,   517,     0,
      70,    54,    58,   251,     0,     0,     0,   326,   822,   820,
     361,   364,   370,     0,    15,     0,   326,   534,     0,     0,
     536,   529,   532,     0,   527,     0,     0,   892,     0,   907,
     452,     0,   230,     0,     0,   225,     0,   244,   243,   906,
       0,   259,     0,   752,     0,   238,     0,   793,   259,   889,
     259,     0,     0,   373,     0,     0,   728,   705,   730,     0,
     717,     0,     0,   706,   736,   829,     0,    70,     0,   197,
     183,     0,     0,     0,   173,    99,   187,     0,     0,   190,
       0,   195,   196,    70,   189,   929,     0,   902,     0,   932,
     711,   710,   654,     0,   705,   466,   656,   477,   705,   855,
       0,   475,   705,   853,   476,     0,   478,   705,   839,   692,
       0,     0,     0,     0,   908,   911,   170,     0,     0,     0,
     324,     0,     0,     0,   147,   258,   260,     0,   323,     0,
     326,     0,   869,   255,   151,   525,     0,     0,   460,   849,
     454,     0,   233,   223,     0,   226,   232,   238,   442,   906,
     326,   906,     0,   835,     0,   792,   326,     0,   326,   259,
     752,   790,   734,   733,   727,     0,   729,   705,   738,    70,
     203,    77,    82,   101,   177,     0,   185,   191,    70,   193,
     916,     0,     0,   463,     0,   859,   858,     0,   843,   842,
     691,     0,    70,   126,     0,     0,     0,     0,     0,   171,
     290,   288,   292,   622,    27,     0,   284,     0,   289,   301,
       0,   299,   304,     0,   303,     0,   302,     0,   130,   262,
       0,   264,     0,   819,     0,   526,   524,   535,   533,   234,
       0,     0,   221,   231,     0,     0,     0,     0,   143,   442,
     906,   794,   149,   255,   153,   326,     0,     0,   741,     0,
     199,     0,     0,    70,   180,   100,   192,   931,   709,     0,
       0,     0,     0,     0,   910,     0,     0,     0,     0,   274,
     278,     0,     0,   269,   586,   585,   582,   584,   583,   603,
     605,   604,   574,   545,   546,   564,   580,   579,   541,   551,
     552,   554,   553,   573,   557,   555,   556,   558,   559,   560,
     561,   562,   563,   565,   566,   567,   568,   569,   570,   572,
     571,   542,   543,   544,   547,   548,   550,   588,   589,   598,
     597,   596,   595,   594,   593,   581,   600,   590,   591,   592,
     575,   576,   577,   578,   601,   602,   606,   608,   607,   609,
     610,   587,   612,   611,   614,   616,   615,   549,   619,   617,
     618,   613,   599,   540,   296,   537,     0,   270,   317,   318,
     316,   309,     0,   310,   271,   343,     0,     0,     0,     0,
     130,   139,   254,     0,     0,     0,   222,   236,   791,     0,
      70,   319,    70,   133,     0,     0,     0,   145,   906,   732,
       0,    70,   178,    83,   102,     0,   462,   857,   841,   504,
     124,   272,   273,   346,   172,     0,     0,   293,   285,     0,
       0,     0,   298,   300,     0,     0,   305,   312,   313,   311,
       0,     0,   261,     0,     0,     0,     0,   256,     0,   235,
       0,   520,   708,     0,     0,    70,   135,   141,     0,   740,
       0,     0,     0,   104,   275,    59,     0,   276,   277,     0,
       0,   291,   295,   538,   539,     0,   286,   314,   315,   307,
     308,   306,   344,   341,   265,   263,   345,     0,   257,   521,
     707,     0,   444,   320,     0,   137,     0,   181,   505,     0,
     128,     0,   326,   294,   297,     0,   752,   267,     0,   518,
     441,   446,   179,     0,     0,   105,   282,     0,   325,   342,
       0,   708,   337,   752,   519,     0,   127,     0,     0,   281,
     906,   752,   208,   338,   339,   340,   934,   336,     0,     0,
       0,   280,     0,   337,     0,   906,     0,   279,   321,    70,
     266,   934,     0,   213,   211,     0,    70,     0,     0,   214,
       0,     0,   209,   268,     0,   322,     0,   217,   207,     0,
     210,   216,   123,   218,     0,     0,   205,   215,     0,   206,
     220,   219
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   820,   554,   184,   278,   508,
     512,   279,   509,   513,   122,   123,   124,   125,   126,   127,
     329,   590,   591,   462,   243,  1441,   468,  1365,  1442,  1670,
     776,   273,   503,  1633,  1010,  1190,  1685,   345,   185,   592,
     865,  1068,  1242,   131,   557,   882,   593,   612,   884,   538,
     881,   594,   558,   883,   347,   296,   312,   134,   867,   823,
     806,  1025,  1388,  1118,   930,  1583,  1445,   718,   936,   467,
     727,   938,  1273,   710,   919,   922,  1107,  1690,  1691,   581,
     582,   606,   607,   283,   284,   290,  1414,  1562,  1563,  1197,
    1315,  1407,  1558,  1676,  1693,  1595,  1637,  1638,  1639,  1395,
    1396,  1397,  1596,  1602,  1646,  1400,  1401,  1405,  1551,  1552,
    1553,  1573,  1720,  1316,  1317,   186,   136,  1706,  1707,  1556,
    1319,   137,   236,   463,   464,   138,   139,   140,   141,   142,
     143,   144,   145,  1426,   146,   864,  1067,   147,   240,   579,
     323,   580,   458,   563,   564,  1141,   565,  1142,   148,   149,
     150,   151,   152,   753,   754,   755,   153,   154,   270,   155,
     271,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     766,   767,  1002,   500,   501,   502,   773,  1622,   156,   559,
    1416,   560,  1039,   828,  1214,  1211,  1544,  1545,   157,   158,
     159,   230,   237,   332,   450,   160,   957,   759,   161,   958,
     856,   849,   959,   908,  1088,  1090,  1091,  1092,   910,  1252,
    1253,   911,   692,   434,   197,   198,   595,   584,   415,   678,
     679,   853,   163,   231,   188,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   643,   174,   233,   234,   541,   222,
     223,   646,   647,  1154,  1155,   573,   570,   574,   571,  1147,
    1144,  1148,  1145,   305,   306,   814,   175,   529,   176,   578,
     177,  1564,   297,   340,   601,   602,   951,  1051,   803,   804,
     731,   732,   733,   264,   265,   851
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1470
static const yytype_int16 yypact[] =
{
   -1470,   154, -1470, -1470,  6373, 13822, 13822,   -83, 13822, 13822,
   13822, 11920, 13822, -1470, 13822, 13822, 13822, 13822, 13822, 13822,
   13822, 13822, 13822, 13822, 13822, 13822, 15088, 15088, 12127, 13822,
   15154,   -42,   -36, -1470, -1470, -1470, -1470, -1470,   159, -1470,
     151, 13822, -1470,   -36,   157,   164,   167,   -36, 12292, 16213,
   12457, -1470, 14292, 10927,   -20, 13822, 15935,   142, -1470, -1470,
   -1470,    24,   357,    40,   175,   192,   194,   205, -1470, 16213,
     214,   217, -1470, -1470, -1470, -1470, -1470,   297,  3447, -1470,
   -1470, 16213, -1470, -1470, -1470, -1470, 16213, -1470, 16213, -1470,
     266,   229,   246,   248,   257, 16213, 16213, -1470,    44, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, 13822, -1470, -1470,   256,   376,   419,   419,
   -1470,   438,   321,   283, -1470,   271, -1470,    63, -1470,   453,
   -1470, -1470, -1470, -1470, 14853,   463, -1470, -1470,   306,   308,
     311,   333,   356,   358,  3050, -1470, -1470, -1470, -1470,   409,
   -1470, -1470, -1470,   507,   516,   389, -1470,    34,   390,   334,
   -1470, -1470,   635,    33,  1259,    99,   395,    32, -1470,   118,
     119,   397,   129, -1470,   144, -1470,   538, -1470, -1470, -1470,
     469,   422,   467, -1470, -1470,   453,   463, 16470,  2276, 16470,
   13822, 16470, 16470, 11117,   442, 15341, 11117,   603, 16213,   594,
     594,   110,   594,   594,   594,   594,   594,   594,   594,   594,
     594, -1470, -1470, 14728,   491, -1470,   521,    23,   481,    23,
   15088, 15385,   476,   676, -1470,   469, 14782,   491,   540,   542,
     486,   126, -1470,    23,    99, 12622, -1470, -1470, 13822,  9685,
     685,    72, 16470, 10720, -1470, 13822, 13822, 16213, -1470, -1470,
    3115,   493, -1470,  3287, 14292, 14292,   528, -1470, -1470,   500,
    2319,   684, -1470,   687, -1470, 16213,   624, -1470,   505,  3651,
     509,   652, -1470,     0,  3754, 16022, 16071, 16213,    76, -1470,
      59, -1470, 14866,    77, -1470,   581, -1470,   586, -1470,   702,
      78, 15088, 15088, 13822,   517,   548, -1470, -1470, 14950, 12127,
     280,   238, -1470, 13987, 15088,   319, -1470, 16213, -1470,   260,
     321, -1470, -1470, -1470, -1470, 14518, 13822, 13822, 13822,   716,
     636, -1470, -1470,   111,   530, 16470,   532,   612,   533,  6580,
   13822,   275,   531,   502,   275,   277,   269, -1470, 16213, 14292,
     541, 11134, 14292, -1470, -1470,  4620, -1470, -1470, -1470, -1470,
   -1470,   453, -1470, -1470, -1470, -1470, -1470, -1470, -1470, 13822,
   13822, 13822, 12829, 13822, 13822, 13822, 13822, 13822, 13822, 13822,
   13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822,
   13822, 13822, 13822, 13822, 15154, 13822, -1470, 13822, 13822, -1470,
   13822,  3653, 16213, 16213, 16213, 14853,   638,   439,  5576, 13822,
   13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822, 13822,
   13822, -1470, -1470, -1470, -1470,  1999, 13822, 13822, -1470, 11134,
   11134, 13822, 13822,   256,   127, 14950,   544,   453, 13036,  3892,
   -1470, 13822, -1470,   546,   745, 14728,   554,    15,   533,  5304,
      23, 13243, -1470, 13450, -1470,   555,    17, -1470,   145, 11134,
   -1470,  1999, -1470, -1470,  3952, -1470, -1470, 11341, -1470, 13822,
   -1470,   669,  9892,   751,   559, 16382,   749,    54,    48, -1470,
   -1470, -1470, -1470, -1470, 14292, 15867,   564,   756, 14584, -1470,
     582, -1470, -1470, -1470,   690, 13822,   692,   693, 13822, 13822,
   13822, -1470,   652, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
     585, -1470, -1470, -1470,   575, -1470, -1470, 16213,   577,   768,
     249, 16213,   580,   774,   273,   274, 16084, -1470, 16213, 13822,
      23,   142, -1470, -1470, -1470, 14584,   705, -1470,    23,    68,
      74,   584,   588,  1201,     7,   589,   591,    85,   661,   597,
      23,    96,   598, 16009, 16213, -1470, -1470,   735,  1724,    10,
   -1470, -1470, -1470,   321, -1470, -1470, -1470,   772,   677,   639,
     163, -1470,   256,   675,   799,   608,   663,   127, -1470, 16470,
     611,   805, 15442,   615,   810,   625, 14292, 14292,   811,   685,
     111,   630,   823, -1470, 14292,    82,   779,   123, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470,   853,  1801, -1470, -1470, -1470,
   -1470,   829,   670, -1470, 15088, 13822,   648,   844, 16470,   841,
   -1470, -1470,   731, 14930,  2965, 12110, 11117, 13822, 16426, 13640,
    3200, 14357, 16619,  4041, 12807, 12807, 12807, 12807,   940,   940,
     940,   940,   836,   836,   590,   590,   590,   110,   110,   110,
   -1470,   594, 16470,   651,   653, 15486,   655,   852, -1470, 13822,
     -47,   664,   127, -1470, -1470, -1470, -1470,   453, 13822,  5738,
   -1470, -1470, 11117, -1470, 11117, 11117, 11117, 11117, 11117, 11117,
   11117, 11117, 11117, 11117, 11117, 11117, 13822, -1470,   132,   -47,
   -1470,   656,  1944,   665,   666,   658,  2069,    97,   671, -1470,
   16470, 14679, -1470, 16213, -1470,   530,    82,   491, 15088, 16470,
   15088, 15543,    82,   133, -1470,   672,   134, -1470, -1470,  9478,
     424, -1470, -1470, 16470, 16470,   -36, -1470, -1470, -1470, 13822,
     776, 14432, 14584, 16213, 10099,   668,   678, -1470,    45,   748,
     726, -1470,   870,   689,  5142, 14292, 14584, 14584, 14584, 14584,
   14584, -1470,   688,    27,   743,   697,   698,   700,   703, 14584,
     224, -1470,   744, -1470, -1470, -1470,   699, -1470, 16556, -1470,
   13822,   718, 16470,   727,   900,  4720,   906, -1470, 16470,  4522,
   -1470,   585,   837, -1470,  6787, 15951,   715,   278, -1470, 16022,
   16213,   281, -1470, 16071, 16213, 16213, -1470, -1470,  2212, -1470,
   16556,   909, 15088,   728, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470,    58, 16213, 15951,   717, 14950, 15034,   914,
   -1470, -1470, -1470, -1470,   724, -1470, 13822, -1470, -1470,  5959,
   -1470, 14292, 15951,   729, -1470, -1470, -1470, -1470,   920, 13822,
   14518, -1470, -1470, 16158, -1470, 13822, -1470, 13822, -1470, 13822,
   -1470, -1470,   736, -1470, 14292, -1470,   746,   913,   112, -1470,
   -1470,   276,  1999, -1470, -1470, 14292, -1470, -1470,    23, 16470,
   -1470, 11548, -1470, 14584,    29,   747, 15951,   677, -1470, -1470,
   13433, 13822, -1470, -1470, 13822, -1470, 13822, -1470,  2277,   750,
   11134,   661,   918,   677,   731, 16213, 15154,    23,  2512,   752,
   -1470, -1470,   136, -1470, -1470, -1470,   937,  1843,  1843, 14679,
   -1470, -1470, -1470, -1470,   753,    41,   754, -1470, -1470, -1470,
     946,   755,   546,    23,    23, 13657,  1999, -1470, -1470,   470,
     -36, 10720, -1470,  6994,   758,  7201,   759, 14432, 15088,   757,
     833,    23, 16556,   949, -1470, -1470, -1470, -1470,   447, -1470,
      21, 14292, -1470, 14292, 16213, 15867, -1470, -1470, -1470,   962,
   -1470,   794,   829,   519,   519,   908,   908, 15688,   789,   966,
   14584,   854, 16213, 14518, 14584, 14584, 14584,  4384, 16171, 14584,
   14584, 14584, 14584, 14383, 14584, 14584, 14584, 14584, 14584, 14584,
   14584, 14584, 14584, 14584, 14584, 14584, 14584, 14584, 14584, 14584,
   14584, 14584, 14584, 14584, 14584, 14584, 16470, 13822, 13822, 13822,
   -1470, -1470, -1470, 13822, 13822, -1470,   652, -1470,   919, -1470,
   -1470, 16213, -1470, -1470, 16213, -1470, -1470, -1470, -1470, 14584,
      23, -1470,    85, -1470,   904,   992, -1470, -1470,    98,   803,
      23, 11755, -1470,  1662, -1470,  6166,   636,   992, -1470,   -41,
     -30, 16470,   875, -1470, 16470, 16470, 15587, -1470,   804,   913,
   14292,   685, 14292,    25,   994,   930,   137,   -47, -1470, 15088,
   13822, 16470, 16556,   812,    29, -1470,   808,    29,   815, 13433,
   16470, 15644,   816, 11134,   817,   809, 14292,   814,   677, -1470,
     486, -1470, 11134, 13822, -1470, -1470, -1470, -1470, -1470, -1470,
     890,   818,  1013, 14679,   878, -1470, 14518, 14679, -1470, -1470,
   -1470, 15088, 16470,   139, -1470,   -36,   996,   951, 10720, -1470,
   -1470, -1470,   825, 13822,   833,    23, 14950, 14432,   827, 14584,
    7408,   567,   828, 13822,    36,   252, -1470,   862, -1470,   907,
   -1470,  5719,  1003,   840, 14584, -1470, 14584, -1470,   843, -1470,
     916,  1028,   847, 16556,   849,  1029, 15745,   855,  1033,   856,
   -1470, -1470, -1470, 15787,   850,  1044, 11531, 13019, 13226, 14584,
   16514, 14191,  2590, 16588, 16260,  3317, 14457, 14457, 14457, 14457,
    1833,  1833,  1833,  1833,   860,   860,   519,   519,   519,   908,
     908,   908,   908, 16470,  5511, 16470, -1470, 16470, -1470,   858,
   -1470, -1470, -1470, 16556, 16213, 14292, 15951,    64, -1470, 14950,
   -1470, -1470, 11117,   857, -1470,   859,   416, -1470,    53, 13822,
   -1470, -1470, -1470, 13822, -1470, 13822, 13822, -1470,   685, -1470,
   -1470,   279,  1049,   985, 14584, -1470,   866,    23, 16470,   913,
     868, -1470,   871,    29, 13822, 11134,   873, -1470, -1470,   636,
   -1470,   864,   876, -1470,   874, 14679, -1470, 14679, -1470,   880,
   -1470,   942,   881,  1068, -1470,    23,  1053, -1470,   877, -1470,
   -1470,   884,   885,   103, -1470, -1470, 16556,   886,   892, -1470,
    2919, -1470, -1470, -1470, -1470, -1470, 14292, -1470, 14292, -1470,
   16556, 15844, -1470, 14584, 14518, -1470, -1470, -1470, 14584, -1470,
   14584, -1470, 14584, -1470, -1470, 14584, -1470, 14584, -1470,  4630,
   14584, 13822,   889,  7615,   991, -1470, -1470,   423, 14292, 15951,
   -1470, 15889,   938, 15131, -1470, -1470, -1470,   638,  5062,    79,
     439,   107, -1470, -1470, -1470,   943,  2557,  2824, 16470, 16470,
   -1470,    35,  1083,  1019, 14584, -1470, 16556, 11134,   989,   913,
     498,   913,   899, 16470,   901, -1470,  1056,   902,  1134, -1470,
      29, -1470, -1470,   974, -1470, 14679, -1470, 14518, -1470, -1470,
    9478, -1470, -1470, -1470, -1470, 10306, -1470, -1470, -1470,  9478,
   -1470,   910, 14584, 16556,   979, 16556, 16556, 15886, 16556, 15943,
    4630,  5396, -1470, -1470, 14292, 15951, 15951,  1095,    70, -1470,
   -1470, -1470, -1470,    80,   911,    81, -1470, 14194, -1470, -1470,
      84, -1470, -1470, 14805, -1470,   915, -1470,  1039,   453, -1470,
   14292, -1470,   638, -1470,  3321, -1470, -1470, -1470, -1470,  1098,
    1042, 14584, -1470, 16556,   922,   925,   923,   386, -1470,   989,
     913, -1470, -1470, -1470, -1470,  1378,   926, 14679, -1470,   999,
    9478, 10513, 10306, -1470, -1470, -1470,  9478, -1470, 16556, 14584,
   14584, 14584, 13822,  7822, -1470,   927,   931, 14584, 15951, -1470,
   -1470,  2275, 15889, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470,   440, -1470,   938, -1470, -1470, -1470,
   -1470, -1470,    62,   353, -1470,  1121,    87, 16213,  1039,  1122,
     453, -1470, -1470,   935,  1125, 14584, -1470, 16556, -1470,   117,
   -1470, -1470, -1470, -1470,   939,   386,  4315, -1470,   913, -1470,
   14679, -1470, -1470, -1470, -1470,  8029, 16556, 16556, 16556,  5007,
   -1470, -1470, -1470, 16556, -1470,  4819,    38, -1470, -1470, 14584,
   14194, 14194,  1085, -1470, 14805, 14805,   478, -1470, -1470, -1470,
   14584,  1062, -1470,   944,    92, 14584, 16213, -1470, 14584, 16556,
    1064, -1470,  1136,  8236,  8443, -1470, -1470, -1470,   386, -1470,
    8650,   948,  1067,  1040, -1470,  1055,  1004, -1470, -1470,  1057,
    2275, -1470, 16556, -1470, -1470,   995, -1470,  1123, -1470, -1470,
   -1470, -1470, 16556,  1140, -1470, -1470, 16556,   957, 16556, -1470,
     125,   963, -1470, -1470,  8857, -1470,   965, -1470, -1470,   975,
    1010, 16213,   439, -1470, -1470, 14584,    30, -1470,  1101, -1470,
   -1470, -1470, -1470, 15951,   715, -1470,  1018, 16213,   403, 16556,
     981,  1174,   488,    30, -1470,  1105, -1470, 15951,   983, -1470,
     913,    83, -1470, -1470, -1470, -1470, 14292, -1470,   987,   988,
      93, -1470,   404,   488,   330,   913,   990, -1470, -1470, -1470,
   -1470, 14292,    39,  1177,  1113,   404, -1470,  9064,   331,  1181,
    1119, 14584, -1470, -1470,  9271, -1470,    50,  1185,  1124, 14584,
   -1470, 16556, -1470,  1186,  1127, 14584, -1470, 16556, 14584, -1470,
   16556, 16556
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1470, -1470, -1470,  -520, -1470, -1470, -1470,    -4, -1470, -1470,
   -1470,   691,   425,   418,   -17,  1080,  3857, -1470,  2450, -1470,
    -423, -1470,     5, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470,  -476, -1470, -1470,  -180,     4,     8,
   -1470, -1470, -1470, -1470, -1470, -1470,    11, -1470, -1470, -1470,
   -1470,    12, -1470, -1470,   834,   838,   842,  -126,   325,  -807,
     329,   410,  -466,   120,  -830, -1470,  -207, -1470, -1470, -1470,
   -1470,  -675,   -33, -1470, -1470, -1470, -1470,  -453, -1470,  -563,
   -1470,  -391, -1470, -1470,   721, -1470,  -188, -1470, -1470,  -986,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,
    -209, -1470, -1470, -1470, -1470, -1470,  -292, -1470,   -58, -1469,
   -1470, -1460,  -469, -1470,  -161,    -1,  -131,  -456, -1470,  -299,
   -1470,   -67,   -21,  1220,  -681,  -367, -1470, -1470,   -39, -1470,
   -1470,  3867,   -44,  -167, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470,  -546,  -793, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470, -1470,   865, -1470,
   -1470,   261, -1470,   777, -1470, -1470, -1470, -1470, -1470, -1470,
   -1470,   263, -1470,   778, -1470, -1470,   497, -1470,   232, -1470,
   -1470, -1470, -1470, -1470, -1470, -1470, -1470,  -976, -1470,  1921,
    3246,  -342, -1470, -1470,   206,  3665,  4382, -1470, -1470,   312,
     -90,  -601, -1470, -1470,   393,  -684,   195, -1470, -1470, -1470,
   -1470, -1470,   375, -1470, -1470, -1470,   130,  -825,  -190,  -416,
    -407,  -140, -1470, -1470,    14, -1470, -1470,   370,   -64, -1470,
   -1470,   -11,  -124, -1470,   171, -1470, -1470, -1470,  -401,  1002,
   -1470, -1470, -1470, -1470, -1470,   976, -1470, -1470, -1470,   339,
   -1470, -1470, -1470,   560,   391, -1470, -1470,  1016,  -305,  -983,
   -1470,   -50,   -75,  -200,   -84,   561, -1470,  -979, -1470,   288,
     366, -1470, -1470, -1470,  -198, -1022
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -899
static const yytype_int16 yytable[] =
{
     121,   396,   263,   135,   352,   426,   862,   909,   129,   128,
     568,   239,   130,   313,   268,   132,   133,   846,   164,   232,
     319,   320,   244,  1052,   687,   418,   248,   447,   683,   684,
    1221,   660,   709,   845,   819,   706,   395,  1042,   926,   280,
     217,   219,   640,   451,   707,   251,   324,  1640,   261,   328,
     424,   940,  1206,  1205,   941,   352,   472,   473,   705,   348,
    1066,   309,   477,   723,   310,   295,  1271,  1022,  1307,   326,
    1218,  1604,   342,  -483,    13,    13,  1077,   792,   725,  1458,
     774,   459,   311,   792,   295,   516,   521,   526,  1410,  -287,
    1462,   295,   295,  1546,   452,  1605,  1611,  1114,  -483,  -779,
    -776,  1611,  1458,  1325,  1222,   808,   808,   808,   504,    13,
     544,   961,   808,  1207,  1419,  1626,   808,   190,  1729,   289,
    1123,  1124,   339,   321,  1212,  1094,  1208,   327,    13,  1743,
     295,   801,   802,   444,   162,  1647,  1648,   413,   414,   351,
     285,   603,   413,   414,   413,   414,   436,   286,   416,   438,
     413,   414,  1620,  -898,     3,   518,    13,    13,   235,   445,
    1678,   397,  1209,   384,   238,   416,   505,  -447,  1665,  1308,
    1140,   613,   302,  1213,  1309,   385,    58,    59,    60,   178,
     179,   349,  1310,   272,  -777,  -778,  1095,  1223,  -624,  -783,
     427,   567,  -813,   421,   433,   421,  1621,  1420,  -780,  -816,
    -814,  1730,  -782,  -781,  1679,  -815,   314,   420,   798,   413,
     414,   338,  1744,   825,   818,   942,   587,  -484,   703,  1311,
    1312,   281,  1313,   211,   211,  1126,   531,   437,  1023,   535,
    -228,  -212,   420,  -785,   440,   121,  -779,  -776,  1272,   121,
     446,   322,  1641,   466,   456,  1340,   350,   688,   461,   532,
    1338,   726,  1346,   164,  1348,   724,  1347,   164,   510,   514,
     515,   479,  1121,  1606,  1125,   343,   352,   652,  1314,   793,
     611,  1241,  1459,  1460,   460,   794,   728,  -625,   517,   522,
     527,  1411,  -287,  1463,  -707,   417,  1547,  1264,   923,  1612,
     553,   652,   609,   925,  1655,  1717,   520,   809,   896,  1198,
    1035,  1063,   417,  1251,  1364,   528,   528,   533,  1413,  -228,
    -707,  1053,   540,  -707,  1331,   652,   826,   300,   549,   313,
     348,  -777,  -778,  -786,   652,   121,  -783,   652,   135,  -813,
     422,   827,   422,   129,   600,  -780,  -816,  -814,   261,  -782,
    -781,   295,  -815,   164,   694,   780,   543,   282,   300,   423,
     116,  1123,  1124,   550,   300,  1054,   300,   245,  1332,   661,
    1427,   550,  1429,  1435,   246,  1722,  1736,   247,   695,   784,
     785,   438,  -898,   232,  1011,   291,   300,  1014,   842,   843,
     651,   301,  1607,   545,   303,   304,   850,   650,   295,   654,
     295,   295,   292,   339,   293,   852,   218,   218,   300,  1608,
     657,   338,  1609,   550,   680,   294,  1028,   413,   414,  1723,
    1737,   677,   338,  1254,   298,   303,   304,   299,   712,   314,
    1307,   303,   304,   303,   304,   338,   338,  -898,   651,   315,
     338,  -898,  1261,   338,   396,   697,  1056,   704,  1055,   540,
     680,  1333,   302,   303,   304,  1057,   316,   677,   317,   437,
     879,  1575,   339,   583,  1599,   300,  1274,   318,   121,   330,
     331,    13,   555,   556,   551,   303,   304,   717,   337,   395,
    1600,   341,   287,   338,   831,  -481,   164,   885,   599,   889,
     288,   836,   391,  -898,   840,   598,  -898,  1601,   344,  1074,
     777,  1374,  1724,  1738,   781,   920,   921,   879,   300,   280,
    1103,  1230,  1307,   334,  1232,  1220,   852,  1649,   353,  1104,
     354,  1120,   916,   355,   787,    58,    59,    60,   178,   179,
     349,  1308,   303,   304,  1650,   568,  1309,  1651,    58,    59,
      60,   178,   179,   349,  1310,   356,   603,   603,   869,   813,
     815,  1105,  1106,    13,  1080,  1122,  1123,  1124,  1385,  1386,
     447,    58,    59,    60,   178,   179,   349,   877,   357,   644,
     358,  1352,   917,  1353,  1439,   303,   304,    51,   992,   993,
     994,  1311,  1312,   387,  1313,    58,    59,    60,   178,   179,
     349,   300,   388,   218,   995,   350,   550,   681,  1571,  1572,
     218,   389,   685,   390,   397,   419,   218,  -784,   350,  1628,
    1703,  1704,  1705,  1308,  -482,  1699,  1718,  1719,  1309,   295,
      58,    59,    60,   178,   179,   349,  1310,  -624,   858,   307,
    1324,   350,   425,  1036,  1643,  1644,   428,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   409,   410,   381,
     382,   383,   946,   384,   430,   350,  1048,   432,   303,   304,
     949,   952,   218,  1311,  1312,   385,  1313,  1058,   568,   385,
     339,   218,   218,   534,  1200,  1268,  1123,  1124,   218,   439,
    1342,  1438,  1330,   887,   218,   411,   412,   333,   335,   336,
     350,   420,  1236,   442,  1714,   443,   449,   907,  -623,   912,
     448,  1244,   546,   457,   924,   470,   552,   474,  -893,  1728,
     475,   478,  1428,   480,   523,   121,   567,   481,   135,   524,
     583,   483,   913,   129,   914,  1263,   525,   537,   536,   933,
     121,  1712,   546,   164,   552,   546,   552,   552,   652,   935,
     576,   585,   577,   586,   588,   931,  1725,   597,   164,   413,
     414,   -65,    51,  1127,   610,  1128,   691,    58,    59,    60,
      61,    62,   349,  1579,   693,   696,   702,   715,    68,   392,
     459,   719,   510,   722,   734,   735,   514,  1303,   760,   761,
     121,   763,   764,   135,   772,   775,  1013,   779,   129,   778,
    1016,  1017,   782,   783,   791,   795,   805,  1436,   164,   796,
     799,   568,   652,   800,   393,   218,   394,   807,  1321,   810,
    1024,   816,   821,   822,   829,   218,  1020,   824,   830,   832,
    1079,   833,   834,   587,   835,   121,   838,   350,   135,   839,
    1098,   540,  1030,   129,   128,   844,   841,   130,   847,  1043,
     132,   133,   848,   164,  1360,   484,   485,   486,   855,   567,
     857,   680,   487,   488,  1344,  -485,   489,   490,   677,   860,
    1369,  1692,  1219,   861,   850,   863,   866,  1226,   872,   875,
     873,   876,  1133,   890,   880,   894,   892,   893,  1692,  1137,
     937,   868,   927,   918,   944,   232,  1713,   943,  1239,   945,
     939,   295,   378,   379,   380,   381,   382,   383,   960,   384,
     947,   962,   968,  1087,  1087,   907,  1629,   963,   964,  1108,
     965,   385,   969,   966,   997,   680,   989,   990,   991,   992,
     993,   994,   677,   998,   999,  1003,  1006,   121,  1009,   121,
    1027,   121,   135,  1019,   135,   995,  1109,   129,  1031,   129,
    1021,  1032,  1038,  1058,  1040,   164,  1440,   164,  1047,   164,
    1129,   931,  1115,  1050,  1049,  1446,  1424,  1064,  1076,   162,
    1073,  1083,  1082,  1093,  1096,  1097,  1099,  1116,  1139,  1453,
    1111,  1113,  1117,  1119,  1152,    58,    59,    60,    61,    62,
     349,  1131,   567,   995,   218,  1136,    68,   392,   583,   568,
    -899,  -899,  -899,  -899,   376,   377,   378,   379,   380,   381,
     382,   383,  1201,   384,   583,  1132,  1135,  1305,  1189,   545,
    1195,  1196,  1248,  1199,  1215,   385,  1217,  1191,  1224,  1225,
    1192,  1231,  1238,  1229,   394,  1233,  1235,  1240,  1237,  1245,
    1585,  1661,  1247,  1250,  1258,  1246,  1257,  1260,  1265,   218,
    1269,   121,  1275,  1278,   135,   350,  1276,  1284,  1288,   129,
     128,  1279,  1292,   130,  1282,  1283,   132,   133,  1286,   164,
    1287,  1285,   568,  1297,  1296,  1289,  1291,  1294,  1293,  1302,
    1307,  1322,  1323,  1334,  1335,  1298,  1337,  1349,   218,  1339,
     218,  1355,  1341,  1227,  1345,  1351,  1350,  1357,  1370,  1361,
    1371,  1354,  1356,  1359,  1256,  1362,  1363,  1384,  1366,   907,
    1702,   218,  1382,   907,  1367,  1399,  1415,  1421,  1422,  1425,
    1430,    13,  1431,  1437,   121,  1433,   214,   214,  1449,  1457,
     227,  1447,  1565,  1259,  1461,  1255,   121,  1554,  1555,   135,
    1409,  1566,   164,  1568,   129,  1569,  1570,  1578,  1580,  1591,
     540,   931,   227,  1592,   164,  1610,  1615,  1617,  1307,  1618,
    1645,  1653,  1625,  1659,  1654,  1660,  1668,  1623,  1669,  1624,
    1667,  -283,  1671,  1672,  1675,  1674,  1605,  1677,  1630,  1412,
     567,  1308,   218,  1358,  1680,   162,  1309,  1682,    58,    59,
      60,   178,   179,   349,  1310,  1683,  1684,   218,   218,    13,
    1694,  1697,  1700,  1701,  1709,  1711,  1454,   352,  1715,  1716,
    1304,  1731,  1732,  1726,   583,  1739,  1318,   583,  1740,  1745,
    1748,  1015,  1664,  1746,  1012,  1318,  1749,   786,  1696,  1078,
    1075,  1311,  1312,   540,  1313,   428,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,  1557,   656,
     653,  1710,  1037,   567,  1262,  1584,   655,  1368,   350,  1308,
    1708,   907,   789,   907,  1309,  1576,    58,    59,    60,   178,
     179,   349,  1310,  1598,  1603,  1406,  1733,  1721,  1387,  1614,
    1432,   241,  1574,   663,   411,   412,  1186,  1188,  1005,   770,
     771,  1210,  1138,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,  1243,  1100,  1249,  1311,
    1312,  1089,  1313,   214,   575,   950,  1727,   218,   218,   121,
     214,   542,   135,  1734,   261,  1149,   214,   129,   530,  1404,
    1194,  1130,     0,     0,     0,     0,   350,   164,     0,   397,
       0,  1408,   411,   412,     0,     0,     0,  1320,   413,   414,
       0,     0,     0,     0,   227,   227,  1320,     0,  1434,  1318,
     227,     0,     0,     0,     0,  1318,     0,  1318,     0,     0,
       0,   907,     0,     0,     0,     0,   121,     0,     0,   135,
    1559,   121,   214,   583,   129,   121,     0,     0,   135,     0,
    1444,   214,   214,   129,   164,     0,     0,     0,   214,   164,
    1616,     0,  1307,   164,   214,     0,   413,   414,     0,     0,
       0,     0,     0,  1543,     0,   227,     0,     0,     0,  1550,
       0,     0,   797,     0,     0,     0,   261,     0,     0,     0,
     261,     0,     0,     0,     0,     0,  1560,     0,     0,   227,
       0,     0,   227,    13,     0,     0,     0,     0,     0,   218,
       0,     0,     0,   907,  1318,     0,   121,   121,   121,   135,
       0,     0,   121,     0,   129,   135,  1582,  1444,     0,   121,
     129,     0,   135,     0,   164,   164,   164,   129,     0,     0,
     164,     0,     0,     0,   227,     0,     0,   164,     0,     0,
    1320,   218,     0,     0,     0,     0,  1320,     0,  1320,     0,
     583,     0,  1613,  1308,     0,     0,   218,   218,  1309,     0,
      58,    59,    60,   178,   179,   349,  1310,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,   850,     0,
       0,  1687,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,   850,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1311,  1312,     0,  1313,     0,     0,     0,
       0,  1657,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   295,   227,   227,     0,   352,   750,     0,
     350,     0,     0,     0,     0,  1320,     0,     0,     0,   218,
       0,     0,   261,     0,     0,     0,   907,     0,     0,     0,
       0,   121,  1577,     0,   135,     0,     0,     0,     0,   129,
       0,  1635,     0,     0,     0,     0,  1543,  1543,     0,   164,
    1550,  1550,     0,     0,     0,   750,     0,     0,     0,     0,
       0,     0,   295,     0,     0,     0,     0,     0,     0,   121,
     121,     0,   135,   135,     0,     0,   121,   129,   129,   135,
       0,     0,     0,     0,   129,     0,     0,   164,   164,     0,
       0,     0,     0,     0,   164,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,   227,     0,     0,
     121,     0,     0,   135,   227,     0,     0,  1686,   129,     0,
       0,  1688,   359,   360,   361,     0,     0,     0,   164,     0,
       0,     0,     0,  1698,   214,     0,     0,     0,     0,     0,
       0,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,   135,   385,     0,     0,
     121,   129,     0,   135,   359,   360,   361,     0,   129,   214,
       0,   164,     0,     0,     0,     0,     0,     0,   164,     0,
       0,     0,     0,   362,     0,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   384,   214,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,   385,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,   750,     0,     0,     0,   583,     0,     0,     0,
       0,   359,   360,   361,   227,   227,   750,   750,   750,   750,
     750,     0,     0,   583,     0,     0,     0,     0,     0,   750,
     362,   583,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,   384,   227,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   385,     0,     0,  1203,
       0,     0,   214,  -899,  -899,  -899,  -899,   987,   988,   989,
     990,   991,   992,   993,   994,   227,     0,   214,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   995,     0,
       0,   227,   227,     0,     0,     0,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,     0,  1084,  1085,  1086,
      36,     0,     0,     0,   227,     0,     0,     0,   817,     0,
       0,     0,     0,     0,     0,   227,     0,     0,     0,     0,
       0,     0,     0,   750,     0,     0,   227,   215,   215,     0,
       0,   228,     0,     0,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,     0,     0,     0,
       0,     0,     0,   362,     0,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   384,    83,    84,
       0,    85,   183,    87,     0,   854,     0,   214,   214,   385,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   227,     0,   227,     0,   227,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     750,     0,     0,   227,   750,   750,   750,     0,     0,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,    36,     0,   211,   359,
     360,   361,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   362,   750,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   384,     0,     0,     0,     0,     0,     0,     0,
     227,     0,   227,     0,   385,     0,     0,     0,     0,   214,
       0,   215,     0,     0,     0,     0,     0,     0,   891,     0,
       0,     0,     0,     0,    83,    84,   227,    85,   183,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,     0,     0,     0,
       0,   214,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   214,   214,     0,   750,
       0,     0,   676,   215,   116,     0,     0,     0,     0,     0,
       0,   227,   215,   215,   750,     0,   750,     0,     0,   215,
       0,     0,   359,   360,   361,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,   750,
       0,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,     0,     0,     0,
       0,     0,     0,   895,     0,   227,   227,   385,     0,   214,
       0,     0,     0,     0,     0,     0,     0,   359,   360,   361,
     428,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,     0,   750,   228,   362,     0,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,     0,     0,     0,     0,     0,     0,     0,   411,
     412,     0,   385,     0,     0,     0,   215,     0,   254,    33,
      34,    35,     0,     0,     0,     0,   227,     0,   227,     0,
     741,     0,     0,   750,   227,     0,     0,     0,   750,     0,
     750,     0,   750,     0,   255,   750,     0,   750,     0,     0,
     750,     0,     0,     0,     0,     0,     0,     0,   227,   227,
       0,   227,     0,     0,     0,     0,    36,     0,   227,   756,
       0,     0,     0,   413,   414,     0,     0,     0,    72,    73,
      74,    75,    76,     0,   750,     0,  1018,     0,     0,   743,
       0,     0,     0,   476,     0,    79,    80,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,     0,    89,
       0,     0,     0,     0,     0,     0,   756,     0,     0,     0,
     256,   257,   750,     0,     0,     0,    97,     0,     0,     0,
       0,     0,     0,     0,   227,   227,   227,     0,   182,     0,
       0,    81,   258,     0,    83,    84,     0,    85,   183,    87,
       0,  1072,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,   259,     0,   227,     0,     0,     0,     0,     0,
       0,   750,   262,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   260,
       0,     0,   359,   360,   361,   215,     0,     0,     0,   750,
     750,   750,     0,     0,     0,     0,     0,   750,   227,     0,
       0,   362,   227,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,   359,   360,   361,
       0,     0,     0,     0,     0,     0,     0,   385,     0,     0,
     215,     0,     0,     0,     0,     0,   362,     0,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,   215,   385,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   215,   756,     0,   750,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   995,   227,   756,   756,   756,
     756,   756,     0,     0,     0,     0,     0,     0,     0,     0,
     756,     0,     0,     0,     0,   227,     0,     0,     0,   750,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     750,     0,     0,     0,     0,   750,  1008,     0,   750,     0,
       0,     0,     0,     0,   262,   262,     0,     0,     0,     0,
     262,     0,     0,   215,     0,     0,  1081,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,     0,   215,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1026,     0,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,   750,     0,     0,     0,     0,
       0,  1417,     0,   227,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,     0,     0,
       0,     0,     0,     0,   756,     0,   227,  1065,     0,   262,
       0,     0,   262,     0,     0,     0,     0,     0,     0,     0,
       0,   227,     0,     0,     0,     0,     0,   228,     0,     0,
       0,   750,     0,     0,     0,     0,     0,     0,     0,   750,
       0,     0,     0,     0,     0,   750,     0,     0,   750,     0,
       0,     0,     0,     0,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,   215,
       0,     0,     0,   362,     0,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   384,     0,     0,
       0,   756,     0,     0,   215,   756,   756,   756,     0,   385,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,     0,     0,     0,
       0,     0,     0,     0,   262,   730,     0,     0,   752,   359,
     360,   361,     0,     0,     0,     0,     0,     0,     0,     0,
     756,     0,     0,     0,     0,     0,     0,     0,   362,  1271,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   384,     0,     0,   752,   360,   361,     0,     0,
     215,     0,     0,     0,   385,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   362,     0,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   215,   384,     0,
       0,     0,   215,     0,     0,     0,   262,   262,  1418,     0,
     385,     0,     0,     0,   262,     0,     0,   215,   215,     0,
     756,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   756,     0,   756,     0,     0,
     359,   360,   361,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   362,
     756,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,     0,   384,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   385,     0,  1306,     0,     0,
     215,  1272,     0,     0,     0,   359,   360,   361,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   362,   756,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,     0,   384,     0,
       0,     0,   752,     0,     0,     0,     0,     0,     0,     0,
     385,     0,     0,     0,   262,   262,   752,   752,   752,   752,
     752,     0,     0,     0,     0,     0,     0,     0,     0,   752,
       0,     0,     0,     0,   756,   215,     0,     0,     0,   756,
       0,   756,     0,   756,     0,     0,   756,     0,   756,     0,
       0,   756,     0,     0,     0,     0,     0,     0,     0,     0,
    1389,     0,  1398,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   386,   384,     0,   756,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   385,     0,     0,     0,     0,
       0,   262,   216,   216,     0,     0,   229,     0,   215,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   756,   262,     0,     0,   359,   360,   361,
       0,     0,     0,     0,     0,   262,  1455,  1456,     0,     0,
       0,     0,     0,   752,     0,     0,   362,   469,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,   756,     0,     0,     0,     0,     0,     0,     0,
     254,     0,   385,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,     0,
     756,   756,   756,     0,     0,     0,   255,     0,   756,  1594,
       0,     0,   995,  1398,     0,     0,     0,     0,     0,     0,
       0,   262,     0,   262,     0,   730,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     752,     0,     0,     0,   752,   752,   752,     0,     0,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,     0,     0,     0,     0,
       0,     0,   256,   257,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,     0,     0,   216,     0,     0,   752,
     182,     0,   216,    81,   258,     0,    83,    84,     0,    85,
     183,    87,     0,     0,     0,     0,   756,     0,     0,   471,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
     262,     0,   262,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     756,   260,     0,     0,    36,  1561,   262,     0,   216,     0,
       0,   756,     0,     0,     0,     0,   756,   216,   216,   756,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   566,     0,     0,     0,     0,     0,     0,     0,   752,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,     0,     0,   752,     0,   752,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,     0,     0,   307,
       0,     0,    83,    84,  1695,    85,   183,    87,     0,   752,
       0,     0,     0,     0,     0,     0,     0,     0,  1389,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     229,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,   262,     0,     0,     0,     0,
     308,     0,   756,     0,     0,     0,     0,     0,     0,     0,
     756,   359,   360,   361,     0,     0,   756,     0,     0,   756,
       0,   216,     0,     0,   752,     0,     0,     0,     0,     0,
     362,   216,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,   384,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   385,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,     0,   262,     0,
      36,     0,   211,   752,     0,     0,     0,     0,   752,     0,
     752,     0,   752,     0,     0,   752,     0,   752,     0,     0,
     752,     0,     0,     0,     0,     0,     0,     0,   262,     0,
       0,     0,     0,     0,   359,   360,   361,     0,   262,     0,
       0,     0,     0,     0,     0,     0,   648,     0,     0,     0,
       0,     0,     0,   362,   752,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   384,    83,    84,
       0,    85,   183,    87,     0,     0,     0,     0,     0,   385,
       0,     0,   752,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     216,     0,     0,   482,     0,     0,   649,     0,   116,     0,
     262,     0,     0,     0,   262,     0,     0,     0,     0,     0,
       0,   752,   187,   189,     0,   191,   192,   193,   195,   196,
       0,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,     0,     0,   221,   224,     0,     0,   752,
     752,   752,   359,   360,   361,   216,     0,   752,   242,     0,
       0,     0,     0,     0,     0,   250,     0,   253,     0,     0,
     269,   362,   274,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   216,   384,   216,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   506,   385,     0,     0,
       0,     0,   359,   360,   361,     0,     0,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     325,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   752,     0,   385,     0,     0,
       0,     0,     0,     0,     0,     0,   262,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,     0,     0,  1636,     0,     0,     0,   752,
       0,     0,     0,   216,   216,     0,     0,   429,     0,     0,
     752,     0,     0,     0,     0,   752,     0,     0,   752,     0,
       0,     0,     0,     0,     0,     0,   566,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   689,   384,     0,     0,     0,     0,     0,
       0,     0,   454,     0,     0,   454,   385,     0,     0,     0,
       0,     0,   242,   465,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   752,     0,     0,     0,     0,
       0,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   757,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   708,     0,     0,   262,     0,     0,     0,
     325,     0,     0,     0,     0,     0,   221,     0,     0,     0,
     548,   262,     0,   216,   216,     0,     0,     0,     0,     0,
       0,   752,     0,   569,   572,   572,     0,     0,     0,   752,
     757,     0,     0,     0,     0,   752,     0,   596,   752,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   608,   566,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   614,   615,   616,   618,
     619,   620,   621,   622,   623,   624,   625,   626,   627,   628,
     629,   630,   631,   632,   633,   634,   635,   636,   637,   638,
     639,     0,   641,     0,   642,   642,     0,   645,     0,     0,
       0,     0,     0,     0,     0,   662,   664,   665,   666,   667,
     668,   669,   670,   671,   672,   673,   674,   675,     0,     0,
       0,     0,     0,   642,   682,     0,   608,   608,   642,   686,
       0,     0,     0,     0,     0,   662,     0,     0,   690,     0,
       0,     0,     0,     0,     0,   216,     0,     0,   699,     0,
     701,     0,     0,     0,     0,     0,   608,     0,     0,     0,
       0,     0,     0,     0,   713,     0,   714,     0,     0,     0,
       0,     0,     0,     0,     0,   751,     0,     0,     0,     0,
       0,     0,   566,     0,   254,     0,     0,   216,     0,     0,
       0,     0,   762,     0,     0,   765,   768,   769,     0,     0,
       0,     0,   216,   216,     0,     0,     0,     0,     0,     0,
     255,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   751,     0,     0,     0,   788,   757,     0,     0,
       0,     0,    36,     0,   970,   971,   972,     0,     0,     0,
       0,   757,   757,   757,   757,   757,     0,     0,     0,     0,
       0,     0,     0,   973,   757,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,   256,   257,     0,   995,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   182,     0,     0,    81,   258,     0,
      83,    84,   859,    85,   183,    87,     0,     0,     0,     0,
       0,     0,     0,     0,   870,     0,     0,     0,   259,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   260,   878,     0,     0,  1627,
       0,     0,     0,     0,     0,   195,     0,     0,   757,     0,
     566,     0,   359,   360,   361,     0,     0,     0,     0,     0,
       0,     0,     0,   888,     0,     0,     0,     0,     0,     0,
       0,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,     0,     0,   751,
       0,     0,     0,     0,     0,  1150,   242,   385,     0,     0,
       0,     0,     0,   751,   751,   751,   751,   751,     0,     0,
       0,     0,     0,   566,     0,     0,   751,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   757,     0,   996,     0,   757,
     757,   757,     0,     0,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,     0,  1033,   757,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   995,  1041,    36,     0,     0,
       0,     0,  1044,     0,  1045,     0,  1046,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1004,     0,
     751,     0,     0,     0,     0,     0,     0,     0,  1061,     0,
     359,   360,   361,     0,     0,     0,     0,     0,  1069,     0,
       0,  1070,     0,  1071,     0,     0,     0,   608,     0,   362,
       0,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,     0,   384,     0,    83,    84,     0,    85,   183,
      87,     0,  1102,     0,   757,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   757,
       0,   757,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   751,     0,     0,
     610,   751,   751,   751,   757,     0,   751,   751,   751,   751,
     751,   751,   751,   751,   751,   751,   751,   751,   751,   751,
     751,   751,   751,   751,   751,   751,   751,   751,   751,   751,
     751,   751,   751,     0,     0,     0,     0,     0,     0,     0,
     758,     0,     0,     0,  1183,  1184,  1185,     0,     0,     0,
     765,  1187,     0,     0,     0,     0,   751,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   757,
       0,     0,     0,     0,     0,     0,    36,     0,  1202,     0,
       0,     0,     0,     0,     0,     0,     0,   790,     0,     0,
       0,     0,  1000,  1001,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     608,     0,     0,     0,     0,     0,     0,     0,   757,   608,
    1202,     0,     0,   757,     0,   757,     0,   757,     0,     0,
     757,     0,   757,     0,     0,   757,     0,     0,   182,     0,
       0,    81,     0,     0,    83,    84,   751,    85,   183,    87,
     242,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1270,   751,     0,   751,     0,     0,     0,     0,     0,   757,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   751,   359,   360,   361,
       0,     0,     0,  1634,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   362,   757,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   385,     0,     0,     0,  1326,     0,     0,     0,
    1327,   751,  1328,  1329,     0,     0,   757,     0,     0,     0,
       0,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1343,   608,     0,   932,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   757,   757,   757,   255,   953,   954,
     955,   956,   757,     0,     0,     0,  1597,     0,     0,     0,
       0,   967,     0,     0,     0,     0,     0,     0,     0,    36,
     751,     0,     0,     0,     0,   751,     0,   751,     0,   751,
       0,     0,   751,     0,   751,     0,     0,   751,     0,     0,
       0,     0,     0,     0,     0,     0,  -325,     0,  1381,     0,
       0,   254,     0,     0,    58,    59,    60,   178,   179,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   751,     0,   256,   257,     0,  1632,   255,     0,     0,
       0,     0,     0,     0,   608,     0,     0,     0,     0,     0,
       0,   182,     0,     0,    81,   258,     0,    83,    84,    36,
      85,   183,    87,     0,     0,     0,     0,     0,     0,   751,
     757,     0,     0,     0,     0,   259,     0,     0,     0,     0,
       0,     0,     0,     0,   350,  1062,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   260,     0,   757,     0,     0,     0,     0,     0,
       0,     0,     0,   256,   257,   757,     0,     0,   751,     0,
     757,     0,     0,   757,     0,     0,     0,     0,     0,     0,
       0,   182,     0,     0,    81,   258,     0,    83,    84,     0,
      85,   183,    87,     0,   948,  1673,   751,   751,   751,     0,
       0,     0,     0,     0,   751,   259,     0,     0,     0,  1589,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     757,     0,   260,     0,     0,     0,  1143,  1146,  1146,     0,
       0,  1153,  1156,  1157,  1158,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,     0,     0,
       0,    36,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   757,     0,     0,     0,
       0,  1193,     0,     0,   757,     0,   359,   360,   361,     0,
     757,     0,     0,   757,     0,     0,     0,     0,     0,     0,
       0,     0,   751,     0,     0,   362,     0,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,     0,   384,
       0,     0,     0,     0,     0,     0,   751,     0,     0,    83,
      84,   385,    85,   183,    87,     0,     0,   751,     0,     0,
       0,     0,   751,     0,     0,   751,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,  1266,     0,     0,     0,     0,     0,   649,     0,   116,
       0,     0,     0,     0,     0,     0,  1280,     0,  1281,     0,
       0,   359,   360,   361,     0,     0,     0,     0,     0,     0,
       0,     0,   751,     0,     0,     0,     0,     0,     0,     0,
     362,  1299,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,   384,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   385,     0,     0,     5,
       6,     7,     8,     9,  1452,     0,     0,     0,   751,    10,
       0,     0,     0,     0,     0,     0,   751,     0,     0,     0,
       0,     0,   751,   658,    12,   751,  1336,     0,     0,     0,
       0,   659,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,  1373,     0,     0,     0,     0,
    1375,     0,  1376,     0,  1377,     0,     0,  1378,     0,  1379,
      51,     0,  1380,     0,     0,     0,     0,     0,    58,    59,
      60,   178,   179,   180,     0,     0,    65,    66,  1301,     0,
       0,     0,     0,     0,     0,     0,   181,    71,     0,    72,
      73,    74,    75,    76,     0,     0,  1423,     0,     0,     0,
      77,     0,     0,     0,     0,   182,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   183,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,   254,    91,
      92,    93,    94,     0,  1448,     0,     0,    97,    98,   266,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   255,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,  1567,     0,     0,   886,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1586,  1587,  1588,     0,     0,     0,     0,     0,  1593,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     256,   257,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,     0,
       0,    81,   258,     0,    83,    84,     0,    85,   183,    87,
       0,  1277,     0,     0,     0,     0,     0,   182,     0,     0,
      81,    82,   259,    83,    84,     0,    85,   183,    87,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   260,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   213,     0,
       0,     0,     0,   116,     0,     0,     0,  1619,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,  1642,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,  1652,     0,     0,     0,     0,  1656,     0,     0,
    1658,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,  1689,     0,     0,
      48,    49,    50,    51,    52,    53,    54,     0,    55,    56,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,    67,    68,    69,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,    78,    79,
      80,    81,    82,  1741,    83,    84,     0,    85,    86,    87,
      88,  1747,     0,    89,     0,     0,    90,  1750,     0,     0,
    1751,     0,    91,    92,    93,    94,    95,     0,    96,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1034,   116,   117,     0,   118,   119,     5,
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
      51,    52,    53,    54,     0,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,    78,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,    95,     0,    96,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1204,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,     0,    55,    56,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,    78,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
      95,     0,    96,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   182,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   183,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,   589,   116,   117,     0,   118,   119,
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
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1007,   116,   117,     0,   118,   119,     5,     6,     7,
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
       0,     0,     0,   182,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   183,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1110,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
    1112,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     182,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     183,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,  1267,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   182,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   183,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   182,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   183,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1383,
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
       0,   182,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   183,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1590,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,  1631,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   182,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   183,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
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
      77,     0,     0,     0,     0,   182,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   183,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1662,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
       0,     0,   182,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   183,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1663,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,  1666,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   182,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   183,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
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
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1681,   116,   117,     0,   118,   119,     5,     6,     7,
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
       0,     0,     0,   182,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   183,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1735,   116,
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
     182,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     183,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1742,   116,   117,     0,   118,
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
       0,     0,    77,     0,     0,     0,     0,   182,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   183,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     178,   179,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   182,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   183,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   716,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   178,   179,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   182,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   183,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,   934,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,   178,   179,    63,     0,    64,    65,
      66,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   182,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   183,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1443,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,   178,   179,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   182,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   183,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1581,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,   178,   179,
      63,     0,    64,    65,    66,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   182,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   183,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   178,   179,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   182,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   183,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
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
      59,    60,   178,   179,   180,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
     266,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   267,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   362,    10,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   604,
     384,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   385,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   178,
     179,   180,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   182,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   183,    87,     0,   605,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   178,   179,   180,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     182,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     183,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,   971,   972,   711,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     973,    10,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,  1059,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   995,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   178,   179,   180,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   182,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   183,    87,     0,
    1060,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   658,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     178,   179,   180,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   182,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   183,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,    94,     0,    10,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   194,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   178,   179,   180,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   182,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   183,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   361,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   362,
      10,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   220,   384,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   385,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   178,   179,   180,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,    94,     0,    10,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   178,   179,   180,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   182,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   183,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,    94,     0,
      10,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   249,     0,     0,   116,   117,     0,
     118,   119,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   178,   179,   180,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,    94,     0,    10,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   252,
       0,     0,   116,   117,     0,   118,   119,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   178,   179,   180,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   182,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   183,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,   453,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  -899,  -899,  -899,  -899,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   617,
     384,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   385,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   178,   179,   180,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   182,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   183,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   972,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   973,    10,
     974,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   659,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   995,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   178,   179,   180,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   182,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   183,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   973,    10,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   698,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   995,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   178,   179,
     180,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   182,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   183,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   700,   384,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   385,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   178,   179,   180,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   182,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   183,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,  1101,   384,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   385,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   178,   179,   180,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,    94,     0,    10,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   178,   179,   180,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   182,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   183,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,    94,     0,
      10,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,   547,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   178,   179,   180,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   182,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   183,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,  1464,  1465,  1466,
    1467,  1468,     0,     0,  1469,  1470,  1471,  1472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1473,  1474,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,     0,     0,     0,     0,  1475,     0,     0,
       0,     0,     0,     0,     0,     0,   995,     0,     0,     0,
       0,  1476,  1477,  1478,  1479,  1480,  1481,  1482,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
    1483,  1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,
    1493,  1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,
    1503,  1504,  1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,
    1513,  1514,  1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,
    1523,   254,     0,     0,  1524,  1525,     0,  1526,  1527,  1528,
    1529,  1530,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1531,  1532,  1533,     0,   255,     0,    83,
      84,     0,    85,   183,    87,  1534,     0,  1535,  1536,     0,
    1537,     0,     0,     0,     0,     0,     0,  1538,     0,    36,
       0,  1539,     0,  1540,     0,  1541,  1542,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,     0,  1159,     0,     0,     0,     0,     0,     0,
       0,     0,   385,   256,   257,     0,     0,     0,     0,   736,
     737,     0,     0,     0,     0,   738,     0,   739,     0,     0,
       0,   182,     0,     0,    81,   258,     0,    83,    84,   740,
      85,   183,    87,     0,     0,     0,     0,    33,    34,    35,
      36,     0,     0,     0,     0,   259,     0,   928,   741,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   260,  -899,  -899,  -899,  -899,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,    36,
       0,   211,     0,     0,   742,     0,    72,    73,    74,    75,
      76,     0,   995,     0,     0,     0,     0,   743,     0,     0,
       0,     0,   182,    79,    80,    81,   744,     0,    83,    84,
       0,    85,   183,    87,     0,     0,     0,    89,     0,   212,
       0,     0,     0,     0,     0,     0,   745,   746,   747,   748,
       0,     0,   929,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   182,     0,   749,    81,    82,     0,    83,    84,     0,
      85,   183,    87,     0,     0,    36,     0,   211,     0,     0,
       0,     0,     0,   561,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     736,   737,   213,     0,     0,   212,   738,   116,   739,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     740,     0,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,     0,     0,     0,     0,     0,   182,     0,   741,
      81,    82,     0,    83,    84,     0,    85,   183,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   742,     0,    72,    73,    74,
      75,    76,     0,   562,     0,   897,   898,     0,   743,     0,
       0,     0,     0,   182,    79,    80,    81,   744,     0,    83,
      84,     0,    85,   183,    87,   899,     0,     0,    89,     0,
       0,     0,     0,   900,   901,   902,    36,   745,   746,   747,
     748,     0,     0,     0,   903,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   749,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    29,    30,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,    38,     0,     0,
     904,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   905,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,    83,    84,     0,    85,   183,    87,
      58,    59,    60,   178,   179,   180,     0,     0,     0,    29,
      30,     0,   906,     0,     0,     0,     0,     0,     0,    36,
       0,   211,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   182,     0,     0,
      81,    82,    36,    83,    84,     0,    85,   183,    87,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   435,     0,
      36,   182,     0,   116,    81,    82,     0,    83,    84,     0,
      85,   183,    87,    36,     0,   211,     0,     0,     0,    90,
       0,     0,     0,     0,     0,     0,     0,     0,  1548,     0,
      83,    84,  1549,    85,   183,    87,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   435,   212,     0,     0,     0,   116,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,  1403,   346,    36,    83,    84,
       0,    85,   183,    87,     0,   182,     0,     0,    81,    82,
       0,    83,    84,     0,    85,   183,    87,    36,     0,   211,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   213,   212,     0,   519,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
     539,     0,     0,     0,     0,    83,    84,     0,    85,   183,
      87,     0,     0,     0,     0,     0,     0,     0,     0,   182,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   183,
      87,    36,     0,   211,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     868,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     213,   212,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,  1029,    36,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   183,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    36,     0,   211,   213,     0,     0,   182,     0,   116,
      81,    82,     0,    83,    84,     0,    85,   183,    87,     0,
       0,     0,  1402,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   225,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,    83,    84,   213,    85,
     183,    87,     0,   116,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   183,    87,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,  1403,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   359,   360,   361,   226,     0,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     362,     0,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,     0,   384,   359,   360,   361,     0,     0,
       0,     0,     0,     0,     0,     0,   385,     0,     0,     0,
       0,     0,     0,     0,   362,     0,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,     0,   384,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     385,     0,   359,   360,   361,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     431,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,   359,   360,   361,     0,
       0,     0,     0,     0,     0,     0,     0,   385,     0,     0,
       0,     0,     0,     0,   441,   362,     0,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,     0,   384,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   385,     0,   359,   360,   361,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   837,   362,     0,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,     0,   384,   359,   360,   361,
       0,     0,     0,     0,     0,     0,     0,     0,   385,     0,
       0,     0,     0,     0,     0,   874,   362,     0,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,     0,
     384,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   385,     0,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   915,   362,     0,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,     0,   384,   970,   971,
     972,     0,     0,     0,     0,     0,     0,     0,     0,   385,
       0,     0,     0,     0,     0,     0,  1216,   973,     0,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   995,     0,   970,   971,   972,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1234,   973,     0,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   970,   971,   972,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     995,     0,     0,     0,     0,     0,   973,  1134,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   995,     0,   970,   971,   972,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   973,  1290,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   970,   971,   972,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   995,
       0,     0,     0,     0,     0,   973,  1295,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,   995,   729,   970,   971,   972,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,   973,  1372,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1390,   995,     0,
       0,     0,    36,     0,     0,  1450,   182,     0,     0,    81,
    1391,  1392,    83,    84,     0,    85,   183,    87,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,   275,
     276,    81,  1393,     0,    83,    84,     0,    85,  1394,    87,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,     0,     0,
       0,     0,  1451,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,   277,   811,   812,
      83,    84,     0,    85,   183,    87,     0,     0,     0,    36,
     182,     0,     0,    81,    82,     0,    83,    84,     0,    85,
     183,    87,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,    83,    84,     0,    85,   183,    87,
       0,     0,     0,     0,   507,     0,     0,    83,    84,     0,
      85,   183,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,     0,   511,     0,     0,    83,    84,     0,    85,
     183,    87,     0,     0,     0,    36,   277,     0,     0,    83,
      84,     0,    85,   183,    87,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   648,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,  1151,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,     0,    83,    84,     0,    85,   183,    87,     0,
       0,     0,     0,     0,     0,   995,    83,    84,     0,    85,
     183,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    83,    84,
       0,    85,   183,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   359,   360,   361,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     720,   362,     0,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,     0,   384,   359,   360,   361,     0,
       0,     0,     0,     0,     0,     0,     0,   385,     0,     0,
       0,     0,     0,     0,     0,   362,   871,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   721,   384,
     359,   360,   361,     0,     0,     0,     0,     0,     0,     0,
       0,   385,     0,     0,     0,     0,     0,     0,     0,   362,
       0,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,     0,   384,   970,   971,   972,     0,     0,     0,
       0,     0,     0,     0,     0,   385,     0,     0,     0,     0,
       0,     0,     0,   973,  1300,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   970,   971,   972,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   995,
       0,     0,     0,     0,     0,   973,     0,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   995,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   995,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,     0,   384,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   385
};

static const yytype_int16 yycheck[] =
{
       4,   162,    52,     4,   135,   185,   607,   691,     4,     4,
     315,    32,     4,    88,    53,     4,     4,   580,     4,    30,
      95,    96,    43,   848,   425,   165,    47,   227,   419,   420,
    1052,   398,   455,   579,   554,   451,   162,   830,   719,    56,
      26,    27,   384,   233,   451,    49,   113,     9,    52,   113,
     174,   726,  1038,  1036,     9,   186,   254,   255,   449,   134,
     867,    78,   260,     9,    81,    69,    30,     9,     4,   113,
    1049,     9,     9,    66,    45,    45,   883,     9,    30,     9,
     503,     9,    86,     9,    88,     9,     9,     9,     9,     9,
       9,    95,    96,     9,   234,    33,     9,   927,    66,    66,
      66,     9,     9,    50,    79,     9,     9,     9,   108,    45,
     300,    84,     9,   154,    79,  1575,     9,   200,    79,    79,
      99,   100,   169,    79,   154,    84,   167,   113,    45,    79,
     134,    46,    47,   223,     4,  1604,  1605,   127,   128,   135,
     116,   339,   127,   128,   127,   128,   213,   123,    66,   213,
     127,   128,    35,   200,     0,    96,    45,    45,   200,   226,
      35,   162,   203,    53,   200,    66,   166,     8,  1628,   105,
     963,   351,   145,   203,   110,    65,   112,   113,   114,   115,
     116,   117,   118,   203,    66,    66,   145,   162,   148,    66,
     186,   315,    66,    66,   198,    66,    79,   162,    66,    66,
      66,   162,    66,    66,    79,    66,   153,   200,   201,   127,
     128,   152,   162,    50,   204,   170,   201,    66,   201,   155,
     156,    79,   158,    79,    79,   204,   293,   213,   170,   293,
     201,   201,   200,   200,   220,   239,   203,   203,   202,   243,
     226,   197,   204,   247,   239,  1231,   182,   427,   243,   293,
    1229,   203,  1238,   239,  1240,   201,  1239,   243,   275,   276,
     277,   265,   937,   201,   939,   202,   397,   391,   204,   201,
     345,  1078,   202,   203,   202,   201,   474,   148,   202,   202,
     202,   202,   202,   202,   201,   203,   202,  1117,   711,   202,
     307,   415,   342,   716,   202,   202,   282,   201,   201,   201,
     820,   864,   203,  1096,   201,   291,   292,   293,   201,   198,
     198,    35,   298,   201,    35,   439,   153,    79,   304,   394,
     395,   203,   203,   200,   448,   329,   203,   451,   329,   203,
     203,   168,   203,   329,   338,   203,   203,   203,   342,   203,
     203,   345,   203,   329,   434,    96,    66,   205,    79,   205,
     205,    99,   100,    84,    79,    79,    79,   200,    79,   398,
    1339,    84,  1341,  1349,   200,    35,    35,   200,   435,    96,
      96,   435,   148,   384,    96,   200,    79,    96,   576,   577,
     391,    84,    29,   145,   146,   147,   584,   391,   392,   393,
     394,   395,   200,   169,   200,   585,    26,    27,    79,    46,
     396,   152,    49,    84,   415,   200,   807,   127,   128,    79,
      79,   415,   152,  1097,   200,   146,   147,   200,   457,   153,
       4,   146,   147,   146,   147,   152,   152,   203,   439,   200,
     152,   148,  1113,   152,   595,   439,   852,   448,   162,   425,
     451,   162,   145,   146,   147,   852,   200,   451,   200,   435,
     650,  1430,   169,   323,    14,    79,   204,   200,   462,   203,
      84,    45,   202,   203,   145,   146,   147,   462,    30,   595,
      30,   200,   115,   152,   564,    66,   462,   657,   209,   679,
     123,   571,   148,   200,   574,   208,   203,    47,    35,   880,
     507,  1284,   162,   162,   511,    71,    72,   697,    79,   516,
     916,  1064,     4,    84,  1067,  1051,   696,    29,   202,   916,
     202,   934,   702,   202,   518,   112,   113,   114,   115,   116,
     117,   105,   146,   147,    46,   830,   110,    49,   112,   113,
     114,   115,   116,   117,   118,   202,   734,   735,   613,   543,
     544,    71,    72,    45,   886,    98,    99,   100,   125,   126,
     750,   112,   113,   114,   115,   116,   117,   647,   202,   388,
     202,  1245,   702,  1247,  1357,   146,   147,   104,    49,    50,
      51,   155,   156,    66,   158,   112,   113,   114,   115,   116,
     117,    79,    66,   213,    65,   182,    84,   416,   202,   203,
     220,   202,   421,   203,   595,   200,   226,   200,   182,  1578,
     112,   113,   114,   105,    66,   202,   202,   203,   110,   613,
     112,   113,   114,   115,   116,   117,   118,   148,   604,   152,
     204,   182,   200,   821,  1600,  1601,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    49,
      50,    51,   732,    53,   202,   182,   844,    44,   146,   147,
     734,   735,   282,   155,   156,    65,   158,   855,   963,    65,
     169,   291,   292,   293,  1031,    98,    99,   100,   298,   148,
    1233,  1355,  1218,   659,   304,    63,    64,   117,   118,   119,
     182,   200,  1073,   207,  1706,     9,   200,   691,   148,   693,
     148,  1082,   301,     8,   715,   202,   305,   169,    14,  1721,
     200,    14,   204,    79,   123,   709,   830,   202,   709,   123,
     580,   202,   698,   709,   700,  1116,    14,   169,   201,   723,
     724,  1700,   331,   709,   333,   334,   335,   336,   852,   724,
      14,   201,    96,   201,   201,   721,  1715,   206,   724,   127,
     128,   200,   104,   941,   200,   943,   200,   112,   113,   114,
     115,   116,   117,  1437,     9,   201,   201,    88,   123,   124,
       9,   202,   779,    14,   200,     9,   783,  1190,   186,    79,
     774,    79,    79,   774,   189,   200,   780,     9,   774,   202,
     784,   785,   202,     9,    79,   201,   125,  1350,   774,   201,
     201,  1096,   916,   202,   159,   425,   161,   200,  1199,   201,
     804,    66,    30,   126,   129,   435,   792,   168,     9,   201,
     885,   148,   201,   201,     9,   819,   201,   182,   819,     9,
     910,   807,   808,   819,   819,    14,   201,   819,   198,   833,
     819,   819,     9,   819,  1257,   183,   184,   185,     9,   963,
     170,   852,   190,   191,  1235,    66,   194,   195,   852,   201,
    1273,  1676,  1050,     9,  1052,    14,   125,  1057,   207,   204,
     207,     9,   952,   207,   200,   207,   201,   201,  1693,   959,
     202,   200,    96,   201,   148,   886,  1701,   129,  1076,     9,
     202,   885,    46,    47,    48,    49,    50,    51,   200,    53,
     201,   148,   148,   897,   898,   899,  1580,   200,   200,   920,
     200,    65,   203,   200,   186,   916,    46,    47,    48,    49,
      50,    51,   916,   186,    14,     9,    79,   921,   203,   923,
     203,   925,   923,    14,   925,    65,   921,   923,    14,   925,
     202,   207,   203,  1131,    14,   921,  1359,   923,   202,   925,
     944,   927,   928,    30,   198,  1368,  1337,   200,    30,   819,
     200,    14,   200,   200,   200,     9,   201,   200,   962,  1382,
     202,   202,   129,    14,   968,   112,   113,   114,   115,   116,
     117,     9,  1096,    65,   604,     9,   123,   124,   848,  1284,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1031,    53,   864,   201,   207,  1195,    79,   145,
      96,     9,  1092,   200,   129,    65,   202,  1011,    14,    79,
    1014,   203,   203,   201,   161,   200,   200,   203,   201,   129,
    1443,  1622,     9,   145,    73,   207,    30,   202,   201,   659,
     202,  1035,   170,    30,  1035,   182,   129,     9,     9,  1035,
    1035,   201,     9,  1035,   201,   129,  1035,  1035,   201,  1035,
     201,  1141,  1357,     9,   204,  1145,   201,   201,  1148,   201,
       4,   204,   203,    14,    79,  1155,   200,   203,   698,   201,
     700,   129,   201,  1059,   201,   201,   200,     9,  1276,   202,
    1278,   201,   201,    30,  1105,   201,   201,    96,   202,  1093,
    1691,   721,   203,  1097,   202,   157,   153,    14,    79,   110,
     201,    45,   201,   129,  1108,   203,    26,    27,   129,    14,
      30,   201,    14,  1108,   203,  1101,  1120,   202,    79,  1120,
    1318,    79,  1108,   201,  1120,   200,   203,   201,   129,   202,
    1116,  1117,    52,   202,  1120,    14,    14,   202,     4,    14,
      55,    79,   203,    79,   200,     9,    79,  1570,   108,  1572,
     202,    96,   148,    96,    14,   160,    33,   200,  1581,  1320,
    1284,   105,   792,  1253,   201,  1035,   110,   202,   112,   113,
     114,   115,   116,   117,   118,   200,   166,   807,   808,    45,
      79,   163,   201,     9,    79,   202,  1384,  1318,   201,   201,
    1194,    14,    79,   203,  1064,    14,  1197,  1067,    79,    14,
      14,   783,  1625,    79,   779,  1206,    79,   516,  1684,   884,
     881,   155,   156,  1199,   158,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,  1408,   395,
     392,  1697,   822,  1357,  1114,  1442,   394,  1270,   182,   105,
    1693,  1245,   521,  1247,   110,  1433,   112,   113,   114,   115,
     116,   117,   118,  1462,  1546,  1313,  1725,  1713,  1308,  1558,
     204,    41,  1429,   398,    63,    64,  1003,  1006,   771,   492,
     492,  1039,   960,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,  1080,   912,  1093,   155,
     156,   898,   158,   213,   318,   734,  1719,   927,   928,  1303,
     220,   299,  1303,  1726,  1308,   966,   226,  1303,   292,  1313,
    1022,   945,    -1,    -1,    -1,    -1,   182,  1303,    -1,  1320,
      -1,  1317,    63,    64,    -1,    -1,    -1,  1197,   127,   128,
      -1,    -1,    -1,    -1,   254,   255,  1206,    -1,   204,  1340,
     260,    -1,    -1,    -1,    -1,  1346,    -1,  1348,    -1,    -1,
      -1,  1355,    -1,    -1,    -1,    -1,  1360,    -1,    -1,  1360,
    1410,  1365,   282,  1233,  1360,  1369,    -1,    -1,  1369,    -1,
    1365,   291,   292,  1369,  1360,    -1,    -1,    -1,   298,  1365,
    1560,    -1,     4,  1369,   304,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,  1397,    -1,   315,    -1,    -1,    -1,  1403,
      -1,    -1,   201,    -1,    -1,    -1,  1410,    -1,    -1,    -1,
    1414,    -1,    -1,    -1,    -1,    -1,  1412,    -1,    -1,   339,
      -1,    -1,   342,    45,    -1,    -1,    -1,    -1,    -1,  1059,
      -1,    -1,    -1,  1437,  1435,    -1,  1440,  1441,  1442,  1440,
      -1,    -1,  1446,    -1,  1440,  1446,  1441,  1442,    -1,  1453,
    1446,    -1,  1453,    -1,  1440,  1441,  1442,  1453,    -1,    -1,
    1446,    -1,    -1,    -1,   384,    -1,    -1,  1453,    -1,    -1,
    1340,  1101,    -1,    -1,    -1,    -1,  1346,    -1,  1348,    -1,
    1350,    -1,  1557,   105,    -1,    -1,  1116,  1117,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   425,    -1,    -1,  1706,    -1,
      -1,  1672,    -1,    -1,    -1,   435,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1721,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   155,   156,    -1,   158,    -1,    -1,    -1,
      -1,  1616,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1557,   474,   475,    -1,  1688,   478,    -1,
     182,    -1,    -1,    -1,    -1,  1435,    -1,    -1,    -1,  1199,
      -1,    -1,  1576,    -1,    -1,    -1,  1580,    -1,    -1,    -1,
      -1,  1585,   204,    -1,  1585,    -1,    -1,    -1,    -1,  1585,
      -1,  1595,    -1,    -1,    -1,    -1,  1600,  1601,    -1,  1585,
    1604,  1605,    -1,    -1,    -1,   525,    -1,    -1,    -1,    -1,
      -1,    -1,  1616,    -1,    -1,    -1,    -1,    -1,    -1,  1623,
    1624,    -1,  1623,  1624,    -1,    -1,  1630,  1623,  1624,  1630,
      -1,    -1,    -1,    -1,  1630,    -1,    -1,  1623,  1624,    -1,
      -1,    -1,    -1,    -1,  1630,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   576,   577,    -1,    -1,
    1664,    -1,    -1,  1664,   584,    -1,    -1,  1671,  1664,    -1,
      -1,  1672,    10,    11,    12,    -1,    -1,    -1,  1664,    -1,
      -1,    -1,    -1,  1687,   604,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1727,    -1,    -1,  1727,    65,    -1,    -1,
    1734,  1727,    -1,  1734,    10,    11,    12,    -1,  1734,   659,
      -1,  1727,    -1,    -1,    -1,    -1,    -1,    -1,  1734,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   698,    -1,
     700,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   721,   722,    -1,    -1,    -1,  1676,    -1,    -1,    -1,
      -1,    10,    11,    12,   734,   735,   736,   737,   738,   739,
     740,    -1,    -1,  1693,    -1,    -1,    -1,    -1,    -1,   749,
      29,  1701,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,   775,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,   207,
      -1,    -1,   792,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   805,    -1,   807,   808,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   821,   822,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     830,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,   844,    -1,    -1,    -1,   204,    -1,
      -1,    -1,    -1,    -1,    -1,   855,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   863,    -1,    -1,   866,    26,    27,    -1,
      -1,    30,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   886,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   155,   156,
      -1,   158,   159,   160,    -1,   204,    -1,   927,   928,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   941,    -1,   943,    -1,   945,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     960,    -1,    -1,   963,   964,   965,   966,    -1,    -1,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,    77,    -1,    79,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1019,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1050,    -1,  1052,    -1,    65,    -1,    -1,    -1,    -1,  1059,
      -1,   220,    -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,
      -1,    -1,    -1,    -1,   155,   156,  1076,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1096,    -1,    -1,    -1,
      -1,  1101,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,  1116,  1117,    -1,  1119,
      -1,    -1,   203,   282,   205,    -1,    -1,    -1,    -1,    -1,
      -1,  1131,   291,   292,  1134,    -1,  1136,    -1,    -1,   298,
      -1,    -1,    10,    11,    12,   304,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   315,    -1,    -1,  1159,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   204,    -1,  1195,  1196,    65,    -1,  1199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1224,   384,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    -1,    65,    -1,    -1,    -1,   425,    -1,    29,    74,
      75,    76,    -1,    -1,    -1,    -1,  1276,    -1,  1278,    -1,
      85,    -1,    -1,  1283,  1284,    -1,    -1,    -1,  1288,    -1,
    1290,    -1,  1292,    -1,    55,  1295,    -1,  1297,    -1,    -1,
    1300,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1308,  1309,
      -1,  1311,    -1,    -1,    -1,    -1,    77,    -1,  1318,   478,
      -1,    -1,    -1,   127,   128,    -1,    -1,    -1,   133,   134,
     135,   136,   137,    -1,  1334,    -1,   204,    -1,    -1,   144,
      -1,    -1,    -1,   104,    -1,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1357,    -1,   164,
      -1,    -1,    -1,    -1,    -1,    -1,   525,    -1,    -1,    -1,
     131,   132,  1372,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1384,  1385,  1386,    -1,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1410,    -1,   173,    -1,  1414,    -1,    -1,    -1,    -1,    -1,
      -1,  1421,    52,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
      -1,    -1,    10,    11,    12,   604,    -1,    -1,    -1,  1449,
    1450,  1451,    -1,    -1,    -1,    -1,    -1,  1457,  1458,    -1,
      -1,    29,  1462,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
     659,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   698,
      -1,   700,    65,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   721,   722,    -1,  1565,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,  1576,   736,   737,   738,
     739,   740,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     749,    -1,    -1,    -1,    -1,  1595,    -1,    -1,    -1,  1599,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1610,    -1,    -1,    -1,    -1,  1615,   775,    -1,  1618,    -1,
      -1,    -1,    -1,    -1,   254,   255,    -1,    -1,    -1,    -1,
     260,    -1,    -1,   792,    -1,    -1,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   805,    -1,   807,   808,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   822,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   830,    -1,    -1,    -1,  1675,    -1,    -1,    -1,    -1,
      -1,   204,    -1,  1683,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1697,    -1,    -1,
      -1,    -1,    -1,    -1,   863,    -1,  1706,   866,    -1,   339,
      -1,    -1,   342,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1721,    -1,    -1,    -1,    -1,    -1,   886,    -1,    -1,
      -1,  1731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1739,
      -1,    -1,    -1,    -1,    -1,  1745,    -1,    -1,  1748,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   927,   928,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,   960,    -1,    -1,   963,   964,   965,   966,    -1,    65,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   474,   475,    -1,    -1,   478,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1019,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   525,    11,    12,    -1,    -1,
    1059,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1096,    53,    -1,
      -1,    -1,  1101,    -1,    -1,    -1,   576,   577,   204,    -1,
      65,    -1,    -1,    -1,   584,    -1,    -1,  1116,  1117,    -1,
    1119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1134,    -1,  1136,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
    1159,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,  1196,    -1,    -1,
    1199,   202,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1224,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,   722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,   734,   735,   736,   737,   738,   739,
     740,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   749,
      -1,    -1,    -1,    -1,  1283,  1284,    -1,    -1,    -1,  1288,
      -1,  1290,    -1,  1292,    -1,    -1,  1295,    -1,  1297,    -1,
      -1,  1300,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1309,    -1,  1311,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   202,    53,    -1,  1334,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,   821,    26,    27,    -1,    -1,    30,    -1,  1357,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1372,   844,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   855,  1385,  1386,    -1,    -1,
      -1,    -1,    -1,   863,    -1,    -1,    29,   202,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,  1421,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    65,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
    1449,  1450,  1451,    -1,    -1,    -1,    55,    -1,  1457,  1458,
      -1,    -1,    65,  1462,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   941,    -1,   943,    -1,   945,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     960,    -1,    -1,    -1,   964,   965,   966,    -1,    -1,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   213,
      -1,    -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,  1019,
     149,    -1,   226,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,  1565,    -1,    -1,   202,
      -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,
    1050,    -1,  1052,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
    1599,   200,    -1,    -1,    77,   204,  1076,    -1,   282,    -1,
      -1,  1610,    -1,    -1,    -1,    -1,  1615,   291,   292,  1618,
      -1,    -1,    -1,    -1,   298,    -1,    -1,    -1,    -1,    -1,
     304,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   315,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1131,    -1,    -1,  1134,    -1,  1136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1675,    -1,    -1,   152,
      -1,    -1,   155,   156,  1683,   158,   159,   160,    -1,  1159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1697,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     384,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,  1195,    -1,    -1,    -1,    -1,
     203,    -1,  1731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1739,    10,    11,    12,    -1,    -1,  1745,    -1,    -1,  1748,
      -1,   425,    -1,    -1,  1224,    -1,    -1,    -1,    -1,    -1,
      29,   435,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1276,    -1,  1278,    -1,
      77,    -1,    79,  1283,    -1,    -1,    -1,    -1,  1288,    -1,
    1290,    -1,  1292,    -1,    -1,  1295,    -1,  1297,    -1,    -1,
    1300,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1308,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,  1318,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1334,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,  1372,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1384,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     604,    -1,    -1,   202,    -1,    -1,   203,    -1,   205,    -1,
    1410,    -1,    -1,    -1,  1414,    -1,    -1,    -1,    -1,    -1,
      -1,  1421,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    28,    29,    -1,    -1,  1449,
    1450,  1451,    10,    11,    12,   659,    -1,  1457,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    -1,    -1,
      53,    29,    55,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   698,    53,   700,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   202,    65,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,   721,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1565,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1576,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   792,    -1,
      -1,    -1,    -1,    -1,    -1,  1595,    -1,    -1,    -1,  1599,
      -1,    -1,    -1,   807,   808,    -1,    -1,   190,    -1,    -1,
    1610,    -1,    -1,    -1,    -1,  1615,    -1,    -1,  1618,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   830,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   201,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   235,    -1,    -1,   238,    65,    -1,    -1,    -1,
      -1,    -1,   245,   246,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1675,    -1,    -1,    -1,    -1,
      -1,    -1,   886,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   478,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,  1706,    -1,    -1,    -1,
     293,    -1,    -1,    -1,    -1,    -1,   299,    -1,    -1,    -1,
     303,  1721,    -1,   927,   928,    -1,    -1,    -1,    -1,    -1,
      -1,  1731,    -1,   316,   317,   318,    -1,    -1,    -1,  1739,
     525,    -1,    -1,    -1,    -1,  1745,    -1,   330,  1748,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   341,   963,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,    -1,   385,    -1,   387,   388,    -1,   390,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,    -1,    -1,
      -1,    -1,    -1,   416,   417,    -1,   419,   420,   421,   422,
      -1,    -1,    -1,    -1,    -1,   428,    -1,    -1,   431,    -1,
      -1,    -1,    -1,    -1,    -1,  1059,    -1,    -1,   441,    -1,
     443,    -1,    -1,    -1,    -1,    -1,   449,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   457,    -1,   459,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   478,    -1,    -1,    -1,    -1,
      -1,    -1,  1096,    -1,    29,    -1,    -1,  1101,    -1,    -1,
      -1,    -1,   485,    -1,    -1,   488,   489,   490,    -1,    -1,
      -1,    -1,  1116,  1117,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   525,    -1,    -1,    -1,   519,   722,    -1,    -1,
      -1,    -1,    77,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   736,   737,   738,   739,   740,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   749,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1199,   131,   132,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,
     155,   156,   605,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   617,    -1,    -1,    -1,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,   649,    -1,    -1,   204,
      -1,    -1,    -1,    -1,    -1,   658,    -1,    -1,   863,    -1,
    1284,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   676,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,   722,
      -1,    -1,    -1,    -1,    -1,   201,   719,    65,    -1,    -1,
      -1,    -1,    -1,   736,   737,   738,   739,   740,    -1,    -1,
      -1,    -1,    -1,  1357,    -1,    -1,   749,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   960,    -1,   760,    -1,   964,
     965,   966,    -1,    -1,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,   816,  1019,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   829,    77,    -1,    -1,
      -1,    -1,   835,    -1,   837,    -1,   839,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,
     863,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   861,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,   871,    -1,
      -1,   874,    -1,   876,    -1,    -1,    -1,   880,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,   155,   156,    -1,   158,   159,
     160,    -1,   915,    -1,  1119,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1134,
      -1,  1136,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   960,    -1,    -1,
     200,   964,   965,   966,  1159,    -1,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     478,    -1,    -1,    -1,   997,   998,   999,    -1,    -1,    -1,
    1003,  1004,    -1,    -1,    -1,    -1,  1019,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1224,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,  1031,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   525,    -1,    -1,
      -1,    -1,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1060,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1073,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1283,  1082,
    1083,    -1,    -1,  1288,    -1,  1290,    -1,  1292,    -1,    -1,
    1295,    -1,  1297,    -1,    -1,  1300,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,   155,   156,  1119,   158,   159,   160,
    1113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1123,  1134,    -1,  1136,    -1,    -1,    -1,    -1,    -1,  1334,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,  1159,    10,    11,    12,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1372,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,  1209,    -1,    -1,    -1,
    1213,  1224,  1215,  1216,    -1,    -1,  1421,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1234,  1235,    -1,   722,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1449,  1450,  1451,    55,   736,   737,
     738,   739,  1457,    -1,    -1,    -1,  1461,    -1,    -1,    -1,
      -1,   749,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
    1283,    -1,    -1,    -1,    -1,  1288,    -1,  1290,    -1,  1292,
      -1,    -1,  1295,    -1,  1297,    -1,    -1,  1300,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,  1301,    -1,
      -1,    29,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1334,    -1,   131,   132,    -1,   189,    55,    -1,    -1,
      -1,    -1,    -1,    -1,  1337,    -1,    -1,    -1,    -1,    -1,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    77,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,  1372,
    1565,    -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   863,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    -1,  1599,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,  1610,    -1,    -1,  1421,    -1,
    1615,    -1,    -1,  1618,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,   162,  1640,  1449,  1450,  1451,    -1,
      -1,    -1,    -1,    -1,  1457,   173,    -1,    -1,    -1,  1452,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
    1675,    -1,   200,    -1,    -1,    -1,   964,   965,   966,    -1,
      -1,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1731,    -1,    -1,    -1,
      -1,  1019,    -1,    -1,  1739,    -1,    10,    11,    12,    -1,
    1745,    -1,    -1,  1748,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1565,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,   155,
     156,    65,   158,   159,   160,    -1,    -1,  1610,    -1,    -1,
      -1,    -1,  1615,    -1,    -1,  1618,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,  1119,    -1,    -1,    -1,    -1,    -1,   203,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,  1134,    -1,  1136,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1675,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,  1159,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,     3,
       4,     5,     6,     7,   188,    -1,    -1,    -1,  1731,    13,
      -1,    -1,    -1,    -1,    -1,    -1,  1739,    -1,    -1,    -1,
      -1,    -1,  1745,    27,    28,  1748,  1224,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,  1283,    -1,    -1,    -1,    -1,
    1288,    -1,  1290,    -1,  1292,    -1,    -1,  1295,    -1,  1297,
     104,    -1,  1300,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,  1334,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    29,   173,
     174,   175,   176,    -1,  1372,    -1,    -1,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    55,    -1,   200,    -1,    -1,    -1,
      -1,   205,   206,    -1,   208,   209,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,  1421,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1449,  1450,  1451,    -1,    -1,    -1,    -1,    -1,  1457,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,   173,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,  1565,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1599,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,  1610,    -1,    -1,    -1,    -1,  1615,    -1,    -1,
    1618,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,  1675,    -1,    -1,
     101,   102,   103,   104,   105,   106,   107,    -1,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,  1731,   155,   156,    -1,   158,   159,   160,
     161,  1739,    -1,   164,    -1,    -1,   167,  1745,    -1,    -1,
    1748,    -1,   173,   174,   175,   176,   177,    -1,   179,    -1,
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
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,   161,    -1,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,   177,    -1,   179,    -1,   181,   182,    -1,
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
      97,    -1,    -1,    -1,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,   110,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
     177,    -1,   179,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,
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
     196,   197,    -1,    -1,   200,    -1,   202,   203,   204,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
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
      -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    95,    -1,    97,    -1,    -1,    -1,   101,
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
     202,   203,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
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
      -1,    92,    93,    94,    -1,    -1,    97,    -1,    -1,    -1,
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
      90,    91,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
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
     200,    -1,   202,   203,    -1,   205,   206,    -1,   208,   209,
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
     196,   197,    -1,    -1,   200,    -1,   202,   203,   204,   205,
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
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
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
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,    -1,   205,   206,    -1,   208,   209,
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
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
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
     159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    11,    12,   203,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,
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
     162,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   101,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    12,    -1,    -1,   205,   206,    -1,   208,   209,
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
     153,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,   164,    -1,    -1,   167,     3,     4,     5,     6,     7,
     173,   174,   175,   176,    -1,    13,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,    -1,    46,    47,
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
       3,     4,     5,     6,     7,   173,   174,   175,   176,    -1,
      13,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,    -1,    -1,   205,   206,    -1,
     208,   209,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
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
      -1,   164,    -1,    -1,   167,     3,     4,     5,     6,     7,
     173,   174,   175,   176,    -1,    13,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
      -1,    -1,   205,   206,    -1,   208,   209,    -1,    46,    47,
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
      -1,    -1,   200,   201,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    30,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,
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
      -1,    12,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
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
     167,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    31,    32,    33,    34,    35,    36,
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
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
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
     153,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,   164,    -1,    -1,   167,     3,     4,     5,     6,     7,
     173,   174,   175,   176,    -1,    13,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,    -1,    46,    47,
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
       3,     4,     5,     6,     7,   173,   174,   175,   176,    -1,
      13,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
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
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    29,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   149,   150,   151,    -1,    55,    -1,   155,
     156,    -1,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,    -1,    -1,    -1,    -1,    -1,    -1,   173,    -1,    77,
      -1,   177,    -1,   179,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,   131,   132,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    -1,    -1,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    66,
     158,   159,   160,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,   173,    -1,    35,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    77,
      -1,    79,    -1,    -1,   131,    -1,   133,   134,   135,   136,
     137,    -1,    65,    -1,    -1,    -1,    -1,   144,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,    -1,   130,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,   149,    -1,   200,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      46,    47,   200,    -1,    -1,   117,    52,   205,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,   149,    -1,    85,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   131,    -1,   133,   134,   135,
     136,   137,    -1,   205,    -1,    46,    47,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    66,    -1,    -1,   164,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,   173,   174,   175,
     176,    -1,    -1,    -1,    85,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
     131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,   155,   156,    -1,   158,   159,   160,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,    67,
      68,    -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,   149,    -1,    -1,
     152,   153,    77,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      77,   149,    -1,   205,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    77,    -1,    79,    -1,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,
     155,   156,   157,   158,   159,   160,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,   117,    -1,    -1,    -1,   205,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,   153,    77,   155,   156,
      -1,   158,   159,   160,    -1,   149,    -1,    -1,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,   200,   117,    -1,   203,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,    -1,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    77,    -1,    79,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
     200,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
     200,   117,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    77,    -1,    79,   200,    -1,    -1,   149,    -1,   205,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,   155,   156,   200,   158,
     159,   160,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    10,    11,    12,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
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
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,   129,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    29,   129,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   129,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    29,   129,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    85,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    29,   129,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    65,    -1,
      -1,    -1,    77,    -1,    -1,   129,   149,    -1,    -1,   152,
     131,   132,   155,   156,    -1,   158,   159,   160,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,   104,
     105,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    77,   152,    79,    80,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    77,
     149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    77,   152,    -1,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,   123,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      28,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    96,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    35,    36,    37,    38,    39,    40,
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
     440,   441,   442,   443,   445,   466,   468,   470,   115,   116,
     117,   130,   149,   159,   217,   248,   325,   341,   434,   341,
     200,   341,   341,   341,   101,   341,   341,   424,   425,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,    79,   117,   200,   225,   399,   400,   434,   437,   434,
      35,   341,   449,   450,   341,   117,   200,   225,   399,   400,
     401,   433,   441,   446,   447,   200,   332,   402,   200,   332,
     348,   333,   341,   234,   332,   200,   200,   200,   332,   202,
     341,   217,   202,   341,    29,    55,   131,   132,   153,   173,
     200,   217,   228,   471,   483,   484,   183,   202,   338,   341,
     368,   370,   203,   241,   341,   104,   105,   152,   218,   221,
     224,    79,   205,   293,   294,   116,   123,   115,   123,    79,
     295,   200,   200,   200,   200,   217,   265,   472,   200,   200,
      79,    84,   145,   146,   147,   463,   464,   152,   203,   224,
     224,   217,   266,   472,   153,   200,   200,   200,   200,   472,
     472,    79,   197,   350,   331,   341,   342,   434,   438,   230,
     203,    84,   403,   463,    84,   463,   463,    30,   152,   169,
     473,   200,     9,   202,    35,   247,   153,   264,   472,   117,
     182,   248,   326,   202,   202,   202,   202,   202,   202,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    53,    65,   202,    66,    66,   202,
     203,   148,   124,   159,   161,   267,   324,   325,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    63,    64,   127,   128,   428,    66,   203,   431,   200,
     200,    66,   203,   205,   442,   200,   247,   248,    14,   341,
     202,   129,    44,   217,   423,   200,   331,   434,   438,   148,
     434,   129,   207,     9,   410,   331,   434,   473,   148,   200,
     404,   428,   431,   201,   341,    30,   232,     8,   352,     9,
     202,   232,   233,   333,   334,   341,   217,   279,   236,   202,
     202,   202,   484,   484,   169,   200,   104,   484,    14,   217,
      79,   202,   202,   202,   183,   184,   185,   190,   191,   194,
     195,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     383,   384,   385,   242,   108,   166,   202,   152,   219,   222,
     224,   152,   220,   223,   224,   224,     9,   202,    96,   203,
     434,     9,   202,   123,   123,    14,     9,   202,   434,   467,
     467,   331,   342,   434,   437,   438,   201,   169,   259,   130,
     434,   448,   449,    66,   428,   145,   464,    78,   341,   434,
      84,   145,   464,   224,   216,   202,   203,   254,   262,   389,
     391,    85,   205,   353,   354,   356,   400,   442,   468,   341,
     456,   458,   341,   455,   457,   455,    14,    96,   469,   349,
     351,   289,   290,   426,   427,   201,   201,   201,   201,   204,
     231,   232,   249,   256,   261,   426,   341,   206,   208,   209,
     217,   474,   475,   484,    35,   162,   291,   292,   341,   471,
     200,   472,   257,   247,   341,   341,   341,    30,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     401,   341,   341,   444,   444,   341,   451,   452,   123,   203,
     217,   441,   442,   265,   217,   266,   264,   248,    27,    35,
     335,   338,   341,   368,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   203,   217,   429,   430,
     441,   444,   341,   291,   291,   444,   341,   448,   247,   201,
     341,   200,   422,     9,   410,   331,   201,   217,    35,   341,
      35,   341,   201,   201,   441,   291,   429,   430,   201,   230,
     283,   203,   338,   341,   341,    88,    30,   232,   277,   202,
      28,    96,    14,     9,   201,    30,   203,   280,   484,    85,
     228,   480,   481,   482,   200,     9,    46,    47,    52,    54,
      66,    85,   131,   144,   153,   173,   174,   175,   176,   200,
     225,   226,   228,   363,   364,   365,   399,   405,   406,   407,
     186,    79,   341,    79,    79,   341,   380,   381,   341,   341,
     373,   383,   189,   386,   230,   200,   240,   224,   202,     9,
      96,   224,   202,     9,    96,    96,   221,   217,   341,   294,
     406,    79,     9,   201,   201,   201,   201,   201,   201,   201,
     202,    46,    47,   478,   479,   125,   270,   200,     9,   201,
     201,    79,    80,   217,   465,   217,    66,   204,   204,   213,
     215,    30,   126,   269,   168,    50,   153,   168,   393,   129,
       9,   410,   201,   148,   201,     9,   410,   129,   201,     9,
     410,   201,   484,   484,    14,   352,   289,   198,     9,   411,
     484,   485,   428,   431,   204,     9,   410,   170,   434,   341,
     201,     9,   411,    14,   345,   250,   125,   268,   200,   472,
     341,    30,   207,   207,   129,   204,     9,   410,   341,   473,
     200,   260,   255,   263,   258,   247,    68,   434,   341,   473,
     207,   204,   201,   201,   207,   204,   201,    46,    47,    66,
      74,    75,    76,    85,   131,   144,   173,   217,   413,   415,
     418,   421,   217,   434,   434,   129,   428,   431,   201,   284,
      71,    72,   285,   230,   332,   230,   334,    96,    35,   130,
     274,   434,   406,   217,    30,   232,   278,   202,   281,   202,
     281,     9,   170,   129,   148,     9,   410,   201,   162,   474,
     475,   476,   474,   406,   406,   406,   406,   406,   409,   412,
     200,    84,   148,   200,   200,   200,   200,   406,   148,   203,
      10,    11,    12,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    65,   341,   186,   186,    14,
     192,   193,   382,     9,   196,   386,    79,   204,   399,   203,
     244,    96,   222,   217,    96,   223,   217,   217,   204,    14,
     434,   202,     9,   170,   217,   271,   399,   203,   448,   130,
     434,    14,   207,   341,   204,   213,   484,   271,   203,   392,
      14,   341,   353,   217,   341,   341,   341,   202,   484,   198,
      30,   477,   427,    35,    79,   162,   429,   430,   484,    35,
     162,   341,   406,   289,   200,   399,   269,   346,   251,   341,
     341,   341,   204,   200,   291,   270,    30,   269,   268,   472,
     401,   204,   200,    14,    74,    75,    76,   217,   414,   414,
     415,   416,   417,   200,    84,   145,   200,     9,   410,   201,
     422,    35,   341,   429,   430,    71,    72,   286,   332,   232,
     204,   202,    89,   202,   274,   434,   200,   129,   273,    14,
     230,   281,    98,    99,   100,   281,   204,   484,   484,   217,
     480,     9,   201,   410,   129,   207,     9,   410,   409,   217,
     353,   355,   357,   406,   460,   462,   406,   459,   461,   459,
     201,   123,   217,   406,   453,   454,   406,   406,   406,    30,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   341,   341,   341,   381,   341,   371,    79,
     245,   217,   217,   406,   479,    96,     9,   299,   201,   200,
     335,   338,   341,   207,   204,   469,   299,   154,   167,   203,
     388,   395,   154,   203,   394,   129,   129,   202,   477,   484,
     352,   485,    79,   162,    14,    79,   473,   434,   341,   201,
     289,   203,   289,   200,   129,   200,   291,   201,   203,   484,
     203,   269,   252,   404,   291,   129,   207,     9,   410,   416,
     145,   353,   419,   420,   415,   434,   332,    30,    73,   232,
     202,   334,   273,   448,   274,   201,   406,    95,    98,   202,
     341,    30,   202,   282,   204,   170,   129,   162,    30,   201,
     406,   406,   201,   129,     9,   410,   201,   201,     9,   410,
     129,   201,     9,   410,   201,   129,   204,     9,   410,   406,
      30,   187,   201,   230,   217,   484,   399,     4,   105,   110,
     118,   155,   156,   158,   204,   300,   323,   324,   325,   330,
     426,   448,   204,   203,   204,    50,   341,   341,   341,   341,
     352,    35,    79,   162,    14,    79,   406,   200,   477,   201,
     299,   201,   289,   341,   291,   201,   299,   469,   299,   203,
     200,   201,   415,   415,   201,   129,   201,     9,   410,    30,
     230,   202,   201,   201,   201,   237,   202,   202,   282,   230,
     484,   484,   129,   406,   353,   406,   406,   406,   406,   406,
     406,   341,   203,   204,    96,   125,   126,   471,   272,   399,
     118,   131,   132,   153,   159,   309,   310,   311,   399,   157,
     315,   316,   121,   200,   217,   317,   318,   301,   248,   484,
       9,   202,   324,   201,   296,   153,   390,   204,   204,    79,
     162,    14,    79,   406,   291,   110,   343,   477,   204,   477,
     201,   201,   204,   203,   204,   299,   289,   129,   415,   353,
     230,   235,   238,    30,   232,   276,   230,   201,   406,   129,
     129,   129,   188,   230,   484,   399,   399,    14,     9,   202,
     203,   203,     9,   202,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    53,    67,    68,    69,    70,
      71,    72,    73,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   130,   131,   133,   134,   135,   136,
     137,   149,   150,   151,   161,   163,   164,   166,   173,   177,
     179,   181,   182,   217,   396,   397,     9,   202,   153,   157,
     217,   318,   319,   320,   202,    79,   329,   247,   302,   471,
     248,   204,   297,   298,   471,    14,    79,   406,   201,   200,
     203,   202,   203,   321,   343,   477,   296,   204,   201,   415,
     129,    30,   232,   275,   276,   230,   406,   406,   406,   341,
     204,   202,   202,   406,   399,   305,   312,   405,   310,    14,
      30,    47,   313,   316,     9,    33,   201,    29,    46,    49,
      14,     9,   202,   472,   329,    14,   247,   202,    14,   406,
      35,    79,   387,   230,   230,   203,   321,   204,   477,   415,
     230,    93,   189,   243,   204,   217,   228,   306,   307,   308,
       9,   204,   406,   397,   397,    55,   314,   319,   319,    29,
      46,    49,   406,    79,   200,   202,   406,   472,   406,    79,
       9,   411,   204,   204,   230,   321,    91,   202,    79,   108,
     239,   148,    96,   405,   160,    14,   303,   200,    35,    79,
     201,   204,   202,   200,   166,   246,   217,   324,   325,   406,
     287,   288,   427,   304,    79,   399,   244,   163,   217,   202,
     201,     9,   411,   112,   113,   114,   327,   328,   287,    79,
     272,   202,   477,   427,   485,   201,   201,   202,   202,   203,
     322,   327,    35,    79,   162,   477,   203,   230,   485,    79,
     162,    14,    79,   322,   230,   204,    35,    79,   162,    14,
      79,   406,   204,    79,   162,    14,    79,   406,    14,    79,
     406,   406
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
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
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
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval).reset();;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
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
#line 1897 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
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
#line 1913 "hphp.y"
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

  case 451:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
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

  case 455:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
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

  case 524:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
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

  case 525:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval).reset();;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval).reset();;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval).reset();;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval).reset();;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval).reset();;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval).reset();;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval).reset();;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { (yyval).reset();;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval).reset();;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { (yyval)++;;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval).reset();;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    {;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    {;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    {;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    {;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13663 "hphp.tab.cpp"
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
#line 3064 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

