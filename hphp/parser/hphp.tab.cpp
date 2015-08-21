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
#define YYLAST   17643

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  276
/* YYNRULES -- Number of rules.  */
#define YYNRULES  947
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1770

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
    2142,  2144,  2146,  2148,  2150,  2152,  2154,  2156,  2158,  2161,
    2163,  2164,  2165,  2167,  2169,  2173,  2174,  2176,  2178,  2180,
    2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,  2200,
    2204,  2207,  2209,  2211,  2216,  2220,  2225,  2227,  2229,  2233,
    2237,  2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,
    2277,  2281,  2285,  2289,  2293,  2297,  2301,  2304,  2307,  2310,
    2313,  2317,  2321,  2325,  2329,  2333,  2337,  2341,  2345,  2351,
    2356,  2360,  2364,  2368,  2370,  2372,  2374,  2376,  2380,  2384,
    2388,  2391,  2392,  2394,  2395,  2397,  2398,  2404,  2408,  2412,
    2414,  2416,  2418,  2420,  2424,  2427,  2429,  2431,  2433,  2435,
    2437,  2441,  2443,  2445,  2447,  2450,  2453,  2458,  2462,  2467,
    2470,  2471,  2477,  2481,  2485,  2487,  2491,  2493,  2496,  2497,
    2503,  2507,  2510,  2511,  2515,  2516,  2521,  2524,  2525,  2529,
    2533,  2535,  2536,  2538,  2540,  2542,  2544,  2548,  2550,  2552,
    2554,  2558,  2560,  2562,  2566,  2570,  2573,  2578,  2581,  2586,
    2588,  2590,  2592,  2594,  2596,  2600,  2606,  2610,  2615,  2620,
    2624,  2626,  2628,  2630,  2632,  2636,  2642,  2647,  2651,  2653,
    2655,  2657,  2659,  2661,  2665,  2669,  2674,  2679,  2683,  2685,
    2687,  2695,  2705,  2713,  2720,  2729,  2731,  2734,  2739,  2744,
    2746,  2748,  2753,  2755,  2756,  2758,  2761,  2763,  2765,  2769,
    2775,  2779,  2783,  2784,  2786,  2790,  2796,  2800,  2803,  2807,
    2814,  2815,  2817,  2822,  2825,  2826,  2832,  2836,  2840,  2842,
    2849,  2854,  2859,  2862,  2865,  2866,  2872,  2876,  2880,  2882,
    2885,  2886,  2892,  2896,  2900,  2902,  2905,  2908,  2910,  2913,
    2915,  2920,  2924,  2928,  2935,  2939,  2941,  2943,  2945,  2950,
    2955,  2960,  2965,  2970,  2975,  2978,  2981,  2986,  2989,  2992,
    2994,  2998,  3002,  3006,  3007,  3010,  3016,  3023,  3030,  3038,
    3040,  3043,  3045,  3050,  3054,  3055,  3057,  3061,  3064,  3068,
    3070,  3072,  3073,  3074,  3077,  3080,  3083,  3088,  3091,  3097,
    3101,  3103,  3105,  3106,  3110,  3115,  3121,  3125,  3127,  3130,
    3131,  3136,  3138,  3142,  3145,  3148,  3151,  3153,  3155,  3157,
    3159,  3163,  3168,  3175,  3177,  3186,  3193,  3195
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     210,     0,    -1,    -1,   211,   212,    -1,   212,   213,    -1,
      -1,   231,    -1,   248,    -1,   255,    -1,   252,    -1,   260,
      -1,   466,    -1,   124,   199,   200,   201,    -1,   151,   223,
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
      -1,   224,   469,    -1,   224,   469,    -1,   228,     9,   467,
      14,   407,    -1,   107,   467,    14,   407,    -1,   229,   230,
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
     201,    -1,   121,   199,   463,   200,   201,    -1,   201,    -1,
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
      -1,   247,   246,   468,   249,   199,   288,   200,   473,   322,
      -1,    -1,   326,   247,   246,   468,   250,   199,   288,   200,
     473,   322,    -1,    -1,   428,   325,   247,   246,   468,   251,
     199,   288,   200,   473,   322,    -1,    -1,   161,   216,   253,
      30,   483,   465,   202,   295,   203,    -1,    -1,   428,   161,
     216,   254,    30,   483,   465,   202,   295,   203,    -1,    -1,
     266,   263,   256,   267,   268,   202,   298,   203,    -1,    -1,
     428,   266,   263,   257,   267,   268,   202,   298,   203,    -1,
      -1,   126,   264,   258,   269,   202,   298,   203,    -1,    -1,
     428,   126,   264,   259,   269,   202,   298,   203,    -1,    -1,
     163,   265,   261,   268,   202,   298,   203,    -1,    -1,   428,
     163,   265,   262,   268,   202,   298,   203,    -1,   468,    -1,
     155,    -1,   468,    -1,   468,    -1,   125,    -1,   118,   125,
      -1,   118,   117,   125,    -1,   117,   118,   125,    -1,   117,
     125,    -1,   127,   400,    -1,    -1,   128,   270,    -1,    -1,
     127,   270,    -1,    -1,   400,    -1,   270,     9,   400,    -1,
     400,    -1,   271,     9,   400,    -1,   131,   273,    -1,    -1,
     438,    -1,    35,   438,    -1,   132,   199,   452,   200,    -1,
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
     484,   164,    79,    -1,   287,     9,   429,   328,   484,    35,
     164,    79,    -1,   287,     9,   429,   328,   484,   164,    -1,
     287,   412,    -1,   429,   328,   484,   164,    79,    -1,   429,
     328,   484,    35,   164,    79,    -1,   429,   328,   484,   164,
      -1,    -1,   429,   328,   484,    79,    -1,   429,   328,   484,
      35,    79,    -1,   429,   328,   484,    35,    79,    14,   345,
      -1,   429,   328,   484,    79,    14,   345,    -1,   287,     9,
     429,   328,   484,    79,    -1,   287,     9,   429,   328,   484,
      35,    79,    -1,   287,     9,   429,   328,   484,    35,    79,
      14,   345,    -1,   287,     9,   429,   328,   484,    79,    14,
     345,    -1,   289,     9,   429,   484,   164,    79,    -1,   289,
       9,   429,   484,    35,   164,    79,    -1,   289,     9,   429,
     484,   164,    -1,   289,   412,    -1,   429,   484,   164,    79,
      -1,   429,   484,    35,   164,    79,    -1,   429,   484,   164,
      -1,    -1,   429,   484,    79,    -1,   429,   484,    35,    79,
      -1,   429,   484,    35,    79,    14,   345,    -1,   429,   484,
      79,    14,   345,    -1,   289,     9,   429,   484,    79,    -1,
     289,     9,   429,   484,    35,    79,    -1,   289,     9,   429,
     484,    35,    79,    14,   345,    -1,   289,     9,   429,   484,
      79,    14,   345,    -1,   291,   412,    -1,    -1,   345,    -1,
      35,   438,    -1,   164,   345,    -1,   291,     9,   345,    -1,
     291,     9,   164,   345,    -1,   291,     9,    35,   438,    -1,
     292,     9,   293,    -1,   293,    -1,    79,    -1,   204,   438,
      -1,   204,   202,   345,   203,    -1,   294,     9,    79,    -1,
     294,     9,    79,    14,   407,    -1,    79,    -1,    79,    14,
     407,    -1,   295,   296,    -1,    -1,   297,   201,    -1,   467,
      14,   407,    -1,   298,   299,    -1,    -1,    -1,   324,   300,
     330,   201,    -1,    -1,   326,   483,   301,   330,   201,    -1,
     331,   201,    -1,   332,   201,    -1,   333,   201,    -1,    -1,
     325,   247,   246,   468,   199,   302,   286,   200,   473,   323,
      -1,    -1,   428,   325,   247,   246,   468,   199,   303,   286,
     200,   473,   323,    -1,   157,   308,   201,    -1,   158,   316,
     201,    -1,   160,   318,   201,    -1,     4,   127,   400,   201,
      -1,     4,   128,   400,   201,    -1,   112,   271,   201,    -1,
     112,   271,   202,   304,   203,    -1,   304,   305,    -1,   304,
     306,    -1,    -1,   227,   150,   216,   165,   271,   201,    -1,
     307,    97,   325,   216,   201,    -1,   307,    97,   326,   201,
      -1,   227,   150,   216,    -1,   216,    -1,   309,    -1,   308,
       9,   309,    -1,   310,   397,   314,   315,    -1,   155,    -1,
      29,   311,    -1,   311,    -1,   133,    -1,   133,   171,   483,
     172,    -1,   133,   171,   483,     9,   483,   172,    -1,   400,
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
     331,     9,   467,    14,   407,    -1,   107,   467,    14,   407,
      -1,   332,     9,   467,    -1,   118,   107,   467,    -1,   118,
     334,   465,    -1,   334,   465,    14,   483,    -1,   107,   176,
     468,    -1,   199,   335,   200,    -1,    68,   402,   405,    -1,
      67,   345,    -1,   389,    -1,   365,    -1,   199,   345,   200,
      -1,   337,     9,   345,    -1,   345,    -1,   337,    -1,    -1,
      27,    -1,    27,   345,    -1,    27,   345,   131,   345,    -1,
     438,    14,   339,    -1,   132,   199,   452,   200,    14,   339,
      -1,    28,   345,    -1,   438,    14,   342,    -1,   132,   199,
     452,   200,    14,   342,    -1,   346,    -1,   438,    -1,   335,
      -1,   442,    -1,   441,    -1,   132,   199,   452,   200,    14,
     345,    -1,   438,    14,   345,    -1,   438,    14,    35,   438,
      -1,   438,    14,    35,    68,   402,   405,    -1,   438,    26,
     345,    -1,   438,    25,   345,    -1,   438,    24,   345,    -1,
     438,    23,   345,    -1,   438,    22,   345,    -1,   438,    21,
     345,    -1,   438,    20,   345,    -1,   438,    19,   345,    -1,
     438,    18,   345,    -1,   438,    17,   345,    -1,   438,    16,
     345,    -1,   438,    15,   345,    -1,   438,    64,    -1,    64,
     438,    -1,   438,    63,    -1,    63,   438,    -1,   345,    31,
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
     345,    30,   345,    -1,   345,    29,    30,   345,    -1,   462,
      -1,    62,   345,    -1,    61,   345,    -1,    60,   345,    -1,
      59,   345,    -1,    58,   345,    -1,    57,   345,    -1,    56,
     345,    -1,    69,   403,    -1,    55,   345,    -1,   409,    -1,
     364,    -1,   363,    -1,   205,   404,   205,    -1,    13,   345,
      -1,   367,    -1,   112,   199,   388,   412,   200,    -1,    -1,
      -1,   247,   246,   199,   349,   288,   200,   473,   347,   202,
     229,   203,    -1,    -1,   326,   247,   246,   199,   350,   288,
     200,   473,   347,   202,   229,   203,    -1,    -1,    79,   352,
     357,    -1,    -1,   181,    79,   353,   357,    -1,    -1,   196,
     354,   288,   197,   473,   357,    -1,    -1,   181,   196,   355,
     288,   197,   473,   357,    -1,    -1,   181,   202,   356,   229,
     203,    -1,     8,   345,    -1,     8,   342,    -1,     8,   202,
     229,   203,    -1,    86,    -1,   464,    -1,   359,     9,   358,
     131,   345,    -1,   358,   131,   345,    -1,   360,     9,   358,
     131,   407,    -1,   358,   131,   407,    -1,   359,   411,    -1,
      -1,   360,   411,    -1,    -1,   175,   199,   361,   200,    -1,
     133,   199,   453,   200,    -1,    66,   453,   206,    -1,   400,
     202,   455,   203,    -1,   400,   202,   457,   203,    -1,   367,
      66,   448,   206,    -1,   368,    66,   448,   206,    -1,   364,
      -1,   464,    -1,   441,    -1,    86,    -1,   199,   346,   200,
      -1,   371,   372,    -1,   438,    14,   369,    -1,   182,    79,
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
     200,    -1,   227,    -1,   155,    -1,   400,    -1,   119,    -1,
     446,    -1,   400,    -1,   119,    -1,   450,    -1,   199,   200,
      -1,   336,    -1,    -1,    -1,    85,    -1,   459,    -1,   199,
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
      78,    -1,   464,    -1,   406,    -1,   207,   459,   207,    -1,
     208,   459,   208,    -1,   146,   459,   147,    -1,   413,   411,
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
     445,    -1,   216,    -1,   202,   345,   203,    -1,   433,    -1,
     445,    -1,    66,   448,   206,    -1,   202,   345,   203,    -1,
     439,   435,    -1,   199,   335,   200,   435,    -1,   451,   435,
      -1,   199,   335,   200,   435,    -1,   445,    -1,   399,    -1,
     443,    -1,   444,    -1,   436,    -1,   438,   430,   432,    -1,
     199,   335,   200,   430,   432,    -1,   401,   150,   445,    -1,
     440,   199,   290,   200,    -1,   441,   199,   290,   200,    -1,
     199,   438,   200,    -1,   399,    -1,   443,    -1,   444,    -1,
     436,    -1,   438,   430,   431,    -1,   199,   335,   200,   430,
     431,    -1,   440,   199,   290,   200,    -1,   199,   438,   200,
      -1,   445,    -1,   436,    -1,   399,    -1,   364,    -1,   406,
      -1,   199,   438,   200,    -1,   199,   346,   200,    -1,   441,
     199,   290,   200,    -1,   440,   199,   290,   200,    -1,   199,
     442,   200,    -1,   348,    -1,   351,    -1,   438,   430,   434,
     469,   199,   290,   200,    -1,   199,   335,   200,   430,   434,
     469,   199,   290,   200,    -1,   401,   150,   216,   469,   199,
     290,   200,    -1,   401,   150,   445,   199,   290,   200,    -1,
     401,   150,   202,   345,   203,   199,   290,   200,    -1,   446,
      -1,   449,   446,    -1,   446,    66,   448,   206,    -1,   446,
     202,   345,   203,    -1,   447,    -1,    79,    -1,   204,   202,
     345,   203,    -1,   345,    -1,    -1,   204,    -1,   449,   204,
      -1,   445,    -1,   437,    -1,   450,   430,   432,    -1,   199,
     335,   200,   430,   432,    -1,   401,   150,   445,    -1,   199,
     438,   200,    -1,    -1,   437,    -1,   450,   430,   431,    -1,
     199,   335,   200,   430,   431,    -1,   199,   438,   200,    -1,
     452,     9,    -1,   452,     9,   438,    -1,   452,     9,   132,
     199,   452,   200,    -1,    -1,   438,    -1,   132,   199,   452,
     200,    -1,   454,   411,    -1,    -1,   454,     9,   345,   131,
     345,    -1,   454,     9,   345,    -1,   345,   131,   345,    -1,
     345,    -1,   454,     9,   345,   131,    35,   438,    -1,   454,
       9,    35,   438,    -1,   345,   131,    35,   438,    -1,    35,
     438,    -1,   456,   411,    -1,    -1,   456,     9,   345,   131,
     345,    -1,   456,     9,   345,    -1,   345,   131,   345,    -1,
     345,    -1,   458,   411,    -1,    -1,   458,     9,   407,   131,
     407,    -1,   458,     9,   407,    -1,   407,   131,   407,    -1,
     407,    -1,   459,   460,    -1,   459,    85,    -1,   460,    -1,
      85,   460,    -1,    79,    -1,    79,    66,   461,   206,    -1,
      79,   430,   216,    -1,   148,   345,   203,    -1,   148,    78,
      66,   345,   206,   203,    -1,   149,   438,   203,    -1,   216,
      -1,    80,    -1,    79,    -1,   122,   199,   463,   200,    -1,
     123,   199,   438,   200,    -1,   123,   199,   346,   200,    -1,
     123,   199,   442,   200,    -1,   123,   199,   441,   200,    -1,
     123,   199,   335,   200,    -1,     7,   345,    -1,     6,   345,
      -1,     5,   199,   345,   200,    -1,     4,   345,    -1,     3,
     345,    -1,   438,    -1,   463,     9,   438,    -1,   401,   150,
     216,    -1,   401,   150,   125,    -1,    -1,    97,   483,    -1,
     176,   468,    14,   483,   201,    -1,   428,   176,   468,    14,
     483,   201,    -1,   178,   468,   465,    14,   483,   201,    -1,
     428,   178,   468,   465,    14,   483,   201,    -1,   216,    -1,
     483,   216,    -1,   216,    -1,   216,   171,   475,   172,    -1,
     171,   471,   172,    -1,    -1,   483,    -1,   470,     9,   483,
      -1,   470,   411,    -1,   470,     9,   164,    -1,   471,    -1,
     164,    -1,    -1,    -1,    30,   483,    -1,    97,   483,    -1,
      98,   483,    -1,   475,     9,   476,   216,    -1,   476,   216,
      -1,   475,     9,   476,   216,   474,    -1,   476,   216,   474,
      -1,    46,    -1,    47,    -1,    -1,    86,   131,   483,    -1,
      29,    86,   131,   483,    -1,   227,   150,   216,   131,   483,
      -1,   478,     9,   477,    -1,   477,    -1,   478,   411,    -1,
      -1,   175,   199,   479,   200,    -1,   227,    -1,   216,   150,
     482,    -1,   216,   469,    -1,    29,   483,    -1,    55,   483,
      -1,   227,    -1,   133,    -1,   134,    -1,   480,    -1,   481,
     150,   482,    -1,   133,   171,   483,   172,    -1,   133,   171,
     483,     9,   483,   172,    -1,   155,    -1,   199,   106,   199,
     472,   200,    30,   483,   200,    -1,   199,   483,     9,   470,
     411,   200,    -1,   483,    -1,    -1
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
    2362,  2363,  2366,  2367,  2368,  2372,  2373,  2374,  2378,  2379,
    2380,  2384,  2385,  2386,  2389,  2391,  2395,  2396,  2397,  2398,
    2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,
    2412,  2417,  2418,  2419,  2421,  2422,  2424,  2425,  2426,  2427,
    2429,  2431,  2433,  2435,  2437,  2438,  2439,  2440,  2441,  2442,
    2443,  2444,  2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,
    2453,  2455,  2457,  2459,  2461,  2462,  2465,  2466,  2470,  2472,
    2476,  2479,  2482,  2488,  2489,  2490,  2491,  2492,  2493,  2494,
    2499,  2501,  2505,  2506,  2509,  2510,  2514,  2517,  2519,  2521,
    2525,  2526,  2527,  2528,  2531,  2535,  2536,  2537,  2538,  2542,
    2544,  2551,  2552,  2553,  2554,  2555,  2556,  2558,  2559,  2564,
    2566,  2569,  2572,  2574,  2576,  2579,  2581,  2585,  2587,  2590,
    2593,  2599,  2601,  2604,  2605,  2610,  2613,  2617,  2617,  2622,
    2625,  2626,  2630,  2631,  2635,  2636,  2637,  2641,  2642,  2646,
    2647,  2651,  2652,  2656,  2657,  2661,  2662,  2667,  2669,  2674,
    2675,  2676,  2677,  2678,  2679,  2689,  2700,  2703,  2705,  2707,
    2711,  2712,  2713,  2714,  2715,  2726,  2738,  2740,  2744,  2745,
    2746,  2747,  2748,  2749,  2750,  2751,  2753,  2758,  2762,  2763,
    2767,  2770,  2777,  2781,  2785,  2792,  2793,  2798,  2800,  2801,
    2804,  2805,  2808,  2809,  2813,  2814,  2818,  2819,  2820,  2831,
    2842,  2845,  2848,  2849,  2850,  2861,  2873,  2877,  2878,  2879,
    2881,  2882,  2883,  2887,  2889,  2892,  2894,  2895,  2896,  2897,
    2900,  2902,  2903,  2907,  2909,  2912,  2914,  2915,  2916,  2920,
    2922,  2925,  2928,  2930,  2932,  2936,  2937,  2939,  2940,  2946,
    2947,  2949,  2959,  2961,  2963,  2966,  2967,  2968,  2972,  2973,
    2974,  2975,  2976,  2977,  2978,  2979,  2980,  2981,  2982,  2986,
    2987,  2991,  2993,  3001,  3003,  3007,  3011,  3016,  3020,  3028,
    3029,  3035,  3036,  3044,  3047,  3051,  3054,  3059,  3064,  3066,
    3067,  3068,  3072,  3073,  3077,  3078,  3081,  3083,  3084,  3087,
    3092,  3093,  3094,  3098,  3102,  3112,  3120,  3122,  3126,  3128,
    3133,  3139,  3142,  3145,  3152,  3155,  3158,  3159,  3162,  3165,
    3166,  3171,  3174,  3178,  3182,  3188,  3198,  3199
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
  "variable", "dimmable_variable", "callable_variable",
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
     400,   400,   401,   401,   401,   402,   402,   402,   403,   403,
     403,   404,   404,   404,   405,   405,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     408,   408,   408,   409,   409,   409,   409,   409,   409,   409,
     410,   410,   411,   411,   412,   412,   413,   413,   413,   413,
     414,   414,   414,   414,   414,   415,   415,   415,   415,   416,
     416,   417,   417,   417,   417,   417,   417,   417,   417,   418,
     418,   419,   419,   419,   419,   420,   420,   421,   421,   422,
     422,   423,   423,   424,   424,   425,   425,   427,   426,   428,
     429,   429,   430,   430,   431,   431,   431,   432,   432,   433,
     433,   434,   434,   435,   435,   436,   436,   437,   437,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     439,   439,   439,   439,   439,   439,   439,   439,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   441,   442,   442,
     443,   443,   444,   444,   444,   445,   445,   446,   446,   446,
     447,   447,   448,   448,   449,   449,   450,   450,   450,   450,
     450,   450,   451,   451,   451,   451,   451,   452,   452,   452,
     452,   452,   452,   453,   453,   454,   454,   454,   454,   454,
     454,   454,   454,   455,   455,   456,   456,   456,   456,   457,
     457,   458,   458,   458,   458,   459,   459,   459,   459,   460,
     460,   460,   460,   460,   460,   461,   461,   461,   462,   462,
     462,   462,   462,   462,   462,   462,   462,   462,   462,   463,
     463,   464,   464,   465,   465,   466,   466,   466,   466,   467,
     467,   468,   468,   469,   469,   470,   470,   471,   472,   472,
     472,   472,   473,   473,   474,   474,   475,   475,   475,   475,
     476,   476,   476,   477,   477,   477,   478,   478,   479,   479,
     480,   481,   482,   482,   483,   483,   483,   483,   483,   483,
     483,   483,   483,   483,   483,   483,   484,   484
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
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     4,     3,     4,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       2,     0,     1,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     2,     2,     4,     3,     4,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     3,     3,     2,     4,     2,     4,     1,
       1,     1,     1,     1,     3,     5,     3,     4,     4,     3,
       1,     1,     1,     1,     3,     5,     4,     3,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     3,     5,
       3,     3,     0,     1,     3,     5,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     6,     7,     1,
       2,     1,     4,     3,     0,     1,     3,     2,     3,     1,
       1,     0,     0,     2,     2,     2,     4,     2,     5,     3,
       1,     1,     0,     3,     4,     5,     3,     1,     2,     0,
       4,     1,     3,     2,     2,     2,     1,     1,     1,     1,
       3,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   372,     0,   757,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   844,     0,
     832,   640,     0,   646,   647,   648,    22,   704,   820,   100,
     101,   649,     0,    81,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   134,     0,     0,     0,     0,     0,     0,
     339,   340,   341,   344,   343,   342,     0,     0,     0,     0,
     161,     0,     0,     0,   653,   655,   656,   650,   651,     0,
       0,     0,   657,   652,     0,   631,    24,    25,    26,    28,
      27,     0,   654,     0,     0,     0,     0,   658,   345,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   464,     0,    99,    71,   824,   641,     0,     0,
       4,    60,    62,    65,   703,     0,   630,     0,     6,   133,
       7,     9,     8,    10,     0,     0,   337,   382,     0,     0,
       0,     0,     0,     0,     0,   380,   808,   809,   450,   449,
     366,   453,     0,     0,   365,   780,   632,     0,   706,   448,
     336,   783,   381,     0,     0,   384,   383,   781,   782,   779,
     815,   819,     0,   438,   705,    11,   344,   343,   342,     0,
       0,    28,    60,   133,     0,   888,   381,   887,     0,   885,
     884,   452,     0,   373,   377,     0,     0,   422,   423,   424,
     425,   447,   445,   444,   443,   442,   441,   440,   439,   820,
     649,   633,     0,     0,   904,   801,   632,     0,   802,   404,
       0,   402,     0,   848,     0,   713,   364,   636,     0,   904,
     635,     0,   645,   827,   826,   637,     0,     0,   639,   446,
       0,     0,     0,     0,   369,     0,    79,   371,     0,     0,
      85,    87,     0,     0,    89,     0,     0,     0,   937,   938,
     943,     0,     0,    60,   936,     0,   939,     0,     0,     0,
      91,     0,     0,     0,     0,   124,     0,     0,     0,     0,
       0,     0,    43,    48,   250,     0,     0,   249,     0,   165,
       0,   162,   255,     0,     0,     0,     0,     0,   901,   149,
     159,   840,   844,     0,   869,     0,   660,     0,     0,     0,
     867,     0,    16,     0,    64,   141,   153,   160,   537,   480,
       0,   893,   462,   466,   468,   761,   382,     0,   380,   381,
     383,     0,     0,   642,     0,   643,     0,     0,     0,   123,
       0,     0,    67,   241,     0,    21,   132,     0,   158,   145,
     157,   342,   345,   133,   338,   114,   115,   116,   117,   118,
     120,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   832,     0,   113,   823,
     823,   121,   854,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   403,   401,   762,   763,     0,
     823,     0,   775,   241,   241,   823,     0,   825,   816,   840,
       0,   133,     0,     0,    93,     0,   759,   754,   713,     0,
     382,   380,     0,   852,     0,   485,   712,   843,     0,     0,
       0,    67,     0,   241,   363,     0,   777,   638,     0,    71,
     201,     0,   461,     0,    96,     0,     0,   370,     0,     0,
       0,     0,     0,    88,   112,    90,   934,   935,     0,   929,
       0,     0,     0,     0,   900,     0,   119,    92,   122,     0,
       0,     0,     0,     0,     0,     0,   495,     0,   502,   504,
     505,   506,   507,   508,   509,   500,   522,   523,    71,     0,
     109,   111,     0,     0,    45,    52,     0,     0,    47,    56,
      49,     0,    18,     0,     0,   251,     0,    94,   164,   163,
       0,     0,    95,   889,     0,     0,   382,   380,   381,   384,
     383,     0,   922,   171,     0,   841,     0,     0,    97,     0,
       0,   659,   868,   704,     0,     0,   866,   709,   865,    63,
       5,    13,    14,     0,   169,     0,     0,   473,     0,     0,
     713,     0,     0,   634,   474,     0,     0,     0,     0,   761,
      71,     0,   715,   760,   947,   362,   435,   789,   807,    76,
      70,    72,    73,    74,    75,   336,     0,   451,   707,   708,
      61,   713,     0,   905,     0,     0,     0,   715,   242,     0,
     456,   135,   167,     0,   407,   409,   408,     0,     0,   405,
     406,   410,   412,   411,   427,   426,   429,   428,   430,   432,
     433,   431,   421,   420,   414,   415,   413,   416,   417,   419,
     434,   418,   822,     0,     0,   858,     0,   713,   892,     0,
     891,   786,   815,   151,   143,   155,     0,   893,   147,   133,
     372,     0,   375,   378,   386,   496,   400,   399,   398,   397,
     396,   395,   394,   393,   392,   391,   390,   389,   765,     0,
     764,   767,   784,   771,   904,   768,     0,     0,     0,     0,
       0,     0,     0,     0,   886,   374,   752,   756,   712,   758,
       0,   804,   904,     0,   847,     0,   846,     0,   831,   830,
       0,     0,   764,   767,   828,   768,   367,   203,   205,    71,
     471,   470,   368,     0,    71,   185,    80,   371,     0,     0,
       0,     0,     0,   197,   197,    86,     0,     0,     0,     0,
     927,   713,     0,   911,     0,     0,     0,     0,     0,   711,
       0,   631,     0,     0,    65,   662,   630,   667,     0,   661,
      69,   666,   904,   940,     0,     0,   512,     0,     0,   518,
     515,   516,   524,     0,   503,   498,     0,   501,     0,     0,
       0,    53,    19,     0,     0,    57,    20,     0,     0,     0,
      42,    50,     0,   248,   256,   253,     0,     0,   878,   883,
     880,   879,   882,   881,    12,   920,   921,     0,     0,     0,
       0,   840,   837,     0,   484,   877,   876,   875,     0,   871,
       0,   872,   874,     0,     5,     0,     0,     0,   531,   532,
     540,   539,     0,     0,   712,   479,   483,     0,     0,   894,
       0,   463,     0,     0,   912,   761,   227,   946,     0,     0,
     776,   821,   712,   907,   903,   243,   244,   629,   714,   240,
       0,   761,     0,     0,   169,   458,   137,   437,     0,   488,
     489,     0,   486,   712,   853,     0,     0,   241,   171,     0,
     169,     0,     0,   167,     0,   832,   387,     0,     0,   773,
     774,   787,   788,   817,   818,     0,     0,     0,   740,   720,
     721,   722,   729,     0,     0,     0,   733,   731,   732,   746,
     713,     0,   754,   851,   850,     0,     0,   778,   644,     0,
     207,     0,     0,    77,     0,     0,     0,     0,     0,     0,
       0,   177,   178,   189,     0,    71,   187,   106,   197,     0,
     197,     0,     0,   941,     0,     0,     0,   712,   928,   930,
     910,   713,   909,     0,   713,   688,   689,   686,   687,   719,
       0,   713,   711,     0,   482,     0,     0,   860,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   933,   497,     0,     0,     0,
     520,   521,   519,     0,     0,   499,     0,   125,     0,   128,
     110,     0,    44,    54,     0,    46,    58,    51,   252,     0,
     890,    98,   922,   902,   917,   170,   172,   262,     0,     0,
     838,     0,   870,     0,    17,     0,   893,   168,   262,     0,
       0,   476,     0,   891,   895,     0,   912,   469,     0,     0,
     947,     0,   232,   230,   767,   785,   904,   906,     0,     0,
     245,    68,     0,   761,   166,     0,   761,     0,   436,   857,
     856,     0,   241,     0,     0,     0,     0,     0,     0,   169,
     139,   645,   766,   241,     0,   725,   726,   727,   728,   734,
     735,   744,     0,   713,     0,   740,     0,   724,   748,   712,
     751,   753,   755,     0,   845,   767,   829,   766,     0,     0,
       0,     0,   204,   472,    82,     0,   371,   177,   179,   840,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   191,
       0,     0,   923,     0,   926,   712,     0,     0,     0,   664,
     712,   710,     0,   701,     0,   713,     0,   668,   702,   700,
     864,     0,   713,   671,   673,   672,     0,     0,   669,   670,
     674,   676,   675,   691,   690,   693,   692,   694,   696,   697,
     695,   684,   683,   678,   679,   677,   680,   681,   682,   685,
     932,   510,     0,   511,   517,   525,   526,     0,    71,    55,
      59,   254,     0,     0,     0,   919,     0,   336,   842,   840,
     376,   379,   385,     0,    15,     0,   336,   543,     0,     0,
     545,   538,   541,     0,   536,     0,   897,     0,   913,   465,
       0,   233,     0,     0,   228,     0,   247,   246,   912,     0,
     262,     0,   761,     0,   241,     0,   813,   262,   893,   262,
     896,     0,     0,     0,   388,     0,     0,   737,   712,   739,
     730,     0,   723,     0,     0,   713,   745,   849,     0,    71,
       0,   200,   186,     0,     0,     0,   176,   102,   190,     0,
       0,   193,     0,   198,   199,    71,   192,   942,   924,     0,
     908,     0,   945,   718,   717,   663,     0,   712,   481,   665,
       0,   487,   712,   859,   699,     0,     0,     0,     0,   916,
     914,   915,   173,     0,     0,     0,   343,   334,     0,     0,
       0,   150,   261,   263,     0,   333,     0,     0,     0,   893,
     336,     0,   873,   258,   154,   534,     0,     0,   475,   467,
       0,   236,   226,     0,   229,   235,   241,   455,   912,   336,
     912,     0,   855,     0,   812,   336,     0,   336,   898,   262,
     761,   810,   743,   742,   736,     0,   738,   712,   747,    71,
     206,    78,    83,   104,   180,     0,   188,   194,    71,   196,
     925,     0,     0,   478,     0,   863,   862,   698,     0,    71,
     129,   918,     0,     0,     0,     0,     0,   174,     0,   893,
       0,   299,   295,   301,   631,    28,     0,   289,     0,   294,
     298,   311,     0,   309,   314,     0,   313,     0,   312,     0,
     133,   265,     0,   267,     0,   268,   269,     0,     0,   839,
       0,   535,   533,   544,   542,   237,     0,     0,   224,   234,
       0,     0,     0,     0,   146,   455,   912,   814,   152,   258,
     156,   336,     0,     0,   750,     0,   202,     0,     0,    71,
     183,   103,   195,   944,   716,     0,     0,     0,     0,     0,
       0,   361,     0,     0,   279,   283,   358,   359,   293,     0,
       0,     0,   274,   595,   594,   591,   593,   592,   612,   614,
     613,   583,   554,   555,   573,   589,   588,   550,   560,   561,
     563,   562,   582,   566,   564,   565,   567,   568,   569,   570,
     571,   572,   574,   575,   576,   577,   578,   579,   581,   580,
     551,   552,   553,   556,   557,   559,   597,   598,   607,   606,
     605,   604,   603,   602,   590,   609,   599,   600,   601,   584,
     585,   586,   587,   610,   611,   615,   617,   616,   618,   619,
     596,   621,   620,   623,   625,   624,   558,   628,   626,   627,
     622,   608,   549,   306,   546,     0,   275,   327,   328,   326,
     319,     0,   320,   276,   353,     0,     0,     0,     0,   357,
       0,   133,   142,   257,     0,     0,     0,   225,   239,   811,
       0,    71,   329,    71,   136,     0,     0,     0,   148,   912,
     741,     0,    71,   181,    84,   105,     0,   477,   861,   513,
     127,   277,   278,   356,   175,     0,     0,   713,     0,   302,
     290,     0,     0,     0,   308,   310,     0,     0,   315,   322,
     323,   321,     0,     0,   264,     0,     0,     0,   360,     0,
     259,     0,   238,     0,   529,   715,     0,     0,    71,   138,
     144,     0,   749,     0,     0,     0,   107,   280,    60,     0,
     281,   282,     0,     0,   296,   712,   304,   300,   305,   547,
     548,     0,   291,   324,   325,   317,   318,   316,   354,   351,
     270,   266,   355,     0,   260,   530,   714,     0,   457,   330,
       0,   140,     0,   184,   514,     0,   131,     0,   336,     0,
     303,   307,     0,   761,   272,     0,   527,   454,   459,   182,
       0,     0,   108,   287,     0,   335,   297,   352,     0,   715,
     347,   761,   528,     0,   130,     0,     0,   286,   912,   761,
     211,   348,   349,   350,   947,   346,     0,     0,     0,   285,
       0,   347,     0,   912,     0,   284,   331,    71,   271,   947,
       0,   216,   214,     0,    71,     0,     0,   217,     0,     0,
     212,   273,     0,   332,     0,   220,   210,     0,   213,   219,
     126,   221,     0,     0,   208,   218,     0,   209,   223,   222
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   824,   560,   182,   281,   513,
     517,   282,   514,   518,   122,   123,   124,   125,   126,   127,
     331,   590,   591,   466,   245,  1447,   472,  1365,  1448,  1686,
     780,   276,   508,  1646,  1010,  1188,  1702,   347,   183,   592,
     862,  1067,  1243,   131,   563,   879,   593,   612,   883,   543,
     878,   594,   564,   880,   349,   299,   316,   134,   864,   827,
     810,  1025,  1386,  1121,   931,  1594,  1451,   726,   937,   471,
     735,   939,  1275,   718,   920,   923,  1110,  1708,  1709,   581,
     582,   606,   607,   286,   287,   293,  1420,  1573,  1574,  1197,
    1312,  1409,  1567,  1693,  1711,  1605,  1650,  1651,  1652,  1396,
    1397,  1398,  1399,  1607,  1608,  1614,  1662,  1402,  1403,  1407,
    1560,  1561,  1562,  1584,  1738,  1313,  1314,   184,   136,  1724,
    1725,  1565,  1316,  1317,  1318,  1319,   137,   238,   467,   468,
     138,   139,   140,   141,   142,   143,   144,   145,  1432,   146,
     861,  1066,   147,   242,   578,   325,   579,   580,   462,   569,
     570,  1145,   571,  1146,   148,   149,   150,   757,   151,   152,
     273,   153,   274,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   770,   771,  1002,   505,   506,   507,   777,  1635,
     154,   565,  1422,   566,  1039,   832,  1214,  1211,  1553,  1554,
     155,   156,   157,   232,   239,   334,   454,   158,   959,   761,
     159,   960,   853,   846,   961,   907,  1089,   908,  1091,  1092,
    1093,   910,  1254,  1255,   911,   697,   438,   195,   196,   595,
     584,   419,   681,   682,   683,   684,   850,   161,   233,   186,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   643,
     172,   235,   236,   546,   224,   225,   646,   647,  1151,  1152,
     309,   310,   818,   173,   534,   174,   577,   175,  1575,   300,
     342,   601,   602,   953,  1049,  1195,   807,   808,   740,   741,
     742,   266,   267,   763,   268,   848
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1435
static const yytype_int16 yypact[] =
{
   -1435,   106, -1435, -1435,  5444, 13478, 13478,   -52, 13478, 13478,
   13478, 11418, 13478, -1435, 13478, 13478, 13478, 13478, 13478, 13478,
   13478, 13478, 13478, 13478, 13478, 13478, 16109, 16109, 11624, 13478,
   16160,     5,     7, -1435, -1435, -1435, -1435, -1435,   203, -1435,
   -1435,   164, 13478, -1435,     7,   118,   195,   236, -1435,     7,
   11830, 17235, 12036, -1435, 14892, 10388,   247, 13478, 16900,    36,
   -1435, -1435, -1435,    80,   342,   312,   239,   266,   269,   307,
   -1435, 17235,   313,   317, -1435, -1435, -1435, -1435, -1435, 13478,
     493,  3332, -1435, -1435, 17235, -1435, -1435, -1435, -1435, 17235,
   -1435, 17235, -1435,   159,   340, 17235, 17235, -1435,    24, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, 13478, -1435, -1435,   359,   459,   462,   462,
   -1435,   314,   360,   380, -1435,   355, -1435,    74, -1435,   529,
   -1435, -1435, -1435, -1435, 17004,   519, -1435, -1435,   369,   379,
     383,   392,   402,   404, 14116, -1435, -1435, -1435, -1435,    46,
   -1435,   521,   553,   425, -1435,    29,   428,   489,   446, -1435,
     778,    39,   860,    47,   455,    53, -1435,    71,    79,   457,
      44, -1435,    50, -1435,   594, -1435, -1435, -1435,   525,   469,
     534, -1435, -1435,   529,   519, 17492,  1143, 17492, 13478, 17492,
   17492, 10577,   494, 16506, 10577,   672, 17235,   668,   668,   160,
     668,   668,   668,   668,   668,   668,   668,   668,   668, -1435,
   -1435, -1435,   308, 13478,   569, -1435, -1435,   586, -1435,   315,
     559,   315, 16109, 16550,   535,   754, -1435,   525, 15453,   569,
     616,   624,   580,   137, -1435,   315,    47, 12242, -1435, -1435,
   13478,  8946,   772,    77, 17492,  9976, -1435, 13478, 13478, 17235,
   -1435, -1435, 14160,   581, -1435, 14204, 14892, 14892,   612, -1435,
   -1435,   582, 14630,    63,   638,   775, -1435,   643, 17235,   715,
   -1435,   601, 14248,   609,   780, -1435,   336, 14292, 17019, 17062,
   17235,    81, -1435,   372, -1435, 15584,    82, -1435,   683, -1435,
     686, -1435,   798,    84, 16109, 16109, 13478,   613,   649, -1435,
   -1435, 15715, 11624,    85,    72,   518, -1435, 13684, 16109,   510,
   -1435, 17235, -1435,   344,   360, -1435, -1435, -1435, -1435, 16279,
     808,   726, -1435, -1435, -1435,    56,   625, 17492,   626,   728,
     629,  5650, 13478,   334,   619,   480,   334,   385,   352, -1435,
   17235, 14892,   631, 10594, 14892, -1435, -1435,  4097, -1435, -1435,
   -1435, -1435, -1435,   529, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, 13478, 13478, 13478, 12448, 13478, 13478, 13478, 13478, 13478,
   13478, 13478, 13478, 13478, 13478, 13478, 13478, 13478, 13478, 13478,
   13478, 13478, 13478, 13478, 13478, 13478, 16160, 13478, -1435, 13478,
   13478, -1435, 13478,  2689, 17235, 17235, 17235, 17235, 17235, 17004,
     725,   654, 10182, 13478, 13478, 13478, 13478, 13478, 13478, 13478,
   13478, 13478, 13478, 13478, 13478, -1435, -1435, -1435, -1435, 14965,
   13478, 13478, -1435, 10594, 10594, 13478, 13478,   359,   146, 15715,
     644,   529, 12654,   124, -1435, 13478, -1435,   648,   825,   704,
     652,   655,  3643,   315, 12860, -1435, 13066, -1435, 13478,   660,
      10, -1435,   142, 10594, -1435, 16320, -1435, -1435, 14336, -1435,
   -1435, 10800, -1435, 13478, -1435,   776,  9152,   855,   670, 17403,
     859,    57,    37, -1435, -1435, -1435, -1435, -1435, 14892,  2038,
     689,   882, 15323, 17235, -1435,   713, -1435, -1435, -1435,   823,
   13478,   826,   829, 13478, 13478, 13478, -1435,   780, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435,   730, -1435, -1435, -1435,   720,
   -1435, -1435, 17235,   719,   905,   374, 17235,   731,   912,   377,
     408, 17077, -1435, 17235, 13478,   315,    36, -1435, -1435, -1435,
   15323,   848, -1435,   315,    60,    69,   729,   733,  2496,   253,
     734,   737,   502,   804,   738,   315,   107,   735, -1435, 16958,
   17235, -1435, -1435,   883,  2705,    -3, -1435, -1435, -1435,   360,
   -1435, -1435, -1435,   921,   827,   787,   382, -1435,   359,   830,
     949,   760,   815,   146, -1435, 14892, 14892,   954,   772,    56,
   -1435,   785,   966, -1435, 14892,    21,   143,   129, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435,   681,  4025, -1435, -1435, -1435,
   -1435,   979,   819, -1435, 16109, 13478,   796,   993, 17492,   989,
   -1435, -1435,   877, 15244, 10989, 11607, 10577, 13478, 17448, 13049,
   13926, 14421,  3746, 12426, 15778, 15778, 15778, 15778,  3106,  3106,
    3106,  3106,   930,   930,   711,   711,   711,   160,   160,   160,
   -1435,   668, 17492,   799,   802, 16607,   806,  1001, -1435, 13478,
      28,   812,   146, -1435, -1435, -1435,   999,   726, -1435,   529,
   13478, 15847, -1435, -1435, 10577, -1435, 10577, 10577, 10577, 10577,
   10577, 10577, 10577, 10577, 10577, 10577, 10577, 10577, -1435, 13478,
     403,   149, -1435, -1435,   569,   405,   809,  4360,   814,   816,
     811,  4422,   112,   820, -1435, 17492,  3032, -1435, 17235, -1435,
      21, -1435,   569, 16109, 17492, 16109, 16651,    21,   150, -1435,
     818, 13478, -1435,   151, -1435, -1435, -1435,  8740,   542, -1435,
   -1435, 17492, 17492,     7, -1435, -1435, -1435, 13478,   923, 15172,
   15323, 17235,  9358,   821,   822, -1435,    75,   935,   893,   875,
   -1435,  1018,   828, 14705, 14892, 15323, 15323, 15323, 15323, 15323,
     832,   879,   834, 15323,   419, -1435,   885, -1435,   835, -1435,
   17578, -1435,   -30, -1435, 13478,   851, 17492,   856,  1026, 11403,
    1035, -1435, 17492, 14380, -1435,   730,   967, -1435,  5856, 16839,
     843,   412, -1435, 17019, 17235,   414, -1435, 17062, 17235, 17235,
   -1435, -1435,  4558, -1435, 17578,  1033, 16109,   847, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435,    76, 17235, 16839,
     849, 15715, 15978,  1038, -1435, -1435, -1435, -1435,   850, -1435,
   13478, -1435, -1435,  5032, -1435, 14892, 16839,   857, -1435, -1435,
   -1435, -1435,  1044, 13478, 16279, -1435, -1435, 17181,   862, -1435,
   14892, -1435,   852,  6062,  1034,   140, -1435, -1435,   158, 14965,
   -1435, -1435, 14892, -1435, -1435,   315, 17492, -1435, 11006, -1435,
   15323,   109,   866, 16839,   827, -1435, -1435, 12637, 13478, -1435,
   -1435, 13478, -1435, 13478, -1435,  4701,   870, 10594,   804,  1042,
     827, 14892,  1059,   877, 17235, 16160,   315,  4760,   876, -1435,
   -1435,   130,   881, -1435, -1435,  1060, 16259, 16259,  3032, -1435,
   -1435, -1435,  1036,   886,   339,   887, -1435, -1435, -1435, -1435,
    1072,   888,   648,   315,   315, 13272, 16320, -1435, -1435,  4840,
     551,     7,  9976, -1435,  6268,   889,  6474,   891, 15172, 16109,
     890,   951,   315, 17578,  1073, -1435, -1435, -1435, -1435,   570,
   -1435,    30, 14892, -1435,   962, 14892, 17235,  2038, -1435, -1435,
   -1435,  1085, -1435,   895,   979,   736,   736,  1031,  1031, 16752,
     894,  1092, 15323, 17235, 16279,  4624, 17196, 15323, 15323, 15323,
   15323, 15042, 15323, 15323, 15323, 15323, 15323, 15323, 15323, 15323,
   15323, 15323, 15323, 15323, 15323, 15323, 15323, 15323, 15323, 15323,
   15323, 15323, 15323, 15323, 17235, -1435, 17492, 13478, 13478, 13478,
   -1435, -1435, -1435, 13478, 13478, -1435,   780, -1435,  1023, -1435,
   -1435, 17235, -1435, -1435, 17235, -1435, -1435, -1435, -1435, 15323,
     315, -1435,   502, -1435,   565,  1095, -1435, -1435,   113,   906,
     315, 11212, -1435,  2369, -1435,  5238,   726,  1095, -1435,   371,
     386, 17492,   975, -1435, -1435,   907,  1034, -1435, 14892,   772,
   14892,    65,  1093,  1030,   152, -1435,   569, -1435, 16109, 13478,
   17492, 17578,   913,   109, -1435,   910,   109,   915, 12637, 17492,
   16708,   916, 10594,   918,   917, 14892,   920,   922, 14892,   827,
   -1435,   580,   447, 10594, 13478, -1435, -1435, -1435, -1435, -1435,
   -1435,   994,   925,  1117,  1046,  3032,   981, -1435, 16279,  3032,
   -1435, -1435, -1435, 16109, 17492,   153, -1435, -1435,     7,  1103,
    1061,  9976, -1435, -1435, -1435,   934, 13478,   951,   315, 15715,
   15172,   937, 15323,  6680,   597,   938, 13478,    49,    41, -1435,
     968, 14892, -1435,  1010, -1435, 14751,  1112,   943, 15323, -1435,
   15323, -1435,   944, -1435,  1014,  1137,   947, -1435, -1435, -1435,
   16809,   945,  1140,  2980, 12021, 12225, 15323, 17536, 13255, 15238,
   15518, 15648,  2498, 15910, 15910, 15910, 15910,  3045,  3045,  3045,
    3045,   791,   791,   736,   736,   736,  1031,  1031,  1031,  1031,
   -1435, 17492, 13669, 17492, -1435, 17492, -1435,   956, -1435, -1435,
   -1435, 17578, 17235, 14892, 14892, -1435, 16839,  1079, -1435, 15715,
   -1435, -1435, 10577,   969, -1435,   952,  1185, -1435,    67, 13478,
   -1435, -1435, -1435, 13478, -1435, 13478, -1435,   772, -1435, -1435,
     357,  1157,  1094, 13478, -1435,   976,   315, 17492,  1034,   974,
   -1435,   977,   109, 13478, 10594,   983, -1435, -1435,   726, -1435,
   -1435,   980,   982,   988, -1435,   990,  3032, -1435,  3032, -1435,
   -1435,   992, -1435,  1057,  1002,  1192, -1435,   315,  1173, -1435,
    1004, -1435, -1435,  1011,  1012,   115, -1435, -1435, 17578,  1015,
    1020, -1435,  4884, -1435, -1435, -1435, -1435, -1435, -1435, 14892,
   -1435, 14892, -1435, 17578, 16853, -1435, 15323, 16279, -1435, -1435,
   15323, -1435, 15323, -1435, 12843, 15323, 13478,  1013,  6886,   565,
   -1435, -1435, -1435,   564, 14824, 16839,  1106, -1435,  1496,  1063,
    3352, -1435, -1435, -1435,   725, 14563,    87,    88,  1022,   726,
     654,   116, -1435, -1435, -1435,  1064,  4963, 13874, 17492, -1435,
      70,  1210,  1147, 13478, -1435, 17492, 10594,  1115,  1034,  1431,
    1034,  1032, 17492,  1040, -1435,  1797,  1029,  1865, -1435, -1435,
     109, -1435, -1435,  1102, -1435,  3032, -1435, 16279, -1435, -1435,
    8740, -1435, -1435, -1435, -1435,  9564, -1435, -1435, -1435,  8740,
   -1435,  1041, 15323, 17578,  1107, 17578, 16910, 12843, 13463, -1435,
   -1435, -1435, 16839, 16839, 17235,  1220,    61, -1435, 14824,   726,
   16885, -1435,  1071, -1435,    91,  1043,    93, -1435, 13930, -1435,
   -1435, -1435,    95, -1435, -1435,  3511, -1435,  1045, -1435,  1164,
     529, -1435, 14892, -1435, 14892, -1435, -1435,  1230,   725, -1435,
   14377, -1435, -1435, -1435, -1435,  1233,  1170, 13478, -1435, 17492,
    1050,  1052,  1051,   513, -1435,  1115,  1034, -1435, -1435, -1435,
   -1435,  1923,  1055,  3032, -1435,  1125,  8740,  9770,  9564, -1435,
   -1435, -1435,  8740, -1435, 17578, 15323, 15323, 13478,  7092,  1056,
    1068, -1435, 15323, 16839, -1435, -1435, -1435, -1435, -1435, 14892,
     541,  1496, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435,   167, -1435,  1063, -1435, -1435, -1435, -1435,
   -1435,    90,   635, -1435,  1244,    98, 17235,  1164,  1248, -1435,
   14892,   529, -1435, -1435,  1074,  1250, 13478, -1435, 17492, -1435,
     111, -1435, -1435, -1435, -1435,  1075,   513, 14444, -1435,  1034,
   -1435,  3032, -1435, -1435, -1435, -1435,  7298, 17578, 17578, 11815,
   -1435, -1435, -1435, 17578, -1435,  2114,    83,  1265,  1077, -1435,
   -1435, 15323, 13930, 13930,  1221, -1435,  3511,  3511,   640, -1435,
   -1435, -1435, 15323,  1199, -1435,  1082,   100, 15323, -1435, 17235,
   -1435, 15323, 17492,  1204, -1435,  1275,  7504,  7710, -1435, -1435,
   -1435,   513, -1435,  7916,  1084,  1207,  1177, -1435,  1191,  1144,
   -1435, -1435,  1193, 14892, -1435,   541, -1435, -1435, 17578, -1435,
   -1435,  1131, -1435,  1262, -1435, -1435, -1435, -1435, 17578,  1284,
   -1435, -1435, 17578,  1108, 17578, -1435,   145,  1111, -1435, -1435,
    8122, -1435,  1105, -1435, -1435,  1114,  1146, 17235,   654,  1145,
   -1435, -1435, 15323,   141, -1435,  1236, -1435, -1435, -1435, -1435,
   16839,   843, -1435,  1151, 17235,   362, -1435, 17578,  1119,  1311,
     596,   141, -1435,  1245, -1435, 16839,  1122, -1435,  1034,   147,
   -1435, -1435, -1435, -1435, 14892, -1435,  1126,  1127,   102, -1435,
     523,   596,   393,  1034,  1123, -1435, -1435, -1435, -1435, 14892,
     157,  1315,  1251,   523, -1435,  8328,   394,  1318,  1254, 13478,
   -1435, -1435,  8534, -1435,   302,  1321,  1257, 13478, -1435, 17492,
   -1435,  1324,  1260, 13478, -1435, 17492, 13478, -1435, 17492, 17492
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1435, -1435, -1435,  -498, -1435, -1435, -1435,   254, -1435, -1435,
   -1435,   831,   557,   560,   214,  1514,  3359, -1435,  2586, -1435,
    -329, -1435,    25, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435, -1435,  -357, -1435, -1435,  -175,   436,    22,
   -1435, -1435, -1435, -1435, -1435, -1435,    26, -1435, -1435, -1435,
   -1435,    31, -1435, -1435,   955,   957,   950,   -99,   465,  -799,
     478,   531,  -356,   241,  -814, -1435,   -87, -1435, -1435, -1435,
   -1435,  -661,    92, -1435, -1435, -1435, -1435,  -348, -1435,  -531,
   -1435,  -364, -1435, -1435,   841, -1435,   -71, -1435, -1435,  -963,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,
    -102, -1435,   -20, -1435, -1435, -1435, -1435, -1435,  -184, -1435,
      62,  -913, -1435, -1434,  -370, -1435,  -135,   144,  -133,  -355,
   -1435,  -193, -1435, -1435, -1435,    86,   -37,   -10,   354,  -669,
    -349, -1435, -1435,   -19, -1435, -1435,    -5,   -31,   -57, -1435,
   -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,  -532,  -780,
   -1435, -1435, -1435, -1435, -1435,  2695, -1435, -1435, -1435, -1435,
     978, -1435, -1435,   376, -1435,   892, -1435, -1435, -1435, -1435,
   -1435, -1435, -1435,   384, -1435,   896, -1435, -1435,   620, -1435,
     358, -1435, -1435, -1435, -1435, -1435, -1435, -1435, -1435,  -881,
   -1435,  2303,  1848,  -330, -1435, -1435,   318,  3157,  3569, -1435,
   -1435,   432,  -145,  -580, -1435, -1435,   503,   310,  -665,   311,
   -1435, -1435, -1435, -1435, -1435,   490, -1435, -1435, -1435,    64,
    -804,  -171,  -398,  -392, -1435,   556,  -108, -1435, -1435,   605,
   -1435, -1435,  1090,   -25, -1435, -1435,    68,  -132, -1435,   166,
   -1435, -1435, -1435,  -385,  1110, -1435, -1435, -1435, -1435, -1435,
     610,   607, -1435, -1435,  1118,  -281,  -614, -1435,   -33,   -63,
    -187,    12,   664, -1435,  -928,   119, -1435,   388,   472, -1435,
   -1435, -1435, -1435,   426,  3107, -1001
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -932
static const yytype_int16 yytable[] =
{
     185,   187,   354,   189,   190,   191,   193,   194,   430,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   265,   241,   223,   226,   400,   130,   859,   317,   128,
     132,   909,   320,   321,   246,   133,   271,   244,   574,   250,
     428,  1050,   451,   882,   692,   252,   841,   255,   842,  1220,
     272,   354,   277,   662,  1042,   422,   640,   713,   927,   688,
     689,   399,   823,   714,   455,  1065,   731,   733,   160,   796,
    1463,   350,  -899,   941,   244,  1206,   326,  -899,   796,  1273,
     447,  1076,   328,   344,   942,  1022,   463,   420,   330,   710,
     521,   526,  1653,   531,   463,  -790,  1412,  1414,   234,  1616,
    -292,    13,  1471,   322,  1555,  -793,     3,  1623,   327,  1623,
     425,  1463,  -490,   420,  1117,   284,   812,  1325,  1217,  -492,
     994,   812,   812,  1617,   812,   812,   417,   418,   456,   209,
     717,  1126,  1127,   550,   361,   362,   363,  -791,   549,   417,
     418,   341,  1126,  1127,  1221,  -792,  1633,   188,   135,  1425,
     417,   418,  1639,   364,    13,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   440,   386,   613,   778,
    1695,  1611,   441,   433,  1144,    13,    13,   573,   330,   387,
    1634,   449,    13,  1051,  -634,  -797,  -796,  1612,   288,   341,
     822,   417,   418,  -833,   237,   289,   240,  1681,   327,  -494,
     708,  -460,   425,   386,  1613,  -794,  -836,  -834,  -795,  -835,
     323,   209,   318,   421,  1696,   387,   324,  -904,  -800,  1222,
    -493,  -790,   458,  1129,  1426,   458,  1747,  1052,  -799,   734,
     285,  -793,   244,   469,  1276,  -801,   426,   943,  1023,   421,
    1274,   843,   424,  -231,   427,  1654,   693,   732,   121,   536,
     797,   652,  1464,  1465,  -899,   537,   460,  1339,   354,   798,
     465,   540,   283,  -791,  1345,   345,  1347,  1124,   464,  1128,
    1242,  -792,   522,   527,   611,   532,   548,   652,  1413,  1415,
    1618,   327,  -292,   699,  1472,   313,  1556,   223,   314,  1624,
    1337,  1671,   554,  1735,   401,   253,  1266,   813,   263,  -231,
     652,   609,   895,  1198,   318,  1364,  1419,   247,  1253,  -492,
     652,  1748,  1053,   652,   694,   298,  1035,   596,  -803,  -806,
    1062,  -797,  -796,   317,   656,   657,   350,  -714,   608,  -833,
    -714,  -215,  -804,   315,   339,   298,   116,  -714,   426,   298,
     298,  -794,  -836,  -834,  -795,  -835,   614,   615,   616,   618,
     619,   620,   621,   622,   623,   624,   625,   626,   627,   628,
     629,   630,   631,   632,   633,   634,   635,   636,   637,   638,
     639,  1761,   641,   663,   642,   642,  1441,   645,   298,   583,
     924,   292,  1330,   439,   248,   926,   243,   664,   666,   667,
     668,   669,   670,   671,   672,   673,   674,   675,   676,   677,
    1433,   326,  1435,   304,   849,   642,   687,   441,   608,   608,
     642,   691,  1205,   330,  1096,   835,  1028,   664,  1740,  1754,
     695,   304,   829,   303,  1256,   249,  1331,   556,   294,   704,
     129,   706,   720,   327,   417,   418,   509,  1263,   608,   275,
     437,  1054,   424,   802,   234,   306,   721,  1055,   722,   290,
     400,   651,  -633,   876,   304,   295,  1762,   291,   296,   523,
     556,   784,  1741,  1755,   788,   135,    60,    61,    62,   176,
     177,   351,   307,   308,   884,   766,  1097,   685,   769,   772,
     773,   725,   515,   519,   520,   121,   399,   888,   340,   121,
     307,   308,   874,   470,   510,   789,   297,  1374,  1586,  1011,
     651,  1014,   301,  1073,   340,   876,   302,  1219,  1105,   792,
     709,  1332,   484,   715,  1106,   559,   340,  1207,   340,   849,
    -904,   340,  1229,   307,   308,  1231,   916,   830,   304,   319,
    1208,   304,  1212,   352,   333,   561,   562,   336,   805,   806,
     866,   341,   831,   574,   343,  1081,   644,  1742,  1756,   304,
     599,   332,   340,  1717,   346,   556,   340,   451,   340,  -904,
     355,   353,   304,  1209,  -769,   995,  -772,  1445,   305,  -904,
     356,  1352,  -904,  1353,   357,   121,   686,   389,  1213,   304,
     341,   690,   598,   358,   600,   556,   948,   304,   263,   917,
     856,   298,  -769,   359,  -772,   360,  1123,   307,   308,   162,
     307,   308,   867,   921,   922,    33,    34,    35,  -770,   390,
     431,  -904,  1108,  1109,  1346,    53,   391,   210,   307,   308,
     392,   219,   221,    60,    61,    62,   176,   177,   351,   393,
     306,   307,   308,   583,   875,  -802,  -770,   650,   298,   654,
     298,   298,   298,   298,   423,   193,  -798,   557,   307,   308,
    -491,  1641,  1193,  1194,  1619,   551,   307,   308,   429,  1665,
    1125,  1126,  1127,   680,   887,  -633,    74,    75,    76,    77,
      78,  1620,  1200,   574,  1621,  1329,  1666,   212,   311,  1667,
    1444,  1382,  1383,    82,    83,   434,   702,  1270,  1126,  1127,
     352,  1341,   573,  1663,  1664,  1417,   919,    92,  1235,   712,
    1721,  1722,  1723,   925,  1582,  1583,   436,   652,   329,  1245,
     121,    97,   244,  1732,  1736,  1737,   781,   335,   337,   338,
     785,  1659,  1660,   387,  1265,   283,   442,   762,  1746,   401,
     341,   445,   432,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   951,   954,   936,   424,   996,
     383,   384,   385,   446,   386,  1100,  -632,   129,    60,    61,
      62,   176,   177,   351,   452,  1467,   387,   791,  1590,   453,
     461,   479,   474,   478,   652,   990,   991,   992,  -931,   482,
    1730,   415,   416,   483,   485,    60,    61,    62,    63,    64,
     351,   993,   486,   817,   819,  1743,    70,   394,   528,  1137,
     488,   529,   530,   541,  1321,  1033,  1141,   574,   329,  1442,
     542,  1080,   575,   576,   597,   585,   586,   443,  1041,   588,
     -66,    53,   573,   450,   698,   352,   659,   987,   988,   989,
     990,   991,   992,   610,   396,   130,   162,   696,   128,   132,
     162,   551,   700,  1060,   133,   701,   993,   417,   418,  1298,
     707,   135,   352,  1068,   463,   723,  1069,   298,  1070,  1225,
    1343,   727,   608,   730,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   160,   743,  1710,
     525,   744,    60,    61,    62,    63,    64,   351,   764,   533,
     533,   538,   765,    70,   394,   767,   545,  1710,   768,   583,
    1104,  1111,   552,   555,   783,  1731,   558,   685,   776,   779,
     782,   787,   135,   415,   416,   583,  1642,   795,   587,   799,
    1360,   809,   786,   800,   803,   814,   162,   811,   804,   395,
     552,   396,   558,   552,   558,   558,  1369,  1112,  1249,   820,
     906,   825,   912,   234,   397,   826,   398,   828,   834,   352,
     836,   833,   489,   490,   491,   837,   573,   135,   840,   492,
     493,   121,  1430,   494,   495,   845,   380,   381,   382,   383,
     384,   385,   844,   386,   715,   934,   121,   135,   852,   417,
     418,   854,  1181,  1182,  1183,   387,   857,   515,   769,  1185,
    1288,   519,   858,   860,   863,   869,   574,  1293,   870,   872,
     873,   877,  1201,   881,   891,   889,   892,   893,   918,   865,
     928,   944,   938,   940,   945,   946,  1202,   947,   949,   963,
    1446,   962,   121,   964,   545,   966,   997,   967,  1013,  1452,
     999,   998,  1016,  1017,  1003,  1009,  1006,  1019,  1021,  1046,
    1458,  1027,  1031,   329,  1227,  1677,  1032,   130,  1040,  1038,
     128,   132,  1024,  1044,  1048,  1063,   133,   608,   135,  1072,
     135,   162,  1075,  1078,  1084,  1083,   574,   121,   608,  1202,
    -805,  1099,  1120,  1303,  1094,  1095,  1098,  1122,  1101,  1119,
    1114,  1043,  1116,  1131,  1135,  1136,   993,   121,  1258,   160,
    1139,  1140,  1187,   680,  1196,  1199,  1215,  1223,  1216,  1224,
    1358,   244,  1230,  1228,  1232,  1234,   220,   220,  1236,  1237,
    1596,  1272,  1239,  1240,    13,  1246,  1248,   583,  1252,  1720,
     583,  1247,   902,  1259,  1260,  1262,  1261,  1267,   298,  1271,
    1277,  1279,  1281,  1282,  1285,  1286,  1287,  1289,  1291,  1292,
    1088,  1088,   906,   129,  1323,   573,  1297,   432,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     712,  1333,  1322,  1334,  1338,  1336,   121,  1340,   121,   135,
     121,  1348,   354,  1344,  1349,  1418,  1304,  1350,  1355,  1303,
    1351,  1305,  1354,    60,    61,    62,   176,  1306,   351,  1307,
    1133,  1357,  1356,  1359,  1326,  1361,   415,   416,  1327,   855,
    1328,  1362,  1363,  1388,   129,  1379,  1366,  1143,  1335,  1421,
    1149,  1367,  1401,  1416,  1427,   573,  1428,  1431,  1342,   608,
      13,  1439,  1436,  1443,  1462,  1566,  1308,  1309,  1455,  1310,
    1437,  1453,  1469,  1564,  1570,  1470,  1563,  1576,   762,  1577,
    1579,  1580,  1636,  1581,  1637,  1589,  1591,  1601,  1622,   129,
     352,  1320,  1627,  1643,  1631,  1189,   886,   135,  1190,  1602,
    1320,  1385,   417,   418,  1655,  1630,  1661,  1638,  1669,   129,
    1657,  1670,  1311,  1675,  1676,  1683,  1684,  1685,  -288,   121,
    1688,  1378,  1304,  1691,  1687,  1617,   583,  1305,  1692,    60,
      61,    62,   176,  1306,   351,  1307,  1699,  1694,   913,  1680,
     914,  1697,   220,  1700,  1701,  1712,  1715,  1706,   220,  1718,
    1719,  1461,   162,  1729,  1727,  1744,  1733,  1734,  1429,  1749,
    1750,   608,  1757,  1758,   932,  1763,  1764,   162,  1766,  1767,
    1012,  1315,  1308,  1309,  1714,  1310,   655,  1015,  1079,   906,
    1315,   653,   790,   906,   658,  1466,  1074,  1037,  1264,  1728,
     129,  1595,   129,  1726,  1368,   121,   352,   793,  1587,  1610,
    1468,  1615,  1408,  1751,  1626,   220,  1739,   121,  1585,  1568,
     665,  1569,  1186,   162,   220,   220,   539,  1184,  1324,   774,
    1450,   220,  1389,   775,  1142,  1005,  1629,  1210,   220,  1244,
    1090,  1020,  1102,  1320,  1250,  1056,  1251,   952,  1745,  1320,
    1192,  1320,   547,   535,   583,  1752,   545,  1030,  1381,  1134,
    1180,     0,  1578,     0,     0,     0,     0,     0,   162,     0,
       0,     0,     0,     0,     0,  1303,     0,     0,     0,     0,
       0,     0,   135,     0,     0,     0,  1299,     0,   162,     0,
       0,     0,  1599,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1656,     0,   401,     0,     0,     0,     0,     0,
       0,   129,  1593,  1450,     0,     0,    13,     0,     0,     0,
       0,     0,     0,  1315,     0,     0,     0,     0,     0,  1315,
       0,  1315,     0,     0,     0,     0,     0,     0,     0,     0,
     906,     0,   906,  1625,   135,  1320,     0,     0,     0,     0,
       0,     0,     0,   135,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,  1390,     0,   162,     0,   162,
       0,   162,     0,   932,  1118,     0,     0,     0,  1304,     0,
     214,   214,     0,  1305,   229,    60,    61,    62,   176,  1306,
     351,  1307,   121,  1704,     0,     0,     0,     0,   263,   129,
       0,     0,     0,     0,  1406,     0,  1673,     0,   229,     0,
       0,  1632,   354,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1315,     0,     0,  1308,  1309,
     135,  1310,     0,     0,    48,     0,   135,     0,     0,     0,
       0,     0,   135,     0,     0,     0,     0,     0,     0,   906,
       0,     0,   352,     0,   121,     0,  1391,     0,     0,   121,
       0,     0,     0,   121,     0,     0,     0,     0,     0,  1392,
    1393,     0,     0,     0,  1434,     0,     0,     0,   298,     0,
     162,     0,   263,     0,     0,     0,     0,   180,     0,     0,
      84,  1394,  1552,    86,    87,     0,    88,  1395,    90,  1559,
       0,     0,     0,  1226,     0,     0,   263,     0,   263,     0,
       0,     0,     0,     0,   263,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   220,     0,     0,   906,     0,     0,
     121,   121,   121,     0,     0,     0,   121,     0,  1257,     0,
       0,     0,   121,     0,     0,     0,   162,     0,     0,     0,
       0,     0,     0,     0,   545,   932,     0,     0,   162,     0,
       0,     0,     0,     0,   129,     0,   214,     0,     0,     0,
     135,     0,   214,     0,  1759,     0,     0,     0,     0,     0,
    1410,   220,  1765,     0,     0,     0,     0,   583,  1768,     0,
       0,  1769,     0,     0,     0,     0,     0,     0,     0,     0,
     229,   229,     0,     0,     0,   583,   229,     0,     0,     0,
     135,   135,     0,   583,     0,     0,     0,   135,     0,     0,
       0,     0,     0,   220,     0,   220,   129,     0,     0,   214,
       0,  1303,     0,     0,   545,   129,     0,     0,   214,   214,
       0,     0,     0,     0,     0,   214,     0,     0,     0,   220,
     298,     0,   214,     0,   135,     0,     0,     0,     0,     0,
       0,     0,  1705,   229,     0,     0,     0,     0,     0,     0,
       0,   263,    13,     0,     0,   906,     0,     0,     0,     0,
     121,     0,     0,     0,  1571,   229,     0,     0,   229,  1648,
       0,     0,     0,     0,     0,     0,  1552,  1552,     0,  1303,
    1559,  1559,     0,     0,   217,   217,     0,     0,   231,     0,
       0,     0,   129,   298,     0,     0,   220,     0,   129,   135,
     121,   121,     0,     0,   129,     0,   135,   121,     0,     0,
     229,   220,   220,   162,  1304,     0,     0,     0,     0,  1305,
      13,    60,    61,    62,   176,  1306,   351,  1307,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1303,     0,     0,
       0,     0,     0,     0,   121,     0,     0,     0,     0,     0,
       0,  1703,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1308,  1309,     0,  1310,  1716,     0,
       0,     0,     0,     0,     0,   162,     0,     0,    13,     0,
     162,     0,  1304,     0,   162,     0,     0,  1305,   352,    60,
      61,    62,   176,  1306,   351,  1307,     0,     0,     0,     0,
       0,     0,   229,   229,     0,     0,   754,     0,     0,   121,
    1438,     0,     0,     0,     0,     0,   121,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,   220,
       0,     0,  1308,  1309,     0,  1310,     0,     0,     0,     0,
    1304,     0,   129,     0,     0,  1305,     0,    60,    61,    62,
     176,  1306,   351,  1307,   754,     0,   352,     0,     0,     0,
       0,   162,   162,   162,     0,     0,     0,   162,     0,     0,
       0,     0,     0,   162,     0,     0,     0,   737,  1440,     0,
     217,     0,   129,   129,     0,     0,   217,     0,     0,   129,
    1308,  1309,     0,  1310,     0,     0,     0,     0,     0,   229,
     229,     0,     0,     0,     0,     0,     0,     0,   229,     0,
       0,     0,     0,     0,   352,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,   129,     0,   214,     0,
       0,     0,     0,     0,   738,     0,  1588,     0,     0,     0,
       0,     0,     0,   217,     0,     0,    48,     0,     0,     0,
       0,     0,   217,   217,     0,     0,     0,     0,   220,   217,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   572,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,   129,     0,     0,     0,     0,     0,     0,   129,   180,
       0,    36,    84,   220,     0,    86,    87,     0,    88,   181,
      90,   162,     0,     0,     0,     0,     0,     0,     0,   220,
     220,     0,    48,     0,     0,     0,     0,   214,     0,   214,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   231,     0,     0,     0,     0,     0,
       0,   162,   162,   214,   754,     0,     0,     0,   162,     0,
       0,     0,     0,     0,     0,     0,     0,   229,   229,   754,
     754,   754,   754,   754,     0,   180,     0,   754,    84,     0,
       0,    86,    87,     0,    88,   181,    90,   217,     0,     0,
       0,     0,     0,     0,     0,   162,     0,     0,     0,   220,
       0,     0,     0,   229,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     214,     0,     0,     0,     0,     0,     0,  1647,     0,     0,
       0,     0,     0,   229,     0,   214,   214,     0,     0,   216,
     216,     0,     0,   230,     0,     0,     0,     0,     0,   229,
     229,     0,     0,     0,     0,     0,     0,     0,   229,     0,
     162,     0,     0,     0,   229,     0,     0,   162,     0,     0,
       0,     0,     0,     0,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,   754,     0,     0,   229,     0,   361,
     362,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   229,     0,     0,   364,   229,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,     0,     0,
       0,     0,   214,   214,     0,     0,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,   229,     0,     0,   229,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   754,     0,   229,     0,
       0,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,     0,   217,
     432,   403,   404,   405,   406,   407,   408,   409,   410,   411,
     412,   413,   414,     0,     0,   216,     0,     0,     0,     0,
       0,     0,     0,   754,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
       0,   217,     0,   217,     0,     0,     0,     0,     0,   415,
     416,     0,   229,   993,   229,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,  1203,     0,   217,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   229,
       0,     0,   229,     0,     0,     0,     0,   216,   216,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
       0,   216,   229,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   216,     0,     0,   417,   418,     0,     0,     0,
       0,     0,     0,   214,   214,     0,   754,     0,     0,     0,
     264,     0,     0,     0,   217,   229,     0,     0,     0,   229,
       0,     0,   754,     0,   754,     0,     0,     0,     0,   217,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     754,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   572,     0,     0,     0,     0,     0,     0,   230,
       0,     0,     0,     0,     0,     0,   801,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   229,   229,     0,
     229,     0,     0,   214,     0,   361,   362,   363,     0,     0,
       0,   215,   215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,   231,   364,     0,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,     0,   386,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   209,     0,
     387,     0,     0,     0,     0,     0,   217,   217,     0,     0,
       0,     0,     0,     0,     0,   758,     0,    48,     0,     0,
       0,     0,     0,   229,     0,   229,     0,     0,     0,     0,
     754,   229,     0,     0,   754,     0,   754,     0,     0,   754,
       0,     0,   572,     0,   648,     0,     0,     0,   229,   229,
       0,     0,   229,     0,     0,     0,     0,     0,     0,   229,
       0,     0,     0,   758,     0,     0,     0,     0,     0,     0,
       0,     0,   264,   264,     0,     0,    86,    87,   264,    88,
     181,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   754,     0,     0,     0,
       0,   649,     0,   116,     0,     0,   229,   229,     0,     0,
       0,     0,   229,     0,   229,     0,   217,   216,   821,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,   215,     0,     0,   229,   264,   229,     0,
     264,     0,     0,     0,   229,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   572,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,     0,   217,   217,   754,
     754,     0,     0,     0,     0,     0,   754,   229,     0,     0,
     215,     0,     0,   229,     0,   229,     0,     0,     0,   215,
     215,   969,   970,     0,     0,     0,   215,     0,     0,     0,
       0,     0,     0,   215,     0,     0,   216,     0,   216,   971,
       0,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   216,   758,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   993,     0,   217,   758,   758,
     758,   758,   758,     0,     0,     0,   758,     0,     0,     0,
       0,     0,     0,     0,   264,   739,     0,     0,   756,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   896,   897,
       0,     0,  1008,     0,   229,  -932,  -932,  -932,  -932,   985,
     986,   987,   988,   989,   990,   991,   992,     0,   898,   216,
       0,   229,     0,     0,     0,     0,   899,   900,   901,    36,
     993,     0,  1026,     0,   216,   216,   756,     0,   902,   229,
       0,     0,     0,     0,   215,   754,     0,     0,     0,  1026,
      48,     0,     0,     0,     0,   572,   754,   216,     0,     0,
       0,   754,     0,     0,     0,   754,  -932,  -932,  -932,  -932,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
       0,   264,   264,   758,     0,   903,  1064,   229,     0,     0,
     264,   387,     0,     0,     0,     0,     0,     0,   904,     0,
       0,     0,     0,   218,   218,     0,     0,     0,   230,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   572,   754,   905,     0,     0,
       0,     0,     0,     0,   229,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   229,
       0,   216,   216,     0,     0,     0,     0,     0,   229,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   229,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   758,     0,   216,     0,     0,
     758,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,   758,   758,   758,
     758,   758,   758,   758,   758,   758,   758,     0,     0,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,     0,     0,     0,
       0,     0,   758,     0,     0,     0,     0,     0,     0,   264,
     264,   756,   756,   756,   756,   756,     0,     0,     0,   756,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,   216,     0,   476,   477,     0,     0,     0,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,     0,     0,     0,     0,   218,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,     0,
     215,   216,     0,     0,     0,     0,   216,     0,     0,    36,
       0,   264,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,   216,   215,   758,   264,     0,     0,    36,
      48,     0,     0,     0,     0,     0,     0,     0,   264,     0,
       0,   758,   218,   758,     0,     0,   756,     0,   603,     0,
      48,   218,   218,     0,     0,     0,     0,     0,   218,   758,
       0,     0,     0,     0,     0,   218,     0,   264,     0,     0,
       0,     0,     0,     0,     0,  1404,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   311,     0,     0,    86,
      87,   215,    88,   181,    90,     0,     0,     0,     0,  1302,
       0,     0,   216,     0,     0,     0,   215,   215,     0,    86,
      87,     0,    88,   181,    90,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   264,     0,
       0,   264,     0,   739,   312,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   756,     0,
       0,  1405,     0,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
       0,     0,     0,     0,     0,   736,   218,     0,    36,   758,
     216,     0,     0,   758,     0,   758,     0,     0,   758,     0,
       0,     0,     0,     0,     0,   756,     0,     0,  1387,    48,
       0,  1400,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,   215,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   264,     0,   264,     0,     0,   759,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216,   264,     0,     0,   264,     0,  1557,     0,    86,    87,
    1558,    88,   181,    90,     0,   758,     0,     0,     0,     0,
       0,     0,   838,   839,     0,  1459,  1460,   759,     0,     0,
       0,   847,     0,  1400,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   756,     0,
    1405,     0,     0,     0,     0,     0,     0,   264,     0,     0,
      36,   264,   209,     0,   756,     0,   756,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   756,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,   758,   758,
       0,   218,     0,     0,     0,   758,  1604,     0,     0,     0,
       0,     0,     0,     0,  1400,     0,     0,     0,     0,   264,
     264,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   215,   386,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,   387,     0,     0,   215,   215,     0,     0,   218,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   755,     0,     0,     0,   649,     0,   116,     0,     0,
     603,   603,     0,     0,     0,     0,     0,     0,     0,     0,
     218,     0,   218,     0,     0,   264,     0,   264,     0,     0,
       0,     0,   756,     0,     0,     0,   756,     0,   756,     0,
       0,   756,     0,     0,     0,     0,   218,   759,     0,   755,
     264,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,   264,   759,   759,   759,   759,   759,     0,     0,     0,
     759,     0,     0,     0,   758,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   758,     0,     0,     0,     0,
     758,     0,  1036,     0,   758,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1045,     0,     0,
       0,     0,     0,   218,     0,     0,     0,     0,   756,  1057,
       0,     0,     0,     0,     0,     0,     0,     0,   218,   218,
       0,     0,     0,     0,   264,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1077,     0,
       0,     0,     0,     0,     0,   758,     0,     0,   264,     0,
     264,     0,     0,  1713,     0,     0,   264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   759,  1387,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   361,   362,   363,     0,     0,
       0,   756,   756,     0,     0,     0,     0,     0,   756,  1130,
       0,   760,  1132,     0,   364,   264,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,     0,   386,     0,
       0,     0,     0,     0,     0,   218,   218,     0,     0,   755,
     387,     0,     0,     0,     0,     0,     0,     0,     0,   794,
       0,     0,     0,     0,   755,   755,   755,   755,   755,     0,
       0,     0,   755,     0,     0,     0,     0,     0,     0,   759,
       0,     0,     0,     0,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,     0,     0,     0,     0,  1218,   264,   847,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   264,    36,     0,   759,     0,     0,     0,
       0,     0,  1238,     0,     0,  1241,     0,     0,     0,     0,
       0,  1649,     0,     0,     0,    48,     0,   756,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   756,     0,
       0,     0,     0,   756,     0,   218,     0,   756,     0,   755,
       0,     0,     0,     0,     0,     0,     0,     0,   851,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1278,   264,
       0,     0,  1057,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,    87,     0,    88,   181,    90,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   218,   218,   756,   759,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   759,   610,   759,     0,   933,
    1300,  1301,     0,     0,     0,     0,     0,     0,     0,     0,
     264,     0,     0,   759,   955,   956,   957,   958,     0,     0,
       0,   755,   965,     0,     0,   264,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,     0,     0,     0,   218,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     361,   362,   363,     0,     0,     0,     0,     0,   755,     0,
       0,     0,     0,     0,     0,     0,  1370,     0,  1371,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,     0,     0,     0,     0,     0,     0,
       0,     0,  1411,     0,     0,   387,     0,     0,     0,  1061,
       0,     0,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   759,     0,     0,     0,   759,     0,   759,
       0,   364,   759,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,     0,     0,     0,     0,
       0,   755,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   755,     0,   755,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   755,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   759,
       0,     0,     0,     0,     0,     0,  1150,  1153,  1154,  1155,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,   890,     0,     0,     0,     0,   361,   362,
     363,     0,     0,     0,     0,     0,  1606,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   364,  1191,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,   759,   759,     0,     0,     0,     0,     0,   759,
       0,     0,     0,   387,     0,   894,     0,  1609,     0,     0,
       0,     0,     0,     0,   968,   969,   970,     0,     0,     0,
       0,     0,     0,     0,     0,   755,     0,     0,     0,   755,
       0,   755,     0,   971,   755,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,     0,  1628,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   993,
       0,  1268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1283,     0,  1284,
       0,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1294,     0,     0,     0,     0,
     364,   755,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
    1689,  1018,     0,     0,     0,     0,   387,     0,   759,     0,
     361,   362,   363,     0,     0,     0,     0,     0,     0,   759,
       0,     0,     0,     0,   759,     0,     0,     0,   759,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,  1690,   386,   755,   755,     0,     0,     0,     0,
       0,   755,     0,     0,  1147,   387,     0,     0,     0,     0,
       0,   847,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   847,     0,     0,   759,
     361,   362,   363,     0,     0,  1373,     0,     0,     0,  1375,
       0,  1376,     0,     0,  1377,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,  1071,   387,     0,     0,     0,     0,
       0,     0,     0,   364,  1273,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,     0,     0,
       0,  1454,     0,     0,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1082,     0,     0,     0,     0,     0,     0,
     755,     0,     0,   361,   362,   363,     0,     0,     0,     0,
       0,   755,     0,     0,     0,     0,   755,     0,     0,     0,
     755,     0,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,     0,  1597,  1598,     0,     0,   387,     0,
       0,  1603,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,  1107,     0,    10,     0,     0,     0,     0,
       0,   755,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,  1274,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,  1423,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
    1658,     0,     0,    81,    82,    83,    84,    85,     0,    86,
      87,  1668,    88,    89,    90,    91,  1672,     0,    92,     0,
    1674,    93,     0,     0,     0,     0,     0,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1034,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1707,     0,     0,     0,    11,    12,     0,     0,     0,
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
     115,  1204,   116,   117,     0,   118,   119,     5,     6,     7,
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
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,   589,   116,   117,     0,   118,   119,     5,
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
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1007,
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
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1047,   116,   117,     0,   118,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1113,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1115,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,    91,     0,     0,
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
       0,    45,     0,    46,     0,    47,  1269,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,    91,     0,     0,    92,     0,     0,    93,
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
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1380,
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
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1600,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,  1644,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
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
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1678,   116,   117,
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
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1679,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1682,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,    91,
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
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1698,   116,   117,     0,   118,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1753,   116,   117,     0,   118,   119,     5,     6,     7,
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
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1760,   116,   117,
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
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   459,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   176,   177,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   724,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   176,
     177,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   935,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   176,   177,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,  1449,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   176,   177,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1592,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   176,   177,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,    92,     0,     0,    93,
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
      60,    61,    62,   176,   177,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   660,
      12,     0,     0,     0,     0,     0,     0,   661,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   176,
     177,   178,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,   269,    99,   100,   101,   102,   103,
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
       0,     0,    60,    61,    62,   176,   177,   178,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
     269,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   270,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   364,    10,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   604,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   387,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   176,   177,   178,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,   605,     0,
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
       0,     0,     0,     0,    60,    61,    62,   176,   177,   178,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
     362,   363,   719,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   364,    10,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,  1058,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   387,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   176,   177,   178,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   179,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
    1059,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   660,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   176,
     177,   178,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   361,   362,   363,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   387,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,   192,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   176,   177,   178,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,  1000,  1001,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   363,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   364,    10,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   222,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   387,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   176,   177,   178,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   361,   362,   363,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   364,     0,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,     0,   386,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     387,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   176,   177,   178,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,  1645,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   251,     0,   970,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     971,     0,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   993,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   176,   177,   178,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   179,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   254,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   971,    10,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     993,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   176,
     177,   178,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,   457,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   617,   386,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   176,   177,   178,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   661,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   387,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   176,   177,   178,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   703,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   993,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   176,   177,   178,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   180,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   705,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   387,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   176,   177,   178,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   179,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,  1103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     993,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   176,
     177,   178,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   361,   362,   363,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   387,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   176,   177,   178,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     179,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
    1457,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   361,
     362,   363,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   387,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   553,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   176,   177,   178,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   179,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      92,     0,     0,    93,     0,  1296,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,   361,   362,   363,     0,   116,   117,
       0,   118,   119,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,     0,     0,
       0,     0,     0,  1473,  1474,  1475,  1476,  1477,     0,   387,
    1478,  1479,  1480,  1481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1482,  1483,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
       0,     0,     0,  1484,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,  1485,  1486,  1487,
    1488,  1489,  1490,  1491,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1492,  1493,  1494,
    1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,    48,  1503,
    1504,  1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,
    1514,  1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,
    1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,     0,
       0,     0,  1533,  1534,     0,  1535,  1536,  1537,  1538,  1539,
       0,     0,     0,     0,     0,     0,     0,  1424,     0,     0,
       0,  1540,  1541,  1542,     0,     0,     0,    86,    87,     0,
      88,   181,    90,  1543,     0,  1544,  1545,     0,  1546,     0,
       0,     0,     0,     0,     0,  1547,  1548,     0,  1549,     0,
    1550,  1551,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,     0,     0,   364,     0,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,   388,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,   473,     0,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,   475,   256,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   257,   386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,   487,
       0,     0,     0,     0,    36,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   256,   386,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,     0,     0,
       0,     0,     0,   511,     0,     0,     0,     0,     0,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     258,   259,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,   180,     0,
       0,    84,   260,     0,    86,    87,   716,    88,   181,    90,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   261,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,  1004,   262,   258,   259,     0,
    1572,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   256,     0,     0,   180,     0,     0,    84,   260,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   257,   261,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,     0,     0,   262,     0,     0,     0,  1640,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   256,
       0,    48,     0,     0,     0,     0,     0,     0,     0,  -335,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     176,   177,   351,     0,     0,   257,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   258,   259,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,   180,     0,     0,    84,   260,     0,
      86,    87,     0,    88,   181,    90,     0,     0,    48,     0,
       0,     0,     0,     0,   256,     0,   480,     0,   261,     0,
       0,     0,     0,     0,   352,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     257,     0,   262,   258,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     256,   180,    36,     0,    84,   260,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,   261,   257,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,    36,   262,
       0,     0,     0,     0,     0,     0,     0,     0,   258,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,   256,     0,     0,   180,     0,     0,    84,
     260,     0,    86,    87,     0,    88,   181,    90,     0,   950,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   257,
     261,     0,     0,     0,   258,   259,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    36,   180,     0,   262,    84,   260,     0,    86,    87,
       0,    88,   181,    90,     0,  1280,     0,     0,     0,     0,
       0,   256,    48,     0,     0,     0,   261,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   257,     0,     0,
     262,     0,     0,     0,     0,     0,     0,   258,   259,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,   180,     0,     0,    84,   260,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,   261,
    1384,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   262,     0,   258,   259,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,   180,   209,     0,    84,   260,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,   261,     0,     0,
       0,     0,  1156,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   745,   746,
       0,   262,     0,     0,   747,     0,   748,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   749,     0,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
     678,     0,    86,    87,     0,    88,   181,    90,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,     0,     0,     0,   679,     0,   116,
       0,     0,     0,     0,     0,   750,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,   180,    82,    83,    84,   751,     0,    86,
      87,     0,    88,   181,    90,     0,     0,   929,    92,     0,
       0,     0,     0,     0,     0,     0,     0,   752,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    28,     0,
       0,   753,     0,     0,     0,     0,    33,    34,    35,    36,
       0,   209,     0,     0,     0,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   993,   930,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   212,     0,
       0,    36,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   745,
     746,   213,     0,     0,     0,   747,   116,   748,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   749,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    86,    87,     0,    88,   181,    90,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   865,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   750,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,   180,    82,    83,    84,   751,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,   752,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    28,
      29,    30,   753,     0,     0,     0,     0,    33,    34,    35,
      36,     0,   209,     0,     0,     0,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   993,     0,     0,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,   180,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      28,     0,   448,     0,     0,     0,     0,   116,    33,    34,
      35,    36,     0,   209,     0,     0,     0,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   993,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,   180,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    28,     0,   213,     0,     0,   524,     0,   116,    33,
      34,    35,    36,     0,   209,     0,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,  -932,  -932,  -932,  -932,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,   544,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,   180,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   181,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,    28,   213,   885,     0,     0,     0,   116,
       0,    33,    34,    35,    36,     0,   209,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,  -932,  -932,  -932,  -932,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,     0,     0,     0,     0,   211,     0,     0,     0,
       0,     0,     0,     0,     0,   993,     0,     0,     0,     0,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   180,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   181,    90,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,    28,     0,   213,     0,     0,     0,
       0,   116,    33,    34,    35,    36,     0,   209,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1029,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,   180,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,    28,     0,   213,     0,     0,
       0,     0,   116,    33,    34,    35,    36,     0,   209,     0,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   209,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   212,     0,     0,    48,     0,
     180,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     181,    90,     0,     0,     0,    92,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   213,     0,
       0,   180,     0,   116,    84,    85,     0,    86,    87,     0,
      88,   181,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1085,  1086,  1087,    36,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,    48,   209,   228,
       0,     0,     0,     0,   116,   567,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,   211,   209,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,    87,    48,    88,
     181,    90,     0,     0,     0,     0,     0,     0,     0,     0,
     180,     0,     0,    84,    85,     0,    86,    87,     0,    88,
     181,    90,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   678,     0,    86,    87,     0,
      88,   181,    90,   568,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   361,   362,   363,     0,
       0,     0,   711,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   364,   435,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
     364,   444,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   364,   871,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,   968,   969,   970,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,     0,     0,     0,
       0,   971,   915,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   993,     0,   968,
     969,   970,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   971,  1233,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,     0,     0,   968,   969,   970,     0,     0,     0,     0,
       0,     0,     0,     0,   993,     0,     0,     0,     0,     0,
       0,     0,   971,  1138,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   993,     0,
     968,   969,   970,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,   971,
    1290,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   993,     0,    36,     0,     0,
       0,     0,     0,    48,  1372,     0,     0,     0,     0,     0,
     180,     0,     0,    84,    85,     0,    86,    87,    48,    88,
     181,    90,     0,     0,     0,  1391,   278,   279,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1392,  1393,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,   180,   815,   816,    84,
      85,  1456,    86,    87,     0,    88,  1395,    90,     0,     0,
       0,     0,     0,     0,   280,     0,    48,    86,    87,     0,
      88,   181,    90,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    36,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,    87,    48,    88,   181,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    36,     0,     0,     0,     0,   348,
      48,    86,    87,     0,    88,   181,    90,     0,     0,     0,
       0,     0,     0,   512,     0,    48,    86,    87,     0,    88,
     181,    90,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   516,     0,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,   280,     0,     0,    86,    87,     0,    88,   181,    90,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   648,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,  1148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,    86,    87,
       0,    88,   181,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    86,    87,     0,    88,   181,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   361,   362,   363,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   728,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,     0,     0,     0,   364,   868,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     729,   386,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,     0,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   968,   969,   970,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,     0,     0,   971,  1295,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,   968,   969,
     970,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   993,     0,     0,     0,     0,     0,   971,     0,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   993
};

static const yytype_int16 yycheck[] =
{
       5,     6,   135,     8,     9,    10,    11,    12,   183,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,    32,    28,    29,   160,     4,   607,    91,     4,
       4,   696,    95,    96,    44,     4,    55,    42,   319,    49,
     172,   845,   229,   657,   429,    50,   578,    52,   579,  1050,
      55,   184,    57,   402,   834,   163,   386,   455,   727,   423,
     424,   160,   560,   455,   235,   864,     9,    30,     4,     9,
       9,   134,     9,   734,    79,  1038,   113,    14,     9,    30,
     225,   880,   113,     9,     9,     9,     9,    66,   113,   453,
       9,     9,     9,     9,     9,    66,     9,     9,    30,     9,
       9,    45,     9,    79,     9,    66,     0,     9,   113,     9,
      66,     9,    66,    66,   928,    79,     9,    50,  1046,    66,
     150,     9,     9,    33,     9,     9,   129,   130,   236,    79,
     459,   101,   102,   304,    10,    11,    12,    66,    66,   129,
     130,   171,   101,   102,    79,    66,    35,   199,     4,    79,
     129,   130,  1586,    29,    45,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   213,    53,   353,   508,
      35,    14,   213,   188,   964,    45,    45,   319,   213,    65,
      79,   228,    45,    35,   150,    66,    66,    30,   118,   171,
     203,   129,   130,    66,   199,   125,   199,  1641,   213,    66,
     200,     8,    66,    53,    47,    66,    66,    66,    66,    66,
     196,    79,   155,   202,    79,    65,   202,   199,   199,   164,
      66,   202,   237,   203,   164,   240,    79,    79,   199,   202,
     204,   202,   247,   248,   203,   199,   202,   172,   172,   202,
     201,   580,   199,   197,   204,   172,   431,   200,     4,   296,
     200,   393,   201,   202,   201,   296,   241,  1230,   401,   200,
     245,   296,    58,   202,  1237,   201,  1239,   938,   201,   940,
    1079,   202,   201,   201,   347,   201,   201,   419,   201,   201,
     200,   296,   201,   438,   201,    81,   201,   302,    84,   201,
    1228,   201,   307,   201,   160,    51,  1120,   200,    54,   200,
     442,   344,   200,   200,   155,   200,   200,   199,  1098,    66,
     452,   164,   164,   455,   200,    71,   824,   332,   199,   199,
     861,   202,   202,   396,   397,   398,   399,   197,   343,   202,
     200,   200,   199,    89,    30,    91,   204,   200,   202,    95,
      96,   202,   202,   202,   202,   202,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,    79,   387,   402,   389,   390,  1349,   392,   134,   325,
     719,    79,    35,    85,   199,   724,    42,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
    1338,   448,  1340,    79,   585,   420,   421,   448,   423,   424,
     425,   426,  1036,   448,    85,   570,   811,   432,    35,    35,
     435,    79,    50,    79,  1099,   199,    79,    85,   199,   444,
       4,   446,   461,   448,   129,   130,   110,  1116,   453,   202,
     196,   849,   199,   200,   386,   147,   461,   849,   463,   117,
     595,   393,   150,   650,    79,   199,   164,   125,   199,    97,
      85,    97,    79,    79,    97,   331,   114,   115,   116,   117,
     118,   119,   148,   149,   659,   490,   147,   419,   493,   494,
     495,   466,   278,   279,   280,   241,   595,   684,   154,   245,
     148,   149,   647,   249,   168,    97,   199,  1287,  1436,    97,
     442,    97,   199,   877,   154,   702,   199,  1049,   916,   524,
     452,   164,   268,   455,   916,   311,   154,   156,   154,   700,
     150,   154,  1063,   148,   149,  1066,   707,   155,    79,   199,
     169,    79,   156,   181,    85,   201,   202,    85,    46,    47,
     613,   171,   170,   834,   199,   885,   390,   164,   164,    79,
     208,   202,   154,   201,    35,    85,   154,   754,   154,   150,
     201,   135,    79,   202,   171,   762,   171,  1357,    85,   199,
     201,  1246,   202,  1248,   201,   331,   420,    66,   202,    79,
     171,   425,   207,   201,   340,    85,   741,    79,   344,   707,
     605,   347,   199,   201,   199,   201,   935,   148,   149,     4,
     148,   149,   617,    71,    72,    74,    75,    76,   171,    66,
     184,   202,    71,    72,  1238,   106,   201,    86,   148,   149,
     202,    26,    27,   114,   115,   116,   117,   118,   119,   150,
     147,   148,   149,   579,   649,   199,   199,   393,   394,   395,
     396,   397,   398,   399,   199,   660,   199,   147,   148,   149,
      66,  1589,    97,    98,    29,   147,   148,   149,   199,    29,
     100,   101,   102,   419,   679,   150,   135,   136,   137,   138,
     139,    46,  1031,   964,    49,  1217,    46,   146,   154,    49,
    1355,   127,   128,   152,   153,   201,   442,   100,   101,   102,
     181,  1232,   834,  1616,  1617,  1319,   711,   166,  1072,   455,
     114,   115,   116,   723,   201,   202,    44,   849,   113,  1083,
     466,   180,   727,  1724,   201,   202,   512,   117,   118,   119,
     516,  1612,  1613,    65,  1119,   521,   150,   483,  1739,   595,
     171,   206,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   743,   744,   732,   199,   764,
      49,    50,    51,     9,    53,   910,   150,   331,   114,   115,
     116,   117,   118,   119,   150,  1389,    65,   523,  1443,   199,
       8,   199,   201,   171,   916,    49,    50,    51,   150,    14,
    1718,    63,    64,   150,    79,   114,   115,   116,   117,   118,
     119,    65,   201,   549,   550,  1733,   125,   126,   125,   954,
     201,   125,    14,   200,  1199,   820,   961,  1098,   213,  1350,
     171,   884,    14,    97,   205,   200,   200,   222,   833,   200,
     199,   106,   964,   228,     9,   181,   400,    46,    47,    48,
      49,    50,    51,   199,   163,   823,   241,   199,   823,   823,
     245,   147,   200,   858,   823,   200,    65,   129,   130,  1188,
     200,   717,   181,   868,     9,    89,   871,   613,   873,  1056,
    1234,   201,   877,    14,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   823,   199,  1693,
     285,     9,   114,   115,   116,   117,   118,   119,   185,   294,
     295,   296,    79,   125,   126,    79,   301,  1711,    79,   845,
     915,   921,   305,   308,     9,  1719,   309,   849,   188,   199,
     201,     9,   778,    63,    64,   861,  1591,    79,   200,   200,
    1259,   127,   201,   200,   200,   200,   331,   199,   201,   161,
     333,   163,   335,   336,   337,   338,  1275,   922,  1093,    66,
     696,    30,   698,   885,   176,   128,   178,   170,     9,   181,
     200,   131,   182,   183,   184,   150,  1098,   823,    14,   189,
     190,   717,  1336,   193,   194,     9,    46,    47,    48,    49,
      50,    51,   197,    53,   916,   731,   732,   843,     9,   129,
     130,   172,   997,   998,   999,    65,   200,   783,  1003,  1004,
    1145,   787,     9,    14,   127,   206,  1287,  1152,   206,   203,
       9,   199,  1031,    14,   200,   206,   200,   206,   200,   199,
      97,    86,   201,   201,   131,   150,  1031,     9,   200,   150,
    1359,   199,   778,   199,   429,   150,   185,   202,   784,  1368,
      14,   185,   788,   789,     9,   202,    79,    14,   201,   197,
    1379,   202,    14,   448,  1059,  1635,   206,  1035,    14,   202,
    1035,  1035,   808,   201,    30,   199,  1035,  1072,   924,   199,
     926,   466,    30,    14,    14,   199,  1357,   823,  1083,  1084,
     199,     9,   131,     4,    48,   199,   199,    14,   200,   199,
     201,   837,   201,   131,     9,   200,    65,   843,  1108,  1035,
     206,     9,    79,   849,     9,   199,   131,    14,   201,    79,
    1255,  1116,   202,   200,   199,   199,    26,    27,   200,   202,
    1449,  1126,   202,   201,    45,   131,     9,  1063,   147,  1709,
    1066,   206,    86,    30,    73,   201,  1111,   200,   884,   201,
     172,   131,    30,   200,   200,   131,     9,   200,   203,     9,
     896,   897,   898,   717,   202,  1287,   200,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     916,    14,   203,    79,   200,   199,   922,   200,   924,  1035,
     926,   201,  1315,   200,   202,  1320,   107,   199,   131,     4,
     200,   112,   200,   114,   115,   116,   117,   118,   119,   120,
     946,     9,   200,    30,  1209,   201,    63,    64,  1213,   604,
    1215,   200,   200,   107,   778,   202,   201,   963,  1223,   155,
     966,   201,   159,   201,    14,  1357,    79,   112,  1233,  1234,
      45,   202,   200,   131,    14,  1410,   157,   158,   131,   160,
     200,   200,   171,    79,    14,   202,   201,    14,   994,    79,
     200,   199,  1581,   202,  1583,   200,   131,   201,    14,   823,
     181,  1197,    14,  1592,    14,  1011,   661,  1123,  1014,   201,
    1206,  1304,   129,   130,     9,   201,    55,   202,    79,   843,
     203,   199,   203,    79,     9,   201,    79,   110,    97,  1035,
      97,  1296,   107,   162,   150,    33,  1232,   112,    14,   114,
     115,   116,   117,   118,   119,   120,   201,   199,   703,  1638,
     705,   200,   222,   199,   168,    79,   165,   172,   228,   200,
       9,  1384,   717,   201,    79,   202,   200,   200,  1333,    14,
      79,  1336,    14,    79,   729,    14,    79,   732,    14,    79,
     783,  1197,   157,   158,  1701,   160,   396,   787,   883,  1095,
    1206,   394,   521,  1099,   399,  1388,   878,   826,  1117,  1715,
     924,  1448,   926,  1711,  1272,  1111,   181,   526,  1439,  1471,
    1390,  1555,  1310,  1743,  1567,   285,  1731,  1123,  1435,  1412,
     402,  1414,  1006,   778,   294,   295,   296,  1003,   203,   497,
    1365,   301,  1306,   497,   962,   775,  1571,  1039,   308,  1081,
     897,   796,   912,  1339,  1094,   849,  1095,   743,  1737,  1345,
    1022,  1347,   302,   295,  1350,  1744,   811,   812,  1299,   947,
     994,    -1,  1427,    -1,    -1,    -1,    -1,    -1,   823,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,  1298,    -1,    -1,    -1,  1192,    -1,   843,    -1,
      -1,    -1,  1457,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1607,    -1,  1320,    -1,    -1,    -1,    -1,    -1,
      -1,  1035,  1447,  1448,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,  1339,    -1,    -1,    -1,    -1,    -1,  1345,
      -1,  1347,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1246,    -1,  1248,  1566,  1360,  1441,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1369,    -1,    -1,    -1,    -1,    -1,   429,
      -1,    -1,    -1,    -1,    -1,    29,    -1,   922,    -1,   924,
      -1,   926,    -1,   928,   929,    -1,    -1,    -1,   107,    -1,
      26,    27,    -1,   112,    30,   114,   115,   116,   117,   118,
     119,   120,  1298,  1688,    -1,    -1,    -1,    -1,  1304,  1123,
      -1,    -1,    -1,    -1,  1310,    -1,  1629,    -1,    54,    -1,
      -1,  1576,  1705,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1441,    -1,    -1,   157,   158,
    1446,   160,    -1,    -1,    98,    -1,  1452,    -1,    -1,    -1,
      -1,    -1,  1458,    -1,    -1,    -1,    -1,    -1,    -1,  1355,
      -1,    -1,   181,    -1,  1360,    -1,   120,    -1,    -1,  1365,
      -1,    -1,    -1,  1369,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,    -1,    -1,   203,    -1,    -1,    -1,  1384,    -1,
    1035,    -1,  1388,    -1,    -1,    -1,    -1,   151,    -1,    -1,
     154,   155,  1398,   157,   158,    -1,   160,   161,   162,  1405,
      -1,    -1,    -1,  1058,    -1,    -1,  1412,    -1,  1414,    -1,
      -1,    -1,    -1,    -1,  1420,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   604,    -1,    -1,  1443,    -1,    -1,
    1446,  1447,  1448,    -1,    -1,    -1,  1452,    -1,  1103,    -1,
      -1,    -1,  1458,    -1,    -1,    -1,  1111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1119,  1120,    -1,    -1,  1123,    -1,
      -1,    -1,    -1,    -1,  1298,    -1,   222,    -1,    -1,    -1,
    1596,    -1,   228,    -1,  1749,    -1,    -1,    -1,    -1,    -1,
    1314,   661,  1757,    -1,    -1,    -1,    -1,  1693,  1763,    -1,
      -1,  1766,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     256,   257,    -1,    -1,    -1,  1711,   262,    -1,    -1,    -1,
    1636,  1637,    -1,  1719,    -1,    -1,    -1,  1643,    -1,    -1,
      -1,    -1,    -1,   703,    -1,   705,  1360,    -1,    -1,   285,
      -1,     4,    -1,    -1,  1199,  1369,    -1,    -1,   294,   295,
      -1,    -1,    -1,    -1,    -1,   301,    -1,    -1,    -1,   729,
    1566,    -1,   308,    -1,  1680,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1688,   319,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1587,    45,    -1,    -1,  1591,    -1,    -1,    -1,    -1,
    1596,    -1,    -1,    -1,  1418,   341,    -1,    -1,   344,  1605,
      -1,    -1,    -1,    -1,    -1,    -1,  1612,  1613,    -1,     4,
    1616,  1617,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,  1446,  1629,    -1,    -1,   796,    -1,  1452,  1745,
    1636,  1637,    -1,    -1,  1458,    -1,  1752,  1643,    -1,    -1,
     386,   811,   812,  1298,   107,    -1,    -1,    -1,    -1,   112,
      45,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    -1,  1680,    -1,    -1,    -1,    -1,    -1,
      -1,  1687,    -1,   429,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,    -1,   160,  1704,    -1,
      -1,    -1,    -1,    -1,    -1,  1360,    -1,    -1,    45,    -1,
    1365,    -1,   107,    -1,  1369,    -1,    -1,   112,   181,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,   478,   479,    -1,    -1,   482,    -1,    -1,  1745,
     203,    -1,    -1,    -1,    -1,    -1,  1752,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   928,   929,
      -1,    -1,   157,   158,    -1,   160,    -1,    -1,    -1,    -1,
     107,    -1,  1596,    -1,    -1,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,   530,    -1,   181,    -1,    -1,    -1,
      -1,  1446,  1447,  1448,    -1,    -1,    -1,  1452,    -1,    -1,
      -1,    -1,    -1,  1458,    -1,    -1,    -1,    29,   203,    -1,
     222,    -1,  1636,  1637,    -1,    -1,   228,    -1,    -1,  1643,
     157,   158,    -1,   160,    -1,    -1,    -1,    -1,    -1,   575,
     576,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   584,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,  1680,    -1,   604,    -1,
      -1,    -1,    -1,    -1,    86,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,   285,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,   294,   295,    -1,    -1,    -1,    -1,  1058,   301,
      -1,    -1,    -1,    -1,    -1,    -1,   308,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   319,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   661,    -1,    -1,    -1,    -1,
      -1,  1745,    -1,    -1,    -1,    -1,    -1,    -1,  1752,   151,
      -1,    77,   154,  1103,    -1,   157,   158,    -1,   160,   161,
     162,  1596,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1119,
    1120,    -1,    98,    -1,    -1,    -1,    -1,   703,    -1,   705,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   386,    -1,    -1,    -1,    -1,    -1,
      -1,  1636,  1637,   729,   730,    -1,    -1,    -1,  1643,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   743,   744,   745,
     746,   747,   748,   749,    -1,   151,    -1,   753,   154,    -1,
      -1,   157,   158,    -1,   160,   161,   162,   429,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1680,    -1,    -1,    -1,  1199,
      -1,    -1,    -1,   779,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     796,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    -1,    -1,   809,    -1,   811,   812,    -1,    -1,    26,
      27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,   825,
     826,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   834,    -1,
    1745,    -1,    -1,    -1,   840,    -1,    -1,  1752,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   852,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   860,    -1,    -1,   863,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   881,    -1,    -1,    29,   885,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   928,   929,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   604,    -1,    -1,    -1,   942,    -1,    -1,   945,
      -1,   947,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   962,    -1,   964,    -1,
      -1,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,    -1,   661,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1019,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,   703,    -1,   705,    -1,    -1,    -1,    -1,    -1,    63,
      64,    -1,  1048,    65,  1050,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1058,    -1,    -1,   206,    -1,   729,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   285,  1075,
      -1,    -1,  1078,    -1,    -1,    -1,    -1,   294,   295,    -1,
      -1,    -1,    -1,    -1,   301,    -1,    -1,    -1,    -1,    -1,
      -1,   308,  1098,    -1,    -1,    -1,    -1,  1103,    -1,    -1,
      -1,    -1,   319,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,  1119,  1120,    -1,  1122,    -1,    -1,    -1,
      54,    -1,    -1,    -1,   796,  1131,    -1,    -1,    -1,  1135,
      -1,    -1,  1138,    -1,  1140,    -1,    -1,    -1,    -1,   811,
     812,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   834,    -1,    -1,    -1,    -1,    -1,    -1,   386,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1193,  1194,    -1,
    1196,    -1,    -1,  1199,    -1,    10,    11,    12,    -1,    -1,
      -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   429,   885,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,
      65,    -1,    -1,    -1,    -1,    -1,   928,   929,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   482,    -1,    98,    -1,    -1,
      -1,    -1,    -1,  1279,    -1,  1281,    -1,    -1,    -1,    -1,
    1286,  1287,    -1,    -1,  1290,    -1,  1292,    -1,    -1,  1295,
      -1,    -1,   964,    -1,   125,    -1,    -1,    -1,  1304,  1305,
      -1,    -1,  1308,    -1,    -1,    -1,    -1,    -1,    -1,  1315,
      -1,    -1,    -1,   530,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   256,   257,    -1,    -1,   157,   158,   262,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1357,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,  1372,    -1,    -1,    -1,
      -1,   202,    -1,   204,    -1,    -1,  1382,  1383,    -1,    -1,
      -1,    -1,  1388,    -1,  1390,    -1,  1058,   604,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   222,    -1,    -1,
      -1,    -1,    -1,   228,    -1,    -1,  1412,   341,  1414,    -1,
     344,    -1,    -1,    -1,  1420,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1098,    -1,    -1,    -1,
      -1,  1103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   661,    -1,    -1,  1119,  1120,  1455,
    1456,    -1,    -1,    -1,    -1,    -1,  1462,  1463,    -1,    -1,
     285,    -1,    -1,  1469,    -1,  1471,    -1,    -1,    -1,   294,
     295,    11,    12,    -1,    -1,    -1,   301,    -1,    -1,    -1,
      -1,    -1,    -1,   308,    -1,    -1,   703,    -1,   705,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   729,   730,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,  1199,   745,   746,
     747,   748,   749,    -1,    -1,    -1,   753,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   478,   479,    -1,    -1,   482,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,   779,    -1,  1570,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    66,   796,
      -1,  1587,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      65,    -1,   809,    -1,   811,   812,   530,    -1,    86,  1605,
      -1,    -1,    -1,    -1,   429,  1611,    -1,    -1,    -1,   826,
      98,    -1,    -1,    -1,    -1,  1287,  1622,   834,    -1,    -1,
      -1,  1627,    -1,    -1,    -1,  1631,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,   575,   576,   860,    -1,   133,   863,  1653,    -1,    -1,
     584,    65,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,    26,    27,    -1,    -1,    -1,   885,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1357,  1692,   175,    -1,    -1,
      -1,    -1,    -1,    -1,  1700,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,  1715,
      -1,   928,   929,    -1,    -1,    -1,    -1,    -1,  1724,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1739,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   962,    -1,   964,    -1,    -1,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,    -1,    -1,   604,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   730,    -1,    -1,    -1,
      -1,    -1,  1019,    -1,    -1,    -1,    -1,    -1,    -1,   743,
     744,   745,   746,   747,   748,   749,    -1,    -1,    -1,   753,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   661,    -1,    -1,    -1,
      -1,  1058,    -1,   256,   257,    -1,    -1,    -1,    -1,   262,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   222,
      -1,    -1,    -1,    -1,    -1,   228,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   703,    -1,
     705,  1098,    -1,    -1,    -1,    -1,  1103,    -1,    -1,    77,
      -1,   825,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1119,  1120,   729,  1122,   840,    -1,    -1,    77,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   852,    -1,
      -1,  1138,   285,  1140,    -1,    -1,   860,    -1,   341,    -1,
      98,   294,   295,    -1,    -1,    -1,    -1,    -1,   301,  1156,
      -1,    -1,    -1,    -1,    -1,   308,    -1,   881,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,   157,
     158,   796,   160,   161,   162,    -1,    -1,    -1,    -1,  1196,
      -1,    -1,  1199,    -1,    -1,    -1,   811,   812,    -1,   157,
     158,    -1,   160,   161,   162,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   942,    -1,
      -1,   945,    -1,   947,   202,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   962,    -1,
      -1,   199,    -1,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
      -1,    -1,    -1,    -1,    -1,   478,   429,    -1,    77,  1286,
    1287,    -1,    -1,  1290,    -1,  1292,    -1,    -1,  1295,    -1,
      -1,    -1,    -1,    -1,    -1,  1019,    -1,    -1,  1305,    98,
      -1,  1308,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   928,   929,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1048,    -1,  1050,    -1,    -1,   482,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1357,  1075,    -1,    -1,  1078,    -1,   155,    -1,   157,   158,
     159,   160,   161,   162,    -1,  1372,    -1,    -1,    -1,    -1,
      -1,    -1,   575,   576,    -1,  1382,  1383,   530,    -1,    -1,
      -1,   584,    -1,  1390,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,  1122,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,  1131,    -1,    -1,
      77,  1135,    79,    -1,  1138,    -1,  1140,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,  1156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1058,    -1,    -1,    -1,    -1,  1455,  1456,
      -1,   604,    -1,    -1,    -1,  1462,  1463,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1471,    -1,    -1,    -1,    -1,  1193,
    1194,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1103,    53,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,  1119,  1120,    -1,    -1,   661,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   482,    -1,    -1,    -1,   202,    -1,   204,    -1,    -1,
     743,   744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     703,    -1,   705,    -1,    -1,  1279,    -1,  1281,    -1,    -1,
      -1,    -1,  1286,    -1,    -1,    -1,  1290,    -1,  1292,    -1,
      -1,  1295,    -1,    -1,    -1,    -1,   729,   730,    -1,   530,
    1304,    -1,    -1,    -1,  1199,    -1,    -1,    -1,    -1,    -1,
      -1,  1315,   745,   746,   747,   748,   749,    -1,    -1,    -1,
     753,    -1,    -1,    -1,  1611,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1622,    -1,    -1,    -1,    -1,
    1627,    -1,   825,    -1,  1631,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   840,    -1,    -1,
      -1,    -1,    -1,   796,    -1,    -1,    -1,    -1,  1372,   852,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   811,   812,
      -1,    -1,    -1,    -1,  1388,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   881,    -1,
      -1,    -1,    -1,    -1,    -1,  1692,    -1,    -1,  1412,    -1,
    1414,    -1,    -1,  1700,    -1,    -1,  1420,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   860,  1715,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,  1455,  1456,    -1,    -1,    -1,    -1,    -1,  1462,   942,
      -1,   482,   945,    -1,    29,  1469,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,   928,   929,    -1,    -1,   730,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   530,
      -1,    -1,    -1,    -1,   745,   746,   747,   748,   749,    -1,
      -1,    -1,   753,    -1,    -1,    -1,    -1,    -1,    -1,   962,
      -1,    -1,    -1,    -1,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,    -1,    -1,    -1,    -1,  1048,  1570,  1050,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1587,    77,    -1,  1019,    -1,    -1,    -1,
      -1,    -1,  1075,    -1,    -1,  1078,    -1,    -1,    -1,    -1,
      -1,  1605,    -1,    -1,    -1,    98,    -1,  1611,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1622,    -1,
      -1,    -1,    -1,  1627,    -1,  1058,    -1,  1631,    -1,   860,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1131,  1653,
      -1,    -1,  1135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,    -1,   160,   161,   162,
    1103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1119,  1120,  1692,  1122,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,  1138,   199,  1140,    -1,   730,
    1193,  1194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1724,    -1,    -1,  1156,   745,   746,   747,   748,    -1,    -1,
      -1,   962,   753,    -1,    -1,  1739,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,    -1,    -1,    -1,  1199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1019,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1279,    -1,  1281,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1315,    -1,    -1,    65,    -1,    -1,    -1,   860,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1286,    -1,    -1,    -1,  1290,    -1,  1292,
      -1,    29,  1295,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,  1122,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1138,    -1,  1140,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1372,
      -1,    -1,    -1,    -1,    -1,    -1,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   203,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,  1469,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1019,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,  1455,  1456,    -1,    -1,    -1,    -1,    -1,  1462,
      -1,    -1,    -1,    65,    -1,   203,    -1,  1470,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1286,    -1,    -1,    -1,  1290,
      -1,  1292,    -1,    29,  1295,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,  1570,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,  1122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1138,    -1,  1140,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,    -1,    -1,
      29,  1372,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
    1653,   203,    -1,    -1,    -1,    -1,    65,    -1,  1611,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1622,
      -1,    -1,    -1,    -1,  1627,    -1,    -1,    -1,  1631,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1655,    53,  1455,  1456,    -1,    -1,    -1,    -1,
      -1,  1462,    -1,    -1,   200,    65,    -1,    -1,    -1,    -1,
      -1,  1724,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1739,    -1,    -1,  1692,
      10,    11,    12,    -1,    -1,  1286,    -1,    -1,    -1,  1290,
      -1,  1292,    -1,    -1,  1295,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,  1372,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
    1611,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,  1622,    -1,    -1,    -1,    -1,  1627,    -1,    -1,    -1,
    1631,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1455,  1456,    -1,    -1,    65,    -1,
      -1,  1462,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,   203,    -1,    13,    -1,    -1,    -1,    -1,
      -1,  1692,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,   201,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,   203,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
    1611,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,  1622,   160,   161,   162,   163,  1627,    -1,   166,    -1,
    1631,   169,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,
     178,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1692,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    12,
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
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     194,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
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
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,
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
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,
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
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
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
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,
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
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
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
      -1,    65,    -1,    -1,    -1,   201,    29,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    55,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    77,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    29,    53,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,   154,   155,    -1,   157,   158,   200,   160,   161,   162,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   195,   199,   133,   134,    -1,
     203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,    -1,    -1,   199,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    29,    -1,   106,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      55,    -1,   199,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   151,    77,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   175,    55,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    77,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    29,    -1,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,   164,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
     175,    -1,    -1,    -1,   133,   134,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    77,   151,    -1,   199,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,   164,    -1,    -1,    -1,    -1,
      -1,    29,    98,    -1,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    55,    -1,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,
     176,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,   199,    -1,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,   151,    79,    -1,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    30,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    46,    47,
      -1,   199,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
     155,    -1,   157,   158,    -1,   160,   161,   162,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,   204,
      -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    35,   166,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    66,    -1,
      -1,   199,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    77,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    46,
      47,   199,    -1,    -1,    -1,    52,   204,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,   157,   158,    -1,   160,   161,   162,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    66,
      67,    68,   199,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      66,    -1,   199,    -1,    -1,    -1,    -1,   204,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    66,    -1,   199,    -1,    -1,   202,    -1,   204,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,   132,   133,    -1,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    66,   199,    68,    -1,    -1,    -1,   204,
      -1,    74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    66,    -1,   199,    -1,    -1,    -1,
      -1,   204,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,
      -1,    -1,   133,    -1,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    98,    -1,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,   166,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,   199,    -1,
      -1,   151,    -1,   204,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,    98,    79,   199,
      -1,    -1,    -1,    -1,   204,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,   119,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,    98,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   155,    -1,   157,   158,    -1,
     160,   161,   162,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    10,    11,    12,    -1,
      -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
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
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   131,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   131,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    65,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    29,
     131,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    98,   131,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    98,   160,
     161,   162,    -1,    -1,    -1,   120,   106,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    77,   151,    79,    80,   154,
     155,   131,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    98,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    77,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,   158,    98,   160,   161,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    77,    -1,    -1,    -1,    -1,   155,
      98,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    98,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,   154,    -1,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    77,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    77,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    28,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      97,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65
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
     446,   447,   449,   462,   464,   466,   117,   118,   119,   132,
     151,   161,   216,   247,   326,   345,   438,   345,   199,   345,
     345,   345,   103,   345,   345,   426,   427,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,    79,
      86,   119,   146,   199,   224,   364,   400,   401,   406,   438,
     441,   438,    35,   345,   453,   454,   345,   119,   199,   224,
     400,   401,   402,   437,   445,   450,   451,   199,   336,   403,
     199,   336,   352,   337,   345,   233,   336,   199,   199,   199,
     336,   201,   345,   216,   201,   345,    29,    55,   133,   134,
     155,   175,   199,   216,   227,   467,   480,   481,   483,   182,
     201,   342,   345,   369,   371,   202,   240,   345,   106,   107,
     154,   217,   220,   223,    79,   204,   292,   293,   118,   125,
     117,   125,    79,   294,   199,   199,   199,   199,   216,   264,
     468,   199,   199,   337,    79,    85,   147,   148,   149,   459,
     460,   154,   202,   223,   223,   216,   265,   468,   155,   199,
     468,   468,    79,   196,   202,   354,   335,   345,   346,   438,
     442,   229,   202,    85,   404,   459,    85,   459,   459,    30,
     154,   171,   469,   199,     9,   201,    35,   246,   155,   263,
     468,   119,   181,   247,   327,   201,   201,   201,   201,   201,
     201,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    53,    65,   201,    66,
      66,   201,   202,   150,   126,   161,   163,   176,   178,   266,
     325,   326,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    63,    64,   129,   130,   430,
      66,   202,   435,   199,   199,    66,   202,   204,   446,   199,
     246,   247,    14,   345,   201,   131,    44,   216,   425,    85,
     335,   346,   150,   438,   131,   206,     9,   411,   199,   335,
     438,   469,   150,   199,   405,   430,   435,   200,   345,    30,
     231,     8,   357,     9,   201,   231,   232,   337,   338,   345,
     216,   278,   235,   201,   201,   201,   483,   483,   171,   199,
     106,   483,    14,   150,   216,    79,   201,   201,   201,   182,
     183,   184,   189,   190,   193,   194,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   384,   385,   386,   241,   110,
     168,   201,   154,   218,   221,   223,   154,   219,   222,   223,
     223,     9,   201,    97,   202,   438,     9,   201,   125,   125,
      14,     9,   201,   438,   463,   463,   335,   346,   438,   441,
     442,   200,   171,   258,   132,   438,   452,   453,   201,    66,
     430,   147,   460,    78,   345,   438,    85,   147,   460,   223,
     215,   201,   202,   253,   261,   390,   392,    86,   204,   358,
     359,   361,   401,   446,   464,    14,    97,   465,   353,   355,
     356,   288,   289,   428,   429,   200,   200,   200,   200,   203,
     230,   231,   248,   255,   260,   428,   345,   205,   207,   208,
     216,   470,   471,   483,    35,   164,   290,   291,   345,   467,
     199,   468,   256,   246,   345,   345,   345,    30,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     402,   345,   345,   448,   448,   345,   455,   456,   125,   202,
     216,   445,   446,   264,   216,   265,   468,   468,   263,   247,
      27,    35,   339,   342,   345,   369,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   155,   202,
     216,   431,   432,   433,   434,   445,   448,   345,   290,   290,
     448,   345,   452,   246,   200,   345,   199,   424,     9,   411,
     200,   200,   216,    35,   345,    35,   345,   200,   200,   445,
     290,   202,   216,   431,   432,   445,   200,   229,   282,   202,
     342,   345,   345,    89,    30,   231,   276,   201,    28,    97,
      14,     9,   200,    30,   202,   279,   483,    29,    86,   227,
     477,   478,   479,   199,     9,    46,    47,    52,    54,    66,
     133,   155,   175,   199,   224,   225,   227,   366,   400,   406,
     407,   408,   216,   482,   185,    79,   345,    79,    79,   345,
     381,   382,   345,   345,   374,   384,   188,   387,   229,   199,
     239,   223,   201,     9,    97,   223,   201,     9,    97,    97,
     220,   216,   345,   293,   407,    79,     9,   200,   200,   200,
     200,   200,   200,   200,   201,    46,    47,   475,   476,   127,
     269,   199,     9,   200,   200,    79,    80,   216,   461,   216,
      66,   203,   203,   212,   214,    30,   128,   268,   170,    50,
     155,   170,   394,   131,     9,   411,   200,   150,   483,   483,
      14,   357,   288,   229,   197,     9,   412,   483,   484,   430,
     435,   203,     9,   411,   172,   438,   345,   200,     9,   412,
      14,   349,   249,   127,   267,   199,   468,   345,    30,   206,
     206,   131,   203,     9,   411,   345,   469,   199,   259,   254,
     262,    14,   465,   257,   246,    68,   438,   345,   469,   206,
     203,   200,   200,   206,   203,   200,    46,    47,    66,    74,
      75,    76,    86,   133,   146,   175,   216,   414,   416,   417,
     420,   423,   216,   438,   438,   131,   430,   435,   200,   345,
     283,    71,    72,   284,   229,   336,   229,   338,    97,    35,
     132,   273,   438,   407,   216,    30,   231,   277,   201,   280,
     201,   280,     9,   172,    86,   131,   150,     9,   411,   200,
     164,   470,   471,   472,   470,   407,   407,   407,   407,   407,
     410,   413,   199,   150,   199,   407,   150,   202,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    65,   150,   469,   345,   185,   185,    14,
     191,   192,   383,     9,   195,   387,    79,   203,   400,   202,
     243,    97,   221,   216,    97,   222,   216,   216,   203,    14,
     438,   201,     9,   172,   216,   270,   400,   202,   452,   132,
     438,    14,   206,   345,   203,   212,   483,   270,   202,   393,
      14,   345,   358,   216,   201,   483,   197,   203,    30,   473,
     429,    35,    79,   164,   431,   432,   434,   483,    35,   164,
     345,   407,   288,   199,   400,   268,   350,   250,   345,   345,
     345,   203,   199,   290,   269,    30,   268,   483,    14,   267,
     468,   402,   203,   199,    14,    74,    75,    76,   216,   415,
     415,   417,   418,   419,    48,   199,    85,   147,   199,     9,
     411,   200,   424,    35,   345,   431,   432,   203,    71,    72,
     285,   336,   231,   203,   201,    90,   201,   273,   438,   199,
     131,   272,    14,   229,   280,   100,   101,   102,   280,   203,
     483,   131,   483,   216,   477,     9,   200,   411,   131,   206,
       9,   411,   410,   216,   358,   360,   362,   200,   125,   216,
     407,   457,   458,   407,   407,   407,    30,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     482,   345,   345,   345,   382,   345,   372,    79,   244,   216,
     216,   407,   476,    97,    98,   474,     9,   298,   200,   199,
     339,   342,   345,   206,   203,   465,   298,   156,   169,   202,
     389,   396,   156,   202,   395,   131,   201,   473,   483,   357,
     484,    79,   164,    14,    79,   469,   438,   345,   200,   288,
     202,   288,   199,   131,   199,   290,   200,   202,   483,   202,
     201,   483,   268,   251,   405,   290,   131,   206,     9,   411,
     416,   418,   147,   358,   421,   422,   417,   438,   336,    30,
      73,   231,   201,   338,   272,   452,   273,   200,   407,    96,
     100,   201,   345,    30,   201,   281,   203,   172,   483,   131,
     164,    30,   200,   407,   407,   200,   131,     9,   411,   200,
     131,   203,     9,   411,   407,    30,   186,   200,   229,   216,
     483,   483,   400,     4,   107,   112,   118,   120,   157,   158,
     160,   203,   299,   324,   325,   326,   331,   332,   333,   334,
     428,   452,   203,   202,   203,    50,   345,   345,   345,   357,
      35,    79,   164,    14,    79,   345,   199,   473,   200,   298,
     200,   288,   345,   290,   200,   298,   465,   298,   201,   202,
     199,   200,   417,   417,   200,   131,   200,     9,   411,    30,
     229,   201,   200,   200,   200,   236,   201,   201,   281,   229,
     483,   483,   131,   407,   358,   407,   407,   407,   345,   202,
     203,   474,   127,   128,   176,   467,   271,   400,   107,   334,
      29,   120,   133,   134,   155,   161,   308,   309,   310,   311,
     400,   159,   316,   317,   123,   199,   216,   318,   319,   300,
     247,   483,     9,   201,     9,   201,   201,   465,   325,   200,
     295,   155,   391,   203,   203,    79,   164,    14,    79,   345,
     290,   112,   347,   473,   203,   473,   200,   200,   203,   202,
     203,   298,   288,   131,   417,   358,   229,   234,   237,    30,
     231,   275,   229,   200,   407,   131,   131,   187,   229,   400,
     400,   468,    14,     9,   201,   202,   467,   465,   311,   171,
     202,     9,   201,     3,     4,     5,     6,     7,    10,    11,
      12,    13,    27,    28,    53,    67,    68,    69,    70,    71,
      72,    73,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   132,   133,   135,   136,   137,   138,   139,
     151,   152,   153,   163,   165,   166,   168,   175,   176,   178,
     180,   181,   216,   397,   398,     9,   201,   155,   159,   216,
     319,   320,   321,   201,    79,   330,   246,   301,   467,   467,
      14,   247,   203,   296,   297,   467,    14,    79,   345,   200,
     199,   202,   201,   202,   322,   347,   473,   295,   203,   200,
     417,   131,    30,   231,   274,   275,   229,   407,   407,   345,
     203,   201,   201,   407,   400,   304,   483,   312,   313,   406,
     309,    14,    30,    47,   314,   317,     9,    33,   200,    29,
      46,    49,    14,     9,   201,   468,   330,    14,   483,   246,
     201,    14,   345,    35,    79,   388,   229,   229,   202,   322,
     203,   473,   417,   229,    94,   188,   242,   203,   216,   227,
     305,   306,   307,     9,   172,     9,   411,   203,   407,   398,
     398,    55,   315,   320,   320,    29,    46,    49,   407,    79,
     199,   201,   407,   468,   407,    79,     9,   412,   203,   203,
     229,   322,    92,   201,    79,   110,   238,   150,    97,   483,
     406,   162,    14,   302,   199,    35,    79,   200,   203,   201,
     199,   168,   245,   216,   325,   326,   172,   407,   286,   287,
     429,   303,    79,   400,   243,   165,   216,   201,   200,     9,
     412,   114,   115,   116,   328,   329,   286,    79,   271,   201,
     473,   429,   484,   200,   200,   201,   201,   202,   323,   328,
      35,    79,   164,   473,   202,   229,   484,    79,   164,    14,
      79,   323,   229,   203,    35,    79,   164,    14,    79,   345,
     203,    79,   164,    14,    79,   345,    14,    79,   345,   345
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
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval).reset();;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval).reset();;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval).reset();;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval).reset();;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval).reset();;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval).reset();;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
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

  case 785:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
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
#line 2702 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
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

  case 795:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
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

  case 796:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval).reset();;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval)++;;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
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

  case 829:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
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

  case 830:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
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

  case 835:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
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

  case 836:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { (yyval).reset();;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
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

  case 872:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    {;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    {;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    {;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14059 "hphp.tab.cpp"
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
#line 3202 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

