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
#define YYLAST   16919

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  278
/* YYNRULES -- Number of rules.  */
#define YYNRULES  940
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1762

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
    2497,  2501,  2505,  2507,  2508,  2510,  2512,  2514,  2516,  2520,
    2522,  2524,  2526,  2530,  2532,  2534,  2538,  2542,  2545,  2550,
    2553,  2558,  2560,  2562,  2564,  2566,  2568,  2572,  2578,  2582,
    2587,  2592,  2596,  2598,  2600,  2602,  2604,  2608,  2614,  2619,
    2623,  2625,  2627,  2631,  2635,  2637,  2639,  2647,  2657,  2665,
    2672,  2681,  2683,  2686,  2691,  2696,  2698,  2700,  2705,  2707,
    2708,  2710,  2713,  2715,  2717,  2721,  2727,  2731,  2735,  2736,
    2738,  2742,  2748,  2752,  2755,  2759,  2766,  2767,  2769,  2774,
    2777,  2778,  2784,  2788,  2792,  2794,  2801,  2806,  2811,  2814,
    2817,  2818,  2824,  2828,  2832,  2834,  2837,  2838,  2844,  2848,
    2852,  2854,  2857,  2858,  2861,  2862,  2868,  2872,  2876,  2878,
    2881,  2882,  2885,  2886,  2892,  2896,  2900,  2902,  2905,  2908,
    2910,  2913,  2915,  2920,  2924,  2928,  2935,  2939,  2941,  2943,
    2945,  2950,  2955,  2960,  2965,  2970,  2975,  2978,  2981,  2986,
    2989,  2992,  2994,  2998,  3002,  3006,  3007,  3010,  3016,  3023,
    3025,  3028,  3030,  3035,  3039,  3040,  3042,  3046,  3049,  3053,
    3055,  3057,  3058,  3059,  3062,  3067,  3070,  3077,  3082,  3084,
    3086,  3087,  3091,  3097,  3101,  3103,  3106,  3107,  3112,  3115,
    3118,  3120,  3122,  3124,  3126,  3131,  3138,  3140,  3149,  3156,
    3158
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     212,     0,    -1,    -1,   213,   214,    -1,   214,   215,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   473,    -1,   123,   201,   202,   203,    -1,   150,   225,
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
     476,    -1,   226,   476,    -1,   230,     9,   474,    14,   407,
      -1,   106,   474,    14,   407,    -1,   231,   232,    -1,    -1,
     233,    -1,   250,    -1,   257,    -1,   262,    -1,   204,   231,
     205,    -1,    70,   333,   233,   284,   286,    -1,    70,   333,
      30,   231,   285,   287,    73,   203,    -1,    -1,    89,   333,
     234,   278,    -1,    -1,    88,   235,   233,    89,   333,   203,
      -1,    -1,    91,   201,   335,   203,   335,   203,   335,   202,
     236,   276,    -1,    -1,    98,   333,   237,   281,    -1,   102,
     203,    -1,   102,   342,   203,    -1,   104,   203,    -1,   104,
     342,   203,    -1,   107,   203,    -1,   107,   342,   203,    -1,
      27,   102,   203,    -1,   112,   294,   203,    -1,   118,   296,
     203,    -1,    87,   334,   203,    -1,   120,   201,   470,   202,
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
      -1,    -1,   105,    -1,    -1,   249,   248,   475,   251,   201,
     290,   202,   480,   322,    -1,    -1,   326,   249,   248,   475,
     252,   201,   290,   202,   480,   322,    -1,    -1,   427,   325,
     249,   248,   475,   253,   201,   290,   202,   480,   322,    -1,
      -1,   160,   218,   255,    30,   487,   472,   204,   297,   205,
      -1,    -1,   427,   160,   218,   256,    30,   487,   472,   204,
     297,   205,    -1,    -1,   268,   265,   258,   269,   270,   204,
     300,   205,    -1,    -1,   427,   268,   265,   259,   269,   270,
     204,   300,   205,    -1,    -1,   125,   266,   260,   271,   204,
     300,   205,    -1,    -1,   427,   125,   266,   261,   271,   204,
     300,   205,    -1,    -1,   162,   267,   263,   270,   204,   300,
     205,    -1,    -1,   427,   162,   267,   264,   270,   204,   300,
     205,    -1,   475,    -1,   154,    -1,   475,    -1,   475,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   400,    -1,    -1,
     127,   272,    -1,    -1,   126,   272,    -1,    -1,   400,    -1,
     272,     9,   400,    -1,   400,    -1,   273,     9,   400,    -1,
     130,   275,    -1,    -1,   437,    -1,    35,   437,    -1,   131,
     201,   451,   202,    -1,   233,    -1,    30,   231,    92,   203,
      -1,   233,    -1,    30,   231,    94,   203,    -1,   233,    -1,
      30,   231,    90,   203,    -1,   233,    -1,    30,   231,    96,
     203,    -1,   218,    14,   407,    -1,   280,     9,   218,    14,
     407,    -1,   204,   282,   205,    -1,   204,   203,   282,   205,
      -1,    30,   282,    99,   203,    -1,    30,   203,   282,    99,
     203,    -1,   282,   100,   342,   283,   231,    -1,   282,   101,
     283,   231,    -1,    -1,    30,    -1,   203,    -1,   284,    71,
     333,   233,    -1,    -1,   285,    71,   333,    30,   231,    -1,
      -1,    72,   233,    -1,    -1,    72,    30,   231,    -1,    -1,
     289,     9,   428,   328,   488,   163,    79,    -1,   289,     9,
     428,   328,   488,    35,   163,    79,    -1,   289,     9,   428,
     328,   488,   163,    -1,   289,   412,    -1,   428,   328,   488,
     163,    79,    -1,   428,   328,   488,    35,   163,    79,    -1,
     428,   328,   488,   163,    -1,    -1,   428,   328,   488,    79,
      -1,   428,   328,   488,    35,    79,    -1,   428,   328,   488,
      35,    79,    14,   407,    -1,   428,   328,   488,    79,    14,
     407,    -1,   289,     9,   428,   328,   488,    79,    -1,   289,
       9,   428,   328,   488,    35,    79,    -1,   289,     9,   428,
     328,   488,    35,    79,    14,   407,    -1,   289,     9,   428,
     328,   488,    79,    14,   407,    -1,   291,     9,   428,   488,
     163,    79,    -1,   291,     9,   428,   488,    35,   163,    79,
      -1,   291,     9,   428,   488,   163,    -1,   291,   412,    -1,
     428,   488,   163,    79,    -1,   428,   488,    35,   163,    79,
      -1,   428,   488,   163,    -1,    -1,   428,   488,    79,    -1,
     428,   488,    35,    79,    -1,   428,   488,    35,    79,    14,
     407,    -1,   428,   488,    79,    14,   407,    -1,   291,     9,
     428,   488,    79,    -1,   291,     9,   428,   488,    35,    79,
      -1,   291,     9,   428,   488,    35,    79,    14,   407,    -1,
     291,     9,   428,   488,    79,    14,   407,    -1,   293,   412,
      -1,    -1,   342,    -1,    35,   437,    -1,   163,   342,    -1,
     293,     9,   342,    -1,   293,     9,   163,   342,    -1,   293,
       9,    35,   437,    -1,   294,     9,   295,    -1,   295,    -1,
      79,    -1,   206,   437,    -1,   206,   204,   342,   205,    -1,
     296,     9,    79,    -1,   296,     9,    79,    14,   407,    -1,
      79,    -1,    79,    14,   407,    -1,   297,   298,    -1,    -1,
     299,   203,    -1,   474,    14,   407,    -1,   300,   301,    -1,
      -1,    -1,   324,   302,   330,   203,    -1,    -1,   326,   487,
     303,   330,   203,    -1,   331,   203,    -1,    -1,   325,   249,
     248,   475,   201,   304,   288,   202,   480,   323,    -1,    -1,
     427,   325,   249,   248,   475,   201,   305,   288,   202,   480,
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
     474,    14,   407,    -1,   106,   474,    14,   407,    -1,   201,
     332,   202,    -1,    68,   402,   405,    -1,    67,   342,    -1,
     389,    -1,   361,    -1,   201,   342,   202,    -1,   334,     9,
     342,    -1,   342,    -1,   334,    -1,    -1,    27,    -1,    27,
     342,    -1,    27,   342,   130,   342,    -1,   437,    14,   336,
      -1,   131,   201,   451,   202,    14,   336,    -1,    28,   342,
      -1,   437,    14,   339,    -1,   131,   201,   451,   202,    14,
     339,    -1,   343,    -1,   437,    -1,   332,    -1,   441,    -1,
     440,    -1,   131,   201,   451,   202,    14,   342,    -1,   437,
      14,   342,    -1,   437,    14,    35,   437,    -1,   437,    14,
      35,    68,   402,   405,    -1,   437,    26,   342,    -1,   437,
      25,   342,    -1,   437,    24,   342,    -1,   437,    23,   342,
      -1,   437,    22,   342,    -1,   437,    21,   342,    -1,   437,
      20,   342,    -1,   437,    19,   342,    -1,   437,    18,   342,
      -1,   437,    17,   342,    -1,   437,    16,   342,    -1,   437,
      15,   342,    -1,   437,    64,    -1,    64,   437,    -1,   437,
      63,    -1,    63,   437,    -1,   342,    31,   342,    -1,   342,
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
      -1,   342,    29,    30,   342,    -1,   469,    -1,    62,   342,
      -1,    61,   342,    -1,    60,   342,    -1,    59,   342,    -1,
      58,   342,    -1,    57,   342,    -1,    56,   342,    -1,    69,
     403,    -1,    55,   342,    -1,   409,    -1,   360,    -1,   359,
      -1,   362,    -1,   363,    -1,   207,   404,   207,    -1,    13,
     342,    -1,   367,    -1,   111,   201,   388,   412,   202,    -1,
      -1,    -1,   249,   248,   201,   346,   290,   202,   480,   344,
     204,   231,   205,    -1,    -1,   326,   249,   248,   201,   347,
     290,   202,   480,   344,   204,   231,   205,    -1,    -1,    79,
     349,   353,    -1,    -1,   183,    79,   350,   353,    -1,    -1,
     198,   351,   290,   199,   480,   353,    -1,    -1,   183,   198,
     352,   290,   199,   480,   353,    -1,     8,   342,    -1,     8,
     339,    -1,     8,   204,   231,   205,    -1,    86,    -1,   471,
      -1,   355,     9,   354,   130,   342,    -1,   354,   130,   342,
      -1,   356,     9,   354,   130,   407,    -1,   354,   130,   407,
      -1,   355,   411,    -1,    -1,   356,   411,    -1,    -1,   174,
     201,   357,   202,    -1,   132,   201,   452,   202,    -1,    66,
     452,   208,    -1,   400,   204,   454,   205,    -1,   176,   201,
     458,   202,    -1,   177,   201,   458,   202,    -1,   175,   201,
     459,   202,    -1,   176,   201,   462,   202,    -1,   177,   201,
     462,   202,    -1,   175,   201,   463,   202,    -1,   400,   204,
     456,   205,    -1,   367,    66,   447,   208,    -1,   368,    66,
     447,   208,    -1,   360,    -1,   471,    -1,   440,    -1,    86,
      -1,   201,   343,   202,    -1,   371,   372,    -1,   437,    14,
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
     400,    -1,   118,    -1,   445,    -1,   400,    -1,   118,    -1,
     449,    -1,   201,   202,    -1,   333,    -1,    -1,    -1,    85,
      -1,   466,    -1,   201,   292,   202,    -1,    -1,    74,    -1,
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
     471,    -1,   406,    -1,   209,   466,   209,    -1,   210,   466,
     210,    -1,   145,   466,   146,    -1,   413,   411,    -1,    -1,
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
     129,    -1,   218,    -1,   154,    -1,   204,   342,   205,    -1,
     430,    -1,   444,    -1,   218,    -1,   204,   342,   205,    -1,
     432,    -1,   444,    -1,    66,   447,   208,    -1,   204,   342,
     205,    -1,   438,   434,    -1,   201,   332,   202,   434,    -1,
     450,   434,    -1,   201,   332,   202,   434,    -1,   444,    -1,
     399,    -1,   442,    -1,   443,    -1,   435,    -1,   437,   429,
     431,    -1,   201,   332,   202,   429,   431,    -1,   401,   149,
     444,    -1,   439,   201,   292,   202,    -1,   440,   201,   292,
     202,    -1,   201,   437,   202,    -1,   399,    -1,   442,    -1,
     443,    -1,   435,    -1,   437,   429,   430,    -1,   201,   332,
     202,   429,   430,    -1,   439,   201,   292,   202,    -1,   201,
     437,   202,    -1,   444,    -1,   435,    -1,   201,   437,   202,
      -1,   201,   441,   202,    -1,   345,    -1,   348,    -1,   437,
     429,   433,   476,   201,   292,   202,    -1,   201,   332,   202,
     429,   433,   476,   201,   292,   202,    -1,   401,   149,   218,
     476,   201,   292,   202,    -1,   401,   149,   444,   201,   292,
     202,    -1,   401,   149,   204,   342,   205,   201,   292,   202,
      -1,   445,    -1,   448,   445,    -1,   445,    66,   447,   208,
      -1,   445,   204,   342,   205,    -1,   446,    -1,    79,    -1,
     206,   204,   342,   205,    -1,   342,    -1,    -1,   206,    -1,
     448,   206,    -1,   444,    -1,   436,    -1,   449,   429,   431,
      -1,   201,   332,   202,   429,   431,    -1,   401,   149,   444,
      -1,   201,   437,   202,    -1,    -1,   436,    -1,   449,   429,
     430,    -1,   201,   332,   202,   429,   430,    -1,   201,   437,
     202,    -1,   451,     9,    -1,   451,     9,   437,    -1,   451,
       9,   131,   201,   451,   202,    -1,    -1,   437,    -1,   131,
     201,   451,   202,    -1,   453,   411,    -1,    -1,   453,     9,
     342,   130,   342,    -1,   453,     9,   342,    -1,   342,   130,
     342,    -1,   342,    -1,   453,     9,   342,   130,    35,   437,
      -1,   453,     9,    35,   437,    -1,   342,   130,    35,   437,
      -1,    35,   437,    -1,   455,   411,    -1,    -1,   455,     9,
     342,   130,   342,    -1,   455,     9,   342,    -1,   342,   130,
     342,    -1,   342,    -1,   457,   411,    -1,    -1,   457,     9,
     407,   130,   407,    -1,   457,     9,   407,    -1,   407,   130,
     407,    -1,   407,    -1,   460,   411,    -1,    -1,   461,   411,
      -1,    -1,   460,     9,   342,   130,   342,    -1,   342,   130,
     342,    -1,   461,     9,   342,    -1,   342,    -1,   464,   411,
      -1,    -1,   465,   411,    -1,    -1,   464,     9,   407,   130,
     407,    -1,   407,   130,   407,    -1,   465,     9,   407,    -1,
     407,    -1,   466,   467,    -1,   466,    85,    -1,   467,    -1,
      85,   467,    -1,    79,    -1,    79,    66,   468,   208,    -1,
      79,   429,   218,    -1,   147,   342,   205,    -1,   147,    78,
      66,   342,   208,   205,    -1,   148,   437,   205,    -1,   218,
      -1,    80,    -1,    79,    -1,   121,   201,   470,   202,    -1,
     122,   201,   437,   202,    -1,   122,   201,   343,   202,    -1,
     122,   201,   441,   202,    -1,   122,   201,   440,   202,    -1,
     122,   201,   332,   202,    -1,     7,   342,    -1,     6,   342,
      -1,     5,   201,   342,   202,    -1,     4,   342,    -1,     3,
     342,    -1,   437,    -1,   470,     9,   437,    -1,   401,   149,
     218,    -1,   401,   149,   124,    -1,    -1,    97,   487,    -1,
     178,   475,    14,   487,   203,    -1,   180,   475,   472,    14,
     487,   203,    -1,   218,    -1,   487,   218,    -1,   218,    -1,
     218,   170,   481,   171,    -1,   170,   478,   171,    -1,    -1,
     487,    -1,   477,     9,   487,    -1,   477,   411,    -1,   477,
       9,   163,    -1,   478,    -1,   163,    -1,    -1,    -1,    30,
     487,    -1,   481,     9,   482,   218,    -1,   482,   218,    -1,
     481,     9,   482,   218,    97,   487,    -1,   482,   218,    97,
     487,    -1,    46,    -1,    47,    -1,    -1,    86,   130,   487,
      -1,   229,   149,   218,   130,   487,    -1,   484,     9,   483,
      -1,   483,    -1,   484,   411,    -1,    -1,   174,   201,   485,
     202,    -1,    29,   487,    -1,    55,   487,    -1,   229,    -1,
     132,    -1,   133,    -1,   486,    -1,   132,   170,   487,   171,
      -1,   132,   170,   487,     9,   487,   171,    -1,   154,    -1,
     201,   105,   201,   479,   202,    30,   487,   202,    -1,   201,
     487,     9,   477,   411,   202,    -1,   487,    -1,    -1
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
    2576,  2581,  2584,  2585,  2589,  2590,  2594,  2595,  2596,  2600,
    2601,  2605,  2606,  2610,  2611,  2615,  2616,  2620,  2621,  2626,
    2628,  2633,  2634,  2635,  2636,  2637,  2638,  2640,  2643,  2646,
    2648,  2650,  2654,  2655,  2656,  2657,  2658,  2661,  2665,  2667,
    2671,  2672,  2673,  2677,  2681,  2682,  2686,  2689,  2696,  2700,
    2704,  2711,  2712,  2717,  2719,  2720,  2723,  2724,  2727,  2728,
    2732,  2733,  2737,  2738,  2739,  2742,  2745,  2748,  2751,  2752,
    2753,  2756,  2760,  2764,  2765,  2766,  2768,  2769,  2770,  2774,
    2776,  2779,  2781,  2782,  2783,  2784,  2787,  2789,  2790,  2794,
    2796,  2799,  2801,  2802,  2803,  2807,  2809,  2812,  2815,  2817,
    2819,  2823,  2825,  2828,  2830,  2833,  2835,  2838,  2839,  2843,
    2845,  2848,  2850,  2853,  2856,  2860,  2862,  2866,  2867,  2869,
    2870,  2876,  2877,  2879,  2881,  2883,  2885,  2888,  2889,  2890,
    2894,  2895,  2896,  2897,  2898,  2899,  2900,  2901,  2902,  2903,
    2904,  2908,  2909,  2913,  2915,  2923,  2925,  2929,  2933,  2940,
    2941,  2947,  2948,  2955,  2958,  2962,  2965,  2970,  2975,  2977,
    2978,  2979,  2983,  2984,  2988,  2990,  2991,  2994,  2999,  3000,
    3001,  3005,  3008,  3017,  3019,  3023,  3026,  3029,  3037,  3040,
    3043,  3044,  3047,  3050,  3051,  3054,  3058,  3062,  3068,  3078,
    3079
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
  "object_property_name_no_variables", "object_property_name",
  "object_method_name_no_variables", "object_method_name", "array_access",
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
     425,   427,   428,   428,   429,   429,   430,   430,   430,   431,
     431,   432,   432,   433,   433,   434,   434,   435,   435,   436,
     436,   437,   437,   437,   437,   437,   437,   437,   437,   437,
     437,   437,   438,   438,   438,   438,   438,   438,   438,   438,
     439,   439,   439,   440,   441,   441,   442,   442,   443,   443,
     443,   444,   444,   445,   445,   445,   446,   446,   447,   447,
     448,   448,   449,   449,   449,   449,   449,   449,   450,   450,
     450,   450,   450,   451,   451,   451,   451,   451,   451,   452,
     452,   453,   453,   453,   453,   453,   453,   453,   453,   454,
     454,   455,   455,   455,   455,   456,   456,   457,   457,   457,
     457,   458,   458,   459,   459,   460,   460,   461,   461,   462,
     462,   463,   463,   464,   464,   465,   465,   466,   466,   466,
     466,   467,   467,   467,   467,   467,   467,   468,   468,   468,
     469,   469,   469,   469,   469,   469,   469,   469,   469,   469,
     469,   470,   470,   471,   471,   472,   472,   473,   473,   474,
     474,   475,   475,   476,   476,   477,   477,   478,   479,   479,
     479,   479,   480,   480,   481,   481,   481,   481,   482,   482,
     482,   483,   483,   484,   484,   485,   485,   486,   487,   487,
     487,   487,   487,   487,   487,   487,   487,   487,   487,   488,
     488
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
       3,     3,     1,     0,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     3,     3,     2,     4,     2,
       4,     1,     1,     1,     1,     1,     3,     5,     3,     4,
       4,     3,     1,     1,     1,     1,     3,     5,     4,     3,
       1,     1,     3,     3,     1,     1,     7,     9,     7,     6,
       8,     1,     2,     4,     4,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     3,     5,     3,     3,     0,     1,
       3,     5,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     0,     2,     0,     5,     3,     3,     1,     2,
       0,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
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
       0,   358,     0,   749,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   830,     0,
     818,   632,     0,   638,   639,   640,    22,   698,   806,    98,
      99,   641,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   330,
     331,   332,   335,   334,   333,     0,     0,     0,     0,   159,
       0,     0,     0,   645,   647,   648,   642,   643,     0,     0,
     649,   644,     0,   623,    23,    24,    25,    27,    26,     0,
     646,     0,     0,     0,     0,     0,     0,     0,   650,   336,
      28,    29,    31,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,   452,     0,    97,    70,   810,   633,     0,
       0,     4,    59,    61,    64,   697,     0,   622,     0,     6,
     131,     7,     9,     8,    10,     0,     0,   328,   368,     0,
       0,     0,     0,     0,     0,     0,   366,   794,   795,   436,
     435,   352,   437,   438,   441,     0,     0,   351,   772,   624,
       0,   700,   434,   327,   775,   367,     0,     0,   370,   369,
     773,   774,   771,   801,   805,     0,   424,   699,    11,   335,
     334,   333,     0,     0,    27,    59,   131,     0,   890,   367,
     889,     0,   887,   886,   440,     0,   359,   363,     0,     0,
     408,   409,   410,   411,   433,   431,   430,   429,   428,   427,
     426,   425,   806,   625,     0,   904,   624,     0,   390,     0,
     388,     0,   834,     0,   707,   350,   628,     0,   904,   627,
       0,   637,   813,   812,   629,     0,     0,   631,   432,     0,
       0,     0,     0,   355,     0,    78,   357,     0,     0,    84,
      86,     0,     0,    88,     0,     0,     0,   931,   932,   936,
       0,     0,    59,   930,     0,   933,     0,     0,    90,     0,
       0,     0,     0,   122,     0,     0,     0,     0,     0,     0,
      42,    47,   248,     0,     0,   247,     0,   163,     0,   160,
     253,     0,     0,     0,     0,     0,   901,   147,   157,   826,
     830,   871,     0,   652,     0,     0,     0,   869,     0,    16,
       0,    63,   139,   151,   158,   529,   466,   854,   852,   852,
       0,   895,   450,   454,   753,   368,     0,   366,   367,   369,
       0,     0,   634,     0,   635,     0,     0,     0,   121,     0,
       0,    66,   239,     0,    21,   130,     0,   156,   143,   155,
     333,   336,   131,   329,   112,   113,   114,   115,   116,   118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   818,     0,   111,   809,   809,
     119,   840,     0,     0,     0,     0,     0,     0,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   389,   387,   754,   755,     0,   809,     0,   767,
     239,   239,   809,     0,   811,   802,   826,     0,   131,     0,
       0,    92,     0,   751,   746,   707,     0,     0,     0,     0,
       0,   838,     0,   471,   706,   829,     0,     0,    66,     0,
     239,   349,     0,   769,   630,     0,    70,   199,     0,   449,
       0,    95,     0,     0,   356,     0,     0,     0,     0,     0,
      87,   110,    89,   928,   929,     0,   926,     0,     0,     0,
     900,     0,   117,    91,   120,     0,     0,     0,     0,     0,
       0,     0,   487,     0,   494,   496,   497,   498,   499,   500,
     501,   492,   514,   515,    70,     0,   107,   109,     0,     0,
      44,    51,     0,     0,    46,    55,    48,     0,    18,     0,
       0,   249,     0,    93,   162,   161,     0,     0,    94,   891,
       0,     0,   368,   366,   367,   370,   369,     0,   920,   169,
       0,   827,     0,     0,     0,     0,   651,   870,   698,     0,
       0,   868,   703,   867,    62,     5,    13,    14,     0,   167,
       0,     0,   459,     0,     0,   707,     0,     0,   626,   460,
     858,     0,   707,     0,     0,   707,     0,     0,     0,     0,
       0,   753,     0,   709,   752,   940,   348,   421,   781,   793,
      75,    69,    71,    72,    73,    74,   327,     0,   439,   701,
     702,    60,   707,     0,   905,     0,     0,     0,   709,   240,
       0,   444,   133,   165,     0,   393,   395,   394,     0,     0,
     391,   392,   396,   398,   397,   413,   412,   415,   414,   416,
     418,   419,   417,   407,   406,   400,   401,   399,   402,   403,
     405,   420,   404,   808,     0,     0,   844,     0,   707,   894,
       0,   893,   778,   801,   149,   141,   153,   145,   131,   358,
       0,   361,   364,   372,   488,   386,   385,   384,   383,   382,
     381,   380,   379,   378,   377,   376,   375,   757,     0,   756,
     759,   776,   763,   904,   760,     0,     0,     0,     0,     0,
       0,     0,     0,   888,   360,   744,   748,   706,   750,     0,
       0,   904,     0,   833,     0,   832,     0,   817,   816,     0,
       0,   756,   759,   814,   760,   353,   201,   203,    70,   457,
     456,   354,     0,    70,   183,    79,   357,     0,     0,     0,
       0,     0,   195,   195,    85,     0,     0,     0,   924,   707,
       0,   911,     0,     0,     0,     0,     0,   705,   641,     0,
       0,   623,     0,     0,     0,     0,     0,    64,   654,   622,
     660,   661,   659,     0,   653,    68,   658,     0,     0,   504,
       0,     0,   510,   507,   508,   516,     0,   495,   490,     0,
     493,     0,     0,     0,    52,    19,     0,     0,    56,    20,
       0,     0,     0,    41,    49,     0,   246,   254,   251,     0,
       0,   880,   885,   882,   881,   884,   883,    12,   918,   919,
       0,     0,     0,     0,   826,   823,     0,   470,   879,   878,
     877,     0,   873,     0,   874,   876,     0,     5,     0,     0,
       0,   523,   524,   532,   531,     0,     0,   706,   465,   469,
       0,   475,   706,   853,     0,   473,   706,   851,   474,     0,
     896,     0,   451,     0,   912,   753,   225,   939,     0,     0,
     768,   807,   706,   907,   903,   241,   242,   621,   708,   238,
       0,   753,     0,     0,   167,   446,   135,   423,     0,   480,
     481,     0,   472,   706,   839,     0,     0,   239,   169,     0,
     167,   165,     0,   818,   373,     0,     0,   765,   766,   779,
     780,   803,   804,     0,     0,     0,   732,   714,   715,   716,
     717,     0,     0,     0,   725,   724,   738,   707,     0,   746,
     837,   836,     0,     0,   770,   636,     0,   205,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     187,     0,    70,   185,   104,   195,     0,   195,     0,     0,
     934,     0,     0,   706,   925,   927,   910,   707,   909,     0,
     707,   682,   683,   680,   681,   713,     0,   707,   705,     0,
       0,   468,   862,   860,   860,     0,     0,   846,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   489,     0,     0,     0,   512,   513,
     511,     0,     0,   491,     0,   123,     0,   126,   108,     0,
      43,    53,     0,    45,    57,    50,   250,     0,   892,    96,
     920,   902,   915,   168,   170,   260,     0,     0,   824,     0,
     872,     0,    17,     0,   895,   166,   260,     0,     0,   462,
       0,   893,   857,   856,     0,   897,     0,   912,     0,     0,
     940,     0,   230,   228,   759,   777,   904,   906,     0,     0,
     243,    67,     0,   753,   164,     0,   753,     0,   422,   843,
     842,     0,   239,     0,     0,     0,     0,   167,   137,   637,
     758,   239,     0,   720,   721,   722,   723,   726,   727,   736,
       0,   707,   732,     0,   719,   740,   706,   743,   745,   747,
       0,   831,   759,   815,   758,     0,     0,     0,     0,   202,
     458,    81,     0,   357,   175,   177,   826,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,     0,   921,     0,
     923,   706,     0,     0,     0,   656,   706,   704,     0,   695,
       0,   707,     0,   866,     0,   707,     0,     0,   707,     0,
     662,   696,   694,   850,     0,   707,   665,   667,   666,     0,
       0,   663,   664,   668,   670,   669,   685,   684,   687,   686,
     688,   690,   691,   689,   678,   677,   672,   673,   671,   674,
     675,   676,   679,   502,     0,   503,   509,   517,   518,     0,
      70,    54,    58,   252,     0,     0,     0,   327,   828,   826,
     362,   365,   371,     0,    15,     0,   327,   535,     0,     0,
     537,   530,   533,     0,   528,     0,     0,   898,     0,   913,
     453,     0,   231,     0,     0,   226,     0,   245,   244,   912,
       0,   260,     0,   753,     0,   239,     0,   799,   260,   895,
     260,     0,     0,   374,     0,     0,   729,   706,   731,     0,
     718,     0,     0,   707,   737,   835,     0,    70,     0,   198,
     184,     0,     0,     0,   174,   100,   188,     0,     0,   191,
       0,   196,   197,    70,   190,   935,     0,   908,     0,   938,
     712,   711,   655,     0,   706,   467,   657,   478,   706,   861,
       0,   476,   706,   859,   477,     0,   479,   706,   845,   693,
       0,     0,     0,     0,   914,   917,   171,     0,     0,     0,
     325,     0,     0,     0,   148,   259,   261,     0,   324,     0,
     327,     0,   875,   256,   152,   526,     0,     0,   461,   855,
     455,     0,   234,   224,     0,   227,   233,   239,   443,   912,
     327,   912,     0,   841,     0,   798,   327,     0,   327,   260,
     753,   796,   735,   734,   728,     0,   730,   706,   739,    70,
     204,    77,    82,   102,   178,     0,   186,   192,    70,   194,
     922,     0,     0,   464,     0,   865,   864,     0,   849,   848,
     692,     0,    70,   127,     0,     0,     0,     0,     0,   172,
     291,   289,   293,   623,    27,     0,   285,     0,   290,   302,
       0,   300,   305,     0,   304,     0,   303,     0,   131,   263,
       0,   265,     0,   825,     0,   527,   525,   536,   534,   235,
       0,     0,   222,   232,     0,     0,     0,     0,   144,   443,
     912,   800,   150,   256,   154,   327,     0,     0,   742,     0,
     200,     0,     0,    70,   181,   101,   193,   937,   710,     0,
       0,     0,     0,     0,   916,     0,     0,     0,     0,   275,
     279,     0,     0,   270,   587,   586,   583,   585,   584,   604,
     606,   605,   575,   546,   547,   565,   581,   580,   542,   552,
     553,   555,   554,   574,   558,   556,   557,   559,   560,   561,
     562,   563,   564,   566,   567,   568,   569,   570,   571,   573,
     572,   543,   544,   545,   548,   549,   551,   589,   590,   599,
     598,   597,   596,   595,   594,   582,   601,   591,   592,   593,
     576,   577,   578,   579,   602,   603,   607,   609,   608,   610,
     611,   588,   613,   612,   615,   617,   616,   550,   620,   618,
     619,   614,   600,   541,   297,   538,     0,   271,   318,   319,
     317,   310,     0,   311,   272,   344,     0,     0,     0,     0,
     131,   140,   255,     0,     0,     0,   223,   237,   797,     0,
      70,   320,    70,   134,     0,     0,     0,   146,   912,   733,
       0,    70,   179,    83,   103,     0,   463,   863,   847,   505,
     125,   273,   274,   347,   173,     0,     0,   294,   286,     0,
       0,     0,   299,   301,     0,     0,   306,   313,   314,   312,
       0,     0,   262,     0,     0,     0,     0,   257,     0,   236,
       0,   521,   709,     0,     0,    70,   136,   142,     0,   741,
       0,     0,     0,   105,   276,    59,     0,   277,   278,     0,
       0,   292,   296,   539,   540,     0,   287,   315,   316,   308,
     309,   307,   345,   342,   266,   264,   346,     0,   258,   522,
     708,     0,   445,   321,     0,   138,     0,   182,   506,     0,
     129,     0,   327,   295,   298,     0,   753,   268,     0,   519,
     442,   447,   180,     0,     0,   106,   283,     0,   326,   343,
       0,   709,   338,   753,   520,     0,   128,     0,     0,   282,
     912,   753,   209,   339,   340,   341,   940,   337,     0,     0,
       0,   281,     0,   338,     0,   912,     0,   280,   322,    70,
     267,   940,     0,   214,   212,     0,    70,     0,     0,   215,
       0,     0,   210,   269,     0,   323,     0,   218,   208,     0,
     211,   217,   124,   219,     0,     0,   206,   216,     0,   207,
     221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   121,   827,   555,   185,   279,   509,
     513,   280,   510,   514,   123,   124,   125,   126,   127,   128,
     330,   591,   592,   463,   244,  1451,   469,  1375,  1452,  1680,
     783,   274,   504,  1643,  1018,  1200,  1695,   346,   186,   593,
     872,  1077,  1252,   132,   558,   889,   594,   613,   891,   539,
     888,   595,   559,   890,   348,   297,   313,   135,   874,   830,
     813,  1033,  1398,  1128,   938,  1593,  1455,   725,   944,   468,
     734,   946,  1283,   717,   927,   930,  1117,  1700,  1701,   582,
     583,   607,   608,   284,   285,   291,  1424,  1572,  1573,  1207,
    1325,  1417,  1568,  1686,  1703,  1605,  1647,  1648,  1649,  1405,
    1406,  1407,  1606,  1612,  1656,  1410,  1411,  1415,  1561,  1562,
    1563,  1583,  1730,  1326,  1327,   187,   137,  1716,  1717,  1566,
    1329,   138,   237,   464,   465,   139,   140,   141,   142,   143,
     144,   145,   146,  1436,   147,   871,  1076,   148,   241,   580,
     324,   581,   459,   564,   565,  1151,   566,  1152,   149,   150,
     151,   152,   153,   760,   761,   762,   154,   155,   271,   156,
     272,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     773,   774,  1010,   501,   502,   503,   780,  1632,   157,   560,
    1426,   561,  1047,   835,  1224,  1221,  1554,  1555,   158,   159,
     160,   231,   238,   333,   451,   161,   965,   766,   162,   966,
     863,   856,   967,   915,  1097,  1099,  1100,  1101,   917,  1262,
    1263,   918,   696,   435,   198,   199,   596,   585,   416,   680,
     681,   682,   683,   860,   164,   232,   189,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   644,   175,   234,   235,
     542,   223,   224,   647,   648,  1164,  1165,   574,   571,   575,
     572,  1157,  1154,  1158,  1155,   306,   307,   821,   176,   530,
     177,   579,   178,  1574,   298,   341,   602,   603,   959,  1059,
     810,   811,   738,   739,   740,   265,   266,   858
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1473
static const yytype_int16 yypact[] =
{
   -1473,   143, -1473, -1473,  6165, 13651, 13651,   -47, 13651, 13651,
   13651, 11739, 13651, -1473, 13651, 13651, 13651, 13651, 13651, 13651,
   13651, 13651, 13651, 13651, 13651, 13651, 15216, 15216, 11947, 13651,
   15282,   -41,   -37, -1473, -1473, -1473, -1473, -1473,   160, -1473,
   -1473,   112, 13651, -1473,   -37,    -5,     1,     7,   -37, 12113,
   13320, 12279, -1473, 14506, 10741,   141, 13651, 15063,    -2, -1473,
   -1473, -1473,    24,   251,    26,   149,   172,   179,   182, -1473,
   13320,   185,   192, -1473, -1473, -1473, -1473, -1473,   283,  4522,
   -1473, -1473, 13320, -1473, -1473, -1473, -1473, 13320, -1473, 13320,
   -1473,    52,   194,   207,   209,   214, 13320, 13320, -1473,    40,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, 13651, -1473, -1473,   195,   338,   432,
     432, -1473,   170,    62,   351, -1473,   221, -1473,    49, -1473,
     308, -1473, -1473, -1473, -1473, 16259,   350, -1473, -1473,   249,
     253,   257,   289,   291,   305,  3810, -1473, -1473, -1473, -1473,
     378, -1473, -1473, -1473,   383,   415,   309, -1473,    41,   298,
     365, -1473, -1473,   840,    95,  1049,    46,   317,    32, -1473,
     106,   107,   337,    35, -1473,    10, -1473,   477, -1473, -1473,
   -1473,   405,   373,   403, -1473, -1473,   308,   350, 16552,  3008,
   16552, 13651, 16552, 16552, 10932,   375, 15668, 10932,   541, 13320,
     522,   522,    98,   522,   522,   522,   522,   522,   522,   522,
     522,   522, -1473, -1473, 14991,   439, -1473,   462,    29,   453,
      29, 15216, 15712,   435,   614, -1473,   405,  4761,   439,   491,
     516,   466,   110, -1473,    29,    46, 12445, -1473, -1473, 13651,
    5497,   661,    56, 16552, 10325, -1473, 13651, 13651, 13320, -1473,
   -1473,  3854,   468, -1473,  3946, 14506, 14506,   503, -1473, -1473,
     487, 14293,   663, -1473,   664, -1473, 13320,   605, -1473,   484,
    4225,   493,   555, -1473,     2,  4360, 16274, 16319, 13320,    64,
   -1473,   180, -1473, 14566,    69, -1473,   570, -1473,   573, -1473,
     684,    70, 15216, 15216, 13651,   497,   532, -1473, -1473, 15076,
   11947,    54,   349, -1473, 13817, 15216,   422, -1473, 13320, -1473,
     254,    62, -1473, -1473, -1473, -1473, 15305, 13651, 13651, 13651,
     689,   608, -1473, -1473,    80,   504, 16552,   505,   582,   506,
    6373, 13651,   299,   502,   465,   299,   278,   273, -1473, 13320,
   14506,   512, 10949, 14506, -1473, -1473, 11782, -1473, -1473, -1473,
   -1473, -1473,   308, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   13651, 13651, 13651, 12653, 13651, 13651, 13651, 13651, 13651, 13651,
   13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651,
   13651, 13651, 13651, 13651, 13651, 15282, 13651, -1473, 13651, 13651,
   -1473, 13651, 15403, 13320, 13320, 13320, 16259,   609,   643, 10533,
   13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651, 13651,
   13651, 13651, -1473, -1473, -1473, -1473,  2658, 13651, 13651, -1473,
   10949, 10949, 13651, 13651,   195,   113, 15076,   518,   308, 12861,
    4412, -1473, 13651, -1473,   520,   708, 14991,   523,   -14,   506,
    4431,    29, 13069, -1473, 13277, -1473,   527,     9, -1473,    43,
   10949, -1473, 15349, -1473, -1473,  4710, -1473, -1473, 11157, -1473,
   13651, -1473,   633,  9493,   714,   535, 16507,   710,    67,    30,
   -1473, -1473, -1473, -1473, -1473, 14506, 14744,   542,   735, 14810,
   -1473,   562, -1473, -1473, -1473,   673, 13651,   674,   676, 13651,
   13651, 13651, -1473,   555, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473,   572, -1473, -1473, -1473,   563, -1473, -1473, 13320,   560,
     757,   198, 13320,   564,   760,   241,   258, 16337, -1473, 13320,
   13651,    29,    -2, -1473, -1473, -1473, 14810,   703, -1473,    29,
      85,    86,   584,   587,  2151,   139,   589,   590,   442,   666,
     595,    29,    87,   596,  3034, 13320, -1473, -1473,   731,  2097,
     236, -1473, -1473, -1473,    62, -1473, -1473, -1473,   770,   672,
     634,    58, -1473,   195,   678,   795,   604,   667,   113, -1473,
   16552,   613,   801, 15768,   615,   809,   617, 14506, 14506,   806,
     661,    80,   624,   815, -1473, 14506,     5,   761,   108, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473,   769,  2401, -1473, -1473,
   -1473, -1473,   823,   662, -1473, 15216, 13651,   636,   830, 16552,
     828, -1473, -1473,   718, 12696, 11348, 16678, 10932, 13651, 15002,
    3306,  2356,  3887, 16854,  1530,  2267,  2267,  2267,  2267,  1108,
    1108,  1108,  1108,   725,   725,   486,   486,   486,    98,    98,
      98, -1473,   522, 16552,   637,   638, 15812,   642,   839, -1473,
   13651,   -52,   649,   113, -1473, -1473, -1473, -1473,   308, 13651,
   14721, -1473, -1473, 10932, -1473, 10932, 10932, 10932, 10932, 10932,
   10932, 10932, 10932, 10932, 10932, 10932, 10932, -1473, 13651,   -15,
     114, -1473, -1473,   439,   202,   644,  2445,   652,   654,   651,
    2683,    88,   650, -1473, 16552, 14942, -1473, 13320, -1473,   504,
       5,   439, 15216, 16552, 15216, 15868,     5,   121, -1473,   658,
   13651, -1473,   123, -1473, -1473, -1473,  9285,   452, -1473, -1473,
   16552, 16552,   -37, -1473, -1473, -1473, 13651,   764, 14033, 14810,
   13320,  9701,   659,   660, -1473,    60,   736,   716, -1473,   858,
     669, 14359, 14506, 14810, 14810, 14810, 14810, 14810, -1473,   668,
      57,   719,   671,   680,   694,   695, 14810,    22, -1473,   724,
   -1473, -1473, -1473,   670, -1473, 16638, -1473, 13651,   688, 16552,
     711,   876,  5443,   888, -1473, 16552,  5012, -1473,   572,   820,
   -1473,  6581, 16196,   699,   280, -1473, 16274, 13320,   282, -1473,
   16319, 13320, 13320, -1473, -1473,  2950, -1473, 16638,   891, 15216,
     707, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
      61, 13320, 16196,   702, 15076, 15161,   897, -1473, -1473, -1473,
   -1473,   704, -1473, 13651, -1473, -1473,  5749, -1473, 14506, 16196,
     709, -1473, -1473, -1473, -1473,   901, 13651, 15305, -1473, -1473,
   12904, -1473, 13651, -1473, 13651, -1473, 13651, -1473, -1473,   713,
   -1473, 14506, -1473,   721,   887,    23, -1473, -1473,   111,  2658,
   -1473, -1473, 14506, -1473, -1473,    29, 16552, -1473, 11365, -1473,
   14810,    27,   720, 16196,   672, -1473, -1473, 16788, 13651, -1473,
   -1473, 13651, -1473, 13651, -1473,  3009,   722, 10949,   666,   892,
     672,   718, 13320, 15282,    29,  3206,   723, -1473, -1473,   127,
   -1473, -1473, -1473,   912, 15474, 15474, 14942, -1473, -1473, -1473,
   -1473,   727,    71,   732, -1473, -1473, -1473,   929,   737,   520,
      29,    29, 13485, 15349, -1473, -1473,  3405,   476,   -37, 10325,
   -1473,  6789,   738,  6997,   739, 14033, 15216,   758,   810,    29,
   16638,   947, -1473, -1473, -1473, -1473,   483, -1473,   246, 14506,
   -1473, 14506, 13320, 14744, -1473, -1473, -1473,   953, -1473,   765,
     823,   565,   565,   898,   898, 16012,   762,   959, 14810,   825,
   13320, 15305, 14810, 14810, 14810,  2323, 13112, 14810, 14810, 14810,
   14810, 14658, 14810, 14810, 14810, 14810, 14810, 14810, 14810, 14810,
   14810, 14810, 14810, 14810, 14810, 14810, 14810, 14810, 14810, 14810,
   14810, 14810, 14810, 14810, 16552, 13651, 13651, 13651, -1473, -1473,
   -1473, 13651, 13651, -1473,   555, -1473,   890, -1473, -1473, 13320,
   -1473, -1473, 13320, -1473, -1473, -1473, -1473, 14810,    29, -1473,
     442, -1473,   877,   964, -1473, -1473,    91,   774,    29, 11573,
   -1473,  1834, -1473,  5957,   608,   964, -1473,   -23,   -19, 16552,
     847, -1473, 16552, 16552, 15912, -1473,   775,   887, 14506,   661,
   14506,    38,   965,   905,   132, -1473,   439, -1473, 15216, 13651,
   16552, 16638,   778,    27, -1473,   782,    27,   786, 16788, 16552,
   15968,   787, 10949,   788,   785, 14506,   789,   672, -1473,   466,
     304, 10949, 13651, -1473, -1473, -1473, -1473, -1473, -1473,   861,
     793,   985, 14942,   849, -1473, 15305, 14942, -1473, -1473, -1473,
   15216, 16552,   133, -1473, -1473,   -37,   966,   924, 10325, -1473,
   -1473, -1473,   800, 13651,   810,    29, 15076, 14033,   802, 14810,
    7205,   521,   803, 13651,    34,   248, -1473,   834, -1473,   879,
   -1473, 14440,   977,   808, 14810, -1473, 14810, -1473,   812, -1473,
     881,  1007,   817, 16638,   818,  1008, 16068,   819,  1013,   824,
   -1473, -1473, -1473, 16112,   822,  1016,  5685, 16718, 16753, 14810,
   16596,  4532,  2716,  3986,  5043, 12631, 14737, 14737, 14737, 14737,
    1184,  1184,  1184,  1184,   481,   481,   565,   565,   565,   898,
     898,   898,   898, 16552, 14097, 16552, -1473, 16552, -1473,   826,
   -1473, -1473, -1473, 16638, 13320, 14506, 16196,   475, -1473, 15076,
   -1473, -1473, 10932,   827, -1473,   829,  1382, -1473,    66, 13651,
   -1473, -1473, -1473, 13651, -1473, 13651, 13651, -1473,   661, -1473,
   -1473,   142,  1015,   951, 14810, -1473,   833,    29, 16552,   887,
     835, -1473,   836,    27, 13651, 10949,   838, -1473, -1473,   608,
   -1473,   831,   842, -1473,   843, 14942, -1473, 14942, -1473,   844,
   -1473,   906,   848,  1032, -1473,    29,  1014, -1473,   846, -1473,
   -1473,   851,   857,   100, -1473, -1473, 16638,   873,   874, -1473,
    3659, -1473, -1473, -1473, -1473, -1473, 14506, -1473, 14506, -1473,
   16638, 16168, -1473, 14810, 15305, -1473, -1473, -1473, 14810, -1473,
   14810, -1473, 14810, -1473, -1473, 14810, -1473, 14810, -1473, 16823,
   14810, 13651,   875,  7413,   983, -1473, -1473,   445, 14506, 16196,
   -1473, 16215,   903, 15497, -1473, -1473, -1473,   609, 14186,    72,
     643,   101, -1473, -1473, -1473,   927,  3482,  3538, 16552, 16552,
   -1473,    44,  1069,  1005, 14810, -1473, 16638, 10949,   974,   887,
    1547,   887,   884, 16552,   886, -1473,  1604,   885,  1705, -1473,
      27, -1473, -1473,   960, -1473, 14942, -1473, 15305, -1473, -1473,
    9285, -1473, -1473, -1473, -1473,  9909, -1473, -1473, -1473,  9285,
   -1473,   889, 14810, 16638,   962, 16638, 16638, 16212, 16638, 16268,
   16823, 14053, -1473, -1473, 14506, 16196, 16196,  1079,    39, -1473,
   -1473, -1473, -1473,    73,   893,    75, -1473,  5251, -1473, -1473,
      78, -1473, -1473, 15454, -1473,   896, -1473,  1017,   308, -1473,
   14506, -1473,   609, -1473,  5042, -1473, -1473, -1473, -1473,  1081,
    1024, 14810, -1473, 16638,   902,   907,   911,   355, -1473,   974,
     887, -1473, -1473, -1473, -1473,  1853,   908, 14942, -1473,   979,
    9285, 10117,  9909, -1473, -1473, -1473,  9285, -1473, 16638, 14810,
   14810, 14810, 13651,  7621, -1473,   913,   914, 14810, 16196, -1473,
   -1473,   490, 16215, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473,   424, -1473,   903, -1473, -1473, -1473,
   -1473, -1473,    53,   356, -1473,  1097,    79, 13320,  1017,  1104,
     308, -1473, -1473,   916,  1106, 14810, -1473, 16638, -1473,   115,
   -1473, -1473, -1473, -1473,   917,   355, 14120, -1473,   887, -1473,
   14942, -1473, -1473, -1473, -1473,  7829, 16638, 16638, 16638, 14009,
   -1473, -1473, -1473, 16638, -1473,  3213,    48, -1473, -1473, 14810,
    5251,  5251,  1067, -1473, 15454, 15454,   470, -1473, -1473, -1473,
   14810,  1044, -1473,   926,    81, 14810, 13320, -1473, 14810, 16638,
    1051, -1473,  1119,  8037,  8245, -1473, -1473, -1473,   355, -1473,
    8453,   928,  1054,  1028, -1473,  1041,   993, -1473, -1473,  1046,
     490, -1473, 16638, -1473, -1473,  1002, -1473,  1113, -1473, -1473,
   -1473, -1473, 16638,  1150, -1473, -1473, 16638,   969, 16638, -1473,
     118,   963, -1473, -1473,  8661, -1473,   968, -1473, -1473,   971,
     999, 13320,   643, -1473, -1473, 14810,    84, -1473,  1088, -1473,
   -1473, -1473, -1473, 16196,   699, -1473,  1010, 13320,   618, 16638,
     973,  1170,   524,    84, -1473,  1101, -1473, 16196,   978, -1473,
     887,    90, -1473, -1473, -1473, -1473, 14506, -1473,   981,   982,
      82, -1473,   414,   524,   319,   887,   984, -1473, -1473, -1473,
   -1473, 14506,    47,  1171,  1107,   414, -1473,  8869,   321,  1175,
    1111, 14810, -1473, -1473,  9077, -1473,    51,  1177,  1114, 14810,
   -1473, 16638, -1473,  1178,  1116, 14810, -1473, 16638, 14810, -1473,
   16638, 16638
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1473, -1473, -1473,  -503, -1473, -1473, -1473,    -4, -1473, -1473,
   -1473,   679,   411,   409,    45,   955,  3559, -1473,  2334, -1473,
    -291, -1473,    17, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473,  -493, -1473, -1473,  -182,     4,     8,
   -1473, -1473, -1473, -1473, -1473, -1473,    11, -1473, -1473, -1473,
   -1473,    15, -1473, -1473,   811,   816,   807,  -118,   314,  -827,
     320,   385,  -495,    93,  -876, -1473,  -234, -1473, -1473, -1473,
   -1473,  -677,   -61, -1473, -1473, -1473, -1473,  -483, -1473,  -551,
   -1473,  -396, -1473, -1473,   700, -1473,  -222, -1473, -1473,  -985,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,
    -235, -1473, -1473, -1473, -1473, -1473,  -317, -1473,   -83,  -962,
   -1473, -1472,  -494, -1473,  -162,    -1,  -134,  -481, -1473,  -325,
   -1473,   -75,   -21,  1202,  -684,  -373, -1473, -1473,   -40, -1473,
   -1473,  3762,   -64,  -194, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473,  -549,  -804, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473, -1473,   853, -1473,
   -1473,   232, -1473,   763, -1473, -1473, -1473, -1473, -1473, -1473,
   -1473,   239, -1473,   766, -1473, -1473,   479, -1473,   208, -1473,
   -1473, -1473, -1473, -1473, -1473, -1473, -1473,  -954, -1473,  1805,
    1771,  -351, -1473, -1473,   169,  3355,  4163, -1473, -1473,   293,
    -103,  -601, -1473, -1473,   357,  -682,   161, -1473, -1473, -1473,
   -1473, -1473,   345, -1473, -1473, -1473,    63,  -833,  -202,  -417,
    -414, -1473,   406,  -129, -1473, -1473,    14, -1473, -1473,    77,
     -70, -1473, -1473,    21,  -169, -1473,    -8, -1473, -1473, -1473,
    -398,   967, -1473, -1473, -1473, -1473, -1473,   949, -1473, -1473,
   -1473,   292, -1473, -1473, -1473,   530,   327, -1473, -1473,   976,
    -311, -1001, -1473,   -43,   -80,  -208,   -66,   531, -1473, -1021,
   -1473,   243,   323, -1473, -1473, -1473,  -181, -1031
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -905
static const yytype_int16 yytable[] =
{
     122,   397,   353,   136,   427,   569,   425,   869,   130,   314,
     264,   240,   131,   916,   269,   133,   320,   321,   165,   134,
     448,   129,  1060,   245,   687,   688,   661,   249,   691,  1231,
     853,   852,   452,  1050,   641,   712,  1228,   419,   713,   325,
     218,   220,   934,  1215,   329,   396,   252,  1075,  1468,   262,
     327,   233,   826,   353,   709,   349,   948,  1650,   343,  1124,
     732,  1216,  1614,  1086,  1281,   460,   296,   163,    13,   949,
    1030,   417,    13,   517,   473,   474,   730,   282,   522,   527,
     478,  1420,  -288,   312,  1472,   296,  1615,  1556,  1621,   212,
    1621,  1468,   296,   296,   799,   799,   815,   815,  -484,   545,
     815,   422,   281,   219,   219,   290,   453,  -782,   832,   815,
     815,   505,   417,  1636,   414,   415,  1335,  1232,   340,   322,
     544,   445,   212,  1429,   310,    13,  1739,   311,   328,    13,
    1753,   296,  1217,   414,   415,    13,  1222,   414,   415,   437,
     352,   286,   969,     3,   439,  1218,  1061,   568,   287,  -904,
    1630,   385,   446,  1688,   191,  -761,  1103,   414,   415,   604,
     236,  -785,   398,   386,   239,   716,  1675,  1150,  -448,   506,
     614,  -904,  -783,  -784,  -789,  -625,  -819,  1341,  -485,   422,
    -786,  1219,   414,   415,  -626,  1223,  -761,  -822,   588,  -820,
    1062,   428,   340,  -788,  1631,   434,   246,  1689,  -787,  -821,
     338,  1233,   247,   303,   283,  -484,   315,  1430,   248,   418,
    1740,   707,   833,   781,  1754,   339,   424,  1104,  1348,   532,
     315,  1342,  -708,   653,   536,  -708,  -904,   834,   438,  -229,
     533,   950,  1031,   421,   733,   441,   122,  1282,   323,   423,
     122,   447,  1469,  1470,   467,  -782,   692,   653,  1357,   117,
     418,  1274,   344,  1651,   165,  1616,  1350,   457,   165,   461,
    1251,   462,   480,  1356,   353,  1358,   612,   518,  1131,   731,
    1135,   653,   523,   528,  1063,  1421,  -288,   519,  1473,  -229,
     653,  1557,  1622,   653,  1665,  1727,  -213,   800,   801,   816,
     903,   219,  -708,  1208,   735,   787,  -791,   521,   219,  -785,
     610,  1261,  1374,  1423,   219,  1343,   529,   529,   534,  -792,
    -783,  -784,  -789,   541,  -819,   314,   349,   423,  -786,   550,
    1072,   511,   515,   516,  1043,  -822,   122,  -820,  1437,   136,
    1439,  -788,   698,   339,   130,   601,  -787,  -821,   791,   262,
     421,   805,   296,   345,   165,   273,  1133,  1134,  1133,  1134,
     292,   339,   301,   554,  1732,   792,  1746,   301,   551,   662,
     219,   699,   301,   551,   414,   415,   439,   288,   302,   219,
     219,   535,  -764,   293,  1445,   289,   219,  1019,   301,  1022,
     294,   645,   219,   295,   859,  1617,   299,   584,   651,   296,
     655,   296,   296,   300,   339,   316,   849,   850,  1733,   331,
    1747,   658,  1618,  -764,   857,  1619,   233,   339,   317,   685,
     318,   339,   679,   652,   689,   319,  1036,   301,   719,  1585,
     304,   305,   342,   332,  1264,   304,   305,   931,   301,   303,
     304,   305,   933,   339,   397,   339,   701,   684,  1609,  1271,
     541,   825,  1064,   886,  -482,  1065,   304,   305,   711,   388,
     438,  1136,   354,  1284,  1610,    52,   355,   556,   557,   122,
     356,   652,   838,    59,    60,    61,   179,   180,   350,   843,
     708,  1611,   847,   714,  -762,   896,   892,   165,   396,  1317,
     724,   389,  1734,   600,  1748,   304,   305,   599,   808,   809,
    1384,  1083,   357,   886,   358,   546,   304,   305,   859,  1659,
    -904,   301,   391,   219,   923,  -762,  1112,   551,   359,  1113,
    1230,   301,   390,   219,   392,   794,  1660,   335,   420,  1661,
      13,   340,  1240,   928,   929,  1242,   569,   997,   998,   999,
    1000,  1001,  1002,   351,   876,   382,   383,   384,  -790,   385,
     820,   822,  1089,  -483,   301,   884,  1003,  1115,  1116,   448,
     551,   386,  -904,   784,  -625,  -904,   308,   788,  1581,  1582,
     604,   604,   281,  1449,    33,    34,    35,  1638,   552,   304,
     305,  1395,  1396,  1362,   426,  1363,   748,   924,   431,   304,
     305,  1318,  1132,  1133,  1134,   433,  1319,   386,    59,    60,
      61,   179,   180,   350,  1320,   398,   429,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   340,
     296,   440,   304,   305,  1000,  1001,  1002,  1728,  1729,   865,
    1278,  1133,  1134,   444,    73,    74,    75,    76,    77,   547,
    1003,  1321,  1322,   553,  1323,   750,   954,  1713,  1714,  1715,
    -624,    80,    81,   443,   584,   412,   413,  1044,   334,   336,
     337,  1130,  1657,  1658,   421,    90,  1653,  1654,   351,   547,
     569,   553,   547,   553,   553,   449,  1210,   450,   568,   458,
    1056,   471,    98,   475,   894,   957,   960,  -899,   479,  1340,
    1324,  1067,   219,  1448,   481,  1724,  1246,   482,   476,  1722,
     653,   914,  1352,   919,   524,  1254,   484,   525,   526,   537,
    1738,   932,   538,   577,  1735,   578,   586,   587,   589,   598,
     414,   415,   122,   -65,    52,   136,   920,   697,   921,   611,
     130,   695,   722,   460,   729,   700,   941,   122,  1273,   706,
     165,    59,    60,    61,   179,   180,   350,   219,   726,   485,
     486,   487,   939,   741,   742,   165,   488,   489,   943,   767,
     490,   491,   768,   770,   653,   771,    59,    60,    61,   179,
     180,   350,   779,   785,   782,  1589,   786,   789,  1137,   790,
    1138,   379,   380,   381,   382,   383,   384,   122,   385,   219,
     136,   219,   798,  1021,   588,   130,   802,  1024,  1025,   803,
     386,   806,   812,   807,   569,   165,   814,   823,   817,   829,
     828,   351,   568,   831,   837,   219,   839,  1032,   836,  1446,
     842,  1331,  1088,  1028,  1107,   841,   840,   845,   846,   848,
     851,  1709,   122,   854,   855,   136,   351,  -486,   541,  1038,
     130,   511,   862,   864,   131,   515,  1051,   133,   867,   868,
     165,   134,   870,   129,   873,   879,   880,   882,   883,  1354,
     887,   875,   897,  1702,   899,   679,   900,  1143,  1236,   901,
     925,   935,   945,   947,  1147,   952,   951,   953,   970,   968,
    1702,   955,   971,   976,   977,  1005,   219,  1229,  1723,   857,
     684,   972,    59,    60,    61,    62,    63,   350,   296,   163,
    1007,   219,   219,    69,   393,   973,   974,  1011,  1006,  1014,
    1096,  1096,   914,  1017,  1249,  1027,  1035,  1118,  1639,  1313,
    1029,  1039,  1040,  1046,   233,  1048,  1055,  1058,   584,   711,
    1057,  1073,  1085,  1082,  1091,   122,  1092,   122,  1102,   122,
     136,   395,   136,  1105,   584,   130,   568,   130,  1106,  1108,
    1127,  1121,  1123,   165,   714,   165,  1119,   165,  1139,   939,
    1125,  1434,   351,    59,    60,    61,    62,    63,   350,  1126,
    1067,  1129,  1141,  1003,    69,   393,  1149,  1142,  1146,  1199,
    1145,   546,  1162,  1206,  1205,  1209,  1370,  1225,  1227,  1234,
    1239,   215,   215,   569,  1235,   228,  1241,  1243,  1245,  1248,
    1247,  1255,  1379,  1250,  1257,  1260,  1267,  1268,  1258,  1211,
     394,  1256,   395,  1270,  1275,  1285,  1279,  1288,   228,  1286,
    1289,  1293,   219,   219,  1292,  1201,  1294,  1298,  1202,  1296,
    1297,  1301,  1302,   351,  1315,  1307,  1304,  1306,  1312,  1344,
    1345,  1671,  1332,  1333,  1347,  1359,  1365,  1349,  1351,   122,
    1355,  1367,   136,  1360,  1369,  1361,  1364,   130,  1295,  1371,
    1366,   131,  1299,  1372,   133,  1303,   569,   165,   134,  1373,
     129,  1409,  1308,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,   410,   411,  1376,  1377,  1450,  1392,
    1394,  1425,  1237,  1431,  1432,  1435,  1440,  1456,  1441,  1443,
    1447,  1457,  1459,  1467,  1266,  1575,  1565,  1471,   914,  1564,
    1712,  1463,   914,  1576,  1578,  1380,   163,  1381,  1579,  1590,
    1588,  1620,   412,   413,   122,  1580,  1601,  1602,  1625,  1627,
    1628,  1635,  1655,  1663,  1265,   568,   122,  1664,  1670,   136,
    1669,  1677,   165,  1678,   130,  1269,   584,  1679,  -284,   584,
     541,   939,  1681,  1682,   165,   219,  1615,  1419,  -905,  -905,
    -905,  -905,   377,   378,   379,   380,   381,   382,   383,   384,
    1368,   385,  1595,  1684,  1685,  1690,  1694,  1704,  1422,   215,
    1687,  1692,  1693,   386,  1707,  1710,   215,   414,   415,  1711,
    1719,  1721,   215,  1725,  1726,  1741,  1742,   219,  1736,  1749,
    1750,  1755,  1758,  1756,   353,  1759,   793,  1020,   568,  1023,
    1314,  1706,   656,   219,   219,  1087,  1328,   657,  1084,   654,
     228,   228,  1720,  1464,  1045,  1328,   228,  1272,  1594,  1378,
    1718,  1586,   796,   541,  -905,  -905,  -905,  -905,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1567,  1608,   215,  1613,
    1416,  1743,  1731,  1624,   242,  1584,  1198,   215,   215,  1003,
    1196,   914,   664,   914,   215,  1220,   777,  1013,  1253,   778,
     215,  1148,  1098,  1259,  1109,  1066,  1159,   543,   576,   531,
    1330,   228,   958,  1204,     0,  1397,  1140,     0,     0,  1330,
       0,     0,     0,     0,     0,     0,   219,     0,     0,  1633,
       0,  1634,     0,     0,     0,   228,     0,     0,   228,     0,
    1640,     0,     0,     0,     0,     0,   584,     0,     0,   122,
       0,     0,   136,     0,   262,     0,     0,   130,     0,  1414,
       0,     0,     0,     0,     0,     0,     0,   165,     0,   398,
       0,  1418,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,     0,     0,  1674,     0,     0,     0,     0,  1328,
       0,     0,     0,     0,     0,  1328,     0,  1328,     0,     0,
       0,   914,     0,     0,     0,     0,   122,     0,     0,   136,
       0,   122,     0,     0,   130,   122,     0,  1569,   136,     0,
       0,   215,     0,   130,   165,     0,  1317,     0,  1626,   165,
       0,   215,  1454,   165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1553,     0,     0,     0,     0,     0,  1560,
       0,     0,     0,  1330,     0,     0,   262,     0,     0,  1330,
     262,  1330,     0,   584,     0,     0,  1570,    13,     0,     0,
     228,   228,     0,     0,   757,     0,     0,     0,  1737,     0,
       0,     0,     0,   914,  1328,  1744,   122,   122,   122,   136,
       0,     0,   122,     0,   130,   136,     0,     0,     0,   122,
     130,     0,   136,     0,   165,   165,   165,   130,  1592,  1454,
     165,     0,     0,     0,     0,     0,     0,   165,     0,     0,
       0,   757,     0,     0,     0,     0,     0,  1623,  1318,     0,
       0,     0,     0,  1319,     0,    59,    60,    61,   179,   180,
     350,  1320,     0,     0,     0,     0,     0,     0,  1330,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1697,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,   228,     0,   857,     0,     0,  1321,  1322,
     228,  1323,     0,     0,     0,     0,  1667,     0,     0,     0,
     857,  1317,     0,     0,     0,     0,     0,     0,     0,     0,
     215,     0,     0,   296,   353,   351,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   262,   385,     0,     0,   914,  1334,     0,     0,
       0,   122,    13,     0,   136,   386,     0,     0,     0,   130,
       0,  1645,     0,     0,     0,     0,  1553,  1553,  1317,   165,
    1560,  1560,     0,     0,     0,   215,     0,     0,     0,     0,
       0,     0,   296,     0,     0,     0,     0,     0,     0,   122,
     122,     0,   136,   136,     0,     0,   122,   130,   130,   136,
       0,     0,     0,     0,   130,     0,     0,   165,   165,    13,
       0,     0,     0,  1318,   165,     0,     0,   215,  1319,   215,
      59,    60,    61,   179,   180,   350,  1320,     0,     0,     0,
     122,     0,     0,   136,     0,     0,     0,  1696,   130,     0,
       0,  1698,     0,   215,   757,     0,     0,     0,   165,     0,
       0,     0,     0,  1708,     0,     0,   228,   228,   757,   757,
     757,   757,   757,  1321,  1322,     0,  1323,     0,     0,  1317,
    1318,   757,     0,     0,     0,  1319,     0,    59,    60,    61,
     179,   180,   350,  1320,     0,     0,     0,     0,     0,     0,
     351,     0,     0,   122,     0,     0,   136,   228,     0,     0,
     122,   130,     0,   136,     0,     0,     0,     0,   130,   584,
      13,   165,  1438,     0,   215,     0,     0,     0,   165,     0,
    1321,  1322,     0,  1323,     0,     0,   584,   228,     0,   215,
     215,     0,     0,     0,   584,     0,     0,     0,     0,     0,
       0,     0,     0,   228,   228,     0,     0,   351,     0,     0,
       0,     0,   228,     0,     0,     0,     0,   217,   217,     0,
       0,   230,     0,     0,     0,     0,   228,     0,     0,  1442,
       0,  1318,     0,     0,     0,     0,  1319,   228,    59,    60,
      61,   179,   180,   350,  1320,   757,     0,     0,   228,     0,
       0,   216,   216,     0,     0,   229,     0,     0,     0,     0,
       0,     0,     0,     0,   360,   361,   362,     0,   228,     0,
       0,     0,     0,     0,     0,     0,     0,  1317,     0,     0,
       0,  1321,  1322,   363,  1323,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,   351,     0,
     215,   215,     0,     0,     0,     0,     0,     0,    13,   386,
       0,     0,     0,     0,   228,     0,   228,     0,   228,     0,
    1444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   757,     0,     0,   228,   757,   757,   757,
       0,     0,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,  1318,
       0,     0,     0,     0,  1319,     0,    59,    60,    61,   179,
     180,   350,  1320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   757,     0,     0,   217,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1321,
    1322,     0,  1323,   228,     0,   228,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   351,     0,     0,     0,
     228,     0,  1213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,     0,     0,     0,  1587,     0,
     228,     0,     0,   217,   217,   215,     0,     0,     0,     0,
     217,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,   215,   215,     0,   757,     0,     0,   567,   216,     0,
       0,     0,     0,     0,     0,     0,   228,   216,   216,   757,
       0,   757,     0,     0,   216,     0,     0,   360,   361,   362,
     216,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,   757,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,     0,     0,     0,     0,   230,     0,     0,     0,
     228,   228,   386,     0,   215,   429,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   757,
     229,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,     0,     0,   412,   413,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,   228,     0,     0,     0,     0,   757,   228,
       0,     0,     0,   757,     0,   757,     0,   757,     0,     0,
     757,     0,   757,     0,     0,   757,     0,     0,     0,     0,
       0,     0,     0,   228,   228,     0,   228,     0,     0,   414,
     415,     0,     0,   228,   763,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   757,
       0,     0,   824,  -905,  -905,  -905,  -905,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,   228,     0,     0,     0,     0,     0,     0,     0,
       0,   763,   386,   978,   979,   980,     0,   757,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   228,
     228,   228,   981,   804,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,   228,   217,     0,     0,   228,
       0,     0,     0,     0,     0,     0,   757,   263,  1003,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,     0,   385,
     216,   360,   361,   362,   757,   757,   757,     0,     0,     0,
       0,   386,   757,   228,     0,     0,     0,   228,     0,     0,
     363,   217,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,     0,   385,   360,   361,   362,     0,     0,
       0,     0,     0,     0,     0,   216,   386,     0,     0,     0,
       0,     0,     0,   217,   363,   217,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,     0,   385,   217,
       0,     0,     0,     0,     0,     0,     0,   216,     0,   216,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1160,     0,     0,     0,     0,
     757,     0,     0,   216,   763,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,   763,   763,
     763,   763,   763,     0,     0,     0,     0,     0,     0,     0,
     228,   763,     0,     0,   757,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,   757,     0,     0,     0,     0,
     757,     0,     0,   757,     0,   217,   217,  1016,     0,   263,
     263,     0,     0,     0,     0,   263,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,   861,     0,   567,     0,
       0,     0,     0,     0,     0,     0,     0,  1034,     0,   216,
     216,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1034,     0,     0,     0,     0,     0,
     757,     0,   216,     0,     0,     0,     0,     0,   228,     0,
     898,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,     0,   230,     0,     0,     0,     0,     0,
       0,   228,     0,     0,   263,   763,     0,   263,  1074,     0,
       0,     0,     0,     0,     0,     0,   228,     0,     0,     0,
       0,     0,     0,   360,   361,   362,   757,     0,   229,     0,
       0,     0,     0,     0,   757,     0,   217,   217,     0,     0,
     757,     0,   363,   757,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,    36,   385,   212,     0,     0,
     216,   216,   567,     0,     0,     0,     0,     0,   386,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,     0,     0,
       0,     0,     0,   763,     0,     0,   216,   763,   763,   763,
       0,  1003,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   263,
     737,     0,   677,   759,    84,    85,     0,    86,   184,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   763,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,     0,     0,
     759,     0,   678,     0,   117,     0,     0,     0,     0,     0,
       0,     0,     0,   216,     0,     0,   567,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,     0,   902,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216,   263,   263,     0,     0,   216,     0,     0,     0,   263,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   216,     0,   763,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   763,
       0,   763,     0,     0,     0,     0,     0,     0,     0,     0,
     360,   361,   362,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   763,     0,     0,     0,     0,   363,
     217,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,     0,   385,     0,     0,     0,     0,     0,     0,
       0,  1316,     0,     0,   216,   386,     0,     0,     0,   360,
     361,   362,   429,   400,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,     0,     0,     0,   363,   763,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,   759,     0,   567,     0,     0,     0,     0,
       0,   412,   413,     0,   386,   263,   263,   759,   759,   759,
     759,   759,     0,     0,     0,     0,     0,     0,     0,     0,
     759,     0,     0,     0,     0,     0,     0,     0,   763,   216,
       0,     0,     0,   763,     0,   763,     0,   763,     0,     0,
     763,    36,   763,   818,   819,   763,     0,     0,     0,     0,
       0,     0,     0,     0,  1399,     0,  1408,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   414,   415,   567,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   763,
       0,     0,     0,     0,     0,  1026,     0,     0,     0,     0,
       0,     0,   263,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,     0,   763,     0,     0,
      84,    85,     0,    86,   184,    88,   263,     0,     0,     0,
    1465,  1466,     0,     0,   759,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1081,     0,   360,   361,   362,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,   363,   763,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,     0,   385,
       0,     0,     0,     0,   763,   763,   763,     0,     0,     0,
       0,   386,   763,  1604,     0,     0,     0,  1408,     0,     0,
       0,     0,     0,   263,     0,   263,     0,   737,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   759,     0,     0,     0,   759,   759,   759,     0,
       0,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,     0,   385,
       0,   759,     0,   183,     0,     0,    82,     0,     0,    84,
      85,   386,    86,   184,    88,     0,     0,     0,     0,     0,
     763,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,     0,   263,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,  1090,     0,     0,   763,   360,   361,   362,  1644,   263,
       0,     0,     0,     0,     0,   763,     0,     0,     0,     0,
     763,     0,     0,   763,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,     0,   385,     0,
       0,     0,     0,   759,     0,     0,     0,     0,     0,     0,
     386,     0,     0,     0,     0,   263,     0,     0,   759,     0,
     759,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     763,     0,   360,   361,   362,     0,     0,     0,  1705,     0,
       0,     0,     0,   759,     0,     0,     0,     0,     0,     0,
       0,   363,  1399,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,   263,
       0,     0,     0,     0,     0,     0,   763,   386,   360,   361,
     362,     0,     0,     0,   763,     0,     0,     0,     0,     0,
     763,     0,     0,   763,     0,     0,     0,   363,   759,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
    1114,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,     0,   263,     0,     0,     0,     0,   759,     0,     0,
       0,     0,   759,     0,   759,     0,   759,     0,     0,   759,
       0,   759,     0,     0,   759,     0,     0,     0,     0,     0,
       0,     0,   263,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,     0,     0,     0,     0,     0,     0,   360,
     361,   362,     0,     0,     0,     0,     0,     0,   759,     0,
       0,     0,     0,     0,     0,     0,     0,  1427,   363,  1281,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,     0,     0,     0,   759,     0,     0,     0,
       0,     0,     0,     0,   386,     0,     0,     0,   263,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1428,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,     0,     0,     0,   263,     0,
       0,     0,     0,     0,     0,   759,     0,   188,   190,     0,
     192,   193,   194,   196,   197,     0,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,     0,     0,
     222,   225,     0,   759,   759,   759,     0,     0,     0,     0,
       0,   759,     0,     0,   243,     0,     0,     0,     0,     0,
       0,   251,     0,   254,     0,     0,   270,     0,   275,     0,
     360,   361,   362,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   764,     0,     0,     0,     0,   363,
       0,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,  1282,   385,   360,   361,   362,     0,     0,     0,
       0,     0,     0,     0,     0,   386,   326,     0,     0,     0,
       0,   764,     0,   363,     0,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,     0,   759,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   386,
     263,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,  1646,
     385,     0,     0,   759,     0,     0,     0,     0,     0,     0,
       0,     0,   386,   430,   759,     0,   360,   361,   362,   759,
       0,     0,   759,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   363,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   455,   385,
       0,   455,     0,     0,     0,     0,     0,     0,   243,   466,
       0,   386,     0,   387,     0,     0,     0,     0,     0,   759,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,   758,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,  1003,     0,     0,     0,     0,   326,   470,     0,     0,
       0,     0,   222,     0,     0,   263,   549,     0,     0,     0,
       0,     0,     0,     0,     0,   759,     0,     0,     0,   570,
     573,   573,     0,   759,   764,   758,     0,     0,     0,   759,
       0,     0,   759,   597,     0,     0,     0,     0,   764,   764,
     764,   764,   764,     0,   609,     0,     0,     0,     0,     0,
       0,   764,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   615,   616,   617,   619,   620,   621,   622,   623,
     624,   625,   626,   627,   628,   629,   630,   631,   632,   633,
     634,   635,   636,   637,   638,   639,   640,     0,   642,   472,
     643,   643,     0,   646,     0,     0,     0,     0,     0,     0,
       0,   663,   665,   666,   667,   668,   669,   670,   671,   672,
     673,   674,   675,   676,     0,     0,     0,     0,     0,   643,
     686,     0,   609,   609,   643,   690,     0,     0,     0,     0,
       0,   663,     0,     0,   694,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   703,     0,   705,     0,     0,     0,
       0,     0,   609,     0,     0,     0,     0,     0,     0,     0,
     720,     0,   721,     0,     0,   764,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,   361,   362,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   769,     0,
       0,   772,   775,   776,   363,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,     0,   385,     0,
       0,     0,   795,     0,     0,     0,     0,     0,   758,     0,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   758,   758,   758,   758,   758,     0,     0,     0,
       0,     0,     0,     0,     0,   758,     0,     0,     0,     0,
       0,     0,     0,   764,     0,     0,     0,   764,   764,   764,
       0,     0,   764,   764,   764,   764,   764,   764,   764,   764,
     764,   764,   764,   764,   764,   764,   764,   764,   764,   764,
     764,   764,   764,   764,   764,   764,   764,   764,   764,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   866,     0,
     360,   361,   362,     0,     0,     0,     0,     0,     0,     0,
     877,     0,   764,     0,     0,     0,     0,     0,     0,   363,
       0,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   885,   385,     0,     0,     0,     0,     0,     0,
       0,   196,   360,   361,   362,   386,     0,     0,   483,   758,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     895,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,   926,     0,     0,     0,     0,   386,     0,     0,
       0,     0,     0,     0,   764,     0,     0,     0,   243,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   764,
       0,   764,     0,     0,     0,     0,     0,     0,    36,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   764,     0,     0,   758,     0,  1004,
       0,   758,   758,   758,     0,     0,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   507,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,     0,  1041,   758,    84,    85,   764,
      86,   184,    88,     0,     0,     0,     0,  1003,  1049,    36,
       0,     0,     0,     0,  1052,     0,  1053,     0,  1054,     0,
       0,     0,     0,     0,   693,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
    1070,     0,     0,     0,     0,   650,     0,   117,     0,     0,
    1078,     0,   765,  1079,     0,  1080,     0,     0,   764,   609,
       0,     0,     0,   764,     0,   764,     0,   764,     0,     0,
     764,     0,   764,     0,     0,   764,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   308,     0,     0,    84,    85,
       0,    86,   184,    88,  1111,     0,     0,     0,   758,   797,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   764,
       0,     0,     0,   758,     0,   758,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     360,   361,   362,     0,     0,     0,   309,     0,   758,     0,
       0,     0,     0,     0,     0,     0,     0,   764,     0,   363,
       0,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,     0,   385,     0,     0,     0,  1193,  1194,  1195,
       0,     0,     0,   772,  1197,   386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   764,     0,     0,     0,
       0,     0,     0,   758,     0,     0,     0,     0,     0,     0,
       0,  1212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   764,   764,   764,     0,     0,     0,
       0,     0,   764,     0,     0,     0,  1607,     0,    29,    30,
       0,  1238,     0,     0,     0,     0,     0,     0,    36,     0,
     212,     0,     0,     0,   609,     0,     0,     0,     0,     0,
       0,     0,   758,   609,  1212,     0,     0,   758,     0,   758,
       0,   758,     0,     0,   758,     0,   758,     0,     0,   758,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,   243,     0,     0,     0,     0,
       0,     0,   940,     0,     0,  1280,     0,     0,     0,     0,
       0,     0,     0,   758,     0,     0,   961,   962,   963,   964,
       0,   183,   715,     0,    82,    83,     0,    84,    85,   975,
      86,   184,    88,     0,     0,     0,     0,     0,     0,    91,
     764,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   758,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,   436,     0,   764,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,   764,     0,     0,     0,     0,
     764,  1336,     0,   764,     0,  1337,     0,  1338,  1339,     0,
     758,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1683,  1353,   609,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   758,   758,
     758,     0,   360,   361,   362,     0,   758,     0,     0,     0,
       0,     0,     0,  1071,     0,     0,     0,     0,     0,     0,
     764,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,   255,     0,  1391,     0,     0,     0,   386,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,     0,   764,   256,     0,     0,
       0,     0,     0,     0,   764,     0,     0,     0,  1003,   609,
     764,     0,     0,   764,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   758,  1153,  1156,  1156,     0,     0,
    1163,  1166,  1167,  1168,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,  1188,  1189,  1190,  1191,  1192,     0,   758,     0,
       0,     0,     0,     0,   257,   258,     0,     0,     0,   758,
       0,     0,     0,     0,   758,     0,     0,   758,     0,     0,
    1203,     0,   183,     0,     0,    82,   259,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,  1012,
       0,     0,     0,     0,     0,     0,   260,     0,     0,     0,
       0,     0,     0,     0,  1599,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,   261,   758,     0,     0,  1571,     0,     0,
       0,     0,     0,     0,  1474,  1475,  1476,  1477,  1478,     0,
       0,  1479,  1480,  1481,  1482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1483,  1484,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1276,     0,     0,     0,     0,     0,     0,     0,
     758,     0,     0,     0,  1485,     0,     0,  1290,   758,  1291,
       0,     0,     0,     0,   758,     0,     0,   758,  1486,  1487,
    1488,  1489,  1490,  1491,  1492,     0,     0,     0,    36,     0,
       0,     0,  1309,     0,     0,     0,     0,     0,  1493,  1494,
    1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,
    1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,     0,
       0,     0,  1534,  1535,     0,  1536,  1537,  1538,  1539,  1540,
       0,     0,     0,     0,     0,     0,     0,  1346,     0,     0,
       0,  1541,  1542,  1543,     0,     0,     0,    84,    85,     0,
      86,   184,    88,  1544,     0,  1545,  1546,     0,  1547,     0,
       0,     0,     0,     0,     0,  1548,     0,     0,     0,  1549,
       0,  1550,     0,  1551,  1552,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,     0,   360,   361,   362,  1383,     0,     0,     0,
       0,  1385,     0,  1386,     0,  1387,     0,     0,  1388,     0,
    1389,     0,   363,  1390,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,     0,   385,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,  1433,   386,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   456,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,  1458,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,  1577,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,   179,   180,    64,     0,    65,    66,    67,
       0,     0,  1596,  1597,  1598,     0,     0,     0,    71,    72,
    1603,    73,    74,    75,    76,    77,  1008,  1009,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   183,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   979,   980,   114,     0,
     115,   116,     0,   117,   118,     0,   119,   120,     0,     0,
       0,     0,     0,     0,   981,     0,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,     0,  1629,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1003,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1652,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,  1662,     0,     0,     0,     0,  1666,     0,
       0,  1668,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,  1699,     0,
       0,    49,    50,    51,    52,    53,    54,    55,     0,    56,
      57,    58,    59,    60,    61,    62,    63,    64,     0,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
      71,    72,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,    79,
      80,    81,    82,    83,  1751,    84,    85,     0,    86,    87,
      88,    89,  1757,     0,    90,     0,     0,    91,  1760,     0,
       0,  1761,     0,    92,    93,    94,    95,    96,     0,    97,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,     0,     0,
     114,     0,   115,   116,  1042,   117,   118,     0,   119,   120,
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
      50,    51,    52,    53,    54,    55,     0,    56,    57,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,    79,    80,    81,
      82,    83,     0,    84,    85,     0,    86,    87,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,    96,     0,    97,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
     115,   116,  1214,   117,   118,     0,   119,   120,     5,     6,
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
      52,    53,    54,    55,     0,    56,    57,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,    79,    80,    81,    82,    83,
       0,    84,    85,     0,    86,    87,    88,    89,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,    96,     0,    97,     0,    98,    99,     0,
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
     112,   113,     0,     0,   114,     0,   115,   116,   590,   117,
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
       0,     0,   114,     0,   115,   116,  1015,   117,   118,     0,
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
     114,     0,   115,   116,  1120,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,  1122,    45,     0,
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
     115,   116,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,  1277,     0,    48,     0,     0,     0,    49,    50,    51,
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
     112,   113,     0,     0,   114,     0,   115,   116,  1393,   117,
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
       0,     0,   114,     0,   115,   116,  1600,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,  1641,    47,     0,     0,    48,     0,     0,
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
     115,   116,  1672,   117,   118,     0,   119,   120,     5,     6,
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
    1673,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,  1676,    46,     0,    47,     0,
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
       0,     0,   114,     0,   115,   116,  1691,   117,   118,     0,
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
     114,     0,   115,   116,  1745,   117,   118,     0,   119,   120,
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
     115,   116,  1752,   117,   118,     0,   119,   120,     5,     6,
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
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   723,     0,     0,     0,     0,     0,     0,
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
       0,   942,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,     0,  1453,
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
       0,     0,     0,     0,    11,    12,     0,  1591,     0,     0,
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
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
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
     659,    12,     0,     0,     0,     0,     0,     0,   660,     0,
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
      95,     0,     0,     0,     0,    98,    99,   267,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,     0,     0,   114,     0,     0,     0,     0,   117,
     118,     0,   119,   120,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,   267,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,   268,     0,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   363,    10,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   605,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   386,     0,     0,
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
      88,     0,   606,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
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
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,   361,
     362,   718,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,   363,    10,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
    1068,   385,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    84,    85,     0,    86,   184,    88,     0,  1069,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,     0,     0,     0,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     659,    12,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   195,     0,     0,    52,     0,     0,     0,     0,     0,
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
       0,     0,   221,   611,     0,     0,     0,     0,     0,     0,
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
       0,     0,    90,     0,     0,    91,     5,     6,     7,     8,
       9,    92,    93,    94,    95,     0,    10,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,     0,
       0,     0,     0,   117,   118,     0,   119,   120,     0,    14,
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
     112,   113,     0,     0,   114,     0,   250,     0,     0,   117,
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
     114,     0,   253,     0,     0,   117,   118,     0,   119,   120,
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
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   114,   454,     0,     0,
       0,   117,   118,     0,   119,   120,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,   618,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1003,     0,     0,    14,
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
     109,   110,   111,   112,     0,     0,   660,   875,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,   179,   180,   181,
       0,    36,    66,    67,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    72,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,    78,     0,     0,     0,
       0,   183,    80,    81,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,   649,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
      84,    85,   114,    86,   184,    88,     0,   117,   118,     0,
     119,   120,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   702,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
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
      88,     0,     0,     0,    90,     0,  1161,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    84,    85,
     114,    86,   184,    88,     0,   117,   118,     0,   119,   120,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   704,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    84,    85,   114,    86,
     184,    88,     0,   117,   118,     0,   119,   120,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
    1110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     110,   111,   112,   113,     0,     0,   114,     0,     0,     0,
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
       5,     6,     7,     8,     9,    92,    93,    94,    95,     0,
      10,     0,     0,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   114,     0,     0,     0,     0,   117,   118,     0,
     119,   120,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,   548,    38,     0,     0,     0,
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
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   114,   360,
     361,   362,     0,   117,   118,     0,   119,   120,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   363,     0,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,     0,   385,   360,   361,   362,     0,     0,   936,     0,
       0,     0,     0,     0,   386,     0,     0,     0,     0,     0,
       0,     0,   363,     0,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,     0,   385,   360,   361,   362,
      36,     0,   212,     0,     0,     0,     0,     0,   386,     0,
       0,     0,     0,     0,     0,     0,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   255,
     385,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   386,     0,   937,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   256,     0,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,    36,     0,  1642,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   255,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,   214,     0,     0,     0,     0,   117,
       0,   256,  1462,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,   259,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,  1311,     0,     0,     0,     0,
       0,  -326,     0,     0,   260,     0,     0,     0,     0,    59,
      60,    61,   179,   180,   350,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   257,   258,
       0,   261,   255,     0,     0,  1637,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
     259,     0,    84,    85,     0,    86,   184,    88,   256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,   351,
      36,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   261,   255,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   477,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   257,   258,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,   259,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,     0,   255,
       0,     0,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   257,   258,     0,   261,   256,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    82,   259,     0,    84,    85,    36,    86,   184,
      88,     0,   956,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,   255,     0,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     261,   256,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,   259,     0,    84,    85,     0,    86,
     184,    88,     0,  1287,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   257,   258,
       0,   261,     0,    36,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
     259,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,   213,     0,     0,     0,  1169,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   743,   744,     0,   261,     0,     0,
     745,     0,   746,     0,     0,     0,   183,     0,     0,    82,
      83,     0,    84,    85,   747,    86,   184,    88,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,   748,     0,     0,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,     0,
     520,     0,   117,  -905,  -905,  -905,  -905,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,   893,
     749,     0,    73,    74,    75,    76,    77,     0,    36,     0,
     212,     0,  1003,   750,     0,     0,     0,     0,   183,    80,
      81,    82,   751,     0,    84,    85,     0,    86,   184,    88,
       0,    36,     0,    90,     0,     0,     0,     0,     0,     0,
     736,     0,   752,   753,   754,   755,     0,     0,     0,   213,
      98,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   743,   744,     0,   756,
       0,     0,   745,     0,   746,     0,     0,     0,     0,     0,
       0,   183,     0,     0,    82,    83,   747,    84,    85,     0,
      86,   184,    88,     0,    33,    34,    35,    36,     0,     0,
       0,     0,     0,     0,   183,     0,   748,    82,     0,     0,
      84,    85,     0,    86,   184,    88,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,   214,     0,     0,     0,     0,   117,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   749,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,   750,     0,     0,     0,     0,
     183,    80,    81,    82,   751,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,    90,     0,     0,     0,     0,
       0,     0,     0,     0,   752,   753,   754,   755,   904,   905,
       0,     0,    98,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   906,     0,
       0,   756,   360,   361,   362,     0,   907,   908,   909,    36,
       0,     0,     0,     0,     0,     0,     0,     0,   910,     0,
       0,   363,   878,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,    29,    30,
       0,     0,     0,     0,     0,     0,     0,   386,    36,     0,
      38,     0,     0,     0,   911,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   912,     0,     0,
       0,     0,     0,     0,     0,     0,    52,     0,    84,    85,
       0,    86,   184,    88,    59,    60,    61,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   913,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      36,   183,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   184,    88,    36,     0,   212,     0,     0,     0,    91,
       0,     0,     0,     0,     0,     0,     0,     0,   276,   277,
       0,     0,     0,     0,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
       0,     0,   436,     0,   213,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   540,     0,     0,
       0,     0,     0,     0,     0,     0,   278,     0,     0,    84,
      85,     0,    86,   184,    88,     0,   183,     0,     0,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    36,     0,
     212,     0,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,   213,
       0,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1037,    36,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   183,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    36,
       0,   212,   214,     0,     0,     0,   183,   117,     0,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,    36,     0,   212,     0,     0,     0,     0,     0,
       0,   562,     0,     0,     0,     0,     0,     0,     0,     0,
     226,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,     0,
       0,     0,   117,   213,     0,     0,    36,     0,   212,     0,
       0,     0,   183,     0,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   184,    88,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      36,     0,   212,   227,     0,     0,     0,     0,   117,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   677,     0,    84,    85,     0,    86,   184,
      88,   563,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   649,     0,     0,
       0,    36,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,  1093,  1094,
    1095,    36,     0,   710,     0,   117,     0,     0,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,     0,     0,     0,   650,  1558,   117,
      84,    85,  1559,    86,   184,    88,     0,     0,     0,  1412,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,    84,    85,  1413,    86,   184,    88,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,     0,     0,     0,   360,   361,
     362,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   363,  1413,   364,
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
       0,     0,     0,     0,     0,     0,     0,   363,   844,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   363,   881,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,     0,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   360,   361,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   363,   922,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,   978,   979,   980,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,     0,     0,     0,
       0,   981,  1226,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1003,   978,   979,
     980,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   981,  1244,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
       0,     0,   978,   979,   980,     0,     0,     0,     0,     0,
       0,     0,     0,  1003,     0,     0,     0,     0,     0,     0,
       0,   981,  1144,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1003,   978,   979,
     980,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   981,  1300,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
       0,     0,   978,   979,   980,     0,     0,     0,     0,     0,
       0,     0,     0,  1003,     0,     0,     0,     0,     0,     0,
       0,   981,  1305,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,  1003,   978,   979,
     980,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,   981,  1382,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1003,  1400,     0,    36,     0,     0,     0,
       0,     0,  1460,     0,     0,     0,   183,  1401,  1402,    82,
      83,    36,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    82,  1403,
       0,    84,    85,     0,    86,  1404,    88,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,    36,     0,  1461,     0,
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
     108,   109,   110,   111,   112,   727,   363,     0,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,     0,
     385,     0,   360,   361,   362,     0,     0,     0,     0,     0,
       0,     0,   386,     0,     0,     0,     0,     0,     0,     0,
       0,   363,     0,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   728,   385,   978,   979,   980,     0,
       0,     0,     0,     0,     0,     0,     0,   386,     0,     0,
       0,     0,     0,     0,     0,   981,  1310,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,   978,   979,
     980,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1003,     0,     0,     0,     0,     0,   981,     0,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1003,     0,     0,     0,   363,     0,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     980,   385,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   386,     0,     0,     0,   981,     0,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   981,  1003,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1003,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
       0,   385,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   386,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1003,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,     0,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   386
};

static const yytype_int16 yycheck[] =
{
       4,   163,   136,     4,   186,   316,   175,   608,     4,    89,
      53,    32,     4,   695,    54,     4,    96,    97,     4,     4,
     228,     4,   855,    44,   420,   421,   399,    48,   426,  1060,
     581,   580,   234,   837,   385,   452,  1057,   166,   452,   114,
      26,    27,   726,  1044,   114,   163,    50,   874,     9,    53,
     114,    30,   555,   187,   450,   135,   733,     9,     9,   935,
      30,  1046,     9,   890,    30,     9,    70,     4,    45,     9,
       9,    66,    45,     9,   255,   256,     9,    79,     9,     9,
     261,     9,     9,    87,     9,    89,    33,     9,     9,    79,
       9,     9,    96,    97,     9,     9,     9,     9,    66,   301,
       9,    66,    57,    26,    27,    79,   235,    66,    50,     9,
       9,   109,    66,  1585,   128,   129,    50,    79,   170,    79,
      66,   224,    79,    79,    79,    45,    79,    82,   114,    45,
      79,   135,   155,   128,   129,    45,   155,   128,   129,   214,
     136,   117,    85,     0,   214,   168,    35,   316,   124,   201,
      35,    53,   227,    35,   201,   170,    85,   128,   129,   340,
     201,    66,   163,    65,   201,   456,  1638,   971,     8,   167,
     352,   149,    66,    66,    66,   149,    66,    35,    66,    66,
      66,   204,   128,   129,   149,   204,   201,    66,   202,    66,
      79,   187,   170,    66,    79,   199,   201,    79,    66,    66,
      30,   163,   201,   146,   206,    66,   154,   163,   201,   204,
     163,   202,   154,   504,   163,   153,   206,   146,  1239,   294,
     154,    79,   199,   392,   294,   202,   204,   169,   214,   202,
     294,   171,   171,   201,   204,   221,   240,   203,   198,   204,
     244,   227,   203,   204,   248,   204,   428,   416,  1249,   206,
     204,  1127,   203,   205,   240,   202,  1241,   240,   244,   203,
    1087,   244,   266,  1248,   398,  1250,   346,   203,   945,   202,
     947,   440,   203,   203,   163,   203,   203,    97,   203,   199,
     449,   203,   203,   452,   203,   203,   202,   202,   202,   202,
     202,   214,   202,   202,   475,    97,   201,   283,   221,   204,
     343,  1105,   202,   202,   227,   163,   292,   293,   294,   201,
     204,   204,   204,   299,   204,   395,   396,   204,   204,   305,
     871,   276,   277,   278,   827,   204,   330,   204,  1349,   330,
    1351,   204,   435,   153,   330,   339,   204,   204,    97,   343,
     201,   202,   346,    35,   330,   204,   100,   101,   100,   101,
     201,   153,    79,   308,    35,    97,    35,    79,    85,   399,
     283,   436,    79,    85,   128,   129,   436,   116,    85,   292,
     293,   294,   170,   201,  1359,   124,   299,    97,    79,    97,
     201,   389,   305,   201,   586,    29,   201,   324,   392,   393,
     394,   395,   396,   201,   153,   201,   577,   578,    79,   204,
      79,   397,    46,   201,   585,    49,   385,   153,   201,   417,
     201,   153,   416,   392,   422,   201,   814,    79,   458,  1440,
     147,   148,   201,    85,  1106,   147,   148,   718,    79,   146,
     147,   148,   723,   153,   596,   153,   440,   416,    14,  1123,
     426,   205,   859,   651,    66,   859,   147,   148,   452,    66,
     436,   205,   203,   205,    30,   105,   203,   203,   204,   463,
     203,   440,   565,   113,   114,   115,   116,   117,   118,   572,
     449,    47,   575,   452,   170,   683,   658,   463,   596,     4,
     463,    66,   163,   210,   163,   147,   148,   209,    46,    47,
    1294,   887,   203,   701,   203,   146,   147,   148,   700,    29,
     149,    79,   204,   426,   706,   201,   923,    85,   203,   923,
    1059,    79,   203,   436,   149,   519,    46,    85,   201,    49,
      45,   170,  1073,    71,    72,  1076,   837,    46,    47,    48,
      49,    50,    51,   183,   614,    49,    50,    51,   201,    53,
     544,   545,   893,    66,    79,   648,    65,    71,    72,   757,
      85,    65,   201,   508,   149,   204,   153,   512,   203,   204,
     741,   742,   517,  1367,    74,    75,    76,  1588,   146,   147,
     148,   126,   127,  1255,   201,  1257,    86,   706,   203,   147,
     148,   106,    99,   100,   101,    44,   111,    65,   113,   114,
     115,   116,   117,   118,   119,   596,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   170,
     614,   149,   147,   148,    49,    50,    51,   203,   204,   605,
      99,   100,   101,     9,   134,   135,   136,   137,   138,   302,
      65,   156,   157,   306,   159,   145,   739,   113,   114,   115,
     149,   151,   152,   208,   581,    63,    64,   828,   118,   119,
     120,   942,  1614,  1615,   201,   165,  1610,  1611,   183,   332,
     971,   334,   335,   336,   337,   149,  1039,   201,   837,     8,
     851,   203,   182,   170,   660,   741,   742,    14,    14,  1228,
     205,   862,   605,  1365,    79,  1716,  1082,   203,   201,  1710,
     859,   695,  1243,   697,   124,  1091,   203,   124,    14,   202,
    1731,   722,   170,    14,  1725,    97,   202,   202,   202,   207,
     128,   129,   716,   201,   105,   716,   702,     9,   704,   201,
     716,   201,    89,     9,    14,   202,   730,   731,  1126,   202,
     716,   113,   114,   115,   116,   117,   118,   660,   203,   184,
     185,   186,   728,   201,     9,   731,   191,   192,   731,   187,
     195,   196,    79,    79,   923,    79,   113,   114,   115,   116,
     117,   118,   190,   203,   201,  1447,     9,   203,   949,     9,
     951,    46,    47,    48,    49,    50,    51,   781,    53,   702,
     781,   704,    79,   787,   202,   781,   202,   791,   792,   202,
      65,   202,   126,   203,  1105,   781,   201,    66,   202,   127,
      30,   183,   971,   169,     9,   728,   202,   811,   130,  1360,
       9,  1209,   892,   799,   917,   202,   149,   202,     9,   202,
      14,   203,   826,   199,     9,   826,   183,    66,   814,   815,
     826,   786,     9,   171,   826,   790,   840,   826,   202,     9,
     826,   826,    14,   826,   126,   208,   208,   205,     9,  1245,
     201,   201,   208,  1686,   202,   859,   202,   960,  1066,   208,
     202,    97,   203,   203,   967,   149,   130,     9,   149,   201,
    1703,   202,   201,   149,   204,   187,   799,  1058,  1711,  1060,
     859,   201,   113,   114,   115,   116,   117,   118,   892,   826,
      14,   814,   815,   124,   125,   201,   201,     9,   187,    79,
     904,   905,   906,   204,  1085,    14,   204,   928,  1590,  1200,
     203,    14,   208,   204,   893,    14,   203,    30,   855,   923,
     199,   201,    30,   201,   201,   929,    14,   931,   201,   933,
     931,   162,   933,   201,   871,   931,  1105,   933,     9,   202,
     130,   203,   203,   929,   923,   931,   929,   933,   952,   935,
     936,  1347,   183,   113,   114,   115,   116,   117,   118,   201,
    1141,    14,     9,    65,   124,   125,   970,   202,     9,    79,
     208,   146,   976,     9,    97,   201,  1267,   130,   203,    14,
     202,    26,    27,  1294,    79,    30,   204,   201,   201,   204,
     202,   130,  1283,   204,     9,   146,    30,    73,  1101,  1039,
     160,   208,   162,   203,   202,   171,   203,    30,    53,   130,
     202,   130,   935,   936,   202,  1019,     9,     9,  1022,   202,
     202,   202,     9,   183,  1205,     9,   202,   205,   202,    14,
      79,  1632,   205,   204,   201,   204,   130,   202,   202,  1043,
     202,     9,  1043,   201,    30,   202,   202,  1043,  1151,   203,
     202,  1043,  1155,   202,  1043,  1158,  1367,  1043,  1043,   202,
    1043,   158,  1165,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   203,   203,  1369,   204,
      97,   154,  1068,    14,    79,   111,   202,  1378,   202,   204,
     130,   202,   130,    14,  1115,    14,    79,   204,  1102,   203,
    1701,  1392,  1106,    79,   202,  1286,  1043,  1288,   201,   130,
     202,    14,    63,    64,  1118,   204,   203,   203,    14,   203,
      14,   204,    55,    79,  1110,  1294,  1130,   201,     9,  1130,
      79,   203,  1118,    79,  1130,  1118,  1073,   109,    97,  1076,
    1126,  1127,   149,    97,  1130,  1068,    33,  1328,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1263,    53,  1453,   161,    14,   202,   167,    79,  1330,   214,
     201,   203,   201,    65,   164,   202,   221,   128,   129,     9,
      79,   203,   227,   202,   202,    14,    79,  1110,   204,    14,
      79,    14,    14,    79,  1328,    79,   517,   786,  1367,   790,
    1204,  1694,   395,  1126,  1127,   891,  1207,   396,   888,   393,
     255,   256,  1707,  1394,   829,  1216,   261,  1124,  1452,  1280,
    1703,  1443,   522,  1209,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1418,  1472,   283,  1556,
    1323,  1735,  1723,  1568,    42,  1439,  1014,   292,   293,    65,
    1011,  1255,   399,  1257,   299,  1047,   493,   778,  1089,   493,
     305,   968,   905,  1102,   919,   859,   974,   300,   319,   293,
    1207,   316,   741,  1030,    -1,  1318,   953,    -1,    -1,  1216,
      -1,    -1,    -1,    -1,    -1,    -1,  1209,    -1,    -1,  1580,
      -1,  1582,    -1,    -1,    -1,   340,    -1,    -1,   343,    -1,
    1591,    -1,    -1,    -1,    -1,    -1,  1243,    -1,    -1,  1313,
      -1,    -1,  1313,    -1,  1318,    -1,    -1,  1313,    -1,  1323,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1313,    -1,  1330,
      -1,  1327,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     385,    -1,    -1,    -1,  1635,    -1,    -1,    -1,    -1,  1350,
      -1,    -1,    -1,    -1,    -1,  1356,    -1,  1358,    -1,    -1,
      -1,  1365,    -1,    -1,    -1,    -1,  1370,    -1,    -1,  1370,
      -1,  1375,    -1,    -1,  1370,  1379,    -1,  1420,  1379,    -1,
      -1,   426,    -1,  1379,  1370,    -1,     4,    -1,  1570,  1375,
      -1,   436,  1375,  1379,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1407,    -1,    -1,    -1,    -1,    -1,  1413,
      -1,    -1,    -1,  1350,    -1,    -1,  1420,    -1,    -1,  1356,
    1424,  1358,    -1,  1360,    -1,    -1,  1422,    45,    -1,    -1,
     475,   476,    -1,    -1,   479,    -1,    -1,    -1,  1729,    -1,
      -1,    -1,    -1,  1447,  1445,  1736,  1450,  1451,  1452,  1450,
      -1,    -1,  1456,    -1,  1450,  1456,    -1,    -1,    -1,  1463,
    1456,    -1,  1463,    -1,  1450,  1451,  1452,  1463,  1451,  1452,
    1456,    -1,    -1,    -1,    -1,    -1,    -1,  1463,    -1,    -1,
      -1,   526,    -1,    -1,    -1,    -1,    -1,  1567,   106,    -1,
      -1,    -1,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,  1445,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1682,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   577,   578,    -1,  1716,    -1,    -1,   156,   157,
     585,   159,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,
    1731,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     605,    -1,    -1,  1567,  1698,   183,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1586,    53,    -1,    -1,  1590,   205,    -1,    -1,
      -1,  1595,    45,    -1,  1595,    65,    -1,    -1,    -1,  1595,
      -1,  1605,    -1,    -1,    -1,    -1,  1610,  1611,     4,  1595,
    1614,  1615,    -1,    -1,    -1,   660,    -1,    -1,    -1,    -1,
      -1,    -1,  1626,    -1,    -1,    -1,    -1,    -1,    -1,  1633,
    1634,    -1,  1633,  1634,    -1,    -1,  1640,  1633,  1634,  1640,
      -1,    -1,    -1,    -1,  1640,    -1,    -1,  1633,  1634,    45,
      -1,    -1,    -1,   106,  1640,    -1,    -1,   702,   111,   704,
     113,   114,   115,   116,   117,   118,   119,    -1,    -1,    -1,
    1674,    -1,    -1,  1674,    -1,    -1,    -1,  1681,  1674,    -1,
      -1,  1682,    -1,   728,   729,    -1,    -1,    -1,  1674,    -1,
      -1,    -1,    -1,  1697,    -1,    -1,   741,   742,   743,   744,
     745,   746,   747,   156,   157,    -1,   159,    -1,    -1,     4,
     106,   756,    -1,    -1,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,  1737,    -1,    -1,  1737,   782,    -1,    -1,
    1744,  1737,    -1,  1744,    -1,    -1,    -1,    -1,  1744,  1686,
      45,  1737,   205,    -1,   799,    -1,    -1,    -1,  1744,    -1,
     156,   157,    -1,   159,    -1,    -1,  1703,   812,    -1,   814,
     815,    -1,    -1,    -1,  1711,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   828,   829,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   837,    -1,    -1,    -1,    -1,    26,    27,    -1,
      -1,    30,    -1,    -1,    -1,    -1,   851,    -1,    -1,   205,
      -1,   106,    -1,    -1,    -1,    -1,   111,   862,   113,   114,
     115,   116,   117,   118,   119,   870,    -1,    -1,   873,    -1,
      -1,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,   893,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
      -1,   156,   157,    29,   159,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   183,    -1,
     935,   936,    -1,    -1,    -1,    -1,    -1,    -1,    45,    65,
      -1,    -1,    -1,    -1,   949,    -1,   951,    -1,   953,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   968,    -1,    -1,   971,   972,   973,   974,
      -1,    -1,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,   106,
      -1,    -1,    -1,    -1,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1027,    -1,    -1,   214,    -1,    -1,    -1,    -1,
      -1,    -1,   221,    -1,    -1,    -1,    -1,    -1,   227,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
     157,    -1,   159,  1058,    -1,  1060,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1068,    -1,    -1,   221,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
    1085,    -1,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   283,    -1,    -1,    -1,   205,    -1,
    1105,    -1,    -1,   292,   293,  1110,    -1,    -1,    -1,    -1,
     299,    -1,    -1,    -1,    -1,    -1,   305,    -1,    -1,    -1,
      -1,  1126,  1127,    -1,  1129,    -1,    -1,   316,   283,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,   292,   293,  1144,
      -1,  1146,    -1,    -1,   299,    -1,    -1,    10,    11,    12,
     305,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   316,    -1,    -1,  1169,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,   385,    -1,    -1,    -1,
    1205,  1206,    65,    -1,  1209,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1234,
     385,    -1,    -1,    -1,    -1,    -1,    -1,   426,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   436,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   426,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1286,    -1,  1288,    -1,    -1,    -1,    -1,  1293,  1294,
      -1,    -1,    -1,  1298,    -1,  1300,    -1,  1302,    -1,    -1,
    1305,    -1,  1307,    -1,    -1,  1310,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1318,  1319,    -1,  1321,    -1,    -1,   128,
     129,    -1,    -1,  1328,   479,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1344,
      -1,    -1,   205,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,  1367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   526,    65,    10,    11,    12,    -1,  1382,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1394,
    1395,  1396,    29,   202,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1420,   605,    -1,    -1,  1424,
      -1,    -1,    -1,    -1,    -1,    -1,  1431,    53,    65,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     605,    10,    11,    12,  1459,  1460,  1461,    -1,    -1,    -1,
      -1,    65,  1467,  1468,    -1,    -1,    -1,  1472,    -1,    -1,
      29,   660,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   660,    65,    -1,    -1,    -1,
      -1,    -1,    -1,   702,    29,   704,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   728,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   702,    -1,   704,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,
    1575,    -1,    -1,   728,   729,    -1,    -1,    -1,    -1,    -1,
      -1,  1586,    -1,    -1,    -1,    -1,    -1,    -1,   743,   744,
     745,   746,   747,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1605,   756,    -1,    -1,  1609,    -1,    -1,    -1,    -1,    -1,
     799,    -1,    -1,    -1,    -1,  1620,    -1,    -1,    -1,    -1,
    1625,    -1,    -1,  1628,    -1,   814,   815,   782,    -1,   255,
     256,    -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   799,    -1,   205,    -1,   837,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   812,    -1,   814,
     815,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   829,    -1,    -1,    -1,    -1,    -1,
    1685,    -1,   837,    -1,    -1,    -1,    -1,    -1,  1693,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1707,    -1,   893,    -1,    -1,    -1,    -1,    -1,
      -1,  1716,    -1,    -1,   340,   870,    -1,   343,   873,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1731,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,  1741,    -1,   893,    -1,
      -1,    -1,    -1,    -1,  1749,    -1,   935,   936,    -1,    -1,
    1755,    -1,    29,  1758,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    77,    53,    79,    -1,    -1,
     935,   936,   971,    -1,    -1,    -1,    -1,    -1,    65,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,   968,    -1,    -1,   971,   972,   973,   974,
      -1,    65,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,   475,
     476,    -1,   154,   479,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1027,    -1,    -1,    -1,    -1,    -1,    -1,  1068,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,
     526,    -1,   204,    -1,   206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1068,    -1,    -1,  1105,    -1,    -1,    -1,
      -1,  1110,    -1,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1126,  1127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1105,   577,   578,    -1,    -1,  1110,    -1,    -1,    -1,   585,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1126,  1127,    -1,  1129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1144,
      -1,  1146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1169,    -1,    -1,    -1,    -1,    29,
    1209,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1206,    -1,    -1,  1209,    65,    -1,    -1,    -1,    10,
      11,    12,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    29,  1234,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,   729,    -1,  1294,    -1,    -1,    -1,    -1,
      -1,    63,    64,    -1,    65,   741,   742,   743,   744,   745,
     746,   747,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     756,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1293,  1294,
      -1,    -1,    -1,  1298,    -1,  1300,    -1,  1302,    -1,    -1,
    1305,    77,  1307,    79,    80,  1310,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1319,    -1,  1321,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,  1367,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1344,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   828,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   851,    -1,  1382,    -1,    -1,
     156,   157,    -1,   159,   160,   161,   862,    -1,    -1,    -1,
    1395,  1396,    -1,    -1,   870,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    10,    11,    12,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,    29,  1431,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,  1459,  1460,  1461,    -1,    -1,    -1,
      -1,    65,  1467,  1468,    -1,    -1,    -1,  1472,    -1,    -1,
      -1,    -1,    -1,   949,    -1,   951,    -1,   953,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   968,    -1,    -1,    -1,   972,   973,   974,    -1,
      -1,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,  1027,    -1,   150,    -1,    -1,   153,    -1,    -1,   156,
     157,    65,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
    1575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1058,    -1,  1060,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   205,    -1,    -1,  1609,    10,    11,    12,   205,  1085,
      -1,    -1,    -1,    -1,    -1,  1620,    -1,    -1,    -1,    -1,
    1625,    -1,    -1,  1628,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,  1129,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,  1141,    -1,    -1,  1144,    -1,
    1146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1685,    -1,    10,    11,    12,    -1,    -1,    -1,  1693,    -1,
      -1,    -1,    -1,  1169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1707,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,  1205,
      -1,    -1,    -1,    -1,    -1,    -1,  1741,    65,    10,    11,
      12,    -1,    -1,    -1,  1749,    -1,    -1,    -1,    -1,    -1,
    1755,    -1,    -1,  1758,    -1,    -1,    -1,    29,  1234,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1286,    -1,  1288,    -1,    -1,    -1,    -1,  1293,    -1,    -1,
      -1,    -1,  1298,    -1,  1300,    -1,  1302,    -1,    -1,  1305,
      -1,  1307,    -1,    -1,  1310,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1328,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1344,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,  1382,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,  1394,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1420,    -1,    -1,    -1,  1424,    -1,
      -1,    -1,    -1,    -1,    -1,  1431,    -1,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      28,    29,    -1,  1459,  1460,  1461,    -1,    -1,    -1,    -1,
      -1,  1467,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    51,    -1,    -1,    54,    -1,    56,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   479,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   203,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   114,    -1,    -1,    -1,
      -1,   526,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,  1575,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
    1586,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1605,
      53,    -1,    -1,  1609,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,   191,  1620,    -1,    10,    11,    12,  1625,
      -1,    -1,  1628,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   236,    53,
      -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,   246,   247,
      -1,    65,    -1,   203,    -1,    -1,    -1,    -1,    -1,  1685,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   479,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1716,    65,    -1,    -1,    -1,    -1,   294,   203,    -1,    -1,
      -1,    -1,   300,    -1,    -1,  1731,   304,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1741,    -1,    -1,    -1,   317,
     318,   319,    -1,  1749,   729,   526,    -1,    -1,    -1,  1755,
      -1,    -1,  1758,   331,    -1,    -1,    -1,    -1,   743,   744,
     745,   746,   747,    -1,   342,    -1,    -1,    -1,    -1,    -1,
      -1,   756,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,    -1,   386,   203,
     388,   389,    -1,   391,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,    -1,    -1,    -1,    -1,    -1,   417,
     418,    -1,   420,   421,   422,   423,    -1,    -1,    -1,    -1,
      -1,   429,    -1,    -1,   432,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   442,    -1,   444,    -1,    -1,    -1,
      -1,    -1,   450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     458,    -1,   460,    -1,    -1,   870,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   486,    -1,
      -1,   489,   490,   491,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,   520,    -1,    -1,    -1,    -1,    -1,   729,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   743,   744,   745,   746,   747,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   756,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   968,    -1,    -1,    -1,   972,   973,   974,
      -1,    -1,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   606,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     618,    -1,  1027,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   650,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   659,    10,    11,    12,    65,    -1,    -1,   203,   870,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     678,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,   710,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,  1129,    -1,    -1,    -1,   726,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1144,
      -1,  1146,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1169,    -1,    -1,   968,    -1,   767,
      -1,   972,   973,   974,    -1,    -1,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,   203,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,   823,  1027,   156,   157,  1234,
     159,   160,   161,    -1,    -1,    -1,    -1,    65,   836,    77,
      -1,    -1,    -1,    -1,   842,    -1,   844,    -1,   846,    -1,
      -1,    -1,    -1,    -1,   202,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
     868,    -1,    -1,    -1,    -1,   204,    -1,   206,    -1,    -1,
     878,    -1,   479,   881,    -1,   883,    -1,    -1,  1293,   887,
      -1,    -1,    -1,  1298,    -1,  1300,    -1,  1302,    -1,    -1,
    1305,    -1,  1307,    -1,    -1,  1310,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,
      -1,   159,   160,   161,   922,    -1,    -1,    -1,  1129,   526,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1344,
      -1,    -1,    -1,  1144,    -1,  1146,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      10,    11,    12,    -1,    -1,    -1,   204,    -1,  1169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1382,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,  1005,  1006,  1007,
      -1,    -1,    -1,  1011,  1012,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1431,    -1,    -1,    -1,
      -1,    -1,    -1,  1234,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1039,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1459,  1460,  1461,    -1,    -1,    -1,
      -1,    -1,  1467,    -1,    -1,    -1,  1471,    -1,    67,    68,
      -1,  1069,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,  1082,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1293,  1091,  1092,    -1,    -1,  1298,    -1,  1300,
      -1,  1302,    -1,    -1,  1305,    -1,  1307,    -1,    -1,  1310,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    -1,    -1,  1123,    -1,    -1,    -1,    -1,
      -1,    -1,   729,    -1,    -1,  1133,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1344,    -1,    -1,   743,   744,   745,   746,
      -1,   150,   202,    -1,   153,   154,    -1,   156,   157,   756,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,   168,
    1575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1382,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,   201,    -1,  1609,    -1,    -1,   206,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1620,    -1,    -1,    -1,    -1,
    1625,  1219,    -1,  1628,    -1,  1223,    -1,  1225,  1226,    -1,
    1431,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1650,  1244,  1245,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1459,  1460,
    1461,    -1,    10,    11,    12,    -1,  1467,    -1,    -1,    -1,
      -1,    -1,    -1,   870,    -1,    -1,    -1,    -1,    -1,    -1,
    1685,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    29,    -1,  1311,    -1,    -1,    -1,    65,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,  1741,    55,    -1,    -1,
      -1,    -1,    -1,    -1,  1749,    -1,    -1,    -1,    65,  1347,
    1755,    -1,    -1,  1758,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1575,   972,   973,   974,    -1,    -1,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,    -1,  1609,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,    -1,    -1,  1620,
      -1,    -1,    -1,    -1,  1625,    -1,    -1,  1628,    -1,    -1,
    1027,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1462,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,   201,  1685,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1741,    -1,    -1,    -1,    53,    -1,    -1,  1144,  1749,  1146,
      -1,    -1,    -1,    -1,  1755,    -1,    -1,  1758,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,  1169,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1234,    -1,    -1,
      -1,   150,   151,   152,    -1,    -1,    -1,   156,   157,    -1,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,   178,
      -1,   180,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,    -1,    10,    11,    12,  1293,    -1,    -1,    -1,
      -1,  1298,    -1,  1300,    -1,  1302,    -1,    -1,  1305,    -1,
    1307,    -1,    29,  1310,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,  1344,    65,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,  1382,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,  1431,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,    -1,  1459,  1460,  1461,    -1,    -1,    -1,   131,   132,
    1467,   134,   135,   136,   137,   138,   193,   194,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    11,    12,   201,    -1,
     203,   204,    -1,   206,   207,    -1,   209,   210,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,  1575,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1609,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,  1620,    -1,    -1,    -1,    -1,  1625,    -1,
      -1,  1628,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,  1685,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,    -1,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,  1741,   156,   157,    -1,   159,   160,
     161,   162,  1749,    -1,   165,    -1,    -1,   168,  1755,    -1,
      -1,  1758,    -1,   174,   175,   176,   177,   178,    -1,   180,
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
     103,   104,   105,   106,   107,   108,    -1,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,   178,    -1,   180,    -1,   182,
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
     105,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   162,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,   178,    -1,   180,    -1,   182,   183,    -1,
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
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
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
     203,   204,    -1,   206,   207,    -1,   209,   210,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    96,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
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
      91,    -1,    93,    94,    95,    -1,    -1,    98,    -1,    -1,
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
      87,    88,    89,    -1,    91,    92,    93,    -1,    95,    -1,
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
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
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
     177,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
     207,    -1,   209,   210,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   203,    -1,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
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
     161,    -1,   163,    -1,   165,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
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
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    11,
      12,   204,    -1,   206,   207,    -1,   209,   210,     3,     4,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   102,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
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
      -1,   168,     3,     4,     5,     6,     7,   174,   175,   176,
     177,    -1,    13,    -1,    -1,   182,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,    -1,    -1,   201,    -1,   203,    -1,    -1,   206,
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
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,   202,    -1,    -1,
      -1,   206,   207,    -1,   209,   210,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    46,
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
     194,   195,   196,   197,    -1,    -1,    35,   201,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
      -1,    77,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,   124,   168,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
      -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     156,   157,   201,   159,   160,   161,    -1,   206,   207,    -1,
     209,   210,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
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
     161,    -1,    -1,    -1,   165,    -1,   124,   168,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
      -1,   182,   183,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   156,   157,
     201,   159,   160,   161,    -1,   206,   207,    -1,   209,   210,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
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
     115,   116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     165,    -1,    -1,   168,     3,     4,     5,     6,     7,   174,
     175,   176,   177,    -1,    13,    -1,    -1,   182,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,    -1,    -1,    -1,
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
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    10,
      11,    12,    -1,   206,   207,    -1,   209,   210,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    77,    -1,   190,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
      -1,    55,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,   174,    -1,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   132,   133,
      -1,   201,    29,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
      77,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   132,   133,    -1,   201,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,   156,   157,    77,   159,   160,
     161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   132,   133,
      -1,   201,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,   118,    -1,    -1,    -1,    30,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    46,    47,    -1,   201,    -1,    -1,
      52,    -1,    54,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    66,   159,   160,   161,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,
     204,    -1,   206,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    68,
     132,    -1,   134,   135,   136,   137,   138,    -1,    77,    -1,
      79,    -1,    65,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    77,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,   174,   175,   176,   177,    -1,    -1,    -1,   118,
     182,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    46,    47,    -1,   201,
      -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    66,   156,   157,    -1,
     159,   160,   161,    -1,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    86,   153,    -1,    -1,
     156,   157,    -1,   159,   160,   161,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   206,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    46,    47,
      -1,    -1,   182,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    66,    -1,
      -1,   201,    10,    11,    12,    -1,    74,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    67,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    77,    -1,
      79,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,   156,   157,
      -1,   159,   160,   161,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      77,   150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    77,    -1,    79,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,
      -1,    -1,    -1,    -1,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
      -1,    -1,   201,    -1,   118,    -1,    -1,   206,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,   118,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    77,
      -1,    79,   201,    -1,    -1,    -1,   150,   206,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   206,   118,    -1,    -1,    77,    -1,    79,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      77,    -1,    79,   201,    -1,    -1,    -1,    -1,   206,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   154,    -1,   156,   157,    -1,   159,   160,
     161,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    77,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    74,    75,
      76,    77,    -1,   204,    -1,   206,    -1,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,    -1,    -1,    -1,   204,   154,   206,
     156,   157,   158,   159,   160,   161,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,   156,   157,   201,   159,   160,   161,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    29,   201,    31,
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
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    97,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      12,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    65,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
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
     401,   406,   409,   427,   435,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   448,   469,   471,   473,   116,
     117,   118,   131,   150,   160,   218,   249,   326,   342,   437,
     342,   201,   342,   342,   342,   102,   342,   342,   425,   426,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,    79,   118,   201,   226,   400,   401,   437,   440,
     437,    35,   342,   452,   453,   342,   118,   201,   226,   400,
     401,   402,   436,   444,   449,   450,   201,   333,   403,   201,
     333,   349,   334,   342,   235,   333,   201,   201,   201,   333,
     203,   342,   218,   203,   342,    29,    55,   132,   133,   154,
     174,   201,   218,   229,   474,   486,   487,   184,   203,   339,
     342,   369,   371,   204,   242,   342,   105,   106,   153,   219,
     222,   225,    79,   206,   294,   295,   117,   124,   116,   124,
      79,   296,   201,   201,   201,   201,   218,   266,   475,   201,
     201,    79,    85,   146,   147,   148,   466,   467,   153,   204,
     225,   225,   218,   267,   475,   154,   201,   201,   201,   201,
     475,   475,    79,   198,   351,   332,   342,   343,   437,   441,
     231,   204,    85,   404,   466,    85,   466,   466,    30,   153,
     170,   476,   201,     9,   203,    35,   248,   154,   265,   475,
     118,   183,   249,   327,   203,   203,   203,   203,   203,   203,
      10,    11,    12,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    53,    65,   203,    66,    66,
     203,   204,   149,   125,   160,   162,   268,   325,   326,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    63,    64,   128,   129,   429,    66,   204,   434,
     201,   201,    66,   204,   206,   445,   201,   248,   249,    14,
     342,   203,   130,    44,   218,   424,   201,   332,   437,   441,
     149,   437,   130,   208,     9,   411,   332,   437,   476,   149,
     201,   405,   429,   434,   202,   342,    30,   233,     8,   353,
       9,   203,   233,   234,   334,   335,   342,   218,   280,   237,
     203,   203,   203,   487,   487,   170,   201,   105,   487,    14,
     218,    79,   203,   203,   203,   184,   185,   186,   191,   192,
     195,   196,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   384,   385,   386,   243,   109,   167,   203,   153,   220,
     223,   225,   153,   221,   224,   225,   225,     9,   203,    97,
     204,   437,     9,   203,   124,   124,    14,     9,   203,   437,
     470,   470,   332,   343,   437,   440,   441,   202,   170,   260,
     131,   437,   451,   452,    66,   429,   146,   467,    78,   342,
     437,    85,   146,   467,   225,   217,   203,   204,   255,   263,
     390,   392,    86,   206,   354,   355,   357,   401,   445,   471,
     342,   459,   461,   342,   458,   460,   458,    14,    97,   472,
     350,   352,   290,   291,   427,   428,   202,   202,   202,   202,
     205,   232,   233,   250,   257,   262,   427,   342,   207,   209,
     210,   218,   477,   478,   487,    35,   163,   292,   293,   342,
     474,   201,   475,   258,   248,   342,   342,   342,    30,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   402,   342,   342,   447,   447,   342,   454,   455,   124,
     204,   218,   444,   445,   266,   218,   267,   265,   249,    27,
      35,   336,   339,   342,   369,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   154,   204,   218,
     430,   431,   432,   433,   444,   447,   342,   292,   292,   447,
     342,   451,   248,   202,   342,   201,   423,     9,   411,   332,
     202,   218,    35,   342,    35,   342,   202,   202,   444,   292,
     204,   218,   430,   431,   444,   202,   231,   284,   204,   339,
     342,   342,    89,    30,   233,   278,   203,    28,    97,    14,
       9,   202,    30,   204,   281,   487,    86,   229,   483,   484,
     485,   201,     9,    46,    47,    52,    54,    66,    86,   132,
     145,   154,   174,   175,   176,   177,   201,   226,   227,   229,
     364,   365,   366,   400,   406,   407,   408,   187,    79,   342,
      79,    79,   342,   381,   382,   342,   342,   374,   384,   190,
     387,   231,   201,   241,   225,   203,     9,    97,   225,   203,
       9,    97,    97,   222,   218,   342,   295,   407,    79,     9,
     202,   202,   202,   202,   202,   202,   202,   203,    46,    47,
     481,   482,   126,   271,   201,     9,   202,   202,    79,    80,
     218,   468,   218,    66,   205,   205,   214,   216,    30,   127,
     270,   169,    50,   154,   169,   394,   130,     9,   411,   202,
     149,   202,     9,   411,   130,   202,     9,   411,   202,   487,
     487,    14,   353,   290,   199,     9,   412,   487,   488,   429,
     434,   205,     9,   411,   171,   437,   342,   202,     9,   412,
      14,   346,   251,   126,   269,   201,   475,   342,    30,   208,
     208,   130,   205,     9,   411,   342,   476,   201,   261,   256,
     264,   259,   248,    68,   437,   342,   476,   208,   205,   202,
     202,   208,   205,   202,    46,    47,    66,    74,    75,    76,
      86,   132,   145,   174,   218,   414,   416,   419,   422,   218,
     437,   437,   130,   429,   434,   202,   342,   285,    71,    72,
     286,   231,   333,   231,   335,    97,    35,   131,   275,   437,
     407,   218,    30,   233,   279,   203,   282,   203,   282,     9,
     171,   130,   149,     9,   411,   202,   163,   477,   478,   479,
     477,   407,   407,   407,   407,   407,   410,   413,   201,    85,
     149,   201,   201,   201,   201,   407,   149,   204,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    65,   342,   187,   187,    14,   193,   194,
     383,     9,   197,   387,    79,   205,   400,   204,   245,    97,
     223,   218,    97,   224,   218,   218,   205,    14,   437,   203,
       9,   171,   218,   272,   400,   204,   451,   131,   437,    14,
     208,   342,   205,   214,   487,   272,   204,   393,    14,   342,
     354,   218,   342,   342,   342,   203,   487,   199,    30,   480,
     428,    35,    79,   163,   430,   431,   433,   487,    35,   163,
     342,   407,   290,   201,   400,   270,   347,   252,   342,   342,
     342,   205,   201,   292,   271,    30,   270,   269,   475,   402,
     205,   201,    14,    74,    75,    76,   218,   415,   415,   416,
     417,   418,   201,    85,   146,   201,     9,   411,   202,   423,
      35,   342,   430,   431,   205,    71,    72,   287,   333,   233,
     205,   203,    90,   203,   275,   437,   201,   130,   274,    14,
     231,   282,    99,   100,   101,   282,   205,   487,   487,   218,
     483,     9,   202,   411,   130,   208,     9,   411,   410,   218,
     354,   356,   358,   407,   463,   465,   407,   462,   464,   462,
     202,   124,   218,   407,   456,   457,   407,   407,   407,    30,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   342,   342,   342,   382,   342,   372,    79,
     246,   218,   218,   407,   482,    97,     9,   300,   202,   201,
     336,   339,   342,   208,   205,   472,   300,   155,   168,   204,
     389,   396,   155,   204,   395,   130,   130,   203,   480,   487,
     353,   488,    79,   163,    14,    79,   476,   437,   342,   202,
     290,   204,   290,   201,   130,   201,   292,   202,   204,   487,
     204,   270,   253,   405,   292,   130,   208,     9,   411,   417,
     146,   354,   420,   421,   416,   437,   333,    30,    73,   233,
     203,   335,   274,   451,   275,   202,   407,    96,    99,   203,
     342,    30,   203,   283,   205,   171,   130,   163,    30,   202,
     407,   407,   202,   130,     9,   411,   202,   202,     9,   411,
     130,   202,     9,   411,   202,   130,   205,     9,   411,   407,
      30,   188,   202,   231,   218,   487,   400,     4,   106,   111,
     119,   156,   157,   159,   205,   301,   324,   325,   326,   331,
     427,   451,   205,   204,   205,    50,   342,   342,   342,   342,
     353,    35,    79,   163,    14,    79,   407,   201,   480,   202,
     300,   202,   290,   342,   292,   202,   300,   472,   300,   204,
     201,   202,   416,   416,   202,   130,   202,     9,   411,    30,
     231,   203,   202,   202,   202,   238,   203,   203,   283,   231,
     487,   487,   130,   407,   354,   407,   407,   407,   407,   407,
     407,   342,   204,   205,    97,   126,   127,   474,   273,   400,
     119,   132,   133,   154,   160,   310,   311,   312,   400,   158,
     316,   317,   122,   201,   218,   318,   319,   302,   249,   487,
       9,   203,   325,   202,   297,   154,   391,   205,   205,    79,
     163,    14,    79,   407,   292,   111,   344,   480,   205,   480,
     202,   202,   205,   204,   205,   300,   290,   130,   416,   354,
     231,   236,   239,    30,   233,   277,   231,   202,   407,   130,
     130,   130,   189,   231,   487,   400,   400,    14,     9,   203,
     204,   204,     9,   203,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    53,    67,    68,    69,    70,
      71,    72,    73,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   131,   132,   134,   135,   136,   137,
     138,   150,   151,   152,   162,   164,   165,   167,   174,   178,
     180,   182,   183,   218,   397,   398,     9,   203,   154,   158,
     218,   319,   320,   321,   203,    79,   330,   248,   303,   474,
     249,   205,   298,   299,   474,    14,    79,   407,   202,   201,
     204,   203,   204,   322,   344,   480,   297,   205,   202,   416,
     130,    30,   233,   276,   277,   231,   407,   407,   407,   342,
     205,   203,   203,   407,   400,   306,   313,   406,   311,    14,
      30,    47,   314,   317,     9,    33,   202,    29,    46,    49,
      14,     9,   203,   475,   330,    14,   248,   203,    14,   407,
      35,    79,   388,   231,   231,   204,   322,   205,   480,   416,
     231,    94,   190,   244,   205,   218,   229,   307,   308,   309,
       9,   205,   407,   398,   398,    55,   315,   320,   320,    29,
      46,    49,   407,    79,   201,   203,   407,   475,   407,    79,
       9,   412,   205,   205,   231,   322,    92,   203,    79,   109,
     240,   149,    97,   406,   161,    14,   304,   201,    35,    79,
     202,   205,   203,   201,   167,   247,   218,   325,   326,   407,
     288,   289,   428,   305,    79,   400,   245,   164,   218,   203,
     202,     9,   412,   113,   114,   115,   328,   329,   288,    79,
     273,   203,   480,   428,   488,   202,   202,   203,   203,   204,
     323,   328,    35,    79,   163,   480,   204,   231,   488,    79,
     163,    14,    79,   323,   231,   205,    35,    79,   163,    14,
      79,   407,   205,    79,   163,    14,    79,   407,    14,    79,
     407,   407
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
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2639 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { (yyval).reset();;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval)++;;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval).reset();;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    {;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    {;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    {;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    {;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13763 "hphp.tab.cpp"
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
#line 3082 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

