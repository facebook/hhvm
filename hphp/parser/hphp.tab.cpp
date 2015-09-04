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
#include <folly/Conv.h>
#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"

#define line0 r.line0
#define char0 r.char0
#define line1 r.line1
#define char1 r.char1

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
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;\
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;\
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
#define BEXP(...) _p->onBinaryOpExp(__VA_ARGS__);
#define UEXP(...) _p->onUnaryOpExp(__VA_ARGS__);

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
#line 651 "hphp.tab.cpp"

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
     T_SUPER = 334,
     T_SWITCH = 335,
     T_ENDSWITCH = 336,
     T_CASE = 337,
     T_DEFAULT = 338,
     T_BREAK = 339,
     T_GOTO = 340,
     T_CONTINUE = 341,
     T_FUNCTION = 342,
     T_CONST = 343,
     T_RETURN = 344,
     T_TRY = 345,
     T_CATCH = 346,
     T_THROW = 347,
     T_USE = 348,
     T_GLOBAL = 349,
     T_PUBLIC = 350,
     T_PROTECTED = 351,
     T_PRIVATE = 352,
     T_FINAL = 353,
     T_ABSTRACT = 354,
     T_STATIC = 355,
     T_VAR = 356,
     T_UNSET = 357,
     T_ISSET = 358,
     T_EMPTY = 359,
     T_HALT_COMPILER = 360,
     T_CLASS = 361,
     T_INTERFACE = 362,
     T_EXTENDS = 363,
     T_IMPLEMENTS = 364,
     T_OBJECT_OPERATOR = 365,
     T_NULLSAFE_OBJECT_OPERATOR = 366,
     T_DOUBLE_ARROW = 367,
     T_LIST = 368,
     T_ARRAY = 369,
     T_CALLABLE = 370,
     T_CLASS_C = 371,
     T_METHOD_C = 372,
     T_FUNC_C = 373,
     T_LINE = 374,
     T_FILE = 375,
     T_COMMENT = 376,
     T_DOC_COMMENT = 377,
     T_OPEN_TAG = 378,
     T_OPEN_TAG_WITH_ECHO = 379,
     T_CLOSE_TAG = 380,
     T_WHITESPACE = 381,
     T_START_HEREDOC = 382,
     T_END_HEREDOC = 383,
     T_DOLLAR_OPEN_CURLY_BRACES = 384,
     T_CURLY_OPEN = 385,
     T_DOUBLE_COLON = 386,
     T_NAMESPACE = 387,
     T_NS_C = 388,
     T_DIR = 389,
     T_NS_SEPARATOR = 390,
     T_XHP_LABEL = 391,
     T_XHP_TEXT = 392,
     T_XHP_ATTRIBUTE = 393,
     T_XHP_CATEGORY = 394,
     T_XHP_CATEGORY_LABEL = 395,
     T_XHP_CHILDREN = 396,
     T_ENUM = 397,
     T_XHP_REQUIRED = 398,
     T_TRAIT = 399,
     T_ELLIPSIS = 400,
     T_INSTEADOF = 401,
     T_TRAIT_C = 402,
     T_HH_ERROR = 403,
     T_FINALLY = 404,
     T_XHP_TAG_LT = 405,
     T_XHP_TAG_GT = 406,
     T_TYPELIST_LT = 407,
     T_TYPELIST_GT = 408,
     T_UNRESOLVED_LT = 409,
     T_COLLECTION = 410,
     T_SHAPE = 411,
     T_TYPE = 412,
     T_UNRESOLVED_TYPE = 413,
     T_NEWTYPE = 414,
     T_UNRESOLVED_NEWTYPE = 415,
     T_COMPILER_HALT_OFFSET = 416,
     T_ASYNC = 417,
     T_FROM = 418,
     T_WHERE = 419,
     T_JOIN = 420,
     T_IN = 421,
     T_ON = 422,
     T_EQUALS = 423,
     T_INTO = 424,
     T_LET = 425,
     T_ORDERBY = 426,
     T_ASCENDING = 427,
     T_DESCENDING = 428,
     T_SELECT = 429,
     T_GROUP = 430,
     T_BY = 431,
     T_LAMBDA_OP = 432,
     T_LAMBDA_CP = 433,
     T_UNRESOLVED_OP = 434
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
#line 885 "hphp.tab.cpp"

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
#define YYLAST   18851

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  282
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1021
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1856

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   434

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   207,     2,   204,    51,    35,   208,
     199,   200,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   201,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   206,    34,     2,   205,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   202,    33,   203,    54,     2,     2,     2,
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
     194,   195,   196,   197,   198
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
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   138,
     140,   142,   144,   146,   148,   150,   152,   154,   156,   158,
     160,   162,   164,   166,   168,   170,   172,   174,   176,   178,
     180,   182,   184,   186,   188,   190,   192,   194,   196,   198,
     200,   202,   204,   206,   208,   210,   212,   214,   216,   218,
     220,   222,   224,   226,   228,   230,   234,   236,   240,   242,
     246,   248,   250,   253,   257,   262,   264,   267,   271,   276,
     278,   281,   285,   290,   292,   296,   298,   302,   305,   307,
     310,   313,   319,   324,   327,   328,   330,   332,   334,   336,
     340,   346,   355,   356,   361,   362,   369,   370,   381,   382,
     387,   390,   394,   397,   401,   404,   408,   412,   416,   420,
     424,   428,   434,   436,   438,   440,   441,   451,   452,   463,
     469,   470,   484,   485,   491,   495,   499,   502,   505,   508,
     511,   514,   517,   521,   524,   527,   531,   534,   535,   540,
     550,   551,   552,   557,   560,   561,   563,   564,   566,   567,
     577,   578,   589,   590,   602,   603,   613,   614,   625,   626,
     635,   636,   646,   647,   655,   656,   665,   666,   674,   675,
     684,   686,   688,   690,   692,   694,   697,   701,   705,   708,
     711,   712,   715,   716,   719,   720,   722,   726,   728,   732,
     735,   736,   738,   741,   746,   748,   753,   755,   760,   762,
     767,   769,   774,   778,   784,   788,   793,   798,   804,   810,
     815,   816,   818,   820,   825,   826,   832,   833,   836,   837,
     841,   842,   850,   859,   866,   869,   875,   882,   887,   888,
     893,   899,   907,   914,   921,   929,   939,   948,   955,   963,
     969,   972,   977,   983,   987,   988,   992,   997,  1004,  1010,
    1016,  1023,  1032,  1040,  1043,  1044,  1046,  1049,  1052,  1056,
    1061,  1066,  1070,  1072,  1074,  1077,  1082,  1086,  1092,  1094,
    1098,  1101,  1102,  1105,  1109,  1112,  1113,  1114,  1119,  1120,
    1126,  1129,  1132,  1135,  1136,  1147,  1148,  1160,  1164,  1168,
    1172,  1177,  1182,  1186,  1192,  1195,  1198,  1199,  1206,  1212,
    1217,  1221,  1223,  1225,  1229,  1234,  1236,  1239,  1241,  1243,
    1248,  1255,  1257,  1259,  1264,  1266,  1268,  1272,  1275,  1278,
    1279,  1282,  1283,  1285,  1289,  1291,  1293,  1295,  1297,  1301,
    1306,  1311,  1316,  1318,  1320,  1323,  1326,  1329,  1333,  1337,
    1339,  1341,  1343,  1345,  1349,  1351,  1355,  1357,  1359,  1361,
    1362,  1364,  1367,  1369,  1371,  1373,  1375,  1377,  1379,  1381,
    1383,  1384,  1386,  1388,  1390,  1394,  1400,  1402,  1406,  1412,
    1417,  1421,  1425,  1429,  1434,  1438,  1442,  1446,  1449,  1451,
    1453,  1457,  1461,  1463,  1465,  1466,  1468,  1471,  1476,  1480,
    1487,  1490,  1494,  1501,  1503,  1505,  1507,  1509,  1511,  1518,
    1522,  1527,  1534,  1538,  1542,  1546,  1550,  1554,  1558,  1562,
    1566,  1570,  1574,  1578,  1582,  1585,  1588,  1591,  1594,  1598,
    1602,  1606,  1610,  1614,  1618,  1622,  1626,  1630,  1634,  1638,
    1642,  1646,  1650,  1654,  1658,  1662,  1665,  1668,  1671,  1674,
    1678,  1682,  1686,  1690,  1694,  1698,  1702,  1706,  1710,  1714,
    1720,  1725,  1727,  1730,  1733,  1736,  1739,  1742,  1745,  1748,
    1751,  1754,  1756,  1758,  1760,  1764,  1767,  1769,  1775,  1776,
    1777,  1789,  1790,  1803,  1804,  1808,  1809,  1814,  1815,  1822,
    1823,  1831,  1832,  1838,  1841,  1844,  1849,  1851,  1853,  1859,
    1863,  1869,  1873,  1876,  1877,  1880,  1881,  1886,  1891,  1895,
    1900,  1905,  1910,  1915,  1917,  1919,  1921,  1923,  1927,  1930,
    1934,  1939,  1942,  1946,  1948,  1951,  1953,  1956,  1958,  1960,
    1962,  1964,  1966,  1968,  1973,  1978,  1981,  1990,  2001,  2004,
    2006,  2010,  2012,  2015,  2017,  2019,  2021,  2023,  2026,  2031,
    2035,  2039,  2044,  2046,  2049,  2054,  2057,  2064,  2065,  2067,
    2072,  2073,  2076,  2077,  2079,  2081,  2085,  2087,  2091,  2093,
    2095,  2099,  2103,  2105,  2107,  2109,  2111,  2113,  2115,  2117,
    2119,  2121,  2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,
    2139,  2141,  2143,  2145,  2147,  2149,  2151,  2153,  2155,  2157,
    2159,  2161,  2163,  2165,  2167,  2169,  2171,  2173,  2175,  2177,
    2179,  2181,  2183,  2185,  2187,  2189,  2191,  2193,  2195,  2197,
    2199,  2201,  2203,  2205,  2207,  2209,  2211,  2213,  2215,  2217,
    2219,  2221,  2223,  2225,  2227,  2229,  2231,  2233,  2235,  2237,
    2239,  2241,  2243,  2245,  2247,  2249,  2251,  2253,  2255,  2257,
    2259,  2261,  2263,  2268,  2270,  2272,  2274,  2276,  2278,  2280,
    2284,  2288,  2290,  2292,  2294,  2297,  2299,  2300,  2301,  2303,
    2305,  2309,  2310,  2312,  2314,  2316,  2318,  2320,  2322,  2324,
    2326,  2328,  2330,  2332,  2334,  2336,  2340,  2343,  2345,  2347,
    2352,  2356,  2361,  2363,  2365,  2369,  2373,  2377,  2381,  2385,
    2389,  2393,  2397,  2401,  2405,  2409,  2413,  2417,  2421,  2425,
    2429,  2433,  2437,  2440,  2443,  2446,  2449,  2453,  2457,  2461,
    2465,  2469,  2473,  2477,  2481,  2487,  2492,  2496,  2500,  2504,
    2506,  2508,  2510,  2512,  2516,  2520,  2524,  2527,  2528,  2530,
    2531,  2533,  2534,  2540,  2544,  2548,  2550,  2552,  2554,  2556,
    2560,  2563,  2565,  2567,  2569,  2571,  2573,  2577,  2579,  2581,
    2583,  2586,  2589,  2594,  2598,  2603,  2606,  2607,  2613,  2617,
    2621,  2623,  2627,  2629,  2632,  2633,  2639,  2643,  2646,  2647,
    2651,  2652,  2657,  2660,  2661,  2665,  2669,  2671,  2672,  2674,
    2676,  2678,  2680,  2684,  2686,  2688,  2690,  2694,  2696,  2698,
    2702,  2706,  2709,  2714,  2717,  2722,  2728,  2734,  2740,  2746,
    2748,  2750,  2752,  2754,  2756,  2758,  2762,  2766,  2771,  2776,
    2780,  2782,  2784,  2786,  2788,  2792,  2794,  2799,  2803,  2805,
    2807,  2809,  2811,  2813,  2817,  2821,  2826,  2831,  2835,  2837,
    2839,  2847,  2857,  2865,  2872,  2881,  2883,  2886,  2891,  2896,
    2898,  2900,  2905,  2907,  2908,  2910,  2913,  2915,  2917,  2919,
    2923,  2927,  2931,  2932,  2934,  2936,  2940,  2944,  2947,  2951,
    2958,  2959,  2961,  2966,  2969,  2970,  2976,  2980,  2984,  2986,
    2993,  2998,  3003,  3006,  3009,  3010,  3016,  3020,  3024,  3026,
    3029,  3030,  3036,  3040,  3044,  3046,  3049,  3052,  3054,  3057,
    3059,  3064,  3068,  3072,  3079,  3083,  3085,  3087,  3089,  3094,
    3099,  3104,  3109,  3114,  3119,  3122,  3125,  3130,  3133,  3136,
    3138,  3142,  3146,  3150,  3151,  3154,  3160,  3167,  3174,  3182,
    3184,  3187,  3189,  3192,  3194,  3199,  3201,  3206,  3210,  3211,
    3213,  3217,  3220,  3224,  3226,  3228,  3229,  3230,  3233,  3236,
    3239,  3244,  3247,  3253,  3257,  3259,  3261,  3262,  3266,  3271,
    3277,  3281,  3283,  3286,  3287,  3292,  3294,  3298,  3301,  3304,
    3307,  3309,  3311,  3313,  3315,  3319,  3324,  3331,  3333,  3342,
    3349,  3351
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     210,     0,    -1,    -1,   211,   212,    -1,   212,   213,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   470,    -1,   124,   199,   200,   201,    -1,   151,   225,
     201,    -1,    -1,   151,   225,   202,   214,   212,   203,    -1,
      -1,   151,   202,   215,   212,   203,    -1,   112,   219,   201,
      -1,   112,   106,   220,   201,    -1,   112,   107,   221,   201,
      -1,   230,   201,    -1,    77,    -1,    98,    -1,   157,    -1,
     158,    -1,   160,    -1,   162,    -1,   161,    -1,   183,    -1,
     184,    -1,   186,    -1,   185,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     194,    -1,   195,    -1,   216,    -1,   134,    -1,   163,    -1,
     127,    -1,   128,    -1,   119,    -1,   118,    -1,   117,    -1,
     116,    -1,   115,    -1,   114,    -1,   107,    -1,    96,    -1,
      92,    -1,    94,    -1,    73,    -1,    90,    -1,    12,    -1,
     113,    -1,   104,    -1,    53,    -1,   165,    -1,   126,    -1,
     151,    -1,    68,    -1,    10,    -1,    11,    -1,   109,    -1,
     112,    -1,   120,    -1,    69,    -1,   132,    -1,    67,    -1,
       7,    -1,     6,    -1,   111,    -1,   133,    -1,    13,    -1,
      87,    -1,     4,    -1,     3,    -1,   108,    -1,    72,    -1,
      71,    -1,   102,    -1,   103,    -1,   105,    -1,    99,    -1,
      27,    -1,   106,    -1,    70,    -1,   100,    -1,   168,    -1,
      91,    -1,    93,    -1,    95,    -1,   101,    -1,    88,    -1,
      89,    -1,    97,    -1,   110,    -1,   217,    -1,   125,    -1,
     219,     9,   222,    -1,   222,    -1,   223,     9,   223,    -1,
     223,    -1,   224,     9,   224,    -1,   224,    -1,   225,    -1,
     154,   225,    -1,   225,    97,   216,    -1,   154,   225,    97,
     216,    -1,   225,    -1,   154,   225,    -1,   225,    97,   216,
      -1,   154,   225,    97,   216,    -1,   225,    -1,   154,   225,
      -1,   225,    97,   216,    -1,   154,   225,    97,   216,    -1,
     216,    -1,   225,   154,   216,    -1,   225,    -1,   151,   154,
     225,    -1,   154,   225,    -1,   226,    -1,   226,   475,    -1,
     226,   475,    -1,   230,     9,   471,    14,   409,    -1,   107,
     471,    14,   409,    -1,   231,   232,    -1,    -1,   233,    -1,
     250,    -1,   257,    -1,   262,    -1,   202,   231,   203,    -1,
      70,   338,   233,   284,   286,    -1,    70,   338,    30,   231,
     285,   287,    73,   201,    -1,    -1,    89,   338,   234,   278,
      -1,    -1,    88,   235,   233,    89,   338,   201,    -1,    -1,
      91,   199,   340,   201,   340,   201,   340,   200,   236,   276,
      -1,    -1,    99,   338,   237,   281,    -1,   103,   201,    -1,
     103,   347,   201,    -1,   105,   201,    -1,   105,   347,   201,
      -1,   108,   201,    -1,   108,   347,   201,    -1,    27,   103,
     201,    -1,   113,   294,   201,    -1,   119,   296,   201,    -1,
      87,   339,   201,    -1,   143,   339,   201,    -1,   121,   199,
     467,   200,   201,    -1,   201,    -1,    81,    -1,    82,    -1,
      -1,    93,   199,   347,    97,   275,   274,   200,   238,   277,
      -1,    -1,    93,   199,   347,    28,    97,   275,   274,   200,
     239,   277,    -1,    95,   199,   280,   200,   279,    -1,    -1,
     109,   242,   110,   199,   402,    79,   200,   202,   231,   203,
     244,   240,   247,    -1,    -1,   109,   242,   168,   241,   245,
      -1,   111,   347,   201,    -1,   104,   216,   201,    -1,   347,
     201,    -1,   341,   201,    -1,   342,   201,    -1,   343,   201,
      -1,   344,   201,    -1,   345,   201,    -1,   108,   344,   201,
      -1,   346,   201,    -1,   372,   201,    -1,   108,   371,   201,
      -1,   216,    30,    -1,    -1,   202,   243,   231,   203,    -1,
     244,   110,   199,   402,    79,   200,   202,   231,   203,    -1,
      -1,    -1,   202,   246,   231,   203,    -1,   168,   245,    -1,
      -1,    35,    -1,    -1,   106,    -1,    -1,   249,   248,   474,
     251,   199,   290,   200,   479,   324,    -1,    -1,   328,   249,
     248,   474,   252,   199,   290,   200,   479,   324,    -1,    -1,
     430,   327,   249,   248,   474,   253,   199,   290,   200,   479,
     324,    -1,    -1,   161,   216,   255,    30,   489,   469,   202,
     297,   203,    -1,    -1,   430,   161,   216,   256,    30,   489,
     469,   202,   297,   203,    -1,    -1,   268,   265,   258,   269,
     270,   202,   300,   203,    -1,    -1,   430,   268,   265,   259,
     269,   270,   202,   300,   203,    -1,    -1,   126,   266,   260,
     271,   202,   300,   203,    -1,    -1,   430,   126,   266,   261,
     271,   202,   300,   203,    -1,    -1,   163,   267,   263,   270,
     202,   300,   203,    -1,    -1,   430,   163,   267,   264,   270,
     202,   300,   203,    -1,   474,    -1,   155,    -1,   474,    -1,
     474,    -1,   125,    -1,   118,   125,    -1,   118,   117,   125,
      -1,   117,   118,   125,    -1,   117,   125,    -1,   127,   402,
      -1,    -1,   128,   272,    -1,    -1,   127,   272,    -1,    -1,
     402,    -1,   272,     9,   402,    -1,   402,    -1,   273,     9,
     402,    -1,   131,   275,    -1,    -1,   442,    -1,    35,   442,
      -1,   132,   199,   456,   200,    -1,   233,    -1,    30,   231,
      92,   201,    -1,   233,    -1,    30,   231,    94,   201,    -1,
     233,    -1,    30,   231,    90,   201,    -1,   233,    -1,    30,
     231,    96,   201,    -1,   216,    14,   409,    -1,   280,     9,
     216,    14,   409,    -1,   202,   282,   203,    -1,   202,   201,
     282,   203,    -1,    30,   282,   100,   201,    -1,    30,   201,
     282,   100,   201,    -1,   282,   101,   347,   283,   231,    -1,
     282,   102,   283,   231,    -1,    -1,    30,    -1,   201,    -1,
     284,    71,   338,   233,    -1,    -1,   285,    71,   338,    30,
     231,    -1,    -1,    72,   233,    -1,    -1,    72,    30,   231,
      -1,    -1,   289,     9,   431,   330,   490,   164,    79,    -1,
     289,     9,   431,   330,   490,    35,   164,    79,    -1,   289,
       9,   431,   330,   490,   164,    -1,   289,   414,    -1,   431,
     330,   490,   164,    79,    -1,   431,   330,   490,    35,   164,
      79,    -1,   431,   330,   490,   164,    -1,    -1,   431,   330,
     490,    79,    -1,   431,   330,   490,    35,    79,    -1,   431,
     330,   490,    35,    79,    14,   347,    -1,   431,   330,   490,
      79,    14,   347,    -1,   289,     9,   431,   330,   490,    79,
      -1,   289,     9,   431,   330,   490,    35,    79,    -1,   289,
       9,   431,   330,   490,    35,    79,    14,   347,    -1,   289,
       9,   431,   330,   490,    79,    14,   347,    -1,   291,     9,
     431,   490,   164,    79,    -1,   291,     9,   431,   490,    35,
     164,    79,    -1,   291,     9,   431,   490,   164,    -1,   291,
     414,    -1,   431,   490,   164,    79,    -1,   431,   490,    35,
     164,    79,    -1,   431,   490,   164,    -1,    -1,   431,   490,
      79,    -1,   431,   490,    35,    79,    -1,   431,   490,    35,
      79,    14,   347,    -1,   431,   490,    79,    14,   347,    -1,
     291,     9,   431,   490,    79,    -1,   291,     9,   431,   490,
      35,    79,    -1,   291,     9,   431,   490,    35,    79,    14,
     347,    -1,   291,     9,   431,   490,    79,    14,   347,    -1,
     293,   414,    -1,    -1,   347,    -1,    35,   442,    -1,   164,
     347,    -1,   293,     9,   347,    -1,   293,     9,   164,   347,
      -1,   293,     9,    35,   442,    -1,   294,     9,   295,    -1,
     295,    -1,    79,    -1,   204,   442,    -1,   204,   202,   347,
     203,    -1,   296,     9,    79,    -1,   296,     9,    79,    14,
     409,    -1,    79,    -1,    79,    14,   409,    -1,   297,   298,
      -1,    -1,   299,   201,    -1,   472,    14,   409,    -1,   300,
     301,    -1,    -1,    -1,   326,   302,   332,   201,    -1,    -1,
     328,   489,   303,   332,   201,    -1,   333,   201,    -1,   334,
     201,    -1,   335,   201,    -1,    -1,   327,   249,   248,   473,
     199,   304,   288,   200,   479,   325,    -1,    -1,   430,   327,
     249,   248,   474,   199,   305,   288,   200,   479,   325,    -1,
     157,   310,   201,    -1,   158,   318,   201,    -1,   160,   320,
     201,    -1,     4,   127,   402,   201,    -1,     4,   128,   402,
     201,    -1,   112,   273,   201,    -1,   112,   273,   202,   306,
     203,    -1,   306,   307,    -1,   306,   308,    -1,    -1,   229,
     150,   216,   165,   273,   201,    -1,   309,    97,   327,   216,
     201,    -1,   309,    97,   328,   201,    -1,   229,   150,   216,
      -1,   216,    -1,   311,    -1,   310,     9,   311,    -1,   312,
     399,   316,   317,    -1,   155,    -1,    29,   313,    -1,   313,
      -1,   133,    -1,   133,   171,   489,   172,    -1,   133,   171,
     489,     9,   489,   172,    -1,   402,    -1,   120,    -1,   161,
     202,   315,   203,    -1,   134,    -1,   408,    -1,   314,     9,
     408,    -1,   314,   413,    -1,    14,   409,    -1,    -1,    55,
     162,    -1,    -1,   319,    -1,   318,     9,   319,    -1,   159,
      -1,   321,    -1,   216,    -1,   123,    -1,   199,   322,   200,
      -1,   199,   322,   200,    49,    -1,   199,   322,   200,    29,
      -1,   199,   322,   200,    46,    -1,   321,    -1,   323,    -1,
     323,    49,    -1,   323,    29,    -1,   323,    46,    -1,   322,
       9,   322,    -1,   322,    33,   322,    -1,   216,    -1,   155,
      -1,   159,    -1,   201,    -1,   202,   231,   203,    -1,   201,
      -1,   202,   231,   203,    -1,   328,    -1,   120,    -1,   328,
      -1,    -1,   329,    -1,   328,   329,    -1,   114,    -1,   115,
      -1,   116,    -1,   119,    -1,   118,    -1,   117,    -1,   181,
      -1,   331,    -1,    -1,   114,    -1,   115,    -1,   116,    -1,
     332,     9,    79,    -1,   332,     9,    79,    14,   409,    -1,
      79,    -1,    79,    14,   409,    -1,   333,     9,   472,    14,
     409,    -1,   107,   472,    14,   409,    -1,   334,     9,   472,
      -1,   118,   107,   472,    -1,   118,   336,   469,    -1,   336,
     469,    14,   489,    -1,   107,   176,   474,    -1,   199,   337,
     200,    -1,    68,   404,   407,    -1,    67,   347,    -1,   391,
      -1,   367,    -1,   199,   347,   200,    -1,   339,     9,   347,
      -1,   347,    -1,   339,    -1,    -1,    27,    -1,    27,   347,
      -1,    27,   347,   131,   347,    -1,   442,    14,   341,    -1,
     132,   199,   456,   200,    14,   341,    -1,    28,   347,    -1,
     442,    14,   344,    -1,   132,   199,   456,   200,    14,   344,
      -1,   348,    -1,   442,    -1,   337,    -1,   446,    -1,   445,
      -1,   132,   199,   456,   200,    14,   347,    -1,   442,    14,
     347,    -1,   442,    14,    35,   442,    -1,   442,    14,    35,
      68,   404,   407,    -1,   442,    26,   347,    -1,   442,    25,
     347,    -1,   442,    24,   347,    -1,   442,    23,   347,    -1,
     442,    22,   347,    -1,   442,    21,   347,    -1,   442,    20,
     347,    -1,   442,    19,   347,    -1,   442,    18,   347,    -1,
     442,    17,   347,    -1,   442,    16,   347,    -1,   442,    15,
     347,    -1,   442,    64,    -1,    64,   442,    -1,   442,    63,
      -1,    63,   442,    -1,   347,    31,   347,    -1,   347,    32,
     347,    -1,   347,    10,   347,    -1,   347,    12,   347,    -1,
     347,    11,   347,    -1,   347,    33,   347,    -1,   347,    35,
     347,    -1,   347,    34,   347,    -1,   347,    48,   347,    -1,
     347,    46,   347,    -1,   347,    47,   347,    -1,   347,    49,
     347,    -1,   347,    50,   347,    -1,   347,    65,   347,    -1,
     347,    51,   347,    -1,   347,    45,   347,    -1,   347,    44,
     347,    -1,    46,   347,    -1,    47,   347,    -1,    52,   347,
      -1,    54,   347,    -1,   347,    37,   347,    -1,   347,    36,
     347,    -1,   347,    39,   347,    -1,   347,    38,   347,    -1,
     347,    40,   347,    -1,   347,    43,   347,    -1,   347,    41,
     347,    -1,   347,    42,   347,    -1,   347,    53,   404,    -1,
     199,   348,   200,    -1,   347,    29,   347,    30,   347,    -1,
     347,    29,    30,   347,    -1,   466,    -1,    62,   347,    -1,
      61,   347,    -1,    60,   347,    -1,    59,   347,    -1,    58,
     347,    -1,    57,   347,    -1,    56,   347,    -1,    69,   405,
      -1,    55,   347,    -1,   411,    -1,   366,    -1,   365,    -1,
     205,   406,   205,    -1,    13,   347,    -1,   369,    -1,   112,
     199,   390,   414,   200,    -1,    -1,    -1,   249,   248,   199,
     351,   290,   200,   479,   349,   202,   231,   203,    -1,    -1,
     328,   249,   248,   199,   352,   290,   200,   479,   349,   202,
     231,   203,    -1,    -1,    79,   354,   359,    -1,    -1,   181,
      79,   355,   359,    -1,    -1,   196,   356,   290,   197,   479,
     359,    -1,    -1,   181,   196,   357,   290,   197,   479,   359,
      -1,    -1,   181,   202,   358,   231,   203,    -1,     8,   347,
      -1,     8,   344,    -1,     8,   202,   231,   203,    -1,    86,
      -1,   468,    -1,   361,     9,   360,   131,   347,    -1,   360,
     131,   347,    -1,   362,     9,   360,   131,   409,    -1,   360,
     131,   409,    -1,   361,   413,    -1,    -1,   362,   413,    -1,
      -1,   175,   199,   363,   200,    -1,   133,   199,   457,   200,
      -1,    66,   457,   206,    -1,   402,   202,   459,   203,    -1,
     402,   202,   461,   203,    -1,   369,    66,   452,   206,    -1,
     370,    66,   452,   206,    -1,   366,    -1,   468,    -1,   445,
      -1,    86,    -1,   199,   348,   200,    -1,   373,   374,    -1,
     442,    14,   371,    -1,   182,    79,   185,   347,    -1,   375,
     386,    -1,   375,   386,   389,    -1,   386,    -1,   386,   389,
      -1,   376,    -1,   375,   376,    -1,   377,    -1,   378,    -1,
     379,    -1,   380,    -1,   381,    -1,   382,    -1,   182,    79,
     185,   347,    -1,   189,    79,    14,   347,    -1,   183,   347,
      -1,   184,    79,   185,   347,   186,   347,   187,   347,    -1,
     184,    79,   185,   347,   186,   347,   187,   347,   188,    79,
      -1,   190,   383,    -1,   384,    -1,   383,     9,   384,    -1,
     347,    -1,   347,   385,    -1,   191,    -1,   192,    -1,   387,
      -1,   388,    -1,   193,   347,    -1,   194,   347,   195,   347,
      -1,   188,    79,   374,    -1,   390,     9,    79,    -1,   390,
       9,    35,    79,    -1,    79,    -1,    35,    79,    -1,   169,
     155,   392,   170,    -1,   394,    50,    -1,   394,   170,   395,
     169,    50,   393,    -1,    -1,   155,    -1,   394,   396,    14,
     397,    -1,    -1,   395,   398,    -1,    -1,   155,    -1,   156,
      -1,   202,   347,   203,    -1,   156,    -1,   202,   347,   203,
      -1,   391,    -1,   400,    -1,   399,    30,   400,    -1,   399,
      47,   400,    -1,   216,    -1,    69,    -1,   106,    -1,   107,
      -1,   108,    -1,    27,    -1,    28,    -1,   109,    -1,   110,
      -1,   168,    -1,   111,    -1,    70,    -1,    71,    -1,    73,
      -1,    72,    -1,    89,    -1,    90,    -1,    88,    -1,    91,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    53,    -1,    97,    -1,    99,    -1,   100,    -1,   101,
      -1,   102,    -1,   103,    -1,   105,    -1,   104,    -1,    87,
      -1,    13,    -1,   125,    -1,   126,    -1,   127,    -1,   128,
      -1,    68,    -1,    67,    -1,   120,    -1,     5,    -1,     7,
      -1,     6,    -1,     4,    -1,     3,    -1,   151,    -1,   112,
      -1,   113,    -1,   122,    -1,   123,    -1,   124,    -1,   119,
      -1,   118,    -1,   117,    -1,   116,    -1,   115,    -1,   114,
      -1,   181,    -1,   121,    -1,   132,    -1,   133,    -1,    10,
      -1,    12,    -1,    11,    -1,   135,    -1,   137,    -1,   136,
      -1,   138,    -1,   139,    -1,   153,    -1,   152,    -1,   180,
      -1,   163,    -1,   166,    -1,   165,    -1,   176,    -1,   178,
      -1,   175,    -1,   228,   199,   292,   200,    -1,   229,    -1,
     155,    -1,   402,    -1,   408,    -1,   119,    -1,   450,    -1,
     199,   348,   200,    -1,   403,   150,   449,    -1,   402,    -1,
     119,    -1,   454,    -1,   199,   200,    -1,   338,    -1,    -1,
      -1,    85,    -1,   463,    -1,   199,   292,   200,    -1,    -1,
      74,    -1,    75,    -1,    76,    -1,    86,    -1,   138,    -1,
     139,    -1,   153,    -1,   135,    -1,   166,    -1,   136,    -1,
     137,    -1,   152,    -1,   180,    -1,   146,    85,   147,    -1,
     146,   147,    -1,   408,    -1,   227,    -1,   133,   199,   412,
     200,    -1,    66,   412,   206,    -1,   175,   199,   364,   200,
      -1,   410,    -1,   368,    -1,   199,   409,   200,    -1,   409,
      31,   409,    -1,   409,    32,   409,    -1,   409,    10,   409,
      -1,   409,    12,   409,    -1,   409,    11,   409,    -1,   409,
      33,   409,    -1,   409,    35,   409,    -1,   409,    34,   409,
      -1,   409,    48,   409,    -1,   409,    46,   409,    -1,   409,
      47,   409,    -1,   409,    49,   409,    -1,   409,    50,   409,
      -1,   409,    51,   409,    -1,   409,    45,   409,    -1,   409,
      44,   409,    -1,   409,    65,   409,    -1,    52,   409,    -1,
      54,   409,    -1,    46,   409,    -1,    47,   409,    -1,   409,
      37,   409,    -1,   409,    36,   409,    -1,   409,    39,   409,
      -1,   409,    38,   409,    -1,   409,    40,   409,    -1,   409,
      43,   409,    -1,   409,    41,   409,    -1,   409,    42,   409,
      -1,   409,    29,   409,    30,   409,    -1,   409,    29,    30,
     409,    -1,   229,   150,   217,    -1,   155,   150,   217,    -1,
     229,   150,   125,    -1,   227,    -1,    78,    -1,   468,    -1,
     408,    -1,   207,   463,   207,    -1,   208,   463,   208,    -1,
     146,   463,   147,    -1,   415,   413,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   415,     9,   409,   131,   409,    -1,
     415,     9,   409,    -1,   409,   131,   409,    -1,   409,    -1,
      74,    -1,    75,    -1,    76,    -1,   146,    85,   147,    -1,
     146,   147,    -1,    74,    -1,    75,    -1,    76,    -1,   216,
      -1,    86,    -1,    86,    48,   418,    -1,   416,    -1,   418,
      -1,   216,    -1,    46,   417,    -1,    47,   417,    -1,   133,
     199,   420,   200,    -1,    66,   420,   206,    -1,   175,   199,
     423,   200,    -1,   421,   413,    -1,    -1,   421,     9,   419,
     131,   419,    -1,   421,     9,   419,    -1,   419,   131,   419,
      -1,   419,    -1,   422,     9,   419,    -1,   419,    -1,   424,
     413,    -1,    -1,   424,     9,   360,   131,   419,    -1,   360,
     131,   419,    -1,   422,   413,    -1,    -1,   199,   425,   200,
      -1,    -1,   427,     9,   216,   426,    -1,   216,   426,    -1,
      -1,   429,   427,   413,    -1,    45,   428,    44,    -1,   430,
      -1,    -1,   129,    -1,   130,    -1,   216,    -1,   155,    -1,
     202,   347,   203,    -1,   433,    -1,   449,    -1,   216,    -1,
     202,   347,   203,    -1,   435,    -1,   449,    -1,    66,   452,
     206,    -1,   202,   347,   203,    -1,   443,   437,    -1,   199,
     337,   200,   437,    -1,   455,   437,    -1,   199,   337,   200,
     437,    -1,   199,   337,   200,   432,   434,    -1,   199,   348,
     200,   432,   434,    -1,   199,   337,   200,   432,   433,    -1,
     199,   348,   200,   432,   433,    -1,   449,    -1,   401,    -1,
     447,    -1,   448,    -1,   438,    -1,   440,    -1,   442,   432,
     434,    -1,   403,   150,   449,    -1,   444,   199,   292,   200,
      -1,   445,   199,   292,   200,    -1,   199,   442,   200,    -1,
     401,    -1,   447,    -1,   448,    -1,   438,    -1,   442,   432,
     433,    -1,   441,    -1,   444,   199,   292,   200,    -1,   199,
     442,   200,    -1,   449,    -1,   438,    -1,   401,    -1,   366,
      -1,   408,    -1,   199,   442,   200,    -1,   199,   348,   200,
      -1,   445,   199,   292,   200,    -1,   444,   199,   292,   200,
      -1,   199,   446,   200,    -1,   350,    -1,   353,    -1,   442,
     432,   436,   475,   199,   292,   200,    -1,   199,   337,   200,
     432,   436,   475,   199,   292,   200,    -1,   403,   150,   218,
     475,   199,   292,   200,    -1,   403,   150,   449,   199,   292,
     200,    -1,   403,   150,   202,   347,   203,   199,   292,   200,
      -1,   450,    -1,   453,   450,    -1,   450,    66,   452,   206,
      -1,   450,   202,   347,   203,    -1,   451,    -1,    79,    -1,
     204,   202,   347,   203,    -1,   347,    -1,    -1,   204,    -1,
     453,   204,    -1,   449,    -1,   439,    -1,   440,    -1,   454,
     432,   434,    -1,   403,   150,   449,    -1,   199,   442,   200,
      -1,    -1,   439,    -1,   441,    -1,   454,   432,   433,    -1,
     199,   442,   200,    -1,   456,     9,    -1,   456,     9,   442,
      -1,   456,     9,   132,   199,   456,   200,    -1,    -1,   442,
      -1,   132,   199,   456,   200,    -1,   458,   413,    -1,    -1,
     458,     9,   347,   131,   347,    -1,   458,     9,   347,    -1,
     347,   131,   347,    -1,   347,    -1,   458,     9,   347,   131,
      35,   442,    -1,   458,     9,    35,   442,    -1,   347,   131,
      35,   442,    -1,    35,   442,    -1,   460,   413,    -1,    -1,
     460,     9,   347,   131,   347,    -1,   460,     9,   347,    -1,
     347,   131,   347,    -1,   347,    -1,   462,   413,    -1,    -1,
     462,     9,   409,   131,   409,    -1,   462,     9,   409,    -1,
     409,   131,   409,    -1,   409,    -1,   463,   464,    -1,   463,
      85,    -1,   464,    -1,    85,   464,    -1,    79,    -1,    79,
      66,   465,   206,    -1,    79,   432,   216,    -1,   148,   347,
     203,    -1,   148,    78,    66,   347,   206,   203,    -1,   149,
     442,   203,    -1,   216,    -1,    80,    -1,    79,    -1,   122,
     199,   467,   200,    -1,   123,   199,   442,   200,    -1,   123,
     199,   348,   200,    -1,   123,   199,   446,   200,    -1,   123,
     199,   445,   200,    -1,   123,   199,   337,   200,    -1,     7,
     347,    -1,     6,   347,    -1,     5,   199,   347,   200,    -1,
       4,   347,    -1,     3,   347,    -1,   442,    -1,   467,     9,
     442,    -1,   403,   150,   217,    -1,   403,   150,   125,    -1,
      -1,    97,   489,    -1,   176,   474,    14,   489,   201,    -1,
     430,   176,   474,    14,   489,   201,    -1,   178,   474,   469,
      14,   489,   201,    -1,   430,   178,   474,   469,    14,   489,
     201,    -1,   218,    -1,   489,   218,    -1,   217,    -1,   489,
     217,    -1,   218,    -1,   218,   171,   481,   172,    -1,   216,
      -1,   216,   171,   481,   172,    -1,   171,   477,   172,    -1,
      -1,   489,    -1,   476,     9,   489,    -1,   476,   413,    -1,
     476,     9,   164,    -1,   477,    -1,   164,    -1,    -1,    -1,
      30,   489,    -1,    97,   489,    -1,    98,   489,    -1,   481,
       9,   482,   216,    -1,   482,   216,    -1,   481,     9,   482,
     216,   480,    -1,   482,   216,   480,    -1,    46,    -1,    47,
      -1,    -1,    86,   131,   489,    -1,    29,    86,   131,   489,
      -1,   229,   150,   216,   131,   489,    -1,   484,     9,   483,
      -1,   483,    -1,   484,   413,    -1,    -1,   175,   199,   485,
     200,    -1,   229,    -1,   216,   150,   488,    -1,   216,   475,
      -1,    29,   489,    -1,    55,   489,    -1,   229,    -1,   133,
      -1,   134,    -1,   486,    -1,   487,   150,   488,    -1,   133,
     171,   489,   172,    -1,   133,   171,   489,     9,   489,   172,
      -1,   155,    -1,   199,   106,   199,   478,   200,    30,   489,
     200,    -1,   199,   489,     9,   476,   413,   200,    -1,   489,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   740,   740,   740,   749,   751,   754,   755,   756,   757,
     758,   759,   760,   763,   765,   765,   767,   767,   769,   770,
     772,   774,   779,   780,   781,   782,   783,   784,   785,   786,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   796,
     797,   798,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   829,
     830,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     840,   841,   842,   843,   844,   845,   846,   847,   848,   849,
     850,   851,   852,   853,   854,   855,   856,   857,   858,   859,
     860,   861,   862,   869,   870,   874,   876,   880,   882,   886,
     888,   892,   893,   894,   895,   900,   901,   902,   903,   908,
     909,   910,   911,   916,   917,   921,   922,   924,   927,   933,
     940,   947,   951,   957,   959,   962,   963,   964,   965,   968,
     969,   973,   978,   978,   984,   984,   991,   990,   996,   996,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1019,  1017,  1026,  1024,  1031,
    1039,  1033,  1043,  1041,  1045,  1046,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1068,  1068,  1073,
    1079,  1083,  1083,  1091,  1092,  1096,  1097,  1101,  1107,  1105,
    1120,  1117,  1133,  1130,  1147,  1146,  1155,  1153,  1165,  1164,
    1183,  1181,  1200,  1199,  1208,  1206,  1218,  1217,  1229,  1227,
    1240,  1241,  1245,  1248,  1251,  1252,  1253,  1256,  1257,  1260,
    1262,  1265,  1266,  1269,  1270,  1273,  1274,  1278,  1279,  1284,
    1285,  1288,  1289,  1290,  1294,  1295,  1299,  1300,  1304,  1305,
    1309,  1310,  1315,  1316,  1321,  1322,  1323,  1324,  1327,  1330,
    1332,  1335,  1336,  1340,  1342,  1345,  1348,  1351,  1352,  1355,
    1356,  1360,  1366,  1372,  1379,  1381,  1386,  1391,  1397,  1401,
    1405,  1409,  1414,  1419,  1424,  1429,  1435,  1444,  1449,  1454,
    1460,  1462,  1466,  1470,  1475,  1479,  1482,  1485,  1489,  1493,
    1497,  1501,  1506,  1514,  1516,  1519,  1520,  1521,  1522,  1524,
    1526,  1531,  1532,  1535,  1536,  1537,  1541,  1542,  1544,  1545,
    1549,  1551,  1554,  1558,  1564,  1566,  1569,  1569,  1573,  1572,
    1576,  1578,  1581,  1584,  1582,  1598,  1594,  1608,  1610,  1612,
    1614,  1616,  1618,  1620,  1624,  1625,  1626,  1629,  1635,  1639,
    1645,  1648,  1653,  1655,  1660,  1665,  1669,  1670,  1674,  1675,
    1677,  1679,  1685,  1686,  1688,  1692,  1693,  1698,  1702,  1703,
    1707,  1708,  1712,  1714,  1720,  1725,  1726,  1728,  1732,  1733,
    1734,  1735,  1739,  1740,  1741,  1742,  1743,  1744,  1746,  1751,
    1754,  1755,  1759,  1760,  1764,  1765,  1768,  1769,  1772,  1773,
    1776,  1777,  1781,  1782,  1783,  1784,  1785,  1786,  1787,  1791,
    1792,  1795,  1796,  1797,  1800,  1802,  1804,  1805,  1808,  1810,
    1814,  1816,  1820,  1824,  1828,  1833,  1834,  1836,  1837,  1838,
    1841,  1845,  1846,  1850,  1851,  1855,  1856,  1857,  1861,  1865,
    1870,  1874,  1878,  1883,  1884,  1885,  1886,  1887,  1891,  1893,
    1894,  1895,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,
    1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,
    1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,
    1936,  1937,  1938,  1939,  1940,  1942,  1943,  1945,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1969,  1973,  1978,
    1977,  1992,  1990,  2007,  2007,  2023,  2022,  2040,  2040,  2056,
    2055,  2074,  2073,  2094,  2095,  2096,  2101,  2103,  2107,  2111,
    2117,  2121,  2127,  2129,  2133,  2135,  2139,  2143,  2144,  2148,
    2155,  2162,  2164,  2169,  2170,  2171,  2172,  2174,  2178,  2182,
    2186,  2190,  2192,  2194,  2196,  2201,  2202,  2207,  2208,  2209,
    2210,  2211,  2212,  2216,  2220,  2224,  2228,  2233,  2238,  2242,
    2243,  2247,  2248,  2252,  2253,  2257,  2258,  2262,  2266,  2270,
    2274,  2275,  2276,  2277,  2281,  2287,  2296,  2309,  2310,  2313,
    2316,  2319,  2320,  2323,  2327,  2330,  2333,  2340,  2341,  2345,
    2346,  2348,  2352,  2353,  2354,  2355,  2356,  2357,  2358,  2359,
    2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,
    2370,  2371,  2372,  2373,  2374,  2375,  2376,  2377,  2378,  2379,
    2380,  2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,  2389,
    2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,
    2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,
    2410,  2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,  2419,
    2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,  2429,
    2430,  2431,  2435,  2440,  2441,  2444,  2445,  2446,  2447,  2449,
    2451,  2456,  2457,  2458,  2462,  2463,  2464,  2468,  2469,  2470,
    2473,  2475,  2479,  2480,  2481,  2482,  2484,  2485,  2486,  2487,
    2488,  2489,  2490,  2491,  2492,  2493,  2496,  2501,  2502,  2503,
    2505,  2506,  2508,  2509,  2510,  2511,  2513,  2515,  2517,  2519,
    2521,  2522,  2523,  2524,  2525,  2526,  2527,  2528,  2529,  2530,
    2531,  2532,  2533,  2534,  2535,  2536,  2537,  2539,  2541,  2543,
    2545,  2546,  2549,  2550,  2554,  2556,  2560,  2563,  2566,  2572,
    2573,  2574,  2575,  2576,  2577,  2578,  2583,  2585,  2589,  2590,
    2593,  2594,  2598,  2601,  2603,  2605,  2609,  2610,  2611,  2612,
    2615,  2619,  2620,  2621,  2622,  2626,  2628,  2635,  2636,  2637,
    2638,  2639,  2640,  2642,  2643,  2648,  2650,  2653,  2656,  2658,
    2660,  2663,  2665,  2669,  2671,  2674,  2677,  2683,  2685,  2688,
    2689,  2694,  2697,  2701,  2701,  2706,  2709,  2710,  2714,  2715,
    2719,  2720,  2721,  2725,  2726,  2730,  2731,  2735,  2736,  2740,
    2741,  2745,  2746,  2751,  2753,  2758,  2769,  2783,  2795,  2810,
    2811,  2812,  2813,  2814,  2815,  2816,  2826,  2829,  2831,  2833,
    2837,  2838,  2839,  2840,  2841,  2852,  2853,  2855,  2859,  2860,
    2861,  2862,  2863,  2864,  2865,  2866,  2868,  2873,  2877,  2878,
    2882,  2885,  2892,  2896,  2900,  2907,  2908,  2913,  2915,  2916,
    2919,  2920,  2923,  2924,  2928,  2929,  2933,  2934,  2935,  2936,
    2947,  2950,  2953,  2954,  2955,  2956,  2967,  2971,  2972,  2973,
    2975,  2976,  2977,  2981,  2983,  2986,  2988,  2989,  2990,  2991,
    2994,  2996,  2997,  3001,  3003,  3006,  3008,  3009,  3010,  3014,
    3016,  3019,  3022,  3024,  3026,  3030,  3031,  3033,  3034,  3040,
    3041,  3043,  3053,  3055,  3057,  3060,  3061,  3062,  3066,  3067,
    3068,  3069,  3070,  3071,  3072,  3073,  3074,  3075,  3076,  3080,
    3081,  3085,  3087,  3095,  3097,  3101,  3105,  3110,  3114,  3122,
    3123,  3127,  3128,  3134,  3135,  3144,  3145,  3153,  3156,  3160,
    3163,  3168,  3173,  3175,  3176,  3177,  3181,  3182,  3186,  3187,
    3190,  3193,  3195,  3199,  3205,  3206,  3207,  3211,  3215,  3225,
    3233,  3235,  3239,  3241,  3246,  3252,  3255,  3260,  3268,  3271,
    3274,  3275,  3278,  3281,  3282,  3287,  3290,  3294,  3298,  3304,
    3314,  3315
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
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE",
  "T_FINAL", "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET",
  "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS",
  "T_IMPLEMENTS", "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CALLABLE", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_LABEL",
  "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_FROM", "T_WHERE", "T_JOIN",
  "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET", "T_ORDERBY",
  "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP", "T_BY",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident_no_semireserved", "ident_for_class_const", "ident",
  "use_declarations", "use_fn_declarations", "use_const_declarations",
  "use_declaration", "use_fn_declaration", "use_const_declaration",
  "namespace_name", "namespace_string_base", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "try_statement_list", "$@12", "additional_catches",
  "finally_statement_list", "$@13", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@14", "$@15", "$@16",
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
  "xhp_nullable_attribute_decl_type", "xhp_attribute_decl_type",
  "non_empty_xhp_attribute_enum", "xhp_attribute_enum",
  "xhp_attribute_default", "xhp_attribute_is_required",
  "xhp_category_stmt", "xhp_category_decl", "xhp_children_stmt",
  "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "function_body", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "class_abstract_constant_declaration", "class_type_constant_declaration",
  "class_type_constant", "expr_with_parens", "parenthesis_expr",
  "expr_list", "for_expr", "yield_expr", "yield_assign_expr",
  "yield_list_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "expr", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@29", "$@30", "lambda_expression", "$@31",
  "$@32", "$@33", "$@34", "$@35", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "query_expr", "query_assign_expr",
  "query_head", "query_body", "query_body_clauses", "query_body_clause",
  "from_clause", "let_clause", "where_clause", "join_clause",
  "join_into_clause", "orderby_clause", "orderings", "ordering",
  "ordering_direction", "select_or_group_clause", "select_clause",
  "group_clause", "query_continuation", "lexical_var_list", "xhp_tag",
  "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_string_expr_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@36", "non_empty_user_attributes",
  "optional_user_attributes", "object_operator",
  "object_property_name_no_variables", "object_property_name",
  "object_method_name_no_variables", "object_method_name", "array_access",
  "dimmable_variable_access", "dimmable_variable_no_calls_access",
  "object_property_access_on_expr",
  "object_property_access_on_expr_no_variables", "variable",
  "dimmable_variable", "callable_variable",
  "lambda_or_closure_with_parens", "lambda_or_closure",
  "object_method_call", "class_method_call", "variable_no_objects",
  "reference_variable", "compound_variable", "dim_offset",
  "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_constname_with_type", "hh_name_with_typevar",
  "hh_name_no_semireserved_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_func_type_list",
  "hh_opt_return_type", "hh_constraint", "hh_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_access_type_start", "hh_access_type", "hh_type",
  "hh_type_opt", 0
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,    40,
      41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   209,   211,   210,   212,   212,   213,   213,   213,   213,
     213,   213,   213,   213,   214,   213,   215,   213,   213,   213,
     213,   213,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   218,   218,   219,   219,   220,   220,   221,
     221,   222,   222,   222,   222,   223,   223,   223,   223,   224,
     224,   224,   224,   225,   225,   226,   226,   226,   227,   228,
     229,   230,   230,   231,   231,   232,   232,   232,   232,   233,
     233,   233,   234,   233,   235,   233,   236,   233,   237,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   238,   233,   239,   233,   233,
     240,   233,   241,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   243,   242,   244,
     244,   246,   245,   247,   247,   248,   248,   249,   251,   250,
     252,   250,   253,   250,   255,   254,   256,   254,   258,   257,
     259,   257,   260,   257,   261,   257,   263,   262,   264,   262,
     265,   265,   266,   267,   268,   268,   268,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   274,
     274,   275,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   280,   280,   281,   281,   281,   281,   282,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   287,
     287,   288,   288,   288,   288,   288,   288,   288,   288,   289,
     289,   289,   289,   289,   289,   289,   289,   290,   290,   290,
     290,   290,   290,   290,   290,   291,   291,   291,   291,   291,
     291,   291,   291,   292,   292,   293,   293,   293,   293,   293,
     293,   294,   294,   295,   295,   295,   296,   296,   296,   296,
     297,   297,   298,   299,   300,   300,   302,   301,   303,   301,
     301,   301,   301,   304,   301,   305,   301,   301,   301,   301,
     301,   301,   301,   301,   306,   306,   306,   307,   308,   308,
     309,   309,   310,   310,   311,   311,   312,   312,   313,   313,
     313,   313,   313,   313,   313,   314,   314,   315,   316,   316,
     317,   317,   318,   318,   319,   320,   320,   320,   321,   321,
     321,   321,   322,   322,   322,   322,   322,   322,   322,   323,
     323,   323,   324,   324,   325,   325,   326,   326,   327,   327,
     328,   328,   329,   329,   329,   329,   329,   329,   329,   330,
     330,   331,   331,   331,   332,   332,   332,   332,   333,   333,
     334,   334,   335,   335,   336,   337,   337,   337,   337,   337,
     338,   339,   339,   340,   340,   341,   341,   341,   342,   343,
     344,   345,   346,   347,   347,   347,   347,   347,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   349,   349,   351,
     350,   352,   350,   354,   353,   355,   353,   356,   353,   357,
     353,   358,   353,   359,   359,   359,   360,   360,   361,   361,
     362,   362,   363,   363,   364,   364,   365,   366,   366,   367,
     368,   369,   369,   370,   370,   370,   370,   370,   371,   372,
     373,   374,   374,   374,   374,   375,   375,   376,   376,   376,
     376,   376,   376,   377,   378,   379,   380,   381,   382,   383,
     383,   384,   384,   385,   385,   386,   386,   387,   388,   389,
     390,   390,   390,   390,   391,   392,   392,   393,   393,   394,
     394,   395,   395,   396,   397,   397,   398,   398,   398,   399,
     399,   399,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   401,   402,   402,   403,   403,   403,   403,   403,
     403,   404,   404,   404,   405,   405,   405,   406,   406,   406,
     407,   407,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   410,   410,   410,   411,
     411,   411,   411,   411,   411,   411,   412,   412,   413,   413,
     414,   414,   415,   415,   415,   415,   416,   416,   416,   416,
     416,   417,   417,   417,   417,   418,   418,   419,   419,   419,
     419,   419,   419,   419,   419,   420,   420,   421,   421,   421,
     421,   422,   422,   423,   423,   424,   424,   425,   425,   426,
     426,   427,   427,   429,   428,   430,   431,   431,   432,   432,
     433,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     443,   443,   443,   443,   443,   443,   443,   443,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   445,   446,   446,
     447,   447,   448,   448,   448,   449,   449,   450,   450,   450,
     451,   451,   452,   452,   453,   453,   454,   454,   454,   454,
     454,   454,   455,   455,   455,   455,   455,   456,   456,   456,
     456,   456,   456,   457,   457,   458,   458,   458,   458,   458,
     458,   458,   458,   459,   459,   460,   460,   460,   460,   461,
     461,   462,   462,   462,   462,   463,   463,   463,   463,   464,
     464,   464,   464,   464,   464,   465,   465,   465,   466,   466,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   467,
     467,   468,   468,   469,   469,   470,   470,   470,   470,   471,
     471,   472,   472,   473,   473,   474,   474,   475,   475,   476,
     476,   477,   478,   478,   478,   478,   479,   479,   480,   480,
     481,   481,   481,   481,   482,   482,   482,   483,   483,   483,
     484,   484,   485,   485,   486,   487,   488,   488,   489,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     490,   490
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     1,     2,     3,     4,     1,     2,     3,     4,     1,
       2,     3,     4,     1,     3,     1,     3,     2,     1,     2,
       2,     5,     4,     2,     0,     1,     1,     1,     1,     3,
       5,     8,     0,     4,     0,     6,     0,    10,     0,     4,
       2,     3,     2,     3,     2,     3,     3,     3,     3,     3,
       3,     5,     1,     1,     1,     0,     9,     0,    10,     5,
       0,    13,     0,     5,     3,     3,     2,     2,     2,     2,
       2,     2,     3,     2,     2,     3,     2,     0,     4,     9,
       0,     0,     4,     2,     0,     1,     0,     1,     0,     9,
       0,    10,     0,    11,     0,     9,     0,    10,     0,     8,
       0,     9,     0,     7,     0,     8,     0,     7,     0,     8,
       1,     1,     1,     1,     1,     2,     3,     3,     2,     2,
       0,     2,     0,     2,     0,     1,     3,     1,     3,     2,
       0,     1,     2,     4,     1,     4,     1,     4,     1,     4,
       1,     4,     3,     5,     3,     4,     4,     5,     5,     4,
       0,     1,     1,     4,     0,     5,     0,     2,     0,     3,
       0,     7,     8,     6,     2,     5,     6,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     6,     5,     5,
       6,     8,     7,     2,     0,     1,     2,     2,     3,     4,
       4,     3,     1,     1,     2,     4,     3,     5,     1,     3,
       2,     0,     2,     3,     2,     0,     0,     4,     0,     5,
       2,     2,     2,     0,    10,     0,    11,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     2,     1,     1,     4,
       6,     1,     1,     4,     1,     1,     3,     2,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     3,     4,     3,     3,     3,     2,     1,     1,
       3,     3,     1,     1,     0,     1,     2,     4,     3,     6,
       2,     3,     6,     1,     1,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      11,     0,    12,     0,     3,     0,     4,     0,     6,     0,
       7,     0,     5,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     1,     1,     1,     1,     3,     2,     3,
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
       1,     1,     4,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     4,
       3,     4,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       2,     2,     4,     3,     4,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     4,     2,     4,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     1,     1,     3,     1,     4,     3,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     1,     3,
       3,     3,     0,     1,     1,     3,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     6,     7,     1,
       2,     1,     2,     1,     4,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     2,     2,     2,
       4,     2,     5,     3,     1,     1,     0,     3,     4,     5,
       3,     1,     2,     0,     4,     1,     3,     2,     2,     2,
       1,     1,     1,     1,     3,     4,     6,     1,     8,     6,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   435,     0,   823,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   914,     0,
     902,   706,     0,   712,   713,   714,    22,   770,   890,   163,
     164,   715,     0,   144,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   197,     0,     0,     0,     0,     0,     0,
     402,   403,   404,   407,   406,   405,     0,     0,     0,     0,
     224,     0,     0,     0,   719,   721,   722,   716,   717,     0,
       0,     0,   723,   718,     0,   694,    24,    25,    26,    28,
      27,     0,   720,     0,     0,     0,     0,   724,   408,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   527,     0,   162,   134,   894,   707,     0,     0,
       4,   123,   125,   128,   769,     0,   693,     0,     6,   196,
       7,     9,     8,    10,     0,     0,   400,   445,     0,     0,
       0,     0,     0,     0,     0,   443,   878,   879,   513,   512,
     429,   516,     0,     0,   428,   850,   695,     0,   772,   511,
     399,   853,   854,   865,   444,     0,     0,   447,   446,   851,
     852,   849,   885,   889,     0,   501,   771,    11,   407,   406,
     405,     0,     0,    28,   123,   196,     0,   958,   444,   957,
       0,   955,   954,   515,     0,   436,   440,     0,     0,   485,
     486,   487,   488,   510,   508,   507,   506,   505,   504,   503,
     502,   890,   715,   697,     0,     0,   978,   871,   695,     0,
     696,   467,     0,   465,     0,   918,     0,   779,   427,   702,
       0,   978,   701,     0,   711,   696,   897,   898,   904,   896,
     703,     0,     0,   705,   509,     0,     0,     0,     0,   432,
       0,   142,   434,     0,     0,   148,   150,     0,     0,   152,
       0,    82,    81,    76,    75,    67,    68,    59,    79,    90,
       0,    62,     0,    74,    66,    72,    92,    85,    84,    57,
      80,    99,   100,    58,    95,    55,    96,    56,    97,    54,
     101,    89,    93,    98,    86,    87,    61,    88,    91,    53,
      83,    69,   102,    77,    70,    60,    52,    51,    50,    49,
      48,    47,    71,   104,    64,    45,    46,    73,  1011,  1012,
      65,  1017,    44,    63,    94,     0,     0,   123,   103,   969,
    1010,     0,  1013,     0,     0,     0,   154,     0,     0,     0,
       0,   187,     0,     0,     0,     0,     0,     0,   106,   111,
     313,     0,     0,   312,     0,   228,     0,   225,   318,     0,
       0,     0,     0,     0,   975,   212,   222,   910,   914,     0,
     939,     0,   726,     0,     0,     0,   937,     0,    16,     0,
     127,   204,   216,   223,   600,   543,     0,   963,   525,   529,
     531,   827,   445,     0,   443,   444,   446,     0,     0,   708,
       0,   709,     0,     0,     0,   186,     0,     0,   130,   304,
       0,    21,   195,     0,   221,   208,   220,   405,   408,   196,
     401,   177,   178,   179,   180,   181,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   902,     0,   176,   893,   893,   184,   924,     0,
       0,     0,     0,     0,     0,     0,     0,   398,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   466,   464,   828,   829,     0,   893,     0,   841,   304,
     304,   893,     0,   895,   886,   910,     0,   196,     0,     0,
     156,     0,   825,   820,   779,     0,   445,   443,     0,   922,
       0,   548,   778,   913,   445,   443,   444,   130,     0,   304,
     426,     0,   843,   704,     0,   134,   264,     0,   524,     0,
     159,     0,     0,   433,     0,     0,     0,     0,     0,   151,
     175,   153,  1011,  1012,  1008,  1009,     0,  1003,     0,     0,
       0,     0,    78,    43,    65,    42,   970,     0,   182,   155,
     185,     0,     0,     0,     0,     0,     0,     0,   558,     0,
     565,   567,   568,   569,   570,   571,   572,   563,   585,   586,
     134,     0,   172,   174,     0,     0,   108,   115,     0,     0,
     110,   119,   112,     0,    18,     0,     0,   314,     0,   157,
     227,   226,     0,     0,   158,   959,     0,     0,   445,   443,
     444,   447,   446,     0,   996,   234,     0,   911,     0,     0,
     160,     0,     0,   725,   938,   770,     0,     0,   936,   775,
     935,   126,     5,    13,    14,     0,   232,     0,     0,   536,
       0,     0,     0,   779,     0,     0,   698,   537,     0,     0,
       0,     0,   827,   134,     0,   781,   826,  1021,   425,   498,
     859,   877,   139,   133,   135,   136,   137,   138,   399,     0,
     514,   773,   774,   124,   779,     0,   979,     0,     0,     0,
     781,   305,     0,   519,   198,   230,     0,   470,   472,   471,
       0,     0,   468,   469,   473,   475,   474,   490,   489,   492,
     491,   493,   495,   496,   494,   484,   483,   477,   478,   476,
     479,   480,   482,   497,   481,   892,     0,     0,   928,     0,
     779,   962,     0,   961,   978,   856,   885,   214,   206,   218,
       0,   963,   210,   196,   435,     0,   438,   441,   449,   559,
     463,   462,   461,   460,   459,   458,   457,   456,   455,   454,
     453,   452,   831,     0,   830,   833,   855,   837,   978,   834,
       0,     0,     0,     0,     0,     0,     0,     0,   956,   437,
     818,   822,   778,   824,     0,   699,     0,   917,     0,   916,
       0,   699,   901,   900,     0,     0,   830,   833,   899,   834,
     430,   266,   268,   134,   534,   533,   431,     0,   134,   248,
     143,   434,     0,     0,     0,     0,     0,   260,   260,   149,
       0,     0,     0,     0,  1001,   779,     0,   985,     0,     0,
       0,     0,     0,   777,     0,   694,     0,     0,   128,   728,
     693,   733,     0,   727,   132,   732,   978,  1014,     0,     0,
     575,     0,     0,   581,   578,   579,   587,     0,   566,   561,
       0,   564,     0,     0,     0,   116,    19,     0,     0,   120,
      20,     0,     0,     0,   105,   113,     0,   311,   319,   316,
       0,     0,   948,   953,   950,   949,   952,   951,    12,   994,
     995,     0,     0,     0,     0,   910,   907,     0,   547,   947,
     946,   945,     0,   941,     0,   942,   944,     0,     5,     0,
       0,     0,   594,   595,   603,   602,     0,   443,     0,   778,
     542,   546,     0,     0,   964,     0,   526,     0,     0,   986,
     827,   290,  1020,     0,     0,   842,     0,   891,   778,   981,
     977,   306,   307,   692,   780,   303,     0,   827,     0,     0,
     232,   521,   200,   500,     0,   551,   552,     0,   549,   778,
     923,     0,     0,   304,   234,     0,   232,     0,     0,   230,
       0,   902,   450,     0,     0,   839,   840,   857,   858,   887,
     888,     0,     0,     0,   806,   786,   787,   788,   795,     0,
       0,     0,   799,   797,   798,   812,   779,     0,   820,   921,
     920,     0,     0,   844,   710,     0,   270,     0,     0,   140,
       0,     0,     0,     0,     0,     0,     0,   240,   241,   252,
       0,   134,   250,   169,   260,     0,   260,     0,     0,  1015,
       0,     0,     0,   778,  1002,  1004,   984,   779,   983,     0,
     779,   754,   755,   752,   753,   785,     0,   779,   777,     0,
     545,     0,     0,   930,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1007,   560,     0,     0,     0,   583,   584,   582,     0,
       0,   562,     0,   188,     0,   191,   173,     0,   107,   117,
       0,   109,   121,   114,   315,     0,   960,   161,   996,   976,
     991,   233,   235,   325,     0,     0,   908,     0,   940,     0,
      17,     0,   963,   231,   325,     0,     0,   699,   539,     0,
     962,   961,   700,   965,     0,   986,   532,     0,     0,  1021,
       0,   295,   293,   833,   845,   978,   833,   846,   980,     0,
       0,   308,   131,     0,   827,   229,     0,   827,     0,   499,
     927,   926,     0,   304,     0,     0,     0,     0,     0,     0,
     232,   202,   711,   832,   304,     0,   791,   792,   793,   794,
     800,   801,   810,     0,   779,     0,   806,     0,   790,   814,
     778,   817,   819,   821,     0,   915,   832,     0,     0,     0,
       0,   267,   535,   145,     0,   434,   240,   242,   910,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   254,     0,
       0,   997,     0,  1000,   778,     0,     0,     0,   730,   778,
     776,     0,   767,     0,   779,     0,   734,   768,   766,   934,
       0,   779,   737,   739,   738,     0,     0,   735,   736,   740,
     742,   741,   757,   756,   759,   758,   760,   762,   763,   761,
     750,   749,   744,   745,   743,   746,   747,   748,   751,  1006,
     573,     0,   574,   580,   588,   589,     0,   134,   118,   122,
     317,     0,     0,     0,   993,     0,   399,   912,   910,   439,
     442,   448,     0,    15,     0,   399,   606,     0,     0,   608,
     601,   604,     0,   599,     0,   967,     0,   987,   528,     0,
     296,     0,     0,   291,     0,   310,   309,   986,     0,   325,
       0,   827,     0,   304,     0,   883,   325,   963,   325,   966,
       0,     0,     0,   451,     0,     0,   803,   778,   805,   796,
       0,   789,     0,     0,   779,   811,   919,     0,   134,     0,
     263,   249,     0,     0,     0,   239,   165,   253,     0,     0,
     256,     0,   261,   262,   134,   255,  1016,   998,     0,   982,
       0,  1019,   784,   783,   729,     0,   778,   544,   731,     0,
     550,   778,   929,   765,     0,     0,     0,     0,   990,   988,
     989,   236,     0,     0,     0,   406,   397,     0,     0,     0,
     213,   324,   326,     0,   396,     0,     0,     0,   963,   399,
       0,   943,   321,   217,   597,     0,     0,   538,   530,     0,
     299,   289,     0,   292,   298,   304,   518,   986,   399,   986,
       0,   925,     0,   882,   399,     0,   399,   968,   325,   827,
     880,   809,   808,   802,     0,   804,   778,   813,   134,   269,
     141,   146,   167,   243,     0,   251,   257,   134,   259,   999,
       0,     0,   541,     0,   933,   932,   764,     0,   134,   192,
     992,     0,     0,     0,   971,     0,     0,     0,   237,     0,
     963,     0,   362,   358,   364,   694,    28,     0,   352,     0,
     357,   361,   374,     0,   372,   377,     0,   376,     0,   375,
       0,   196,   328,     0,   330,     0,   331,   332,     0,     0,
     909,     0,   598,   596,   607,   605,   300,     0,     0,   287,
     297,     0,     0,     0,     0,   209,   518,   986,   884,   215,
     321,   219,   399,     0,     0,   816,     0,   265,     0,     0,
     134,   246,   166,   258,  1018,   782,     0,     0,     0,     0,
       0,     0,   424,     0,   972,     0,   342,   346,   421,   422,
     356,     0,     0,     0,   337,   658,   657,   654,   656,   655,
     675,   677,   676,   646,   617,   618,   636,   652,   651,   613,
     623,   624,   626,   625,   645,   629,   627,   628,   630,   631,
     632,   633,   634,   635,   637,   638,   639,   640,   641,   642,
     644,   643,   614,   615,   616,   619,   620,   622,   660,   661,
     670,   669,   668,   667,   666,   665,   653,   672,   662,   663,
     664,   647,   648,   649,   650,   673,   674,   678,   680,   679,
     681,   682,   659,   684,   683,   686,   688,   687,   621,   691,
     689,   690,   685,   671,   612,   369,   609,     0,   338,   390,
     391,   389,   382,     0,   383,   339,   416,     0,     0,     0,
       0,   420,     0,   196,   205,   320,     0,     0,     0,   288,
     302,   881,     0,   134,   392,   134,   199,     0,     0,     0,
     211,   986,   807,     0,   134,   244,   147,   168,     0,   540,
     931,   576,   190,   340,   341,   419,   238,     0,     0,   779,
       0,   365,   353,     0,     0,     0,   371,   373,     0,     0,
     378,   385,   386,   384,     0,     0,   327,   973,     0,     0,
       0,   423,     0,   322,     0,   301,     0,   592,   781,     0,
       0,   134,   201,   207,     0,   815,     0,     0,     0,   170,
     343,   123,     0,   344,   345,     0,     0,   359,   778,   367,
     363,   368,   610,   611,     0,   354,   387,   388,   380,   381,
     379,   417,   414,   996,   333,   329,   418,     0,   323,   593,
     780,     0,   520,   393,     0,   203,     0,   247,   577,     0,
     194,     0,   399,     0,   366,   370,     0,     0,   827,   335,
       0,   590,   517,   522,   245,     0,     0,   171,   350,     0,
     398,   360,   415,   974,     0,   781,   410,   827,   591,     0,
     193,     0,     0,   349,   986,   827,   274,   411,   412,   413,
    1021,   409,     0,     0,     0,   348,     0,   410,     0,   986,
       0,   347,   394,   134,   334,  1021,     0,   279,   277,     0,
     134,     0,     0,   280,     0,     0,   275,   336,     0,   395,
       0,   283,   273,     0,   276,   282,   189,   284,     0,     0,
     271,   281,     0,   272,   286,   285
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   898,   632,   184,  1464,   329,
     347,   585,   589,   348,   586,   590,   122,   123,   124,   125,
     126,   127,   397,   663,   664,   532,   250,  1528,   538,  1444,
    1529,  1770,   854,   342,   580,  1729,  1086,  1267,  1787,   413,
     185,   665,   938,  1148,  1322,   131,   635,   955,   666,   685,
     959,   615,   954,   667,   636,   956,   415,   365,   382,   134,
     940,   901,   884,  1101,  1467,  1200,  1007,  1676,  1532,   800,
    1013,   537,   809,  1015,  1354,   792,   996,   999,  1189,  1794,
    1795,   654,   655,   679,   680,   352,   353,   359,  1501,  1655,
    1656,  1276,  1391,  1490,  1649,  1778,  1797,  1687,  1733,  1734,
    1735,  1477,  1478,  1479,  1480,  1689,  1690,  1696,  1745,  1483,
    1484,  1488,  1642,  1643,  1644,  1666,  1824,  1392,  1393,   186,
     136,  1810,  1811,  1647,  1395,  1396,  1397,  1398,   137,   243,
     533,   534,   138,   139,   140,   141,   142,   143,   144,   145,
    1513,   146,   937,  1147,   147,   247,   651,   391,   652,   653,
     528,   642,   643,  1224,   644,  1225,   148,   149,   150,   831,
     151,   152,   339,   153,   340,   568,   569,   570,   571,   572,
     573,   574,   575,   576,   844,   845,  1078,   577,   578,   579,
     851,  1718,   154,   637,  1503,   638,  1115,   906,  1293,  1290,
    1635,  1636,   155,   156,   157,   234,   244,   400,   520,   158,
    1035,   835,   159,  1036,   929,   921,  1037,   983,  1170,   984,
    1172,  1173,  1174,   986,  1333,  1334,   987,   771,   504,   197,
     198,   668,   657,   485,  1133,  1134,   757,   758,   925,   161,
     236,   162,   163,   188,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   716,   174,   240,   241,   618,   226,   227,
     719,   720,  1230,  1231,   375,   376,   892,   175,   606,   176,
     650,   177,   331,  1657,  1708,   366,   408,   674,   675,  1029,
    1128,  1274,   881,   882,   814,   815,   816,   332,   333,   837,
    1466,   923
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1499
static const yytype_int16 yypact[] =
{
   -1499,   198, -1499, -1499,  5413, 13241, 13241,     5, 13241, 13241,
   13241, 11181, 13241, -1499, 13241, 13241, 13241, 13241, 13241, 13241,
   13241, 13241, 13241, 13241, 13241, 13241, 17450, 17450, 11387, 13241,
   17576,    95,   186, -1499, -1499, -1499, -1499, -1499,   193, -1499,
   -1499,   173, 13241, -1499,   186,   192,   216,   256, -1499,   186,
   11593, 18443, 11799, -1499, 14470, 10151,   203, 13241,  2005,   166,
   -1499, -1499, -1499,   289,   259,   118,   260,   291,   355,   365,
   -1499, 18443,   372,   377, -1499, -1499, -1499, -1499, -1499, 13241,
     545,  3863, -1499, -1499, 18443, -1499, -1499, -1499, -1499, 18443,
   -1499, 18443, -1499,   441,   410, 18443, 18443, -1499,   108, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, 13241, -1499, -1499,   417,   499,   522,   522,
   -1499,   592,   475,   526, -1499,   437, -1499,    81, -1499,   597,
   -1499, -1499, -1499, -1499, 18284,   644, -1499, -1499,   444,   464,
     477,   490,   504,   517, 15701, -1499, -1499, -1499, -1499,    99,
   -1499,   609,   689,   548, -1499,   147,   577,   632,   258, -1499,
     669,   162, -1499, -1499,  2581,    63,   590,   136, -1499,   130,
     177,   604,   200, -1499,   309, -1499,   758, -1499, -1499, -1499,
     681,   637,   685, -1499, -1499,   597,   644, 18700,  3234, 18700,
   13241, 18700, 18700, 10340,   654, 17888, 10340,   809, 18443,   792,
     792,   329,   792,   792,   792,   792,   792,   792,   792,   792,
     792, -1499, -1499, -1499,   433, 13241,   696, -1499, -1499,   712,
     667,   559,   676,   559, 17450, 17932,   662,   867, -1499,   681,
   13241,   696,   732,   733,   693, -1499,   184, -1499, -1499, -1499,
     559,    63, 12005, -1499, -1499, 13241,  4717,   891,    83, 18700,
    9739, -1499, 13241, 13241, 18443, -1499, -1499, 15745,   700, -1499,
   15789, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   16217, -1499, 16217, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,    88,    77,
     685, -1499, -1499, -1499, -1499,   703,  4102,    90, -1499, -1499,
     756,   893, -1499,   759, 15248,   829, -1499,   709, 15833,   710,
     697, -1499,    53, 15877, 18338, 18386, 18443,    87, -1499,    84,
   -1499, 16925,    91, -1499,   787, -1499,   788, -1499,   901,    96,
   17450, 17450, 13241,   716,   747, -1499, -1499, 17056, 11387,    98,
     528,   532, -1499, 13447, 17450,   552, -1499, 18443, -1499,    18,
     475, -1499, -1499, -1499, -1499, 17702,   905,   823, -1499, -1499,
   -1499,    61,   721, 18700,   722,  2243,   728,  5619, 13241,   535,
     725,   571,   535,   467,   456, -1499, 18443, 16217,   734, 10357,
   14470, -1499, -1499, 17499, -1499, -1499, -1499, -1499, -1499,   597,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, 13241, 13241, 13241,
   12211, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241,
   13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241,
   13241, 13241, 17576, 13241, -1499, 13241, 13241, -1499, 13241, 13653,
   18443, 18443, 18443, 18443, 18443, 18284,   828,   691,  9945, 13241,
   13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241, 13241,
   13241, -1499, -1499, -1499, -1499,  1933, 13241, 13241, -1499, 10357,
   10357, 13241, 13241,   417,   190, 17056,   737,   597, 12417,  4651,
   -1499, 13241, -1499,   739,   931,   798,   746,   751, 13796,   559,
   12623, -1499, 12829, -1499,   760,   765,  2264, -1499,   311, 10357,
   -1499, 16290, -1499, -1499, 15921, -1499, -1499, 10563, -1499, 13241,
   -1499,   858,  8915,   949,   768, 18611,   958,    76,    50, -1499,
   -1499, -1499,   806, -1499, -1499, -1499, 16217,  2563,   779,   970,
   16665, 18443, -1499, -1499, -1499, -1499, -1499,   797, -1499, -1499,
   -1499,   909, 13241,   910,   914, 13241, 13241, 13241, -1499,   697,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499,   796, -1499, -1499,
   -1499,   808, -1499, -1499, 18443,   794,   987,   110, 18443,   804,
     999,   459,   472, 18403, -1499, 18443, 13241,   559,   166, -1499,
   -1499, -1499, 16665,   933, -1499,   559,   111,   119,   813,   814,
    2340,   321,   818,   819,   142,   896,   825,   559,   127,   826,
   -1499,  4145, 18443, -1499, -1499,   959,  2173,   420, -1499, -1499,
   -1499,   475, -1499, -1499, -1499,   997,   900,   859,   427,   880,
   13241,   417,   902,  1023,   834,   886,   190, -1499, 16217, 16217,
    1025,   891,    61, -1499,   840,  1033, -1499, 16217,   201,   413,
     170, -1499, -1499, -1499, -1499, -1499, -1499, -1499,   966,  2749,
   -1499, -1499, -1499, -1499,  1034,   873, -1499, 17450, 13241,   847,
    1039, 18700,  1035, -1499, -1499,   925, 17625,  3669, 11370, 10340,
   13241, 18656, 12812,  3412,  3604, 14271,  2626,  3855,  3855,  3855,
    3855,  1077,  1077,  1077,  1077,   906,   906,   770,   770,   770,
     329,   329,   329, -1499,   792, 18700,   850,   851, 17989,   855,
    1050,   202, 13241,   307,   696,   378,   190, -1499, -1499, -1499,
    1047,   823, -1499,   597, 13241, 17188, -1499, -1499, 10340, -1499,
   10340, 10340, 10340, 10340, 10340, 10340, 10340, 10340, 10340, 10340,
   10340, 10340, -1499, 13241,   352,   204, -1499, -1499,   696,   367,
     856,  2973,   863,   864,   860,  3093,   155,   874, -1499, 18700,
   16795, -1499, 18443, -1499,   201,    64, 17450, 18700, 17450, 18033,
     201,   559,   217,   926,   878, 13241, -1499,   296, -1499, -1499,
   -1499,  8709,   146, -1499, -1499, 18700, 18700,   186, -1499, -1499,
   -1499, 13241,   990, 16497, 16665, 18443,  9121,   888,   889, -1499,
     102,  1010,   969,   954, -1499,  1097,   915, 16076, 16217, 16665,
   16665, 16665, 16665, 16665,   911,   982,   917, 16665,   501, -1499,
     983, -1499,   907, -1499, 18786, -1499,    34, -1499, 13241,   951,
   18700,   964,  1120, 11166,  1134, -1499, 18700, 16007, -1499,   796,
    1071, -1499,  5825, 18219,   950,   489, -1499, 18338, 18443,   544,
   -1499, 18386, 18443, 18443, -1499, -1499,  3308, -1499, 18786,  1139,
   17450,   961, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499,   121, 18443, 18219,   962, 17056, 17319,  1149, -1499, -1499,
   -1499, -1499,   960, -1499, 13241, -1499, -1499,  5001, -1499, 16217,
   18219,   976, -1499, -1499, -1499, -1499,  1154,   979, 13241, 17702,
   -1499, -1499, 13939,   986, -1499, 16217, -1499,   992,  6031,  1150,
      49, -1499, -1499,   381,  1933, -1499, 16290, -1499, 16217, -1499,
   -1499,   559, 18700, -1499, 10769, -1499, 16665,   128,   984, 18219,
     900, -1499, -1499, 12400, 13241, -1499, -1499, 13241, -1499, 13241,
   -1499,  3825,   985, 10357,   896,  1161,   900, 16217,  1178,   925,
   18443, 17576,   559,  4370,   994, -1499, -1499,   176,   995, -1499,
   -1499,  1181, 16717, 16717, 16795, -1499, -1499, -1499,  1148,  1002,
     445,  1003, -1499, -1499, -1499, -1499,  1189,  1004,   739,   559,
     559, 13035, 16290, -1499, -1499,  4416,   661,   186,  9739, -1499,
    6237,  1006,  6443,  1009, 16497, 17450,  1013,  1072,   559, 18786,
    1191, -1499, -1499, -1499, -1499,   642, -1499,   206, 16217, -1499,
    1089, 16217, 18443,  2563, -1499, -1499, -1499,  1216, -1499,  1024,
    1034,   603,   603,  1162,  1162, 18134,  1020,  1219, 16665, 15534,
   17702, 15965, 15391, 16665, 16665, 16665, 16665, 16367, 16665, 16665,
   16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665,
   16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665, 16665,
   18443, -1499, 18700, 13241, 13241, 13241, -1499, -1499, -1499, 13241,
   13241, -1499,   697, -1499,  1152, -1499, -1499, 18443, -1499, -1499,
   18443, -1499, -1499, -1499, -1499, 16665,   559, -1499,   142, -1499,
     695,  1220, -1499, -1499,   158,  1036,   559, 10975, -1499,   106,
   -1499,  5207,   823,  1220, -1499,    39,    10, -1499, 18700,  1103,
   -1499, -1499, -1499, -1499,  1037,  1150, -1499, 16217,   891, 16217,
      45,  1222,  1158,   299, -1499,   696,   300, -1499, -1499, 17450,
   13241, 18700, 18786,  1040,   128, -1499,  1041,   128,  1043, 12400,
   18700, 18090,  1046, 10357,  1048,  1044, 16217,  1051,  1038, 16217,
     900, -1499,   693,   496, 10357, 13241, -1499, -1499, -1499, -1499,
   -1499, -1499,  1119,  1049,  1243,  1168, 16795,  1115, -1499, 17702,
   16795, -1499, -1499, -1499, 17450, 18700, -1499,   186,  1233,  1192,
    9739, -1499, -1499, -1499,  1063, 13241,  1072,   559, 17056, 16497,
    1066, 16665,  6649,   652,  1067, 13241,    59,   446, -1499,  1098,
   16217, -1499,  1141, -1499, 16149,  1244,  1073, 16665, -1499, 16665,
   -1499,  1075, -1499,  1145,  1268,  1078, -1499, -1499, -1499, 18191,
    1076,  1272, 10752, 11784, 11988, 16665, 18744, 13018, 13648,  3512,
   14465,  2779, 12189, 12189, 12189, 12189,  2465,  2465,  2465,  2465,
     952,   952,   603,   603,   603,  1162,  1162,  1162,  1162, -1499,
   18700, 13432, 18700, -1499, 18700, -1499,  1082, -1499, -1499, -1499,
   18786, 18443, 16217, 16217, -1499, 18219,   115, -1499, 17056, -1499,
   -1499, 10340,  1081, -1499,  1083,   657, -1499,    60, 13241, -1499,
   -1499, -1499, 13241, -1499, 13241, -1499,   891, -1499, -1499,   454,
    1278,  1207, 13241, -1499,  1094,   559, 18700,  1150,  1095, -1499,
    1096,   128, 13241, 10357,  1100, -1499, -1499,   823, -1499, -1499,
    1093,  1101,  1102, -1499,  1105, 16795, -1499, 16795, -1499, -1499,
    1109, -1499,  1167,  1110,  1302, -1499,   559,  1282, -1499,  1112,
   -1499, -1499,  1114,  1116,   159, -1499, -1499, 18786,  1117,  1122,
   -1499,  4920, -1499, -1499, -1499, -1499, -1499, -1499, 16217, -1499,
   16217, -1499, 18786, 18235, -1499, 16665, 17702, -1499, -1499, 16665,
   -1499, 16665, -1499, 12606, 16665, 13241,  1123,  6855,   695, -1499,
   -1499, -1499,   619, 14664, 18219,  1210, -1499, 16595,  1170, 14912,
   -1499, -1499, -1499,   828, 16030,   100,   104,  1125,   823,   691,
     160, -1499, -1499, -1499,  1172,  4515,  4559, 18700, -1499,    47,
    1316,  1252, 13241, -1499, 18700, 10357,  1223,  1150,  1099,  1150,
    1132, 18700,  1137, -1499,  1463,  1136,  1838, -1499, -1499,   128,
   -1499, -1499,  1203, -1499, 16795, -1499, 17702, -1499, -1499,  8709,
   -1499, -1499, -1499, -1499,  9327, -1499, -1499, -1499,  8709, -1499,
    1140, 16665, 18786,  1211, 18786, 18292, 12606, 13226, -1499, -1499,
   -1499, 18219, 18219, 18443, -1499,  1329, 15534,    79, -1499, 14664,
     823, 18267, -1499,  1173, -1499,   105,  1143,   112, -1499, 15055,
   -1499, -1499, -1499,   113, -1499, -1499, 16730, -1499,  1146, -1499,
    1271,   597, -1499, 14858, -1499, 14858, -1499, -1499,  1337,   828,
   -1499, 14082, -1499, -1499, -1499, -1499,  1338,  1275, 13241, -1499,
   18700,  1155,  1157,  1156,   611, -1499,  1223,  1150, -1499, -1499,
   -1499, -1499,  1866,  1159, 16795, -1499,  1226,  8709,  9533,  9327,
   -1499, -1499, -1499,  8709, -1499, 18786, 16665, 16665, 13241,  7061,
    1160,  1164, -1499, 16665, -1499, 18219, -1499, -1499, -1499, -1499,
   -1499, 16217,  1019, 16595, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499,   169, -1499,  1170, -1499, -1499,
   -1499, -1499, -1499,   125,   157, -1499,  1348,   114, 15248,  1271,
    1352, -1499, 16217,   597, -1499, -1499,  1171,  1354, 13241, -1499,
   18700, -1499,   145, -1499, -1499, -1499, -1499,  1169,   611, 14276,
   -1499,  1150, -1499, 16795, -1499, -1499, -1499, -1499,  7267, 18786,
   18786, 11578, -1499, -1499, -1499, 18786, -1499,  1516,   123,  1360,
    1174, -1499, -1499, 16665, 15055, 15055,  1319, -1499, 16730, 16730,
     678, -1499, -1499, -1499, 16665,  1296, -1499,  1205,  1179,   116,
   16665, -1499, 18443, -1499, 16665, 18700,  1300, -1499,  1371,  7473,
    7679, -1499, -1499, -1499,   611, -1499,  7885,  1180,  1304,  1274,
   -1499,  1288,  1236, -1499, -1499,  1290, 16217, -1499,  1019, -1499,
   -1499, 18786, -1499, -1499,  1228, -1499,  1355, -1499, -1499, -1499,
   -1499, 18786,  1378,   142, -1499, -1499, 18786,  1194, 18786, -1499,
     332,  1195, -1499, -1499,  8091, -1499,  1193, -1499, -1499,  1197,
    1230, 18443,   691,  1227, -1499, -1499, 16665,   152,   129, -1499,
    1321, -1499, -1499, -1499, -1499, 18219,   950, -1499,  1237, 18443,
     621, -1499, 18786, -1499,  1201,  1394,   683,   129, -1499,  1326,
   -1499, 18219,  1208, -1499,  1150,   133, -1499, -1499, -1499, -1499,
   16217, -1499,  1206,  1213,   122, -1499,   625,   683,   457,  1150,
    1209, -1499, -1499, -1499, -1499, 16217,   292,  1400,  1336,   625,
   -1499,  8297,   474,  1403,  1339, 13241, -1499, -1499,  8503, -1499,
     316,  1406,  1342, 13241, -1499, 18700, -1499,  1408,  1344, 13241,
   -1499, 18700, 13241, -1499, 18700, 18700
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1499, -1499, -1499,  -561, -1499, -1499, -1499,   249,     2,  -326,
   -1499, -1499, -1499,   815,   567,   569,    -3,  1499,  2375, -1499,
    2568, -1499,  -477, -1499,    40, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499, -1499,  -358, -1499, -1499,  -158,
      65,    27, -1499, -1499, -1499, -1499, -1499, -1499,    28, -1499,
   -1499, -1499, -1499,    29, -1499, -1499,   967,   971,   972,   -95,
     476,  -858,   479,   536,  -364,   242,  -920, -1499,   -89, -1499,
   -1499, -1499, -1499,  -729,    93, -1499, -1499, -1499, -1499,  -356,
   -1499,  -589, -1499,  -449, -1499, -1499,   848, -1499,   -75, -1499,
   -1499, -1047, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499, -1499,  -106, -1499,   -22, -1499, -1499, -1499, -1499, -1499,
    -187, -1499,    66,  -850, -1499, -1498,  -377, -1499,  -139,    97,
    -133,  -360, -1499,  -196, -1499, -1499, -1499,    75,   -40,    -6,
     338,  -737,  -411, -1499, -1499,   -19, -1499, -1499,    -5,   -53,
     -55, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
    -609,  -826, -1499, -1499, -1499, -1499, -1499,   228, -1499, -1499,
   -1499, -1499,   996, -1499, -1499,   380, -1499,   894, -1499, -1499,
   -1499, -1499, -1499, -1499, -1499,   387, -1499,   903, -1499, -1499,
     620, -1499,   356, -1499, -1499, -1499, -1499, -1499, -1499, -1499,
   -1499,  -843, -1499,  2028,  1294,  -394, -1499, -1499,   308,  3305,
    2943, -1499, -1499,   435,  -152,  -658, -1499, -1499,   502,   301,
    -683,   302, -1499, -1499, -1499, -1499, -1499,   491, -1499, -1499,
   -1499,    30,  -874,  -147,  -426,  -409, -1499,   553,  -114, -1499,
   -1499,    24,    31,   613, -1499, -1499,   165,   -36, -1499, -1499,
      85,  -108, -1499,   230, -1499, -1499, -1499,  -467,  1113, -1499,
   -1499, -1499, -1499, -1499,   649,   494, -1499, -1499,  1121,  -350,
    -692, -1499,  1070,  -895, -1499,   -66,  -159,    56,   671, -1499,
   -1063,   124,  -267,   391,   468, -1499, -1499, -1499, -1499,   422,
    1215, -1080
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1006
static const yytype_int16 yytable[] =
{
     187,   189,   420,   191,   192,   193,   195,   196,   556,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   466,   935,   225,   228,   383,   246,   496,   766,   386,
     387,   130,   132,   133,   160,   647,   337,   249,   251,   958,
     762,   763,   916,   255,   128,   257,  1129,   260,   791,  1299,
     338,   488,   343,   420,   237,   349,   328,   736,   713,   755,
     394,   238,  1296,   917,  1003,   465,   494,  1285,   416,   129,
     784,   897,   517,   392,   249,   513,   756,   396,   379,  1017,
     807,   380,  1146,  1119,  1196,   805,   -43,   985,  1545,  1352,
     410,   -43,   529,   521,    13,   787,   593,   -78,  1157,   -42,
     598,   135,   -78,   852,   -42,   603,    13,   529,   393,  1493,
    1404,  1018,   788,  1495,  -355,   239,   427,   428,   429,  1382,
     870,  1553,  1637,  1705,  1300,  1705,  1506,   522,   870,   486,
    1098,  1545,  1736,   724,  1698,   430,   886,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,  1699,   452,
      13,  1098,   507,   581,   886,  -553,  1291,   886,   886,   886,
    1722,   453,   406,    13,    13,   506,   918,   515,    13,   396,
    1716,   595,   724,  1693,  1070,   499,  1701,   388,   879,   880,
     514,   222,   222,   483,   484,  1286,  -861,   358,     3,  1694,
     419,  -523,  -555,  1702,   190,   407,  1703,   858,  1287,  1301,
     393,  1507,  1292,  -860,  1223,   384,  1695,   997,   998,   633,
     634,   582,  1383,   622,  1717,   393,  1765,  1384,  -863,    60,
      61,    62,   178,  1385,   417,  1386,  -867,   524,   406,  -556,
     524,  1288,  -866,  -862,  1416,   350,  -780,   249,   535,  -780,
    -903,   497,   808,   121,   217,   217,   491,   467,  -294,   546,
    1353,   686,  1418,  -874,   406,   487,   491,   486,  -697,  1424,
    -864,  1426,  1387,  1388,  1019,  1389,   806,   646,   -43,  1345,
    1546,  1547,   411,  -906,   530,  1203,   526,  1207,   594,   -78,
     531,   -42,   599,  1099,   242,  1737,   418,   604,  -871,   620,
     258,  1494,  1321,   327,   389,  1496,  -355,  1205,  1206,   609,
     390,   871,  1282,  1554,  1638,  1706,  1000,  1755,  1390,   872,
     364,  1002,   608,  1821,  1793,  1700,   612,   887,  -294,  -278,
     483,   484,  -861,  -780,   420,   490,   328,  1111,   381,   767,
     364,   587,   591,   592,   364,   364,  -870,   684,  1143,  -860,
    -698,   726,   773,  1332,  1514,   971,  1516,   393,  1277,  1443,
    1500,  -869,  -905,   225,  -863,  -847,  -848,  1780,   626,  -873,
     351,  1833,  -867,  -104,   631,  -876,   356,   726,  -866,  -862,
     248,  1522,   452,   364,   357,   245,  -903,  -555,   211,   222,
     211,   252,   492,   669,   453,  1847,   383,   730,   731,   416,
     726,  -104,   492,   487,   681,   341,  -864,   354,  -696,  1208,
     726,  1781,   328,   726,   355,   253,  1130,   369,  1104,  -906,
    1284,   656,   687,   688,   689,   691,   692,   693,   694,   695,
     696,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   503,   714,   737,
     715,   715,   217,   718,  1668,   254,  1834,  -872,  1342,   360,
    1131,   723,   129,   738,   740,   741,   742,   743,   744,   745,
     746,   747,   748,   749,   750,   751,   237,   903,  -103,  -557,
    1848,   715,   761,   238,   681,   681,   715,   765,  1465,  1409,
     361,   910,  1826,   738,   135,   121,   769,  1335,  -905,   121,
    1136,  -847,  -848,   536,  1154,   777,  -103,   779,   794,  1840,
     328,   924,   926,   493,   681,   116,   222,  1137,   505,  1298,
     490,   876,   795,  -835,   796,   222,   222,   611,  -700,   466,
    1177,   733,   222,  1410,  1202,   370,  1827,   239,  -838,   222,
    1453,   628,   483,   484,   725,  1132,   370,  1205,  1206,   483,
     484,  -835,   628,  1841,   362,  1308,   862,   840,  1310,   647,
     843,   846,   847,  -699,   363,   952,  -838,  1162,   950,   863,
     759,   367,   799,   465,  1548,   960,   368,   953,   370,   217,
     372,   855,   904,   555,   399,   859,  1087,   907,   217,   217,
     349,   866,  1178,   725,   621,   217,   384,   905,  1650,   964,
    1651,   370,   217,   783,   373,   374,   789,   402,  1724,   385,
    1526,   370,  -874,   406,   370,   373,   374,   164,  1411,   398,
     942,  1828,   405,   896,   370,  1425,   406,   924,   926,   406,
     371,   370,   412,   992,   926,   393,   409,   628,  1842,   221,
     223,  1090,  1431,   406,  1432,   421,   121,   373,   374,  1355,
     370,  -978,  1066,  1067,  1068,   673,   628,   483,   484,   327,
     222,  1382,   364,  1024,   672,   422,   993,  -836,  1069,   517,
     373,   374,   407,   932,   671,   455,  -978,  1071,   423,   623,
     373,   374,   656,   373,   374,   943,   717,  1408,   483,   484,
     647,   424,   372,   373,   374,  -836,  1279,   407,   406,   629,
     373,   374,    13,  -978,  1314,   425,  1498,  1748,   555,   364,
     728,   364,   364,   364,   364,  1324,   760,   951,   426,   373,
     374,   764,  1420,   217,  1749,  -978,   395,  1750,  -978,   195,
    1818,  1344,  1187,  1188,   754,    60,    61,    62,   178,   179,
     417,  1816,  1204,  1205,  1206,  1832,  1461,  1462,   963,   457,
      53,  1525,  1349,  1205,  1206,   456,  1829,   555,    60,    61,
      62,   178,   179,   417,  1383,   467,   401,   403,   404,  1384,
     786,    60,    61,    62,   178,  1385,   417,  1386,  1549,   458,
     995,   121,   459,    60,    61,    62,    63,    64,   417,   489,
    1377,  1001,  1272,  1273,    70,   460,   249,  1807,  1808,  1809,
     836,   646,   418,  -868,   726,    60,    61,    62,   178,   179,
     417,  1400,  1664,  1665,  1387,  1388,   726,  1389,   726,   449,
     450,   451,  1803,   452,  -554,   418,  1822,  1823,   395,   647,
     461,  -697,   462,  1072,  1181,   453,   495,   509,   418,   377,
    1523,  1672,   222,   516,   865,   463,  1012,   464,  1746,  1747,
     418,  1742,  1743,   502,   587,   500,   129,   453,   591,   164,
    1403,  1439,   508,   164,  1422,   624,  -872,   407,   511,   630,
     891,   893,   418,  1027,  1030,   490,   512,  1448,  1216,   561,
     562,   563,  -695,   518,   726,  1220,   564,   565,   135,  1109,
     566,   567,   519,   624,  1161,   630,   624,   630,   630,   527,
     222,   540,   547,  1118,  1796,   217, -1005,   550,   557,   551,
     558,   560,   600,   601,  1121,   602,   613,   129,   614,   648,
     649,   658,   659,  1796,   130,   132,   133,   160,   661,  1141,
     670,  1817,   646,  -129,    53,   364,   683,   128,   770,  1149,
     772,   222,  1150,   222,  1151,   623,   774,   797,   681,   135,
     656,   775,   446,   447,   448,   449,   450,   451,   529,   452,
     780,  1527,   129,   217,   597,   781,  1511,   656,   222,   801,
    1533,   453,   804,   605,   605,   610,  1304,   546,   817,   818,
     617,  1539,   838,   129,   850,   237,  1185,   627,   839,   841,
    1725,  1190,   238,   842,   135,   856,   857,  1122,  1063,  1064,
    1065,  1066,  1067,  1068,   217,   860,   217,   853,   861,   759,
     164,   789,   869,   873,   874,   135,   647,  1069,   877,   982,
     878,   988,  1328,   883,   885,   894,   888,   899,   900,   902,
    -715,   217,   909,   908,   911,   222,   912,   919,  1191,   915,
     121,  1222,   920,   928,  1228,   930,   239,   933,   934,   936,
     222,   222,   939,  1678,  1010,   121,   945,   946,   948,   949,
    1761,   957,   965,   967,   968,   129,   969,   129,  1260,  1261,
    1262,   646,  1367,   941,   843,  1264,  -700,   789,   994,  1372,
      60,    61,    62,    63,    64,   417,   647,  1004,  1280,  1014,
    1016,    70,   460,    33,    34,    35,  1020,   135,   217,   135,
    1021,   121,  1281,  1382,  1022,   212,  1023,  1089,   617,  1043,
    1038,  1092,  1093,   217,   217,  1025,  1040, -1006, -1006, -1006,
   -1006,   444,   445,   446,   447,   448,   449,   450,   451,   462,
     452,  1100,  1039,  1042,  1075,  1306,  1073,  1806,   130,   132,
     133,   160,   453,  1079,    13,   164,   121,   418,   681,  1074,
    1082,   128,  1085,  1095,    74,    75,    76,    77,    78,   681,
    1281,   555,  1097,  1107,  1103,   214,  1108,   121,  1116,   222,
     222,    82,    83,   754,   656,   786,   129,   656,  1114,  1117,
    1127,  1337,  1437,  1144,  1153,    92,  1719,  1123,  1720,  1125,
     249,  1156,  1159,  1164,  -875,  1165,  1175,  1726,  1180,    97,
    1351,  1176,  1179,  1199,  1182,  1201,  1383,  1193,   135,   364,
    1195,  1384,  1198,    60,    61,    62,   178,  1385,   417,  1386,
    1210,  1169,  1169,   982,  1215,  1214,  1218,  1069,  1219,  1275,
    1340,  1266,   217,   217,  1294,  1278,  1302,  1303,  1295,  1319,
    1307,   786,  1311,  1309,  1764,  1313,  1316,   121,  1315,   121,
    1325,   121,  1327,  1318,   978,  1326,  1387,  1388,   646,  1389,
    1499,   420,  1331,  1338,  1341,  1339,  1346,   129,  1350,   334,
    1356,  1212,  1358,  1361,  1360,  1364,  1365,  1366,  1368,  1370,
     418,  1371,  1376,  1405,  1401,  1402,  1413,  1406,   555,  1407,
     931,   555,  1412,  1415,  1427,  1417,  1419,  1414,  1434,   135,
    1423,  1429,  1515,  1428,   222,  1430,  1399,  1421,   681,  1433,
    1435,  1436,  1438,  1440,  1441,  1399,  1442,  1469,  1445,   836,
     219,   219,  1707,  1446,   233,  1458,  1497,  1502,   646,  1482,
    1508,  1509,  1517,  1648,  1524,  1512,  1268,  1518,  1520,  1269,
    1534,   656,  1536,  1543,  1551,  1552,  1831,  1645,   962,   222,
    1646,  1652,  1658,  1838,  1659,  1661,  1662,  1673,  1663,  1671,
     121,  1683,  1704,   222,   222,  1684,  1710,   217,  1714,  1738,
    1457,  1721,  1713,  1394,  1744,  1752,  1753,  1740,  1754,  1759,
    1760,  1767,  1394,  1768,  1769,  -351,  1771,  1772,  1699,   989,
    1775,   990,  1776,  1779,  1784,  1782,  1785,  1542,  1786,  1791,
    1798,  1804,  1801,  1805,   164,  1813,  1819,  1510,   864,  1815,
     681,  1830,   217,  1820,  1835,  1836,  1008,  1843,  1844,   164,
    1849,  1850,  1852,  1853,  1088,   982,   217,   217,  1800,   982,
    1091,   727,   732,  1155,   729,  1160,  1113,  1814,  1343,   121,
    1677,  1812,   129,   222,  1447,  1669,   867,  1692,  1399,  1550,
    1697,   121,  1837,  1709,  1399,  1489,  1399,  1825,  1491,   656,
    1470,  1667,  1265,   848,   739,   164,  1263,  1382,  1544,  1081,
    1323,  1289,   849,  1221,   135,  1171,  1329,  1135,  1330,  1183,
     682,   619,   607,  1096,  1531,   544,  1777,   545,  1028,  1271,
       0,  1213,  1259,     0,     0,  1712,   467,     0,   617,  1106,
       0,     0,  1460,  1660,   129,     0,   217,     0,    13,     0,
     164,     0,     0,   129,     0,  1394,     0,     0,   219,     0,
    1378,  1394,     0,  1394,     0,   216,   216,     0,     0,   231,
       0,   164,     0,  1681,     0,     0,   135,  1739,     0,     0,
       0,   549,     0,     0,     0,   135,     0,     0,     0,     0,
       0,     0,  1399,   231,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1653,     0,     0,     0,  1675,  1531,
    1383,     0,     0,     0,   982,  1384,   982,    60,    61,    62,
     178,  1385,   417,  1386,     0,     0,     0,     0,     0,     0,
       0,     0,   129,    36,     0,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   129,     0,     0,     0,     0,     0,
       0,   164,     0,   164,    48,   164,     0,  1008,  1197,  1394,
    1387,  1388,   676,  1389,   135,   334,   121,     0,     0,     0,
     135,     0,   327,  1789,     0,     0,   135,     0,  1487,     0,
       0,     0,     0,     0,   418,   219,  1757,     0,     0,     0,
     328,     0,     0,  1715,   219,   219,     0,   420,     0,     0,
       0,   219,     0,     0,     0,     0,  1519,   182,   219,     0,
      84,     0,     0,    86,    87,     0,    88,   183,    90,   645,
       0,     0,     0,   982,     0,     0,     0,     0,   121,     0,
       0,     0,     0,   121,     0,     0,     0,   121,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   364,     0,     0,   555,     0,     0,   327,  1730,
       0,     0,     0,   216,   164,     0,     0,     0,  1634,     0,
       0,     0,     0,     0,     0,  1641,     0,     0,     0,     0,
       0,     0,   327,   129,   327,     0,   233,     0,     0,     0,
     327,     0,  1305,     0,     0,     0,     0,     0,     0,     0,
       0,   810,     0,     0,     0,     0,     0,     0,     0,   231,
       0,   231,     0,   982,     0,   135,   121,   121,   121,     0,
       0,     0,   121,     0,   129,   129,     0,     0,   121,   219,
       0,   129,     0,     0,     0,     0,     0,  1336,     0,     0,
       0,     0,     0,   164,     0,     0,     0,     0,   656,     0,
       0,   617,  1008,     0,     0,   164,   135,   135,     0,     0,
       0,     0,     0,   135,     0,   231,     0,   656,     0,   129,
    1845,     0,     0,     0,     0,   656,     0,     0,  1851,     0,
       0,     0,  1382,     0,  1854,     0,     0,  1855,     0,     0,
     216,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     216,   135,     0,   913,   914,     0,   216,     0,     0,  1790,
    1382,     0,   922,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,   231,     0,     0,     0,     0,     0,
       0,   617,     0,     0,     0,     0,   129,   555,     0,     0,
       0,     0,     0,   129,     0,     0,   231,     0,     0,   231,
       0,    13,     0,     0,     0,     0,     0,     0,   327,     0,
       0,     0,   982,     0,     0,     0,     0,   121,   135,     0,
       0,     0,     0,     0,     0,   135,  1731,     0,     0,     0,
       0,     0,     0,  1634,  1634,  1383,     0,  1641,  1641,     0,
    1384,   231,    60,    61,    62,   178,  1385,   417,  1386,     0,
       0,   364,     0,     0,     0,     0,     0,     0,   121,   121,
       0,   219,     0,  1383,     0,   121,     0,     0,  1384,     0,
      60,    61,    62,   178,  1385,   417,  1386,     0,     0,     0,
     164,     0,     0,     0,   216,  1387,  1388,     0,  1389,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   211,   121,     0,     0,     0,     0,     0,   418,
    1788,     0,     0,  1387,  1388,     0,  1389,     0,     0,   219,
       0,    48,   676,   676,     0,     0,     0,     0,  1802,     0,
       0,  1521,     0,     0,     0,   231,   231,   418,     0,   828,
       0,     0,   164,     0,   218,   218,     0,   164,   232,     0,
       0,   164,     0,     0,     0,     0,     0,     0,     0,  1670,
     219,     0,   219,     0,     0,     0,     0,     0,     0,     0,
     121,     0,    36,     0,     0,     0,     0,   121,   752,     0,
      86,    87,     0,    88,   183,    90,     0,   219,     0,     0,
       0,   828,     0,    48,     0,     0,     0,     0,     0,     0,
       0,   344,   345,     0,  1112,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
    1124,     0,     0,     0,     0,   753,     0,   116,     0,     0,
     164,   164,   164,  1138,     0,     0,   164,   231,   231,     0,
       0,     0,   164,     0,     0,     0,   231,     0,     0,   346,
       0,     0,    86,    87,   219,    88,   183,    90,     0,     0,
       0,     0,  1158,     0,     0,     0,   216,     0,     0,   219,
     219,     0,     0,   427,   428,   429,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   430,   645,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,     0,   452,     0,     0,     0,
       0,     0,     0,  1209,   216,     0,  1211,     0,   453,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   218,     0,     0,   233,     0,   498,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,     0,     0,     0,     0,   216,     0,   216,   498,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   164,     0,     0,     0,     0,     0,     0,   219,   219,
       0,     0,   216,   828,     0,     0,   481,   482,     0,     0,
       0,     0,     0,     0,     0,     0,   231,   231,   828,   828,
     828,   828,   828,     0,     0,     0,   828,   481,   482,     0,
       0,     0,   164,   164,   645,     0,     0,     0,     0,   164,
       0,     0,  1297,     0,   922,     0,     0,     0,     0,     0,
       0,     0,   231,     0,   498,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,     0,   216,
       0,  1317,   483,   484,  1320,     0,   895,   164,     0,   218,
       0,     0,   231,     0,   216,   216,     0,     0,   218,   218,
       0,     0,     0,   483,   484,   218,     0,     0,   231,   231,
       0,     0,   218,   481,   482,     0,     0,     0,   231,     0,
       0,     0,     0,   218,   231,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1357,     0,   231,     0,  1138,
       0,     0,     0,   219,     0,   828,     0,     0,   231,     0,
       0,     0,     0,   660,   164,     0,     0,     0,     0,     0,
       0,   164,     0,     0,     0,     0,   231,     0,     0,     0,
     231,     0,     0,     0,   782,     0,     0,     0,     0,   483,
     484,     0,     0,   645,     0,     0,     0,     0,   219,     0,
     232,     0,     0,     0,     0,     0,     0,  1379,  1380,     0,
       0,     0,   219,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   216, -1006, -1006, -1006, -1006,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,   231,     0,     0,
     231,     0,   231,   218,     0,     0,     0,     0,     0,     0,
    1069,     0,     0,     0,     0,     0,     0,   828,     0,   231,
     875,     0,   828,   828,   828,   828,   828,   828,   828,   828,
     828,   828,   828,   828,   828,   828,   828,   828,   828,   828,
     828,   828,   828,   828,   828,   828,   828,   828,   828,     0,
       0,     0,   219,  1449,     0,  1450,     0,     0,   832,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   811,     0,   828,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,  1492,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   330,     0,     0,     0,   231,     0,   231,     0,
     832,     0,     0,     0,     0,     0,     0,     0,   216,     0,
      36,     0,     0,     0,   481,   482,     0,     0,     0,   812,
       0,     0,     0,     0,     0,   231,     0,     0,   231,     0,
     645,    48,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   231,   452,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,   453,     0,     0,     0,     0,     0,   216,   216,     0,
     828,     0,     0,     0,     0,   218,     0,     0,     0,   231,
     483,   484,     0,   231,   182,     0,   828,    84,   828,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
     645,     0,     0,     0,   828,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   427,
     428,   429,     0,   218,     0,     0,  1688,     0,     0,     0,
       0,   231,   231,     0,   231,     0,     0,   216,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,     0,   452,     0,   218,     0,   218,     0,     0,     0,
       0,     0,     0,     0,   453,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,   218,   832,     0,     0,     0,     0,     0,   330,     0,
     330,     0,     0,     0,  1069,     0,     0,   832,   832,   832,
     832,   832,     0,     0,     0,   832,     0,   231,     0,   231,
       0,     0,     0,     0,   828,   231,     0,  1711,   828,     0,
     828,     0,     0,   828,     0,     0,     0,     0,     0,     0,
       0,  1084,   231,   231,     0,     0,   231,     0,     0,     0,
       0,     0,     0,   231,   330,     0,     0,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1102,     0,   218,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   829,     0,     0,  1102,     0,
       0,     0,     0,     0,     0,   231,     0,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     828,  1773,   927,     0,     0,     0,     0,     0,     0,     0,
     231,   231,     0,     0,   832,     0,     0,  1145,   231,     0,
     231,     0,     0,     0,     0,   330,     0,   829,   330,     0,
       0,     0,     0,   427,   428,   429,     0,     0,     0,   232,
       0,     0,   231,     0,   231,     0,     0,     0,     0,     0,
     231,     0,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   922,   452,     0,     0,     0,
       0,     0,   218,   218,     0,   828,   828,     0,   453,     0,
     922,     0,   828,     0,   231,     0,     0,     0,     0,     0,
     231,     0,   231,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   832,     0,   218,     0,
       0,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,     0,     0,
       0,     0,     0,   427,   428,   429,     0,     0,     0,     0,
       0,     0,     0,     0,   330,   813,     0,     0,   830,     0,
       0,     0,   430,   832,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,     0,   452,     0,     0,     0,
       0,   231,     0,     0,     0,     0,     0,     0,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   218,   231,     0,
     830,     0,     0,     0,     0,     0,   966,     0,     0,   829,
       0,     0,     0,     0,     0,     0,   231,     0,     0,     0,
       0,     0,   828,     0,   829,   829,   829,   829,   829,     0,
       0,     0,   829,   828,     0,     0,     0,   218,     0,   828,
       0,     0,   218,   828,     0,     0,   330,   330,     0,     0,
       0,     0,     0,     0,     0,   330,   218,   218,     0,   832,
       0,     0,     0,     0,     0,   231,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   832,     0,   832,   498,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,     0,   832,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   828,     0,     0,     0,     0,
       0,     0,     0,     0,   231,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   970,   481,   482,     0,
     231,     0,     0,  1381,     0,     0,   218,     0,     0,   231,
       0,   829,     0,     0,     0,     0,     0,     0,   427,   428,
     429,     0,     0,     0,   231,     0,     0,     0,     0,     0,
       0,   220,   220,     0,     0,   235,     0,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
       0,   452,     0,   483,   484,     0,     0,     0,     0,     0,
       0,     0,   830,   453,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   330,   330,   830,   830,   830,
     830,   830,     0,   832,   218,   830,     0,   832,     0,   832,
       0,     0,   832,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1468,   829,     0,  1481,     0,     0,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   218,   452,     0,   330,     0,     0,
     829,     0,     0,     0,     0,     0,     0,   453,     0,   832,
       0,     0,     0,   330,     0,     0,     0,     0,     0,  1540,
    1541,     0,     0,   834,     0,     0,   330,     0,     0,  1481,
       0,     0,     0,     0,   830,     0,     0,     0,     0,     0,
       0,  1094,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   330,     0,     0,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   868,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,   832,   832,     0,     0,     0,     0,
       0,   832,     0,  1686,     0,     0,   829,  1069,     0,     0,
       0,  1481,     0,     0,     0,     0,   330,     0,     0,   330,
       0,   813,   829,     0,   829,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   830,     0,     0,     0,
     829,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   220,   452,     0,     0,
       0,     0,     0,   830,     0,   220,   220,     0,     0,   453,
       0,     0,   220,     0,     0,     0,     0,     0,     0,   220,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     235,     0,     0,     0,     0,   330,     0,   330,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   832,   452,     0,   330,     0,     0,   330,     0,     0,
       0,     0,   832,     0,   453,     0,     0,     0,   832,     0,
     829,     0,   832,     0,   829,     0,   829,  1009,     0,   829,
       0,     0,     0,     0,     0,     0,     0,   235,     0,     0,
       0,     0,  1031,  1032,  1033,  1034,     0,     0,     0,   830,
    1041,     0,     0,     0,     0,     0,     0,     0,   330,     0,
       0,     0,   330,     0,     0,   830,     0,   830,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,     0,     0,   830,   832,     0,     0,     0,     0,     0,
       0,     0,     0,  1799,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   829,     0,     0,  1468,
       0,     0,     0,     0,     0,   427,   428,   429,     0,     0,
     330,   330,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   430,   833,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,     0,   452,  1142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     453, -1006, -1006, -1006, -1006,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   833,   452,     0,
       0,   829,   829,     0,     0,     0,     0,     0,   829,     0,
     453,     0,     0,     0,     0,     0,   330,     0,   330,     0,
       0,     0,     0,   830,     0,     0,     0,   830,     0,   830,
      36,     0,   830,     0,     0,     0,     0,     0,     0,     0,
       0,   330,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   330,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,     0,     0,     0,  1229,  1232,  1233,  1234,
    1236,  1237,  1238,  1239,  1240,  1241,  1242,  1243,  1244,  1245,
    1246,  1247,  1248,  1249,  1250,  1251,  1252,  1253,  1254,  1255,
    1256,  1257,  1258,     0,     0,     0,     0,   377,     0,   830,
      86,    87,     0,    88,   183,    90,     0,     0,  1152,     0,
       0,     0,     0,     0,     0,     0,     0,   330,  1270,     0,
     220,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   330,     0,   330,     0,   378,     0,     0,   829,   330,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   829,
       0,   220,     0,   220,     0,   829,     0,     0,     0,   829,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   830,   830,     0,     0,   220,   833,
       0,   830,     0,     0,     0,     0,     0,     0,     0,   330,
       0,     0,     0,     0,   833,   833,   833,   833,   833,     0,
       0,   270,   833,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1347,     0,     0,     0,     0,     0,
       0,   829,     0,     0,     0,     0,     0,   272,     0,     0,
    1362,     0,  1363,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,     0,     0,  1373,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,   220,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,   548,     0,
       0,     0,     0,     0,   235,     0,     0,     0,     0,     0,
     330,     0,    36,     0,   889,   890,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,   543,   330,     0,     0,
       0,   833,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,     0,  1732,    84,   321,     0,    86,
      87,   830,    88,   183,    90,     0,   235,     0,     0,     0,
       0,     0,   830,     0,     0,     0,     0,   325,   830,     0,
       0,     0,   830,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   326,    86,    87,   330,    88,   183,    90,  1452,   220,
     220,     0,  1454,     0,  1455,     0,     0,  1456,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   833,   830,   235,     0,     0,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,     0,     0,     0,   330,     0,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   330,  1535,     0,     0,     0,     0,   430,
     833,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,     0,   452,     0,     0,   427,   428,   429,     0,
       0,     0,     0,     0,     0,   453,     0,     0,     0,     0,
       0,     0,     0,     0,   220,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,     0,   452,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1679,
    1680,   453,     0,     0,   235,     0,  1685,     0,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   220,   220,     0,   833,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   833,     0,   833,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     833,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,     0,   452,   427,
     428,   429,     0,  1163,     0,     0,     0,     0,     0,     0,
     453,     0,     0,   220,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,     0,   452,     0,     0,     0,     0,     0,     0,  1186,
       0,     0,     0,     0,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1741,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1751,     0,     0,
       0,     0,     0,  1756,     0,     0,     0,  1758,     0,     0,
       0,   427,   428,   429,     0,     0,     0,     0,     0,     0,
     833,   235,     0,     0,   833,     0,   833,     0,     0,   833,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,     0,   452,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   453,     0,  1504,  1792,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   235,     0,     0,    11,    12,     0,   525,     0,     0,
       0,     0,     0,     0,     0,     0,   833,     0,     0,     0,
       0,     0,  1505,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   178,   179,    65,     0,    66,    67,
      68,   833,   833,     0,     0,     0,     0,     0,   833,    72,
      73,   768,    74,    75,    76,    77,    78,  1691,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     0,     0,     0,     0,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   430,
    1352,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,     0,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   453,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   833,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,   833,
       0,     0,     0,     0,    10,   833,     0,     0,     0,   833,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1774,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,   833,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    62,    63,    64,
      65,  1353,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1110,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,    81,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,    95,     0,    96,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1283,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,    81,    82,    83,    84,    85,     0,
      86,    87,     0,    88,    89,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,    95,
       0,    96,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,   662,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1083,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1126,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1192,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,  1194,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,  1348,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
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
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1459,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1682,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,  1727,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
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
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1762,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1763,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,  1766,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
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
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1783,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1839,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1846,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   798,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,   178,   179,    65,     0,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,  1011,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   178,   179,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,  1530,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   178,   179,    65,     0,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1674,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     178,   179,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
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
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   178,   179,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   734,    12,     0,     0,     0,     0,     0,     0,
     735,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,   335,    99,   100,
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
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,   335,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   336,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   430,
      10,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   677,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   453,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,   678,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
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
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     178,   179,   180,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,  1045,  1046,   793,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,  1047,    10,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1139,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,  1069,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,  1140,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   734,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   427,   428,   429,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,     0,   452,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   453,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,   194,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,  1076,  1077,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   429,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   430,
      10,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   224,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   453,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   427,   428,
     429,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
       0,   452,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   453,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     178,   179,   180,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,  1728,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   256,     0,  1046,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1047,     0,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,  1069,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     259,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,  1047,    10,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,  1069,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,   523,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10, -1006, -1006, -1006, -1006,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,   690,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1069,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   735,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   453,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,   776,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,  1069,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     178,   179,   180,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   778,   452,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   453,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1184,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,  1069,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   427,   428,   429,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,     0,   452,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   453,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,  1538,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   427,   428,   429,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,     0,   452,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   453,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,   625,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,  1375,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,   261,   262,     0,   263,
     264,     0,     0,   265,   266,   267,   268,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
       0,     0,     0,     0,     0,     0,   271,     0,     0,     0,
       0,     0,     0,  1069,     0,     0,     0,     0,     0,     0,
     273,   274,   275,   276,   277,   278,   279,     0,     0,     0,
      36,     0,   211,     0,     0,     0,     0,     0,     0,     0,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,    48,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,     0,     0,     0,     0,   721,   314,
     315,   316,     0,     0,     0,   317,   552,   553,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   261,
     262,     0,   263,   264,   554,     0,   265,   266,   267,   268,
      86,    87,     0,    88,   183,    90,   322,     0,   323,     0,
       0,   324,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   271,
       0,     0,     0,     0,     0,   722,     0,   116,     0,     0,
       0,     0,     0,   273,   274,   275,   276,   277,   278,   279,
       0,     0,     0,    36,     0,   211,     0,     0,     0,     0,
       0,     0,     0,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,    48,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     0,     0,     0,
       0,   313,   314,   315,   316,     0,     0,     0,   317,   552,
     553,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   261,   262,     0,   263,   264,   554,     0,   265,
     266,   267,   268,    86,    87,     0,    88,   183,    90,   322,
       0,   323,     0,     0,   324,     0,   269,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   271,     0,     0,     0,     0,     0,   722,     0,
     116,     0,     0,     0,     0,     0,   273,   274,   275,   276,
     277,   278,   279,     0,     0,     0,    36,     0,   211,     0,
       0,     0,     0,     0,     0,     0,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,    48,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
       0,     0,     0,     0,  1120,   314,   315,   316,     0,     0,
       0,   317,   552,   553,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   261,   262,     0,   263,   264,
     554,     0,   265,   266,   267,   268,    86,    87,     0,    88,
     183,    90,   322,     0,   323,     0,     0,   324,     0,   269,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   271,     0,   272,     0,     0,
       0,     0,     0,   116,     0,     0,     0,     0,     0,   273,
     274,   275,   276,   277,   278,   279,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
      48,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,     0,     0,     0,     0,     0,   314,   315,
     316,     0,     0,     0,   317,   318,   319,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,    84,   321,     0,    86,
      87,     0,    88,   183,    90,   322,     0,   323,     0,     0,
     324,     0,     0,     0,     0,     0,     0,   325,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   261,
     262,   326,   263,   264,     0,  1654,   265,   266,   267,   268,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,     0,   270,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,     0,   452,     0,     0,     0,     0,   271,
       0,   272,     0,     0,     0,     0,   453,     0,     0,     0,
       0,     0,     0,   273,   274,   275,   276,   277,   278,   279,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,    48,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     0,     0,     0,
       0,     0,   314,   315,   316,     0,     0,     0,   317,   318,
     319,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   320,     0,     0,
      84,   321,     0,    86,    87,     0,    88,   183,    90,   322,
       0,   323,     0,     0,   324,     0,     0,     0,     0,     0,
       0,   325,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   261,   262,   326,   263,   264,     0,  1723,
     265,   266,   267,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,     0,   270,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,     0,     0,     0,
       0,     0,     0,   271,     0,   272,     0,     0,     0,     0,
    1069,     0,     0,     0,     0,     0,     0,   273,   274,   275,
     276,   277,   278,   279,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,    48,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,     0,     0,     0,     0,   313,   314,   315,   316,     0,
       0,     0,   317,   318,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,    84,   321,     0,    86,    87,     0,
      88,   183,    90,   322,     0,   323,     0,     0,   324,     0,
       0,     0,     0,     0,     0,   325,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   261,   262,   326,
     263,   264,     0,     0,   265,   266,   267,   268,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   271,     0,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   273,   274,   275,   276,   277,   278,   279,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,    48,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,     0,     0,     0,     0,     0,
     314,   315,   316,     0,     0,     0,   317,   318,   319,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,    84,   321,
       0,    86,    87,     0,    88,   183,    90,   322,     0,   323,
       0,     0,   324,     0,     0,     0,     0,     0,     0,   325,
    1463,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   261,   262,   326,   263,   264,     0,     0,   265,   266,
     267,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,   270,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   271,     0,   272,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   273,   274,   275,   276,   277,
     278,   279,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,    48,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,     0,
       0,     0,     0,     0,   314,   315,   316,     0,     0,    36,
     317,   318,   319,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
      48,     0,    84,   321,     0,    86,    87,     0,    88,   183,
      90,   322,     0,   323,     0,     0,   324,     0,     0,     0,
       0,     0,     0,   325,     0,  1485,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   326,  1555,  1556,
    1557,  1558,  1559,     0,     0,  1560,  1561,  1562,  1563,    86,
      87,     0,    88,   183,    90,     0,     0,     0,     0,     0,
       0,     0,  1564,  1565,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,  1566,     0,
       0,  1486,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1567,  1568,  1569,  1570,  1571,  1572,  1573,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,
    1582,  1583,  1584,    48,  1585,  1586,  1587,  1588,  1589,  1590,
    1591,  1592,  1593,  1594,  1595,  1596,  1597,  1598,  1599,  1600,
    1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,
    1611,  1612,  1613,  1614,     0,     0,     0,  1615,  1616,     0,
    1617,  1618,  1619,  1620,  1621,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1622,  1623,  1624,     0,
       0,     0,    86,    87,     0,    88,   183,    90,  1625,     0,
    1626,  1627,     0,  1628,     0,     0,     0,     0,     0,     0,
    1629,  1630,     0,  1631,     0,  1632,  1633,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   261,   262,     0,   263,   264,     0,     0,   265,   266,
     267,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   271,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   273,   274,   275,   276,   277,
     278,   279,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,    48,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,     0,
       0,     0,     0,   313,   314,   315,   316,     0,     0,     0,
     317,   552,   553,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,     0,   263,   264,   554,
       0,   265,   266,   267,   268,    86,    87,     0,    88,   183,
      90,   322,     0,   323,     0,     0,   324,     0,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   271,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   273,   274,
     275,   276,   277,   278,   279,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,    48,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,     0,     0,     0,     0,  1227,   314,   315,   316,
       0,     0,     0,   317,   552,   553,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   261,   262,     0,
     263,   264,   554,     0,   265,   266,   267,   268,    86,    87,
       0,    88,   183,    90,   322,     0,   323,     0,     0,   324,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   271,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   273,   274,   275,   276,   277,   278,   279,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,    48,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,     0,     0,     0,     0,     0,
     314,   315,   316,     0,     0,     0,   317,   552,   553,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   554,     0,     0,     0,     0,
       0,    86,    87,     0,    88,   183,    90,   322,     0,   323,
       0,     0,   324,     0,     0,     0,     0,     0,     0,     0,
       0,   427,   428,   429,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,     0,   452,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   453,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,     0,   452,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     453,     0,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,     0,   452,   427,   428,   429,     0,     0,     0,     0,
       0,     0,     0,     0,   453,     0,     0,     0,     0,     0,
       0,     0,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,     0,   452,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,   453,     0,
       0,     0,   454,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,     0,
     452,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   453,     0,     0,     0,   539,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,     0,   452,  1044,  1045,  1046,     0,     0,
       0,     0,     0,     0,     0,     0,   453,     0,     0,     0,
     541,     0,     0,     0,  1047,     0,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1069,     0,     0,     0,   559,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   270,
     452,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   453,     0,     0,     0,     0,     0,   583,     0,
       0,     0,     0,     0,     0,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   790,     0,     0,     0,     0,     0,     0,    48,     0,
       0,   272,     0,     0,     0,     0,  -398,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   417,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   542,   543,  1226,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   270,     0,
       0,   182,     0,     0,    84,   321,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1080,     0,   272,   325,     0,     0,     0,   542,
     543,   418,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,   182,     0,   326,
      84,   321,     0,    86,    87,     0,    88,   183,    90,     0,
    1026,     0,     0,     0,     0,     0,   270,    48,     0,     0,
       0,   325,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   272,     0,     0,   326,     0,     0,     0,     0,
       0,     0,   542,   543,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
     182,     0,     0,    84,   321,     0,    86,    87,     0,    88,
     183,    90,     0,  1359,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,   325,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   326,     0,
     542,   543,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,   182,   211,
       0,    84,   321,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,   325,     0,     0,     0,     0,  1235,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   819,   820,     0,   326,     0,     0,   821,
       0,   822,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   823,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,   752,     0,    86,    87,     0,
      88,   183,    90,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,   785,     0,   116,     0,     0,     0,     0,     0,
     824,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,   182,    82,
      83,    84,   825,     0,    86,    87,     0,    88,   183,    90,
       0,     0,  1005,    92,     0,     0,     0,     0,     0,     0,
       0,     0,   826,     0,     0,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    28,     0,     0,   827,     0,     0,     0,
       0,    33,    34,    35,    36,     0,   211,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,  1471,     0,     0,     0,     0,  1006,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    48,     0,     0,   215,     0,     0,     0,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   819,   820,     0,     0,  1472,     0,   821,     0,   822,
       0,     0,     0,     0,     0,     0,     0,     0,  1473,  1474,
       0,   823,     0,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,     0,     0,     0,   182,     0,     0,    84,
    1475,   212,    86,    87,     0,    88,  1476,    90,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,  1166,  1167,  1168,    36,     0,     0,     0,   824,     0,
      74,    75,    76,    77,    78,     0,     0,    36,     0,     0,
       0,   214,     0,     0,     0,    48,   182,    82,    83,    84,
     825,     0,    86,    87,     0,    88,   183,    90,    48,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
     826,   972,   973,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   974,     0,     0,   827,     0,     0,     0,     0,   975,
     976,   977,    36,     0,    86,    87,     0,    88,   183,    90,
       0,   978,     0,     0,     0,  1639,     0,    86,    87,  1640,
      88,   183,    90,    48,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   979,  1486,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   980,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     981,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    28,     0,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,     0,   211,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,    28,     0,   215,     0,     0,   596,     0,   116,
      33,    34,    35,    36,     0,   211,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   616,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,    28,   215,   961,     0,     0,     0,
     116,     0,    33,    34,    35,    36,     0,   211,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,    28,     0,   215,     0,     0,
       0,     0,   116,    33,    34,    35,    36,     0,   211,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1105,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,    28,     0,   215,     0,
       0,     0,     0,   116,    33,    34,    35,    36,     0,   211,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,    48,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   215,
      33,    34,    35,    36,   116,   211,    86,    87,     0,    88,
     183,    90,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   229,     0,     0,   683,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   214,    48,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   230,    33,    34,    35,    36,
     116,   211,    86,    87,     0,    88,   183,    90,   639,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   213,     0,     0,   941,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   427,   428,
     429,   640,     0,     0,     0,     0,   641,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
       0,   452,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,   453,     0,     0,     0,     0,     0,     0,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,     0,   452,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   453,     0,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   430,   501,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,     0,   452,   427,   428,   429,     0,     0,     0,     0,
       0,     0,     0,     0,   453,     0,     0,     0,     0,     0,
       0,     0,   430,   510,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,     0,   452,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   453,     0,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   430,
     947,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,     0,   452,  1044,  1045,  1046,     0,     0,     0,
       0,     0,     0,     0,     0,   453,     0,     0,     0,     0,
       0,     0,     0,  1047,   991,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1069,
       0,  1044,  1045,  1046,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1047,  1312,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,     0,     0,  1044,  1045,  1046,     0,     0,
       0,     0,     0,     0,     0,     0,  1069,     0,     0,     0,
       0,     0,     0,     0,  1047,  1217,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
    1069,     0,  1044,  1045,  1046,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,  1047,  1369,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1069,     0,     0,
       0,    36,     0,     0,     0,    48,  1451,     0,     0,     0,
     182,     0,     0,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    48,     0,     0,     0,     0,  1472,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1473,  1474,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,     0,   182,     0,
       0,    84,    85,  1537,    86,    87,     0,    88,  1476,    90,
       0,     0,     0,     0,     0,     0,    48,     0,     0,   414,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,   584,     0,     0,    86,    87,     0,    88,   183,
      90,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,     0,     0,
     588,    48,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,     0,     0,     0,     0,     0,   346,     0,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   427,   428,   429,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   802,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,     0,   452,     0,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   430,   944,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   803,   452,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,   453,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,     0,   452,  1044,  1045,  1046,     0,     0,     0,
       0,     0,     0,     0,     0,   453,     0,     0,     0,     0,
       0,     0,     0,  1047,  1374,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1044,  1045,  1046,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1069,
       0,     0,     0,     0,     0,  1047,     0,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1069
};

static const yytype_int16 yycheck[] =
{
       5,     6,   135,     8,     9,    10,    11,    12,   334,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   160,   680,    28,    29,    91,    32,   185,   495,    95,
      96,     4,     4,     4,     4,   385,    55,    42,    44,   731,
     489,   490,   651,    49,     4,    50,   920,    52,   525,  1129,
      55,   165,    57,   186,    30,    58,    54,   468,   452,   485,
     113,    30,  1125,   652,   801,   160,   174,  1114,   134,     4,
     519,   632,   231,   113,    79,   227,   485,   113,    81,   808,
      30,    84,   940,   909,  1004,     9,     9,   770,     9,    30,
       9,    14,     9,   240,    45,   521,     9,     9,   956,     9,
       9,     4,    14,   580,    14,     9,    45,     9,   113,     9,
      50,     9,   521,     9,     9,    30,    10,    11,    12,     4,
       9,     9,     9,     9,    79,     9,    79,   241,     9,    66,
       9,     9,     9,   459,     9,    29,     9,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    33,    53,
      45,     9,   215,   110,     9,    66,   156,     9,     9,     9,
    1668,    65,   154,    45,    45,   215,   653,   230,    45,   215,
      35,    97,   508,    14,   150,   190,    29,    79,    46,    47,
     230,    26,    27,   129,   130,   156,    66,    79,     0,    30,
     135,     8,    66,    46,   199,   171,    49,    97,   169,   164,
     215,   164,   202,    66,  1040,   155,    47,    71,    72,   201,
     202,   168,   107,   370,    79,   230,  1724,   112,    66,   114,
     115,   116,   117,   118,   119,   120,    66,   242,   154,    66,
     245,   202,    66,    66,  1307,    79,   197,   252,   253,   200,
      66,   186,   202,     4,    26,    27,    66,   160,   197,   171,
     201,   419,  1309,   199,   154,   202,    66,    66,   150,  1316,
      66,  1318,   157,   158,   172,   160,   200,   385,   201,  1199,
     201,   202,   201,    66,   201,  1014,   246,  1016,   201,   201,
     250,   201,   201,   172,   199,   172,   181,   201,   199,   201,
      51,   201,  1160,    54,   196,   201,   201,   101,   102,   362,
     202,   200,   206,   201,   201,   201,   793,   201,   203,   200,
      71,   798,   362,   201,   172,   200,   362,   200,   200,   200,
     129,   130,   202,   200,   467,   199,   334,   898,    89,   497,
      91,   344,   345,   346,    95,    96,   199,   413,   937,   202,
     150,   459,   504,  1179,  1417,   200,  1419,   362,   200,   200,
     200,   199,    66,   368,   202,    66,    66,    35,   373,   199,
     204,    79,   202,   171,   377,   199,   117,   485,   202,   202,
      42,  1428,    53,   134,   125,   199,   202,    66,    79,   224,
      79,   199,   202,   398,    65,    79,   462,   463,   464,   465,
     508,   199,   202,   202,   409,   202,   202,   118,   150,   203,
     518,    79,   410,   521,   125,   199,    35,    79,   885,   202,
    1112,   391,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   198,   453,   468,
     455,   456,   224,   458,  1517,   199,   164,   199,  1195,   199,
      79,   459,   397,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   452,    50,   171,    66,
     164,   486,   487,   452,   489,   490,   491,   492,  1383,    35,
     199,   643,    35,   498,   397,   246,   501,  1180,   202,   250,
     926,   202,   202,   254,   953,   510,   199,   512,   527,    35,
     508,   658,   659,   204,   519,   204,   351,   926,    85,  1128,
     199,   200,   527,   171,   529,   360,   361,   362,   150,   668,
      85,   466,   367,    79,  1011,    79,    79,   452,   171,   374,
    1366,    85,   129,   130,   459,   164,    79,   101,   102,   129,
     130,   199,    85,    79,   199,  1144,    97,   562,  1147,   909,
     565,   566,   567,   150,   199,   724,   199,   961,   720,    97,
     485,   199,   532,   668,  1469,   733,   199,   199,    79,   351,
     147,   584,   155,   334,    85,   588,    97,   640,   360,   361,
     593,   596,   147,   508,    66,   367,   155,   170,  1493,   758,
    1495,    79,   374,   518,   148,   149,   521,    85,  1671,   199,
    1436,    79,   199,   154,    79,   148,   149,     4,   164,   202,
     686,   164,    30,   203,    79,  1317,   154,   774,   775,   154,
      85,    79,    35,   780,   781,   640,   199,    85,   164,    26,
      27,    97,  1325,   154,  1327,   201,   397,   148,   149,   203,
      79,   150,    49,    50,    51,   406,    85,   129,   130,   410,
     495,     4,   413,   815,   208,   201,   780,   171,    65,   828,
     148,   149,   171,   678,   207,    66,   150,   836,   201,   147,
     148,   149,   652,   148,   149,   690,   456,  1296,   129,   130,
    1040,   201,   147,   148,   149,   199,  1107,   171,   154,   147,
     148,   149,    45,   202,  1153,   201,  1398,    29,   459,   460,
     461,   462,   463,   464,   465,  1164,   486,   722,   201,   148,
     149,   491,  1311,   495,    46,   199,   113,    49,   202,   734,
    1810,  1198,    71,    72,   485,   114,   115,   116,   117,   118,
     119,  1804,   100,   101,   102,  1825,   127,   128,   753,   201,
     106,  1434,   100,   101,   102,    66,  1819,   508,   114,   115,
     116,   117,   118,   119,   107,   668,   117,   118,   119,   112,
     521,   114,   115,   116,   117,   118,   119,   120,  1470,   202,
     785,   532,   150,   114,   115,   116,   117,   118,   119,   199,
    1267,   797,    97,    98,   125,   126,   801,   114,   115,   116,
     551,   909,   181,   199,   912,   114,   115,   116,   117,   118,
     119,  1278,   201,   202,   157,   158,   924,   160,   926,    49,
      50,    51,   201,    53,    66,   181,   201,   202,   215,  1179,
     161,   150,   163,   838,   986,    65,   199,   224,   181,   154,
    1429,  1524,   677,   230,   595,   176,   806,   178,  1698,  1699,
     181,  1694,  1695,    44,   857,   201,   791,    65,   861,   246,
     203,  1338,   150,   250,  1313,   371,   199,   171,   206,   375,
     621,   622,   181,   817,   818,   199,     9,  1354,  1030,   182,
     183,   184,   150,   150,   992,  1037,   189,   190,   791,   894,
     193,   194,   199,   399,   960,   401,   402,   403,   404,     8,
     735,   201,   199,   908,  1778,   677,   150,    14,    79,   150,
     201,   201,   125,   125,   912,    14,   200,   852,   171,    14,
      97,   200,   200,  1797,   897,   897,   897,   897,   200,   934,
     205,  1805,  1040,   199,   106,   686,   199,   897,   199,   944,
       9,   776,   947,   778,   949,   147,   200,    89,   953,   852,
     920,   200,    46,    47,    48,    49,    50,    51,     9,    53,
     200,  1438,   897,   735,   351,   200,  1415,   937,   803,   201,
    1447,    65,    14,   360,   361,   362,  1135,   171,   199,     9,
     367,  1458,   185,   918,   188,   961,   991,   374,    79,    79,
    1673,   997,   961,    79,   897,   201,     9,   912,    46,    47,
      48,    49,    50,    51,   776,   201,   778,   199,     9,   924,
     397,   926,    79,   200,   200,   918,  1366,    65,   200,   770,
     201,   772,  1174,   127,   199,    66,   200,    30,   128,   170,
     150,   803,     9,   131,   200,   870,   150,   197,   998,    14,
     791,  1039,     9,     9,  1042,   172,   961,   200,     9,    14,
     885,   886,   127,  1530,   805,   806,   206,   206,   203,     9,
    1718,    14,   206,   200,   200,  1000,   206,  1002,  1073,  1074,
    1075,  1179,  1224,   199,  1079,  1080,   150,   992,   200,  1231,
     114,   115,   116,   117,   118,   119,  1436,    97,  1107,   201,
     201,   125,   126,    74,    75,    76,    86,  1000,   870,  1002,
     131,   852,  1107,     4,   150,    86,     9,   858,   495,   202,
     199,   862,   863,   885,   886,   200,   199,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   163,
      53,   882,   150,   150,    14,  1140,   185,  1795,  1111,  1111,
    1111,  1111,    65,     9,    45,   532,   897,   181,  1153,   185,
      79,  1111,   202,    14,   135,   136,   137,   138,   139,  1164,
    1165,   912,   201,    14,   202,   146,   206,   918,    14,  1004,
    1005,   152,   153,   924,  1144,   926,  1111,  1147,   202,   200,
      30,  1187,  1334,   199,   199,   166,  1663,   201,  1665,   197,
    1195,    30,    14,   199,   199,    14,    48,  1674,     9,   180,
    1205,   199,   199,   131,   200,    14,   107,   201,  1111,   960,
     201,   112,   199,   114,   115,   116,   117,   118,   119,   120,
     131,   972,   973,   974,   200,     9,   206,    65,     9,     9,
    1190,    79,  1004,  1005,   131,   199,    14,    79,   201,   201,
     200,   992,   199,   202,  1721,   199,   202,   998,   200,  1000,
     131,  1002,     9,   202,    86,   206,   157,   158,  1366,   160,
    1399,  1394,   147,    30,   201,    73,   200,  1202,   201,    54,
     172,  1022,   131,   200,    30,   200,   131,     9,   200,   203,
     181,     9,   200,  1288,   203,   202,    79,  1292,  1039,  1294,
     677,  1042,    14,   199,   201,   200,   200,  1302,   131,  1202,
     200,   199,   203,   202,  1139,   200,  1276,  1312,  1313,   200,
     200,     9,    30,   201,   200,  1285,   200,   107,   201,  1070,
      26,    27,  1648,   201,    30,   202,   201,   155,  1436,   159,
      14,    79,   200,  1491,   131,   112,  1087,   200,   202,  1090,
     200,  1311,   131,    14,   171,   202,  1823,   201,   735,  1184,
      79,    14,    14,  1830,    79,   200,   199,   131,   202,   200,
    1111,   201,    14,  1198,  1199,   201,    14,  1139,    14,     9,
    1375,   202,   201,  1276,    55,    79,   171,   203,   199,    79,
       9,   201,  1285,    79,   110,    97,   150,    97,    33,   776,
     162,   778,    14,   199,   201,   200,   199,  1463,   168,   172,
      79,   200,   165,     9,   791,    79,   200,  1412,   593,   201,
    1415,   202,  1184,   200,    14,    79,   803,    14,    79,   806,
      14,    79,    14,    79,   857,  1176,  1198,  1199,  1786,  1180,
     861,   460,   465,   954,   462,   959,   900,  1801,  1196,  1190,
    1529,  1797,  1377,  1278,  1351,  1520,   598,  1553,  1418,  1471,
    1637,  1202,  1829,  1649,  1424,  1389,  1426,  1817,  1393,  1429,
    1385,  1516,  1082,   569,   468,   852,  1079,     4,  1466,   849,
    1162,  1115,   569,  1038,  1377,   973,  1175,   924,  1176,   988,
     410,   368,   361,   870,  1444,   270,  1753,   272,   817,  1098,
      -1,  1023,  1070,    -1,    -1,  1653,  1399,    -1,   885,   886,
      -1,    -1,  1378,  1508,  1439,    -1,  1278,    -1,    45,    -1,
     897,    -1,    -1,  1448,    -1,  1418,    -1,    -1,   224,    -1,
    1271,  1424,    -1,  1426,    -1,    26,    27,    -1,    -1,    30,
      -1,   918,    -1,  1538,    -1,    -1,  1439,  1689,    -1,    -1,
      -1,   326,    -1,    -1,    -1,  1448,    -1,    -1,    -1,    -1,
      -1,    -1,  1522,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1499,    -1,    -1,    -1,  1528,  1529,
     107,    -1,    -1,    -1,  1325,   112,  1327,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1527,    77,    -1,    -1,    -1,    -1,  1533,    -1,
      -1,    -1,    -1,    -1,  1539,    -1,    -1,    -1,    -1,    -1,
      -1,   998,    -1,  1000,    98,  1002,    -1,  1004,  1005,  1522,
     157,   158,   407,   160,  1527,   410,  1377,    -1,    -1,    -1,
    1533,    -1,  1383,  1772,    -1,    -1,  1539,    -1,  1389,    -1,
      -1,    -1,    -1,    -1,   181,   351,  1712,    -1,    -1,    -1,
    1648,    -1,    -1,  1658,   360,   361,    -1,  1790,    -1,    -1,
      -1,   367,    -1,    -1,    -1,    -1,   203,   151,   374,    -1,
     154,    -1,    -1,   157,   158,    -1,   160,   161,   162,   385,
      -1,    -1,    -1,  1434,    -1,    -1,    -1,    -1,  1439,    -1,
      -1,    -1,    -1,  1444,    -1,    -1,    -1,  1448,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,  1463,    -1,    -1,  1466,    -1,    -1,  1469,   203,
      -1,    -1,    -1,   224,  1111,    -1,    -1,    -1,  1479,    -1,
      -1,    -1,    -1,    -1,    -1,  1486,    -1,    -1,    -1,    -1,
      -1,    -1,  1493,  1678,  1495,    -1,   452,    -1,    -1,    -1,
    1501,    -1,  1139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   546,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   270,
      -1,   272,    -1,  1524,    -1,  1678,  1527,  1528,  1529,    -1,
      -1,    -1,  1533,    -1,  1719,  1720,    -1,    -1,  1539,   495,
      -1,  1726,    -1,    -1,    -1,    -1,    -1,  1184,    -1,    -1,
      -1,    -1,    -1,  1190,    -1,    -1,    -1,    -1,  1778,    -1,
      -1,  1198,  1199,    -1,    -1,  1202,  1719,  1720,    -1,    -1,
      -1,    -1,    -1,  1726,    -1,   326,    -1,  1797,    -1,  1764,
    1835,    -1,    -1,    -1,    -1,  1805,    -1,    -1,  1843,    -1,
      -1,    -1,     4,    -1,  1849,    -1,    -1,  1852,    -1,    -1,
     351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   360,
     361,  1764,    -1,   648,   649,    -1,   367,    -1,    -1,  1772,
       4,    -1,   657,   374,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,   385,    -1,    -1,    -1,    -1,    -1,
      -1,  1278,    -1,    -1,    -1,    -1,  1831,  1648,    -1,    -1,
      -1,    -1,    -1,  1838,    -1,    -1,   407,    -1,    -1,   410,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,  1669,    -1,
      -1,    -1,  1673,    -1,    -1,    -1,    -1,  1678,  1831,    -1,
      -1,    -1,    -1,    -1,    -1,  1838,  1687,    -1,    -1,    -1,
      -1,    -1,    -1,  1694,  1695,   107,    -1,  1698,  1699,    -1,
     112,   452,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,  1712,    -1,    -1,    -1,    -1,    -1,    -1,  1719,  1720,
      -1,   677,    -1,   107,    -1,  1726,    -1,    -1,   112,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
    1377,    -1,    -1,    -1,   495,   157,   158,    -1,   160,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,  1764,    -1,    -1,    -1,    -1,    -1,   181,
    1771,    -1,    -1,   157,   158,    -1,   160,    -1,    -1,   735,
      -1,    98,   817,   818,    -1,    -1,    -1,    -1,  1789,    -1,
      -1,   203,    -1,    -1,    -1,   546,   547,   181,    -1,   550,
      -1,    -1,  1439,    -1,    26,    27,    -1,  1444,    30,    -1,
      -1,  1448,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,
     776,    -1,   778,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1831,    -1,    77,    -1,    -1,    -1,    -1,  1838,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,   803,    -1,    -1,
      -1,   602,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,    -1,   899,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
     915,    -1,    -1,    -1,    -1,   202,    -1,   204,    -1,    -1,
    1527,  1528,  1529,   928,    -1,    -1,  1533,   648,   649,    -1,
      -1,    -1,  1539,    -1,    -1,    -1,   657,    -1,    -1,   154,
      -1,    -1,   157,   158,   870,   160,   161,   162,    -1,    -1,
      -1,    -1,   957,    -1,    -1,    -1,   677,    -1,    -1,   885,
     886,    -1,    -1,    10,    11,    12,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    29,   909,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,  1018,   735,    -1,  1021,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   224,    -1,    -1,   961,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,   776,    -1,   778,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,  1678,    -1,    -1,    -1,    -1,    -1,    -1,  1004,  1005,
      -1,    -1,   803,   804,    -1,    -1,    63,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   817,   818,   819,   820,
     821,   822,   823,    -1,    -1,    -1,   827,    63,    64,    -1,
      -1,    -1,  1719,  1720,  1040,    -1,    -1,    -1,    -1,  1726,
      -1,    -1,  1127,    -1,  1129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   853,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,   870,
      -1,  1156,   129,   130,  1159,    -1,   203,  1764,    -1,   351,
      -1,    -1,   883,    -1,   885,   886,    -1,    -1,   360,   361,
      -1,    -1,    -1,   129,   130,   367,    -1,    -1,   899,   900,
      -1,    -1,   374,    63,    64,    -1,    -1,    -1,   909,    -1,
      -1,    -1,    -1,   385,   915,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1210,    -1,   928,    -1,  1214,
      -1,    -1,    -1,  1139,    -1,   936,    -1,    -1,   939,    -1,
      -1,    -1,    -1,   200,  1831,    -1,    -1,    -1,    -1,    -1,
      -1,  1838,    -1,    -1,    -1,    -1,   957,    -1,    -1,    -1,
     961,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,   129,
     130,    -1,    -1,  1179,    -1,    -1,    -1,    -1,  1184,    -1,
     452,    -1,    -1,    -1,    -1,    -1,    -1,  1272,  1273,    -1,
      -1,    -1,  1198,  1199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1004,  1005,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1018,    -1,    -1,
    1021,    -1,  1023,   495,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,  1040,
     200,    -1,  1043,  1044,  1045,  1046,  1047,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,    -1,
      -1,    -1,  1278,  1358,    -1,  1360,    -1,    -1,   550,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,  1095,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,  1394,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    -1,    -1,    -1,  1127,    -1,  1129,    -1,
     602,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1139,    -1,
      77,    -1,    -1,    -1,    63,    64,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,  1159,    -1,
    1366,    98,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1179,    53,
      -1,    -1,    -1,  1184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,  1198,  1199,    -1,
    1201,    -1,    -1,    -1,    -1,   677,    -1,    -1,    -1,  1210,
     129,   130,    -1,  1214,   151,    -1,  1217,   154,  1219,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
    1436,    -1,    -1,    -1,  1235,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    10,
      11,    12,    -1,   735,    -1,    -1,  1551,    -1,    -1,    -1,
      -1,  1272,  1273,    -1,  1275,    -1,    -1,  1278,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,   776,    -1,   778,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   803,   804,    -1,    -1,    -1,    -1,    -1,   270,    -1,
     272,    -1,    -1,    -1,    65,    -1,    -1,   819,   820,   821,
     822,   823,    -1,    -1,    -1,   827,    -1,  1358,    -1,  1360,
      -1,    -1,    -1,    -1,  1365,  1366,    -1,  1652,  1369,    -1,
    1371,    -1,    -1,  1374,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   853,  1383,  1384,    -1,    -1,  1387,    -1,    -1,    -1,
      -1,    -1,    -1,  1394,   326,    -1,    -1,    -1,   870,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   883,    -1,   885,   886,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   550,    -1,    -1,   900,    -1,
      -1,    -1,    -1,    -1,    -1,  1436,    -1,   909,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1451,  1736,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1461,  1462,    -1,    -1,   936,    -1,    -1,   939,  1469,    -1,
    1471,    -1,    -1,    -1,    -1,   407,    -1,   602,   410,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,   961,
      -1,    -1,  1493,    -1,  1495,    -1,    -1,    -1,    -1,    -1,
    1501,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1810,    53,    -1,    -1,    -1,
      -1,    -1,  1004,  1005,    -1,  1536,  1537,    -1,    65,    -1,
    1825,    -1,  1543,    -1,  1545,    -1,    -1,    -1,    -1,    -1,
    1551,    -1,  1553,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,  1040,    -1,
      -1,  1043,  1044,  1045,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   546,   547,    -1,    -1,   550,    -1,
      -1,    -1,    29,  1095,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,  1652,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1139,  1669,    -1,
     602,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,   804,
      -1,    -1,    -1,    -1,    -1,    -1,  1687,    -1,    -1,    -1,
      -1,    -1,  1693,    -1,   819,   820,   821,   822,   823,    -1,
      -1,    -1,   827,  1704,    -1,    -1,    -1,  1179,    -1,  1710,
      -1,    -1,  1184,  1714,    -1,    -1,   648,   649,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   657,  1198,  1199,    -1,  1201,
      -1,    -1,    -1,    -1,    -1,  1736,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1217,    -1,  1219,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,  1235,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1776,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1785,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    63,    64,    -1,
    1801,    -1,    -1,  1275,    -1,    -1,  1278,    -1,    -1,  1810,
      -1,   936,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,  1825,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    27,    -1,    -1,    30,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   804,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   817,   818,   819,   820,   821,
     822,   823,    -1,  1365,  1366,   827,    -1,  1369,    -1,  1371,
      -1,    -1,  1374,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1384,  1038,    -1,  1387,    -1,    -1,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1436,    53,    -1,   899,    -1,    -1,
    1095,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,  1451,
      -1,    -1,    -1,   915,    -1,    -1,    -1,    -1,    -1,  1461,
    1462,    -1,    -1,   550,    -1,    -1,   928,    -1,    -1,  1471,
      -1,    -1,    -1,    -1,   936,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   957,    -1,    -1,    -1,   224,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   602,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1536,  1537,    -1,    -1,    -1,    -1,
      -1,  1543,    -1,  1545,    -1,    -1,  1201,    65,    -1,    -1,
      -1,  1553,    -1,    -1,    -1,    -1,  1018,    -1,    -1,  1021,
      -1,  1023,  1217,    -1,  1219,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,    -1,    -1,
    1235,  1043,  1044,  1045,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   351,    53,    -1,    -1,
      -1,    -1,    -1,  1095,    -1,   360,   361,    -1,    -1,    65,
      -1,    -1,   367,    -1,    -1,    -1,    -1,    -1,    -1,   374,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     385,    -1,    -1,    -1,    -1,  1127,    -1,  1129,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1693,    53,    -1,  1156,    -1,    -1,  1159,    -1,    -1,
      -1,    -1,  1704,    -1,    65,    -1,    -1,    -1,  1710,    -1,
    1365,    -1,  1714,    -1,  1369,    -1,  1371,   804,    -1,  1374,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   452,    -1,    -1,
      -1,    -1,   819,   820,   821,   822,    -1,    -1,    -1,  1201,
     827,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1210,    -1,
      -1,    -1,  1214,    -1,    -1,  1217,    -1,  1219,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     495,    -1,    -1,  1235,  1776,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1785,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1451,    -1,    -1,  1801,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
    1272,  1273,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   550,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   936,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   602,    53,    -1,
      -1,  1536,  1537,    -1,    -1,    -1,    -1,    -1,  1543,    -1,
      65,    -1,    -1,    -1,    -1,    -1,  1358,    -1,  1360,    -1,
      -1,    -1,    -1,  1365,    -1,    -1,    -1,  1369,    -1,  1371,
      77,    -1,  1374,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,  1394,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   677,    -1,    -1,    -1,  1043,  1044,  1045,  1046,
    1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,    -1,    -1,    -1,    -1,   154,    -1,  1451,
     157,   158,    -1,   160,   161,   162,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1469,  1095,    -1,
     735,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,  1493,    -1,  1495,    -1,   202,    -1,    -1,  1693,  1501,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,
      -1,   776,    -1,   778,    -1,  1710,    -1,    -1,    -1,  1714,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1536,  1537,    -1,    -1,   803,   804,
      -1,  1543,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1551,
      -1,    -1,    -1,    -1,   819,   820,   821,   822,   823,    -1,
      -1,    29,   827,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1201,    -1,    -1,    -1,    -1,    -1,
      -1,  1776,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
    1217,    -1,  1219,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   870,    -1,    -1,  1235,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     885,   886,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,   909,    -1,    -1,    -1,    -1,    -1,
    1652,    -1,    77,    -1,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,  1669,    -1,    -1,
      -1,   936,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,  1687,   154,   155,    -1,   157,
     158,  1693,   160,   161,   162,    -1,   961,    -1,    -1,    -1,
      -1,    -1,  1704,    -1,    -1,    -1,    -1,   175,  1710,    -1,
      -1,    -1,  1714,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
      -1,   199,   157,   158,  1736,   160,   161,   162,  1365,  1004,
    1005,    -1,  1369,    -1,  1371,    -1,    -1,  1374,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,  1038,  1776,  1040,    -1,    -1,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,    -1,    -1,    -1,  1810,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1825,  1451,    -1,    -1,    -1,    -1,    29,
    1095,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1139,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1536,
    1537,    65,    -1,    -1,  1179,    -1,  1543,    -1,    -1,  1184,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1198,  1199,    -1,  1201,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1217,    -1,  1219,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1235,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,  1278,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1693,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,    -1,    -1,
      -1,    -1,    -1,  1710,    -1,    -1,    -1,  1714,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
    1365,  1366,    -1,    -1,  1369,    -1,  1371,    -1,    -1,  1374,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   203,  1776,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1436,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1451,    -1,    -1,    -1,
      -1,    -1,   203,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,  1536,  1537,    -1,    -1,    -1,    -1,    -1,  1543,   132,
     133,   200,   135,   136,   137,   138,   139,  1552,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,    -1,   207,   208,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1693,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,  1704,
      -1,    -1,    -1,    -1,    13,  1710,    -1,    -1,    -1,  1714,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1738,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,  1776,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   201,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,   178,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
     163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,   176,    -1,   178,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
     107,   108,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,   118,   119,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,   163,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,   176,
      -1,   178,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,    -1,
     121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,   203,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,   121,   122,   123,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,   163,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,   203,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,   111,    -1,   113,   114,   115,   116,   117,   118,
     119,    -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
     163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,    -1,   121,   122,   123,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,   163,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    96,    -1,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,    -1,
     121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,   121,   122,   123,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,   163,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,   203,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,   111,    -1,   113,   114,   115,   116,   117,   118,
     119,    -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    94,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
     163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,    -1,   121,   122,   123,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,   163,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,   203,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,    -1,
     121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,   203,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    92,    93,    -1,
      95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,   121,   122,   123,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,   163,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,   111,    -1,   113,   114,   115,   116,   117,   118,
     119,    -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
     163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,    -1,   121,   122,   123,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,   163,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,   203,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,    -1,
     121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,   121,   122,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,   111,    -1,   113,   114,   115,   116,   117,   118,
     119,    -1,   121,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,    -1,   121,   122,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,    -1,
     121,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,    -1,    -1,   122,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,    -1,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,   164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    11,    12,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,    -1,
      -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,    -1,    -1,   122,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,    10,    11,    12,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    65,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,   103,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,   191,   192,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,    12,    -1,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,    10,    11,
      12,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,   188,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,    -1,    12,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,    -1,
      -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,    -1,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,    -1,    -1,   122,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,   200,    -1,    -1,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,    -1,
      -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,    -1,    -1,   122,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,    10,    11,    12,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    65,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
     169,    -1,    -1,   187,    -1,    -1,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,    10,    11,    12,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,   186,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,   205,    -1,   207,   208,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
     127,   128,    -1,    -1,    -1,   132,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,   151,    -1,    10,    11,    12,    13,
     157,   158,    -1,   160,   161,   162,   163,    -1,   165,    -1,
      -1,   168,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    53,
      -1,    -1,    -1,    -1,    -1,   202,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,   127,   128,    -1,    -1,    -1,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,   151,    -1,    10,
      11,    12,    13,   157,   158,    -1,   160,   161,   162,   163,
      -1,   165,    -1,    -1,   168,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    53,    -1,    -1,    -1,    -1,    -1,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,   127,   128,    -1,    -1,
      -1,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
     151,    -1,    10,    11,    12,    13,   157,   158,    -1,   160,
     161,   162,   163,    -1,   165,    -1,    -1,   168,    -1,    27,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,    -1,    -1,    -1,   132,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,     3,
       4,   199,     6,     7,    -1,   203,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    53,
      -1,    55,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,    -1,   126,   127,   128,    -1,    -1,    -1,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,     3,     4,   199,     6,     7,    -1,   203,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,    -1,    -1,   125,   126,   127,   128,    -1,
      -1,    -1,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,     3,     4,   199,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,    -1,    -1,    -1,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,   175,
     176,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,     3,     4,   199,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,    -1,   126,   127,   128,    -1,    -1,    77,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      98,    -1,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,    -1,   175,    -1,   123,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,    -1,   199,     3,     4,
       5,     6,     7,    -1,    -1,    10,    11,    12,    13,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    53,    -1,
      -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   151,   152,   153,    -1,
      -1,    -1,   157,   158,    -1,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,
     175,   176,    -1,   178,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,   127,   128,    -1,    -1,    -1,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,   151,
      -1,    10,    11,    12,    13,   157,   158,    -1,   160,   161,
     162,   163,    -1,   165,    -1,    -1,   168,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,   127,   128,
      -1,    -1,    -1,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,   151,    -1,    10,    11,    12,    13,   157,   158,
      -1,   160,   161,   162,   163,    -1,   165,    -1,    -1,   168,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,    -1,    -1,    -1,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,   201,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    55,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,   134,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    29,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    55,   175,    -1,    -1,    -1,   133,
     134,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,   151,    -1,   199,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
     164,    -1,    -1,    -1,    -1,    -1,    29,    98,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    55,    -1,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,   164,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,   199,    -1,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,   151,    79,
      -1,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    30,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    46,    47,    -1,   199,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,   155,    -1,   157,   158,    -1,
     160,   161,   162,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,    -1,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    35,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    66,    -1,    -1,   199,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    98,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,   120,    -1,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,   151,    -1,    -1,   154,
     155,    86,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    74,    75,    76,    77,    -1,    -1,    -1,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    77,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    98,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    98,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     175,    46,    47,    -1,    -1,   180,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    66,    -1,    -1,   199,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    86,    -1,    -1,    -1,   155,    -1,   157,   158,   159,
     160,   161,   162,    98,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   133,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    66,    -1,   199,    -1,    -1,   202,    -1,   204,
      74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    66,   199,    68,    -1,    -1,    -1,
     204,    -1,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    66,    -1,   199,    -1,    -1,
      -1,    -1,   204,    74,    75,    76,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    66,    -1,   199,    -1,
      -1,    -1,    -1,   204,    74,    75,    76,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    98,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,   199,
      74,    75,    76,    77,   204,    79,   157,   158,    -1,   160,
     161,   162,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   119,    -1,    -1,   199,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    98,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,    74,    75,    76,    77,
     204,    79,   157,   158,    -1,   160,   161,   162,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   119,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    10,    11,
      12,   199,    -1,    -1,    -1,    -1,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   131,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     131,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   131,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   131,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   131,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      65,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    29,   131,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    98,   131,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    98,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    77,    -1,    -1,   151,    -1,
      -1,   154,   155,   131,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    77,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,   157,   158,    -1,   160,   161,
     162,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    98,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    28,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    97,    53,
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
      -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   210,   211,     0,   212,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      82,    86,    87,    88,    89,    91,    93,    95,    98,    99,
     103,   104,   105,   106,   107,   108,   109,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   121,   122,   123,   124,
     125,   126,   132,   133,   135,   136,   137,   138,   139,   143,
     146,   151,   152,   153,   154,   155,   157,   158,   160,   161,
     162,   163,   166,   169,   175,   176,   178,   180,   181,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   199,   201,   202,   204,   205,   207,   208,
     213,   216,   225,   226,   227,   228,   229,   230,   233,   249,
     250,   254,   257,   262,   268,   328,   329,   337,   341,   342,
     343,   344,   345,   346,   347,   348,   350,   353,   365,   366,
     367,   369,   370,   372,   391,   401,   402,   403,   408,   411,
     430,   438,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   453,   466,   468,   470,   117,   118,
     119,   132,   151,   161,   216,   249,   328,   347,   442,   347,
     199,   347,   347,   347,   103,   347,   347,   428,   429,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,    79,    86,   119,   146,   199,   226,   366,   402,   403,
     408,   442,   445,   442,    35,   347,   457,   458,   347,   119,
     199,   226,   402,   403,   404,   408,   439,   440,   441,   449,
     454,   455,   199,   338,   405,   199,   338,   354,   339,   347,
     235,   338,   199,   199,   199,   338,   201,   347,   216,   201,
     347,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    53,    55,    67,    68,    69,    70,    71,    72,    73,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   125,   126,   127,   128,   132,   133,   134,
     151,   155,   163,   165,   168,   175,   199,   216,   217,   218,
     229,   471,   486,   487,   489,   182,   201,   344,   347,   371,
     373,   202,   242,   347,   106,   107,   154,   219,   222,   225,
      79,   204,   294,   295,   118,   125,   117,   125,    79,   296,
     199,   199,   199,   199,   216,   266,   474,   199,   199,   339,
      79,    85,   147,   148,   149,   463,   464,   154,   202,   225,
     225,   216,   267,   474,   155,   199,   474,   474,    79,   196,
     202,   356,   337,   347,   348,   442,   446,   231,   202,    85,
     406,   463,    85,   463,   463,    30,   154,   171,   475,   199,
       9,   201,    35,   248,   155,   265,   474,   119,   181,   249,
     329,   201,   201,   201,   201,   201,   201,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    53,    65,   201,    66,    66,   201,   202,   150,
     126,   161,   163,   176,   178,   268,   327,   328,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    63,    64,   129,   130,   432,    66,   202,   437,   199,
     199,    66,   202,   204,   450,   199,   248,   249,    14,   347,
     201,   131,    44,   216,   427,    85,   337,   348,   150,   442,
     131,   206,     9,   413,   337,   348,   442,   475,   150,   199,
     407,   432,   437,   200,   347,    30,   233,     8,   359,     9,
     201,   233,   234,   339,   340,   347,   216,   280,   237,   201,
     201,   201,   133,   134,   489,   489,   171,   199,   106,   489,
      14,   150,   133,   134,   151,   216,   218,    79,   201,   201,
     201,   182,   183,   184,   189,   190,   193,   194,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   386,   387,   388,
     243,   110,   168,   201,   154,   220,   223,   225,   154,   221,
     224,   225,   225,     9,   201,    97,   202,   442,     9,   201,
     125,   125,    14,     9,   201,   442,   467,   467,   337,   348,
     442,   445,   446,   200,   171,   260,   132,   442,   456,   457,
     201,    66,   432,   147,   464,    78,   347,   442,    85,   147,
     464,   225,   215,   201,   202,   255,   263,   392,   394,    86,
     199,   204,   360,   361,   363,   403,   450,   468,    14,    97,
     469,   355,   357,   358,   290,   291,   430,   431,   200,   200,
     200,   200,   203,   232,   233,   250,   257,   262,   430,   347,
     205,   207,   208,   216,   476,   477,   489,    35,   164,   292,
     293,   347,   471,   199,   474,   258,   248,   347,   347,   347,
      30,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   404,   347,   347,   452,   452,   347,   459,
     460,   125,   202,   217,   218,   449,   450,   266,   216,   267,
     474,   474,   265,   249,    27,    35,   341,   344,   347,   371,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   155,   202,   216,   433,   434,   435,   436,   449,
     452,   347,   292,   292,   452,   347,   456,   248,   200,   347,
     199,   426,     9,   413,   200,   200,    35,   347,    35,   347,
     200,   200,   200,   449,   292,   202,   216,   433,   434,   449,
     200,   231,   284,   202,   344,   347,   347,    89,    30,   233,
     278,   201,    28,    97,    14,     9,   200,    30,   202,   281,
     489,    29,    86,   229,   483,   484,   485,   199,     9,    46,
      47,    52,    54,    66,   133,   155,   175,   199,   226,   227,
     229,   368,   402,   408,   409,   410,   216,   488,   185,    79,
     347,    79,    79,   347,   383,   384,   347,   347,   376,   386,
     188,   389,   231,   199,   241,   225,   201,     9,    97,   225,
     201,     9,    97,    97,   222,   216,   347,   295,   409,    79,
       9,   200,   200,   200,   200,   200,   200,   200,   201,    46,
      47,   481,   482,   127,   271,   199,     9,   200,   200,    79,
      80,   216,   465,   216,    66,   203,   203,   212,   214,    30,
     128,   270,   170,    50,   155,   170,   396,   348,   131,     9,
     413,   200,   150,   489,   489,    14,   359,   290,   231,   197,
       9,   414,   489,   490,   432,   437,   432,   203,     9,   413,
     172,   442,   347,   200,     9,   414,    14,   351,   251,   127,
     269,   199,   474,   347,    30,   206,   206,   131,   203,     9,
     413,   347,   475,   199,   261,   256,   264,    14,   469,   259,
     248,    68,   442,   347,   475,   206,   203,   200,   200,   206,
     203,   200,    46,    47,    66,    74,    75,    76,    86,   133,
     146,   175,   216,   416,   418,   419,   422,   425,   216,   442,
     442,   131,   432,   437,   200,   347,   285,    71,    72,   286,
     231,   338,   231,   340,    97,    35,   132,   275,   442,   409,
     216,    30,   233,   279,   201,   282,   201,   282,     9,   172,
      86,   131,   150,     9,   413,   200,   164,   476,   477,   478,
     476,   409,   409,   409,   409,   409,   412,   415,   199,   150,
     199,   409,   150,   202,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    65,
     150,   475,   347,   185,   185,    14,   191,   192,   385,     9,
     195,   389,    79,   203,   402,   202,   245,    97,   223,   216,
      97,   224,   216,   216,   203,    14,   442,   201,     9,   172,
     216,   272,   402,   202,   456,   132,   442,    14,   206,   347,
     203,   212,   489,   272,   202,   395,    14,   200,   347,   360,
     125,   217,   449,   201,   489,   197,   203,    30,   479,   431,
      35,    79,   164,   433,   434,   436,   433,   434,   489,    35,
     164,   347,   409,   290,   199,   402,   270,   352,   252,   347,
     347,   347,   203,   199,   292,   271,    30,   270,   489,    14,
     269,   474,   404,   203,   199,    14,    74,    75,    76,   216,
     417,   417,   419,   420,   421,    48,   199,    85,   147,   199,
       9,   413,   200,   426,    35,   347,   203,    71,    72,   287,
     338,   233,   203,   201,    90,   201,   275,   442,   199,   131,
     274,    14,   231,   282,   100,   101,   102,   282,   203,   489,
     131,   489,   216,   483,     9,   200,   413,   131,   206,     9,
     413,   412,   217,   360,   362,   364,   200,   125,   217,   409,
     461,   462,   409,   409,   409,    30,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   488,
     347,   347,   347,   384,   347,   374,    79,   246,   216,   216,
     409,   482,    97,    98,   480,     9,   300,   200,   199,   341,
     344,   347,   206,   203,   469,   300,   156,   169,   202,   391,
     398,   156,   202,   397,   131,   201,   479,   489,   359,   490,
      79,   164,    14,    79,   475,   442,   347,   200,   290,   202,
     290,   199,   131,   199,   292,   200,   202,   489,   202,   201,
     489,   270,   253,   407,   292,   131,   206,     9,   413,   418,
     420,   147,   360,   423,   424,   419,   442,   338,    30,    73,
     233,   201,   340,   274,   456,   275,   200,   409,    96,   100,
     201,   347,    30,   201,   283,   203,   172,   489,   131,   164,
      30,   200,   409,   409,   200,   131,     9,   413,   200,   131,
     203,     9,   413,   409,    30,   186,   200,   231,   216,   489,
     489,   402,     4,   107,   112,   118,   120,   157,   158,   160,
     203,   301,   326,   327,   328,   333,   334,   335,   336,   430,
     456,   203,   202,   203,    50,   347,   347,   347,   359,    35,
      79,   164,    14,    79,   347,   199,   479,   200,   300,   200,
     290,   347,   292,   200,   300,   469,   300,   201,   202,   199,
     200,   419,   419,   200,   131,   200,     9,   413,    30,   231,
     201,   200,   200,   200,   238,   201,   201,   283,   231,   489,
     489,   131,   409,   360,   409,   409,   409,   347,   202,   203,
     480,   127,   128,   176,   217,   472,   489,   273,   402,   107,
     336,    29,   120,   133,   134,   155,   161,   310,   311,   312,
     313,   402,   159,   318,   319,   123,   199,   216,   320,   321,
     302,   249,   489,     9,   201,     9,   201,   201,   469,   327,
     200,   297,   155,   393,   203,   203,    79,   164,    14,    79,
     347,   292,   112,   349,   479,   203,   479,   200,   200,   203,
     202,   203,   300,   290,   131,   419,   360,   231,   236,   239,
      30,   233,   277,   231,   200,   409,   131,   131,   187,   231,
     402,   402,   474,    14,   217,     9,   201,   202,   472,   469,
     313,   171,   202,     9,   201,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    53,    67,    68,    69,
      70,    71,    72,    73,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   132,   133,   135,   136,   137,
     138,   139,   151,   152,   153,   163,   165,   166,   168,   175,
     176,   178,   180,   181,   216,   399,   400,     9,   201,   155,
     159,   216,   321,   322,   323,   201,    79,   332,   248,   303,
     472,   472,    14,   249,   203,   298,   299,   472,    14,    79,
     347,   200,   199,   202,   201,   202,   324,   349,   479,   297,
     203,   200,   419,   131,    30,   233,   276,   277,   231,   409,
     409,   347,   203,   201,   201,   409,   402,   306,   489,   314,
     315,   408,   311,    14,    30,    47,   316,   319,     9,    33,
     200,    29,    46,    49,    14,     9,   201,   218,   473,   332,
      14,   489,   248,   201,    14,   347,    35,    79,   390,   231,
     231,   202,   324,   203,   479,   419,   231,    94,   188,   244,
     203,   216,   229,   307,   308,   309,     9,   172,     9,   413,
     203,   409,   400,   400,    55,   317,   322,   322,    29,    46,
      49,   409,    79,   171,   199,   201,   409,   474,   409,    79,
       9,   414,   203,   203,   231,   324,    92,   201,    79,   110,
     240,   150,    97,   489,   408,   162,    14,   481,   304,   199,
      35,    79,   200,   203,   201,   199,   168,   247,   216,   327,
     328,   172,   409,   172,   288,   289,   431,   305,    79,   402,
     245,   165,   216,   201,   200,     9,   414,   114,   115,   116,
     330,   331,   288,    79,   273,   201,   479,   431,   490,   200,
     200,   201,   201,   202,   325,   330,    35,    79,   164,   479,
     202,   231,   490,    79,   164,    14,    79,   325,   231,   203,
      35,    79,   164,    14,    79,   347,   203,    79,   164,    14,
      79,   347,    14,    79,   347,   347
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
#line 740 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { (yyval).reset();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
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

  case 210:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
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

  case 212:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset();;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval).reset();;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval).reset();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { (yyval).reset();;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { (yyval).reset();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval).reset();;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval).reset();;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval).reset();;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval).reset();;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
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

  case 527:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
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

  case 531:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { Token u; Token v; Token w; Token x;
                                         Token y;
                                         (yyvsp[(1) - (5)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (5)]), nullptr, (yyvsp[(1) - (5)]));
                                         _p->finishStatement((yyvsp[(4) - (5)]), (yyvsp[(4) - (5)])); (yyvsp[(4) - (5)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (5)]),
                                                            u,v,w,(yyvsp[(4) - (5)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);
                                         _p->onCall((yyval),1,(yyval),y,NULL);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
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

  case 596:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
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

  case 597:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval).reset();;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval).reset();;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval).reset();;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval).reset();;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval).reset();;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval).reset();;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval).reset();;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval).reset();;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval).reset();;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval).reset();;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(2) - (5)]),
                                        !(yyvsp[(4) - (5)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(5) - (5)])
                                      );
                                    ;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval).reset();;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval)++;;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[(1) - (3)]),
                                        !(yyvsp[(2) - (3)]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[(3) - (3)])
                                      );
                                    ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { (yyval).reset();;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[(1) - (3)]),
                                           !(yyvsp[(2) - (3)]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[(3) - (3)])
                                         );
                                       ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    {;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    {;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    {;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3261 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3284 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3291 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3294 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3306 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3315 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14460 "hphp.tab.cpp"
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
#line 3318 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

