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
#define YYLAST   16409

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  274
/* YYNRULES -- Number of rules.  */
#define YYNRULES  925
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1735

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
     694,   695,   699,   700,   708,   717,   724,   727,   733,   740,
     745,   746,   751,   757,   765,   772,   779,   787,   797,   806,
     813,   821,   827,   830,   835,   841,   845,   846,   850,   855,
     862,   868,   874,   881,   890,   898,   901,   902,   904,   907,
     910,   914,   919,   924,   928,   930,   932,   935,   940,   944,
     950,   952,   956,   959,   960,   963,   967,   970,   971,   972,
     977,   978,   984,   987,   988,   999,  1000,  1012,  1016,  1020,
    1024,  1029,  1034,  1038,  1044,  1047,  1050,  1051,  1058,  1064,
    1069,  1073,  1075,  1077,  1081,  1086,  1088,  1090,  1092,  1094,
    1099,  1101,  1103,  1107,  1110,  1111,  1114,  1115,  1117,  1121,
    1123,  1125,  1127,  1129,  1133,  1138,  1143,  1148,  1150,  1152,
    1155,  1158,  1161,  1165,  1169,  1171,  1173,  1175,  1177,  1181,
    1183,  1187,  1189,  1191,  1193,  1194,  1196,  1199,  1201,  1203,
    1205,  1207,  1209,  1211,  1213,  1215,  1216,  1218,  1220,  1222,
    1226,  1232,  1234,  1238,  1244,  1249,  1253,  1257,  1260,  1262,
    1264,  1268,  1272,  1274,  1276,  1277,  1279,  1282,  1287,  1291,
    1298,  1301,  1305,  1312,  1314,  1316,  1318,  1325,  1329,  1334,
    1341,  1345,  1349,  1353,  1357,  1361,  1365,  1369,  1373,  1377,
    1381,  1385,  1389,  1392,  1395,  1398,  1401,  1405,  1409,  1413,
    1417,  1421,  1425,  1429,  1433,  1437,  1441,  1445,  1449,  1453,
    1457,  1461,  1465,  1469,  1472,  1475,  1478,  1481,  1485,  1489,
    1493,  1497,  1501,  1505,  1509,  1513,  1517,  1521,  1527,  1532,
    1534,  1537,  1540,  1543,  1546,  1549,  1552,  1555,  1558,  1561,
    1563,  1565,  1567,  1569,  1571,  1575,  1578,  1580,  1582,  1584,
    1590,  1591,  1592,  1604,  1605,  1618,  1619,  1623,  1624,  1629,
    1630,  1637,  1638,  1646,  1649,  1652,  1657,  1659,  1661,  1667,
    1671,  1677,  1681,  1684,  1685,  1688,  1689,  1694,  1699,  1703,
    1708,  1713,  1718,  1723,  1728,  1733,  1738,  1743,  1748,  1753,
    1755,  1757,  1759,  1763,  1766,  1770,  1775,  1778,  1782,  1784,
    1787,  1789,  1792,  1794,  1796,  1798,  1800,  1802,  1804,  1809,
    1814,  1817,  1826,  1837,  1840,  1842,  1846,  1848,  1851,  1853,
    1855,  1857,  1859,  1862,  1867,  1871,  1875,  1880,  1882,  1885,
    1890,  1893,  1900,  1901,  1903,  1908,  1909,  1912,  1913,  1915,
    1917,  1921,  1923,  1927,  1929,  1931,  1935,  1939,  1941,  1943,
    1945,  1947,  1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,
    1965,  1967,  1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,
    1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,
    2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,
    2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,
    2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,
    2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,
    2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2104,  2106,
    2108,  2110,  2112,  2114,  2116,  2118,  2120,  2123,  2125,  2126,
    2127,  2129,  2131,  2135,  2136,  2138,  2140,  2142,  2144,  2146,
    2148,  2150,  2152,  2154,  2156,  2158,  2160,  2162,  2166,  2169,
    2171,  2173,  2178,  2182,  2187,  2189,  2191,  2193,  2195,  2199,
    2203,  2207,  2211,  2215,  2219,  2223,  2227,  2231,  2235,  2239,
    2243,  2247,  2251,  2255,  2259,  2263,  2267,  2270,  2273,  2276,
    2279,  2283,  2287,  2291,  2295,  2299,  2303,  2307,  2311,  2317,
    2322,  2326,  2330,  2334,  2336,  2338,  2340,  2342,  2346,  2350,
    2354,  2357,  2358,  2360,  2361,  2363,  2364,  2370,  2374,  2378,
    2380,  2382,  2384,  2386,  2388,  2392,  2395,  2397,  2399,  2401,
    2403,  2405,  2407,  2410,  2413,  2418,  2422,  2427,  2430,  2431,
    2437,  2441,  2445,  2447,  2451,  2453,  2456,  2457,  2463,  2467,
    2470,  2471,  2475,  2476,  2481,  2484,  2485,  2489,  2493,  2495,
    2496,  2498,  2500,  2502,  2506,  2508,  2510,  2514,  2518,  2521,
    2526,  2529,  2534,  2536,  2538,  2540,  2542,  2544,  2548,  2554,
    2558,  2563,  2567,  2569,  2571,  2573,  2575,  2579,  2585,  2590,
    2594,  2596,  2598,  2602,  2610,  2620,  2628,  2635,  2644,  2646,
    2649,  2654,  2659,  2661,  2663,  2668,  2670,  2671,  2673,  2676,
    2678,  2680,  2684,  2690,  2694,  2698,  2699,  2701,  2705,  2711,
    2715,  2718,  2722,  2729,  2730,  2732,  2737,  2740,  2741,  2747,
    2751,  2755,  2757,  2764,  2769,  2774,  2777,  2780,  2781,  2787,
    2791,  2795,  2797,  2800,  2801,  2807,  2811,  2815,  2817,  2820,
    2821,  2824,  2825,  2831,  2835,  2839,  2841,  2844,  2845,  2848,
    2849,  2855,  2859,  2863,  2865,  2868,  2871,  2873,  2876,  2878,
    2883,  2887,  2891,  2898,  2902,  2904,  2906,  2908,  2913,  2918,
    2923,  2928,  2931,  2934,  2939,  2942,  2945,  2947,  2951,  2955,
    2959,  2960,  2963,  2969,  2976,  2978,  2981,  2983,  2988,  2992,
    2993,  2995,  2999,  3002,  3006,  3008,  3010,  3011,  3012,  3015,
    3020,  3023,  3030,  3035,  3037,  3039,  3040,  3044,  3050,  3054,
    3056,  3059,  3060,  3065,  3068,  3071,  3073,  3075,  3077,  3079,
    3084,  3091,  3093,  3102,  3109,  3111
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
     427,   327,   483,    35,   162,    79,    -1,   288,     9,   427,
     327,   483,   162,    -1,   288,   411,    -1,   427,   327,   483,
     162,    79,    -1,   427,   327,   483,    35,   162,    79,    -1,
     427,   327,   483,   162,    -1,    -1,   427,   327,   483,    79,
      -1,   427,   327,   483,    35,    79,    -1,   427,   327,   483,
      35,    79,    14,   406,    -1,   427,   327,   483,    79,    14,
     406,    -1,   288,     9,   427,   327,   483,    79,    -1,   288,
       9,   427,   327,   483,    35,    79,    -1,   288,     9,   427,
     327,   483,    35,    79,    14,   406,    -1,   288,     9,   427,
     327,   483,    79,    14,   406,    -1,   290,     9,   427,   483,
     162,    79,    -1,   290,     9,   427,   483,    35,   162,    79,
      -1,   290,     9,   427,   483,   162,    -1,   290,   411,    -1,
     427,   483,   162,    79,    -1,   427,   483,    35,   162,    79,
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
     341,    -1,   341,    -1,   333,    -1,    -1,    27,    -1,    27,
     341,    -1,    27,   341,   129,   341,    -1,   434,    14,   335,
      -1,   130,   200,   446,   201,    14,   335,    -1,    28,   341,
      -1,   434,    14,   338,    -1,   130,   200,   446,   201,    14,
     338,    -1,   342,    -1,   434,    -1,   331,    -1,   130,   200,
     446,   201,    14,   341,    -1,   434,    14,   341,    -1,   434,
      14,    35,   434,    -1,   434,    14,    35,    68,   401,   404,
      -1,   434,    26,   341,    -1,   434,    25,   341,    -1,   434,
      24,   341,    -1,   434,    23,   341,    -1,   434,    22,   341,
      -1,   434,    21,   341,    -1,   434,    20,   341,    -1,   434,
      19,   341,    -1,   434,    18,   341,    -1,   434,    17,   341,
      -1,   434,    16,   341,    -1,   434,    15,   341,    -1,   434,
      64,    -1,    64,   434,    -1,   434,    63,    -1,    63,   434,
      -1,   341,    31,   341,    -1,   341,    32,   341,    -1,   341,
      10,   341,    -1,   341,    12,   341,    -1,   341,    11,   341,
      -1,   341,    33,   341,    -1,   341,    35,   341,    -1,   341,
      34,   341,    -1,   341,    48,   341,    -1,   341,    46,   341,
      -1,   341,    47,   341,    -1,   341,    49,   341,    -1,   341,
      50,   341,    -1,   341,    65,   341,    -1,   341,    51,   341,
      -1,   341,    45,   341,    -1,   341,    44,   341,    -1,    46,
     341,    -1,    47,   341,    -1,    52,   341,    -1,    54,   341,
      -1,   341,    37,   341,    -1,   341,    36,   341,    -1,   341,
      39,   341,    -1,   341,    38,   341,    -1,   341,    40,   341,
      -1,   341,    43,   341,    -1,   341,    41,   341,    -1,   341,
      42,   341,    -1,   341,    53,   401,    -1,   200,   342,   201,
      -1,   341,    29,   341,    30,   341,    -1,   341,    29,    30,
     341,    -1,   464,    -1,    62,   341,    -1,    61,   341,    -1,
      60,   341,    -1,    59,   341,    -1,    58,   341,    -1,    57,
     341,    -1,    56,   341,    -1,    69,   402,    -1,    55,   341,
      -1,   408,    -1,   359,    -1,   358,    -1,   361,    -1,   362,
      -1,   206,   403,   206,    -1,    13,   341,    -1,   344,    -1,
     347,    -1,   366,    -1,   110,   200,   387,   411,   201,    -1,
      -1,    -1,   248,   247,   200,   345,   289,   201,   475,   343,
     203,   230,   204,    -1,    -1,   325,   248,   247,   200,   346,
     289,   201,   475,   343,   203,   230,   204,    -1,    -1,    79,
     348,   352,    -1,    -1,   182,    79,   349,   352,    -1,    -1,
     197,   350,   289,   198,   475,   352,    -1,    -1,   182,   197,
     351,   289,   198,   475,   352,    -1,     8,   341,    -1,     8,
     338,    -1,     8,   203,   230,   204,    -1,    85,    -1,   466,
      -1,   354,     9,   353,   129,   341,    -1,   353,   129,   341,
      -1,   355,     9,   353,   129,   406,    -1,   353,   129,   406,
      -1,   354,   410,    -1,    -1,   355,   410,    -1,    -1,   173,
     200,   356,   201,    -1,   131,   200,   447,   201,    -1,    66,
     447,   207,    -1,   399,   203,   449,   204,    -1,   175,   200,
     453,   201,    -1,   176,   200,   453,   201,    -1,   174,   200,
     454,   201,    -1,   175,   200,   457,   201,    -1,   176,   200,
     457,   201,    -1,   174,   200,   458,   201,    -1,   399,   203,
     451,   204,    -1,   366,    66,   442,   207,    -1,   367,    66,
     442,   207,    -1,   359,    -1,   466,    -1,    85,    -1,   200,
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
     117,    -1,   440,    -1,   399,    -1,   117,    -1,   444,    -1,
     200,   201,    -1,   332,    -1,    -1,    -1,    84,    -1,   461,
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
     228,   148,   123,    -1,   226,    -1,    78,    -1,   466,    -1,
     405,    -1,   208,   461,   208,    -1,   209,   461,   209,    -1,
     144,   461,   145,    -1,   412,   410,    -1,    -1,     9,    -1,
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
     217,    -1,   203,   341,   204,    -1,   429,    -1,   439,    -1,
      66,   442,   207,    -1,   203,   341,   204,    -1,   435,   431,
      -1,   200,   331,   201,   431,    -1,   445,   431,    -1,   200,
     331,   201,   431,    -1,   439,    -1,   398,    -1,   437,    -1,
     438,    -1,   432,    -1,   434,   428,   430,    -1,   200,   331,
     201,   428,   430,    -1,   400,   148,   439,    -1,   436,   200,
     291,   201,    -1,   200,   434,   201,    -1,   398,    -1,   437,
      -1,   438,    -1,   432,    -1,   434,   428,   429,    -1,   200,
     331,   201,   428,   429,    -1,   436,   200,   291,   201,    -1,
     200,   434,   201,    -1,   439,    -1,   432,    -1,   200,   434,
     201,    -1,   434,   428,   430,   471,   200,   291,   201,    -1,
     200,   331,   201,   428,   430,   471,   200,   291,   201,    -1,
     400,   148,   217,   471,   200,   291,   201,    -1,   400,   148,
     439,   200,   291,   201,    -1,   400,   148,   203,   341,   204,
     200,   291,   201,    -1,   440,    -1,   443,   440,    -1,   440,
      66,   442,   207,    -1,   440,   203,   341,   204,    -1,   441,
      -1,    79,    -1,   205,   203,   341,   204,    -1,   341,    -1,
      -1,   205,    -1,   443,   205,    -1,   439,    -1,   433,    -1,
     444,   428,   430,    -1,   200,   331,   201,   428,   430,    -1,
     400,   148,   439,    -1,   200,   434,   201,    -1,    -1,   433,
      -1,   444,   428,   429,    -1,   200,   331,   201,   428,   429,
      -1,   200,   434,   201,    -1,   446,     9,    -1,   446,     9,
     434,    -1,   446,     9,   130,   200,   446,   201,    -1,    -1,
     434,    -1,   130,   200,   446,   201,    -1,   448,   410,    -1,
      -1,   448,     9,   341,   129,   341,    -1,   448,     9,   341,
      -1,   341,   129,   341,    -1,   341,    -1,   448,     9,   341,
     129,    35,   434,    -1,   448,     9,    35,   434,    -1,   341,
     129,    35,   434,    -1,    35,   434,    -1,   450,   410,    -1,
      -1,   450,     9,   341,   129,   341,    -1,   450,     9,   341,
      -1,   341,   129,   341,    -1,   341,    -1,   452,   410,    -1,
      -1,   452,     9,   406,   129,   406,    -1,   452,     9,   406,
      -1,   406,   129,   406,    -1,   406,    -1,   455,   410,    -1,
      -1,   456,   410,    -1,    -1,   455,     9,   341,   129,   341,
      -1,   341,   129,   341,    -1,   456,     9,   341,    -1,   341,
      -1,   459,   410,    -1,    -1,   460,   410,    -1,    -1,   459,
       9,   406,   129,   406,    -1,   406,   129,   406,    -1,   460,
       9,   406,    -1,   406,    -1,   461,   462,    -1,   461,    84,
      -1,   462,    -1,    84,   462,    -1,    79,    -1,    79,    66,
     463,   207,    -1,    79,   428,   217,    -1,   146,   341,   204,
      -1,   146,    78,    66,   341,   207,   204,    -1,   147,   434,
     204,    -1,   217,    -1,    80,    -1,    79,    -1,   120,   200,
     465,   201,    -1,   121,   200,   434,   201,    -1,   121,   200,
     342,   201,    -1,   121,   200,   331,   201,    -1,     7,   341,
      -1,     6,   341,    -1,     5,   200,   341,   201,    -1,     4,
     341,    -1,     3,   341,    -1,   434,    -1,   465,     9,   434,
      -1,   400,   148,   217,    -1,   400,   148,   123,    -1,    -1,
      96,   482,    -1,   177,   470,    14,   482,   202,    -1,   179,
     470,   467,    14,   482,   202,    -1,   217,    -1,   482,   217,
      -1,   217,    -1,   217,   169,   476,   170,    -1,   169,   473,
     170,    -1,    -1,   482,    -1,   472,     9,   482,    -1,   472,
     410,    -1,   472,     9,   162,    -1,   473,    -1,   162,    -1,
      -1,    -1,    30,   482,    -1,   476,     9,   477,   217,    -1,
     477,   217,    -1,   476,     9,   477,   217,    96,   482,    -1,
     477,   217,    96,   482,    -1,    46,    -1,    47,    -1,    -1,
      85,   129,   482,    -1,   228,   148,   217,   129,   482,    -1,
     479,     9,   478,    -1,   478,    -1,   479,   410,    -1,    -1,
     173,   200,   480,   201,    -1,    29,   482,    -1,    55,   482,
      -1,   228,    -1,   131,    -1,   132,    -1,   481,    -1,   131,
     169,   482,   170,    -1,   131,   169,   482,     9,   482,   170,
      -1,   153,    -1,   200,   104,   200,   474,   201,    30,   482,
     201,    -1,   200,   482,     9,   472,   410,   201,    -1,   482,
      -1,    -1
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
    1265,  1268,  1269,  1273,  1279,  1285,  1292,  1294,  1299,  1304,
    1310,  1314,  1318,  1322,  1327,  1332,  1337,  1342,  1348,  1357,
    1362,  1367,  1373,  1375,  1379,  1383,  1388,  1392,  1395,  1398,
    1402,  1406,  1410,  1414,  1419,  1427,  1429,  1432,  1433,  1434,
    1435,  1437,  1439,  1444,  1445,  1448,  1449,  1450,  1454,  1455,
    1457,  1458,  1462,  1464,  1467,  1471,  1477,  1479,  1482,  1482,
    1486,  1485,  1489,  1493,  1491,  1506,  1503,  1516,  1518,  1520,
    1522,  1524,  1526,  1528,  1532,  1533,  1534,  1537,  1543,  1546,
    1552,  1555,  1560,  1562,  1567,  1572,  1576,  1577,  1583,  1584,
    1586,  1590,  1591,  1596,  1597,  1601,  1602,  1606,  1608,  1614,
    1619,  1620,  1622,  1626,  1627,  1628,  1629,  1633,  1634,  1635,
    1636,  1637,  1638,  1640,  1645,  1648,  1649,  1653,  1654,  1658,
    1659,  1662,  1663,  1666,  1667,  1670,  1671,  1675,  1676,  1677,
    1678,  1679,  1680,  1681,  1685,  1686,  1689,  1690,  1691,  1694,
    1696,  1698,  1699,  1702,  1704,  1709,  1710,  1712,  1713,  1714,
    1717,  1721,  1722,  1726,  1727,  1731,  1732,  1733,  1737,  1741,
    1746,  1750,  1754,  1759,  1760,  1761,  1764,  1766,  1767,  1768,
    1771,  1772,  1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,
    1781,  1782,  1783,  1784,  1785,  1786,  1787,  1788,  1789,  1790,
    1791,  1792,  1793,  1794,  1795,  1796,  1797,  1798,  1799,  1800,
    1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,
    1811,  1812,  1813,  1815,  1816,  1818,  1820,  1821,  1822,  1823,
    1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,  1833,
    1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,  1846,
    1850,  1855,  1854,  1869,  1867,  1884,  1884,  1900,  1899,  1917,
    1917,  1933,  1932,  1953,  1954,  1955,  1960,  1962,  1966,  1970,
    1976,  1980,  1986,  1988,  1992,  1994,  1998,  2002,  2003,  2007,
    2014,  2015,  2019,  2023,  2025,  2030,  2035,  2042,  2044,  2049,
    2050,  2051,  2053,  2057,  2061,  2065,  2069,  2071,  2073,  2075,
    2080,  2081,  2086,  2087,  2088,  2089,  2090,  2091,  2095,  2099,
    2103,  2107,  2112,  2117,  2121,  2122,  2126,  2127,  2131,  2132,
    2136,  2137,  2141,  2145,  2149,  2153,  2154,  2155,  2156,  2160,
    2166,  2175,  2188,  2189,  2192,  2195,  2198,  2199,  2202,  2206,
    2209,  2212,  2219,  2220,  2224,  2225,  2227,  2231,  2232,  2233,
    2234,  2235,  2236,  2237,  2238,  2239,  2240,  2241,  2242,  2243,
    2244,  2245,  2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,
    2254,  2255,  2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,
    2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,
    2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,
    2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,
    2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,
    2304,  2305,  2306,  2307,  2308,  2309,  2310,  2314,  2319,  2320,
    2323,  2324,  2325,  2329,  2330,  2331,  2335,  2336,  2337,  2341,
    2342,  2343,  2346,  2348,  2352,  2353,  2354,  2355,  2357,  2358,
    2359,  2360,  2361,  2362,  2363,  2364,  2365,  2366,  2369,  2374,
    2375,  2376,  2378,  2379,  2381,  2382,  2383,  2384,  2385,  2386,
    2388,  2390,  2392,  2394,  2396,  2397,  2398,  2399,  2400,  2401,
    2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,  2410,  2411,
    2412,  2414,  2416,  2418,  2420,  2421,  2424,  2425,  2429,  2431,
    2435,  2438,  2441,  2447,  2448,  2449,  2450,  2451,  2452,  2453,
    2458,  2460,  2464,  2465,  2468,  2469,  2473,  2476,  2478,  2480,
    2484,  2485,  2486,  2487,  2489,  2492,  2496,  2497,  2498,  2499,
    2502,  2503,  2504,  2505,  2506,  2508,  2509,  2514,  2516,  2519,
    2522,  2524,  2526,  2529,  2531,  2535,  2537,  2540,  2543,  2549,
    2551,  2554,  2555,  2560,  2563,  2567,  2567,  2572,  2575,  2576,
    2580,  2581,  2585,  2586,  2590,  2591,  2595,  2596,  2600,  2601,
    2606,  2608,  2613,  2614,  2615,  2616,  2617,  2618,  2620,  2623,
    2626,  2628,  2632,  2633,  2634,  2635,  2636,  2638,  2641,  2643,
    2647,  2648,  2649,  2653,  2656,  2663,  2667,  2671,  2678,  2679,
    2684,  2686,  2687,  2690,  2691,  2694,  2695,  2699,  2700,  2704,
    2705,  2706,  2709,  2712,  2715,  2718,  2719,  2720,  2722,  2725,
    2729,  2730,  2731,  2733,  2734,  2735,  2739,  2741,  2744,  2746,
    2747,  2748,  2749,  2752,  2754,  2755,  2759,  2761,  2764,  2766,
    2767,  2768,  2772,  2774,  2777,  2780,  2782,  2784,  2788,  2790,
    2793,  2795,  2798,  2800,  2803,  2804,  2808,  2810,  2813,  2815,
    2818,  2821,  2825,  2827,  2831,  2832,  2834,  2835,  2841,  2842,
    2844,  2846,  2848,  2850,  2853,  2854,  2855,  2859,  2860,  2861,
    2862,  2863,  2864,  2865,  2866,  2867,  2871,  2872,  2876,  2878,
    2886,  2888,  2892,  2896,  2903,  2904,  2910,  2911,  2918,  2921,
    2925,  2928,  2933,  2938,  2940,  2941,  2942,  2946,  2947,  2951,
    2953,  2954,  2957,  2962,  2963,  2964,  2968,  2971,  2980,  2982,
    2986,  2989,  2992,  3000,  3003,  3006,  3007,  3010,  3013,  3014,
    3017,  3021,  3025,  3031,  3041,  3042
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
     285,   286,   286,   287,   287,   287,   287,   287,   287,   287,
     287,   288,   288,   288,   288,   288,   288,   288,   288,   289,
     289,   289,   289,   289,   289,   289,   289,   290,   290,   290,
     290,   290,   290,   290,   290,   291,   291,   292,   292,   292,
     292,   292,   292,   293,   293,   294,   294,   294,   295,   295,
     295,   295,   296,   296,   297,   298,   299,   299,   301,   300,
     302,   300,   300,   303,   300,   304,   300,   300,   300,   300,
     300,   300,   300,   300,   305,   305,   305,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   311,   311,
     311,   312,   312,   313,   313,   314,   314,   315,   315,   316,
     317,   317,   317,   318,   318,   318,   318,   319,   319,   319,
     319,   319,   319,   319,   320,   320,   320,   321,   321,   322,
     322,   323,   323,   324,   324,   325,   325,   326,   326,   326,
     326,   326,   326,   326,   327,   327,   328,   328,   328,   329,
     329,   329,   329,   330,   330,   331,   331,   331,   331,   331,
     332,   333,   333,   334,   334,   335,   335,   335,   336,   337,
     338,   339,   340,   341,   341,   341,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   343,
     343,   345,   344,   346,   344,   348,   347,   349,   347,   350,
     347,   351,   347,   352,   352,   352,   353,   353,   354,   354,
     355,   355,   356,   356,   357,   357,   358,   359,   359,   360,
     361,   361,   362,   363,   363,   364,   365,   366,   366,   367,
     367,   367,   367,   368,   369,   370,   371,   371,   371,   371,
     372,   372,   373,   373,   373,   373,   373,   373,   374,   375,
     376,   377,   378,   379,   380,   380,   381,   381,   382,   382,
     383,   383,   384,   385,   386,   387,   387,   387,   387,   388,
     389,   389,   390,   390,   391,   391,   392,   392,   393,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   398,   399,   399,
     400,   400,   400,   401,   401,   401,   402,   402,   402,   403,
     403,   403,   404,   404,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     407,   407,   407,   408,   408,   408,   408,   408,   408,   408,
     409,   409,   410,   410,   411,   411,   412,   412,   412,   412,
     413,   413,   413,   413,   413,   413,   414,   414,   414,   414,
     415,   415,   415,   415,   415,   415,   415,   416,   416,   417,
     417,   417,   417,   418,   418,   419,   419,   420,   420,   421,
     421,   422,   422,   423,   423,   425,   424,   426,   427,   427,
     428,   428,   429,   429,   430,   430,   431,   431,   432,   432,
     433,   433,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   435,   435,   435,   435,   435,   435,   435,   435,
     436,   436,   436,   437,   437,   438,   438,   438,   439,   439,
     440,   440,   440,   441,   441,   442,   442,   443,   443,   444,
     444,   444,   444,   444,   444,   445,   445,   445,   445,   445,
     446,   446,   446,   446,   446,   446,   447,   447,   448,   448,
     448,   448,   448,   448,   448,   448,   449,   449,   450,   450,
     450,   450,   451,   451,   452,   452,   452,   452,   453,   453,
     454,   454,   455,   455,   456,   456,   457,   457,   458,   458,
     459,   459,   460,   460,   461,   461,   461,   461,   462,   462,
     462,   462,   462,   462,   463,   463,   463,   464,   464,   464,
     464,   464,   464,   464,   464,   464,   465,   465,   466,   466,
     467,   467,   468,   468,   469,   469,   470,   470,   471,   471,
     472,   472,   473,   474,   474,   474,   474,   475,   475,   476,
     476,   476,   476,   477,   477,   477,   478,   478,   479,   479,
     480,   480,   481,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   483,   483
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
       0,     3,     0,     7,     8,     6,     2,     5,     6,     4,
       0,     4,     5,     7,     6,     6,     7,     9,     8,     6,
       7,     5,     2,     4,     5,     3,     0,     3,     4,     6,
       5,     5,     6,     8,     7,     2,     0,     1,     2,     2,
       3,     4,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     2,     3,     2,     0,     0,     4,
       0,     5,     2,     0,    10,     0,    11,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     1,     1,     1,     4,
       1,     1,     3,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     3,     3,     2,     1,     1,
       3,     3,     1,     1,     0,     1,     2,     4,     3,     6,
       2,     3,     6,     1,     1,     1,     6,     3,     4,     6,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     4,     0,
       6,     0,     7,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     1,
       1,     1,     3,     2,     3,     4,     2,     3,     1,     2,
       1,     2,     1,     1,     1,     1,     1,     1,     4,     4,
       2,     8,    10,     2,     1,     3,     1,     2,     1,     1,
       1,     1,     2,     4,     3,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     4,     0,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     4,     3,     4,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       2,     0,     1,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     2,     2,     4,     3,     4,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     3,     1,     1,     3,     3,     2,     4,
       2,     4,     1,     1,     1,     1,     1,     3,     5,     3,
       4,     3,     1,     1,     1,     1,     3,     5,     4,     3,
       1,     1,     3,     7,     9,     7,     6,     8,     1,     2,
       4,     4,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     3,     5,     3,     3,     0,     1,     3,     5,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     0,
       2,     0,     5,     3,     3,     1,     2,     0,     2,     0,
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     4,
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
       0,   355,     0,   745,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   817,     0,
     805,   628,     0,   634,   635,   636,    22,   694,   793,    98,
     637,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   327,   328,
     329,   332,   331,   330,     0,     0,     0,     0,   158,     0,
       0,     0,   641,   643,   644,   638,   639,     0,     0,   645,
     640,     0,   619,    23,    24,    25,    27,    26,     0,   642,
       0,     0,     0,     0,     0,     0,     0,   646,   333,    28,
      29,    31,    30,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   449,     0,    97,    70,   797,   629,     0,     0,
       4,    59,    61,    64,   693,     0,   618,     0,     6,   130,
       7,     9,     8,    10,     0,     0,   325,   365,     0,     0,
       0,     0,     0,     0,     0,   363,   436,   437,   431,   430,
     349,   432,   433,   438,     0,     0,   348,   763,   620,     0,
     696,   429,   324,   766,   364,     0,     0,   764,   765,   762,
     788,   792,     0,   419,   695,    11,   332,   331,   330,     0,
       0,    27,    59,   130,     0,   875,   364,   874,     0,   872,
     871,   435,     0,   356,   360,     0,     0,   403,   404,   405,
     406,   428,   426,   425,   424,   423,   422,   421,   420,   793,
     621,     0,   889,   620,     0,   385,   383,     0,   821,     0,
     703,   347,   624,     0,   889,   623,     0,   633,   800,   799,
     625,     0,     0,   627,   427,     0,     0,     0,     0,   352,
       0,    78,   354,     0,     0,    84,    86,     0,     0,    88,
       0,     0,     0,   916,   917,   921,     0,     0,    59,   915,
       0,   918,     0,     0,    90,     0,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,    42,    47,   245,     0,
       0,   244,   160,   159,   250,     0,     0,     0,     0,     0,
     886,   146,   156,   813,   817,   858,     0,   648,     0,     0,
       0,   856,     0,    16,     0,    63,   138,   150,   157,   525,
     463,   841,   839,   839,     0,   880,   447,   451,   749,   365,
       0,   363,   364,     0,     0,   630,     0,   631,     0,     0,
       0,   120,     0,     0,    66,   236,     0,    21,   129,     0,
     155,   142,   154,   330,   333,   130,   326,   111,   112,   113,
     114,   115,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   805,     0,
     110,   796,   796,   118,   827,     0,     0,     0,     0,     0,
       0,   323,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   384,   382,   750,   751,     0,
     796,     0,   758,   236,   796,     0,   798,   789,   813,     0,
     130,     0,     0,    92,     0,   747,   742,   703,     0,     0,
       0,     0,   825,     0,   468,   702,   816,     0,     0,    66,
       0,   236,   346,     0,   760,   626,     0,    70,   196,     0,
     446,     0,    95,     0,     0,   353,     0,     0,     0,     0,
       0,    87,   109,    89,   913,   914,     0,   911,     0,     0,
       0,   885,     0,   116,    91,   119,     0,     0,     0,     0,
       0,     0,     0,   483,     0,   490,   492,   493,   494,   495,
     496,   497,   488,   510,   511,    70,     0,   106,   108,     0,
       0,    44,    51,     0,     0,    46,    55,    48,     0,    18,
       0,     0,   246,     0,    93,     0,     0,    94,   876,     0,
       0,   365,   363,   364,     0,   905,   166,     0,   814,     0,
       0,     0,     0,   647,   857,   694,     0,     0,   855,   699,
     854,    62,     5,    13,    14,     0,   164,     0,     0,   456,
       0,     0,   703,     0,     0,   622,   457,   845,     0,   703,
       0,     0,   703,     0,     0,     0,     0,     0,   749,     0,
     705,   748,   925,   345,   416,   771,    75,    69,    71,    72,
      73,    74,   324,     0,   434,   697,   698,    60,   703,     0,
     890,     0,     0,     0,   705,   237,     0,   441,   132,   162,
       0,   388,   390,   389,     0,     0,   386,   387,   391,   393,
     392,   408,   407,   410,   409,   411,   413,   414,   412,   402,
     401,   395,   396,   394,   397,   398,   400,   415,   399,   795,
       0,     0,   831,     0,   703,   879,     0,   878,   769,   788,
     148,   140,   152,   144,   130,   355,     0,   358,   361,   367,
     484,   381,   380,   379,   378,   377,   376,   375,   374,   373,
     372,   371,   370,     0,   752,   754,   767,   755,     0,     0,
       0,     0,     0,     0,     0,   873,   357,   740,   744,   702,
     746,     0,     0,   889,     0,   820,     0,   819,     0,   804,
     803,     0,   754,   801,   350,   198,   200,    70,   454,   453,
     351,     0,    70,   180,    79,   354,     0,     0,     0,     0,
       0,   192,   192,    85,     0,     0,     0,   909,   703,     0,
     896,     0,     0,     0,     0,     0,   701,   637,     0,     0,
     619,     0,     0,     0,     0,     0,    64,   650,   618,   656,
     657,   655,     0,   649,    68,   654,     0,     0,   500,     0,
       0,   506,   503,   504,   512,     0,   491,   486,     0,   489,
       0,     0,     0,    52,    19,     0,     0,    56,    20,     0,
       0,     0,    41,    49,     0,   243,   251,   248,     0,     0,
     867,   870,   869,   868,    12,   903,   904,     0,     0,     0,
       0,   813,   810,     0,   467,   866,   865,   864,     0,   860,
       0,   861,   863,     0,     5,     0,     0,     0,   519,   520,
     528,   527,     0,     0,   702,   462,   466,     0,   472,   702,
     840,     0,   470,   702,   838,   471,     0,   881,     0,   448,
       0,   897,   749,   222,   924,     0,     0,   759,   794,   702,
     892,   888,   238,   239,   617,   704,   235,     0,   749,     0,
       0,   164,   443,   134,   418,     0,   477,   478,     0,   469,
     702,   826,     0,     0,   236,   166,     0,   164,   162,     0,
     805,   368,     0,     0,   756,   757,   770,   790,   791,     0,
       0,     0,   728,   710,   711,   712,   713,     0,     0,     0,
     721,   720,   734,   703,     0,   742,   824,   823,     0,     0,
     761,   632,   202,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   184,     0,    70,   182,   103,
     192,     0,   192,     0,     0,   919,     0,     0,   702,   910,
     912,   895,   703,   894,     0,   703,   678,   679,   676,   677,
     709,     0,   703,   701,     0,     0,   465,   849,   847,   847,
       0,     0,   833,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   485,
       0,     0,     0,   508,   509,   507,     0,     0,   487,     0,
     122,     0,   125,   107,     0,    43,    53,     0,    45,    57,
      50,   247,     0,   877,    96,   905,   887,   900,   165,   167,
     257,     0,     0,   811,     0,   859,     0,    17,     0,   880,
     163,   257,     0,     0,   459,     0,   878,   844,   843,     0,
     882,     0,   897,     0,     0,   925,     0,   227,   225,   754,
     768,   891,     0,     0,   240,    67,     0,   749,   161,     0,
     749,     0,   417,   830,   829,     0,   236,     0,     0,     0,
       0,   164,   136,   633,   753,   236,     0,   716,   717,   718,
     719,   722,   723,   732,     0,   703,   728,     0,   715,   736,
     702,   739,   741,   743,     0,   818,   754,   802,     0,     0,
       0,     0,   199,   455,    81,     0,   354,   172,   174,   813,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,   906,     0,   908,   702,     0,     0,     0,   652,   702,
     700,     0,   691,     0,   703,     0,   853,     0,   703,     0,
       0,   703,     0,   658,   692,   690,   837,     0,   703,   661,
     663,   662,     0,     0,   659,   660,   664,   666,   665,   681,
     680,   683,   682,   684,   686,   687,   685,   674,   673,   668,
     669,   667,   670,   671,   672,   675,   498,     0,   499,   505,
     513,   514,     0,    70,    54,    58,   249,     0,     0,     0,
     324,   815,   813,   359,   362,   366,     0,    15,     0,   324,
     531,     0,     0,   533,   526,   529,     0,   524,     0,     0,
     883,     0,   898,   450,     0,   228,     0,     0,   223,     0,
     242,   241,   897,     0,   257,     0,   749,     0,   236,     0,
     786,   257,   880,   257,     0,     0,   369,     0,     0,   725,
     702,   727,     0,   714,     0,     0,   703,   733,   822,     0,
      70,     0,   195,   181,     0,     0,     0,   171,    99,   185,
       0,     0,   188,     0,   193,   194,    70,   187,   920,     0,
     893,     0,   923,   708,   707,   651,     0,   702,   464,   653,
     475,   702,   848,     0,   473,   702,   846,   474,     0,   476,
     702,   832,   689,     0,     0,     0,     0,   899,   902,   168,
       0,     0,     0,   322,     0,     0,     0,   147,   256,   258,
       0,   321,     0,   324,     0,   862,   253,   151,   522,     0,
       0,   458,   842,   452,     0,   231,   221,     0,   224,   230,
     236,   440,   897,   324,   897,     0,   828,     0,   785,   324,
       0,   324,   257,   749,   783,   731,   730,   724,     0,   726,
     702,   735,    70,   201,    77,    82,   101,   175,     0,   183,
     189,    70,   191,   907,     0,     0,   461,     0,   852,   851,
       0,   836,   835,   688,     0,    70,   126,     0,     0,     0,
       0,     0,   169,   288,   286,   290,   619,    27,     0,   282,
       0,   287,   299,     0,   297,   302,     0,   301,     0,   300,
       0,   130,   260,     0,   262,     0,   812,     0,   523,   521,
     532,   530,   232,     0,     0,   219,   229,     0,     0,     0,
       0,   143,   440,   897,   787,   149,   253,   153,   324,     0,
       0,   738,     0,   197,     0,     0,    70,   178,   100,   190,
     922,   706,     0,     0,     0,     0,     0,   901,     0,     0,
       0,     0,   272,   276,     0,     0,   267,   583,   582,   579,
     581,   580,   600,   602,   601,   571,   542,   543,   561,   577,
     576,   538,   548,   549,   551,   550,   570,   554,   552,   553,
     555,   556,   557,   558,   559,   560,   562,   563,   564,   565,
     566,   567,   569,   568,   539,   540,   541,   544,   545,   547,
     585,   586,   595,   594,   593,   592,   591,   590,   578,   597,
     587,   588,   589,   572,   573,   574,   575,   598,   599,   603,
     605,   604,   606,   607,   584,   609,   608,   611,   613,   612,
     546,   616,   614,   615,   610,   596,   537,   294,   534,     0,
     268,   315,   316,   314,   307,     0,   308,   269,   341,     0,
       0,     0,     0,   130,   139,   252,     0,     0,     0,   220,
     234,   784,     0,    70,   317,    70,   133,     0,     0,     0,
     145,   897,   729,     0,    70,   176,    83,   102,     0,   460,
     850,   834,   501,   124,   270,   271,   344,   170,     0,     0,
     291,   283,     0,     0,     0,   296,   298,     0,     0,   303,
     310,   311,   309,     0,     0,   259,     0,     0,     0,     0,
     254,     0,   233,     0,   517,   705,     0,     0,    70,   135,
     141,     0,   737,     0,     0,     0,   104,   273,    59,     0,
     274,   275,     0,     0,   289,   293,   535,   536,     0,   284,
     312,   313,   305,   306,   304,   342,   339,   263,   261,   343,
       0,   255,   518,   704,     0,   442,   318,     0,   137,     0,
     179,   502,     0,   128,     0,   324,   292,   295,     0,   749,
     265,     0,   515,   439,   444,   177,     0,     0,   105,   280,
       0,   323,   340,     0,   705,   335,   749,   516,     0,   127,
       0,     0,   279,   897,   749,   206,   336,   337,   338,   925,
     334,     0,     0,     0,   278,     0,   335,     0,   897,     0,
     277,   319,    70,   264,   925,     0,   211,   209,     0,    70,
       0,     0,   212,     0,     0,   207,   266,     0,   320,     0,
     215,   205,     0,   208,   214,   123,   216,     0,     0,   203,
     213,     0,   204,   218,   217
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   804,   542,   182,   275,   500,
     504,   276,   501,   505,   122,   123,   124,   125,   126,   127,
     323,   577,   578,   454,   240,  1424,   460,  1348,  1425,  1653,
     762,   270,   495,  1616,   993,  1173,  1668,   339,   183,   579,
     849,  1051,  1225,   131,   545,   866,   580,   599,   868,   526,
     865,   581,   546,   867,   341,   291,   307,   134,   851,   807,
     790,  1008,  1371,  1101,   913,  1566,  1428,   704,   919,   459,
     713,   921,  1256,   696,   902,   905,  1090,  1673,  1674,   569,
     570,   593,   594,   280,   281,   285,  1397,  1545,  1546,  1180,
    1298,  1390,  1541,  1659,  1676,  1578,  1620,  1621,  1622,  1378,
    1379,  1380,  1579,  1585,  1629,  1383,  1384,  1388,  1534,  1535,
    1536,  1556,  1703,  1299,  1300,   184,   136,  1689,  1690,  1539,
    1302,   137,   233,   455,   456,   138,   139,   140,   141,   142,
     143,   144,   145,  1409,   146,   848,  1050,   147,   237,   567,
     318,   568,   450,   551,   552,  1124,   553,  1125,   148,   149,
     150,   151,   152,   739,   740,   741,   153,   154,   267,   155,
     268,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     752,   753,   985,   492,   493,   494,   759,  1605,   156,   547,
    1399,   548,  1022,   812,  1197,  1194,  1527,  1528,   157,   158,
     159,   227,   234,   326,   442,   160,   940,   745,   161,   941,
     840,   833,   942,   891,  1071,  1073,  1074,  1075,   893,  1235,
    1236,   894,   678,   427,   195,   196,   582,   572,   409,   665,
     666,   837,   163,   228,   186,   165,   166,   167,   168,   169,
     170,   171,   630,   172,   230,   231,   529,   219,   220,   633,
     634,  1137,  1138,   561,   558,   562,   559,  1130,  1127,  1131,
    1128,   300,   301,   798,   173,   519,   174,   566,   175,  1547,
     292,   334,   588,   589,   934,  1034,   787,   788,   717,   718,
     719,   261,   262,   835
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1421
static const yytype_int16 yypact[] =
{
   -1421,   120, -1421, -1421,  6218, 13874, 13874,   -66, 13874, 13874,
   13874, 11972, 13874, -1421, 13874, 13874, 13874, 13874, 13874, 13874,
   13874, 13874, 13874, 13874, 13874, 13874, 14882, 14882, 12179, 13874,
   14948,   -60,   -58, -1421, -1421, -1421, -1421, -1421,   144, -1421,
     104, 13874, -1421,   -58,   -28,   -20,    -6,   -58, 12344, 15939,
   12509, -1421, 14393, 10979,   -18, 13874, 15672,   103, -1421, -1421,
   -1421,   110,   168,    47,     0,   165,   169,   186, -1421, 15939,
     191,   219, -1421, -1421, -1421, -1421, -1421,   317,  4542, -1421,
   -1421, 15939, -1421, -1421, -1421, -1421, 15939, -1421, 15939, -1421,
      73,   229,   243,   257,   273, 15939, 15939, -1421,    30, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, 13874, -1421, -1421,    21,   314,   325,   325,
   -1421,   404,   329,   326, -1421,   299, -1421,    51, -1421,   431,
   -1421, -1421, -1421, -1421,  3549,   420, -1421, -1421,   336,   346,
     359,   372,   375,   377,  3014, -1421, -1421, -1421, -1421,   444,
   -1421, -1421, -1421,   492,   516,   382, -1421,    41,   328,   453,
   -1421, -1421,  1358,     9,   595,    94,   392,    96,   101,   406,
      27, -1421,   124, -1421,   542, -1421, -1421, -1421,   475,   433,
     482, -1421, -1421,   431,   420, 16228,  9549, 16228, 13874, 16228,
   16228, 11169,   442,  4476, 11169,   601, 15939,   581,   581,   307,
     581,   581,   581,   581,   581,   581,   581,   581,   581, -1421,
   -1421,  4220,   478, -1421,   501,    77,    77, 14882, 15135,   443,
     642, -1421,   475,  4220,   478,   505,   508,   457,   107, -1421,
      77,    94, 12674, -1421, -1421, 13874,  9530,   653,    53, 16228,
   10565, -1421, 13874, 13874, 15939, -1421, -1421,  3062,   460, -1421,
    3147, 14393, 14393,   495, -1421, -1421,   466,  5512,   651, -1421,
     654, -1421, 15939,   590, -1421,   470,  3376,   472,   547, -1421,
       2,  3560, 15805, 15819, 15939,    59, -1421,     7, -1421, 14607,
      60, -1421, -1421, -1421,   666,    64, 14882, 14882, 13874,   477,
     512, -1421, -1421, 14744, 12179,    22,   261, -1421, 14039, 14882,
     362, -1421, 15939, -1421,   -19,   329, -1421, -1421, -1421, -1421,
   14478, 13874, 13874, 13874,   670,   591, -1421, -1421,    31,   484,
   16228,   487,  1751,  6425, 13874,   301,   483,   403,   301,   277,
     289, -1421, 15939, 14393,   490, 11186, 14393, -1421, -1421,  5581,
   -1421, -1421, -1421, -1421, -1421,   431, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, 13874, 13874, 13874, 12881, 13874, 13874, 13874,
   13874, 13874, 13874, 13874, 13874, 13874, 13874, 13874, 13874, 13874,
   13874, 13874, 13874, 13874, 13874, 13874, 13874, 13874, 14948, 13874,
   -1421, 13874, 13874, -1421, 13874,  3739, 15939, 15939, 15939,  3549,
     588,   450, 10772, 13874, 13874, 13874, 13874, 13874, 13874, 13874,
   13874, 13874, 13874, 13874, 13874, -1421, -1421, -1421, -1421,  2673,
   13874, 13874, -1421, 11186, 13874, 13874,    21,   111, 14744,   493,
     431, 13088,  3736, -1421, 13874, -1421,   496,   686,  4220,   502,
     -15,  3489,    77, 13295, -1421, 13502, -1421,   503,   236, -1421,
     137, 11186, -1421,  2673, -1421, -1421,  3840, -1421, -1421, 11393,
   -1421, 13874, -1421,   614,  9737,   699,   514, 16140,   703,    80,
      42, -1421, -1421, -1421, -1421, -1421, 14393, 15685,   518,   710,
   14695, -1421,   534, -1421, -1421, -1421,   646, 13874,   648,   656,
   13874, 13874, 13874, -1421,   547, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421,   550, -1421, -1421, -1421,   536, -1421, -1421, 15939,
     538,   734,    26, 15939,   545,   735,   263,   266, 15866, -1421,
   15939, 13874,    77,   103, -1421, 14695,   669, -1421,    77,    81,
      85,   549,   551,  1768,   552,   356,   628,   555,    77,    87,
     556, 15729, 15939, -1421, -1421,   692,  1810,    18, -1421, -1421,
   -1421,   329, -1421, -1421, -1421,   730,   635,   600,   291, -1421,
      21,   634,   760,   569,   623,   111, -1421, 16228,   571,   764,
   15179,   574,   768,   577, 14393, 14393,   765,   653,    31,   583,
     773, -1421, 14393,    20,   717,   131, -1421, -1421, -1421, -1421,
   -1421, -1421,   854,  1895, -1421, -1421, -1421, -1421,   776,   617,
   -1421, 14882, 13874,   593,   780, 16228,   777, -1421, -1421,   672,
   14925, 11583, 13071, 11169, 13874, 16184, 14243,  1699,  4079, 14414,
    2217, 12859, 12859, 12859, 12859,  2683,  2683,  2683,  2683,   589,
     589,   494,   494,   494,   307,   307,   307, -1421,   581, 16228,
     594,   596, 15236,   605,   786, -1421, 13874,   -26,   598,   111,
   -1421, -1421, -1421, -1421,   431, 13874,  4631, -1421, -1421, 11169,
   -1421, 11169, 11169, 11169, 11169, 11169, 11169, 11169, 11169, 11169,
   11169, 11169, 11169, 13874, -1421,   115,   -26, -1421,   597,  2037,
     609,   604,  2162,    88,   620, -1421, 16228,  3178, -1421, 15939,
   -1421,   484,    20,   478, 14882, 16228, 14882, 15280,    20,   121,
   -1421,   613,   123, -1421, -1421,  9323,   418, -1421, -1421, 16228,
   16228,   -58, -1421, -1421, -1421, 13874,   725,  4884, 14695, 15939,
    9944,   621,   622, -1421,    58,   693,   677, -1421,   817,   627,
    5562, 14393, 14695, 14695, 14695, 14695, 14695, -1421,   630,    45,
     683,   633,   636,   638,   639, 14695,    10, -1421,   694, -1421,
   -1421, -1421,   631, -1421, 16314, -1421, 13874,   659, 16228,   660,
     826,  4431,   838, -1421, 16228,  4035, -1421,   550,   770, -1421,
    6632, 15746,   647,   274, -1421, 15805, 15939,   279, -1421, 15819,
   15939, 15939, -1421, -1421,  2342, -1421, 16314,   837, 14882,   650,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421,   105, 15939, 15746,
     652, 14744, 14828,   839, -1421, -1421, -1421, -1421,   649, -1421,
   13874, -1421, -1421,  5804, -1421, 14393, 15746,   661, -1421, -1421,
   -1421, -1421,   840, 13874, 14478, -1421, -1421, 15879, -1421, 13874,
   -1421, 13874, -1421, 13874, -1421, -1421,   655, -1421, 14393, -1421,
     671,   829,    19, -1421, -1421,    46,  2673, -1421, -1421, 14393,
   -1421, -1421,    77, 16228, -1421, 11600, -1421, 14695,    83,   662,
   15746,   635, -1421, -1421, 13692, 13874, -1421, -1421, 13874, -1421,
   13874, -1421,  2389,   667, 11186,   628,   841,   635,   672, 15939,
   14948,    77,  2531,   668, -1421, -1421,   130, -1421, -1421,   856,
    1936,  1936,  3178, -1421, -1421, -1421, -1421,   673,    48,   674,
   -1421, -1421, -1421,   866,   679,   496,    77,    77, 13709,  2673,
   -1421, -1421,   441,   -58, 10565, -1421,  6839,   680,  7046,   681,
    4884, 14882,   684,   756,    77, 16314,   872, -1421, -1421, -1421,
   -1421,    57, -1421,     6, 14393, -1421, 14393, 15939, 15685, -1421,
   -1421, -1421,   878, -1421,   687,   776,   465,   465,   825,   825,
   15438,   685,   882, 14695,   748, 15939, 14478, 14695, 14695, 14695,
    3978, 15953, 14695, 14695, 14695, 14695, 14544, 14695, 14695, 14695,
   14695, 14695, 14695, 14695, 14695, 14695, 14695, 14695, 14695, 14695,
   14695, 14695, 14695, 14695, 14695, 14695, 14695, 14695, 14695, 16228,
   13874, 13874, 13874, -1421, -1421, -1421, 13874, 13874, -1421,   547,
   -1421,   815, -1421, -1421, 15939, -1421, -1421, 15939, -1421, -1421,
   -1421, -1421, 14695,    77, -1421,   356, -1421,   800,   888, -1421,
   -1421,    89,   698,    77, 11807, -1421,  1224, -1421,  6011,   591,
     888, -1421,   246,   -39, 16228,   772, -1421, 16228, 16228, 15337,
   -1421,   697,   829, 14393,   653, 14393,    40,   889,   830,   133,
     -26, -1421, 14882, 13874, 16228, 16314,   713,    83, -1421,   708,
      83,   715, 13692, 16228, 15381,   721, 11186,   716,   723, 14393,
     724,   635, -1421,   457, -1421, 11186, 13874, -1421, -1421, -1421,
   -1421, -1421, -1421,   790,   722,   923,  3178,   789, -1421, 14478,
    3178, -1421, -1421, -1421, 14882, 16228,   135, -1421,   -58,   907,
     865, 10565, -1421, -1421, -1421,   737, 13874,   756,    77, 14744,
    4884,   739, 14695,  7253,   278,   741, 13874,    33,    17, -1421,
     774, -1421,   816, -1421, 14344,   916,   747, 14695, -1421, 14695,
   -1421,   749, -1421,   822,   943,   752, 16314,   754,   954, 15480,
     778,   964,   782, -1421, -1421, -1421, 15537,   771,   972, 12162,
   13278, 13485, 14695, 16272,  4855,  2475,  5422, 14876, 16041, 16344,
   16344, 16344, 16344,  1843,  1843,  1843,  1843,   664,   664,   465,
     465,   465,   825,   825,   825,   825, 16228,  5361, 16228, -1421,
   16228, -1421,   783, -1421, -1421, -1421, 16314, 15939, 14393, 15746,
     234, -1421, 14744, -1421, -1421, 11169,   781, -1421,   784,   473,
   -1421,    61, 13874, -1421, -1421, -1421, 13874, -1421, 13874, 13874,
   -1421,   653, -1421, -1421,   136,   977,   913, 14695, -1421,   794,
      77, 16228,   829,   796, -1421,   802,    83, 13874, 11186,   803,
   -1421, -1421,   591, -1421,   792,   801, -1421,   806,  3178, -1421,
    3178, -1421,   807, -1421,   880,   809,  1002, -1421,    77,   986,
   -1421,   819, -1421, -1421,   818,   823,    93, -1421, -1421, 16314,
     821,   833, -1421,  2920, -1421, -1421, -1421, -1421, -1421, 14393,
   -1421, 14393, -1421, 16314, 15579, -1421, 14695, 14478, -1421, -1421,
   -1421, 14695, -1421, 14695, -1421, 14695, -1421, -1421, 14695, -1421,
   14695, -1421,  3502, 14695, 13874,   824,  7460,   929, -1421, -1421,
     394, 14393, 15746, -1421, 15626,   871,  5164, -1421, -1421, -1421,
     588,  5419,    65,   450,    95, -1421, -1421, -1421,   885,  2605,
    2651, 16228, 16228, -1421,    44,  1025,   961, 14695, -1421, 16314,
   11186,   931,   829,   844,   829,   842, 16228,   843, -1421,   968,
     847,  1047, -1421,    83, -1421, -1421,   917, -1421,  3178, -1421,
   14478, -1421, -1421,  9323, -1421, -1421, -1421, -1421, 10151, -1421,
   -1421, -1421,  9323, -1421,   846, 14695, 16314,   924, 16314, 16314,
   15636, 16314, 15678,  3502,  5087, -1421, -1421, 14393, 15746, 15746,
    1031,    49, -1421, -1421, -1421, -1421,    68,   849,    70, -1421,
   14246, -1421, -1421,    71, -1421, -1421,  4755, -1421,   852, -1421,
     976,   431, -1421, 14393, -1421,   588, -1421,  3299, -1421, -1421,
   -1421, -1421,  1043,   979, 14695, -1421, 16314,   859,   864,   862,
     353, -1421,   931,   829, -1421, -1421, -1421, -1421,  1346,   870,
    3178, -1421,   938,  9323, 10358, 10151, -1421, -1421, -1421,  9323,
   -1421, 16314, 14695, 14695, 14695, 13874,  7667, -1421,   873,   875,
   14695, 15746, -1421, -1421,   994, 15626, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421,   341, -1421,   871,
   -1421, -1421, -1421, -1421, -1421,    62,   454, -1421,  1060,    74,
   15939,   976,  1074,   431, -1421, -1421,   891,  1076, 14695, -1421,
   16314, -1421,   119, -1421, -1421, -1421, -1421,   892,   353,  4010,
   -1421,   829, -1421,  3178, -1421, -1421, -1421, -1421,  7874, 16314,
   16314, 16314,  4528, -1421, -1421, -1421, 16314, -1421,  1489,    38,
   -1421, -1421, 14695, 14246, 14246,  1036, -1421,  4755,  4755,   455,
   -1421, -1421, -1421, 14695,  1017, -1421,   900,    76, 14695, 15939,
   -1421, 14695, 16314,  1027, -1421,  1095,  8081,  8288, -1421, -1421,
   -1421,   353, -1421,  8495,   906,  1030,  1003, -1421,  1014,   970,
   -1421, -1421,  1016,   994, -1421, 16314, -1421, -1421,   956, -1421,
    1086, -1421, -1421, -1421, -1421, 16314,  1106, -1421, -1421, 16314,
     921, 16314, -1421,   128,   932, -1421, -1421,  8702, -1421,   920,
   -1421, -1421,   925,   966, 15939,   450, -1421, -1421, 14695,    86,
   -1421,  1055, -1421, -1421, -1421, -1421, 15746,   647, -1421,   973,
   15939,   339, 16314,   934,  1130,   440,    86, -1421,  1061, -1421,
   15746,   939, -1421,   829,    91, -1421, -1421, -1421, -1421, 14393,
   -1421,   941,   945,    78, -1421,   366,   440,   308,   829,   946,
   -1421, -1421, -1421, -1421, 14393,    72,  1134,  1072,   366, -1421,
    8909,   318,  1139,  1075, 14695, -1421, -1421,  9116, -1421,   233,
    1141,  1077, 14695, -1421, 16314, -1421,  1152,  1089, 14695, -1421,
   16314, 14695, -1421, 16314, 16314
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1421, -1421, -1421,  -493, -1421, -1421, -1421,    -4, -1421, -1421,
   -1421,   663,   405,   408,    43,  1190,  3907, -1421,  2553, -1421,
    -377, -1421,    24, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421,  -487, -1421, -1421,  -179,     4,    13,
   -1421, -1421, -1421, -1421, -1421, -1421,    15, -1421, -1421, -1421,
   -1421,    16, -1421, -1421,   793,   795,   797,  -124,   315,  -812,
     319,   380,  -491,    97,  -853, -1421,  -235, -1421, -1421, -1421,
   -1421,  -651,   -62, -1421, -1421, -1421, -1421,  -483, -1421,  -545,
   -1421,  -389, -1421, -1421,   682, -1421,  -219, -1421, -1421,  -975,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,
    -247, -1421, -1421, -1421, -1421, -1421,  -330, -1421,   -96, -1016,
   -1421, -1420,  -504, -1421,  -160,    -1,  -134,  -490, -1421,  -334,
   -1421,   -70,   -22,  1167,  -663,  -362, -1421, -1421,   -38, -1421,
   -1421,  3950,    -5,  -203, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421,  -538,  -770, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421, -1421,   827, -1421,
   -1421,   221, -1421,   727, -1421, -1421, -1421, -1421, -1421, -1421,
   -1421,   227, -1421,   731, -1421, -1421,   461, -1421,   192, -1421,
   -1421, -1421, -1421, -1421, -1421, -1421, -1421,  -989, -1421,  2035,
     719,  -345, -1421, -1421,   158,  3711,  4244, -1421, -1421,   280,
    -142,  -585, -1421, -1421,   344,  -655,   146, -1421, -1421, -1421,
   -1421, -1421,   332, -1421, -1421, -1421,    12,  -818,  -194,  -408,
    -406,  -131, -1421, -1421,    14, -1421, -1421, -1421, -1421,    36,
    -166, -1421,  -245, -1421, -1421, -1421,  -392,   936, -1421, -1421,
   -1421, -1421, -1421,   919, -1421, -1421, -1421,   284, -1421, -1421,
   -1421,   481,   297, -1421, -1421,   950,  -303,  -963, -1421,   -41,
     -83,  -197,  -117,   519, -1421, -1001, -1421,   235,   310, -1421,
   -1421, -1421,  -198, -1003
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -890
static const yytype_int16 yytable[] =
{
     121,   346,   390,   135,   419,   308,   417,   556,   129,   846,
     236,   260,   314,   315,  1035,   265,   162,   130,   164,   132,
     133,   241,   892,   830,   670,   245,   673,   439,   128,   829,
     647,  1201,  1204,   627,   412,   692,   443,   693,   389,  1049,
     215,   216,   909,   319,  1025,   248,  1189,  1623,   258,   803,
     346,   342,   691,   464,   465,  1060,  1188,  1097,  1441,   469,
     336,   923,   451,  1254,    13,   290,   229,   924,   508,   513,
     695,  1587,   711,   516,  1393,  -775,    13,  -285,   436,  1445,
    1529,  1036,   306,  1594,   290,  1594,   410,  1441,   531,   709,
     778,   290,   290,   414,   778,  1588,   792,   792,   792,   277,
     444,   532,   792,   510,   792,  1106,  1107,  -772,   321,   316,
     496,  1308,   407,   408,  1005,  1195,  1106,  1107,   760,  1205,
       3,   304,   766,  1402,   305,  1037,   284,   322,    13,   944,
     290,    13,  1077,   332,   188,   590,    13,   631,  1609,   345,
     232,   429,   235,   333,   555,   407,   408,   407,   408,   407,
     408,  1712,  -445,   437,  1603,  1105,  1106,  1107,  -889,   332,
     410,   391,  -773,  1661,  1196,   668,   600,  -774,   497,   671,
    -481,  1314,   242,  -806,  -889,  -622,  1123,   414,   332,   333,
     243,  -776,   278,   543,   544,   269,   575,  -809,   420,  -807,
     297,  1648,   426,  1078,   244,  -621,  -778,  -779,  1604,  -777,
     286,  -808,  1206,   209,   407,   408,  1403,  1662,  1038,  -781,
    1109,  1321,  -775,  -889,   309,  1315,   209,  -704,   521,   639,
    -704,  1257,   802,   411,   324,   430,   309,   317,   925,  -226,
     415,   432,   121,   282,  1713,  1255,   121,   438,  1290,  1323,
     458,   674,  1624,   639,  -772,   712,  1329,  1247,  1331,  1224,
     164,  1442,  1443,   337,   164,   452,   598,   346,   471,  1330,
     448,   509,   514,  1589,   453,   639,   517,  1394,   714,  1104,
    -285,  1108,  1446,  1530,   639,  1006,  1595,   639,  1638,    13,
    1700,   710,   779,   522,  -226,   680,   780,  -210,   793,   879,
    1181,   283,  -704,   512,  1347,   596,  1396,   411,  1316,  -773,
     518,   518,   523,  1046,  -774,   308,   342,   528,   279,  1234,
    -806,  1018,  1726,   537,   415,   502,   506,   507,  -776,   121,
     906,  1410,   135,  1412,  -809,   908,  -807,   129,   587,   416,
     571,  -782,   258,  -778,  -779,   290,  -777,   164,  -808,  1291,
     295,   809,   116,  1705,  1292,   541,    58,    59,    60,   176,
     177,   343,  1293,  1719,   648,  1582,   295,  1418,   681,   770,
     378,   538,   771,   407,   408,   287,   826,   827,   295,   288,
     994,  1583,   379,   538,   834,   997,  1251,  1106,  1107,   836,
     295,   637,   290,   641,   290,   290,   289,  1706,  1584,  1294,
    1295,   293,  1296,   295,   644,  1727,   295,  1720,   325,  1011,
    1190,   296,   785,   786,   295,   664,   533,   298,   299,   328,
     815,   698,  1558,  1191,   229,   332,   344,   820,   332,   294,
     824,   638,   390,   298,   299,  1237,   332,   683,  1039,   310,
    1040,   332,   528,  1244,   331,   298,   299,   689,  1297,   664,
     863,   295,   430,   311,   810,   667,   538,   298,   299,  1192,
     121,    58,    59,    60,   176,   177,   343,   312,   389,   811,
     298,   299,   297,   298,   299,   869,   338,   638,   164,   873,
    1707,   298,   299,   313,  -889,  1057,   690,  1290,   703,   667,
    1721,   332,   295,  1590,  1632,   585,   863,   538,   836,   903,
     904,  1086,   861,  1087,   899,   333,  1203,  1357,   586,   335,
    1591,  1633,  1213,  1592,  1634,  1215,   773,   539,   298,   299,
    -479,   556,  1088,  1089,   975,   976,   977,   853,    13,  1368,
    1369,   344,   590,   590,    51,  1063,  -889,   797,   799,  -889,
     978,   384,    58,    59,    60,   176,   177,   343,   347,   439,
    1103,  1682,   763,   375,   376,   377,   767,   378,   348,   298,
     299,   277,  1686,  1687,  1688,  1554,  1555,   900,   381,   379,
    1611,   349,    58,    59,    60,   176,   177,   343,  1701,  1702,
    1422,  1630,  1631,  1335,   350,  1336,   929,   351,  1291,   352,
     571,   391,   382,  1292,   383,    58,    59,    60,   176,   177,
     343,  1293,   413,   534,  1626,  1627,   290,   540,   327,   329,
     330,   385,   344,   932,   935,   842,  -780,  1019,  -480,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   534,  -621,   540,   534,   540,   540,  1294,  1295,
    1031,  1296,   344,   418,   302,   372,   373,   374,   375,   376,
     377,  1041,   378,   556,   423,   425,   379,   333,   555,   431,
     434,   435,  1183,  -620,   379,   344,   440,   441,   405,   406,
     871,   449,   462,  1313,   466,  -884,   467,  1219,   470,   472,
     639,  1325,   473,   890,   475,   895,  1227,  1307,   524,   907,
     515,   525,  1695,  1421,   564,   573,  1697,   565,   574,   584,
     -65,   121,    51,   597,   135,   679,   677,  1708,   896,   129,
     897,  1711,   701,   682,   688,   916,   121,  1246,   451,   164,
     972,   973,   974,   975,   976,   977,   705,   708,   720,   721,
     746,   914,   407,   408,   164,   747,  1110,   749,  1111,   978,
     476,   477,   478,   639,   918,   750,   761,   479,   480,   758,
     764,   481,   482,   765,   769,   214,   214,   768,   777,   226,
     781,  1081,   782,   789,   784,   791,   121,   794,   800,   135,
     805,   806,   996,   813,   129,  1562,   999,  1000,   808,   814,
     816,   817,   818,   819,   164,   822,   556,   823,   825,   828,
     555,   831,   832,  -482,  1007,   839,  1062,   841,  1419,   845,
    1304,   847,  1003,  1116,   844,   860,  1286,   850,   864,   121,
    1120,   856,   135,   857,   874,   528,  1013,   129,   502,   859,
     876,   877,   506,  1026,   901,   162,   130,   164,   132,   133,
     852,   910,   926,   920,   922,   927,   928,   128,   930,  1327,
     943,   945,   664,   946,   952,  1202,   947,   834,   948,   949,
     982,  1675,   951,  1209,   571,   980,   981,   986,  1290,   989,
     992,  1002,  1004,  1014,  1023,  1010,  1015,  1030,  1675,  1033,
     571,  1222,  1047,  1343,  1021,   290,  1696,  1056,  1065,  1032,
    1066,  1059,   667,  1076,  1079,  1080,  1070,  1070,   890,  1352,
    1082,  1091,  1094,  1096,  1099,  1100,  1102,  1114,  1115,    13,
     978,  1119,  1118,   533,  1172,   664,  1178,  1179,  1182,  1200,
     121,  1198,   121,  1207,   121,   135,   229,   135,  1612,  1208,
     129,  1214,   129,   555,  1212,  1216,  1041,  1220,   164,  1228,
     164,  1218,   164,  1112,   914,  1098,  1221,  1223,  1092,  1229,
     214,  1407,  1230,  1231,  1233,   667,   214,  1240,  1241,  1243,
    1248,  1122,   214,  1252,  1258,  1259,  1261,  1135,  1262,  1291,
    1265,  1266,  1267,  1269,  1292,  1270,    58,    59,    60,   176,
     177,   343,  1293,  1271,   556,  1423,    58,    59,    60,    61,
      62,   343,  1290,  1275,  1429,  1279,  1184,    68,   386,  1274,
    1288,  1280,  1268,  1277,  1285,  1305,  1272,  1306,  1436,  1276,
    1174,  1317,  1318,  1175,  1320,  1332,  1281,  1322,   214,  1294,
    1295,  1333,  1296,  1324,  1328,   214,   214,  1334,  1337,  1338,
    1339,  1340,   214,    13,   121,   388,  1342,   135,   214,  1345,
    1644,  1344,   129,  1349,  1346,  1367,   344,  1365,  1382,   554,
     162,   130,   164,   132,   133,  1350,   344,   556,  1398,  1404,
    1405,  1408,   128,  1413,  1414,  1440,  1420,  1430,  1411,  1568,
    1416,  1290,  1444,  1432,  1537,  1538,  1210,  1548,  1549,   571,
    1551,  1353,   571,  1354,  1552,  1553,  1239,  1563,    33,    34,
      35,  1561,   890,  1291,  1593,  1574,   890,  1575,  1292,   727,
      58,    59,    60,   176,   177,   343,  1293,   121,  1598,  1685,
    1601,  1628,    13,  1600,  1341,  1608,  1636,   226,  1238,   121,
    1637,   555,   135,  1392,  1643,   164,  1642,   129,  1650,  1651,
    -281,  1652,  1655,   528,   914,  1242,  1657,   164,  1654,  1588,
    1658,  1660,  1665,  1294,  1295,  1666,  1296,    72,    73,    74,
      75,    76,  1667,  1663,  1677,  1683,  1680,   214,   729,  1684,
    1692,  1694,  1698,  1395,    79,    80,  1699,   214,  1714,  1709,
     344,  1715,  1291,  1722,  1723,  1728,  1729,  1292,    89,    58,
      59,    60,   176,   177,   343,  1293,  1731,   346,  1732,  1437,
     995,   772,  1415,  1287,   555,    97,  1606,   998,  1607,  1301,
    1679,   640,   643,  1061,  1058,   642,  1020,  1613,  1301,  1693,
    1567,  1351,  1303,  1691,  1245,   775,   528,  1559,  1581,  1586,
    1389,  1303,  1294,  1295,  1716,  1296,  1704,  1597,   238,  1557,
    1171,   756,  1540,  1169,  1193,   757,   212,   212,   988,   650,
     224,  1226,  1232,  1121,   890,  1072,   890,  1083,   571,   344,
     530,  1647,   563,  1132,   353,   354,   355,   520,  1113,   933,
    1177,     0,   224,     0,     0,     0,     0,     0,     0,     0,
    1370,  1417,     0,   356,     0,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,     0,     0,
       0,     0,   121,     0,     0,   135,     0,   258,     0,   379,
     129,     0,  1387,     0,     0,     0,     0,     0,     0,     0,
     164,     0,   391,     0,  1391,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1301,     0,     0,  1710,     0,     0,  1301,     0,
    1301,     0,  1717,     0,   890,  1303,     0,     0,     0,   121,
       0,  1303,   135,  1303,   121,   571,     0,   129,   121,     0,
    1290,   135,  1542,     0,     0,     0,   129,   164,     0,     0,
       0,     0,   164,     0,  1599,   214,   164,     0,     0,     0,
       0,     0,  1427,     0,     0,     0,  1526,     0,     0,     0,
       0,     0,  1533,     0,     0,     0,     0,     0,     0,   258,
       0,    13,     0,   258,     0,     0,     0,     0,     0,  1543,
       0,   212,     0,   214,     0,   214,     0,   212,     0,     0,
       0,     0,     0,   212,     0,     0,   890,  1301,     0,   121,
     121,   121,   135,     0,     0,   121,   214,   129,   135,     0,
    1303,  1186,   121,   129,     0,   135,     0,   164,   164,   164,
     129,   224,   224,   164,     0,     0,     0,   224,  1565,  1427,
     164,  1291,     0,     0,     0,     0,  1292,  1596,    58,    59,
      60,   176,   177,   343,  1293,     0,     0,     0,     0,   212,
      58,    59,    60,    61,    62,   343,   212,   212,     0,     0,
       0,    68,   386,   212,     0,     0,     0,     0,     0,   212,
       0,   834,     0,     0,     0,  1670,     0,   214,     0,     0,
     224,  1294,  1295,     0,  1296,     0,   834,     0,     0,     0,
     214,   214,     0,     0,     0,     0,  1640,   387,     0,   388,
       0,     0,     0,   224,     0,     0,   224,     0,   344,     0,
       0,     0,     0,   554,     0,     0,   290,   346,     0,     0,
     344,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1560,     0,     0,     0,     0,   258,     0,     0,     0,   890,
       0,     0,     0,     0,   121,     0,    36,   135,   224,     0,
       0,     0,   129,     0,  1618,     0,     0,     0,     0,  1526,
    1526,     0,   164,  1533,  1533,     0,     0,     0,     0,   226,
       0,     0,     0,     0,     0,   290,     0,     0,     0,     0,
       0,     0,   121,   121,     0,   135,   135,     0,   212,   121,
     129,   129,   135,     0,     0,     0,     0,   129,   212,     0,
     164,   164,     0,     0,     0,     0,     0,   164,     0,   214,
     214,     0,     0,     0,     0,     0,     0,     0,   180,     0,
       0,    81,     0,   121,    83,    84,   135,    85,   181,    87,
    1669,   129,     0,     0,  1671,     0,   224,   224,     0,     0,
     736,   164,     0,     0,     0,   554,  1681,     0,     0,     0,
       0,   571,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   571,     0,
       0,     0,     0,  1617,     0,     0,   571,     0,     0,     0,
       0,     0,     0,     0,     0,   736,   121,     0,     0,   135,
       0,     0,     0,   121,   129,     0,   135,     0,     0,     0,
       0,   129,     0,     0,   164,     0,     0,     0,     0,     0,
       0,   164,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,   224,   224,     0,     0,     0,     0,
       0,   214,   224,     0,   379,   421,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,     0,     0,
       0,   212,   421,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,     0,     0,     0,   554,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   405,   406,     0,     0,   214,   214,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,   405,   406,     0,     0,     0,   212,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,   379,   212,     0,   407,   408,
       0,     0,     0,  -890,  -890,  -890,  -890,   970,   971,   972,
     973,   974,   975,   976,   977,   407,   408,   212,   736,     0,
       0,   214,     0,     0,     0,   353,   354,   355,   978,     0,
     224,   224,   736,   736,   736,   736,   736,     0,     0,     0,
       0,     0,     0,     0,   356,   736,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,   224,   575,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,   212,   783,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,   212,   212,     0,     0,     0,   554,     0,     0,     0,
       0,     0,     0,     0,     0,   224,   224,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
    1067,  1068,  1069,    36,   801,     0,     0,     0,   224,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,     0,     0,     0,   736,     0,     0,
     224,     0,     0,     0,     0,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   554,
     224,   213,   213,     0,     0,   225,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,    83,    84,     0,    85,   181,    87,     0,     0,   838,
     212,   212,   379,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,   224,     0,   224,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   736,     0,     0,   224,   736,   736,   736,
       0,     0,   736,   736,   736,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,     0,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   356,   736,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,     0,     0,     0,
       0,     0,     0,   224,     0,   224,     0,   379,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,   875,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,   213,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   224,
     378,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,   379,     0,     0,     0,     0,     0,     0,   212,
     212,     0,   736,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,   736,     0,   736,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,   213,   213,     0,     0,     0,     0,     0,   213,     0,
       0,     0,   736,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   878,     0,   224,   224,
       0,   356,   212,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,   736,     0,   353,
     354,   355,     0,     0,     0,     0,     0,   379,     0,     0,
       0,     0,     0,   225,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,   224,
       0,   224,     0,   213,   379,     0,   736,   224,     0,     0,
       0,   736,     0,   736,     0,   736,     0,     0,   736,     0,
     736,     0,     0,   736,     0,     0,     0,     0,     0,     0,
       0,   224,   224,     0,   224,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   742,     0,   736,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,     0,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     978,   353,   354,   355,     0,   736,  1001,     0,     0,     0,
     742,     0,     0,     0,     0,     0,     0,   224,   224,   224,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   224,   378,     0,     0,   224,     0,     0,
       0,     0,     0,  1055,   736,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,   259,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   353,   354,   355,     0,     0,
       0,     0,   736,   736,   736,     0,   213,     0,     0,     0,
     736,   224,     0,     0,   356,   224,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
       0,   353,   354,   355,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     356,   213,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,   213,
       0,   213,     0,  -890,  -890,  -890,  -890,   370,   371,   372,
     373,   374,   375,   376,   377,  1064,   378,     0,   736,     0,
       0,     0,   213,   742,     0,     0,     0,     0,   379,   224,
      36,     0,   209,     0,     0,     0,     0,   742,   742,   742,
     742,   742,     0,     0,     0,     0,     0,     0,   224,     0,
     742,     0,   736,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   736,     0,     0,     0,     0,   736,     0,
       0,   736,     0,     0,     0,     0,   991,     0,     0,     0,
       0,     0,     0,     0,   259,   259,     0,     0,     0,  1400,
     259,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1009,     0,   213,   213,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,     0,     0,
       0,  1009,     0,     0,     0,     0,     0,     0,   736,   213,
       0,     0,     0,     0,     0,  1401,   224,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     224,     0,     0,     0,     0,     0,   663,     0,   116,   224,
       0,     0,   742,     0,     0,  1048,   259,     0,     0,   259,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   736,   225,     0,     0,     0,     0,
       0,     0,   736,     0,     0,     0,     0,     0,   736,     0,
       0,   736,     0,     0,     0,     0,     0,     0,     0,     0,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   213,     0,     0,   356,
    1254,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,     0,   742,     0,
       0,   213,   742,   742,   742,   379,     0,   742,   742,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,   742,   742,   742,   742,   742,   742,
     742,   742,   742,   742,     0,     0,     0,     0,     0,   259,
     716,     0,     0,   738,   353,   354,   355,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   742,     0,     0,
       0,     0,     0,   356,     0,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,   738,     0,
       0,     0,   353,   354,   355,     0,     0,   213,     0,   379,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   213,   378,     0,   259,   259,   213,
       0,     0,  1255,     0,     0,   259,     0,   379,     0,     0,
       0,     0,     0,     0,   213,   213,     0,   742,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   742,     0,   742,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   742,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,     0,  1289,     0,   380,   213,     0,     0,
       0,     0,     0,     0,   880,   881,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   742,     0,   882,     0,     0,     0,     0,     0,
       0,     0,   883,   884,   885,    36,     0,     0,     0,     0,
       0,   738,     0,   886,   461,     0,     0,     0,     0,     0,
       0,     0,     0,   259,   259,   738,   738,   738,   738,   738,
       0,     0,     0,     0,     0,     0,     0,     0,   738,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   742,   213,     0,     0,     0,   742,     0,   742,   887,
     742,     0,     0,   742,     0,   742,     0,     0,   742,     0,
       0,     0,   888,     0,     0,     0,     0,  1372,   251,  1381,
       0,     0,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   463,
       0,   889,   742,     0,   252,     0,     0,     0,   259,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   213,    36,     0,     0,     0,
       0,   259,     0,     0,     0,     0,   353,   354,   355,     0,
     742,     0,   259,     0,     0,     0,     0,     0,     0,     0,
     738,     0,     0,  1438,  1439,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
     253,   254,     0,     0,     0,     0,     0,     0,     0,   742,
       0,   379,     0,     0,     0,     0,     0,     0,   180,     0,
       0,    81,   255,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,     0,     0,   742,   742,   742,
       0,     0,   256,     0,     0,   742,  1577,   259,     0,   259,
    1381,   716,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   738,     0,     0,   257,
     738,   738,   738,  1544,     0,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,     0,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,     0,   738,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,   978,   209,     0,
     353,   354,   355,     0,     0,     0,     0,     0,   474,     0,
       0,     0,     0,   742,     0,     0,   259,     0,   259,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   259,   378,     0,     0,     0,   742,     0,     0,
       0,     0,     0,     0,     0,   379,    36,     0,   742,     0,
       0,     0,     0,   742,     0,     0,   742,     0,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,   738,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,     0,     0,
     738,     0,   738,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,   636,   742,   116,   738,     0,     0,     0,     0,
       0,  1678,   340,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,  1372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   259,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   353,   354,   355,   742,
       0,     0,     0,     0,     0,     0,     0,   742,     0,     0,
     738,     0,   498,   742,     0,   356,   742,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259,     0,   259,     0,    36,     0,   209,   738,
       0,     0,     0,     0,   738,     0,   738,     0,   738,     0,
       0,   738,     0,   738,     0,     0,   738,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
     353,   354,   355,     0,   259,     0,     0,     0,     0,     0,
       0,     0,   635,     0,     0,     0,     0,     0,     0,   356,
     738,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,   379,     0,     0,   738,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   675,     0,     0,
       0,     0,   636,     0,   116,     0,   259,     0,     0,     0,
     259,     0,     0,     0,     0,   185,   187,   738,   189,   190,
     191,   193,   194,     0,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,     0,     0,   218,   221,
       0,     0,     0,     0,     0,   738,   738,   738,   953,   954,
     955,   239,     0,   738,     0,     0,     0,     0,   247,     0,
     250,     0,     0,   266,     0,   271,     0,   956,     0,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   251,
       0,   694,     0,   978,     0,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,   356,   252,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,    36,   378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     379,   738,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,  1619,   378,     0,     0,   738,     0,     0,   422,     0,
       0,   253,   254,     0,   379,     0,   738,     0,     0,     0,
       0,   738,     0,     0,   738,     0,     0,     0,     0,   180,
       0,     0,    81,   255,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,     0,     0,     0,  1133,
       0,   743,   446,   256,     0,   446,     0,     0,     0,     0,
       0,     0,   239,   457,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     257,   738,     0,     0,  1610,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   743,     0,     0,     0,
       0,   987,     0,     0,     0,     0,     0,     0,   320,     0,
       0,     0,   259,     0,   218,     0,     0,     0,   536,     0,
       0,     0,     0,     0,     0,     0,     0,   259,     0,     0,
       0,   557,   560,   560,     0,     0,     0,   738,     0,     0,
       0,     0,     0,     0,   583,   738,     0,     0,     0,     0,
       0,   738,     0,     0,   738,   595,     0,    29,    30,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   209,
       0,     0,     0,   601,   602,   603,   605,   606,   607,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   617,   618,
     619,   620,   621,   622,   623,   624,   625,   626,     0,   628,
       0,   629,   629,     0,   632,     0,     0,   210,     0,     0,
       0,     0,   649,   651,   652,   653,   654,   655,   656,   657,
     658,   659,   660,   661,   662,     0,     0,     0,     0,     0,
     629,   669,     0,   595,   629,   672,     0,     0,     0,   180,
       0,   649,    81,    82,   676,    83,    84,   737,    85,   181,
      87,     0,     0,   685,     0,   687,     0,    90,     0,     0,
       0,   595,     0,     0,     0,     0,     0,     0,     0,   699,
       0,   700,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   743,
     428,     0,   737,     0,     0,   116,     0,   748,     0,     0,
     751,   754,   755,   743,   743,   743,   743,   743,     0,     0,
       0,   353,   354,   355,     0,     0,   743,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     356,   774,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,     0,     0,     0,     0,     0,     0,   353,   354,
     355,   379,   843,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   854,     0,     0,   356,   743,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,     0,     0,   862,     0,     0,     0,
       0,     0,     0,   379,     0,   193,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   424,     0,     0,     0,     0,
       0,     0,     0,   872,     0,   737,     0,     0,     0,    36,
       0,     0,     0,   983,   984,     0,     0,     0,     0,   737,
     737,   737,   737,   737,     0,     0,     0,     0,     0,     0,
       0,     0,   737,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   743,   239,     0,     0,   743,   743,
     743,     0,     0,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
       0,     0,     0,     0,   302,     0,   979,    83,    84,   870,
      85,   181,    87,     0,     0,     0,     0,     0,    36,     0,
     209,     0,     0,   743,   744,     0,     0,  1615,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,     0,     0,     0,   303,     0,     0,   210,     0,
    1016,     0,     0,     0,   737,     0,     0,     0,     0,   776,
       0,     0,     0,  1024,     0,     0,     0,     0,     0,  1027,
       0,  1028,     0,  1029,     0,     0,     0,     0,     0,     0,
     180,     0,     0,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,  1044,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1052,     0,     0,  1053,     0,
    1054,     0,     0,   743,   595,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   743,     0,
     743,   211,    36,     0,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1085,     0,
     737,     0,     0,   743,   737,   737,   737,     0,     0,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,     0,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,     0,  1531,   737,
      83,    84,  1532,    85,   181,    87,     0,     0,   743,   911,
     978,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1166,  1167,  1168,     0,     0,     0,   751,  1170,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   915,     0,     0,  1386,     0,     0,     0,     0,
       0,    36,     0,   209,  1185,     0,   936,   937,   938,   939,
       0,     0,     0,     0,     0,     0,     0,   743,     0,   950,
       0,     0,   743,     0,   743,     0,   743,     0,     0,   743,
       0,   743,     0,  1211,   743,     0,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,   595,     0,     0,   737,
       0,     0,     0,     0,   912,   595,  1185,     0,     0,     0,
       0,     0,     0,     0,   737,     0,   737,     0,   743,     0,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,   239,     0,     0,   737,
       0,     0,     0,     0,     0,     0,  1253,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   743,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   211,     0,     0,     0,     0,   116,
       0,  1045,     0,     0,     0,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   737,   743,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,  1309,   743,   743,   743,  1310,     0,  1311,  1312,
       0,   743,   379,     0,     0,  1580,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1326,   595,     0,
       0,     0,     0,   737,     0,     0,     0,     0,   737,     0,
     737,     0,   737,     0,     0,   737,     0,   737,     0,     0,
     737,  1126,  1129,  1129,     0,     0,  1136,  1139,  1140,  1141,
    1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,     0,   737,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1364,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,  1176,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   743,
       0,     0,   737,     0,     0,     0,     0,     0,     0,     0,
     595,     0,     0,     0,     0,  1435,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1385,     0,     0,     0,     0,
       0,     0,     0,   743,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   743,     0,     0,     0,     0,   743,
       0,   737,   743,     0,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1656,     0,     0,     0,     0,   737,
     737,   737,     0,     0,     0,     0,  1249,   737,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,  1263,     0,  1264,  1386,     0,     0,     0,     0,   743,
       0,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1572,  1282,     0,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   743,   379,     0,     0,     0,
       0,     0,     0,   743,     0,     0,     0,     0,     0,   743,
       0,     0,   743,     0,     0,     0,     0,     0,   251,     0,
       0,  1319,     0,     0,     0,   737,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   252,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   978,     0,   737,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
     737,     0,     0,     0,     0,   737,     0,     0,   737,     0,
    1356,     0,     0,     0,     0,  1358,     0,  1359,     0,  1360,
       0,     0,  1361,  -323,  1362,     0,     0,  1363,     0,     0,
       0,    58,    59,    60,   176,   177,   343,     0,     0,     0,
       0,   251,     0,     0,     0,     0,     0,     0,  1284,     0,
     253,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1406,     0,     0,     0,   737,     0,   252,   180,     0,
       0,    81,   255,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   251,   256,     0,     0,     0,     0,     0,     0,  1431,
       0,   344,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   468,   252,     0,   257,
       0,   737,     0,     0,     0,     0,     0,     0,     0,   737,
       0,     0,     0,     0,     0,   737,     0,     0,   737,    36,
       0,     0,     0,   253,   254,     0,     0,     0,  1550,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,   180,     0,     0,    81,   255,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,  1569,  1570,  1571,     0,
       0,     0,     0,     0,  1576,   256,     0,     0,     0,     0,
       0,     0,     0,   253,   254,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   180,   257,     0,    81,   255,     0,    83,    84,     0,
      85,   181,    87,     0,   931,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   256,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   257,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   597,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1602,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,  1625,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,  1635,     0,     0,
       0,     0,  1639,     0,     0,  1641,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,  1672,     0,     0,    48,    49,    50,    51,    52,
      53,    54,     0,    55,    56,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,    78,    79,    80,    81,    82,  1724,    83,
      84,     0,    85,    86,    87,    88,  1730,     0,    89,     0,
       0,    90,  1733,     0,     0,  1734,     0,    91,    92,    93,
      94,    95,     0,    96,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1017,   116,
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
       0,     0,    48,    49,    50,    51,    52,    53,    54,     0,
      55,    56,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,    67,    68,    69,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
      78,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1187,   116,   117,     0,   118,
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
      49,    50,    51,    52,    53,    54,     0,    55,    56,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,    78,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,    95,     0,    96,     0,    97,
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
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,   576,
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
       0,     0,   113,     0,   114,   115,   990,   116,   117,     0,
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
       0,   114,   115,  1093,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,  1095,    44,     0,    45,     0,
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
      42,    43,     0,    44,     0,    45,     0,    46,  1250,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1366,   116,   117,     0,   118,   119,
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
     115,  1573,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,  1614,    46,     0,
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
       0,   113,     0,   114,   115,  1645,   116,   117,     0,   118,
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
     114,   115,  1646,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,  1649,    45,     0,    46,
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
       0,     0,   113,     0,   114,   115,  1664,   116,   117,     0,
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
       0,   114,   115,  1718,   116,   117,     0,   118,   119,     5,
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
    1725,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     447,     0,     0,   421,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,   405,   406,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   176,   177,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,   407,   408,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   702,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   176,   177,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   917,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,   176,
     177,    63,     0,    64,    65,    66,     0,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,  1426,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,  1564,     0,
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
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   645,
      12,     0,     0,     0,     0,     0,     0,   646,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
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
       0,     0,     0,    97,    98,   263,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
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
      97,    98,   263,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   264,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   356,    10,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   591,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   379,     0,     0,     0,    16,     0,
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
       0,    83,    84,     0,    85,   181,    87,     0,   592,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
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
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,   354,   355,   697,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   356,    10,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,  1042,   378,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   379,     0,
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
      87,     0,  1043,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   645,    12,     0,     0,     0,     0,
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
       0,     0,     0,   192,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,   954,   955,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   956,    10,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   217,     0,     0,     0,     0,     0,
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
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,    94,     0,    10,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     0,
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
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
      94,     0,    10,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   246,     0,     0,   116,
     117,     0,   118,   119,     0,    14,    15,     0,     0,     0,
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
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,    94,     0,    10,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   249,     0,     0,   116,   117,     0,   118,   119,     0,
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
     111,   112,     0,     0,   113,   445,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  -890,  -890,  -890,  -890,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   604,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,    14,    15,     0,
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
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   355,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     356,    10,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   646,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   379,     0,     0,     0,
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
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     955,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,   956,    10,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     684,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,   978,     0,     0,     0,    16,     0,    17,
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
       0,     0,     0,     0,   956,    10,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   686,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     978,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,     0,    10,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,  1084,   378,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   379,     0,     0,
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
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,    94,     0,    10,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     0,
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
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
      94,     0,    10,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,   535,    38,     0,
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
       0,     0,     0,     0,   116,   117,     0,   118,   119,  1447,
    1448,  1449,  1450,  1451,     0,     0,  1452,  1453,  1454,  1455,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1456,  1457,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,     0,     0,  1458,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,  1459,  1460,  1461,  1462,  1463,  1464,  1465,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,  1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,
    1474,  1475,  1476,  1477,  1478,  1479,  1480,  1481,  1482,  1483,
    1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,
    1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,
    1504,  1505,  1506,   251,     0,     0,  1507,  1508,     0,  1509,
    1510,  1511,  1512,  1513,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1514,  1515,  1516,     0,   252,
       0,    83,    84,     0,    85,   181,    87,  1517,     0,  1518,
    1519,     0,  1520,     0,     0,     0,     0,     0,     0,  1521,
       0,    36,   251,  1522,     0,  1523,     0,  1524,  1525,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,     0,     0,   252,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,     0,     0,
      36,     0,     0,     0,     0,   253,   254,     0,     0,   379,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   180,     0,     0,    81,   255,     0,    83,
      84,     0,    85,   181,    87,     0,  1260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   256,     0,     0,
       0,     0,     0,     0,   253,   254,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   180,     0,   257,    81,   255,     0,    83,    84,
       0,    85,   181,    87,     0,    36,     0,   209,     0,     0,
       0,     0,     0,   549,     0,     0,   256,     0,     0,     0,
       0,     0,     0,     0,  1142,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     722,   723,     0,   257,     0,   210,   724,     0,   725,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     726,     0,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,     0,     0,     0,     0,     0,   180,     0,   727,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   728,     0,    72,    73,    74,
      75,    76,     0,   550,    36,     0,   209,     0,   729,     0,
       0,     0,     0,   180,    79,    80,    81,   730,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,     0,     0,     0,     0,     0,     0,   731,   732,   733,
     734,     0,     0,     0,   210,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   722,   723,     0,   735,     0,     0,   724,     0,   725,
       0,     0,     0,     0,     0,     0,   180,     0,     0,    81,
      82,   726,    83,    84,     0,    85,   181,    87,     0,    33,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
     727,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   211,     0,     0,
     511,     0,   116,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,   209,     0,     0,   728,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,   729,
       0,     0,     0,     0,   180,    79,    80,    81,   730,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,   210,     0,     0,     0,     0,     0,     0,   731,   732,
     733,   734,     0,     0,   527,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   180,     0,   735,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    36,     0,   209,     0,     0,
       0,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   978,     0,     0,   211,   210,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,  1012,    36,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   210,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,   209,   211,     0,
       0,   180,     0,   116,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   222,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      83,    84,   211,    85,   181,    87,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   852,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   353,   354,   355,   223,     0,
       0,     0,     0,   116,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   433,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     0,     0,     0,     0,     0,   821,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   858,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,   379,     0,     0,     0,     0,     0,     0,   898,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,   953,   954,
     955,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1199,   956,     0,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     953,   954,   955,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   978,     0,     0,     0,     0,     0,   956,
    1217,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   978,     0,   953,   954,   955,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   956,  1117,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   953,
     954,   955,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   978,     0,     0,     0,     0,     0,   956,  1273,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   978,     0,   953,   954,   955,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   956,  1278,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   953,   954,
     955,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   978,     0,    36,     0,     0,     0,   956,  1355,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   978,  1373,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,  1374,  1375,     0,
       0,     0,    36,     0,     0,  1433,     0,     0,     0,     0,
     715,     0,     0,     0,     0,   180,   272,   273,    81,  1376,
       0,    83,    84,     0,    85,  1377,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,  1434,   795,   796,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,   274,     0,     0,    83,    84,     0,
      85,   181,    87,     0,   180,     0,     0,    81,     0,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    36,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,   180,    36,     0,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,   499,     0,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,   503,     0,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   635,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,     0,   274,     0,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      36,     0,     0,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,  1134,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,     0,   978,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   706,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,   353,   354,   355,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,   356,   855,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   707,   378,   353,   354,
     355,     0,     0,     0,     0,     0,     0,     0,     0,   379,
       0,     0,     0,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,   953,   954,   955,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,     0,     0,
       0,   956,  1283,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   953,   954,   955,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   978,     0,     0,
       0,     0,     0,   956,     0,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   978,
    -890,  -890,  -890,  -890,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   978
};

static const yytype_int16 yycheck[] =
{
       4,   135,   162,     4,   183,    88,   172,   310,     4,   594,
      32,    52,    95,    96,   832,    53,     4,     4,     4,     4,
       4,    43,   677,   568,   413,    47,   418,   224,     4,   567,
     392,  1032,  1035,   378,   165,   443,   230,   443,   162,   851,
      26,    27,   705,   113,   814,    49,  1021,     9,    52,   542,
     184,   134,   441,   251,   252,   867,  1019,   910,     9,   257,
       9,   712,     9,    30,    45,    69,    30,     9,     9,     9,
     447,     9,    30,     9,     9,    66,    45,     9,   220,     9,
       9,    35,    86,     9,    88,     9,    66,     9,    66,     9,
       9,    95,    96,    66,     9,    33,     9,     9,     9,    56,
     231,   295,     9,    96,     9,    99,   100,    66,   113,    79,
     108,    50,   127,   128,     9,   154,    99,   100,   495,    79,
       0,    78,    96,    79,    81,    79,    79,   113,    45,    84,
     134,    45,    84,   152,   200,   333,    45,   382,  1558,   135,
     200,   211,   200,   169,   310,   127,   128,   127,   128,   127,
     128,    79,     8,   223,    35,    98,    99,   100,   148,   152,
      66,   162,    66,    35,   203,   410,   345,    66,   166,   414,
      66,    35,   200,    66,   200,   148,   946,    66,   152,   169,
     200,    66,    79,   202,   203,   203,   201,    66,   184,    66,
     145,  1611,   196,   145,   200,   148,    66,    66,    79,    66,
     200,    66,   162,    79,   127,   128,   162,    79,   162,   200,
     204,  1212,   203,   203,   153,    79,    79,   198,   288,   385,
     201,   204,   204,   203,   203,   211,   153,   197,   170,   198,
     203,   217,   236,   123,   162,   202,   240,   223,     4,  1214,
     244,   420,   204,   409,   203,   203,  1221,  1100,  1223,  1061,
     236,   202,   203,   202,   240,   202,   339,   391,   262,  1222,
     236,   202,   202,   201,   240,   431,   202,   202,   466,   920,
     202,   922,   202,   202,   440,   170,   202,   443,   202,    45,
     202,   201,   201,   288,   201,   427,   201,   201,   201,   201,
     201,   123,   201,   279,   201,   336,   201,   203,   162,   203,
     286,   287,   288,   848,   203,   388,   389,   293,   205,  1079,
     203,   804,    79,   299,   203,   272,   273,   274,   203,   323,
     697,  1322,   323,  1324,   203,   702,   203,   323,   332,   205,
     318,   200,   336,   203,   203,   339,   203,   323,   203,   105,
      79,    50,   205,    35,   110,   302,   112,   113,   114,   115,
     116,   117,   118,    35,   392,    14,    79,  1332,   428,    96,
      53,    84,    96,   127,   128,   200,   564,   565,    79,   200,
      96,    30,    65,    84,   572,    96,    98,    99,   100,   573,
      79,   385,   386,   387,   388,   389,   200,    79,    47,   155,
     156,   200,   158,    79,   390,   162,    79,    79,    84,   791,
     154,    84,    46,    47,    79,   409,   145,   146,   147,    84,
     552,   449,  1413,   167,   378,   152,   182,   559,   152,   200,
     562,   385,   582,   146,   147,  1080,   152,   431,   836,   200,
     836,   152,   418,  1096,    30,   146,   147,   201,   204,   443,
     637,    79,   428,   200,   153,   409,    84,   146,   147,   203,
     454,   112,   113,   114,   115,   116,   117,   200,   582,   168,
     146,   147,   145,   146,   147,   644,    35,   431,   454,   666,
     162,   146,   147,   200,   148,   864,   440,     4,   454,   443,
     162,   152,    79,    29,    29,   208,   683,    84,   682,    71,
      72,   899,   634,   899,   688,   169,  1034,  1267,   209,   200,
      46,    46,  1047,    49,    49,  1050,   510,   145,   146,   147,
      66,   814,    71,    72,    49,    50,    51,   600,    45,   125,
     126,   182,   720,   721,   104,   870,   200,   531,   532,   203,
      65,   203,   112,   113,   114,   115,   116,   117,   202,   736,
     917,   202,   499,    49,    50,    51,   503,    53,   202,   146,
     147,   508,   112,   113,   114,   202,   203,   688,    66,    65,
    1561,   202,   112,   113,   114,   115,   116,   117,   202,   203,
    1340,  1587,  1588,  1228,   202,  1230,   718,   202,   105,   202,
     568,   582,    66,   110,   202,   112,   113,   114,   115,   116,
     117,   118,   200,   296,  1583,  1584,   600,   300,   117,   118,
     119,   148,   182,   720,   721,   591,   200,   805,    66,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   325,   148,   327,   328,   329,   330,   155,   156,
     828,   158,   182,   200,   152,    46,    47,    48,    49,    50,
      51,   839,    53,   946,   202,    44,    65,   169,   814,   148,
     207,     9,  1014,   148,    65,   182,   148,   200,    63,    64,
     646,     8,   202,  1201,   169,    14,   200,  1056,    14,    79,
     836,  1216,   202,   677,   202,   679,  1065,   204,   201,   701,
      14,   169,  1683,  1338,    14,   201,  1689,    96,   201,   206,
     200,   695,   104,   200,   695,     9,   200,  1698,   684,   695,
     686,  1704,    88,   201,   201,   709,   710,  1099,     9,   695,
      46,    47,    48,    49,    50,    51,   202,    14,   200,     9,
     186,   707,   127,   128,   710,    79,   924,    79,   926,    65,
     183,   184,   185,   899,   710,    79,   200,   190,   191,   189,
     202,   194,   195,     9,     9,    26,    27,   202,    79,    30,
     201,   893,   201,   125,   202,   200,   760,   201,    66,   760,
      30,   126,   766,   129,   760,  1420,   770,   771,   168,     9,
     201,   148,   201,     9,   760,   201,  1079,     9,   201,    14,
     946,   198,     9,    66,   788,     9,   869,   170,  1333,     9,
    1182,    14,   778,   935,   201,     9,  1173,   125,   200,   803,
     942,   207,   803,   207,   207,   791,   792,   803,   765,   204,
     201,   207,   769,   817,   201,   803,   803,   803,   803,   803,
     200,    96,   129,   202,   202,   148,     9,   803,   201,  1218,
     200,   148,   836,   200,   203,  1033,   200,  1035,   200,   200,
      14,  1659,   148,  1040,   832,   186,   186,     9,     4,    79,
     203,    14,   202,    14,    14,   203,   207,   202,  1676,    30,
     848,  1059,   200,  1240,   203,   869,  1684,   200,   200,   198,
      14,    30,   836,   200,   200,     9,   880,   881,   882,  1256,
     201,   903,   202,   202,   200,   129,    14,     9,   201,    45,
      65,     9,   207,   145,    79,   899,    96,     9,   200,   202,
     904,   129,   906,    14,   908,   906,   870,   908,  1563,    79,
     906,   203,   908,  1079,   201,   200,  1114,   201,   904,   129,
     906,   200,   908,   927,   910,   911,   203,   203,   904,   207,
     211,  1320,     9,  1075,   145,   899,   217,    30,    73,   202,
     201,   945,   223,   202,   170,   129,    30,   951,   201,   105,
     201,   129,     9,   201,   110,   201,   112,   113,   114,   115,
     116,   117,   118,     9,  1267,  1342,   112,   113,   114,   115,
     116,   117,     4,     9,  1351,   204,  1014,   123,   124,   201,
    1178,     9,  1124,   201,   201,   204,  1128,   203,  1365,  1131,
     994,    14,    79,   997,   200,   203,  1138,   201,   279,   155,
     156,   200,   158,   201,   201,   286,   287,   201,   201,   129,
     201,     9,   293,    45,  1018,   161,    30,  1018,   299,   201,
    1605,   202,  1018,   202,   201,    96,   182,   203,   157,   310,
    1018,  1018,  1018,  1018,  1018,   202,   182,  1340,   153,    14,
      79,   110,  1018,   201,   201,    14,   129,   201,   204,  1426,
     203,     4,   203,   129,   202,    79,  1042,    14,    79,  1047,
     201,  1259,  1050,  1261,   200,   203,  1088,   129,    74,    75,
      76,   201,  1076,   105,    14,   202,  1080,   202,   110,    85,
     112,   113,   114,   115,   116,   117,   118,  1091,    14,  1674,
      14,    55,    45,   202,  1236,   203,    79,   378,  1084,  1103,
     200,  1267,  1103,  1301,     9,  1091,    79,  1103,   202,    79,
      96,   108,    96,  1099,  1100,  1091,   160,  1103,   148,    33,
      14,   200,   202,   155,   156,   200,   158,   133,   134,   135,
     136,   137,   166,   201,    79,   201,   163,   418,   144,     9,
      79,   202,   201,  1303,   150,   151,   201,   428,    14,   203,
     182,    79,   105,    14,    79,    14,    79,   110,   164,   112,
     113,   114,   115,   116,   117,   118,    14,  1301,    79,  1367,
     765,   508,   204,  1177,  1340,   181,  1553,   769,  1555,  1180,
    1667,   386,   389,   868,   865,   388,   806,  1564,  1189,  1680,
    1425,  1253,  1180,  1676,  1097,   513,  1182,  1416,  1445,  1529,
    1296,  1189,   155,   156,  1708,   158,  1696,  1541,    41,  1412,
     989,   484,  1391,   986,  1022,   484,    26,    27,   757,   392,
      30,  1063,  1076,   943,  1228,   881,  1230,   895,  1216,   182,
     294,  1608,   313,   949,    10,    11,    12,   287,   928,   720,
    1005,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1291,   204,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,  1286,    -1,    -1,  1286,    -1,  1291,    -1,    65,
    1286,    -1,  1296,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1286,    -1,  1303,    -1,  1300,    -1,    -1,    -1,    -1,    -1,
     591,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1323,    -1,    -1,  1702,    -1,    -1,  1329,    -1,
    1331,    -1,  1709,    -1,  1338,  1323,    -1,    -1,    -1,  1343,
      -1,  1329,  1343,  1331,  1348,  1333,    -1,  1343,  1352,    -1,
       4,  1352,  1393,    -1,    -1,    -1,  1352,  1343,    -1,    -1,
      -1,    -1,  1348,    -1,  1543,   646,  1352,    -1,    -1,    -1,
      -1,    -1,  1348,    -1,    -1,    -1,  1380,    -1,    -1,    -1,
      -1,    -1,  1386,    -1,    -1,    -1,    -1,    -1,    -1,  1393,
      -1,    45,    -1,  1397,    -1,    -1,    -1,    -1,    -1,  1395,
      -1,   211,    -1,   684,    -1,   686,    -1,   217,    -1,    -1,
      -1,    -1,    -1,   223,    -1,    -1,  1420,  1418,    -1,  1423,
    1424,  1425,  1423,    -1,    -1,  1429,   707,  1423,  1429,    -1,
    1418,   207,  1436,  1429,    -1,  1436,    -1,  1423,  1424,  1425,
    1436,   251,   252,  1429,    -1,    -1,    -1,   257,  1424,  1425,
    1436,   105,    -1,    -1,    -1,    -1,   110,  1540,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,   279,
     112,   113,   114,   115,   116,   117,   286,   287,    -1,    -1,
      -1,   123,   124,   293,    -1,    -1,    -1,    -1,    -1,   299,
      -1,  1689,    -1,    -1,    -1,  1655,    -1,   778,    -1,    -1,
     310,   155,   156,    -1,   158,    -1,  1704,    -1,    -1,    -1,
     791,   792,    -1,    -1,    -1,    -1,  1599,   159,    -1,   161,
      -1,    -1,    -1,   333,    -1,    -1,   336,    -1,   182,    -1,
      -1,    -1,    -1,   814,    -1,    -1,  1540,  1671,    -1,    -1,
     182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     204,    -1,    -1,    -1,    -1,  1559,    -1,    -1,    -1,  1563,
      -1,    -1,    -1,    -1,  1568,    -1,    77,  1568,   378,    -1,
      -1,    -1,  1568,    -1,  1578,    -1,    -1,    -1,    -1,  1583,
    1584,    -1,  1568,  1587,  1588,    -1,    -1,    -1,    -1,   870,
      -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,    -1,    -1,
      -1,    -1,  1606,  1607,    -1,  1606,  1607,    -1,   418,  1613,
    1606,  1607,  1613,    -1,    -1,    -1,    -1,  1613,   428,    -1,
    1606,  1607,    -1,    -1,    -1,    -1,    -1,  1613,    -1,   910,
     911,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,  1647,   155,   156,  1647,   158,   159,   160,
    1654,  1647,    -1,    -1,  1655,    -1,   466,   467,    -1,    -1,
     470,  1647,    -1,    -1,    -1,   946,  1670,    -1,    -1,    -1,
      -1,  1659,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,  1676,    -1,
      -1,    -1,    -1,   204,    -1,    -1,  1684,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   515,  1710,    -1,    -1,  1710,
      -1,    -1,    -1,  1717,  1710,    -1,  1717,    -1,    -1,    -1,
      -1,  1717,    -1,    -1,  1710,    -1,    -1,    -1,    -1,    -1,
      -1,  1717,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,   564,   565,    -1,    -1,    -1,    -1,
      -1,  1042,   572,    -1,    65,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,   591,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,  1079,    -1,
      -1,    -1,    -1,  1084,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    -1,    -1,  1099,  1100,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    -1,    -1,    -1,   646,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   684,    65,   686,    -1,   127,   128,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   127,   128,   707,   708,    -1,
      -1,  1182,    -1,    -1,    -1,    10,    11,    12,    65,    -1,
     720,   721,   722,   723,   724,   725,   726,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   735,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,   761,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   778,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   789,
      -1,   791,   792,    -1,    -1,    -1,  1267,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   805,   806,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   814,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,   204,    -1,    -1,    -1,   828,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   839,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   847,    -1,    -1,
     850,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1340,
     870,    26,    27,    -1,    -1,    30,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,   155,   156,    -1,   158,   159,   160,    -1,    -1,   204,
     910,   911,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   924,    -1,   926,    -1,   928,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   943,    -1,    -1,   946,   947,   948,   949,
      -1,    -1,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1002,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1033,    -1,  1035,    -1,    65,    -1,    -1,
      -1,    -1,  1042,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1059,
      -1,    -1,   217,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1079,
      53,    -1,    -1,    -1,  1084,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,  1099,
    1100,    -1,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1114,    -1,    -1,  1117,    -1,  1119,
      -1,    -1,    -1,    -1,   279,    -1,    -1,    -1,    -1,    -1,
      -1,   286,   287,    -1,    -1,    -1,    -1,    -1,   293,    -1,
      -1,    -1,  1142,    -1,   299,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   310,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,  1178,  1179,
      -1,    29,  1182,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,  1207,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,   378,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,  1259,
      -1,  1261,    -1,   418,    65,    -1,  1266,  1267,    -1,    -1,
      -1,  1271,    -1,  1273,    -1,  1275,    -1,    -1,  1278,    -1,
    1280,    -1,    -1,  1283,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1291,  1292,    -1,  1294,    -1,    -1,    -1,    -1,    -1,
      -1,  1301,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   470,    -1,  1317,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
    1340,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    10,    11,    12,    -1,  1355,   204,    -1,    -1,    -1,
     515,    -1,    -1,    -1,    -1,    -1,    -1,  1367,  1368,  1369,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1393,    53,    -1,    -1,  1397,    -1,    -1,
      -1,    -1,    -1,   204,  1404,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,  1432,  1433,  1434,    -1,   591,    -1,    -1,    -1,
    1440,  1441,    -1,    -1,    29,  1445,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   646,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,   684,
      -1,   686,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   204,    53,    -1,  1548,    -1,
      -1,    -1,   707,   708,    -1,    -1,    -1,    -1,    65,  1559,
      77,    -1,    79,    -1,    -1,    -1,    -1,   722,   723,   724,
     725,   726,    -1,    -1,    -1,    -1,    -1,    -1,  1578,    -1,
     735,    -1,  1582,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1593,    -1,    -1,    -1,    -1,  1598,    -1,
      -1,  1601,    -1,    -1,    -1,    -1,   761,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   251,   252,    -1,    -1,    -1,   204,
     257,    -1,    -1,   778,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   789,    -1,   791,   792,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   806,    -1,    -1,    -1,    -1,    -1,    -1,  1658,   814,
      -1,    -1,    -1,    -1,    -1,   204,  1666,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
    1680,    -1,    -1,    -1,    -1,    -1,   203,    -1,   205,  1689,
      -1,    -1,   847,    -1,    -1,   850,   333,    -1,    -1,   336,
      -1,    -1,    -1,    -1,  1704,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1714,   870,    -1,    -1,    -1,    -1,
      -1,    -1,  1722,    -1,    -1,    -1,    -1,    -1,  1728,    -1,
      -1,  1731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   910,   911,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,   943,    -1,
      -1,   946,   947,   948,   949,    65,    -1,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,    -1,    -1,    -1,    -1,    -1,   466,
     467,    -1,    -1,   470,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1002,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   515,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,  1042,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1079,    53,    -1,   564,   565,  1084,
      -1,    -1,   202,    -1,    -1,   572,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,  1099,  1100,    -1,  1102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1117,    -1,  1119,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1142,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,  1179,    -1,   202,  1182,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1207,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,   708,    -1,    85,   202,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   720,   721,   722,   723,   724,   725,   726,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   735,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1266,  1267,    -1,    -1,    -1,  1271,    -1,  1273,   131,
    1275,    -1,    -1,  1278,    -1,  1280,    -1,    -1,  1283,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,  1292,    29,  1294,
      -1,    -1,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,   173,  1317,    -1,    55,    -1,    -1,    -1,   805,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,  1340,    77,    -1,    -1,    -1,
      -1,   828,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
    1355,    -1,   839,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     847,    -1,    -1,  1368,  1369,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1404,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1432,  1433,  1434,
      -1,    -1,   173,    -1,    -1,  1440,  1441,   924,    -1,   926,
    1445,   928,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   943,    -1,    -1,   200,
     947,   948,   949,   204,    -1,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,  1002,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    65,    79,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,    -1,  1548,    -1,    -1,  1033,    -1,  1035,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1059,    53,    -1,    -1,    -1,  1582,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    77,    -1,  1593,    -1,
      -1,    -1,    -1,  1598,    -1,    -1,  1601,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,  1102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1114,    -1,    -1,
    1117,    -1,  1119,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,   203,  1658,   205,  1142,    -1,    -1,    -1,    -1,
      -1,  1666,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,  1680,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1178,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    10,    11,    12,  1714,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1722,    -1,    -1,
    1207,    -1,   202,  1728,    -1,    29,  1731,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1259,    -1,  1261,    -1,    77,    -1,    79,  1266,
      -1,    -1,    -1,    -1,  1271,    -1,  1273,    -1,  1275,    -1,
      -1,  1278,    -1,  1280,    -1,    -1,  1283,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1291,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,  1301,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,    29,
    1317,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,  1355,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1367,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,   201,    -1,    -1,
      -1,    -1,   203,    -1,   205,    -1,  1393,    -1,    -1,    -1,
    1397,    -1,    -1,    -1,    -1,     5,     6,  1404,     8,     9,
      10,    11,    12,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,  1432,  1433,  1434,    10,    11,
      12,    41,    -1,  1440,    -1,    -1,    -1,    -1,    48,    -1,
      50,    -1,    -1,    53,    -1,    55,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,   201,    -1,    65,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    29,    55,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    77,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,  1548,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1559,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1578,    53,    -1,    -1,  1582,    -1,    -1,   188,    -1,
      -1,   131,   132,    -1,    65,    -1,  1593,    -1,    -1,    -1,
      -1,  1598,    -1,    -1,  1601,    -1,    -1,    -1,    -1,   149,
      -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,   470,   232,   173,    -1,   235,    -1,    -1,    -1,    -1,
      -1,    -1,   242,   243,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
     200,  1658,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   515,    -1,    -1,    -1,
      -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,   288,    -1,
      -1,    -1,  1689,    -1,   294,    -1,    -1,    -1,   298,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,    -1,    -1,
      -1,   311,   312,   313,    -1,    -1,    -1,  1714,    -1,    -1,
      -1,    -1,    -1,    -1,   324,  1722,    -1,    -1,    -1,    -1,
      -1,  1728,    -1,    -1,  1731,   335,    -1,    67,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,
      -1,    -1,    -1,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,    -1,   379,
      -1,   381,   382,    -1,   384,    -1,    -1,   117,    -1,    -1,
      -1,    -1,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,    -1,    -1,    -1,    -1,    -1,
     410,   411,    -1,   413,   414,   415,    -1,    -1,    -1,   149,
      -1,   421,   152,   153,   424,   155,   156,   470,   158,   159,
     160,    -1,    -1,   433,    -1,   435,    -1,   167,    -1,    -1,
      -1,   441,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   449,
      -1,   451,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   708,
     200,    -1,   515,    -1,    -1,   205,    -1,   477,    -1,    -1,
     480,   481,   482,   722,   723,   724,   725,   726,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,   735,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   511,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    65,   592,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   604,    -1,    -1,    29,   847,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,   636,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,   645,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   663,    -1,   708,    -1,    -1,    -1,    77,
      -1,    -1,    -1,   192,   193,    -1,    -1,    -1,    -1,   722,
     723,   724,   725,   726,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   735,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   943,   705,    -1,    -1,   947,   948,
     949,    -1,    -1,   952,   953,   954,   955,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
      -1,    -1,    -1,    -1,   152,    -1,   746,   155,   156,    68,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,  1002,   470,    -1,    -1,   189,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,   117,    -1,
     800,    -1,    -1,    -1,   847,    -1,    -1,    -1,    -1,   515,
      -1,    -1,    -1,   813,    -1,    -1,    -1,    -1,    -1,   819,
      -1,   821,    -1,   823,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,   845,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   855,    -1,    -1,   858,    -1,
     860,    -1,    -1,  1102,   864,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,  1117,    -1,
    1119,   200,    77,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   898,    -1,
     943,    -1,    -1,  1142,   947,   948,   949,    -1,    -1,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,   153,  1002,
     155,   156,   157,   158,   159,   160,    -1,    -1,  1207,    35,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     980,   981,   982,    -1,    -1,    -1,   986,   987,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   708,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    79,  1014,    -1,   722,   723,   724,   725,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1266,    -1,   735,
      -1,    -1,  1271,    -1,  1273,    -1,  1275,    -1,    -1,  1278,
      -1,  1280,    -1,  1043,  1283,    -1,    -1,    -1,    -1,    -1,
      -1,   117,    -1,    -1,    -1,    -1,  1056,    -1,    -1,  1102,
      -1,    -1,    -1,    -1,   130,  1065,  1066,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1117,    -1,  1119,    -1,  1317,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,  1096,    -1,    -1,  1142,
      -1,    -1,    -1,    -1,    -1,    -1,  1106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1355,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
      -1,   847,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1207,  1404,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,  1192,  1432,  1433,  1434,  1196,    -1,  1198,  1199,
      -1,  1440,    65,    -1,    -1,  1444,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1217,  1218,    -1,
      -1,    -1,    -1,  1266,    -1,    -1,    -1,    -1,  1271,    -1,
    1273,    -1,  1275,    -1,    -1,  1278,    -1,  1280,    -1,    -1,
    1283,   947,   948,   949,    -1,    -1,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,    -1,  1317,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1284,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,  1002,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1548,
      -1,    -1,  1355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1320,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1582,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1593,    -1,    -1,    -1,    -1,  1598,
      -1,  1404,  1601,    -1,    -1,    -1,    -1,    -1,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1623,    -1,    -1,    -1,    -1,  1432,
    1433,  1434,    -1,    -1,    -1,    -1,  1102,  1440,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,  1117,    -1,  1119,   200,    -1,    -1,    -1,    -1,  1658,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1435,  1142,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1714,    65,    -1,    -1,    -1,
      -1,    -1,    -1,  1722,    -1,    -1,    -1,    -1,    -1,  1728,
      -1,    -1,  1731,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,  1207,    -1,    -1,    -1,  1548,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,  1582,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
    1593,    -1,    -1,    -1,    -1,  1598,    -1,    -1,  1601,    -1,
    1266,    -1,    -1,    -1,    -1,  1271,    -1,  1273,    -1,  1275,
      -1,    -1,  1278,   104,  1280,    -1,    -1,  1283,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1317,    -1,    -1,    -1,  1658,    -1,    55,   149,    -1,
      -1,   152,   153,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    29,   173,    -1,    -1,    -1,    -1,    -1,    -1,  1355,
      -1,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   104,    55,    -1,   200,
      -1,  1714,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1722,
      -1,    -1,    -1,    -1,    -1,  1728,    -1,    -1,  1731,    77,
      -1,    -1,    -1,   131,   132,    -1,    -1,    -1,  1404,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,   149,    -1,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,  1432,  1433,  1434,    -1,
      -1,    -1,    -1,    -1,  1440,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   149,   200,    -1,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   173,   155,   156,    -1,   158,
     159,   160,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1548,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1582,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,  1593,    -1,    -1,
      -1,    -1,  1598,    -1,    -1,  1601,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,  1658,    -1,    -1,   101,   102,   103,   104,   105,
     106,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,  1714,   155,
     156,    -1,   158,   159,   160,   161,  1722,    -1,   164,    -1,
      -1,   167,  1728,    -1,    -1,  1731,    -1,   173,   174,   175,
     176,   177,    -1,   179,    -1,   181,   182,    -1,   184,   185,
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
      -1,    -1,   101,   102,   103,   104,   105,   106,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,    -1,   155,   156,    -1,   158,
     159,   160,   161,    -1,    -1,   164,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,   177,    -1,
     179,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
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
     102,   103,   104,   105,   106,   107,    -1,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,   161,
      -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,   177,    -1,   179,    -1,   181,
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
      -1,    85,    86,    87,    88,    89,    90,    -1,    92,    -1,
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
      87,    88,    -1,    90,    -1,    92,    -1,    94,    95,    -1,
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
      86,    87,    88,    -1,    90,    -1,    92,    93,    94,    -1,
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
      85,    86,    87,    88,    -1,    90,    91,    92,    -1,    94,
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
     197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    63,    64,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,   127,   128,    -1,   149,
     150,   151,   152,   153,    -1,   155,   156,    -1,   158,   159,
     160,    -1,    -1,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,    -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,
      -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,
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
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
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
      -1,    -1,    -1,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
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
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,
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
      -1,   155,   156,    -1,   158,   159,   160,    -1,   162,    -1,
     164,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
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
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    11,    12,   203,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    29,    13,    31,    32,    33,    34,    35,    36,
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
     160,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   101,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,   149,   150,   151,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,   164,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    11,    12,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
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
      -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,     5,
       6,     7,   173,   174,   175,   176,    -1,    13,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,    -1,
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
      -1,   167,     3,     4,     5,     6,     7,   173,   174,   175,
     176,    -1,    13,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,   202,    -1,    -1,   205,
     206,    -1,   208,   209,    -1,    46,    47,    -1,    -1,    -1,
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
      -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,     5,
       6,     7,   173,   174,   175,   176,    -1,    13,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,    -1,    -1,   205,   206,    -1,   208,   209,    -1,
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
     196,   197,    -1,    -1,   200,   201,    -1,    -1,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    30,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    46,    47,    -1,
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
      -1,   200,    -1,    12,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   164,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      12,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,
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
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    -1,    -1,
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
      -1,    -1,    -1,   164,    -1,    -1,   167,     3,     4,     5,
       6,     7,   173,   174,   175,   176,    -1,    13,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,    -1,
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
      -1,   167,     3,     4,     5,     6,     7,   173,   174,   175,
     176,    -1,    13,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
     206,    -1,   208,   209,    -1,    46,    47,    -1,    -1,    -1,
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
       4,     5,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    29,    -1,    -1,   130,   131,    -1,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   149,   150,   151,    -1,    55,
      -1,   155,   156,    -1,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,   173,
      -1,    77,    29,   177,    -1,   179,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,    -1,    -1,    55,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      77,    -1,    -1,    -1,    -1,   131,   132,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,   149,    -1,   200,   152,   153,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      46,    47,    -1,   200,    -1,   117,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,   149,    -1,    85,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   131,    -1,   133,   134,   135,
     136,   137,    -1,   205,    77,    -1,    79,    -1,   144,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,    -1,    -1,   117,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    46,    47,    -1,   200,    -1,    -1,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,   152,
     153,    66,   155,   156,    -1,   158,   159,   160,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,
     203,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,   131,    -1,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,   164,
      -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,    -1,   130,    -1,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,   149,    -1,   200,   152,   153,    -1,   155,
     156,    -1,   158,   159,   160,    77,    -1,    79,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    65,    -1,    -1,   200,   117,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    77,    -1,    79,   200,    -1,
      -1,   149,    -1,   205,   152,   153,    -1,   155,   156,    -1,
     158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
     155,   156,   200,   158,   159,   160,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
     152,   153,    -1,   155,   156,    -1,   158,   159,   160,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    10,    11,    12,   200,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   129,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   129,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,
     129,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   129,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,   129,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   129,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    77,    -1,    -1,    -1,    29,   129,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   118,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
      -1,    -1,    77,    -1,    -1,   129,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,   149,   104,   105,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,   129,    79,    80,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    77,   152,    -1,    -1,   155,   156,    -1,
     158,   159,   160,    -1,   149,    -1,    -1,   152,    -1,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    77,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,   149,    77,    -1,   152,   153,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,   152,    -1,    -1,
     155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   123,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    77,    -1,   152,    -1,
      -1,   155,   156,    -1,   158,   159,   160,    -1,    -1,    -1,
      77,    -1,    -1,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   123,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,   155,   156,    -1,   158,   159,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   155,   156,
      -1,   158,   159,   160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    96,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
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
     202,   475,   482,   352,   483,    79,   162,    14,    79,   471,
     434,   341,   201,   289,   203,   289,   200,   129,   200,   291,
     201,   203,   482,   203,   269,   252,   404,   291,   129,   207,
       9,   410,   416,   145,   353,   419,   420,   415,   434,   332,
      30,    73,   232,   202,   334,   273,   446,   274,   201,   406,
      95,    98,   202,   341,    30,   202,   282,   204,   170,   129,
     162,    30,   201,   406,   406,   201,   129,     9,   410,   201,
     201,     9,   410,   129,   201,     9,   410,   201,   129,   204,
       9,   410,   406,    30,   187,   201,   230,   217,   482,   399,
       4,   105,   110,   118,   155,   156,   158,   204,   300,   323,
     324,   325,   330,   426,   446,   204,   203,   204,    50,   341,
     341,   341,   341,   352,    35,    79,   162,    14,    79,   406,
     200,   475,   201,   299,   201,   289,   341,   291,   201,   299,
     467,   299,   203,   200,   201,   415,   415,   201,   129,   201,
       9,   410,    30,   230,   202,   201,   201,   201,   237,   202,
     202,   282,   230,   482,   482,   129,   406,   353,   406,   406,
     406,   406,   406,   406,   341,   203,   204,    96,   125,   126,
     469,   272,   399,   118,   131,   132,   153,   159,   309,   310,
     311,   399,   157,   315,   316,   121,   200,   217,   317,   318,
     301,   248,   482,     9,   202,   324,   201,   296,   153,   390,
     204,   204,    79,   162,    14,    79,   406,   291,   110,   343,
     475,   204,   475,   201,   201,   204,   203,   204,   299,   289,
     129,   415,   353,   230,   235,   238,    30,   232,   276,   230,
     201,   406,   129,   129,   129,   188,   230,   482,   399,   399,
      14,     9,   202,   203,   203,     9,   202,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    53,    67,
      68,    69,    70,    71,    72,    73,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   130,   131,   133,
     134,   135,   136,   137,   149,   150,   151,   161,   163,   164,
     166,   173,   177,   179,   181,   182,   217,   396,   397,     9,
     202,   153,   157,   217,   318,   319,   320,   202,    79,   329,
     247,   302,   469,   248,   204,   297,   298,   469,    14,    79,
     406,   201,   200,   203,   202,   203,   321,   343,   475,   296,
     204,   201,   415,   129,    30,   232,   275,   276,   230,   406,
     406,   406,   341,   204,   202,   202,   406,   399,   305,   312,
     405,   310,    14,    30,    47,   313,   316,     9,    33,   201,
      29,    46,    49,    14,     9,   202,   470,   329,    14,   247,
     202,    14,   406,    35,    79,   387,   230,   230,   203,   321,
     204,   475,   415,   230,    93,   189,   243,   204,   217,   228,
     306,   307,   308,     9,   204,   406,   397,   397,    55,   314,
     319,   319,    29,    46,    49,   406,    79,   200,   202,   406,
     470,   406,    79,     9,   411,   204,   204,   230,   321,    91,
     202,    79,   108,   239,   148,    96,   405,   160,    14,   303,
     200,    35,    79,   201,   204,   202,   200,   166,   246,   217,
     324,   325,   406,   287,   288,   427,   304,    79,   399,   244,
     163,   217,   202,   201,     9,   411,   112,   113,   114,   327,
     328,   287,    79,   272,   202,   475,   427,   483,   201,   201,
     202,   202,   203,   322,   327,    35,    79,   162,   475,   203,
     230,   483,    79,   162,    14,    79,   322,   230,   204,    35,
      79,   162,    14,    79,   406,   204,    79,   162,    14,    79,
     406,    14,    79,   406,   406
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
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval).reset();;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval).reset();;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval).reset();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval).reset();;}
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
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval).reset();;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { (yyval).reset();;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval).reset();;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
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

  case 449:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
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

  case 453:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
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

  case 521:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
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

  case 522:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval).reset();;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval).reset();;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
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
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval).reset();;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval).reset();;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval).reset();;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval).reset();;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval).reset();;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset();;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval).reset();;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval)++;;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval).reset();;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
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
#line 2820 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    {;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    {;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    {;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    {;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13528 "hphp.tab.cpp"
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
#line 3045 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

