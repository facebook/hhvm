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
#define YYLAST   17169

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  278
/* YYNRULES -- Number of rules.  */
#define YYNRULES  954
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1781

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
     100,   102,   104,   108,   110,   114,   116,   120,   122,   124,
     127,   131,   136,   138,   141,   145,   150,   152,   155,   159,
     164,   166,   170,   172,   176,   179,   181,   184,   187,   193,
     198,   201,   202,   204,   206,   208,   210,   214,   220,   229,
     230,   235,   236,   243,   244,   255,   256,   261,   264,   268,
     271,   275,   278,   282,   286,   290,   294,   298,   302,   308,
     310,   312,   314,   315,   325,   326,   337,   343,   344,   358,
     359,   365,   369,   373,   376,   379,   382,   385,   388,   391,
     395,   398,   401,   405,   408,   409,   414,   424,   425,   426,
     431,   434,   435,   437,   438,   440,   441,   451,   452,   463,
     464,   476,   477,   487,   488,   499,   500,   509,   510,   520,
     521,   529,   530,   539,   540,   548,   549,   558,   560,   562,
     564,   566,   568,   571,   575,   579,   582,   585,   586,   589,
     590,   593,   594,   596,   600,   602,   606,   609,   610,   612,
     615,   620,   622,   627,   629,   634,   636,   641,   643,   648,
     652,   658,   662,   667,   672,   678,   684,   689,   690,   692,
     694,   699,   700,   706,   707,   710,   711,   715,   716,   724,
     733,   740,   743,   749,   756,   761,   762,   767,   773,   781,
     788,   795,   803,   813,   822,   829,   837,   843,   846,   851,
     857,   861,   862,   866,   871,   878,   884,   890,   897,   906,
     914,   917,   918,   920,   923,   926,   930,   935,   940,   944,
     946,   948,   951,   956,   960,   966,   968,   972,   975,   976,
     979,   983,   986,   987,   988,   993,   994,  1000,  1003,  1006,
    1009,  1010,  1021,  1022,  1034,  1038,  1042,  1046,  1051,  1056,
    1060,  1066,  1069,  1072,  1073,  1080,  1086,  1091,  1095,  1097,
    1099,  1103,  1108,  1110,  1113,  1115,  1117,  1122,  1129,  1131,
    1133,  1138,  1140,  1142,  1146,  1149,  1152,  1153,  1156,  1157,
    1159,  1163,  1165,  1167,  1169,  1171,  1175,  1180,  1185,  1190,
    1192,  1194,  1197,  1200,  1203,  1207,  1211,  1213,  1215,  1217,
    1219,  1223,  1225,  1229,  1231,  1233,  1235,  1236,  1238,  1241,
    1243,  1245,  1247,  1249,  1251,  1253,  1255,  1257,  1258,  1260,
    1262,  1264,  1268,  1274,  1276,  1280,  1286,  1291,  1295,  1299,
    1303,  1308,  1312,  1316,  1320,  1323,  1325,  1327,  1331,  1335,
    1337,  1339,  1340,  1342,  1345,  1350,  1354,  1361,  1364,  1368,
    1375,  1377,  1379,  1381,  1383,  1385,  1392,  1396,  1401,  1408,
    1412,  1416,  1420,  1424,  1428,  1432,  1436,  1440,  1444,  1448,
    1452,  1456,  1459,  1462,  1465,  1468,  1472,  1476,  1480,  1484,
    1488,  1492,  1496,  1500,  1504,  1508,  1512,  1516,  1520,  1524,
    1528,  1532,  1536,  1539,  1542,  1545,  1548,  1552,  1556,  1560,
    1564,  1568,  1572,  1576,  1580,  1584,  1588,  1594,  1599,  1601,
    1604,  1607,  1610,  1613,  1616,  1619,  1622,  1625,  1628,  1630,
    1632,  1634,  1638,  1641,  1643,  1649,  1650,  1651,  1663,  1664,
    1677,  1678,  1682,  1683,  1688,  1689,  1696,  1697,  1705,  1706,
    1712,  1715,  1718,  1723,  1725,  1727,  1733,  1737,  1743,  1747,
    1750,  1751,  1754,  1755,  1760,  1765,  1769,  1774,  1779,  1784,
    1789,  1791,  1793,  1795,  1797,  1801,  1804,  1808,  1813,  1816,
    1820,  1822,  1825,  1827,  1830,  1832,  1834,  1836,  1838,  1840,
    1842,  1847,  1852,  1855,  1864,  1875,  1878,  1880,  1884,  1886,
    1889,  1891,  1893,  1895,  1897,  1900,  1905,  1909,  1913,  1918,
    1920,  1923,  1928,  1931,  1938,  1939,  1941,  1946,  1947,  1950,
    1951,  1953,  1955,  1959,  1961,  1965,  1967,  1969,  1973,  1977,
    1979,  1981,  1983,  1985,  1987,  1989,  1991,  1993,  1995,  1997,
    1999,  2001,  2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,
    2019,  2021,  2023,  2025,  2027,  2029,  2031,  2033,  2035,  2037,
    2039,  2041,  2043,  2045,  2047,  2049,  2051,  2053,  2055,  2057,
    2059,  2061,  2063,  2065,  2067,  2069,  2071,  2073,  2075,  2077,
    2079,  2081,  2083,  2085,  2087,  2089,  2091,  2093,  2095,  2097,
    2099,  2101,  2103,  2105,  2107,  2109,  2111,  2113,  2115,  2117,
    2119,  2121,  2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,
    2142,  2144,  2146,  2148,  2150,  2152,  2154,  2158,  2162,  2164,
    2166,  2168,  2171,  2173,  2174,  2175,  2177,  2179,  2183,  2184,
    2186,  2188,  2190,  2192,  2194,  2196,  2198,  2200,  2202,  2204,
    2206,  2208,  2210,  2214,  2217,  2219,  2221,  2226,  2230,  2235,
    2237,  2239,  2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,
    2275,  2279,  2283,  2287,  2291,  2295,  2299,  2303,  2307,  2311,
    2314,  2317,  2320,  2323,  2327,  2331,  2335,  2339,  2343,  2347,
    2351,  2355,  2361,  2366,  2370,  2374,  2378,  2380,  2382,  2384,
    2386,  2390,  2394,  2398,  2401,  2402,  2404,  2405,  2407,  2408,
    2414,  2418,  2422,  2424,  2426,  2428,  2430,  2434,  2437,  2439,
    2441,  2443,  2445,  2447,  2451,  2453,  2455,  2457,  2460,  2463,
    2468,  2472,  2477,  2480,  2481,  2487,  2491,  2495,  2497,  2501,
    2503,  2506,  2507,  2513,  2517,  2520,  2521,  2525,  2526,  2531,
    2534,  2535,  2539,  2543,  2545,  2546,  2548,  2550,  2552,  2554,
    2558,  2560,  2562,  2564,  2568,  2570,  2572,  2576,  2580,  2583,
    2588,  2591,  2596,  2602,  2608,  2614,  2620,  2622,  2624,  2626,
    2628,  2630,  2632,  2636,  2640,  2645,  2650,  2654,  2656,  2658,
    2660,  2662,  2666,  2668,  2673,  2677,  2679,  2681,  2683,  2685,
    2687,  2691,  2695,  2700,  2705,  2709,  2711,  2713,  2721,  2731,
    2739,  2746,  2755,  2757,  2760,  2765,  2770,  2772,  2774,  2779,
    2781,  2782,  2784,  2787,  2789,  2791,  2793,  2797,  2801,  2805,
    2806,  2808,  2810,  2814,  2818,  2821,  2825,  2832,  2833,  2835,
    2840,  2843,  2844,  2850,  2854,  2858,  2860,  2867,  2872,  2877,
    2880,  2883,  2884,  2890,  2894,  2898,  2900,  2903,  2904,  2910,
    2914,  2918,  2920,  2923,  2926,  2928,  2931,  2933,  2938,  2942,
    2946,  2953,  2957,  2959,  2961,  2963,  2968,  2973,  2978,  2983,
    2988,  2993,  2996,  2999,  3004,  3007,  3010,  3012,  3016,  3020,
    3024,  3025,  3028,  3034,  3041,  3048,  3056,  3058,  3061,  3063,
    3068,  3072,  3073,  3075,  3079,  3082,  3086,  3088,  3090,  3091,
    3092,  3095,  3098,  3101,  3106,  3109,  3115,  3119,  3121,  3123,
    3124,  3128,  3133,  3139,  3143,  3145,  3148,  3149,  3154,  3156,
    3160,  3163,  3166,  3169,  3171,  3173,  3175,  3177,  3181,  3186,
    3193,  3195,  3204,  3211,  3213
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     210,     0,    -1,    -1,   211,   212,    -1,   212,   213,    -1,
      -1,   231,    -1,   248,    -1,   255,    -1,   252,    -1,   260,
      -1,   468,    -1,   124,   199,   200,   201,    -1,   151,   223,
     201,    -1,    -1,   151,   223,   202,   214,   212,   203,    -1,
      -1,   151,   202,   215,   212,   203,    -1,   112,   217,   201,
      -1,   112,   106,   218,   201,    -1,   112,   107,   219,   201,
      -1,   228,   201,    -1,    77,    -1,    98,    -1,   157,    -1,
     158,    -1,   160,    -1,   162,    -1,   161,    -1,   183,    -1,
     184,    -1,   186,    -1,   185,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     194,    -1,   195,    -1,   217,     9,   220,    -1,   220,    -1,
     221,     9,   221,    -1,   221,    -1,   222,     9,   222,    -1,
     222,    -1,   223,    -1,   154,   223,    -1,   223,    97,   216,
      -1,   154,   223,    97,   216,    -1,   223,    -1,   154,   223,
      -1,   223,    97,   216,    -1,   154,   223,    97,   216,    -1,
     223,    -1,   154,   223,    -1,   223,    97,   216,    -1,   154,
     223,    97,   216,    -1,   216,    -1,   223,   154,   216,    -1,
     223,    -1,   151,   154,   223,    -1,   154,   223,    -1,   224,
      -1,   224,   471,    -1,   224,   471,    -1,   228,     9,   469,
      14,   407,    -1,   107,   469,    14,   407,    -1,   229,   230,
      -1,    -1,   231,    -1,   248,    -1,   255,    -1,   260,    -1,
     202,   229,   203,    -1,    70,   336,   231,   282,   284,    -1,
      70,   336,    30,   229,   283,   285,    73,   201,    -1,    -1,
      89,   336,   232,   276,    -1,    -1,    88,   233,   231,    89,
     336,   201,    -1,    -1,    91,   199,   338,   201,   338,   201,
     338,   200,   234,   274,    -1,    -1,    99,   336,   235,   279,
      -1,   103,   201,    -1,   103,   345,   201,    -1,   105,   201,
      -1,   105,   345,   201,    -1,   108,   201,    -1,   108,   345,
     201,    -1,    27,   103,   201,    -1,   113,   292,   201,    -1,
     119,   294,   201,    -1,    87,   337,   201,    -1,   143,   337,
     201,    -1,   121,   199,   465,   200,   201,    -1,   201,    -1,
      81,    -1,    82,    -1,    -1,    93,   199,   345,    97,   273,
     272,   200,   236,   275,    -1,    -1,    93,   199,   345,    28,
      97,   273,   272,   200,   237,   275,    -1,    95,   199,   278,
     200,   277,    -1,    -1,   109,   240,   110,   199,   400,    79,
     200,   202,   229,   203,   242,   238,   245,    -1,    -1,   109,
     240,   168,   239,   243,    -1,   111,   345,   201,    -1,   104,
     216,   201,    -1,   345,   201,    -1,   339,   201,    -1,   340,
     201,    -1,   341,   201,    -1,   342,   201,    -1,   343,   201,
      -1,   108,   342,   201,    -1,   344,   201,    -1,   370,   201,
      -1,   108,   369,   201,    -1,   216,    30,    -1,    -1,   202,
     241,   229,   203,    -1,   242,   110,   199,   400,    79,   200,
     202,   229,   203,    -1,    -1,    -1,   202,   244,   229,   203,
      -1,   168,   243,    -1,    -1,    35,    -1,    -1,   106,    -1,
      -1,   247,   246,   470,   249,   199,   288,   200,   475,   322,
      -1,    -1,   326,   247,   246,   470,   250,   199,   288,   200,
     475,   322,    -1,    -1,   428,   325,   247,   246,   470,   251,
     199,   288,   200,   475,   322,    -1,    -1,   161,   216,   253,
      30,   485,   467,   202,   295,   203,    -1,    -1,   428,   161,
     216,   254,    30,   485,   467,   202,   295,   203,    -1,    -1,
     266,   263,   256,   267,   268,   202,   298,   203,    -1,    -1,
     428,   266,   263,   257,   267,   268,   202,   298,   203,    -1,
      -1,   126,   264,   258,   269,   202,   298,   203,    -1,    -1,
     428,   126,   264,   259,   269,   202,   298,   203,    -1,    -1,
     163,   265,   261,   268,   202,   298,   203,    -1,    -1,   428,
     163,   265,   262,   268,   202,   298,   203,    -1,   470,    -1,
     155,    -1,   470,    -1,   470,    -1,   125,    -1,   118,   125,
      -1,   118,   117,   125,    -1,   117,   118,   125,    -1,   117,
     125,    -1,   127,   400,    -1,    -1,   128,   270,    -1,    -1,
     127,   270,    -1,    -1,   400,    -1,   270,     9,   400,    -1,
     400,    -1,   271,     9,   400,    -1,   131,   273,    -1,    -1,
     440,    -1,    35,   440,    -1,   132,   199,   454,   200,    -1,
     231,    -1,    30,   229,    92,   201,    -1,   231,    -1,    30,
     229,    94,   201,    -1,   231,    -1,    30,   229,    90,   201,
      -1,   231,    -1,    30,   229,    96,   201,    -1,   216,    14,
     407,    -1,   278,     9,   216,    14,   407,    -1,   202,   280,
     203,    -1,   202,   201,   280,   203,    -1,    30,   280,   100,
     201,    -1,    30,   201,   280,   100,   201,    -1,   280,   101,
     345,   281,   229,    -1,   280,   102,   281,   229,    -1,    -1,
      30,    -1,   201,    -1,   282,    71,   336,   231,    -1,    -1,
     283,    71,   336,    30,   229,    -1,    -1,    72,   231,    -1,
      -1,    72,    30,   229,    -1,    -1,   287,     9,   429,   328,
     486,   164,    79,    -1,   287,     9,   429,   328,   486,    35,
     164,    79,    -1,   287,     9,   429,   328,   486,   164,    -1,
     287,   412,    -1,   429,   328,   486,   164,    79,    -1,   429,
     328,   486,    35,   164,    79,    -1,   429,   328,   486,   164,
      -1,    -1,   429,   328,   486,    79,    -1,   429,   328,   486,
      35,    79,    -1,   429,   328,   486,    35,    79,    14,   345,
      -1,   429,   328,   486,    79,    14,   345,    -1,   287,     9,
     429,   328,   486,    79,    -1,   287,     9,   429,   328,   486,
      35,    79,    -1,   287,     9,   429,   328,   486,    35,    79,
      14,   345,    -1,   287,     9,   429,   328,   486,    79,    14,
     345,    -1,   289,     9,   429,   486,   164,    79,    -1,   289,
       9,   429,   486,    35,   164,    79,    -1,   289,     9,   429,
     486,   164,    -1,   289,   412,    -1,   429,   486,   164,    79,
      -1,   429,   486,    35,   164,    79,    -1,   429,   486,   164,
      -1,    -1,   429,   486,    79,    -1,   429,   486,    35,    79,
      -1,   429,   486,    35,    79,    14,   345,    -1,   429,   486,
      79,    14,   345,    -1,   289,     9,   429,   486,    79,    -1,
     289,     9,   429,   486,    35,    79,    -1,   289,     9,   429,
     486,    35,    79,    14,   345,    -1,   289,     9,   429,   486,
      79,    14,   345,    -1,   291,   412,    -1,    -1,   345,    -1,
      35,   440,    -1,   164,   345,    -1,   291,     9,   345,    -1,
     291,     9,   164,   345,    -1,   291,     9,    35,   440,    -1,
     292,     9,   293,    -1,   293,    -1,    79,    -1,   204,   440,
      -1,   204,   202,   345,   203,    -1,   294,     9,    79,    -1,
     294,     9,    79,    14,   407,    -1,    79,    -1,    79,    14,
     407,    -1,   295,   296,    -1,    -1,   297,   201,    -1,   469,
      14,   407,    -1,   298,   299,    -1,    -1,    -1,   324,   300,
     330,   201,    -1,    -1,   326,   485,   301,   330,   201,    -1,
     331,   201,    -1,   332,   201,    -1,   333,   201,    -1,    -1,
     325,   247,   246,   470,   199,   302,   286,   200,   475,   323,
      -1,    -1,   428,   325,   247,   246,   470,   199,   303,   286,
     200,   475,   323,    -1,   157,   308,   201,    -1,   158,   316,
     201,    -1,   160,   318,   201,    -1,     4,   127,   400,   201,
      -1,     4,   128,   400,   201,    -1,   112,   271,   201,    -1,
     112,   271,   202,   304,   203,    -1,   304,   305,    -1,   304,
     306,    -1,    -1,   227,   150,   216,   165,   271,   201,    -1,
     307,    97,   325,   216,   201,    -1,   307,    97,   326,   201,
      -1,   227,   150,   216,    -1,   216,    -1,   309,    -1,   308,
       9,   309,    -1,   310,   397,   314,   315,    -1,   155,    -1,
      29,   311,    -1,   311,    -1,   133,    -1,   133,   171,   485,
     172,    -1,   133,   171,   485,     9,   485,   172,    -1,   400,
      -1,   120,    -1,   161,   202,   313,   203,    -1,   134,    -1,
     406,    -1,   312,     9,   406,    -1,   312,   411,    -1,    14,
     407,    -1,    -1,    55,   162,    -1,    -1,   317,    -1,   316,
       9,   317,    -1,   159,    -1,   319,    -1,   216,    -1,   123,
      -1,   199,   320,   200,    -1,   199,   320,   200,    49,    -1,
     199,   320,   200,    29,    -1,   199,   320,   200,    46,    -1,
     319,    -1,   321,    -1,   321,    49,    -1,   321,    29,    -1,
     321,    46,    -1,   320,     9,   320,    -1,   320,    33,   320,
      -1,   216,    -1,   155,    -1,   159,    -1,   201,    -1,   202,
     229,   203,    -1,   201,    -1,   202,   229,   203,    -1,   326,
      -1,   120,    -1,   326,    -1,    -1,   327,    -1,   326,   327,
      -1,   114,    -1,   115,    -1,   116,    -1,   119,    -1,   118,
      -1,   117,    -1,   181,    -1,   329,    -1,    -1,   114,    -1,
     115,    -1,   116,    -1,   330,     9,    79,    -1,   330,     9,
      79,    14,   407,    -1,    79,    -1,    79,    14,   407,    -1,
     331,     9,   469,    14,   407,    -1,   107,   469,    14,   407,
      -1,   332,     9,   469,    -1,   118,   107,   469,    -1,   118,
     334,   467,    -1,   334,   467,    14,   485,    -1,   107,   176,
     470,    -1,   199,   335,   200,    -1,    68,   402,   405,    -1,
      67,   345,    -1,   389,    -1,   365,    -1,   199,   345,   200,
      -1,   337,     9,   345,    -1,   345,    -1,   337,    -1,    -1,
      27,    -1,    27,   345,    -1,    27,   345,   131,   345,    -1,
     440,    14,   339,    -1,   132,   199,   454,   200,    14,   339,
      -1,    28,   345,    -1,   440,    14,   342,    -1,   132,   199,
     454,   200,    14,   342,    -1,   346,    -1,   440,    -1,   335,
      -1,   444,    -1,   443,    -1,   132,   199,   454,   200,    14,
     345,    -1,   440,    14,   345,    -1,   440,    14,    35,   440,
      -1,   440,    14,    35,    68,   402,   405,    -1,   440,    26,
     345,    -1,   440,    25,   345,    -1,   440,    24,   345,    -1,
     440,    23,   345,    -1,   440,    22,   345,    -1,   440,    21,
     345,    -1,   440,    20,   345,    -1,   440,    19,   345,    -1,
     440,    18,   345,    -1,   440,    17,   345,    -1,   440,    16,
     345,    -1,   440,    15,   345,    -1,   440,    64,    -1,    64,
     440,    -1,   440,    63,    -1,    63,   440,    -1,   345,    31,
     345,    -1,   345,    32,   345,    -1,   345,    10,   345,    -1,
     345,    12,   345,    -1,   345,    11,   345,    -1,   345,    33,
     345,    -1,   345,    35,   345,    -1,   345,    34,   345,    -1,
     345,    48,   345,    -1,   345,    46,   345,    -1,   345,    47,
     345,    -1,   345,    49,   345,    -1,   345,    50,   345,    -1,
     345,    65,   345,    -1,   345,    51,   345,    -1,   345,    45,
     345,    -1,   345,    44,   345,    -1,    46,   345,    -1,    47,
     345,    -1,    52,   345,    -1,    54,   345,    -1,   345,    37,
     345,    -1,   345,    36,   345,    -1,   345,    39,   345,    -1,
     345,    38,   345,    -1,   345,    40,   345,    -1,   345,    43,
     345,    -1,   345,    41,   345,    -1,   345,    42,   345,    -1,
     345,    53,   402,    -1,   199,   346,   200,    -1,   345,    29,
     345,    30,   345,    -1,   345,    29,    30,   345,    -1,   464,
      -1,    62,   345,    -1,    61,   345,    -1,    60,   345,    -1,
      59,   345,    -1,    58,   345,    -1,    57,   345,    -1,    56,
     345,    -1,    69,   403,    -1,    55,   345,    -1,   409,    -1,
     364,    -1,   363,    -1,   205,   404,   205,    -1,    13,   345,
      -1,   367,    -1,   112,   199,   388,   412,   200,    -1,    -1,
      -1,   247,   246,   199,   349,   288,   200,   475,   347,   202,
     229,   203,    -1,    -1,   326,   247,   246,   199,   350,   288,
     200,   475,   347,   202,   229,   203,    -1,    -1,    79,   352,
     357,    -1,    -1,   181,    79,   353,   357,    -1,    -1,   196,
     354,   288,   197,   475,   357,    -1,    -1,   181,   196,   355,
     288,   197,   475,   357,    -1,    -1,   181,   202,   356,   229,
     203,    -1,     8,   345,    -1,     8,   342,    -1,     8,   202,
     229,   203,    -1,    86,    -1,   466,    -1,   359,     9,   358,
     131,   345,    -1,   358,   131,   345,    -1,   360,     9,   358,
     131,   407,    -1,   358,   131,   407,    -1,   359,   411,    -1,
      -1,   360,   411,    -1,    -1,   175,   199,   361,   200,    -1,
     133,   199,   455,   200,    -1,    66,   455,   206,    -1,   400,
     202,   457,   203,    -1,   400,   202,   459,   203,    -1,   367,
      66,   450,   206,    -1,   368,    66,   450,   206,    -1,   364,
      -1,   466,    -1,   443,    -1,    86,    -1,   199,   346,   200,
      -1,   371,   372,    -1,   440,    14,   369,    -1,   182,    79,
     185,   345,    -1,   373,   384,    -1,   373,   384,   387,    -1,
     384,    -1,   384,   387,    -1,   374,    -1,   373,   374,    -1,
     375,    -1,   376,    -1,   377,    -1,   378,    -1,   379,    -1,
     380,    -1,   182,    79,   185,   345,    -1,   189,    79,    14,
     345,    -1,   183,   345,    -1,   184,    79,   185,   345,   186,
     345,   187,   345,    -1,   184,    79,   185,   345,   186,   345,
     187,   345,   188,    79,    -1,   190,   381,    -1,   382,    -1,
     381,     9,   382,    -1,   345,    -1,   345,   383,    -1,   191,
      -1,   192,    -1,   385,    -1,   386,    -1,   193,   345,    -1,
     194,   345,   195,   345,    -1,   188,    79,   372,    -1,   388,
       9,    79,    -1,   388,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   169,   155,   390,   170,    -1,   392,    50,
      -1,   392,   170,   393,   169,    50,   391,    -1,    -1,   155,
      -1,   392,   394,    14,   395,    -1,    -1,   393,   396,    -1,
      -1,   155,    -1,   156,    -1,   202,   345,   203,    -1,   156,
      -1,   202,   345,   203,    -1,   389,    -1,   398,    -1,   397,
      30,   398,    -1,   397,    47,   398,    -1,   216,    -1,    69,
      -1,   106,    -1,   107,    -1,   108,    -1,    27,    -1,    28,
      -1,   109,    -1,   110,    -1,   168,    -1,   111,    -1,    70,
      -1,    71,    -1,    73,    -1,    72,    -1,    89,    -1,    90,
      -1,    88,    -1,    91,    -1,    92,    -1,    93,    -1,    94,
      -1,    95,    -1,    96,    -1,    53,    -1,    97,    -1,    99,
      -1,   100,    -1,   101,    -1,   102,    -1,   103,    -1,   105,
      -1,   104,    -1,    87,    -1,    13,    -1,   125,    -1,   126,
      -1,   127,    -1,   128,    -1,    68,    -1,    67,    -1,   120,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   151,    -1,   112,    -1,   113,    -1,   122,    -1,   123,
      -1,   124,    -1,   119,    -1,   118,    -1,   117,    -1,   116,
      -1,   115,    -1,   114,    -1,   181,    -1,   121,    -1,   132,
      -1,   133,    -1,    10,    -1,    12,    -1,    11,    -1,   135,
      -1,   137,    -1,   136,    -1,   138,    -1,   139,    -1,   153,
      -1,   152,    -1,   180,    -1,   163,    -1,   166,    -1,   165,
      -1,   176,    -1,   178,    -1,   175,    -1,   226,   199,   290,
     200,    -1,   227,    -1,   155,    -1,   400,    -1,   406,    -1,
     119,    -1,   448,    -1,   199,   346,   200,    -1,   401,   150,
     447,    -1,   400,    -1,   119,    -1,   452,    -1,   199,   200,
      -1,   336,    -1,    -1,    -1,    85,    -1,   461,    -1,   199,
     290,   200,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      86,    -1,   138,    -1,   139,    -1,   153,    -1,   135,    -1,
     166,    -1,   136,    -1,   137,    -1,   152,    -1,   180,    -1,
     146,    85,   147,    -1,   146,   147,    -1,   406,    -1,   225,
      -1,   133,   199,   410,   200,    -1,    66,   410,   206,    -1,
     175,   199,   362,   200,    -1,   408,    -1,   366,    -1,   199,
     407,   200,    -1,   407,    31,   407,    -1,   407,    32,   407,
      -1,   407,    10,   407,    -1,   407,    12,   407,    -1,   407,
      11,   407,    -1,   407,    33,   407,    -1,   407,    35,   407,
      -1,   407,    34,   407,    -1,   407,    48,   407,    -1,   407,
      46,   407,    -1,   407,    47,   407,    -1,   407,    49,   407,
      -1,   407,    50,   407,    -1,   407,    51,   407,    -1,   407,
      45,   407,    -1,   407,    44,   407,    -1,   407,    65,   407,
      -1,    52,   407,    -1,    54,   407,    -1,    46,   407,    -1,
      47,   407,    -1,   407,    37,   407,    -1,   407,    36,   407,
      -1,   407,    39,   407,    -1,   407,    38,   407,    -1,   407,
      40,   407,    -1,   407,    43,   407,    -1,   407,    41,   407,
      -1,   407,    42,   407,    -1,   407,    29,   407,    30,   407,
      -1,   407,    29,    30,   407,    -1,   227,   150,   216,    -1,
     155,   150,   216,    -1,   227,   150,   125,    -1,   225,    -1,
      78,    -1,   466,    -1,   406,    -1,   207,   461,   207,    -1,
     208,   461,   208,    -1,   146,   461,   147,    -1,   413,   411,
      -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   413,     9,
     407,   131,   407,    -1,   413,     9,   407,    -1,   407,   131,
     407,    -1,   407,    -1,    74,    -1,    75,    -1,    76,    -1,
     146,    85,   147,    -1,   146,   147,    -1,    74,    -1,    75,
      -1,    76,    -1,   216,    -1,    86,    -1,    86,    48,   416,
      -1,   414,    -1,   416,    -1,   216,    -1,    46,   415,    -1,
      47,   415,    -1,   133,   199,   418,   200,    -1,    66,   418,
     206,    -1,   175,   199,   421,   200,    -1,   419,   411,    -1,
      -1,   419,     9,   417,   131,   417,    -1,   419,     9,   417,
      -1,   417,   131,   417,    -1,   417,    -1,   420,     9,   417,
      -1,   417,    -1,   422,   411,    -1,    -1,   422,     9,   358,
     131,   417,    -1,   358,   131,   417,    -1,   420,   411,    -1,
      -1,   199,   423,   200,    -1,    -1,   425,     9,   216,   424,
      -1,   216,   424,    -1,    -1,   427,   425,   411,    -1,    45,
     426,    44,    -1,   428,    -1,    -1,   129,    -1,   130,    -1,
     216,    -1,   155,    -1,   202,   345,   203,    -1,   431,    -1,
     447,    -1,   216,    -1,   202,   345,   203,    -1,   433,    -1,
     447,    -1,    66,   450,   206,    -1,   202,   345,   203,    -1,
     441,   435,    -1,   199,   335,   200,   435,    -1,   453,   435,
      -1,   199,   335,   200,   435,    -1,   199,   335,   200,   430,
     432,    -1,   199,   346,   200,   430,   432,    -1,   199,   335,
     200,   430,   431,    -1,   199,   346,   200,   430,   431,    -1,
     447,    -1,   399,    -1,   445,    -1,   446,    -1,   436,    -1,
     438,    -1,   440,   430,   432,    -1,   401,   150,   447,    -1,
     442,   199,   290,   200,    -1,   443,   199,   290,   200,    -1,
     199,   440,   200,    -1,   399,    -1,   445,    -1,   446,    -1,
     436,    -1,   440,   430,   431,    -1,   439,    -1,   442,   199,
     290,   200,    -1,   199,   440,   200,    -1,   447,    -1,   436,
      -1,   399,    -1,   364,    -1,   406,    -1,   199,   440,   200,
      -1,   199,   346,   200,    -1,   443,   199,   290,   200,    -1,
     442,   199,   290,   200,    -1,   199,   444,   200,    -1,   348,
      -1,   351,    -1,   440,   430,   434,   471,   199,   290,   200,
      -1,   199,   335,   200,   430,   434,   471,   199,   290,   200,
      -1,   401,   150,   216,   471,   199,   290,   200,    -1,   401,
     150,   447,   199,   290,   200,    -1,   401,   150,   202,   345,
     203,   199,   290,   200,    -1,   448,    -1,   451,   448,    -1,
     448,    66,   450,   206,    -1,   448,   202,   345,   203,    -1,
     449,    -1,    79,    -1,   204,   202,   345,   203,    -1,   345,
      -1,    -1,   204,    -1,   451,   204,    -1,   447,    -1,   437,
      -1,   438,    -1,   452,   430,   432,    -1,   401,   150,   447,
      -1,   199,   440,   200,    -1,    -1,   437,    -1,   439,    -1,
     452,   430,   431,    -1,   199,   440,   200,    -1,   454,     9,
      -1,   454,     9,   440,    -1,   454,     9,   132,   199,   454,
     200,    -1,    -1,   440,    -1,   132,   199,   454,   200,    -1,
     456,   411,    -1,    -1,   456,     9,   345,   131,   345,    -1,
     456,     9,   345,    -1,   345,   131,   345,    -1,   345,    -1,
     456,     9,   345,   131,    35,   440,    -1,   456,     9,    35,
     440,    -1,   345,   131,    35,   440,    -1,    35,   440,    -1,
     458,   411,    -1,    -1,   458,     9,   345,   131,   345,    -1,
     458,     9,   345,    -1,   345,   131,   345,    -1,   345,    -1,
     460,   411,    -1,    -1,   460,     9,   407,   131,   407,    -1,
     460,     9,   407,    -1,   407,   131,   407,    -1,   407,    -1,
     461,   462,    -1,   461,    85,    -1,   462,    -1,    85,   462,
      -1,    79,    -1,    79,    66,   463,   206,    -1,    79,   430,
     216,    -1,   148,   345,   203,    -1,   148,    78,    66,   345,
     206,   203,    -1,   149,   440,   203,    -1,   216,    -1,    80,
      -1,    79,    -1,   122,   199,   465,   200,    -1,   123,   199,
     440,   200,    -1,   123,   199,   346,   200,    -1,   123,   199,
     444,   200,    -1,   123,   199,   443,   200,    -1,   123,   199,
     335,   200,    -1,     7,   345,    -1,     6,   345,    -1,     5,
     199,   345,   200,    -1,     4,   345,    -1,     3,   345,    -1,
     440,    -1,   465,     9,   440,    -1,   401,   150,   216,    -1,
     401,   150,   125,    -1,    -1,    97,   485,    -1,   176,   470,
      14,   485,   201,    -1,   428,   176,   470,    14,   485,   201,
      -1,   178,   470,   467,    14,   485,   201,    -1,   428,   178,
     470,   467,    14,   485,   201,    -1,   216,    -1,   485,   216,
      -1,   216,    -1,   216,   171,   477,   172,    -1,   171,   473,
     172,    -1,    -1,   485,    -1,   472,     9,   485,    -1,   472,
     411,    -1,   472,     9,   164,    -1,   473,    -1,   164,    -1,
      -1,    -1,    30,   485,    -1,    97,   485,    -1,    98,   485,
      -1,   477,     9,   478,   216,    -1,   478,   216,    -1,   477,
       9,   478,   216,   476,    -1,   478,   216,   476,    -1,    46,
      -1,    47,    -1,    -1,    86,   131,   485,    -1,    29,    86,
     131,   485,    -1,   227,   150,   216,   131,   485,    -1,   480,
       9,   479,    -1,   479,    -1,   480,   411,    -1,    -1,   175,
     199,   481,   200,    -1,   227,    -1,   216,   150,   484,    -1,
     216,   471,    -1,    29,   485,    -1,    55,   485,    -1,   227,
      -1,   133,    -1,   134,    -1,   482,    -1,   483,   150,   484,
      -1,   133,   171,   485,   172,    -1,   133,   171,   485,     9,
     485,   172,    -1,   155,    -1,   199,   106,   199,   474,   200,
      30,   485,   200,    -1,   199,   485,     9,   472,   411,   200,
      -1,   485,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   740,   740,   740,   749,   751,   754,   755,   756,   757,
     758,   759,   760,   763,   765,   765,   767,   767,   769,   770,
     772,   774,   779,   780,   781,   782,   783,   784,   785,   786,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   796,
     797,   798,   802,   804,   808,   810,   814,   816,   820,   821,
     822,   823,   828,   829,   830,   831,   836,   837,   838,   839,
     844,   845,   849,   850,   852,   855,   861,   868,   875,   879,
     885,   887,   890,   891,   892,   893,   896,   897,   901,   906,
     906,   912,   912,   919,   918,   924,   924,   929,   930,   931,
     932,   933,   934,   935,   936,   937,   938,   939,   940,   941,
     942,   943,   947,   945,   954,   952,   959,   967,   961,   971,
     969,   973,   974,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   996,   996,  1001,  1007,  1011,  1011,
    1019,  1020,  1024,  1025,  1029,  1034,  1033,  1046,  1044,  1058,
    1056,  1072,  1071,  1080,  1078,  1090,  1089,  1108,  1106,  1125,
    1124,  1133,  1131,  1143,  1142,  1154,  1152,  1165,  1166,  1170,
    1173,  1176,  1177,  1178,  1181,  1182,  1185,  1187,  1190,  1191,
    1194,  1195,  1198,  1199,  1203,  1204,  1209,  1210,  1213,  1214,
    1215,  1219,  1220,  1224,  1225,  1229,  1230,  1234,  1235,  1240,
    1241,  1246,  1247,  1248,  1249,  1252,  1255,  1257,  1260,  1261,
    1265,  1267,  1270,  1273,  1276,  1277,  1280,  1281,  1285,  1291,
    1297,  1304,  1306,  1311,  1316,  1322,  1326,  1330,  1334,  1339,
    1344,  1349,  1354,  1360,  1369,  1374,  1379,  1385,  1387,  1391,
    1395,  1400,  1404,  1407,  1410,  1414,  1418,  1422,  1426,  1431,
    1439,  1441,  1444,  1445,  1446,  1447,  1449,  1451,  1456,  1457,
    1460,  1461,  1462,  1466,  1467,  1469,  1470,  1474,  1476,  1479,
    1483,  1489,  1491,  1494,  1494,  1498,  1497,  1501,  1503,  1506,
    1509,  1507,  1522,  1519,  1532,  1534,  1536,  1538,  1540,  1542,
    1544,  1548,  1549,  1550,  1553,  1559,  1562,  1568,  1571,  1576,
    1578,  1583,  1588,  1592,  1593,  1597,  1598,  1600,  1602,  1608,
    1609,  1611,  1615,  1616,  1621,  1625,  1626,  1630,  1631,  1635,
    1637,  1643,  1648,  1649,  1651,  1655,  1656,  1657,  1658,  1662,
    1663,  1664,  1665,  1666,  1667,  1669,  1674,  1677,  1678,  1682,
    1683,  1687,  1688,  1691,  1692,  1695,  1696,  1699,  1700,  1704,
    1705,  1706,  1707,  1708,  1709,  1710,  1714,  1715,  1718,  1719,
    1720,  1723,  1725,  1727,  1728,  1731,  1733,  1737,  1739,  1743,
    1747,  1751,  1755,  1756,  1758,  1759,  1760,  1763,  1767,  1768,
    1772,  1773,  1777,  1778,  1779,  1783,  1787,  1792,  1796,  1800,
    1805,  1806,  1807,  1808,  1809,  1813,  1815,  1816,  1817,  1820,
    1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,
    1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,
    1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,
    1851,  1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,
    1861,  1862,  1864,  1865,  1867,  1869,  1870,  1871,  1872,  1873,
    1874,  1875,  1876,  1877,  1878,  1879,  1880,  1881,  1882,  1883,
    1884,  1885,  1886,  1887,  1891,  1895,  1900,  1899,  1914,  1912,
    1929,  1929,  1945,  1944,  1962,  1962,  1978,  1977,  1996,  1995,
    2016,  2017,  2018,  2023,  2025,  2029,  2033,  2039,  2043,  2049,
    2051,  2055,  2057,  2061,  2065,  2066,  2070,  2077,  2084,  2086,
    2091,  2092,  2093,  2094,  2096,  2100,  2104,  2108,  2112,  2114,
    2116,  2118,  2123,  2124,  2129,  2130,  2131,  2132,  2133,  2134,
    2138,  2142,  2146,  2150,  2155,  2160,  2164,  2165,  2169,  2170,
    2174,  2175,  2179,  2180,  2184,  2188,  2192,  2196,  2197,  2198,
    2199,  2203,  2209,  2218,  2231,  2232,  2235,  2238,  2241,  2242,
    2245,  2249,  2252,  2255,  2262,  2263,  2267,  2268,  2270,  2274,
    2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,
    2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,
    2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,
    2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,
    2315,  2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,
    2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,
    2335,  2336,  2337,  2338,  2339,  2340,  2341,  2342,  2343,  2344,
    2345,  2346,  2347,  2348,  2349,  2350,  2351,  2352,  2353,  2357,
    2362,  2363,  2366,  2367,  2368,  2369,  2371,  2373,  2378,  2379,
    2380,  2384,  2385,  2386,  2390,  2391,  2392,  2395,  2397,  2401,
    2402,  2403,  2404,  2406,  2407,  2408,  2409,  2410,  2411,  2412,
    2413,  2414,  2415,  2418,  2423,  2424,  2425,  2427,  2428,  2430,
    2431,  2432,  2433,  2435,  2437,  2439,  2441,  2443,  2444,  2445,
    2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,
    2456,  2457,  2458,  2459,  2461,  2463,  2465,  2467,  2468,  2471,
    2472,  2476,  2478,  2482,  2485,  2488,  2494,  2495,  2496,  2497,
    2498,  2499,  2500,  2505,  2507,  2511,  2512,  2515,  2516,  2520,
    2523,  2525,  2527,  2531,  2532,  2533,  2534,  2537,  2541,  2542,
    2543,  2544,  2548,  2550,  2557,  2558,  2559,  2560,  2561,  2562,
    2564,  2565,  2570,  2572,  2575,  2578,  2580,  2582,  2585,  2587,
    2591,  2593,  2596,  2599,  2605,  2607,  2610,  2611,  2616,  2619,
    2623,  2623,  2628,  2631,  2632,  2636,  2637,  2641,  2642,  2643,
    2647,  2648,  2652,  2653,  2657,  2658,  2662,  2663,  2667,  2668,
    2673,  2675,  2680,  2691,  2705,  2717,  2732,  2733,  2734,  2735,
    2736,  2737,  2738,  2748,  2751,  2753,  2755,  2759,  2760,  2761,
    2762,  2763,  2774,  2775,  2777,  2781,  2782,  2783,  2784,  2785,
    2786,  2787,  2788,  2790,  2795,  2799,  2800,  2804,  2807,  2814,
    2818,  2822,  2829,  2830,  2835,  2837,  2838,  2841,  2842,  2845,
    2846,  2850,  2851,  2855,  2856,  2857,  2858,  2869,  2872,  2875,
    2876,  2877,  2878,  2889,  2893,  2894,  2895,  2897,  2898,  2899,
    2903,  2905,  2908,  2910,  2911,  2912,  2913,  2916,  2918,  2919,
    2923,  2925,  2928,  2930,  2931,  2932,  2936,  2938,  2941,  2944,
    2946,  2948,  2952,  2953,  2955,  2956,  2962,  2963,  2965,  2975,
    2977,  2979,  2982,  2983,  2984,  2988,  2989,  2990,  2991,  2992,
    2993,  2994,  2995,  2996,  2997,  2998,  3002,  3003,  3007,  3009,
    3017,  3019,  3023,  3027,  3032,  3036,  3044,  3045,  3051,  3052,
    3060,  3063,  3067,  3070,  3075,  3080,  3082,  3083,  3084,  3088,
    3089,  3093,  3094,  3097,  3099,  3100,  3103,  3108,  3109,  3110,
    3114,  3118,  3128,  3136,  3138,  3142,  3144,  3149,  3155,  3158,
    3161,  3168,  3171,  3174,  3175,  3178,  3181,  3182,  3187,  3190,
    3194,  3198,  3204,  3214,  3215
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
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3", "ident",
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
  "hh_name_with_type", "hh_name_with_typevar", "hh_typeargs_opt",
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
     216,   216,   217,   217,   218,   218,   219,   219,   220,   220,
     220,   220,   221,   221,   221,   221,   222,   222,   222,   222,
     223,   223,   224,   224,   224,   225,   226,   227,   228,   228,
     229,   229,   230,   230,   230,   230,   231,   231,   231,   232,
     231,   233,   231,   234,   231,   235,   231,   231,   231,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   231,   236,   231,   237,   231,   231,   238,   231,   239,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   231,   231,   231,   241,   240,   242,   242,   244,   243,
     245,   245,   246,   246,   247,   249,   248,   250,   248,   251,
     248,   253,   252,   254,   252,   256,   255,   257,   255,   258,
     255,   259,   255,   261,   260,   262,   260,   263,   263,   264,
     265,   266,   266,   266,   266,   266,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   272,   272,   273,   273,
     273,   274,   274,   275,   275,   276,   276,   277,   277,   278,
     278,   279,   279,   279,   279,   280,   280,   280,   281,   281,
     282,   282,   283,   283,   284,   284,   285,   285,   286,   286,
     286,   286,   286,   286,   286,   286,   287,   287,   287,   287,
     287,   287,   287,   287,   288,   288,   288,   288,   288,   288,
     288,   288,   289,   289,   289,   289,   289,   289,   289,   289,
     290,   290,   291,   291,   291,   291,   291,   291,   292,   292,
     293,   293,   293,   294,   294,   294,   294,   295,   295,   296,
     297,   298,   298,   300,   299,   301,   299,   299,   299,   299,
     302,   299,   303,   299,   299,   299,   299,   299,   299,   299,
     299,   304,   304,   304,   305,   306,   306,   307,   307,   308,
     308,   309,   309,   310,   310,   311,   311,   311,   311,   311,
     311,   311,   312,   312,   313,   314,   314,   315,   315,   316,
     316,   317,   318,   318,   318,   319,   319,   319,   319,   320,
     320,   320,   320,   320,   320,   320,   321,   321,   321,   322,
     322,   323,   323,   324,   324,   325,   325,   326,   326,   327,
     327,   327,   327,   327,   327,   327,   328,   328,   329,   329,
     329,   330,   330,   330,   330,   331,   331,   332,   332,   333,
     333,   334,   335,   335,   335,   335,   335,   336,   337,   337,
     338,   338,   339,   339,   339,   340,   341,   342,   343,   344,
     345,   345,   345,   345,   345,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   347,   347,   349,   348,   350,   348,
     352,   351,   353,   351,   354,   351,   355,   351,   356,   351,
     357,   357,   357,   358,   358,   359,   359,   360,   360,   361,
     361,   362,   362,   363,   364,   364,   365,   366,   367,   367,
     368,   368,   368,   368,   368,   369,   370,   371,   372,   372,
     372,   372,   373,   373,   374,   374,   374,   374,   374,   374,
     375,   376,   377,   378,   379,   380,   381,   381,   382,   382,
     383,   383,   384,   384,   385,   386,   387,   388,   388,   388,
     388,   389,   390,   390,   391,   391,   392,   392,   393,   393,
     394,   395,   395,   396,   396,   396,   397,   397,   397,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   399,
     400,   400,   401,   401,   401,   401,   401,   401,   402,   402,
     402,   403,   403,   403,   404,   404,   404,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   408,   408,   408,   409,   409,   409,   409,
     409,   409,   409,   410,   410,   411,   411,   412,   412,   413,
     413,   413,   413,   414,   414,   414,   414,   414,   415,   415,
     415,   415,   416,   416,   417,   417,   417,   417,   417,   417,
     417,   417,   418,   418,   419,   419,   419,   419,   420,   420,
     421,   421,   422,   422,   423,   423,   424,   424,   425,   425,
     427,   426,   428,   429,   429,   430,   430,   431,   431,   431,
     432,   432,   433,   433,   434,   434,   435,   435,   436,   436,
     437,   437,   438,   438,   439,   439,   440,   440,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   441,   441,   441,
     441,   441,   441,   441,   441,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   443,   444,   444,   445,   445,   446,
     446,   446,   447,   447,   448,   448,   448,   449,   449,   450,
     450,   451,   451,   452,   452,   452,   452,   452,   452,   453,
     453,   453,   453,   453,   454,   454,   454,   454,   454,   454,
     455,   455,   456,   456,   456,   456,   456,   456,   456,   456,
     457,   457,   458,   458,   458,   458,   459,   459,   460,   460,
     460,   460,   461,   461,   461,   461,   462,   462,   462,   462,
     462,   462,   463,   463,   463,   464,   464,   464,   464,   464,
     464,   464,   464,   464,   464,   464,   465,   465,   466,   466,
     467,   467,   468,   468,   468,   468,   469,   469,   470,   470,
     471,   471,   472,   472,   473,   474,   474,   474,   474,   475,
     475,   476,   476,   477,   477,   477,   477,   478,   478,   478,
     479,   479,   479,   480,   480,   481,   481,   482,   483,   484,
     484,   485,   485,   485,   485,   485,   485,   485,   485,   485,
     485,   485,   485,   486,   486
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     3,     1,     1,     2,
       3,     4,     1,     2,     3,     4,     1,     2,     3,     4,
       1,     3,     1,     3,     2,     1,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     3,     5,     1,
       1,     1,     0,     9,     0,    10,     5,     0,    13,     0,
       5,     3,     3,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     3,     2,     0,     4,     9,     0,     0,     4,
       2,     0,     1,     0,     1,     0,     9,     0,    10,     0,
      11,     0,     9,     0,    10,     0,     8,     0,     9,     0,
       7,     0,     8,     0,     7,     0,     8,     1,     1,     1,
       1,     1,     2,     3,     3,     2,     2,     0,     2,     0,
       2,     0,     1,     3,     1,     3,     2,     0,     1,     2,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     5,     4,     0,     1,     1,
       4,     0,     5,     0,     2,     0,     3,     0,     7,     8,
       6,     2,     5,     6,     4,     0,     4,     5,     7,     6,
       6,     7,     9,     8,     6,     7,     5,     2,     4,     5,
       3,     0,     3,     4,     6,     5,     5,     6,     8,     7,
       2,     0,     1,     2,     2,     3,     4,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     2,
       3,     2,     0,     0,     4,     0,     5,     2,     2,     2,
       0,    10,     0,    11,     3,     3,     3,     4,     4,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     2,     1,     1,     4,     6,     1,     1,
       4,     1,     1,     3,     2,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     3,
       4,     3,     3,     3,     2,     1,     1,     3,     3,     1,
       1,     0,     1,     2,     4,     3,     6,     2,     3,     6,
       1,     1,     1,     1,     1,     6,     3,     4,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     3,     2,     1,     5,     0,     0,    11,     0,    12,
       0,     3,     0,     4,     0,     6,     0,     7,     0,     5,
       2,     2,     4,     1,     1,     5,     3,     5,     3,     2,
       0,     2,     0,     4,     4,     3,     4,     4,     4,     4,
       1,     1,     1,     1,     3,     2,     3,     4,     2,     3,
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
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     2,     2,     4,
       3,     4,     2,     0,     5,     3,     3,     1,     3,     1,
       2,     0,     5,     3,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     1,     1,     1,     3,
       1,     1,     1,     3,     1,     1,     3,     3,     2,     4,
       2,     4,     5,     5,     5,     5,     1,     1,     1,     1,
       1,     1,     3,     3,     4,     4,     3,     1,     1,     1,
       1,     3,     1,     4,     3,     1,     1,     1,     1,     1,
       3,     3,     4,     4,     3,     1,     1,     7,     9,     7,
       6,     8,     1,     2,     4,     4,     1,     1,     4,     1,
       0,     1,     2,     1,     1,     1,     3,     3,     3,     0,
       1,     1,     3,     3,     2,     3,     6,     0,     1,     4,
       2,     0,     5,     3,     3,     1,     6,     4,     4,     2,
       2,     0,     5,     3,     3,     1,     2,     0,     5,     3,
       3,     1,     2,     2,     1,     2,     1,     4,     3,     3,
       6,     3,     1,     1,     1,     4,     4,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     6,     7,     1,     2,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     2,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     3,     1,     2,     0,     4,     1,     3,
       2,     2,     2,     1,     1,     1,     1,     3,     4,     6,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   372,     0,   760,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   851,     0,
     839,   643,     0,   649,   650,   651,    22,   707,   827,   100,
     101,   652,     0,    81,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   134,     0,     0,     0,     0,     0,     0,
     339,   340,   341,   344,   343,   342,     0,     0,     0,     0,
     161,     0,     0,     0,   656,   658,   659,   653,   654,     0,
       0,     0,   660,   655,     0,   631,    24,    25,    26,    28,
      27,     0,   657,     0,     0,     0,     0,   661,   345,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   464,     0,    99,    71,   831,   644,     0,     0,
       4,    60,    62,    65,   706,     0,   630,     0,     6,   133,
       7,     9,     8,    10,     0,     0,   337,   382,     0,     0,
       0,     0,     0,     0,     0,   380,   815,   816,   450,   449,
     366,   453,     0,     0,   365,   787,   632,     0,   709,   448,
     336,   790,   791,   802,   381,     0,     0,   384,   383,   788,
     789,   786,   822,   826,     0,   438,   708,    11,   344,   343,
     342,     0,     0,    28,    60,   133,     0,   895,   381,   894,
       0,   892,   891,   452,     0,   373,   377,     0,     0,   422,
     423,   424,   425,   447,   445,   444,   443,   442,   441,   440,
     439,   827,   652,   634,     0,     0,   911,   808,   632,     0,
     633,   404,     0,   402,     0,   855,     0,   716,   364,   639,
       0,   911,   638,     0,   648,   633,   834,   835,   841,   833,
     640,     0,     0,   642,   446,     0,     0,     0,     0,   369,
       0,    79,   371,     0,     0,    85,    87,     0,     0,    89,
       0,     0,     0,   944,   945,   950,     0,     0,    60,   943,
       0,   946,     0,     0,     0,    91,     0,     0,     0,     0,
     124,     0,     0,     0,     0,     0,     0,    43,    48,   250,
       0,     0,   249,     0,   165,     0,   162,   255,     0,     0,
       0,     0,     0,   908,   149,   159,   847,   851,     0,   876,
       0,   663,     0,     0,     0,   874,     0,    16,     0,    64,
     141,   153,   160,   537,   480,     0,   900,   462,   466,   468,
     764,   382,     0,   380,   381,   383,     0,     0,   645,     0,
     646,     0,     0,     0,   123,     0,     0,    67,   241,     0,
      21,   132,     0,   158,   145,   157,   342,   345,   133,   338,
     114,   115,   116,   117,   118,   120,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   839,     0,   113,   830,   830,   121,   861,     0,     0,
       0,     0,     0,     0,     0,     0,   335,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     403,   401,   765,   766,     0,   830,     0,   778,   241,   241,
     830,     0,   832,   823,   847,     0,   133,     0,     0,    93,
       0,   762,   757,   716,     0,   382,   380,     0,   859,     0,
     485,   715,   850,   382,   380,   381,    67,     0,   241,   363,
       0,   780,   641,     0,    71,   201,     0,   461,     0,    96,
       0,     0,   370,     0,     0,     0,     0,     0,    88,   112,
      90,   941,   942,     0,   936,     0,     0,     0,     0,   907,
       0,   119,    92,   122,     0,     0,     0,     0,     0,     0,
       0,   495,     0,   502,   504,   505,   506,   507,   508,   509,
     500,   522,   523,    71,     0,   109,   111,     0,     0,    45,
      52,     0,     0,    47,    56,    49,     0,    18,     0,     0,
     251,     0,    94,   164,   163,     0,     0,    95,   896,     0,
       0,   382,   380,   381,   384,   383,     0,   929,   171,     0,
     848,     0,     0,    97,     0,     0,   662,   875,   707,     0,
       0,   873,   712,   872,    63,     5,    13,    14,     0,   169,
       0,     0,   473,     0,     0,     0,   716,     0,     0,   635,
     474,     0,     0,     0,     0,   764,    71,     0,   718,   763,
     954,   362,   435,   796,   814,    76,    70,    72,    73,    74,
      75,   336,     0,   451,   710,   711,    61,   716,     0,   912,
       0,     0,     0,   718,   242,     0,   456,   135,   167,     0,
     407,   409,   408,     0,     0,   405,   406,   410,   412,   411,
     427,   426,   429,   428,   430,   432,   433,   431,   421,   420,
     414,   415,   413,   416,   417,   419,   434,   418,   829,     0,
       0,   865,     0,   716,   899,     0,   898,   793,   822,   151,
     143,   155,     0,   900,   147,   133,   372,     0,   375,   378,
     386,   496,   400,   399,   398,   397,   396,   395,   394,   393,
     392,   391,   390,   389,   768,     0,   767,   770,   792,   774,
     911,   771,     0,     0,     0,     0,     0,     0,     0,     0,
     893,   374,   755,   759,   715,   761,     0,   636,   911,     0,
     854,     0,   853,     0,   636,   838,   837,     0,     0,   767,
     770,   836,   771,   367,   203,   205,    71,   471,   470,   368,
       0,    71,   185,    80,   371,     0,     0,     0,     0,     0,
     197,   197,    86,     0,     0,     0,     0,   934,   716,     0,
     918,     0,     0,     0,     0,     0,   714,     0,   631,     0,
       0,    65,   665,   630,   670,     0,   664,    69,   669,   911,
     947,     0,     0,   512,     0,     0,   518,   515,   516,   524,
       0,   503,   498,     0,   501,     0,     0,     0,    53,    19,
       0,     0,    57,    20,     0,     0,     0,    42,    50,     0,
     248,   256,   253,     0,     0,   885,   890,   887,   886,   889,
     888,    12,   927,   928,     0,     0,     0,     0,   847,   844,
       0,   484,   884,   883,   882,     0,   878,     0,   879,   881,
       0,     5,     0,     0,     0,   531,   532,   540,   539,     0,
     380,     0,   715,   479,   483,     0,     0,   901,     0,   463,
       0,     0,   919,   764,   227,   953,     0,     0,   779,     0,
     828,   715,   914,   910,   243,   244,   629,   717,   240,     0,
     764,     0,     0,   169,   458,   137,   437,     0,   488,   489,
       0,   486,   715,   860,     0,     0,   241,   171,     0,   169,
       0,     0,   167,     0,   839,   387,     0,     0,   776,   777,
     794,   795,   824,   825,     0,     0,     0,   743,   723,   724,
     725,   732,     0,     0,     0,   736,   734,   735,   749,   716,
       0,   757,   858,   857,     0,     0,   781,   647,     0,   207,
       0,     0,    77,     0,     0,     0,     0,     0,     0,     0,
     177,   178,   189,     0,    71,   187,   106,   197,     0,   197,
       0,     0,   948,     0,     0,     0,   715,   935,   937,   917,
     716,   916,     0,   716,   691,   692,   689,   690,   722,     0,
     716,   714,     0,   482,     0,     0,   867,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   940,   497,     0,     0,     0,   520,
     521,   519,     0,     0,   499,     0,   125,     0,   128,   110,
       0,    44,    54,     0,    46,    58,    51,   252,     0,   897,
      98,   929,   909,   924,   170,   172,   262,     0,     0,   845,
       0,   877,     0,    17,     0,   900,   168,   262,     0,     0,
     636,   476,     0,   898,   637,   902,     0,   919,   469,     0,
       0,   954,     0,   232,   230,   770,   782,   911,   770,   783,
     913,     0,     0,   245,    68,     0,   764,   166,     0,   764,
       0,   436,   864,   863,     0,   241,     0,     0,     0,     0,
       0,     0,   169,   139,   648,   769,   241,     0,   728,   729,
     730,   731,   737,   738,   747,     0,   716,     0,   743,     0,
     727,   751,   715,   754,   756,   758,     0,   852,   769,     0,
       0,     0,     0,   204,   472,    82,     0,   371,   177,   179,
     847,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     191,     0,     0,   930,     0,   933,   715,     0,     0,     0,
     667,   715,   713,     0,   704,     0,   716,     0,   671,   705,
     703,   871,     0,   716,   674,   676,   675,     0,     0,   672,
     673,   677,   679,   678,   694,   693,   696,   695,   697,   699,
     700,   698,   687,   686,   681,   682,   680,   683,   684,   685,
     688,   939,   510,     0,   511,   517,   525,   526,     0,    71,
      55,    59,   254,     0,     0,     0,   926,     0,   336,   849,
     847,   376,   379,   385,     0,    15,     0,   336,   543,     0,
       0,   545,   538,   541,     0,   536,     0,   904,     0,   920,
     465,     0,   233,     0,     0,   228,     0,   247,   246,   919,
       0,   262,     0,   764,     0,   241,     0,   820,   262,   900,
     262,   903,     0,     0,     0,   388,     0,     0,   740,   715,
     742,   733,     0,   726,     0,     0,   716,   748,   856,     0,
      71,     0,   200,   186,     0,     0,     0,   176,   102,   190,
       0,     0,   193,     0,   198,   199,    71,   192,   949,   931,
       0,   915,     0,   952,   721,   720,   666,     0,   715,   481,
     668,     0,   487,   715,   866,   702,     0,     0,     0,     0,
     923,   921,   922,   173,     0,     0,     0,   343,   334,     0,
       0,     0,   150,   261,   263,     0,   333,     0,     0,     0,
     900,   336,     0,   880,   258,   154,   534,     0,     0,   475,
     467,     0,   236,   226,     0,   229,   235,   241,   455,   919,
     336,   919,     0,   862,     0,   819,   336,     0,   336,   905,
     262,   764,   817,   746,   745,   739,     0,   741,   715,   750,
      71,   206,    78,    83,   104,   180,     0,   188,   194,    71,
     196,   932,     0,     0,   478,     0,   870,   869,   701,     0,
      71,   129,   925,     0,     0,     0,     0,     0,   174,     0,
     900,     0,   299,   295,   301,   631,    28,     0,   289,     0,
     294,   298,   311,     0,   309,   314,     0,   313,     0,   312,
       0,   133,   265,     0,   267,     0,   268,   269,     0,     0,
     846,     0,   535,   533,   544,   542,   237,     0,     0,   224,
     234,     0,     0,     0,     0,   146,   455,   919,   821,   152,
     258,   156,   336,     0,     0,   753,     0,   202,     0,     0,
      71,   183,   103,   195,   951,   719,     0,     0,     0,     0,
       0,     0,   361,     0,     0,   279,   283,   358,   359,   293,
       0,     0,     0,   274,   595,   594,   591,   593,   592,   612,
     614,   613,   583,   554,   555,   573,   589,   588,   550,   560,
     561,   563,   562,   582,   566,   564,   565,   567,   568,   569,
     570,   571,   572,   574,   575,   576,   577,   578,   579,   581,
     580,   551,   552,   553,   556,   557,   559,   597,   598,   607,
     606,   605,   604,   603,   602,   590,   609,   599,   600,   601,
     584,   585,   586,   587,   610,   611,   615,   617,   616,   618,
     619,   596,   621,   620,   623,   625,   624,   558,   628,   626,
     627,   622,   608,   549,   306,   546,     0,   275,   327,   328,
     326,   319,     0,   320,   276,   353,     0,     0,     0,     0,
     357,     0,   133,   142,   257,     0,     0,     0,   225,   239,
     818,     0,    71,   329,    71,   136,     0,     0,     0,   148,
     919,   744,     0,    71,   181,    84,   105,     0,   477,   868,
     513,   127,   277,   278,   356,   175,     0,     0,   716,     0,
     302,   290,     0,     0,     0,   308,   310,     0,     0,   315,
     322,   323,   321,     0,     0,   264,     0,     0,     0,   360,
       0,   259,     0,   238,     0,   529,   718,     0,     0,    71,
     138,   144,     0,   752,     0,     0,     0,   107,   280,    60,
       0,   281,   282,     0,     0,   296,   715,   304,   300,   305,
     547,   548,     0,   291,   324,   325,   317,   318,   316,   354,
     351,   270,   266,   355,     0,   260,   530,   717,     0,   457,
     330,     0,   140,     0,   184,   514,     0,   131,     0,   336,
       0,   303,   307,     0,   764,   272,     0,   527,   454,   459,
     182,     0,     0,   108,   287,     0,   335,   297,   352,     0,
     718,   347,   764,   528,     0,   130,     0,     0,   286,   919,
     764,   211,   348,   349,   350,   954,   346,     0,     0,     0,
     285,     0,   347,     0,   919,     0,   284,   331,    71,   271,
     954,     0,   216,   214,     0,    71,     0,     0,   217,     0,
       0,   212,   273,     0,   332,     0,   220,   210,     0,   213,
     219,   126,   221,     0,     0,   208,   218,     0,   209,   223,
     222
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   831,   565,   184,   286,   518,
     522,   287,   519,   523,   122,   123,   124,   125,   126,   127,
     336,   596,   597,   471,   250,  1458,   477,  1376,  1459,  1697,
     787,   281,   513,  1657,  1019,  1199,  1713,   352,   185,   598,
     871,  1080,  1254,   131,   568,   888,   599,   618,   892,   548,
     887,   600,   569,   889,   354,   304,   321,   134,   873,   834,
     817,  1034,  1397,  1132,   940,  1605,  1462,   733,   946,   476,
     742,   948,  1286,   725,   929,   932,  1121,  1719,  1720,   587,
     588,   612,   613,   291,   292,   298,  1431,  1584,  1585,  1208,
    1323,  1420,  1578,  1704,  1722,  1616,  1661,  1662,  1663,  1407,
    1408,  1409,  1410,  1618,  1619,  1625,  1673,  1413,  1414,  1418,
    1571,  1572,  1573,  1595,  1749,  1324,  1325,   186,   136,  1735,
    1736,  1576,  1327,  1328,  1329,  1330,   137,   243,   472,   473,
     138,   139,   140,   141,   142,   143,   144,   145,  1443,   146,
     870,  1079,   147,   247,   584,   330,   585,   586,   467,   575,
     576,  1156,   577,  1157,   148,   149,   150,   764,   151,   152,
     278,   153,   279,   501,   502,   503,   504,   505,   506,   507,
     508,   509,   777,   778,  1011,   510,   511,   512,   784,  1646,
     154,   570,  1433,   571,  1048,   839,  1225,  1222,  1564,  1565,
     155,   156,   157,   234,   244,   339,   459,   158,   968,   768,
     159,   969,   862,   854,   970,   916,  1102,   917,  1104,  1105,
    1106,   919,  1265,  1266,   920,   703,   443,   197,   198,   601,
     590,   424,  1065,  1066,   689,   690,   858,   161,   236,   162,
     163,   188,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   649,   174,   240,   241,   551,   226,   227,   652,   653,
    1162,  1163,   314,   315,   825,   175,   539,   176,   583,   177,
    1586,   305,   347,   607,   608,   962,  1060,  1206,   814,   815,
     747,   748,   749,   271,   272,   770,   273,   856
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1408
static const yytype_int16 yypact[] =
{
   -1408,   125, -1408, -1408,  5024, 13058, 13058,   -13, 13058, 13058,
   13058, 10998, 13058, -1408, 13058, 13058, 13058, 13058, 13058, 13058,
   13058, 13058, 13058, 13058, 13058, 13058, 15625, 15625, 11204, 13058,
   15751,    -2,     5, -1408, -1408, -1408, -1408, -1408,   191, -1408,
   -1408,   141, 13058, -1408,     5,   161,   253,   292, -1408,     5,
   11410, 16743, 11616, -1408, 14539,  9968,   328, 13058, 16477,    17,
   -1408, -1408, -1408,    77,   532,    56,   349,   376,   383,   385,
   -1408, 16743,   404,   440, -1408, -1408, -1408, -1408, -1408, 13058,
     403,  1133, -1408, -1408, 16743, -1408, -1408, -1408, -1408, 16743,
   -1408, 16743, -1408,   438,   452, 16743, 16743, -1408,   307, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, 13058, -1408, -1408,   464,   482,   504,   504,
   -1408,   237,   519,   484, -1408,   486, -1408,    67, -1408,   646,
   -1408, -1408, -1408, -1408, 16555,   498, -1408, -1408,   497,   499,
     509,   516,   527,   566, 13740, -1408, -1408, -1408, -1408,    28,
   -1408,   704,   705,   579, -1408,    62,   583,   640,   290, -1408,
    1009,    64, -1408, -1408,  3222,    33,   592,   111, -1408,    51,
     128,   593,    31, -1408,    40, -1408,   727, -1408, -1408, -1408,
     647,   596,   656, -1408, -1408,   646,   498, 17018,  4115, 17018,
   13058, 17018, 17018, 10157,   599, 16095, 10157,   777, 16743,   758,
     758,   407,   758,   758,   758,   758,   758,   758,   758,   758,
     758, -1408, -1408, -1408,    84, 13058,   655, -1408, -1408,   675,
     629,   270,   630,   270, 15625, 16139,   626,   824, -1408,   647,
   13058,   655,   684,   685,   649, -1408,   132, -1408, -1408, -1408,
     270,    33, 11822, -1408, -1408, 13058,  8526,   831,    83, 17018,
    9556, -1408, 13058, 13058, 16743, -1408, -1408, 13784,   644, -1408,
   13828, 14539, 14539,   670, -1408, -1408,   651, 14277,    76,   694,
     837, -1408,   706, 16743,   775, -1408,   657, 13872,   660,   840,
   -1408,    19, 13916, 16609, 16624, 16743,    86, -1408,   330, -1408,
   15100,    89, -1408,   730, -1408,   738, -1408,   850,    92, 15625,
   15625, 13058,   667,   697, -1408, -1408, 15231, 11204,    93,   350,
     520, -1408, 13264, 15625,   462, -1408, 16743, -1408,   377,   519,
   -1408, -1408, -1408, -1408, 15877,   855,   774, -1408, -1408, -1408,
      41,   673, 17018,   674,  2246,   676,  5230, 13058,   515,   677,
     591,   515,   390,   372, -1408, 16743, 14539,   680, 10174, 14539,
   -1408, -1408, 15800, -1408, -1408, -1408, -1408, -1408,   646, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, 13058, 13058, 13058, 12028,
   13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058,
   13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058,
   13058, 15751, 13058, -1408, 13058, 13058, -1408, 13058,  2647, 16743,
   16743, 16743, 16743, 16743, 16555,   778,   823,  9762, 13058, 13058,
   13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058, 13058,
   -1408, -1408, -1408, -1408,  4069, 13058, 13058, -1408, 10174, 10174,
   13058, 13058,   464,   145, 15231,   681,   646, 12234, 13960, -1408,
   13058, -1408,   686,   874,   739,   688,   690, 14610,   270, 12440,
   -1408, 12646, -1408,   693,   695,  2377, -1408,   121, 10174, -1408,
    4318, -1408, -1408, 14004, -1408, -1408, 10380, -1408, 13058, -1408,
     800,  8732,   888,   699, 16929,   884,    74,    48, -1408, -1408,
   -1408, -1408, -1408, 14539,  4303,   702,   896, 14970, 16743, -1408,
     722, -1408, -1408, -1408,   830, 13058,   832,   833, 13058, 13058,
   13058, -1408,   840, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
     725, -1408, -1408, -1408,   716, -1408, -1408, 16743,   715,   908,
     352, 16743,   720,   913,   357,   374, 16674, -1408, 16743, 13058,
     270,    17, -1408, -1408, -1408, 14970,   844, -1408,   270,   117,
     118,   724,   729,  2543,   156,   732,   726,   211,   798,   734,
     270,   122,   736, -1408, 16542, 16743, -1408, -1408,   868,  2655,
      53, -1408, -1408, -1408,   519, -1408, -1408, -1408,   915,   815,
     776,   264,   797, 13058,   464,   818,   941,   751,   803,   145,
   -1408, 14539, 14539,   942,   831,    41, -1408,   760,   946, -1408,
   14539,    44,   375,   142, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408,   639,  2748, -1408, -1408, -1408, -1408,   949,   792, -1408,
   15625, 13058,   772,   964, 17018,   960, -1408, -1408,   849, 15926,
   10569, 11601, 10157, 13058, 16974, 12835, 14135, 15165,  2478, 12006,
   15426, 15426, 15426, 15426,  1916,  1916,  1916,  1916,   765,   765,
     733,   733,   733,   407,   407,   407, -1408,   758, 17018,   771,
     781, 16196,   779,   969, -1408, 13058,   297,   315,   145, -1408,
   -1408, -1408,   970,   774, -1408,   646, 13058, 15363, -1408, -1408,
   10157, -1408, 10157, 10157, 10157, 10157, 10157, 10157, 10157, 10157,
   10157, 10157, 10157, 10157, -1408, 13058,   341,   147, -1408, -1408,
     655,   355,   782,  3333,   789,   791,   787,  3536,   123,   795,
   -1408, 17018,  1539, -1408, 16743, -1408,    44,   347,   655, 15625,
   17018, 15625, 16240,    44,   270,   148,   846,   799, 13058, -1408,
     149, -1408, -1408, -1408,  8320,   492, -1408, -1408, 17018, 17018,
       5, -1408, -1408, -1408, 13058,   901, 14819, 14970, 16743,  8938,
     804,   805, -1408,    71,   914,   880,   863, -1408,  1005,   816,
   14352, 14539, 14970, 14970, 14970, 14970, 14970,   819,   875,   827,
   14970,    34, -1408,   878, -1408,   829, -1408, 17104, -1408,   316,
   -1408, 13058,   851, 17018,   852,  1024, 10983,  1031, -1408, 17018,
   14090, -1408,   725,   965, -1408,  5436, 16490,   841,   416, -1408,
   16609, 16743,   418, -1408, 16624, 16743, 16743, -1408, -1408,  3981,
   -1408, 17104,  1034, 15625,   854, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408,    80, 16743, 16490,   843, 15231, 15494,
    1036, -1408, -1408, -1408, -1408,   845, -1408, 13058, -1408, -1408,
    4610, -1408, 14539, 16490,   856, -1408, -1408, -1408, -1408,  1038,
     853, 13058, 15877, -1408, -1408, 15674,   858, -1408, 14539, -1408,
     859,  5642,  1027,    32, -1408, -1408,   363,  4069, -1408,  4318,
   -1408, 14539, -1408, -1408,   270, 17018, -1408, 10586, -1408, 14970,
      42,   861, 16490,   815, -1408, -1408, 12423, 13058, -1408, -1408,
   13058, -1408, 13058, -1408,  4264,   865, 10174,   798,  1040,   815,
   14539,  1054,   849, 16743, 15751,   270,  4312,   872, -1408, -1408,
     146,   873, -1408, -1408,  1062, 16420, 16420,  1539, -1408, -1408,
   -1408,  1029,   879,    87,   883, -1408, -1408, -1408, -1408,  1075,
     885,   686,   270,   270, 12852,  4318, -1408, -1408,  4393,   549,
       5,  9556, -1408,  5848,   886,  6054,   887, 14819, 15625,   890,
     955,   270, 17104,  1076, -1408, -1408, -1408, -1408,   614, -1408,
      36, 14539, -1408,   962, 14539, 16743,  4303, -1408, -1408, -1408,
    1085, -1408,   895,   949,   654,   654,  1033,  1033, 16341,   894,
    1087, 14970, 16743, 15877, 14048, 16728, 14970, 14970, 14970, 14970,
   14689, 14970, 14970, 14970, 14970, 14970, 14970, 14970, 14970, 14970,
   14970, 14970, 14970, 14970, 14970, 14970, 14970, 14970, 14970, 14970,
   14970, 14970, 14970, 16743, -1408, 17018, 13058, 13058, 13058, -1408,
   -1408, -1408, 13058, 13058, -1408,   840, -1408,  1023, -1408, -1408,
   16743, -1408, -1408, 16743, -1408, -1408, -1408, -1408, 14970,   270,
   -1408,   211, -1408,   538,  1094, -1408, -1408,   127,   905,   270,
   10792, -1408,   110, -1408,  4818,   774,  1094, -1408,   463,   308,
   -1408, 17018,   975, -1408, -1408, -1408,   906,  1027, -1408, 14539,
     831, 14539,    14,  1095,  1032,   150, -1408,   655,   152, -1408,
   -1408, 15625, 13058, 17018, 17104,   910,    42, -1408,   911,    42,
     909, 12423, 17018, 16297,   917, 10174,   912,   916, 14539,   919,
     932, 14539,   815, -1408,   649,   388, 10174, 13058, -1408, -1408,
   -1408, -1408, -1408, -1408,   983,   930,  1129,  1053,  1539,   995,
   -1408, 15877,  1539, -1408, -1408, -1408, 15625, 17018, -1408,     5,
    1114,  1072,  9556, -1408, -1408, -1408,   945, 13058,   955,   270,
   15231, 14819,   947, 14970,  6260,   624,   950, 13058,    54,   354,
   -1408,   976, 14539, -1408,  1019, -1408, 14398,  1123,   954, 14970,
   -1408, 14970, -1408,   961, -1408,  1035,  1151,   963, -1408, -1408,
   -1408, 16398,   966,  1155, 11187, 11807, 12217, 14970, 17062, 13507,
   14885, 15296,  4154, 14169, 15557, 15557, 15557, 15557,  1987,  1987,
    1987,  1987,   920,   920,   654,   654,   654,  1033,  1033,  1033,
    1033, -1408, 17018, 13249, 17018, -1408, 17018, -1408,   967, -1408,
   -1408, -1408, 17104, 16743, 14539, 14539, -1408, 16490,   689, -1408,
   15231, -1408, -1408, 10157,   971, -1408,   973,  1884, -1408,    65,
   13058, -1408, -1408, -1408, 13058, -1408, 13058, -1408,   831, -1408,
   -1408,   394,  1162,  1086, 13058, -1408,   978,   270, 17018,  1027,
     982, -1408,   984,    42, 13058, 10174,   986, -1408, -1408,   774,
   -1408, -1408,   977,   981,   989, -1408,   992,  1539, -1408,  1539,
   -1408, -1408,   993, -1408,  1058,   994,  1186, -1408,   270,  1167,
   -1408,   997, -1408, -1408,  1003,  1006,   153, -1408, -1408, 17104,
    1004,  1007, -1408, 13696, -1408, -1408, -1408, -1408, -1408, -1408,
   14539, -1408, 14539, -1408, 17104, 16442, -1408, 14970, 15877, -1408,
   -1408, 14970, -1408, 14970, -1408, 12629, 14970, 13058,  1011,  6466,
     538, -1408, -1408, -1408,   560, 14471, 16490,  1100, -1408,  3073,
    1050, 14892, -1408, -1408, -1408,   778, 14210,    97,    98,  1010,
     774,   823,   157, -1408, -1408, -1408,  1059,  4527, 13454, 17018,
   -1408,   230,  1202,  1139, 13058, -1408, 17018, 10174,  1108,  1027,
    1903,  1027,  1022, 17018,  1025, -1408,  2002,  1021,  2022, -1408,
   -1408,    42, -1408, -1408,  1093, -1408,  1539, -1408, 15877, -1408,
   -1408,  8320, -1408, -1408, -1408, -1408,  9144, -1408, -1408, -1408,
    8320, -1408,  1026, 14970, 17104,  1097, 17104, 16499, 12629, 13043,
   -1408, -1408, -1408, 16490, 16490, 16743,  1216,    70, -1408, 14471,
     774,  3369, -1408,  1063, -1408,   100,  1039,   102, -1408, 13510,
   -1408, -1408, -1408,   103, -1408, -1408,  3966, -1408,  1037, -1408,
    1154,   646, -1408, 14539, -1408, 14539, -1408, -1408,  1222,   778,
   -1408,  2835, -1408, -1408, -1408, -1408,  1223,  1163, 13058, -1408,
   17018,  1043,  1046,  1044,   458, -1408,  1108,  1027, -1408, -1408,
   -1408, -1408,  2250,  1049,  1539, -1408,  1121,  8320,  9350,  9144,
   -1408, -1408, -1408,  8320, -1408, 17104, 14970, 14970, 13058,  6672,
    1052,  1055, -1408, 14970, 16490, -1408, -1408, -1408, -1408, -1408,
   14539,  1369,  3073, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408,   162, -1408,  1050, -1408, -1408, -1408,
   -1408, -1408,   131,   643, -1408,  1240,   104, 16743,  1154,  1241,
   -1408, 14539,   646, -1408, -1408,  1056,  1244, 13058, -1408, 17018,
   -1408,    88, -1408, -1408, -1408, -1408,  1057,   458, 14089, -1408,
    1027, -1408,  1539, -1408, -1408, -1408, -1408,  6878, 17104, 17104,
   11395, -1408, -1408, -1408, 17104, -1408,  3560,    82,  1251,  1060,
   -1408, -1408, 14970, 13510, 13510,  1206, -1408,  3966,  3966,   645,
   -1408, -1408, -1408, 14970,  1187, -1408,  1071,   105, 14970, -1408,
   16743, -1408, 14970, 17018,  1192, -1408,  1263,  7084,  7290, -1408,
   -1408, -1408,   458, -1408,  7496,  1074,  1194,  1166, -1408,  1180,
    1130, -1408, -1408,  1182, 14539, -1408,  1369, -1408, -1408, 17104,
   -1408, -1408,  1119, -1408,  1249, -1408, -1408, -1408, -1408, 17104,
    1270, -1408, -1408, 17104,  1089, 17104, -1408,   144,  1096, -1408,
   -1408,  7702, -1408,  1084, -1408, -1408,  1090,  1118, 16743,   823,
    1125, -1408, -1408, 14970,    59, -1408,  1219, -1408, -1408, -1408,
   -1408, 16490,   841, -1408,  1134, 16743,   526, -1408, 17104,  1103,
    1296,   627,    59, -1408,  1227, -1408, 16490,  1106, -1408,  1027,
      60, -1408, -1408, -1408, -1408, 14539, -1408,  1110,  1111,   107,
   -1408,   535,   627,   402,  1027,  1113, -1408, -1408, -1408, -1408,
   14539,   254,  1294,  1233,   535, -1408,  7908,   413,  1299,  1252,
   13058, -1408, -1408,  8114, -1408,   267,  1316,  1254, 13058, -1408,
   17018, -1408,  1320,  1258, 13058, -1408, 17018, 13058, -1408, 17018,
   17018
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1408, -1408, -1408,  -492, -1408, -1408, -1408,   224, -1408, -1408,
   -1408,   814,   551,   550,   112,  1345,  2697, -1408,  2333, -1408,
      68, -1408,    30, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408,  -369, -1408, -1408,  -157,   114,    22,
   -1408, -1408, -1408, -1408, -1408, -1408,    29, -1408, -1408, -1408,
   -1408,    35, -1408, -1408,   943,   951,   944,  -101,   456,  -807,
     465,   518,  -377,   225,  -866, -1408,  -104, -1408, -1408, -1408,
   -1408,  -666,    73, -1408, -1408, -1408, -1408,  -362, -1408,  -542,
   -1408,  -388, -1408, -1408,   826, -1408,   -89, -1408, -1408,  -959,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
    -120, -1408,   -37, -1408, -1408, -1408, -1408, -1408,  -198, -1408,
      63,  -881, -1408, -1407,  -376, -1408,  -158,    57,  -114,  -363,
   -1408,  -201, -1408, -1408, -1408,    66,    11,     0,   476,  -665,
    -365, -1408, -1408,   -19, -1408, -1408,    -5,   -50,   -65, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,  -538,  -782,
   -1408, -1408, -1408, -1408, -1408,  1944, -1408, -1408, -1408, -1408,
     979, -1408, -1408,   365, -1408,   889, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408,   370, -1408,   891, -1408, -1408,   603, -1408,
     339, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,  -874,
   -1408,  2139,  1174,  -329, -1408, -1408,   295,  3117,  2909, -1408,
   -1408,   419,  -146,  -605, -1408, -1408,   488,   285,  -667,   287,
   -1408, -1408, -1408, -1408, -1408,   475, -1408, -1408, -1408,    61,
    -826,  -106,  -360,  -357, -1408,   540,  -108, -1408, -1408,    24,
      26,   432, -1408, -1408,   243,    58, -1408, -1408,   138,  -136,
   -1408,  -206, -1408, -1408, -1408,  -379,  1098, -1408, -1408, -1408,
   -1408, -1408,   642,   434, -1408, -1408,  1101,  -271,  -612, -1408,
     -23,   -66,  -173,    12,   653, -1408, -1009,    94, -1408,   378,
     454, -1408, -1408, -1408, -1408,   405,   719, -1039
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -939
static const yytype_int16 yytable[] =
{
     187,   189,   405,   191,   192,   193,   195,   196,   868,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   359,  1231,   225,   228,   322,   130,  1061,   435,   325,
     326,   270,   246,   132,   128,   918,   276,   249,   433,   133,
     694,   695,   668,   850,   251,   257,   849,   260,  1228,   255,
     277,   891,   282,   580,   237,   698,   238,   427,   456,   404,
    1052,   135,   646,   333,   687,   160,  1078,   688,   355,   936,
     717,  1128,   359,   830,   249,   950,   349,    13,   740,  1474,
     951,   452,  1089,   738,  1284,  -906,    13,    13,  1217,  1031,
    -906,  1664,   468,  1232,  -490,   526,   289,   430,   531,   425,
     720,   536,   468,   721,    13,    13,  1423,  1425,   332,  -292,
     425,  1482,  1566,  1634,  1634,  1336,  1474,  -798,   129,   211,
     366,   367,   368,  1644,   331,     3,   803,   803,  -797,   514,
    -800,   819,   819,   461,   460,   297,   819,  1137,  1138,   369,
    1627,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   819,   391,  1628,   446,   819,  1645,   239,   444,
     288,   335,  1109,   422,   423,   392,  1622,  -492,  1233,  1706,
     454,  -635,   422,   423,  -911,   438,   190,   515,   579,   650,
    1650,  1155,  1623,   318,  -799,   293,   319,   242,  -840,  -460,
     211,   619,   294,   555,   245,   346,  -634,  -493,  -804,  1624,
     332,   430,  -803,  -801,  -843,  -842,  -784,   406,  -785,   692,
     323,   290,  -492,  1707,   696,   332,   445,  -808,   121,  -717,
    1348,   311,  -717,   431,  1110,   426,  -911,   463,  -231,  1140,
     463,   453,  -231,   952,   432,  1692,   426,   249,   474,   358,
     741,   542,  1032,  -798,  1665,  1285,   829,   812,   813,  -215,
    -717,  -807,   658,  -806,  -797,  1277,  -800,   344,   350,   222,
     222,  1475,  1476,   335,   739,   258,   465,  -906,   268,   699,
     470,  1135,  1350,  1139,   469,  1253,   617,   527,   658,  1356,
     532,  1358,   359,   537,   553,   303,   332,   705,  1424,  1426,
     436,  -292,   225,  1483,  1567,  1635,  1682,   559,  1746,  1436,
     429,   658,   541,   320,   836,   303,  1214,   804,   805,   303,
     303,   658,   820,   904,   658,   116,   615,  1209,  1075,  1264,
    -799,  1629,   602,  1758,  -840,   322,   662,   663,   355,  1044,
    1444,  -810,  1446,   614,  -804,  -813,  1772,   431,  -803,  -801,
    -843,  -842,  -784,  1375,  -785,   429,   809,  1430,   303,   545,
     252,   620,   621,   622,   624,   625,   626,   627,   628,   629,
     630,   631,   632,   633,   634,   635,   636,   637,   638,   639,
     640,   641,   642,   643,   644,   645,   327,   647,   669,   648,
     648,   589,   651,   135,  1437,   520,   524,   525,  1062,   422,
     423,  1452,   670,   672,   673,   674,   675,   676,   677,   678,
     679,   680,   681,   682,   683,   237,   554,   238,  1759,   837,
     648,   693,   442,   614,   614,   648,   697,   528,   564,  1341,
     843,  1773,   670,  1216,   838,   701,   164,  1751,  1597,  1037,
    -633,  -494,  1063,   405,   710,  1267,   712,   727,  1765,   791,
     129,   309,   253,   614,   795,  1137,  1138,   561,   221,   223,
     391,   728,  1274,   729,  1223,  -637,  1003,   222,   346,   309,
     121,   796,   392,  1342,   121,   561,   422,   423,   475,   422,
     423,  1752,   309,   885,   345,   857,   859,   346,   310,  -809,
     773,   254,  1766,   776,   779,   780,  -911,   489,  1086,  1068,
     404,   732,  1069,   328,   422,   423,   345,   883,   893,   329,
    1224,   345,  -772,  1020,   886,  1023,  1385,   897,   248,   665,
     312,   313,  1230,   840,   799,  -636,  -775,  1064,   345,   239,
     280,   345,   724,   222,  1240,   885,   657,  1242,   312,   313,
    -772,   309,   222,   222,   544,   334,  -811,   561,   299,   222,
     311,   312,   313,   875,  -775,   308,   222,  1287,  1343,  -773,
     121,   309,   691,   930,   931,  1094,  1753,   338,   332,   606,
     345,   580,   345,   268,  -811,   300,   303,  1767,   566,   567,
     605,   785,   301,   309,   302,   657,  1456,  -773,   456,   341,
    1363,  1652,  1364,   323,   309,   716,  1004,   604,   722,   309,
     857,   859,   957,   306,    53,   926,   865,   925,   859,   562,
     312,   313,    60,    61,    62,   178,   179,   356,   876,  1218,
    1119,  1120,   656,   303,   660,   303,   303,   303,   303,   788,
     312,   313,  1219,   792,  -911,  1204,  1205,  1357,   288,   307,
      60,    61,    62,   178,   179,   356,   589,   334,   686,   295,
     884,   324,   312,   313,   851,   346,   448,   296,   406,  1593,
    1594,   195,   455,   312,   313,  1220,   337,   556,   312,   313,
     309,   708,  1630,   345,  1676,  1211,   561,   222,   164,   357,
     896,   351,   164,  -911,   719,   348,  -911,  1393,  1394,  1631,
    1340,  1677,  1632,  1314,  1678,   121,  1743,  1246,   360,  1455,
     361,  1352,   580,   999,  1000,  1001,   579,   357,  1256,   658,
     362,  1757,   769,   928,  1136,  1137,  1138,   363,  1428,  1002,
    1741,   658,   530,   658,  1281,  1137,  1138,  1728,   364,   249,
     934,   538,   538,   543,    13,  1754,  1747,  1748,   550,   312,
     313,  1732,  1733,  1734,   557,   560,  1674,  1675,   563,  1670,
    1671,  1276,   798,    60,    61,    62,    63,    64,   356,   340,
     342,   343,   960,   963,    70,   399,  1005,   365,   164,   945,
     394,   395,   557,  1113,   563,   557,   563,   563,   824,   826,
     396,   135,   388,   389,   390,   397,   391,  1601,  1478,   658,
     398,   428,  -805,  -491,   933,   434,  1315,  -634,   392,   935,
     439,  1316,   401,    60,    61,    62,   178,  1317,   356,  1318,
     316,   385,   386,   387,   388,   389,   390,  1148,   391,  1453,
     357,   441,  1042,   392,  1152,   447,   346,  1093,  -809,   429,
     392,  1332,   450,   451,  -632,   457,  1051,   579,   129,   466,
     580,   483,   135,   303,  -938,   479,  1319,  1320,   458,  1321,
     484,   487,   130,   222,   490,   533,   488,  1354,   491,   132,
     128,   493,  1073,   534,   535,   133,   550,   546,   547,   581,
     357,   582,  1081,   591,   592,  1082,   594,  1083,  1721,   -66,
     616,   614,   603,   704,    53,   702,   556,   135,   706,   730,
     707,   160,  1322,   713,  1236,   714,  1721,   468,   737,   129,
     734,   750,   520,   164,  1742,   751,   524,   771,   135,   772,
     222,   774,   775,   783,   589,   786,   789,   790,   237,  1117,
     238,   793,   794,   802,   806,   816,   915,   811,   921,   807,
    1122,   589,   810,   818,   827,  1653,   821,    60,    61,    62,
     178,   179,   356,   833,   129,   832,   835,  -652,   121,   841,
     842,   844,   222,   845,   222,   853,   848,   852,   861,  1441,
    1260,  1123,   943,   121,   863,   129,   996,   997,   998,   999,
    1000,  1001,   866,   867,   869,   579,   872,   878,   882,   222,
     481,   482,   881,  1054,   890,  1002,   486,   879,   898,   900,
     135,   901,   135,   902,   874,   691,  -637,   722,   937,   927,
     953,  1192,  1193,  1194,   357,   947,   949,   776,  1196,   121,
    1299,   954,  1134,   955,   956,  1022,   958,  1304,   971,  1025,
    1026,  1212,   494,   495,   496,   972,   973,   580,   975,   497,
     498,   976,   239,   499,   500,  1213,  1006,  1007,  1008,  1033,
    1012,  1688,   864,  1018,  1015,  1036,   222,   129,  1028,   129,
    1040,  1041,  1049,  1050,   121,  1030,  1057,  1059,  1047,  1055,
    1076,   222,   222,   722,  1085,   609,   130,  1238,  1091,  1053,
    1088,  1096,  -812,   132,   128,   121,  1097,  1107,  1108,   133,
     614,   686,  1111,   719,  1112,  1114,  1131,  1125,  1127,  1130,
    1133,   614,  1213,  1142,  1146,  1147,  1151,   580,  1002,   895,
    1150,   135,  1198,  1207,  1210,   160,  1226,  1227,  1243,  1234,
    1239,  1235,  1247,  1241,  1257,  1731,  1245,   303,  1248,  1269,
    1369,  1250,   249,    60,    61,    62,    63,    64,   356,  1101,
    1101,   915,  1283,  1251,    70,   399,  1258,   589,  1259,   911,
     589,   922,  1263,   923,  1270,  1271,  1273,  1278,  1288,   719,
    1290,  1282,  1272,  1292,  1293,   121,   164,   121,   129,   121,
    1298,  1296,   579,  1300,  1303,  1345,  1297,  1308,   941,  1302,
     400,   164,   401,  1429,  1333,  1334,  1344,  1347,  1359,  1144,
     222,   222,  1349,  1360,  1351,   402,  1355,   403,  1361,  1366,
     357,   135,  1362,  1365,  1367,  1368,  1154,  1370,  1372,  1160,
     219,   219,   743,  1373,   233,  1377,  1374,  1399,  1378,  1412,
      36,  1427,   359,  1390,  1432,  1337,  1438,   164,  1439,  1338,
    1442,  1339,  1447,  1450,  1454,  1448,  1464,   769,  1466,  1346,
    1473,    48,   579,  1575,  1480,  1029,  1581,  1587,  1574,  1353,
     614,  1481,  1588,  1590,  1200,  1591,  1592,  1201,   129,  1600,
     550,  1039,  1602,  1612,  1633,  1638,  1613,  1641,  1642,  1649,
    1666,  1672,   164,  1668,  1577,  1326,  1680,  1309,   121,  1331,
    1681,  1686,  1687,  1695,  1326,  1694,  1696,  -288,  1331,  1699,
    1698,  1702,  1628,   164,  1703,  1710,  1712,   316,  1705,  1711,
      86,    87,  1396,    88,   183,    90,  1708,  1717,  1723,  1726,
     846,   847,  1389,  1729,   589,  1730,  1738,  1740,  1760,   855,
    1744,  1745,  1761,  1768,   222,  1755,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,  1472,
    1774,  1769,   915,  1775,  1777,   317,   915,  1778,  1371,  1440,
     797,  1021,   614,  1725,  1024,   661,   121,   664,  1092,  1739,
     659,  1046,  1087,  1275,  1380,  1606,  1379,   800,   121,   222,
    1737,  1598,  1621,   164,  1479,   164,   135,   164,  1626,   941,
    1129,   216,   216,   222,   222,   231,  1477,  1637,  1762,  1750,
    1197,  1596,  1195,  1400,  1419,  1014,   671,  1221,   406,  1255,
    1153,   781,  1261,   782,  1103,  1262,  1115,  1067,   219,   231,
    1579,   540,  1580,   961,  1392,   552,  1461,  1326,  1191,  1203,
    1145,  1331,     0,  1326,     0,  1326,     0,  1331,     0,  1331,
       0,     0,   589,   129,     0,  1640,     0,  1310,   135,     0,
       0,     0,     0,  1589,     0,     0,     0,   135,  1457,  1421,
       0,     0,     0,    33,    34,    35,     0,  1463,     0,     0,
       0,     0,     0,   222,     0,   212,     0,     0,  1469,     0,
       0,     0,     0,  1610,   219,     0,     0,     0,     0,   609,
     609,     0,  1667,   219,   219,     0,   164,     0,     0,     0,
     219,   915,     0,   915,     0,   129,     0,   219,  1604,  1461,
       0,     0,     0,     0,   129,     0,     0,     0,   578,     0,
       0,     0,     0,  1237,    74,    75,    76,    77,    78,  1326,
       0,  1636,     0,  1331,   135,   214,     0,     0,     0,     0,
     135,    82,    83,     0,     0,     0,   135,     0,  1607,     0,
       0,     0,     0,   121,     0,    92,     0,     0,     0,   268,
       0,  1715,     0,  1582,     0,  1417,     0,     0,  1268,    97,
       0,  1045,     0,     0,   164,     0,     0,     0,     0,     0,
       0,     0,   550,   941,     0,   233,   164,  1056,     0,   216,
       0,   129,     0,     0,  1684,     0,     0,   129,     0,     0,
    1070,     0,  1643,   129,     0,   905,   906,     0,     0,     0,
     915,     0,     0,     0,     0,   121,     0,     0,     0,     0,
     121,     0,   359,     0,   121,   907,   231,   231,   219,  1090,
       0,     0,   231,   908,   909,   910,    36,     0,     0,   303,
       0,     0,     0,   268,     0,   911,     0,     0,     0,     0,
       0,     0,     0,  1563,     0,   216,     0,    48,     0,     0,
    1570,     0,   550,     0,   216,   216,     0,   268,     0,   268,
       0,   216,     0,     0,     0,   268,     0,     0,   216,     0,
    1647,     0,  1648,     0,   135,     0,     0,     0,     0,   231,
    1141,  1654,   912,  1143,     0,     0,     0,     0,   915,     0,
       0,   121,   121,   121,     0,   913,     0,   121,     0,     0,
       0,   231,     0,   121,   231,     0,    86,    87,     0,    88,
     183,    90,     0,     0,   135,   135,     0,     0,     0,     0,
       0,   135,     0,     0,   914,     0,     0,  1691,     0,     0,
       0,   129,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   231,     0,     0,     0,
       0,   164,     0,     0,     0,     0,     0,     0,   135,     0,
       0,     0,     0,     0,     0,  1770,  1716,     0,     0,     0,
       0,   129,   129,  1776,     0,   589,     0,     0,   129,  1779,
       0,     0,  1780,     0,     0,     0,     0,     0,  1229,   216,
     855,     0,     0,   589,   219,     0,     0,     0,     0,     0,
       0,   589,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   303,     0,   164,     0,   129,     0,  1249,   164,     0,
    1252,     0,   164,   135,     0,     0,  1756,     0,     0,     0,
     135,     0,   268,  1763,     0,     0,   915,     0,   231,   231,
       0,   121,   761,     0,     0,     0,     0,     0,     0,     0,
    1659,   219,     0,     0,     0,     0,     0,  1563,  1563,     0,
       0,  1570,  1570,     0,     0,     0,     0,     0,     0,     0,
       0,  1289,     0,     0,   303,  1070,     0,     0,     0,     0,
     129,   121,   121,     0,     0,     0,     0,   129,   121,     0,
     761,     0,     0,   219,     0,   219,     0,     0,  1314,   164,
     164,   164,     0,     0,     0,   164,     0,     0,     0,     0,
       0,   164,     0,     0,     0,     0,     0,  1314,     0,     0,
     219,     0,     0,     0,     0,   121,     0,     0,     0,     0,
       0,     0,  1714,  1311,  1312,     0,   231,   231,     0,    13,
       0,     0,     0,     0,     0,   231,     0,     0,     0,  1727,
       0,     0,     0,     0,     0,     0,     0,     0,    13,     0,
       0,     0,     0,     0,     0,   216,  -939,  -939,  -939,  -939,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
     217,   217,     0,     0,     0,     0,     0,   219,     0,     0,
     121,   392,     0,     0,     0,     0,     0,   121,     0,     0,
       0,  1315,   219,   219,     0,     0,  1316,     0,    60,    61,
      62,   178,  1317,   356,  1318,     0,  1314,     0,     0,  1381,
    1315,  1382,   216,     0,     0,  1316,   578,    60,    61,    62,
     178,  1317,   356,  1318,     0,     0,  1314,  -939,  -939,  -939,
    -939,   994,   995,   996,   997,   998,   999,  1000,  1001,   164,
       0,  1319,  1320,     0,  1321,  1422,     0,    13,     0,     0,
       0,     0,  1002,     0,   216,     0,   216,     0,     0,     0,
    1319,  1320,     0,  1321,     0,   357,     0,    13,   233,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   164,
     164,   216,   761,     0,   357,     0,   164,  1335,     0,     0,
       0,     0,     0,     0,     0,   231,   231,   761,   761,   761,
     761,   761,     0,     0,     0,   761,  1445,     0,     0,  1315,
       0,   219,   219,     0,  1316,     0,    60,    61,    62,   178,
    1317,   356,  1318,   164,     0,     0,     0,     0,     0,  1315,
       0,   231,     0,     0,  1316,     0,    60,    61,    62,   178,
    1317,   356,  1318,     0,     0,     0,     0,   578,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1319,
    1320,   231,  1321,   216,   216,   218,   218,     0,   217,   232,
       0,     0,     0,     0,     0,     0,     0,   231,   231,  1319,
    1320,     0,  1321,   357,     0,     0,     0,   231,   164,     0,
       0,     0,     0,   231,     0,   164,     0,     0,     0,  1617,
       0,     0,     0,   357,     0,  1449,   231,     0,     0,     0,
       0,     0,     0,     0,   761,     0,     0,   231,     0,     0,
       0,     0,     0,     0,     0,  1451,     0,     0,     0,     0,
       0,     0,     0,     0,   217,   231,     0,     0,     0,   231,
       0,     0,     0,   217,   217,   219,     0,     0,     0,     0,
     217,     0,     0,     0,  1314,     0,     0,   217,     0,     0,
     437,   408,   409,   410,   411,   412,   413,   414,   415,   416,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,   216,     0,   578,     0,     0,     0,     0,
     219,     0,     0,     0,     0,    13,   231,     0,     0,   231,
    1639,   231,     0,     0,   219,   219,     0,     0,     0,   420,
     421,     0,     0,     0,     0,     0,   761,     0,   231,     0,
       0,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,   761,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1315,     0,     0,
       0,     0,  1316,   218,    60,    61,    62,   178,  1317,   356,
    1318,     0,     0,   761,     0,   422,   423,     0,   217,     0,
       0,     0,     0,  1700,   219,     0,     0,   269,     0,     0,
       0,   437,   408,   409,   410,   411,   412,   413,   414,   415,
     416,   417,   418,   419,   231,     0,   231,  1319,  1320,     0,
    1321,     0,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,   357,     0,   231,     0,     0,   231,     0,   218,   218,
     420,   421,     0,     0,     0,   218,   593,     0,     0,     0,
       0,     0,   218,  1599,   855,     0,   231,     0,     0,     0,
       0,   216,     0,   218,     0,     0,     0,     0,     0,   855,
       0,     0,   578,     0,     0,   216,   216,     0,   761,     0,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
       0,   231,     0,     0,   761,     0,   761,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   422,   423,     0,     0,
       0,     0,   761,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     232,   391,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   578,   392,     0,     0,     0,     0,     0,   231,
     231,     0,   231,     0,   217,   216,     0,   437,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
       0,     0,     0,   218,     0,     0,     0,   715,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   269,     0,     0,     0,     0,
     269,     0,     0,     0,     0,     0,   420,   421,     0,     0,
       0,   217,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   765,     0,     0,     0,
       0,     0,     0,     0,     0,   231,     0,   231,     0,     0,
       0,     0,   761,   231,     0,     0,   761,     0,   761,     0,
       0,   761,     0,   217,     0,   217,     0,     0,     0,     0,
     231,   231,     0,     0,   231,   366,   367,   368,     0,     0,
       0,   231,   422,   423,   765,     0,     0,     0,     0,   269,
     217,     0,   269,     0,   369,     0,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,     0,   391,     0,
       0,     0,     0,   231,     0,     0,     0,     0,     0,     0,
     392,     0,     0,     0,    36,     0,   211,     0,   761,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   231,   231,
       0,     0,     0,   808,   231,    48,   231,   217,     0,   218,
       0,     0,     0,     0,     0,     0,     0,     0,   366,   367,
     368,     0,   217,   217,     0,     0,     0,     0,   231,     0,
     231,     0,   654,     0,     0,     0,   231,   369,     0,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
       0,   391,     0,     0,    86,    87,   218,    88,   183,    90,
       0,   761,   761,   392,     0,     0,   269,   746,   761,   231,
     763,     0,     0,     0,     0,   231,     0,   231,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,     0,     0,   218,   655,
     218,   116,     0,     0,     0,     0,     0,     0,   828,     0,
       0,     0,     0,     0,   261,     0,     0,     0,   763,     0,
       0,     0,     0,     0,     0,   218,   765,     0,     0,     0,
       0,   217,   217,     0,     0,     0,     0,     0,     0,     0,
     262,   765,   765,   765,   765,   765,     0,     0,     0,   765,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   269,   269,     0,     0,     0,     0,
       0,     0,     0,   269,     0,  1017,   231,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,   218,   231,     0,     0,     0,     0,     0,     0,
       0,   860,     0,     0,     0,  1035,     0,   218,   218,     0,
       0,   231,     0,     0,     0,     0,     0,   761,   263,   264,
       0,     0,  1035,     0,     0,     0,     0,     0,   761,     0,
       0,   218,     0,   761,     0,     0,   182,   761,     0,    84,
     265,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   765,   231,
     266,  1077,     0,     0,     0,   217,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   232,   267,     0,     0,     0,  1583,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   761,     0,
       0,     0,     0,     0,     0,     0,   231,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     763,   231,     0,     0,   217,   217,   218,   218,     0,     0,
     231,     0,     0,   269,   269,   763,   763,   763,   763,   763,
       0,     0,     0,   763,     0,   231,     0,     0,     0,     0,
       0,     0,  1401,     0,     0,     0,     0,     0,     0,     0,
     765,     0,   218,     0,     0,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,     0,   220,   220,     0,     0,   235,     0,     0,
      36,     0,     0,     0,   217,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,   765,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,     0,   762,     0,     0,     0,     0,     0,
       0,     0,     0,  1402,   269,     0,     0,     0,     0,     0,
       0,     0,   763,     0,     0,     0,  1403,  1404,     0,     0,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   182,     0,     0,    84,  1405,     0,
      86,    87,   762,    88,  1406,    90,   407,   408,   409,   410,
     411,   412,   413,   414,   415,   416,   417,   418,   419,     0,
     218,     0,     0,     0,     0,   218,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   218,
     218,     0,   765,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   420,   421,   269,   765,   746,
     765,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   763,     0,   765,     0,     0,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,     0,     0,     0,     0,
       0,   220,     0,   366,   367,   368,  1313,     0,     0,   218,
       0,   422,   423,     0,     0,     0,     0,     0,     0,     0,
       0,   763,   369,     0,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,     0,   391,     0,     0,     0,
       0,     0,   269,     0,   269,     0,   767,     0,   392,     0,
       0,     0,     0,     0,     0,     0,     0,   220,     0,     0,
       0,     0,     0,     0,     0,     0,   220,   220,     0,     0,
       0,   269,     0,   220,   269,     0,     0,     0,     0,     0,
     220,     0,     0,     0,   762,     0,   765,   218,     0,     0,
     765,   235,   765,     0,   801,   765,    36,     0,     0,   762,
     762,   762,   762,   762,     0,  1398,     0,   762,  1411,     0,
       0,     0,     0,     0,     0,     0,   763,    48,     0,     0,
       0,     0,     0,     0,     0,   269,     0,     0,     0,   269,
       0,     0,   763,     0,   763,     0,     0,     0,     0,  1402,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     763,     0,  1403,  1404,     0,     0,     0,   218,   235,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,     0,   765,    84,    85,     0,    86,    87,     0,    88,
    1406,    90,  1470,  1471,     0,     0,   899,   269,   269,     0,
    1411,     0,     0,     0,     0,     0,   366,   367,   368,     0,
       0,   220,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   369,   762,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,   766,   765,   765,     0,     0,     0,
       0,     0,   765,  1615,     0,     0,     0,     0,     0,     0,
       0,  1411,     0,   269,     0,   269,     0,     0,     0,     0,
     763,     0,     0,     0,   763,     0,   763,    36,     0,   763,
       0,     0,     0,     0,     0,     0,   942,     0,   269,     0,
       0,     0,   766,     0,     0,     0,     0,     0,    48,   269,
       0,   964,   965,   966,   967,     0,     0,     0,   762,   974,
       0,     0,     0,   762,   762,   762,   762,   762,   762,   762,
     762,   762,   762,   762,   762,   762,   762,   762,   762,   762,
     762,   762,   762,   762,   762,   762,   762,   762,   762,   762,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,     0,     0,    84,     0,   763,    86,    87,     0,
      88,   183,    90,     0,     0,   762,     0,   220,     0,     0,
       0,     0,   269,     0,     0,     0,     0,     0,     0,   903,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   269,     0,   269,     0,
       0,   765,     0,  1658,   269,     0,     0,     0,     0,     0,
       0,     0,   765,     0,     0,     0,     0,   765,  1074,     0,
       0,   765,     0,     0,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   763,
     763,     0,     0,     0,     0,     0,   763,     0,     0,     0,
       0,     0,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   220,     0,   220,     0,
     762,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   765,     0,     0,     0,   762,     0,   762,     0,
    1724,     0,     0,   220,   766,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   762,  1398,     0,     0,     0,   766,
     766,   766,   766,   766,     0,     0,     0,   766,     0,     0,
       0,     0,     0,     0,     0,  1161,  1164,  1165,  1166,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
    1189,  1190,     0,     0,   269,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,     0,     0,   220,   220,  1202,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1660,
       0,     0,     0,     0,     0,   763,     0,     0,     0,   235,
       0,     0,     0,     0,     0,     0,   763,     0,     0,     0,
       0,   763,     0,     0,     0,   763,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   766,     0,     0,     0,
       0,   366,   367,   368,   762,     0,     0,   269,   762,     0,
     762,     0,     0,   762,     0,     0,     0,     0,     0,     0,
     369,   235,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,     0,   391,     0,   763,     0,     0,     0,
       0,     0,  1279,    36,     0,     0,   392,     0,     0,     0,
       0,     0,     0,     0,   220,   220,     0,     0,  1294,     0,
    1295,     0,     0,     0,    48,     0,     0,     0,   269,     0,
       0,     0,     0,     0,     0,     0,  1305,     0,     0,     0,
     762,     0,     0,   269,     0,     0,     0,     0,   766,     0,
     235,     0,     0,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,   766,   766,   766,
       0,  1568,     0,    86,    87,  1569,    88,   183,    90,   437,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     418,   419,     0,     0,     0,   766,    36,     0,   211,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   762,   762,  1416,     0,    48,     0,     0,
     762,     0,     0,     0,     0,     0,     0,     0,   420,   421,
       0,     0,     0,     0,  1027,     0,     0,     0,   220,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1384,     0,     0,     0,
    1386,     0,  1387,     0,     0,  1388,     0,     0,     0,  1002,
       0,     0,     0,     0,   684,     0,    86,    87,   235,    88,
     183,    90,     0,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   422,   423,     0,   220,   220,     0,
     766,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   766,     0,   766,     0,
       0,   685,     0,   116,   366,   367,   368,     0,     0,     0,
       0,     0,     0,     0,   766,     0,     0,     0,     0,     0,
       0,     0,  1465,   369,     0,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,     0,   391,     0,   762,
       0,     0,   366,   367,   368,     0,     0,   220,     0,   392,
     762,     0,   744,     0,     0,   762,     0,     0,     0,   762,
       0,   369,     0,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   389,   390,     0,   391,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1608,  1609,   392,     0,     0,
      36,     0,  1614,     0,     0,     0,     0,     0,     0,   745,
       0,     0,     0,     0,     0,    36,     0,   211,     0,     0,
     762,    48,     0,   366,   367,   368,     0,     0,     0,     0,
       0,     0,     0,     0,   766,   235,    48,     0,   766,     0,
     766,     0,   369,   766,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,     0,   391,     0,     0,     0,
       0,     0,     0,     0,   182,     0,     0,    84,   392,     0,
      86,    87,     0,    88,   183,    90,     0,  1084,     0,     0,
       0,     0,     0,   684,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,     0,   235,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     766,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,  1095,     0,     0,     0,     0,
     718,     0,   116,     0,     0,     0,     0,     0,     0,     0,
       0,  1669,     0,     0,     0,     0,     0,   366,   367,   368,
       0,     0,  1679,     0,     0,     0,     0,  1683,     0,     0,
       0,  1685,     0,     0,     0,     0,   369,     0,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,     0,
     391,     0,     0,   766,   766,     0,     0,     0,     0,     0,
     766,     0,   392,     0,     0,     0,  1118,     0,  1620,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1718,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    1434,    66,    67,    68,    69,    70,    71,     0,     0,   766,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
     766,     0,     0,    79,     0,   766,    80,     0,     0,   766,
       0,    81,    82,    83,    84,    85,     0,    86,    87,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,  1701,     0,    94,    95,     0,    96,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1043,   116,   117,     0,   118,   119,     0,
     766,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,    81,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,    95,     0,    96,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1215,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,    81,    82,    83,    84,    85,
       0,    86,    87,     0,    88,    89,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
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
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,   595,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1016,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1058,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1124,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1126,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
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
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,  1280,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1391,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1611,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,  1655,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
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
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1689,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1690,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1693,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
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
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1709,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1764,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1771,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   464,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   178,   179,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   731,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   178,
     179,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   944,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   178,   179,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,  1460,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   178,   179,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1603,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   178,   179,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   178,   179,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   666,
      12,     0,     0,     0,     0,     0,     0,   667,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,   274,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
     274,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   275,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   369,    10,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,   610,
     391,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   392,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,   611,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
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
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   180,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
     367,   368,   726,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   369,    10,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,  1071,   391,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   392,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   178,   179,   180,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
    1072,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   666,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   366,   367,   368,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   369,     0,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,     0,   391,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   392,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,   194,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,  1009,  1010,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,   978,   979,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   980,    10,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,   224,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,  1002,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   366,   367,   368,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   369,     0,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,     0,   391,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     392,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   180,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,  1656,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   256,     0,   368,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     369,     0,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,     0,   391,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   392,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   178,   179,   180,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   259,     0,   979,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   980,     0,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,  1002,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,   462,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,   623,   391,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   980,    10,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,   667,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,  1002,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   709,   391,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   392,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   180,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,   711,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,  1002,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   178,   179,   180,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,  1116,   391,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     392,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   366,   367,   368,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   369,     0,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,     0,   391,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   392,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
    1468,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   366,
     367,   368,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   369,     0,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,     0,   391,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   392,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   558,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,  1307,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,   366,   367,   368,     0,   116,   117,
       0,   118,   119,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   369,     0,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,     0,   391,     0,     0,
       0,     0,     0,  1484,  1485,  1486,  1487,  1488,     0,   392,
    1489,  1490,  1491,  1492,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1493,  1494,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,     0,
       0,     0,     0,  1495,     0,     0,     0,     0,     0,     0,
       0,     0,  1002,     0,     0,     0,     0,  1496,  1497,  1498,
    1499,  1500,  1501,  1502,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1503,  1504,  1505,
    1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,    48,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,  1534,
    1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,  1543,     0,
       0,     0,  1544,  1545,     0,  1546,  1547,  1548,  1549,  1550,
       0,     0,     0,     0,     0,     0,     0,  1435,     0,     0,
       0,  1551,  1552,  1553,     0,     0,     0,    86,    87,     0,
      88,   183,    90,  1554,     0,  1555,  1556,     0,  1557,     0,
       0,     0,     0,     0,     0,  1558,  1559,     0,  1560,     0,
    1561,  1562,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   366,   367,   368,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   369,  1284,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,     0,     0,     0,     0,     0,   369,
       0,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,     0,   391,   366,   367,   368,     0,     0,     0,
       0,     0,     0,     0,     0,   392,     0,     0,     0,     0,
       0,     0,     0,   369,     0,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,     0,   391,   366,   367,
     368,     0,     0,     0,     0,     0,     0,     0,     0,   392,
       0,     0,     0,     0,     0,     0,     0,   369,     0,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
       0,   391,   366,   367,   368,     0,     0,     0,     0,     0,
       0,     0,     0,   392,     0,     0,     0,  1285,     0,     0,
       0,   369,     0,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   389,   390,     0,   391,   366,   367,   368,     0,
       0,     0,     0,     0,     0,     0,     0,   392,     0,     0,
       0,   393,     0,     0,     0,   369,     0,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,     0,   478,     0,     0,     0,   369,
       0,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,     0,   391,   366,   367,   368,     0,     0,     0,
       0,     0,     0,     0,     0,   392,     0,     0,     0,   480,
       0,     0,     0,   369,     0,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,     0,   391,   977,   978,
     979,     0,     0,     0,     0,     0,     0,     0,     0,   392,
       0,     0,     0,   492,     0,     0,     0,   980,     0,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   996,   997,   998,   999,  1000,  1001,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1002,     0,     0,     0,   516,   261,   369,
       0,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,     0,   391,   262,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   392,     0,     0,     0,     0,
     700,     0,     0,     0,     0,     0,    36,     0,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,    48,   391,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     392,     0,     0,     0,   723,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,     0,   263,   264,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1002,     0,     0,     0,     0,   261,
     182,     0,     0,    84,   265,     0,    86,    87,  1158,    88,
     183,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   266,   262,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,  1013,     0,    36,   267,     0,
       0,     0,  1651,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,     0,    48,     0,
       0,     0,     0,     0,     0,     0,  -335,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   356,
       0,     0,   262,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   263,   264,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,   182,     0,     0,    84,   265,     0,    86,    87,     0,
      88,   183,    90,     0,     0,    48,     0,     0,     0,     0,
       0,   261,     0,   485,     0,   266,     0,     0,     0,     0,
       0,   357,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   262,     0,   267,
     263,   264,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   261,   182,    36,
       0,    84,   265,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,   266,   262,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,    36,   267,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
     261,     0,     0,   182,     0,     0,    84,   265,     0,    86,
      87,     0,    88,   183,    90,     0,   959,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   266,     0,     0,
       0,   263,   264,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,   182,
       0,   267,    84,   265,     0,    86,    87,     0,    88,   183,
      90,     0,  1291,     0,     0,     0,     0,     0,   261,    48,
       0,     0,     0,   266,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   262,     0,     0,   267,     0,     0,
       0,     0,     0,     0,   263,   264,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,   182,     0,     0,    84,   265,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,   266,  1395,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     267,     0,   263,   264,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   211,
     182,     0,     0,    84,   265,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,   266,     0,     0,     0,     0,  1167,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   752,   753,     0,   267,     0,
       0,   754,     0,   755,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   756,     0,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    86,    87,     0,
      88,   183,    90,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,   655,     0,   116,     0,     0,     0,     0,     0,
       0,     0,   757,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,   758,     0,    86,    87,     0,    88,
     183,    90,     0,     0,   938,    92,     0,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    28,     0,     0,   760,     0,
       0,     0,     0,    33,    34,    35,    36,     0,   211,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1002,   939,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,    36,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,  1415,   752,   753,   215,     0,
       0,     0,   754,   116,   755,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,     0,    86,
      87,     0,    88,   183,    90,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,  1416,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   757,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,   758,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,   759,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    28,     0,     0,   760,
       0,     0,     0,     0,    33,    34,    35,    36,     0,   211,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,     0,   391,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     392,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,    28,     0,   215,
       0,     0,   529,     0,   116,    33,    34,    35,    36,     0,
     211,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1002,     0,   549,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,    28,
     215,   894,     0,     0,     0,   116,     0,    33,    34,    35,
      36,     0,   211,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,  -939,  -939,  -939,  -939,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,     0,     0,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      28,     0,   215,     0,     0,     0,     0,   116,    33,    34,
      35,    36,     0,   211,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,  -939,  -939,  -939,  -939,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,     0,
       0,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,  1002,     0,     0,     0,  1038,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    28,     0,   215,     0,     0,     0,     0,   116,    33,
      34,    35,    36,     0,   211,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,    36,     0,   211,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   214,    48,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,   654,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   215,    33,    34,    35,    36,   116,
     211,    86,    87,     0,    88,   183,    90,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     229,     0,     0,     0,     0,     0,     0,    36,   116,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   214,    48,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     230,    33,    34,    35,    36,   116,   211,    86,    87,     0,
      88,   183,    90,   572,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   213,     0,     0,   616,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   214,    48,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   573,     0,     0,     0,
       0,   574,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   366,   367,   368,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   369,   874,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,     0,   391,   366,
     367,   368,     0,     0,     0,     0,     0,     0,     0,     0,
     392,     0,     0,     0,     0,     0,     0,     0,   369,     0,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,     0,   391,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   392,     0,   366,   367,   368,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   369,   440,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,     0,   391,
     366,   367,   368,     0,     0,     0,     0,     0,     0,     0,
       0,   392,     0,     0,     0,     0,     0,     0,     0,   369,
     449,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,     0,   391,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   392,     0,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   369,   880,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,     0,
     391,   977,   978,   979,     0,     0,     0,     0,     0,     0,
       0,     0,   392,     0,     0,     0,     0,     0,     0,     0,
     980,   924,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1002,     0,   977,   978,
     979,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   980,  1244,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   994,   995,   996,   997,   998,   999,  1000,  1001,
       0,     0,   977,   978,   979,     0,     0,     0,     0,     0,
       0,     0,     0,  1002,     0,     0,     0,     0,     0,     0,
       0,   980,  1149,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1098,  1099,  1100,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1002,     0,   977,
     978,   979,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   980,  1301,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1002,     0,     0,    36,     0,     0,
       0,     0,     0,  1383,     0,    48,     0,    86,    87,     0,
      88,   183,    90,   283,   284,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,    36,
       0,   822,   823,     0,     0,     0,     0,     0,     0,     0,
    1467,   285,    36,     0,    86,    87,     0,    88,   183,    90,
      48,   182,     0,     0,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    48,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    86,
      87,    36,    88,   183,    90,     0,     0,    48,     0,     0,
     353,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,    48,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   517,     0,     0,    86,    87,     0,    88,
     183,    90,    48,     0,     0,     0,     0,     0,   521,     0,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,     0,     0,     0,     0,     0,    48,     0,   285,     0,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1159,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   366,
     367,   368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   735,   369,     0,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,     0,   391,     0,   366,   367,   368,     0,     0,     0,
       0,     0,     0,     0,   392,     0,     0,     0,     0,     0,
       0,     0,     0,   369,   877,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   736,   391,   366,   367,
     368,     0,     0,     0,     0,     0,     0,     0,     0,   392,
       0,     0,     0,     0,     0,     0,     0,   369,     0,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
       0,   391,   977,   978,   979,     0,     0,     0,     0,     0,
       0,     0,     0,   392,     0,     0,     0,     0,     0,     0,
       0,   980,  1306,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,   977,   978,   979,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1002,     0,     0,
       0,     0,     0,   980,     0,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1002
};

static const yytype_int16 yycheck[] =
{
       5,     6,   160,     8,     9,    10,    11,    12,   613,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   135,  1061,    28,    29,    91,     4,   853,   185,    95,
      96,    54,    32,     4,     4,   702,    55,    42,   174,     4,
     428,   429,   407,   585,    44,    50,   584,    52,  1057,    49,
      55,   663,    57,   324,    30,   434,    30,   165,   231,   160,
     842,     4,   391,   113,   424,     4,   873,   424,   134,   734,
     458,   937,   186,   565,    79,   741,     9,    45,    30,     9,
       9,   227,   889,     9,    30,     9,    45,    45,  1047,     9,
      14,     9,     9,    79,    66,     9,    79,    66,     9,    66,
     460,     9,     9,   460,    45,    45,     9,     9,   113,     9,
      66,     9,     9,     9,     9,    50,     9,    66,     4,    79,
      10,    11,    12,    35,   113,     0,     9,     9,    66,   110,
      66,     9,     9,   241,   240,    79,     9,   101,   102,    29,
       9,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     9,    53,    33,   215,     9,    79,    30,    85,
      58,   113,    85,   129,   130,    65,    14,    66,   164,    35,
     230,   150,   129,   130,   150,   190,   199,   168,   324,   395,
    1597,   973,    30,    81,    66,   118,    84,   199,    66,     8,
      79,   358,   125,   309,   199,   171,   150,    66,    66,    47,
     215,    66,    66,    66,    66,    66,    66,   160,    66,   425,
     155,   204,    66,    79,   430,   230,   215,   199,     4,   197,
    1239,   147,   200,   202,   147,   202,   202,   242,   197,   203,
     245,   230,   200,   172,   204,  1652,   202,   252,   253,   135,
     202,   301,   172,   202,   172,   201,   203,    46,    47,   200,
     200,   199,   398,   199,   202,  1131,   202,    30,   201,    26,
      27,   201,   202,   215,   200,    51,   246,   201,    54,   436,
     250,   947,  1241,   949,   201,  1092,   352,   201,   424,  1248,
     201,  1250,   406,   201,   201,    71,   301,   443,   201,   201,
     186,   201,   307,   201,   201,   201,   201,   312,   201,    79,
     199,   447,   301,    89,    50,    91,   206,   200,   200,    95,
      96,   457,   200,   200,   460,   204,   349,   200,   870,  1111,
     202,   200,   337,    79,   202,   401,   402,   403,   404,   831,
    1349,   199,  1351,   348,   202,   199,    79,   202,   202,   202,
     202,   202,   202,   200,   202,   199,   200,   200,   134,   301,
     199,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,    79,   392,   407,   394,
     395,   330,   397,   336,   164,   283,   284,   285,    35,   129,
     130,  1360,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   391,    66,   391,   164,   155,
     425,   426,   198,   428,   429,   430,   431,    97,   316,    35,
     576,   164,   437,  1045,   170,   440,     4,    35,  1447,   818,
     150,    66,    79,   601,   449,  1112,   451,   466,    35,    97,
     336,    79,   199,   458,    97,   101,   102,    85,    26,    27,
      53,   466,  1127,   468,   156,   150,   150,   224,   171,    79,
     246,    97,    65,    79,   250,    85,   129,   130,   254,   129,
     130,    79,    79,   656,   154,   591,   592,   171,    85,   199,
     495,   199,    79,   498,   499,   500,   199,   273,   886,   859,
     601,   471,   859,   196,   129,   130,   154,   653,   665,   202,
     202,   154,   171,    97,   199,    97,  1298,   690,    42,   405,
     148,   149,  1060,   573,   529,   150,   171,   164,   154,   391,
     202,   154,   464,   290,  1076,   708,   398,  1079,   148,   149,
     199,    79,   299,   300,   301,   113,   199,    85,   199,   306,
     147,   148,   149,   619,   199,    79,   313,   203,   164,   171,
     336,    79,   424,    71,    72,   894,   164,    85,   573,   345,
     154,   842,   154,   349,   199,   199,   352,   164,   201,   202,
     208,   513,   199,    79,   199,   447,  1368,   199,   761,    85,
    1257,  1600,  1259,   155,    79,   457,   769,   207,   460,    79,
     706,   707,   748,   199,   106,   713,   611,   713,   714,   147,
     148,   149,   114,   115,   116,   117,   118,   119,   623,   156,
      71,    72,   398,   399,   400,   401,   402,   403,   404,   517,
     148,   149,   169,   521,   150,    97,    98,  1249,   526,   199,
     114,   115,   116,   117,   118,   119,   585,   215,   424,   117,
     655,   199,   148,   149,   586,   171,   224,   125,   601,   201,
     202,   666,   230,   148,   149,   202,   202,   147,   148,   149,
      79,   447,    29,   154,    29,  1040,    85,   434,   246,   181,
     685,    35,   250,   199,   460,   199,   202,   127,   128,    46,
    1228,    46,    49,     4,    49,   471,  1735,  1085,   201,  1366,
     201,  1243,   973,    49,    50,    51,   842,   181,  1096,   845,
     201,  1750,   488,   718,   100,   101,   102,   201,  1330,    65,
    1729,   857,   290,   859,   100,   101,   102,   201,   201,   734,
     730,   299,   300,   301,    45,  1744,   201,   202,   306,   148,
     149,   114,   115,   116,   310,   313,  1627,  1628,   314,  1623,
    1624,  1130,   528,   114,   115,   116,   117,   118,   119,   117,
     118,   119,   750,   751,   125,   126,   771,   201,   336,   739,
      66,    66,   338,   919,   340,   341,   342,   343,   554,   555,
     201,   724,    49,    50,    51,   202,    53,  1454,  1400,   925,
     150,   199,   199,    66,   726,   199,   107,   150,    65,   731,
     201,   112,   163,   114,   115,   116,   117,   118,   119,   120,
     154,    46,    47,    48,    49,    50,    51,   963,    53,  1361,
     181,    44,   827,    65,   970,   150,   171,   893,   199,   199,
      65,  1210,   206,     9,   150,   150,   841,   973,   724,     8,
    1111,   171,   785,   619,   150,   201,   157,   158,   199,   160,
     199,    14,   830,   610,    79,   125,   150,  1245,   201,   830,
     830,   201,   867,   125,    14,   830,   434,   200,   171,    14,
     181,    97,   877,   200,   200,   880,   200,   882,  1704,   199,
     199,   886,   205,     9,   106,   199,   147,   830,   200,    89,
     200,   830,   203,   200,  1067,   200,  1722,     9,    14,   785,
     201,   199,   790,   471,  1730,     9,   794,   185,   851,    79,
     667,    79,    79,   188,   853,   199,   201,     9,   894,   924,
     894,   201,     9,    79,   200,   127,   702,   201,   704,   200,
     930,   870,   200,   199,    66,  1602,   200,   114,   115,   116,
     117,   118,   119,   128,   830,    30,   170,   150,   724,   131,
       9,   200,   709,   150,   711,     9,    14,   197,     9,  1347,
    1106,   931,   738,   739,   172,   851,    46,    47,    48,    49,
      50,    51,   200,     9,    14,  1111,   127,   206,     9,   736,
     261,   262,   203,   845,    14,    65,   267,   206,   206,   200,
     933,   200,   935,   206,   199,   857,   150,   859,    97,   200,
      86,  1006,  1007,  1008,   181,   201,   201,  1012,  1013,   785,
    1156,   131,   944,   150,     9,   791,   200,  1163,   199,   795,
     796,  1040,   182,   183,   184,   150,   199,  1298,   150,   189,
     190,   202,   894,   193,   194,  1040,   185,   185,    14,   815,
       9,  1646,   610,   202,    79,   202,   803,   933,    14,   935,
      14,   206,    14,   200,   830,   201,   197,    30,   202,   201,
     199,   818,   819,   925,   199,   346,  1044,  1072,    14,   845,
      30,   199,   199,  1044,  1044,   851,    14,    48,   199,  1044,
    1085,   857,   199,   859,     9,   200,   131,   201,   201,   199,
      14,  1096,  1097,   131,     9,   200,     9,  1368,    65,   667,
     206,  1044,    79,     9,   199,  1044,   131,   201,   199,    14,
     200,    79,   200,   202,   131,  1720,   199,   893,   202,  1119,
    1266,   202,  1127,   114,   115,   116,   117,   118,   119,   905,
     906,   907,  1137,   201,   125,   126,   206,  1076,     9,    86,
    1079,   709,   147,   711,    30,    73,   201,   200,   172,   925,
     131,   201,  1122,    30,   200,   931,   724,   933,  1044,   935,
       9,   200,  1298,   200,     9,    79,   131,   200,   736,   203,
     161,   739,   163,  1331,   203,   202,    14,   199,   201,   955,
     937,   938,   200,   202,   200,   176,   200,   178,   199,   131,
     181,  1134,   200,   200,   200,     9,   972,    30,   201,   975,
      26,    27,   483,   200,    30,   201,   200,   107,   201,   159,
      77,   201,  1326,   202,   155,  1220,    14,   785,    79,  1224,
     112,  1226,   200,   202,   131,   200,   200,  1003,   131,  1234,
      14,    98,  1368,    79,   171,   803,    14,    14,   201,  1244,
    1245,   202,    79,   200,  1020,   199,   202,  1023,  1134,   200,
     818,   819,   131,   201,    14,    14,   201,   201,    14,   202,
       9,    55,   830,   203,  1421,  1208,    79,  1199,  1044,  1208,
     199,    79,     9,    79,  1217,   201,   110,    97,  1217,    97,
     150,   162,    33,   851,    14,   201,   168,   154,   199,   199,
     157,   158,  1315,   160,   161,   162,   200,   172,    79,   165,
     581,   582,  1307,   200,  1243,     9,    79,   201,    14,   590,
     200,   200,    79,    14,  1071,   202,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,  1395,
      14,    79,  1108,    79,    14,   202,  1112,    79,  1270,  1344,
     526,   790,  1347,  1712,   794,   401,  1122,   404,   892,  1726,
     399,   833,   887,  1128,  1286,  1459,  1283,   531,  1134,  1116,
    1722,  1450,  1482,   931,  1401,   933,  1309,   935,  1566,   937,
     938,    26,    27,  1130,  1131,    30,  1399,  1578,  1754,  1742,
    1015,  1446,  1012,  1317,  1321,   782,   407,  1048,  1331,  1094,
     971,   502,  1107,   502,   906,  1108,   921,   857,   224,    54,
    1423,   300,  1425,   750,  1310,   307,  1376,  1350,  1003,  1031,
     956,  1350,    -1,  1356,    -1,  1358,    -1,  1356,    -1,  1358,
      -1,    -1,  1361,  1309,    -1,  1582,    -1,  1203,  1371,    -1,
      -1,    -1,    -1,  1438,    -1,    -1,    -1,  1380,  1370,  1325,
      -1,    -1,    -1,    74,    75,    76,    -1,  1379,    -1,    -1,
      -1,    -1,    -1,  1210,    -1,    86,    -1,    -1,  1390,    -1,
      -1,    -1,    -1,  1468,   290,    -1,    -1,    -1,    -1,   750,
     751,    -1,  1618,   299,   300,    -1,  1044,    -1,    -1,    -1,
     306,  1257,    -1,  1259,    -1,  1371,    -1,   313,  1458,  1459,
      -1,    -1,    -1,    -1,  1380,    -1,    -1,    -1,   324,    -1,
      -1,    -1,    -1,  1071,   135,   136,   137,   138,   139,  1452,
      -1,  1577,    -1,  1452,  1457,   146,    -1,    -1,    -1,    -1,
    1463,   152,   153,    -1,    -1,    -1,  1469,    -1,  1460,    -1,
      -1,    -1,    -1,  1309,    -1,   166,    -1,    -1,    -1,  1315,
      -1,  1699,    -1,  1429,    -1,  1321,    -1,    -1,  1116,   180,
      -1,   832,    -1,    -1,  1122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1130,  1131,    -1,   391,  1134,   848,    -1,   224,
      -1,  1457,    -1,    -1,  1640,    -1,    -1,  1463,    -1,    -1,
     861,    -1,  1587,  1469,    -1,    46,    47,    -1,    -1,    -1,
    1366,    -1,    -1,    -1,    -1,  1371,    -1,    -1,    -1,    -1,
    1376,    -1,  1716,    -1,  1380,    66,   261,   262,   434,   890,
      -1,    -1,   267,    74,    75,    76,    77,    -1,    -1,  1395,
      -1,    -1,    -1,  1399,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1409,    -1,   290,    -1,    98,    -1,    -1,
    1416,    -1,  1210,    -1,   299,   300,    -1,  1423,    -1,  1425,
      -1,   306,    -1,    -1,    -1,  1431,    -1,    -1,   313,    -1,
    1592,    -1,  1594,    -1,  1607,    -1,    -1,    -1,    -1,   324,
     951,  1603,   133,   954,    -1,    -1,    -1,    -1,  1454,    -1,
      -1,  1457,  1458,  1459,    -1,   146,    -1,  1463,    -1,    -1,
      -1,   346,    -1,  1469,   349,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,  1647,  1648,    -1,    -1,    -1,    -1,
      -1,  1654,    -1,    -1,   175,    -1,    -1,  1649,    -1,    -1,
      -1,  1607,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,   391,    -1,    -1,    -1,
      -1,  1309,    -1,    -1,    -1,    -1,    -1,    -1,  1691,    -1,
      -1,    -1,    -1,    -1,    -1,  1760,  1699,    -1,    -1,    -1,
      -1,  1647,  1648,  1768,    -1,  1704,    -1,    -1,  1654,  1774,
      -1,    -1,  1777,    -1,    -1,    -1,    -1,    -1,  1059,   434,
    1061,    -1,    -1,  1722,   610,    -1,    -1,    -1,    -1,    -1,
      -1,  1730,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1577,    -1,  1371,    -1,  1691,    -1,  1088,  1376,    -1,
    1091,    -1,  1380,  1756,    -1,    -1,  1748,    -1,    -1,    -1,
    1763,    -1,  1598,  1755,    -1,    -1,  1602,    -1,   483,   484,
      -1,  1607,   487,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1616,   667,    -1,    -1,    -1,    -1,    -1,  1623,  1624,    -1,
      -1,  1627,  1628,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1142,    -1,    -1,  1640,  1146,    -1,    -1,    -1,    -1,
    1756,  1647,  1648,    -1,    -1,    -1,    -1,  1763,  1654,    -1,
     535,    -1,    -1,   709,    -1,   711,    -1,    -1,     4,  1457,
    1458,  1459,    -1,    -1,    -1,  1463,    -1,    -1,    -1,    -1,
      -1,  1469,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
     736,    -1,    -1,    -1,    -1,  1691,    -1,    -1,    -1,    -1,
      -1,    -1,  1698,  1204,  1205,    -1,   581,   582,    -1,    45,
      -1,    -1,    -1,    -1,    -1,   590,    -1,    -1,    -1,  1715,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,   610,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      26,    27,    -1,    -1,    -1,    -1,    -1,   803,    -1,    -1,
    1756,    65,    -1,    -1,    -1,    -1,    -1,  1763,    -1,    -1,
      -1,   107,   818,   819,    -1,    -1,   112,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,     4,    -1,    -1,  1290,
     107,  1292,   667,    -1,    -1,   112,   842,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,     4,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1607,
      -1,   157,   158,    -1,   160,  1326,    -1,    45,    -1,    -1,
      -1,    -1,    65,    -1,   709,    -1,   711,    -1,    -1,    -1,
     157,   158,    -1,   160,    -1,   181,    -1,    45,   894,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1647,
    1648,   736,   737,    -1,   181,    -1,  1654,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   750,   751,   752,   753,   754,
     755,   756,    -1,    -1,    -1,   760,   203,    -1,    -1,   107,
      -1,   937,   938,    -1,   112,    -1,   114,   115,   116,   117,
     118,   119,   120,  1691,    -1,    -1,    -1,    -1,    -1,   107,
      -1,   786,    -1,    -1,   112,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,    -1,    -1,    -1,   973,   803,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,   816,   160,   818,   819,    26,    27,    -1,   224,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   832,   833,   157,
     158,    -1,   160,   181,    -1,    -1,    -1,   842,  1756,    -1,
      -1,    -1,    -1,   848,    -1,  1763,    -1,    -1,    -1,  1480,
      -1,    -1,    -1,   181,    -1,   203,   861,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   869,    -1,    -1,   872,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   290,   890,    -1,    -1,    -1,   894,
      -1,    -1,    -1,   299,   300,  1071,    -1,    -1,    -1,    -1,
     306,    -1,    -1,    -1,     4,    -1,    -1,   313,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   937,   938,    -1,  1111,    -1,    -1,    -1,    -1,
    1116,    -1,    -1,    -1,    -1,    45,   951,    -1,    -1,   954,
    1581,   956,    -1,    -1,  1130,  1131,    -1,    -1,    -1,    63,
      64,    -1,    -1,    -1,    -1,    -1,   971,    -1,   973,    -1,
      -1,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,
      -1,    -1,   112,   224,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,  1028,    -1,   129,   130,    -1,   434,    -1,
      -1,    -1,    -1,  1664,  1210,    -1,    -1,    54,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,  1059,    -1,  1061,   157,   158,    -1,
     160,    -1,    -1,    -1,    -1,    -1,  1071,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,
      -1,   181,    -1,  1088,    -1,    -1,  1091,    -1,   299,   300,
      63,    64,    -1,    -1,    -1,   306,   200,    -1,    -1,    -1,
      -1,    -1,   313,   203,  1735,    -1,  1111,    -1,    -1,    -1,
      -1,  1116,    -1,   324,    -1,    -1,    -1,    -1,    -1,  1750,
      -1,    -1,  1298,    -1,    -1,  1130,  1131,    -1,  1133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1142,    -1,    -1,
      -1,  1146,    -1,    -1,  1149,    -1,  1151,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,  1167,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     391,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1368,    65,    -1,    -1,    -1,    -1,    -1,  1204,
    1205,    -1,  1207,    -1,   610,  1210,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,   434,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   261,   262,    -1,    -1,    -1,    -1,
     267,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,    -1,
      -1,   667,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   487,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1290,    -1,  1292,    -1,    -1,
      -1,    -1,  1297,  1298,    -1,    -1,  1301,    -1,  1303,    -1,
      -1,  1306,    -1,   709,    -1,   711,    -1,    -1,    -1,    -1,
    1315,  1316,    -1,    -1,  1319,    10,    11,    12,    -1,    -1,
      -1,  1326,   129,   130,   535,    -1,    -1,    -1,    -1,   346,
     736,    -1,   349,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,  1368,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    77,    -1,    79,    -1,  1383,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1393,  1394,
      -1,    -1,    -1,   200,  1399,    98,  1401,   803,    -1,   610,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,   818,   819,    -1,    -1,    -1,    -1,  1423,    -1,
    1425,    -1,   125,    -1,    -1,    -1,  1431,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,   157,   158,   667,   160,   161,   162,
      -1,  1466,  1467,    65,    -1,    -1,   483,   484,  1473,  1474,
     487,    -1,    -1,    -1,    -1,  1480,    -1,  1482,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,    -1,    -1,   709,   202,
     711,   204,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,   535,    -1,
      -1,    -1,    -1,    -1,    -1,   736,   737,    -1,    -1,    -1,
      -1,   937,   938,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,   752,   753,   754,   755,   756,    -1,    -1,    -1,   760,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,   581,   582,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   590,    -1,   786,  1581,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   803,  1598,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,   816,    -1,   818,   819,    -1,
      -1,  1616,    -1,    -1,    -1,    -1,    -1,  1622,   133,   134,
      -1,    -1,   833,    -1,    -1,    -1,    -1,    -1,  1633,    -1,
      -1,   842,    -1,  1638,    -1,    -1,   151,  1642,    -1,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   869,  1664,
     175,   872,    -1,    -1,    -1,  1071,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   894,   199,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1703,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1711,    -1,    -1,    -1,
    1116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     737,  1726,    -1,    -1,  1130,  1131,   937,   938,    -1,    -1,
    1735,    -1,    -1,   750,   751,   752,   753,   754,   755,   756,
      -1,    -1,    -1,   760,    -1,  1750,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     971,    -1,   973,    -1,    -1,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,    -1,    26,    27,    -1,    -1,    30,    -1,    -1,
      77,    -1,    -1,    -1,  1210,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   832,    -1,  1028,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   848,    -1,    -1,   487,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   861,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   869,    -1,    -1,    -1,   133,   134,    -1,    -1,
    1071,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   890,   151,    -1,    -1,   154,   155,    -1,
     157,   158,   535,   160,   161,   162,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
    1111,    -1,    -1,    -1,    -1,  1116,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,  1130,
    1131,    -1,  1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   951,    63,    64,   954,  1149,   956,
    1151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   971,    -1,  1167,    -1,    -1,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,    -1,    -1,    -1,    -1,
      -1,   224,    -1,    10,    11,    12,  1207,    -1,    -1,  1210,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1028,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,  1059,    -1,  1061,    -1,   487,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   299,   300,    -1,    -1,
      -1,  1088,    -1,   306,  1091,    -1,    -1,    -1,    -1,    -1,
     313,    -1,    -1,    -1,   737,    -1,  1297,  1298,    -1,    -1,
    1301,   324,  1303,    -1,   535,  1306,    77,    -1,    -1,   752,
     753,   754,   755,   756,    -1,  1316,    -1,   760,  1319,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1133,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1142,    -1,    -1,    -1,  1146,
      -1,    -1,  1149,    -1,  1151,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1167,    -1,   133,   134,    -1,    -1,    -1,  1368,   391,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,  1383,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,  1393,  1394,    -1,    -1,   203,  1204,  1205,    -1,
    1401,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   434,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    29,   869,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,   487,  1466,  1467,    -1,    -1,    -1,
      -1,    -1,  1473,  1474,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1482,    -1,  1290,    -1,  1292,    -1,    -1,    -1,    -1,
    1297,    -1,    -1,    -1,  1301,    -1,  1303,    77,    -1,  1306,
      -1,    -1,    -1,    -1,    -1,    -1,   737,    -1,  1315,    -1,
      -1,    -1,   535,    -1,    -1,    -1,    -1,    -1,    98,  1326,
      -1,   752,   753,   754,   755,    -1,    -1,    -1,   971,   760,
      -1,    -1,    -1,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,    -1,  1383,   157,   158,    -1,
     160,   161,   162,    -1,    -1,  1028,    -1,   610,    -1,    -1,
      -1,    -1,  1399,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,  1423,    -1,  1425,    -1,
      -1,  1622,    -1,   203,  1431,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1633,    -1,    -1,    -1,    -1,  1638,   869,    -1,
      -1,  1642,    -1,    -1,   667,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1466,
    1467,    -1,    -1,    -1,    -1,    -1,  1473,    -1,    -1,    -1,
      -1,    -1,    -1,  1480,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   709,    -1,   711,    -1,
    1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1703,    -1,    -1,    -1,  1149,    -1,  1151,    -1,
    1711,    -1,    -1,   736,   737,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1167,  1726,    -1,    -1,    -1,   752,
     753,   754,   755,   756,    -1,    -1,    -1,   760,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,    -1,    -1,  1581,    -1,    -1,    -1,    -1,    -1,
     803,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1598,    -1,    -1,    -1,   818,   819,  1028,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1616,
      -1,    -1,    -1,    -1,    -1,  1622,    -1,    -1,    -1,   842,
      -1,    -1,    -1,    -1,    -1,    -1,  1633,    -1,    -1,    -1,
      -1,  1638,    -1,    -1,    -1,  1642,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   869,    -1,    -1,    -1,
      -1,    10,    11,    12,  1297,    -1,    -1,  1664,  1301,    -1,
    1303,    -1,    -1,  1306,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   894,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,  1703,    -1,    -1,    -1,
      -1,    -1,  1133,    77,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   937,   938,    -1,    -1,  1149,    -1,
    1151,    -1,    -1,    -1,    98,    -1,    -1,    -1,  1735,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1167,    -1,    -1,    -1,
    1383,    -1,    -1,  1750,    -1,    -1,    -1,    -1,   971,    -1,
     973,    -1,    -1,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
      -1,   155,    -1,   157,   158,   159,   160,   161,   162,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,  1028,    77,    -1,    79,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,  1466,  1467,   199,    -1,    98,    -1,    -1,
    1473,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,  1071,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1297,    -1,    -1,    -1,
    1301,    -1,  1303,    -1,    -1,  1306,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,   155,    -1,   157,   158,  1111,   160,
     161,   162,    -1,  1116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,  1130,  1131,    -1,
    1133,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,  1149,    -1,  1151,    -1,
      -1,   202,    -1,   204,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1383,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,  1622,
      -1,    -1,    10,    11,    12,    -1,    -1,  1210,    -1,    65,
    1633,    -1,    29,    -1,    -1,  1638,    -1,    -1,    -1,  1642,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1466,  1467,    65,    -1,    -1,
      77,    -1,  1473,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
    1703,    98,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1297,  1298,    98,    -1,  1301,    -1,
    1303,    -1,    29,  1306,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,    65,    -1,
     157,   158,    -1,   160,   161,   162,    -1,   203,    -1,    -1,
      -1,    -1,    -1,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,    -1,  1368,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
    1383,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,   203,    -1,    -1,    -1,    -1,
     202,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1622,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,  1633,    -1,    -1,    -1,    -1,  1638,    -1,    -1,
      -1,  1642,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,  1466,  1467,    -1,    -1,    -1,    -1,    -1,
    1473,    -1,    65,    -1,    -1,    -1,   203,    -1,  1481,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1703,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,   107,   108,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     203,   121,   122,   123,   124,   125,   126,    -1,    -1,  1622,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
    1633,    -1,    -1,   143,    -1,  1638,   146,    -1,    -1,  1642,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,  1666,    -1,   175,   176,    -1,   178,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,   203,   204,   205,    -1,   207,   208,    -1,
    1703,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,   107,   108,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,   176,    -1,   178,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,   107,   108,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
     176,    -1,   178,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,   203,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,   203,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    96,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,   203,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,   203,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,   203,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    92,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,   203,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      11,    12,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
     164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    10,    11,    12,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,   191,   192,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    11,    12,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    10,    11,    12,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      65,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,   188,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,    -1,    12,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    12,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,   200,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    30,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    10,    11,    12,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
     187,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    10,
      11,    12,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,   186,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    10,    11,    12,    -1,   204,   205,
      -1,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    65,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
      -1,   151,   152,   153,    -1,    -1,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,   165,   166,    -1,   168,    -1,
      -1,    -1,    -1,    -1,    -1,   175,   176,    -1,   178,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,   201,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,   201,    29,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    77,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    98,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,   200,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    29,
     151,    -1,    -1,   154,   155,    -1,   157,   158,   200,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    55,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   195,    -1,    77,   199,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    29,    -1,   106,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    55,    -1,   199,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   151,    77,
      -1,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   175,    55,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    77,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      29,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,   164,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,   175,    -1,    -1,
      -1,   133,   134,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    77,   151,
      -1,   199,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,   164,    -1,    -1,    -1,    -1,    -1,    29,    98,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    55,    -1,    -1,   199,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    -1,
     199,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,    30,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    46,    47,    -1,   199,    -1,
      -1,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    35,   166,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    66,    -1,    -1,   199,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,   132,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    77,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   123,    46,    47,   199,    -1,
      -1,    -1,    52,   204,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
      -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    66,    -1,    -1,   199,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    66,    -1,   199,
      -1,    -1,   202,    -1,   204,    74,    75,    76,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    66,
     199,    68,    -1,    -1,    -1,   204,    -1,    74,    75,    76,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      66,    -1,   199,    -1,    -1,    -1,    -1,   204,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    66,    -1,   199,    -1,    -1,    -1,    -1,   204,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    98,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    -1,   199,    74,    75,    76,    77,   204,
      79,   157,   158,    -1,   160,   161,   162,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    77,   204,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    98,    -1,
      -1,    -1,   151,   152,   153,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    -1,
     199,    74,    75,    76,    77,   204,    79,   157,   158,    -1,
     160,   161,   162,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   119,    -1,    -1,   199,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    98,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    29,   199,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   131,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     131,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   131,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   131,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   131,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,   131,    -1,    98,    -1,   157,   158,    -1,
     160,   161,   162,   106,   107,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    77,
      -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   154,    77,    -1,   157,   158,    -1,   160,   161,   162,
      98,   151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    98,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,    77,   160,   161,   162,    -1,    -1,    98,    -1,    -1,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    98,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,   157,   158,    -1,   160,
     161,   162,    98,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    77,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,    -1,    -1,    -1,    -1,    -1,    98,    -1,   154,    -1,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    97,    53,    10,    11,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
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
     213,   216,   223,   224,   225,   226,   227,   228,   231,   247,
     248,   252,   255,   260,   266,   326,   327,   335,   339,   340,
     341,   342,   343,   344,   345,   346,   348,   351,   363,   364,
     365,   367,   368,   370,   389,   399,   400,   401,   406,   409,
     428,   436,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   451,   464,   466,   468,   117,   118,
     119,   132,   151,   161,   216,   247,   326,   345,   440,   345,
     199,   345,   345,   345,   103,   345,   345,   426,   427,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,    79,    86,   119,   146,   199,   224,   364,   400,   401,
     406,   440,   443,   440,    35,   345,   455,   456,   345,   119,
     199,   224,   400,   401,   402,   406,   437,   438,   439,   447,
     452,   453,   199,   336,   403,   199,   336,   352,   337,   345,
     233,   336,   199,   199,   199,   336,   201,   345,   216,   201,
     345,    29,    55,   133,   134,   155,   175,   199,   216,   227,
     469,   482,   483,   485,   182,   201,   342,   345,   369,   371,
     202,   240,   345,   106,   107,   154,   217,   220,   223,    79,
     204,   292,   293,   118,   125,   117,   125,    79,   294,   199,
     199,   199,   199,   216,   264,   470,   199,   199,   337,    79,
      85,   147,   148,   149,   461,   462,   154,   202,   223,   223,
     216,   265,   470,   155,   199,   470,   470,    79,   196,   202,
     354,   335,   345,   346,   440,   444,   229,   202,    85,   404,
     461,    85,   461,   461,    30,   154,   171,   471,   199,     9,
     201,    35,   246,   155,   263,   470,   119,   181,   247,   327,
     201,   201,   201,   201,   201,   201,    10,    11,    12,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    53,    65,   201,    66,    66,   201,   202,   150,   126,
     161,   163,   176,   178,   266,   325,   326,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      63,    64,   129,   130,   430,    66,   202,   435,   199,   199,
      66,   202,   204,   448,   199,   246,   247,    14,   345,   201,
     131,    44,   216,   425,    85,   335,   346,   150,   440,   131,
     206,     9,   411,   335,   346,   440,   471,   150,   199,   405,
     430,   435,   200,   345,    30,   231,     8,   357,     9,   201,
     231,   232,   337,   338,   345,   216,   278,   235,   201,   201,
     201,   485,   485,   171,   199,   106,   485,    14,   150,   216,
      79,   201,   201,   201,   182,   183,   184,   189,   190,   193,
     194,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     384,   385,   386,   241,   110,   168,   201,   154,   218,   221,
     223,   154,   219,   222,   223,   223,     9,   201,    97,   202,
     440,     9,   201,   125,   125,    14,     9,   201,   440,   465,
     465,   335,   346,   440,   443,   444,   200,   171,   258,   132,
     440,   454,   455,   201,    66,   430,   147,   462,    78,   345,
     440,    85,   147,   462,   223,   215,   201,   202,   253,   261,
     390,   392,    86,   199,   204,   358,   359,   361,   401,   448,
     466,    14,    97,   467,   353,   355,   356,   288,   289,   428,
     429,   200,   200,   200,   200,   203,   230,   231,   248,   255,
     260,   428,   345,   205,   207,   208,   216,   472,   473,   485,
      35,   164,   290,   291,   345,   469,   199,   470,   256,   246,
     345,   345,   345,    30,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   402,   345,   345,   450,
     450,   345,   457,   458,   125,   202,   216,   447,   448,   264,
     216,   265,   470,   470,   263,   247,    27,    35,   339,   342,
     345,   369,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   155,   202,   216,   431,   432,   433,
     434,   447,   450,   345,   290,   290,   450,   345,   454,   246,
     200,   345,   199,   424,     9,   411,   200,   200,   216,    35,
     345,    35,   345,   200,   200,   200,   447,   290,   202,   216,
     431,   432,   447,   200,   229,   282,   202,   342,   345,   345,
      89,    30,   231,   276,   201,    28,    97,    14,     9,   200,
      30,   202,   279,   485,    29,    86,   227,   479,   480,   481,
     199,     9,    46,    47,    52,    54,    66,   133,   155,   175,
     199,   224,   225,   227,   366,   400,   406,   407,   408,   216,
     484,   185,    79,   345,    79,    79,   345,   381,   382,   345,
     345,   374,   384,   188,   387,   229,   199,   239,   223,   201,
       9,    97,   223,   201,     9,    97,    97,   220,   216,   345,
     293,   407,    79,     9,   200,   200,   200,   200,   200,   200,
     200,   201,    46,    47,   477,   478,   127,   269,   199,     9,
     200,   200,    79,    80,   216,   463,   216,    66,   203,   203,
     212,   214,    30,   128,   268,   170,    50,   155,   170,   394,
     346,   131,     9,   411,   200,   150,   485,   485,    14,   357,
     288,   229,   197,     9,   412,   485,   486,   430,   435,   430,
     203,     9,   411,   172,   440,   345,   200,     9,   412,    14,
     349,   249,   127,   267,   199,   470,   345,    30,   206,   206,
     131,   203,     9,   411,   345,   471,   199,   259,   254,   262,
      14,   467,   257,   246,    68,   440,   345,   471,   206,   203,
     200,   200,   206,   203,   200,    46,    47,    66,    74,    75,
      76,    86,   133,   146,   175,   216,   414,   416,   417,   420,
     423,   216,   440,   440,   131,   430,   435,   200,   345,   283,
      71,    72,   284,   229,   336,   229,   338,    97,    35,   132,
     273,   440,   407,   216,    30,   231,   277,   201,   280,   201,
     280,     9,   172,    86,   131,   150,     9,   411,   200,   164,
     472,   473,   474,   472,   407,   407,   407,   407,   407,   410,
     413,   199,   150,   199,   407,   150,   202,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   150,   471,   345,   185,   185,    14,   191,
     192,   383,     9,   195,   387,    79,   203,   400,   202,   243,
      97,   221,   216,    97,   222,   216,   216,   203,    14,   440,
     201,     9,   172,   216,   270,   400,   202,   454,   132,   440,
      14,   206,   345,   203,   212,   485,   270,   202,   393,    14,
     200,   345,   358,   216,   447,   201,   485,   197,   203,    30,
     475,   429,    35,    79,   164,   431,   432,   434,   431,   432,
     485,    35,   164,   345,   407,   288,   199,   400,   268,   350,
     250,   345,   345,   345,   203,   199,   290,   269,    30,   268,
     485,    14,   267,   470,   402,   203,   199,    14,    74,    75,
      76,   216,   415,   415,   417,   418,   419,    48,   199,    85,
     147,   199,     9,   411,   200,   424,    35,   345,   203,    71,
      72,   285,   336,   231,   203,   201,    90,   201,   273,   440,
     199,   131,   272,    14,   229,   280,   100,   101,   102,   280,
     203,   485,   131,   485,   216,   479,     9,   200,   411,   131,
     206,     9,   411,   410,   216,   358,   360,   362,   200,   125,
     216,   407,   459,   460,   407,   407,   407,    30,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   484,   345,   345,   345,   382,   345,   372,    79,   244,
     216,   216,   407,   478,    97,    98,   476,     9,   298,   200,
     199,   339,   342,   345,   206,   203,   467,   298,   156,   169,
     202,   389,   396,   156,   202,   395,   131,   201,   475,   485,
     357,   486,    79,   164,    14,    79,   471,   440,   345,   200,
     288,   202,   288,   199,   131,   199,   290,   200,   202,   485,
     202,   201,   485,   268,   251,   405,   290,   131,   206,     9,
     411,   416,   418,   147,   358,   421,   422,   417,   440,   336,
      30,    73,   231,   201,   338,   272,   454,   273,   200,   407,
      96,   100,   201,   345,    30,   201,   281,   203,   172,   485,
     131,   164,    30,   200,   407,   407,   200,   131,     9,   411,
     200,   131,   203,     9,   411,   407,    30,   186,   200,   229,
     216,   485,   485,   400,     4,   107,   112,   118,   120,   157,
     158,   160,   203,   299,   324,   325,   326,   331,   332,   333,
     334,   428,   454,   203,   202,   203,    50,   345,   345,   345,
     357,    35,    79,   164,    14,    79,   345,   199,   475,   200,
     298,   200,   288,   345,   290,   200,   298,   467,   298,   201,
     202,   199,   200,   417,   417,   200,   131,   200,     9,   411,
      30,   229,   201,   200,   200,   200,   236,   201,   201,   281,
     229,   485,   485,   131,   407,   358,   407,   407,   407,   345,
     202,   203,   476,   127,   128,   176,   469,   271,   400,   107,
     334,    29,   120,   133,   134,   155,   161,   308,   309,   310,
     311,   400,   159,   316,   317,   123,   199,   216,   318,   319,
     300,   247,   485,     9,   201,     9,   201,   201,   467,   325,
     200,   295,   155,   391,   203,   203,    79,   164,    14,    79,
     345,   290,   112,   347,   475,   203,   475,   200,   200,   203,
     202,   203,   298,   288,   131,   417,   358,   229,   234,   237,
      30,   231,   275,   229,   200,   407,   131,   131,   187,   229,
     400,   400,   470,    14,     9,   201,   202,   469,   467,   311,
     171,   202,     9,   201,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    53,    67,    68,    69,    70,
      71,    72,    73,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   132,   133,   135,   136,   137,   138,
     139,   151,   152,   153,   163,   165,   166,   168,   175,   176,
     178,   180,   181,   216,   397,   398,     9,   201,   155,   159,
     216,   319,   320,   321,   201,    79,   330,   246,   301,   469,
     469,    14,   247,   203,   296,   297,   469,    14,    79,   345,
     200,   199,   202,   201,   202,   322,   347,   475,   295,   203,
     200,   417,   131,    30,   231,   274,   275,   229,   407,   407,
     345,   203,   201,   201,   407,   400,   304,   485,   312,   313,
     406,   309,    14,    30,    47,   314,   317,     9,    33,   200,
      29,    46,    49,    14,     9,   201,   470,   330,    14,   485,
     246,   201,    14,   345,    35,    79,   388,   229,   229,   202,
     322,   203,   475,   417,   229,    94,   188,   242,   203,   216,
     227,   305,   306,   307,     9,   172,     9,   411,   203,   407,
     398,   398,    55,   315,   320,   320,    29,    46,    49,   407,
      79,   199,   201,   407,   470,   407,    79,     9,   412,   203,
     203,   229,   322,    92,   201,    79,   110,   238,   150,    97,
     485,   406,   162,    14,   302,   199,    35,    79,   200,   203,
     201,   199,   168,   245,   216,   325,   326,   172,   407,   286,
     287,   429,   303,    79,   400,   243,   165,   216,   201,   200,
       9,   412,   114,   115,   116,   328,   329,   286,    79,   271,
     201,   475,   429,   486,   200,   200,   201,   201,   202,   323,
     328,    35,    79,   164,   475,   202,   229,   486,    79,   164,
      14,    79,   323,   229,   203,    35,    79,   164,    14,    79,
     345,   203,    79,   164,    14,    79,   345,    14,    79,   345,
     345
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

  case 42:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { (yyval).reset();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { (yyval).reset();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
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

  case 147:

/* Line 1455 of yacc.c  */
#line 1108 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
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

  case 149:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval).reset();;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval).reset();;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
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

  case 187:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval).reset();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval).reset();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { (yyval).reset();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval).reset();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { (yyval).reset();;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval).reset();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval).reset();;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval).reset();;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval).reset();;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval).reset();;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
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

  case 464:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
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

  case 468:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
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

  case 470:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
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

  case 533:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
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

  case 534:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval).reset();;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval).reset();;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval).reset();;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval).reset();;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval).reset();;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval).reset();;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
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

  case 783:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
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

  case 784:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
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

  case 785:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
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

  case 786:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
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

  case 793:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
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

  case 802:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyval).reset();;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval)++;;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
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

  case 837:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
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

  case 843:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { (yyval).reset();;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
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

  case 879:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    {;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    {;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    {;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14027 "hphp.tab.cpp"
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
#line 3218 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

