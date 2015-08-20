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
#define YYLAST   17184

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  276
/* YYNRULES -- Number of rules.  */
#define YYNRULES  941
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1769

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
    2655,  2659,  2663,  2665,  2667,  2675,  2685,  2693,  2700,  2709,
    2711,  2714,  2719,  2724,  2726,  2728,  2733,  2735,  2736,  2738,
    2741,  2743,  2745,  2749,  2755,  2759,  2763,  2764,  2766,  2770,
    2776,  2780,  2783,  2787,  2794,  2795,  2797,  2802,  2805,  2806,
    2812,  2816,  2820,  2822,  2829,  2834,  2839,  2842,  2845,  2846,
    2852,  2856,  2860,  2862,  2865,  2866,  2872,  2876,  2880,  2882,
    2885,  2888,  2890,  2893,  2895,  2900,  2904,  2908,  2915,  2919,
    2921,  2923,  2925,  2930,  2935,  2940,  2945,  2950,  2955,  2958,
    2961,  2966,  2969,  2972,  2974,  2978,  2982,  2986,  2987,  2990,
    2996,  3003,  3010,  3018,  3020,  3023,  3025,  3030,  3034,  3035,
    3037,  3041,  3044,  3048,  3050,  3052,  3053,  3054,  3057,  3060,
    3063,  3068,  3071,  3077,  3081,  3083,  3085,  3086,  3090,  3095,
    3101,  3105,  3107,  3110,  3111,  3116,  3118,  3122,  3125,  3128,
    3131,  3133,  3135,  3137,  3139,  3143,  3148,  3155,  3157,  3166,
    3173,  3175
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
      -1,   445,    -1,   436,    -1,   199,   438,   200,    -1,   199,
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
    2746,  2750,  2754,  2755,  2759,  2762,  2769,  2773,  2777,  2784,
    2785,  2790,  2792,  2793,  2796,  2797,  2800,  2801,  2805,  2806,
    2810,  2811,  2812,  2823,  2834,  2837,  2840,  2841,  2842,  2853,
    2865,  2869,  2870,  2871,  2873,  2874,  2875,  2879,  2881,  2884,
    2886,  2887,  2888,  2889,  2892,  2894,  2895,  2899,  2901,  2904,
    2906,  2907,  2908,  2912,  2914,  2917,  2920,  2922,  2924,  2928,
    2929,  2931,  2932,  2938,  2939,  2941,  2951,  2953,  2955,  2958,
    2959,  2960,  2964,  2965,  2966,  2967,  2968,  2969,  2970,  2971,
    2972,  2973,  2974,  2978,  2979,  2983,  2985,  2993,  2995,  2999,
    3003,  3008,  3012,  3020,  3021,  3027,  3028,  3036,  3039,  3043,
    3046,  3051,  3056,  3058,  3059,  3060,  3064,  3065,  3069,  3070,
    3073,  3075,  3076,  3079,  3084,  3085,  3086,  3090,  3094,  3104,
    3112,  3114,  3118,  3120,  3125,  3131,  3134,  3137,  3144,  3147,
    3150,  3151,  3154,  3157,  3158,  3163,  3166,  3170,  3174,  3180,
    3190,  3191
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
     440,   441,   442,   442,   443,   443,   444,   444,   444,   445,
     445,   446,   446,   446,   447,   447,   448,   448,   449,   449,
     450,   450,   450,   450,   450,   450,   451,   451,   451,   451,
     451,   452,   452,   452,   452,   452,   452,   453,   453,   454,
     454,   454,   454,   454,   454,   454,   454,   455,   455,   456,
     456,   456,   456,   457,   457,   458,   458,   458,   458,   459,
     459,   459,   459,   460,   460,   460,   460,   460,   460,   461,
     461,   461,   462,   462,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   463,   463,   464,   464,   465,   465,   466,
     466,   466,   466,   467,   467,   468,   468,   469,   469,   470,
     470,   471,   472,   472,   472,   472,   473,   473,   474,   474,
     475,   475,   475,   475,   476,   476,   476,   477,   477,   477,
     478,   478,   479,   479,   480,   481,   482,   482,   483,   483,
     483,   483,   483,   483,   483,   483,   483,   483,   483,   483,
     484,   484
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
       3,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     4,     1,     0,     1,     2,
       1,     1,     3,     5,     3,     3,     0,     1,     3,     5,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     4,     3,     0,     1,
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
       0,   372,     0,   757,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   838,     0,
     826,   640,     0,   646,   647,   648,    22,   704,   814,   100,
     101,   649,     0,    81,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   134,     0,     0,     0,     0,     0,     0,
     339,   340,   341,   344,   343,   342,     0,     0,     0,     0,
     161,     0,     0,     0,   653,   655,   656,   650,   651,     0,
       0,     0,   657,   652,     0,   631,    24,    25,    26,    28,
      27,     0,   654,     0,     0,     0,     0,   658,   345,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   464,     0,    99,    71,   818,   641,     0,     0,
       4,    60,    62,    65,   703,     0,   630,     0,     6,   133,
       7,     9,     8,    10,     0,     0,   337,   382,     0,     0,
       0,     0,     0,     0,     0,   380,   802,   803,   450,   449,
     366,   453,     0,     0,   365,   780,   632,     0,   706,   448,
     336,   783,   381,     0,     0,   384,   383,   781,   782,   779,
     809,   813,     0,   438,   705,    11,   344,   343,   342,     0,
       0,    28,    60,   133,     0,   882,   381,   881,     0,   879,
     878,   452,     0,   373,   377,     0,     0,   422,   423,   424,
     425,   447,   445,   444,   443,   442,   441,   440,   439,   814,
     633,     0,   898,   632,     0,   404,     0,   402,     0,   842,
       0,   713,   364,   636,     0,   898,   635,     0,   645,   821,
     820,   637,     0,     0,   639,   446,     0,     0,     0,     0,
     369,     0,    79,   371,     0,     0,    85,    87,     0,     0,
      89,     0,     0,     0,   931,   932,   937,     0,     0,    60,
     930,     0,   933,     0,     0,     0,    91,     0,     0,     0,
       0,   124,     0,     0,     0,     0,     0,     0,    43,    48,
     250,     0,     0,   249,     0,   165,     0,   162,   255,     0,
       0,     0,     0,     0,   895,   149,   159,   834,   838,     0,
     863,     0,   660,     0,     0,     0,   861,     0,    16,     0,
      64,   141,   153,   160,   537,   480,     0,   887,   462,   466,
     468,   761,   382,     0,   380,   381,   383,     0,     0,   642,
       0,   643,     0,     0,     0,   123,     0,     0,    67,   241,
       0,    21,   132,     0,   158,   145,   157,   342,   345,   133,
     338,   114,   115,   116,   117,   118,   120,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   826,     0,   113,   817,   817,   121,   848,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   403,   401,   762,   763,     0,   817,     0,   775,   241,
     241,   817,     0,   819,   810,   834,     0,   133,     0,     0,
      93,     0,   759,   754,   713,     0,     0,     0,     0,     0,
     846,     0,   485,   712,   837,     0,     0,    67,     0,   241,
     363,     0,   777,   638,     0,    71,   201,     0,   461,     0,
      96,     0,     0,   370,     0,     0,     0,     0,     0,    88,
     112,    90,   928,   929,     0,   923,     0,     0,     0,     0,
     894,     0,   119,    92,   122,     0,     0,     0,     0,     0,
       0,     0,   495,     0,   502,   504,   505,   506,   507,   508,
     509,   500,   522,   523,    71,     0,   109,   111,     0,     0,
      45,    52,     0,     0,    47,    56,    49,     0,    18,     0,
       0,   251,     0,    94,   164,   163,     0,     0,    95,   883,
       0,     0,   382,   380,   381,   384,   383,     0,   916,   171,
       0,   835,     0,     0,    97,     0,     0,   659,   862,   704,
       0,     0,   860,   709,   859,    63,     5,    13,    14,     0,
     169,     0,     0,   473,     0,     0,   713,     0,     0,   634,
     474,     0,     0,     0,     0,   761,    71,     0,   715,   760,
     941,   362,   435,   789,   801,    76,    70,    72,    73,    74,
      75,   336,     0,   451,   707,   708,    61,   713,     0,   899,
       0,     0,     0,   715,   242,     0,   456,   135,   167,     0,
     407,   409,   408,     0,     0,   405,   406,   410,   412,   411,
     427,   426,   429,   428,   430,   432,   433,   431,   421,   420,
     414,   415,   413,   416,   417,   419,   434,   418,   816,     0,
       0,   852,     0,   713,   886,     0,   885,   786,   809,   151,
     143,   155,     0,   887,   147,   133,   372,     0,   375,   378,
     386,   496,   400,   399,   398,   397,   396,   395,   394,   393,
     392,   391,   390,   389,   765,     0,   764,   767,   784,   771,
     898,   768,     0,     0,     0,     0,     0,     0,     0,     0,
     880,   374,   752,   756,   712,   758,     0,     0,   898,     0,
     841,     0,   840,     0,   825,   824,     0,     0,   764,   767,
     822,   768,   367,   203,   205,    71,   471,   470,   368,     0,
      71,   185,    80,   371,     0,     0,     0,     0,     0,   197,
     197,    86,     0,     0,     0,     0,   921,   713,     0,   905,
       0,     0,     0,     0,     0,   711,   649,     0,     0,   631,
       0,     0,    65,   662,   630,   667,     0,   661,    69,   666,
     898,   934,     0,     0,   512,     0,     0,   518,   515,   516,
     524,     0,   503,   498,     0,   501,     0,     0,     0,    53,
      19,     0,     0,    57,    20,     0,     0,     0,    42,    50,
       0,   248,   256,   253,     0,     0,   872,   877,   874,   873,
     876,   875,    12,   914,   915,     0,     0,     0,     0,   834,
     831,     0,   484,   871,   870,   869,     0,   865,     0,   866,
     868,     0,     5,     0,     0,     0,   531,   532,   540,   539,
       0,     0,   712,   479,   483,     0,     0,   888,     0,   463,
       0,     0,   906,   761,   227,   940,     0,     0,   776,   815,
     712,   901,   897,   243,   244,   629,   714,   240,     0,   761,
       0,     0,   169,   458,   137,   437,     0,   488,   489,     0,
     486,   712,   847,     0,     0,   241,   171,     0,   169,     0,
       0,   167,     0,   826,   387,     0,     0,   773,   774,   787,
     788,   811,   812,     0,     0,     0,   740,   720,   721,   722,
     729,     0,     0,     0,   733,   731,   732,   746,   713,     0,
     754,   845,   844,     0,     0,   778,   644,     0,   207,     0,
       0,    77,     0,     0,     0,     0,     0,     0,     0,   177,
     178,   189,     0,    71,   187,   106,   197,     0,   197,     0,
       0,   935,     0,     0,     0,   712,   922,   924,   904,   713,
     903,     0,   713,   688,   689,   686,   687,   719,     0,   713,
     711,     0,     0,   482,     0,     0,   854,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   927,   497,     0,     0,     0,   520,
     521,   519,     0,     0,   499,     0,   125,     0,   128,   110,
       0,    44,    54,     0,    46,    58,    51,   252,     0,   884,
      98,   916,   896,   911,   170,   172,   262,     0,     0,   832,
       0,   864,     0,    17,     0,   887,   168,   262,     0,     0,
     476,     0,   885,   889,     0,   906,   469,     0,     0,   941,
       0,   232,   230,   767,   785,   898,   900,     0,     0,   245,
      68,     0,   761,   166,     0,   761,     0,   436,   851,   850,
       0,   241,     0,     0,     0,     0,     0,     0,   169,   139,
     645,   766,   241,     0,   725,   726,   727,   728,   734,   735,
     744,     0,   713,     0,   740,     0,   724,   748,   712,   751,
     753,   755,     0,   839,   767,   823,   766,     0,     0,     0,
       0,   204,   472,    82,     0,   371,   177,   179,   834,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   191,     0,
       0,   917,     0,   920,   712,     0,     0,     0,   664,   712,
     710,     0,   701,     0,   713,     0,   668,   702,   700,   858,
       0,   713,   671,   673,   672,     0,     0,   669,   670,   674,
     676,   675,   691,   690,   693,   692,   694,   696,   697,   695,
     684,   683,   678,   679,   677,   680,   681,   682,   685,   926,
     510,     0,   511,   517,   525,   526,     0,    71,    55,    59,
     254,     0,     0,     0,   913,     0,   336,   836,   834,   376,
     379,   385,     0,    15,     0,   336,   543,     0,     0,   545,
     538,   541,     0,   536,     0,   891,     0,   907,   465,     0,
     233,     0,     0,   228,     0,   247,   246,   906,     0,   262,
       0,   761,     0,   241,     0,   807,   262,   887,   262,   890,
       0,     0,     0,   388,     0,     0,   737,   712,   739,   730,
       0,   723,     0,     0,   713,   745,   843,     0,    71,     0,
     200,   186,     0,     0,     0,   176,   102,   190,     0,     0,
     193,     0,   198,   199,    71,   192,   936,   918,     0,   902,
       0,   939,   718,   717,   663,     0,   712,   481,   665,     0,
     487,   712,   853,   699,     0,     0,     0,     0,   910,   908,
     909,   173,     0,     0,     0,   343,   334,     0,     0,     0,
     150,   261,   263,     0,   333,     0,     0,     0,   887,   336,
       0,   867,   258,   154,   534,     0,     0,   475,   467,     0,
     236,   226,     0,   229,   235,   241,   455,   906,   336,   906,
       0,   849,     0,   806,   336,     0,   336,   892,   262,   761,
     804,   743,   742,   736,     0,   738,   712,   747,    71,   206,
      78,    83,   104,   180,     0,   188,   194,    71,   196,   919,
       0,     0,   478,     0,   857,   856,   698,     0,    71,   129,
     912,     0,     0,     0,     0,     0,   174,     0,   887,     0,
     299,   295,   301,   631,    28,     0,   289,     0,   294,   298,
     311,     0,   309,   314,     0,   313,     0,   312,     0,   133,
     265,     0,   267,     0,   268,   269,     0,     0,   833,     0,
     535,   533,   544,   542,   237,     0,     0,   224,   234,     0,
       0,     0,     0,   146,   455,   906,   808,   152,   258,   156,
     336,     0,     0,   750,     0,   202,     0,     0,    71,   183,
     103,   195,   938,   716,     0,     0,     0,     0,     0,     0,
     361,     0,     0,   279,   283,   358,   359,   293,     0,     0,
       0,   274,   595,   594,   591,   593,   592,   612,   614,   613,
     583,   554,   555,   573,   589,   588,   550,   560,   561,   563,
     562,   582,   566,   564,   565,   567,   568,   569,   570,   571,
     572,   574,   575,   576,   577,   578,   579,   581,   580,   551,
     552,   553,   556,   557,   559,   597,   598,   607,   606,   605,
     604,   603,   602,   590,   609,   599,   600,   601,   584,   585,
     586,   587,   610,   611,   615,   617,   616,   618,   619,   596,
     621,   620,   623,   625,   624,   558,   628,   626,   627,   622,
     608,   549,   306,   546,     0,   275,   327,   328,   326,   319,
       0,   320,   276,   353,     0,     0,     0,     0,   357,     0,
     133,   142,   257,     0,     0,     0,   225,   239,   805,     0,
      71,   329,    71,   136,     0,     0,     0,   148,   906,   741,
       0,    71,   181,    84,   105,     0,   477,   855,   513,   127,
     277,   278,   356,   175,     0,     0,   713,     0,   302,   290,
       0,     0,     0,   308,   310,     0,     0,   315,   322,   323,
     321,     0,     0,   264,     0,     0,     0,   360,     0,   259,
       0,   238,     0,   529,   715,     0,     0,    71,   138,   144,
       0,   749,     0,     0,     0,   107,   280,    60,     0,   281,
     282,     0,     0,   296,   712,   304,   300,   305,   547,   548,
       0,   291,   324,   325,   317,   318,   316,   354,   351,   270,
     266,   355,     0,   260,   530,   714,     0,   457,   330,     0,
     140,     0,   184,   514,     0,   131,     0,   336,     0,   303,
     307,     0,   761,   272,     0,   527,   454,   459,   182,     0,
       0,   108,   287,     0,   335,   297,   352,     0,   715,   347,
     761,   528,     0,   130,     0,     0,   286,   906,   761,   211,
     348,   349,   350,   941,   346,     0,     0,     0,   285,     0,
     347,     0,   906,     0,   284,   331,    71,   271,   941,     0,
     216,   214,     0,    71,     0,     0,   217,     0,     0,   212,
     273,     0,   332,     0,   220,   210,     0,   213,   219,   126,
     221,     0,     0,   208,   218,     0,   209,   223,   222
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   822,   556,   182,   277,   509,
     513,   278,   510,   514,   122,   123,   124,   125,   126,   127,
     327,   586,   587,   462,   241,  1446,   468,  1364,  1447,  1685,
     778,   272,   504,  1645,  1009,  1187,  1701,   343,   183,   588,
     860,  1066,  1242,   131,   559,   877,   589,   608,   881,   539,
     876,   590,   560,   878,   345,   295,   312,   134,   862,   825,
     808,  1024,  1385,  1120,   929,  1593,  1450,   722,   935,   467,
     731,   937,  1274,   714,   918,   921,  1109,  1707,  1708,   577,
     578,   602,   603,   282,   283,   289,  1419,  1572,  1573,  1196,
    1311,  1408,  1566,  1692,  1710,  1604,  1649,  1650,  1651,  1395,
    1396,  1397,  1398,  1606,  1607,  1613,  1661,  1401,  1402,  1406,
    1559,  1560,  1561,  1583,  1737,  1312,  1313,   184,   136,  1723,
    1724,  1564,  1315,  1316,  1317,  1318,   137,   234,   463,   464,
     138,   139,   140,   141,   142,   143,   144,   145,  1431,   146,
     859,  1065,   147,   238,   574,   321,   575,   576,   458,   565,
     566,  1144,   567,  1145,   148,   149,   150,   755,   151,   152,
     269,   153,   270,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   768,   769,  1001,   501,   502,   503,   775,  1634,
     154,   561,  1421,   562,  1038,   830,  1213,  1210,  1552,  1553,
     155,   156,   157,   228,   235,   330,   450,   158,   957,   759,
     159,   958,   851,   844,   959,   905,  1088,   906,  1090,  1091,
    1092,   908,  1253,  1254,   909,   693,   434,   195,   196,   591,
     580,   415,   677,   678,   679,   680,   848,   161,   229,   186,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   639,
     172,   231,   232,   542,   220,   221,   642,   643,  1150,  1151,
     305,   306,   816,   173,   530,   174,   573,   175,  1574,   296,
     338,   597,   598,   951,  1048,  1194,   805,   806,   736,   737,
     738,   262,   263,   761,   264,   846
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1440
static const yytype_int16 yypact[] =
{
   -1440,   155, -1440, -1440,  5152, 13186, 13186,   -33, 13186, 13186,
   13186, 11126, 13186, -1440, 13186, 13186, 13186, 13186, 13186, 13186,
   13186, 13186, 13186, 13186, 13186, 13186, 15562, 15562, 11332, 13186,
   15613,    65,   183, -1440, -1440, -1440, -1440, -1440,   179, -1440,
   -1440,   157, 13186, -1440,   183,   185,   228,   238, -1440,   183,
   11538, 16746, 11744, -1440, 14563, 10096,   249, 13186, 16411,    10,
   -1440, -1440, -1440,    46,   525,    38,   257,   259,   264,   268,
   -1440, 16746,   288,   302, -1440, -1440, -1440, -1440, -1440, 13186,
     390, 15214, -1440, -1440, 16746, -1440, -1440, -1440, -1440, 16746,
   -1440, 16746, -1440,   145,   322, 16746, 16746, -1440,   362, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, 13186, -1440, -1440,   278,   423,   476,   476,
   -1440,   217,   316,   -17, -1440,   337, -1440,    70, -1440,   439,
   -1440, -1440, -1440, -1440, 16515,   520, -1440, -1440,   350,   361,
     379,   387,   394,   397,  4621, -1440, -1440, -1440, -1440,   541,
   -1440,   543,   548,   415, -1440,   127,   425,   509, -1440, -1440,
     776,    33,  2576,   137,   433,   129, -1440,   139,   253,   465,
     270, -1440,    32, -1440,   600, -1440, -1440, -1440,   522,   480,
     523, -1440, -1440,   439,   520, 17003,  3215, 17003, 13186, 17003,
   17003, 10285,   488, 16017, 10285,   637, 16746,   618,   618,   109,
     618,   618,   618,   618,   618,   618,   618,   618,   618, -1440,
   -1440, 15112,   526, -1440,   550,    30,   513,    30, 15562, 16061,
     501,   706, -1440,   522, 15166,   526,   567,   582,   552,   271,
   -1440,    30,   137, 11950, -1440, -1440, 13186,  8654,   734,    74,
   17003,  9684, -1440, 13186, 13186, 16746, -1440, -1440, 13784,   557,
   -1440, 13828, 14563, 14563,   594, -1440, -1440,   568, 14303,    67,
     619,   756, -1440,   622, 16746,   695, -1440,   574, 13872,   575,
     689, -1440,    -7, 13916, 16530, 16573, 16746,    76, -1440,    59,
   -1440, 15315,    77, -1440,   652, -1440,   654, -1440,   766,    78,
   15562, 15562, 13186,   581,   611, -1440, -1440, 15413, 11332,    85,
     395,   437, -1440, 13392, 15562,   508, -1440, 16746, -1440,    -4,
     316, -1440, -1440, -1440, -1440, 15732,   769,   688, -1440, -1440,
   -1440,   112,   588, 17003,   591,   985,   598,  5358, 13186,    41,
     596,   481,    41,   421,   355, -1440, 16746, 14563,   604, 10302,
   14563, -1440, -1440,  2047, -1440, -1440, -1440, -1440, -1440,   439,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, 13186, 13186, 13186,
   12156, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186,
   13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186,
   13186, 13186, 15613, 13186, -1440, 13186, 13186, -1440, 13186, 14636,
   16746, 16746, 16746, 16746, 16746, 16515,   698,   869,  9890, 13186,
   13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186, 13186,
   13186, -1440, -1440, -1440, -1440, 14856, 13186, 13186, -1440, 10302,
   10302, 13186, 13186,   278,   274, 15413,   608,   439, 12362, 13960,
   -1440, 13186, -1440,   609,   800, 15112,   610,     0,   598, 15831,
      30, 12568, -1440, 12774, -1440,   612,    18, -1440,    55, 10302,
   -1440, 15773, -1440, -1440, 14004, -1440, -1440, 10508, -1440, 13186,
   -1440,   725,  8860,   806,   615, 16914,   805,   104,    54, -1440,
   -1440, -1440, -1440, -1440, 14563, 14783,   624,   812, 14933, 16746,
   -1440,   640, -1440, -1440, -1440,   757, 13186,   758,   761, 13186,
   13186, 13186, -1440,   689, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440,   647, -1440, -1440, -1440,   642, -1440, -1440, 16746,   646,
     834,    71, 16746,   648,   839,   295,   325, 16588, -1440, 16746,
   13186,    30,    10, -1440, -1440, -1440, 14933,   771, -1440,    30,
     107,   110,   657,   658,  2320,   191,   659,   651,   126,   735,
     668,    30,   114,   669, -1440, 16469, 16746, -1440, -1440,   811,
    2647,    13, -1440, -1440, -1440,   316, -1440, -1440, -1440,   851,
     760,   714,    64, -1440,   278,   755,   880,   699,   750,   274,
   -1440, 14563, 14563,   889,   734,   112, -1440,   707,   896, -1440,
   14563,    40,   841,   149, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440,   630,  2748, -1440, -1440, -1440, -1440,   900,   738, -1440,
   15562, 13186,   712,   904, 17003,   905, -1440, -1440,   793,  2937,
   10697, 11729, 10285, 13186, 16959, 12757, 12963, 14133,  3831, 12134,
   15795, 15795, 15795, 15795,  1264,  1264,  1264,  1264,   781,   781,
     685,   685,   685,   109,   109,   109, -1440,   618, 17003,   715,
     717, 16118,   723,   918, -1440, 13186,   -22,   730,   274, -1440,
   -1440, -1440,   916,   688, -1440,   439, 13186, 15264, -1440, -1440,
   10285, -1440, 10285, 10285, 10285, 10285, 10285, 10285, 10285, 10285,
   10285, 10285, 10285, 10285, -1440, 13186,   272,   276, -1440, -1440,
     526,   369,   727,  3405,   736,   740,   729,  3539,   117,   739,
   -1440, 17003, 15063, -1440, 16746, -1440,   588,    40,   526, 15562,
   17003, 15562, 16162,    40,   281, -1440,   742, 13186, -1440,   311,
   -1440, -1440, -1440,  8448,   461, -1440, -1440, 17003, 17003,   183,
   -1440, -1440, -1440, 13186,   846,  4035, 14933, 16746,  9066,   743,
     746, -1440,    69,   863,   819,   801, -1440,   944,   762, 14376,
   14563, 14933, 14933, 14933, 14933, 14933, -1440,   765,    47,   810,
     767, 14933,   275, -1440,   815, -1440,   772, -1440, 17089, -1440,
       8, -1440, 13186,   783, 17003,   784,   957, 11111,   966, -1440,
   17003, 14090, -1440,   647,   897, -1440,  5564, 16350,   777,   338,
   -1440, 16530, 16746,   413, -1440, 16573, 16746, 16746, -1440, -1440,
    3690, -1440, 17089,   964, 15562,   789, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440,    83, 16746, 16350,   778, 15413,
   15511,   980, -1440, -1440, -1440, -1440,   790, -1440, 13186, -1440,
   -1440,  4728, -1440, 14563, 16350,   813, -1440, -1440, -1440, -1440,
     981, 13186, 15732, -1440, -1440, 16692,   816, -1440, 14563, -1440,
     817,  5770,   982,    43, -1440, -1440,    66, 14856, -1440, -1440,
   14563, -1440, -1440,    30, 17003, -1440, 10714, -1440, 14933,    45,
     827, 16350,   760, -1440, -1440, 12551, 13186, -1440, -1440, 13186,
   -1440, 13186, -1440,  3960,   829, 10302,   735,   986,   760, 14563,
    1015,   793, 16746, 15613,    30,  4236,   832, -1440, -1440,   340,
   -1440, -1440, -1440,  1018, 15712, 15712, 15063, -1440, -1440, -1440,
     987,   835,    82,   838, -1440, -1440, -1440, -1440,  1030,   843,
     609,    30,    30, 12980, 15773, -1440, -1440,  4345,   511,   183,
    9684, -1440,  5976,   844,  6182,   845,  4035, 15562,   848,   921,
      30, 17089,  1040, -1440, -1440, -1440, -1440,   620, -1440,   328,
   14563, -1440,   928, 14563, 16746, 14783, -1440, -1440, -1440,  1052,
   -1440,   864,   900,   603,   603,   998,   998, 16263,   861,  1061,
   14933,   924, 16746, 15732, 14048, 16707, 14933, 14933, 14933, 14933,
   14713, 14933, 14933, 14933, 14933, 14933, 14933, 14933, 14933, 14933,
   14933, 14933, 14933, 14933, 14933, 14933, 14933, 14933, 14933, 14933,
   14933, 14933, 14933, 16746, -1440, 17003, 13186, 13186, 13186, -1440,
   -1440, -1440, 13186, 13186, -1440,   689, -1440,   993, -1440, -1440,
   16746, -1440, -1440, 16746, -1440, -1440, -1440, -1440, 14933,    30,
   -1440,   126, -1440,   514,  1065, -1440, -1440,   118,   881,    30,
   10920, -1440,  2043, -1440,  4946,   688,  1065, -1440,   374,    19,
   17003,   950, -1440, -1440,   883,   982, -1440, 14563,   734, 14563,
      56,  1071,  1007,   342, -1440,   526, -1440, 15562, 13186, 17003,
   17089,   887,    45, -1440,   886,    45,   891, 12551, 17003, 16219,
     892, 10302,   893,   890, 14563,   894,   898, 14563,   760, -1440,
     552,   407, 10302, 13186, -1440, -1440, -1440, -1440, -1440, -1440,
     963,   899,  1088,  1012, 15063,   953, -1440, 15732, 15063, -1440,
   -1440, -1440, 15562, 17003,   344, -1440, -1440,   183,  1072,  1031,
    9684, -1440, -1440, -1440,   902, 13186,   921,    30, 15413,  4035,
     906, 14933,  6388,   629,   907, 13186,    50,   331, -1440,   937,
   14563, -1440,   990, -1440, 14444,  1081,   912, 14933, -1440, 14933,
   -1440,   913, -1440,   991,  1108,   923, -1440, -1440, -1440, 16320,
     915,  1115, 11315, 11935, 12345, 14933, 17047,  3918, 13594, 14881,
   15127, 15904, 17119, 17119, 17119, 17119,  2118,  2118,  2118,  2118,
     973,   973,   603,   603,   603,   998,   998,   998,   998, -1440,
   17003, 13377, 17003, -1440, 17003, -1440,   925, -1440, -1440, -1440,
   17089, 16746, 14563, 14563, -1440, 16350,    92, -1440, 15413, -1440,
   -1440, 10285,   926, -1440,   929,   503, -1440,    72, 13186, -1440,
   -1440, -1440, 13186, -1440, 13186, -1440,   734, -1440, -1440,   182,
    1112,  1048, 13186, -1440,   933,    30, 17003,   982,   934, -1440,
     947,    45, 13186, 10302,   951, -1440, -1440,   688, -1440, -1440,
     927,   931,   945, -1440,   952, 15063, -1440, 15063, -1440, -1440,
     955, -1440,  1022,   956,  1148, -1440,    30,  1128, -1440,   958,
   -1440, -1440,   960,   965,   122, -1440, -1440, 17089,   968,   969,
   -1440,  4562, -1440, -1440, -1440, -1440, -1440, -1440, 14563, -1440,
   14563, -1440, 17089, 16364, -1440, 14933, 15732, -1440, -1440, 14933,
   -1440, 14933, -1440,  3263, 14933, 13186,   962,  6594,   514, -1440,
   -1440, -1440,    53, 14517, 16350,  1059, -1440,  2170,  1013, 15462,
   -1440, -1440, -1440,   698, 14257,    88,    89,   970,   688,   869,
     135, -1440, -1440, -1440,  1024,  4418,  4518, 17003, -1440,    60,
    1159,  1098, 13186, -1440, 17003, 10302,  1070,   982,  1023,   982,
     984, 17003,   988, -1440,  1289,   992,  1429, -1440, -1440,    45,
   -1440, -1440,  1055, -1440, 15063, -1440, 15732, -1440, -1440,  8448,
   -1440, -1440, -1440, -1440,  9272, -1440, -1440, -1440,  8448, -1440,
     989, 14933, 17089,  1062, 17089, 16421,  3263, 13171, -1440, -1440,
   -1440, 16350, 16350, 16746,  1178,    61, -1440, 14517,   688, 16396,
   -1440,  1025, -1440,    91,   995,    93, -1440, 13598, -1440, -1440,
   -1440,    95, -1440, -1440, 15363, -1440,   994, -1440,  1120,   439,
   -1440, 14563, -1440, 14563, -1440, -1440,  1186,   698, -1440, 14089,
   -1440, -1440, -1440, -1440,  1188,  1126, 13186, -1440, 17003,  1006,
    1009,  1008,   473, -1440,  1070,   982, -1440, -1440, -1440, -1440,
    1829,  1011, 15063, -1440,  1083,  8448,  9478,  9272, -1440, -1440,
   -1440,  8448, -1440, 17089, 14933, 14933, 13186,  6800,  1014,  1017,
   -1440, 14933, 16350, -1440, -1440, -1440, -1440, -1440, 14563,   779,
    2170, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440,   482, -1440,  1013, -1440, -1440, -1440, -1440, -1440,
     103,   528, -1440,  1198,    98, 16746,  1120,  1206, -1440, 14563,
     439, -1440, -1440,  1020,  1209, 13186, -1440, 17003, -1440,   105,
   -1440, -1440, -1440, -1440,  1027,   473, 14136, -1440,   982, -1440,
   15063, -1440, -1440, -1440, -1440,  7006, 17089, 17089, 11523, -1440,
   -1440, -1440, 17089, -1440,  2301,   116,  1215,  1028, -1440, -1440,
   14933, 13598, 13598,  1170, -1440, 15363, 15363,   636, -1440, -1440,
   -1440, 14933,  1151, -1440,  1034,   100, 14933, -1440, 16746, -1440,
   14933, 17003,  1155, -1440,  1226,  7212,  7418, -1440, -1440, -1440,
     473, -1440,  7624,  1035,  1158,  1129, -1440,  1143,  1091, -1440,
   -1440,  1146, 14563, -1440,   779, -1440, -1440, 17089, -1440, -1440,
    1082, -1440,  1212, -1440, -1440, -1440, -1440, 17089,  1232, -1440,
   -1440, 17089,  1050, 17089, -1440,   143,  1051, -1440, -1440,  7830,
   -1440,  1049, -1440, -1440,  1054,  1087, 16746,   869,  1085, -1440,
   -1440, 14933,    48, -1440,  1179, -1440, -1440, -1440, -1440, 16350,
     777, -1440,  1094, 16746,   486, -1440, 17089,  1060,  1252,   625,
      48, -1440,  1183, -1440, 16350,  1064, -1440,   982,   106, -1440,
   -1440, -1440, -1440, 14563, -1440,  1063,  1066,   101, -1440,   490,
     625,   386,   982,  1067, -1440, -1440, -1440, -1440, 14563,    62,
    1254,  1191,   490, -1440,  8036,   389,  1259,  1195, 13186, -1440,
   -1440,  8242, -1440,    73,  1261,  1197, 13186, -1440, 17003, -1440,
    1263,  1200, 13186, -1440, 17003, 13186, -1440, 17003, 17003
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1440, -1440, -1440,  -496, -1440, -1440, -1440,   254, -1440, -1440,
   -1440,   764,   504,   497,    37,  1450,  3077, -1440,  2520, -1440,
    -302, -1440,    35, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440, -1440,  -414, -1440, -1440,  -157,   478,    21,
   -1440, -1440, -1440, -1440, -1440, -1440,    23, -1440, -1440, -1440,
   -1440,    24, -1440, -1440,   901,   908,   895,   -97,   410,  -796,
     416,   470,  -419,   181,  -853, -1440,  -147, -1440, -1440, -1440,
   -1440,  -653,    31, -1440, -1440, -1440, -1440,  -409, -1440,  -543,
   -1440,  -358, -1440, -1440,   796, -1440,  -118, -1440, -1440,  -969,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
    -148, -1440,   -66, -1440, -1440, -1440, -1440, -1440,  -230, -1440,
      22,  -921, -1440, -1439,  -416, -1440,  -152,   120,  -113,  -402,
   -1440,  -234, -1440, -1440, -1440,    28,   -48,     9,   154,  -667,
    -352, -1440, -1440,   -12, -1440, -1440,    -5,   -38,   -98, -1440,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,  -520,  -777,
   -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,
     939, -1440, -1440,   333, -1440,   847, -1440, -1440, -1440, -1440,
   -1440, -1440, -1440,   339, -1440,   849, -1440, -1440,   566, -1440,
     305, -1440, -1440, -1440, -1440, -1440, -1440, -1440, -1440,  -902,
   -1440,  2235,  1747,  -331, -1440, -1440,   265,  2790,  3357, -1440,
   -1440,   384,  -181,  -601, -1440, -1440,   451,   256,  -656,   260,
   -1440, -1440, -1440, -1440, -1440,   440, -1440, -1440, -1440,    17,
    -812,  -162,  -394,  -387, -1440,   500,  -104, -1440, -1440,   462,
   -1440, -1440,  2592,   -46, -1440, -1440,    75,  -124, -1440,   255,
   -1440, -1440, -1440,  -381,  1053, -1440, -1440, -1440, -1440, -1440,
     643,   463, -1440, -1440,  1068,  -273,  -604, -1440,   -19,   -62,
    -187,   -14,   614, -1440,  -930,    58, -1440,   336,   417, -1440,
   -1440, -1440, -1440,   365,  1431, -1019
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -926
static const yytype_int16 yytable[] =
{
     185,   187,   857,   189,   190,   191,   193,   194,   396,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   160,   350,   219,   222,   130,   426,   132,   133,   313,
    1219,  1049,   840,   316,   317,   261,   907,   240,   447,   128,
     444,   237,   570,   267,   688,   248,   658,   251,   424,   880,
     268,   636,   273,   242,   839,  1041,   925,   709,   246,   418,
     821,   684,   685,   395,   710,   322,  1064,   326,  1205,   451,
    1462,   350,   346,  1116,   240,   324,  -893,   939,   940,   340,
    1272,  -893,  1075,   459,   729,   517,   522,   527,    13,   280,
      13,   706,  1021,    13,   459,   279,  1302,  1411,  1413,  -793,
    -292,  1050,  1470,   505,  1554,   230,   416,  1622,   323,  1622,
    1462,   209,  1615,   727,   827,  1216,   794,   288,   309,   794,
     300,   310,  1324,   810,   135,  1652,   810,   810,   452,   413,
     414,   810,   961,  -898,   209,  1220,  1616,    13,   546,  1424,
    1632,  1746,   413,   414,   810,  1051,  1638,   413,   414,   337,
     336,    13,  1760,   713,   337,     3,   519,    13,   993,   413,
     414,   506,   382,   436,   284,   438,   188,  1095,   782,   413,
     414,   285,   803,   804,   383,  1211,   445,  -898,  1694,   337,
    1381,  1382,  -898,   429,  1633,  -898,  1143,  -460,  -633,   303,
     304,   569,   609,  -790,   302,  -492,   239,   557,   558,  1303,
     583,  1680,   776,   416,  1304,  -791,    60,    61,    62,   176,
    1305,   347,  1306,   336,   281,  -797,   820,  1329,   704,   828,
    1221,  1212,  1695,  -493,  1425,   336,  1747,   314,   454,  1096,
    1052,   454,  -799,   299,   829,  -793,   423,  1761,   240,   465,
    -714,   941,   417,  -714,   532,  -231,   536,   335,  -215,  1307,
    1308,  1273,  1309,   695,   533,  1022,   730,  -492,   121,   116,
    1338,  1330,  1463,  1464,   233,   648,  1265,  1344,  -893,  1346,
     689,   341,   456,   348,   841,   460,   461,   518,   523,   528,
     397,   607,  1241,  1123,   350,  1127,   544,   323,  1653,  1412,
    1414,   648,  -292,   219,  1471,  1310,  1555,  1336,   550,  1623,
     314,  1670,  1734,  1617,   728,   249,  -714,   795,   259,  -231,
     796,   511,   515,   516,   811,   648,  1061,   893,  1197,  -792,
    1252,   605,  1363,   592,   648,   294,  1034,   648,   420,  -790,
     313,   652,   653,   346,   604,  1418,   421,  -827,   579,   417,
     421,  -791,  -794,   311,   555,   294,  1331,  -830,  -800,   294,
     294,  -797,   610,   611,   612,   614,   615,   616,   617,   618,
     619,   620,   621,   622,   623,   624,   625,   626,   627,   628,
     629,   630,   631,   632,   633,   634,   635,  -828,   637,  1440,
     638,   638,   236,   641,   243,   833,   659,   696,   294,   438,
     420,   800,   786,   660,   662,   663,   664,   665,   666,   667,
     668,   669,   670,   671,   672,   673,  -796,  1432,  -795,  1434,
    -829,   638,   683,   922,   604,   604,   638,   687,   924,   847,
    -634,  1739,   787,   660,  1753,  -898,   691,   244,  1027,  1125,
    1126,  1204,  1125,  1126,   300,  1010,   700,   245,   702,   396,
     552,   318,  1255,  -769,   604,   716,   337,   135,  1262,   336,
     433,   271,   717,  1053,   718,  -792,   290,   230,   291,   874,
    1054,   545,   872,   292,   647,  1740,   162,   293,  1754,   300,
     336,  -769,   422,  -827,   342,   301,   422,  -898,  -794,   336,
     328,   764,   129,  -830,   767,   770,   771,   297,   215,   217,
     681,   121,   336,   886,   395,   121,  1610,   721,   882,   466,
     300,   298,   300,   303,   304,  1585,   552,  1302,   329,  1373,
    1013,   874,  1611,  -828,   647,   790,   300,  1072,   480,  1228,
    1104,   315,  1230,   705,   413,   414,   711,  1105,  1218,  1612,
    1206,  1128,   919,   920,  1275,   847,   339,   302,   303,   304,
    -772,   914,  -796,  1207,  -795,   779,  -829,   864,    13,   783,
    1741,   351,  1080,  1755,   279,   300,   946,  1618,   319,   570,
     300,   332,   352,   595,   320,   447,   552,   336,  -772,   303,
     304,   303,   304,   994,  1619,   325,  1208,  1620,  -770,  1444,
     353,   121,  1107,  1108,   547,   303,   304,   300,   354,  1351,
     596,  1352,   579,   552,   259,   355,   854,   294,   356,   915,
      60,    61,    62,   176,   177,   347,  -770,  -490,   865,   385,
    1303,  1192,  1193,   349,   386,  1304,   387,    60,    61,    62,
     176,  1305,   347,  1306,   303,   304,    53,   388,   594,   303,
     304,  1122,   419,  1345,    60,    61,    62,   176,   177,   347,
     873,   640,   286,   646,   294,   650,   294,   294,   294,   294,
     287,   193,   989,   990,   991,   553,   303,   304,  1640,   389,
    1307,  1308,   427,  1309,  -798,  1664,  -491,   348,   992,   676,
     885,   682,  -633,   437,  1581,  1582,   686,   307,  1199,   425,
     440,   432,  1665,   383,   348,  1666,   446,  1716,  1340,   430,
     570,  1735,  1736,   698,  1662,  1663,  1328,   337,  1443,   162,
     439,   348,   917,   162,  1731,   708,  1323,   442,   569,  1658,
    1659,   397,   420,  1234,  1416,   443,   121,  -632,   240,  1745,
    1124,  1125,  1126,   648,  1244,   949,   952,  1099,   923,  1269,
    1125,  1126,   448,   760,   379,   380,   381,  1264,   382,  1720,
    1721,  1722,   457,   521,    60,    61,    62,    63,    64,   347,
     383,   449,   529,   529,   534,    70,   390,   995,   470,   541,
     331,   333,   334,   934,   548,   474,   551,   475,   554,  -925,
     478,  1136,   479,   789,   481,   482,   484,   524,  1140,   525,
     526,   537,   538,   571,  1466,   572,  1589,  1729,   581,   162,
     648,   582,   548,   392,   554,   548,   554,   554,   584,   815,
     817,   593,  1742,   -66,    53,   129,  1441,   606,   692,   694,
     697,   348,   703,  1032,   719,   459,   723,  1320,   511,   726,
    1079,   740,   515,   739,   570,   762,  1040,   376,   377,   378,
     379,   380,   381,   135,   382,   774,   763,   765,   160,   569,
     766,   777,   130,   781,   132,   133,   383,   780,   785,   784,
     793,  1059,   802,    33,    34,    35,   128,   797,   798,   801,
     579,  1067,   807,   294,  1068,   746,  1069,   809,  1224,   812,
     604,   485,   486,   487,   655,  1342,   579,   818,   488,   489,
    1709,   823,   490,   491,   826,  1297,   831,   541,   824,   832,
      60,    61,    62,    63,    64,   347,   135,   437,  1709,   834,
     835,    70,   390,   838,   842,   843,  1730,  -494,  1103,   850,
     852,  1248,   855,   856,    74,    75,    76,    77,    78,   858,
     861,   867,   681,   868,   162,   748,   870,   871,  1110,   875,
     879,    82,    83,   887,  1641,   891,   889,   391,   863,   392,
     890,   135,   916,   926,   936,    92,   904,   938,   910,   942,
     943,   944,   393,   945,   394,  1111,  1359,   348,   230,    97,
     962,   135,   947,  1287,   960,   965,   963,   121,   996,   997,
    1292,   998,  1368,   569,   966,  1002,  1005,  1429,  1018,  1008,
    1026,   932,   121,    60,    61,    62,   176,   177,   347,   711,
    1020,  1180,  1181,  1182,  1030,  1039,  1031,   767,  1184,   428,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   408,
     409,   410,  1047,   570,  1045,  1037,  1074,  1043,  1200,   986,
     987,   988,   989,   990,   991,  1201,  1062,  1302,  1071,  1077,
     121,  1082,  1083,  1676,  1094,  1093,  1012,  1097,   992,  1098,
    1015,  1016,   135,  1100,   135,  1113,  1115,  1118,   411,   412,
     348,   160,  1119,  1226,  1121,   130,  1445,   132,   133,  1130,
    1023,  1134,   853,   992,  1135,  1451,   604,  1138,    13,   128,
    1139,   547,  1186,  1357,  1195,   121,  1457,   604,  1201,   579,
    1198,  1214,   579,   570,  1215,  1222,  1223,  1227,  1229,  1042,
    1231,  1233,  1236,  1235,  1245,   121,  1238,  1247,   900,  1239,
    1251,   676,  1258,  1261,  1259,  1246,  1266,  1719,  1270,  1276,
     240,  1280,  1281,  1284,   413,   414,  1257,  1286,  1290,   884,
    1271,  1278,  1285,  1288,  1291,  1296,  1332,  1333,  1347,  1321,
    1303,  1322,  1335,  1348,  1337,  1304,   294,    60,    61,    62,
     176,  1305,   347,  1306,  1349,  1260,  1595,  1339,  1087,  1087,
     904,  1343,  1350,  1354,   135,  1353,  1355,  1356,  1358,  1360,
    1361,   911,   569,   912,  1378,  1362,  1387,  1417,   708,  1365,
    1366,  1415,  1400,  1426,   121,   162,   121,  1427,   121,  1420,
    1307,  1308,  1430,  1309,  1435,   583,  1442,   930,  1436,  1452,
     162,   129,  1461,  1454,  1438,  1562,  1468,  1469,  1132,  1563,
    1569,   350,  1575,  1325,   348,  1576,  1578,  1326,  1579,  1327,
    1580,  1588,  1621,  1319,  1590,  1600,  1142,  1334,  1601,  1148,
    1626,  1629,  1319,  1630,  1654,  1660,  1433,  1341,   604,  1637,
    1668,  1656,   569,  1669,  1674,  1675,  1682,  1683,   162,  1684,
    -288,  1686,   135,  1687,  1690,  1616,  1691,   760,   579,  1693,
    1698,  1696,  1565,  1699,   129,  1700,  1019,  1705,  1711,  1714,
    1717,  1718,  1726,  1732,  1188,  1728,  1733,  1189,  1748,  1743,
    1749,   541,  1029,  1756,  1757,  1762,  1763,  1765,  1635,  1766,
    1636,   788,  1014,   162,  1384,  1011,  1713,   651,   121,  1642,
    1377,  1078,  1073,  1302,  1036,  1727,   654,  1263,   649,   129,
    1594,  1725,  1367,   162,  -926,  -926,  -926,  -926,   374,   375,
     376,   377,   378,   379,   380,   381,  1314,   382,   791,   129,
    1586,  1460,  1609,  1467,  1614,  1314,  1750,  1428,  1738,   383,
     604,  1407,  1625,  1388,    13,  1679,  1584,   661,  1185,  1004,
     772,  1183,   773,  1209,  1141,  1243,  1089,  1055,   904,  1249,
    1101,   543,   904,   950,  1250,  1319,  1380,  1191,  1179,   531,
       0,  1319,  1133,  1319,   121,     0,   579,     0,  1465,     0,
       0,     0,     0,     0,     0,     0,   121,     0,     0,     0,
       0,     0,   162,     0,   162,     0,   162,     0,   930,  1117,
       0,     0,  1567,     0,  1568,     0,  1303,     0,     0,  1449,
     129,  1304,   129,    60,    61,    62,   176,  1305,   347,  1306,
       0,     0,     0,  1628,     0,     0,     0,   135,     0,     0,
       0,  1577,     0,     0,     0,  1655,     0,     0,     0,     0,
       0,     0,     0,  1302,  1744,     0,     0,     0,     0,   397,
       0,  1751,     0,     0,     0,  1298,  1307,  1308,     0,  1309,
       0,  1598,     0,     0,     0,     0,     0,  1319,  1314,     0,
       0,     0,     0,     0,  1314,     0,  1314,     0,     0,     0,
     348,     0,     0,     0,    13,     0,   212,   212,     0,   135,
     225,  1592,  1449,     0,     0,     0,     0,     0,   135,     0,
       0,     0,  1437,     0,     0,     0,   162,     0,     0,   904,
       0,   904,     0,  1624,   225,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,     0,     0,     0,  1225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1703,  1303,     0,     0,     0,
       0,  1304,     0,    60,    61,    62,   176,  1305,   347,  1306,
       0,   121,     0,     0,     0,     0,     0,   259,     0,     0,
    1314,     0,     0,  1405,  1256,   135,  1672,     0,     0,     0,
    1631,   135,   162,     0,     0,     0,     0,   135,     0,     0,
     541,   930,     0,     0,   162,     0,  1307,  1308,     0,  1309,
       0,   350,     0,     0,     0,     0,     0,     0,     0,     0,
     129,     0,     0,     0,     0,     0,     0,     0,   904,     0,
     348,     0,     0,   121,     0,     0,     0,     0,   121,     0,
       0,     0,   121,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1439,     0,     0,     0,     0,   294,     0,     0,
       0,   259,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1551,     0,     0,     0,     0,     0,     0,  1558,     0,
     541,   212,     0,     0,     0,   259,     0,   259,   212,     0,
       0,     0,     0,   259,   212,     0,     0,     0,     0,     0,
       0,     0,     0,   472,   473,     0,     0,     0,     0,   477,
       0,     0,     0,     0,     0,     0,   904,     0,     0,   121,
     121,   121,   225,   225,     0,   121,     0,     0,   225,   579,
       0,   121,     0,     0,     0,   135,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   579,     0,     0,
       0,   212,     0,     0,     0,   579,     0,     0,     0,     0,
     212,   212,     0,  1758,     0,     0,     0,   212,     0,     0,
       0,  1764,     0,     0,   212,   135,   135,  1767,     0,   162,
    1768,     0,   135,     0,     0,   225,     0,     0,   599,     0,
       0,     0,     0,   214,   214,   129,     0,   227,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   225,     0,     0,
     225,  1409,     0,     0,     0,     0,     0,     0,     0,   135,
       0,     0,     0,     0,     0,     0,     0,  1704,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
       0,   162,     0,     0,     0,     0,   162,     0,     0,     0,
     162,     0,   225,  1302,     0,     0,     0,   129,     0,     0,
     259,     0,     0,     0,   904,     0,   129,     0,     0,   121,
       0,     0,     0,     0,     0,     0,     0,     0,  1647,     0,
       0,     0,     0,     0,   135,  1551,  1551,     0,     0,  1558,
    1558,   135,     0,     0,    13,   212,     0,     0,     0,     0,
       0,     0,   294,     0,     0,   212,     0,     0,     0,   121,
     121,     0,     0,     0,     0,  1570,   121,     0,     0,     0,
       0,     0,     0,     0,     0,   732,     0,   162,   162,   162,
       0,     0,     0,   162,     0,     0,     0,     0,     0,   162,
       0,     0,     0,   129,   225,   225,     0,     0,   752,   129,
       0,     0,     0,   121,     0,   129,  1303,     0,     0,     0,
    1702,  1304,     0,    60,    61,    62,   176,  1305,   347,  1306,
       0,     0,     0,     0,     0,     0,     0,  1715,   214,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   752,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1307,  1308,     0,  1309,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,     0,   836,   837,     0,   121,     0,     0,     0,     0,
     348,   845,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   225,   225,     0,     0,     0,     0,     0,   214,     0,
     225,     0,  1587,     0,     0,     0,     0,   214,   214,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
     212,   214,     0,   357,   358,   359,     0,   162,     0,     0,
       0,     0,   568,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   360,   129,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,     0,   382,   162,   162,     0,
       0,     0,     0,     0,   162,     0,     0,   212,   383,     0,
       0,     0,     0,   129,   129,     0,     0,     0,     0,     0,
     129,     0,     0,     0,    36,     0,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   162,     0,     0,     0,    48,     0,     0,     0,   212,
       0,   212,     0,     0,     0,     0,     0,   129,  -926,  -926,
    -926,  -926,   984,   985,   986,   987,   988,   989,   990,   991,
     599,   599,   214,     0,     0,   212,   752,     0,     0,     0,
       0,     0,   214,   992,     0,     0,     0,     0,     0,   225,
     225,   752,   752,   752,   752,   752,     0,     0,     0,  1389,
       0,   752,     0,     0,    86,    87,   162,    88,   181,    90,
       0,     0,     0,   162,     0,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,     0,   225,     0,   129,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,   212,     0,   606,    36,     0,  1202,
       0,     0,     0,     0,  1035,     0,     0,   225,     0,   212,
     212,   213,   213,     0,     0,   226,     0,     0,    48,  1044,
       0,     0,     0,   225,   225,     0,     0,     0,     0,     0,
       0,  1056,   225,     0,     0,     0,     0,     0,   225,     0,
    1390,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     225,     0,     0,  1391,  1392,     0,     0,     0,   752,     0,
    1076,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   180,     0,     0,    84,  1393,     0,    86,    87,   225,
      88,  1394,    90,   225,   428,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   214,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,  1129,     0,     0,  1131,     0,   212,   212,    36,     0,
       0,     0,     0,   411,   412,     0,     0,     0,     0,     0,
     225,     0,     0,   225,     0,   225,     0,     0,     0,    48,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
     752,     0,     0,   225,     0,     0,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,     0,     0,     0,   214,     0,   214,   413,
     414,     0,   180,   213,     0,    84,     0,     0,    86,    87,
       0,    88,   181,    90,     0,     0,     0,     0,   752,     0,
       0,     0,   214,     0,     0,     0,     0,     0,  1217,     0,
     845,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   225,     0,   225,
       0,     0,     0,     0,  1646,  1237,     0,   212,  1240,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
     799,     0,     0,     0,   225,   213,   213,   225,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,   213,
       0,   214,     0,     0,     0,     0,     0,   225,     0,     0,
     213,     0,   212,     0,     0,     0,   214,   214,     0,     0,
       0,  1277,     0,     0,     0,  1056,     0,     0,   212,   212,
       0,   752,     0,     0,   260,     0,     0,     0,     0,   568,
     225,     0,     0,     0,   225,     0,     0,   752,     0,   752,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,     0,     0,   752,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   226,   216,   216,
       0,     0,     0,  1299,  1300,     0,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,     0,     0,     0,   411,
     412,     0,   225,   225,     0,   225,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,   214,     0,   360,     0,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,     0,
     382,     0,     0,     0,     0,   413,   414,     0,     0,  1369,
     568,  1370,   383,   756,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   225,     0,
     225,     0,     0,     0,     0,   752,   225,     0,     0,   752,
       0,   752,     0,     0,   752,  1410,     0,     0,     0,     0,
       0,     0,     0,   225,   225,     0,     0,   225,   357,   358,
     359,   756,     0,     0,   225,     0,     0,     0,     0,     0,
       0,     0,   260,   260,     0,     0,     0,   360,   260,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
       0,   382,     0,   216,   214,     0,   225,     0,     0,     0,
     216,     0,     0,   383,     0,     0,   216,     0,     0,     0,
       0,   752,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   225,   225,     0,     0,   213,     0,   225,     0,   225,
       0,     0,     0,     0,   568,     0,     0,     0,     0,   214,
     819,     0,     0,     0,     0,     0,     0,   260,     0,     0,
     260,   225,     0,   225,     0,   214,   214,     0,     0,   225,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,   216,   216,   535,     0,     0,     0,     0,   216,
       0,     0,   213,     0,     0,     0,   216,     0,     0,  1605,
       0,     0,     0,     0,   752,   752,     0,     0,     0,     0,
       0,   752,   225,     0,     0,     0,     0,     0,   225,     0,
     225,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,   849,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   756,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,   756,   756,   756,
     756,     0,     0,     0,     0,     0,   756,     0,     0,     0,
       0,     0,     0,     0,   260,   735,     0,     0,   754,     0,
    1627,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1007,     0,    36,     0,     0,   216,     0,   225,
       0,     0,     0,     0,     0,     0,     0,   216,     0,   213,
       0,     0,     0,   568,     0,    48,   225,     0,     0,     0,
       0,     0,  1025,     0,   213,   213,   754,     0,     0,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,  1025,
     752,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,   752,     0,     0,     0,     0,   752,     0,     0,     0,
     752,     0,     0,  1688,     0,     0,     0,     0,     0,     0,
       0,   260,   260,   756,    86,    87,  1063,    88,   181,    90,
     260,     0,   225,   568,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   863,     0,     0,     0,
       0,   752,     0,     0,     0,     0,     0,     0,     0,   225,
       0,     0,     0,     0,   845,     0,     0,     0,     0,     0,
       0,   213,   213,     0,   225,     0,     0,     0,     0,   845,
       0,     0,     0,   225,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   225,     0,
       0,     0,   216,     0,     0,   756,     0,     0,   213,     0,
       0,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,   756,   756,
     756,   756,   756,   756,   756,   756,   756,   756,     0,   428,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   408,
     409,   410,     0,     0,     0,     0,   754,     0,     0,   216,
       0,     0,     0,   756,     0,     0,     0,     0,     0,   260,
     260,   754,   754,   754,   754,   754,     0,     0,   757,     0,
       0,   754,     0,     0,     0,     0,     0,     0,   411,   412,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   213,   216,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,     0,   757,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   992,     0,
       0,     0,   213,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,   260,   413,   414,     0,     0,     0,     0,
       0,     0,     0,   213,   213,     0,   756,     0,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,   756,     0,   756,     0,     0,     0,   754,     0,
       0,     0,     0,     0,     0,     0,   216,     0,     0,     0,
     756,     0,     0,     0,     0,     0,     0,     0,     0,   260,
       0,   216,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   357,   358,   359,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1301,     0,     0,   213,   360,     0,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,     0,   382,     0,
     260,     0,     0,   260,     0,   735,     0,     0,     0,     0,
     383,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     754,     0,     0,     0,     0,     0,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,     0,     0,     0,   757,     0,   216,   216,
     756,   213,     0,     0,   756,     0,   756,     0,     0,   756,
       0,   757,   757,   757,   757,   757,     0,     0,   754,  1386,
       0,   757,  1399,     0,     0,     0,     0,     0,     0,   357,
     358,   359,     0,     0,     0,   753,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,   360,   260,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   213,   382,     0,   260,     0,     0,   260,     0,     0,
       0,     0,     0,   753,   383,     0,   756,     0,   888,     0,
       0,     0,     0,     0,     0,     0,  1458,  1459,     0,     0,
       0,     0,     0,     0,  1399,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   754,     0,     0,     0,     0,     0,     0,   757,   216,
     260,     0,     0,     0,   260,     0,     0,   754,     0,   754,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   754,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   756,
     756,     0,     0,     0,   216,     0,   756,  1603,     0,     0,
     357,   358,   359,     0,     0,  1399,     0,     0,     0,     0,
     216,   216,   260,   260,     0,     0,     0,     0,     0,   360,
       0,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   892,   382,     0,     0,     0,     0,     0,     0,
     757,     0,     0,     0,     0,   383,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,   757,   757,   757,   757,   757,   757,   757,
     757,   757,   757,     0,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,     0,     0,     0,   260,     0,
     260,     0,     0,   753,     0,   754,     0,     0,   757,   754,
       0,   754,     0,     0,   754,     0,     0,     0,   753,   753,
     753,   753,   753,   260,     0,     0,     0,     0,   753,     0,
       0,     0,     0,     0,   260,   758,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   756,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,     0,     0,     0,
       0,   756,     0,     0,     0,   756,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   792,   382,     0,     0,     0,     0,     0,
       0,   754,     0,  1017,     0,     0,   383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,     0,     0,
       0,   757,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   756,   757,     0,   757,
       0,   260,     0,   260,  1712,   753,     0,     0,     0,   260,
       0,     0,     0,     0,     0,   757,     0,     0,     0,  1386,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     357,   358,   359,     0,   754,   754,     0,     0,     0,     0,
       0,   754,     0,   992,     0,     0,     0,     0,   260,   360,
       0,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,     0,   382,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   383,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   753,     0,     0,
       0,     0,     0,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     927,     0,     0,     0,     0,   757,     0,     0,     0,   757,
       0,   757,     0,   931,   757,     0,     0,     0,     0,   260,
       0,     0,     0,     0,     0,   753,     0,     0,   953,   954,
     955,   956,     0,     0,     0,     0,   260,     0,   964,     0,
       0,     0,    36,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1648,     0,     0,     0,     0,     0,
     754,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,   754,     0,     0,     0,     0,   754,     0,     0,     0,
     754,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,   757,     0,  1070,     0,     0,     0,   928,     0,     0,
       0,     0,   260,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   180,     0,     0,    84,
      85,     0,    86,    87,     0,    88,   181,    90,   753,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   754,     0,     0,   753,  1060,   753,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   753,     0,   211,     0,     0,     0,     0,   116,
       0,     0,     0,   260,   757,   757,   357,   358,   359,     0,
       0,   757,     0,     0,     0,     0,     0,     0,   260,  1608,
       0,     0,     0,     0,     0,   360,     0,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,     0,   382,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   383,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1149,  1152,  1153,  1154,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
       0,     0,     0,     0,     0,   357,   358,   359,     0,     0,
       0,     0,   753,     0,     0,     0,   753,     0,   753,     0,
       0,   753,     0,     0,   360,  1190,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,     0,   382,     0,
     757,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     383,   757,     0,     0,     0,     0,   757,     0,     0,     0,
     757,     0,     0,     0,     0,     0,     0,     0,   357,   358,
     359,     0,     0,     0,     0,     0,     0,     0,     0,  1081,
       0,     0,     0,     0,  1689,     0,     0,   360,   753,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
       0,   382,     0,     0,     0,     0,     0,     0,  1267,     0,
       0,   757,     0,   383,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1282,     0,  1283,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   357,   358,
     359,   753,   753,     0,     0,     0,     0,     0,   753,     0,
       0,     0,     0,     0,     0,     0,     0,   360,  1106,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
       0,   382,   357,   358,   359,     0,     0,     0,     0,     0,
       0,     0,     0,   383,     0,     0,     0,     0,     0,     0,
       0,   360,  1272,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,     0,   382,     0,     0,     0,     0,
       0,  1422,     0,     0,     0,     0,     0,   383,     0,     0,
       0,   357,   358,   359,     0,     0,     0,     0,     0,     0,
       0,     0,  1372,     0,     0,     0,  1374,     0,  1375,     0,
     360,  1376,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,     0,   382,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   383,   753,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   753,     0,
       0,     0,     0,   753,     0,     0,     0,   753,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1423,     0,     0,     0,     0,     0,     0,  1453,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,  1273,     0,     0,     0,     0,   753,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,  1596,  1597,     0,    41,    42,    43,    44,  1602,    45,
       0,    46,   384,    47,     0,     0,    48,    49,     0,     0,
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
     115,  1033,   116,   117,     0,   118,   119,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,  1657,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,  1667,     0,
       0,     0,     0,  1671,     0,     0,     0,  1673,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,  1706,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,    81,    82,    83,
      84,    85,     0,    86,    87,     0,    88,    89,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,    95,     0,    96,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1203,
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
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,    81,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
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
     115,   585,   116,   117,     0,   118,   119,     5,     6,     7,
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
     112,     0,     0,   113,     0,   114,   115,  1006,   116,   117,
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
       0,   114,   115,  1046,   116,   117,     0,   118,   119,     5,
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
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1112,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,  1114,    45,     0,    46,     0,    47,     0,     0,
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
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,  1268,     0,    48,    49,     0,     0,
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
     112,     0,     0,   113,     0,   114,   115,  1379,   116,   117,
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
       0,   114,   115,  1599,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
    1643,    47,     0,     0,    48,    49,     0,     0,     0,    50,
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
       0,   113,     0,   114,   115,  1677,   116,   117,     0,   118,
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
     115,  1678,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,  1681,    46,     0,    47,
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
       0,   114,   115,  1697,   116,   117,     0,   118,   119,     5,
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
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1752,
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
       0,   113,     0,   114,   115,  1759,   116,   117,     0,   118,
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
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   455,     0,     0,     0,     0,     0,
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
     720,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,     0,   933,     0,     0,     0,
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
      12,     0,  1448,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,  1591,     0,
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
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   656,    12,     0,
       0,     0,     0,     0,     0,   657,     0,     0,     0,     0,
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
      97,    98,   265,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
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
       0,    94,     0,     0,     0,     0,    97,    98,   265,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   266,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   360,    10,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   600,   382,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     383,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
      87,     0,    88,   181,    90,     0,   601,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
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
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,   358,   359,
     715,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   360,    10,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,  1057,
     382,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   383,     0,     0,     0,    16,     0,    17,    18,
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
       0,    86,    87,     0,    88,   181,    90,     0,  1058,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   656,    12,     0,
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
       0,   357,   358,   359,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     360,     0,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,     0,   382,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   383,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,   192,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   176,   177,   178,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   179,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   180,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,   999,  1000,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,   968,   969,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   970,    10,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     992,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,   113,     0,   357,   358,   359,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   360,     0,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,     0,   382,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   383,     0,
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
       0,  1644,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   247,
       0,   359,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   360,     0,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,     0,   382,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   383,     0,    16,     0,    17,    18,
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
     112,     0,     0,   113,     0,   250,     0,   969,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   970,     0,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     992,     0,    16,     0,    17,    18,    19,    20,    21,    22,
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
     453,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   613,   382,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   383,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
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
       0,     0,     0,     0,   970,    10,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   657,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     992,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   699,   382,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   383,     0,     0,     0,
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
       8,     9,     0,     0,     0,     0,     0,    10,     0,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   701,
     382,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   383,     0,     0,     0,    16,     0,    17,    18,
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
       0,     0,     0,    10,     0,     0,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,  1102,   382,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   383,     0,
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
       0,   357,   358,   359,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     360,     0,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,     0,   382,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   383,     0,    16,     0,
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
       0,     0,    92,     0,     0,    93,     0,     0,  1456,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   357,   358,   359,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   360,     0,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,     0,
     382,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   383,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
     549,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   176,
     177,   178,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   179,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   180,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,    92,     0,
       0,    93,     0,  1295,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,  1472,  1473,  1474,  1475,  1476,     0,     0,  1477,  1478,
    1479,  1480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1481,  1482,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,     0,     0,     0,     0,
       0,  1483,     0,     0,     0,     0,     0,     0,     0,   992,
       0,     0,     0,     0,     0,  1484,  1485,  1486,  1487,  1488,
    1489,  1490,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1491,  1492,  1493,  1494,  1495,
    1496,  1497,  1498,  1499,  1500,  1501,    48,  1502,  1503,  1504,
    1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,     0,     0,     0,
    1532,  1533,     0,  1534,  1535,  1536,  1537,  1538,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1539,
    1540,  1541,     0,     0,     0,    86,    87,     0,    88,   181,
      90,  1542,     0,  1543,  1544,     0,  1545,     0,     0,     0,
       0,     0,     0,  1546,  1547,     0,  1548,     0,  1549,  1550,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   357,   358,   359,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   360,     0,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,     0,   382,   357,   358,
     359,     0,     0,     0,     0,     0,     0,     0,     0,   383,
       0,     0,     0,     0,     0,     0,     0,   360,     0,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
       0,   382,   357,   358,   359,     0,     0,     0,     0,     0,
       0,     0,     0,   383,     0,     0,     0,     0,     0,     0,
       0,   360,     0,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,     0,   382,   357,   358,   359,     0,
       0,     0,     0,     0,     0,     0,     0,   383,     0,     0,
       0,     0,     0,     0,     0,   360,     0,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,     0,   382,
     357,   358,   359,     0,     0,     0,     0,     0,     0,     0,
       0,   383,     0,     0,     0,   469,     0,     0,     0,   360,
       0,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,     0,   382,   357,   358,   359,     0,     0,     0,
       0,     0,     0,     0,     0,   383,     0,     0,     0,   471,
       0,     0,     0,   360,     0,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,     0,   382,   967,   968,
     969,     0,     0,     0,     0,     0,     0,     0,     0,   383,
       0,     0,     0,   483,     0,     0,     0,   970,     0,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     357,   358,   359,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   992,     0,     0,     0,   507,   252,   360,
       0,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,     0,   382,   253,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   383,     0,     0,     0,     0,
     690,     0,     0,     0,     0,   252,    36,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,     0,   382,    48,     0,     0,
       0,   253,     0,     0,     0,     0,     0,     0,   383,     0,
       0,     0,     0,     0,   712,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,   254,   255,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
     180,     0,     0,    84,   256,     0,    86,    87,  1146,    88,
     181,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   257,     0,     0,     0,     0,   254,
     255,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,  1003,   252,   180,   258,     0,
      84,   256,  1571,    86,    87,     0,    88,   181,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   257,   253,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   252,     0,    36,   258,     0,     0,     0,  1639,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,   253,     0,
       0,     0,     0,  -335,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   176,   177,   347,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     254,   255,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,   252,     0,     0,   180,   476,
       0,    84,   256,     0,    86,    87,     0,    88,   181,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   253,   257,     0,     0,     0,   254,   255,   348,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,   180,     0,   258,    84,   256,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,     0,     0,   252,    48,     0,     0,     0,   257,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   253,
       0,     0,   258,     0,     0,     0,     0,     0,     0,   254,
     255,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,   180,     0,     0,
      84,   256,     0,    86,    87,     0,    88,   181,    90,     0,
     948,     0,    48,     0,     0,     0,   252,     0,     0,     0,
       0,   257,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   253,     0,     0,   258,     0,   254,   255,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   252,     0,    36,   180,     0,     0,    84,   256,
       0,    86,    87,     0,    88,   181,    90,     0,  1279,     0,
       0,     0,     0,     0,     0,    48,     0,     0,   253,   257,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,     0,     0,   258,     0,     0,     0,     0,     0,     0,
     254,   255,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,   180,     0,
       0,    84,   256,     0,    86,    87,     0,    88,   181,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,  1383,     0,     0,   254,   255,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,   180,   209,   258,    84,   256,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   257,     0,
       0,     0,     0,  1155,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   741,
     742,   644,   258,     0,     0,   743,     0,   744,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   745,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,     0,     0,    86,    87,     0,    88,   181,    90,   746,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   733,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,     0,   645,     0,
     116,     0,     0,     0,     0,     0,   747,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   748,
      36,     0,     0,     0,   180,    82,    83,    84,   749,   734,
      86,    87,     0,    88,   181,    90,     0,     0,     0,    92,
       0,    48,     0,     0,     0,     0,     0,     0,   750,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   751,     0,     0,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,    36,   180,   209,     0,    84,     0,     0,
      86,    87,     0,    88,   181,    90,   992,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   741,
     742,     0,     0,     0,     0,   743,     0,   744,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   745,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,   674,     0,    86,    87,     0,    88,   181,    90,   746,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,     0,   675,     0,
     116,     0,     0,     0,     0,     0,   747,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   748,
       0,     0,     0,     0,   180,    82,    83,    84,   749,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,   750,   894,
     895,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   896,
       0,     0,   751,     0,     0,     0,     0,   897,   898,   899,
      36,     0,     0,     0,     0,     0,     0,     0,     0,   900,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,    29,
      30,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,    38,   992,     0,     0,     0,   901,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   902,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
      86,    87,     0,    88,   181,    90,    60,    61,    62,   176,
     177,   178,     0,    29,    30,     0,     0,     0,   903,     0,
       0,     0,     0,    36,     0,   209,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,     0,   180,    48,     0,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,    93,     0,     0,     0,   210,     0,     0,     0,     0,
       0,    36,     0,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   435,    48,     0,     0,     0,   116,   180,     0,     0,
      84,    85,     0,    86,    87,     0,    88,   181,    90,     0,
       0,     0,   883,     0,     0,    93,     0,     0,     0,     0,
       0,    36,     0,   209,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    48,     0,     0,   435,     0,     0,   307,     0,
     116,    86,    87,     0,    88,   181,    90,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   209,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,    48,     0,   180,   308,     0,    84,    85,
       0,    86,    87,     0,    88,   181,    90,     0,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    48,     0,   211,     0,     0,   180,     0,   116,    84,
      85,     0,    86,    87,     0,    88,   181,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   209,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    48,     0,     0,   211,     0,     0,   520,  1556,   116,
      86,    87,  1557,    88,   181,    90,     0,     0,     0,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,   540,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      48,     0,  1404,     0,   180,     0,     0,    84,    85,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1403,     0,     0,    36,     0,
     209,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    48,
       0,     0,   211,     0,     0,     0,     0,   116,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   209,     0,  1028,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
      48,  1404,   180,     0,     0,    84,    85,     0,    86,    87,
       0,    88,   181,    90,     0,     0,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   209,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     211,    48,     0,   180,     0,   116,    84,    85,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   211,     0,     0,   180,     0,   116,    84,    85,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1084,  1085,  1086,    36,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    36,
      48,   209,   224,     0,     0,     0,     0,   116,   563,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,  -926,  -926,  -926,  -926,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,     0,   382,     0,
      36,   210,   209,     0,     0,     0,     0,     0,     0,     0,
     383,     0,     0,     0,     0,     0,     0,     0,     0,    86,
      87,    48,    88,   181,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   180,     0,     0,    84,    85,     0,    86,
      87,     0,    88,   181,    90,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,     0,
     209,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   674,    48,
      86,    87,     0,    88,   181,    90,   564,     0,     0,     0,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   992,
       0,     0,     0,     0,     0,   707,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,    87,
       0,    88,   181,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   357,   358,   359,
       0,     0,     0,   645,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   360,     0,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,     0,
     382,   357,   358,   359,     0,     0,     0,     0,     0,     0,
       0,     0,   383,     0,     0,     0,     0,     0,     0,     0,
     360,     0,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,     0,   382,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   383,     0,   357,   358,
     359,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   360,   431,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
       0,   382,   357,   358,   359,     0,     0,     0,     0,     0,
       0,     0,     0,   383,     0,     0,     0,     0,     0,     0,
       0,   360,   441,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,     0,   382,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   383,     0,   357,
     358,   359,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   360,   869,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,     0,   382,   967,   968,   969,     0,     0,     0,     0,
       0,     0,     0,     0,   383,     0,     0,     0,     0,     0,
       0,     0,   970,   913,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   992,     0,
     967,   968,   969,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   970,
    1232,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,     0,     0,   967,   968,   969,     0,     0,     0,
       0,     0,     0,     0,     0,   992,     0,     0,     0,     0,
       0,     0,     0,   970,  1137,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   992,
       0,   967,   968,   969,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
     970,  1289,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   992,     0,    36,     0,
       0,     0,     0,     0,    48,  1371,     0,     0,     0,     0,
       0,   180,     0,     0,    84,    85,     0,    86,    87,    48,
      88,   181,    90,     0,     0,     0,  1390,   274,   275,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1391,
    1392,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,   180,   813,   814,
      84,    85,  1455,    86,    87,     0,    88,  1394,    90,     0,
       0,     0,     0,     0,     0,   276,     0,    48,    86,    87,
       0,    88,   181,    90,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    36,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    36,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,    87,    48,    88,
     181,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,     0,     0,     0,
     344,    48,    86,    87,     0,    88,   181,    90,     0,     0,
       0,     0,     0,     0,   508,     0,    48,    86,    87,     0,
      88,   181,    90,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   512,     0,     0,
      86,    87,     0,    88,   181,    90,     0,     0,     0,     0,
       0,     0,   276,     0,     0,    86,    87,     0,    88,   181,
      90,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    36,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    36,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   644,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,  1147,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,    86,
      87,     0,    88,   181,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,    87,     0,    88,   181,    90,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    86,    87,     0,    88,   181,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   357,   358,   359,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   724,   360,     0,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,     0,   382,     0,   357,
     358,   359,     0,     0,     0,     0,     0,     0,     0,   383,
       0,     0,     0,     0,     0,     0,     0,     0,   360,   866,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   725,   382,   357,   358,   359,     0,     0,     0,     0,
       0,     0,     0,     0,   383,     0,     0,     0,     0,     0,
       0,     0,   360,     0,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,     0,   382,   967,   968,   969,
       0,     0,     0,     0,     0,     0,     0,     0,   383,     0,
       0,     0,     0,     0,     0,     0,   970,  1294,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   967,
     968,   969,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   992,     0,     0,     0,     0,     0,   970,     0,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   992,  -926,  -926,  -926,  -926,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   992
};

static const yytype_int16 yycheck[] =
{
       5,     6,   603,     8,     9,    10,    11,    12,   160,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     4,   135,    28,    29,     4,   183,     4,     4,    91,
    1049,   843,   575,    95,    96,    54,   692,    42,   225,     4,
     221,    32,   315,    55,   425,    50,   398,    52,   172,   653,
      55,   382,    57,    44,   574,   832,   723,   451,    49,   163,
     556,   419,   420,   160,   451,   113,   862,   113,  1037,   231,
       9,   184,   134,   926,    79,   113,     9,   730,     9,     9,
      30,    14,   878,     9,    30,     9,     9,     9,    45,    79,
      45,   449,     9,    45,     9,    58,     4,     9,     9,    66,
       9,    35,     9,   110,     9,    30,    66,     9,   113,     9,
       9,    79,     9,     9,    50,  1045,     9,    79,    81,     9,
      79,    84,    50,     9,     4,     9,     9,     9,   232,   129,
     130,     9,    85,   150,    79,    79,    33,    45,   300,    79,
      35,    79,   129,   130,     9,    79,  1585,   129,   130,   171,
     154,    45,    79,   455,   171,     0,    97,    45,   150,   129,
     130,   168,    53,   211,   118,   211,   199,    85,    97,   129,
     130,   125,    46,    47,    65,   156,   224,   199,    35,   171,
     127,   128,   199,   188,    79,   202,   963,     8,   150,   148,
     149,   315,   349,    66,   147,    66,    42,   201,   202,   107,
     200,  1640,   504,    66,   112,    66,   114,   115,   116,   117,
     118,   119,   120,   154,   204,    66,   203,    35,   200,   155,
     164,   202,    79,    66,   164,   154,   164,   155,   233,   147,
     164,   236,   199,    79,   170,   202,   204,   164,   243,   244,
     197,   172,   202,   200,   292,   200,   292,    30,   200,   157,
     158,   201,   160,   434,   292,   172,   202,    66,     4,   204,
    1229,    79,   201,   202,   199,   389,  1119,  1236,   201,  1238,
     427,   201,   237,   181,   576,   201,   241,   201,   201,   201,
     160,   343,  1078,   936,   397,   938,   201,   292,   172,   201,
     201,   415,   201,   298,   201,   203,   201,  1227,   303,   201,
     155,   201,   201,   200,   200,    51,   200,   200,    54,   197,
     200,   274,   275,   276,   200,   439,   859,   200,   200,    66,
    1097,   340,   200,   328,   448,    71,   822,   451,   199,   202,
     392,   393,   394,   395,   339,   200,    66,    66,   321,   202,
      66,   202,    66,    89,   307,    91,   164,    66,   199,    95,
      96,   202,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,    66,   383,  1348,
     385,   386,   199,   388,   199,   566,   398,   435,   134,   435,
     199,   200,    97,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,    66,  1337,    66,  1339,
      66,   416,   417,   715,   419,   420,   421,   422,   720,   581,
     150,    35,    97,   428,    35,   150,   431,   199,   809,   101,
     102,  1035,   101,   102,    79,    97,   441,   199,   443,   591,
      85,    79,  1098,   171,   449,   457,   171,   327,  1115,   154,
     196,   202,   457,   847,   459,   202,   199,   382,   199,   646,
     847,    66,   643,   199,   389,    79,     4,   199,    79,    79,
     154,   199,   202,   202,    35,    85,   202,   202,   202,   154,
     202,   486,     4,   202,   489,   490,   491,   199,    26,    27,
     415,   237,   154,   680,   591,   241,    14,   462,   655,   245,
      79,   199,    79,   148,   149,  1435,    85,     4,    85,  1286,
      97,   698,    30,   202,   439,   520,    79,   875,   264,  1062,
     914,   199,  1065,   448,   129,   130,   451,   914,  1048,    47,
     156,   203,    71,    72,   203,   697,   199,   147,   148,   149,
     171,   703,   202,   169,   202,   508,   202,   609,    45,   512,
     164,   201,   883,   164,   517,    79,   737,    29,   196,   832,
      79,    85,   201,   208,   202,   752,    85,   154,   199,   148,
     149,   148,   149,   760,    46,   113,   202,    49,   171,  1356,
     201,   327,    71,    72,   147,   148,   149,    79,   201,  1245,
     336,  1247,   575,    85,   340,   201,   601,   343,   201,   703,
     114,   115,   116,   117,   118,   119,   199,    66,   613,    66,
     107,    97,    98,   135,    66,   112,   201,   114,   115,   116,
     117,   118,   119,   120,   148,   149,   106,   202,   207,   148,
     149,   933,   199,  1237,   114,   115,   116,   117,   118,   119,
     645,   386,   117,   389,   390,   391,   392,   393,   394,   395,
     125,   656,    49,    50,    51,   147,   148,   149,  1588,   150,
     157,   158,   184,   160,   199,    29,    66,   181,    65,   415,
     675,   416,   150,   211,   201,   202,   421,   154,  1030,   199,
     218,    44,    46,    65,   181,    49,   224,   201,  1231,   201,
     963,   201,   202,   439,  1615,  1616,  1216,   171,  1354,   237,
     150,   181,   707,   241,  1723,   451,   203,   206,   832,  1611,
    1612,   591,   199,  1071,  1318,     9,   462,   150,   723,  1738,
     100,   101,   102,   847,  1082,   739,   740,   908,   719,   100,
     101,   102,   150,   479,    49,    50,    51,  1118,    53,   114,
     115,   116,     8,   281,   114,   115,   116,   117,   118,   119,
      65,   199,   290,   291,   292,   125,   126,   762,   201,   297,
     117,   118,   119,   728,   301,   171,   304,   199,   305,   150,
      14,   952,   150,   519,    79,   201,   201,   125,   959,   125,
      14,   200,   171,    14,  1388,    97,  1442,  1717,   200,   327,
     914,   200,   329,   163,   331,   332,   333,   334,   200,   545,
     546,   205,  1732,   199,   106,   327,  1349,   199,   199,     9,
     200,   181,   200,   818,    89,     9,   201,  1198,   781,    14,
     882,     9,   785,   199,  1097,   185,   831,    46,    47,    48,
      49,    50,    51,   713,    53,   188,    79,    79,   821,   963,
      79,   199,   821,     9,   821,   821,    65,   201,     9,   201,
      79,   856,   201,    74,    75,    76,   821,   200,   200,   200,
     843,   866,   127,   609,   869,    86,   871,   199,  1055,   200,
     875,   182,   183,   184,   396,  1233,   859,    66,   189,   190,
    1692,    30,   193,   194,   170,  1187,   131,   425,   128,     9,
     114,   115,   116,   117,   118,   119,   776,   435,  1710,   200,
     150,   125,   126,    14,   197,     9,  1718,    66,   913,     9,
     172,  1092,   200,     9,   135,   136,   137,   138,   139,    14,
     127,   206,   847,   206,   462,   146,   203,     9,   919,   199,
      14,   152,   153,   206,  1590,   206,   200,   161,   199,   163,
     200,   821,   200,    97,   201,   166,   692,   201,   694,    86,
     131,   150,   176,     9,   178,   920,  1258,   181,   883,   180,
     150,   841,   200,  1144,   199,   150,   199,   713,   185,   185,
    1151,    14,  1274,  1097,   202,     9,    79,  1335,    14,   202,
     202,   727,   728,   114,   115,   116,   117,   118,   119,   914,
     201,   996,   997,   998,    14,    14,   206,  1002,  1003,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    30,  1286,   197,   202,    30,   201,  1030,    46,
      47,    48,    49,    50,    51,  1030,   199,     4,   199,    14,
     776,   199,    14,  1634,   199,    48,   782,   199,    65,     9,
     786,   787,   922,   200,   924,   201,   201,   199,    63,    64,
     181,  1034,   131,  1058,    14,  1034,  1358,  1034,  1034,   131,
     806,     9,   600,    65,   200,  1367,  1071,   206,    45,  1034,
       9,   147,    79,  1254,     9,   821,  1378,  1082,  1083,  1062,
     199,   131,  1065,  1356,   201,    14,    79,   200,   202,   835,
     199,   199,   202,   200,   131,   841,   202,     9,    86,   201,
     147,   847,    30,   201,    73,   206,   200,  1708,   201,   172,
    1115,    30,   200,   200,   129,   130,  1107,     9,   203,   657,
    1125,   131,   131,   200,     9,   200,    14,    79,   201,   203,
     107,   202,   199,   202,   200,   112,   882,   114,   115,   116,
     117,   118,   119,   120,   199,  1110,  1448,   200,   894,   895,
     896,   200,   200,   131,  1034,   200,   200,     9,    30,   201,
     200,   699,  1286,   701,   202,   200,   107,  1319,   914,   201,
     201,   201,   159,    14,   920,   713,   922,    79,   924,   155,
     157,   158,   112,   160,   200,   200,   131,   725,   200,   200,
     728,   713,    14,   131,   202,   201,   171,   202,   944,    79,
      14,  1314,    14,  1208,   181,    79,   200,  1212,   199,  1214,
     202,   200,    14,  1196,   131,   201,   962,  1222,   201,   965,
      14,   201,  1205,    14,     9,    55,   203,  1232,  1233,   202,
      79,   203,  1356,   199,    79,     9,   201,    79,   776,   110,
      97,   150,  1122,    97,   162,    33,    14,   993,  1231,   199,
     201,   200,  1409,   199,   776,   168,   794,   172,    79,   165,
     200,     9,    79,   200,  1010,   201,   200,  1013,    14,   202,
      79,   809,   810,    14,    79,    14,    79,    14,  1580,    79,
    1582,   517,   785,   821,  1303,   781,  1700,   392,  1034,  1591,
    1295,   881,   876,     4,   824,  1714,   395,  1116,   390,   821,
    1447,  1710,  1271,   841,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1196,    53,   522,   841,
    1438,  1383,  1470,  1389,  1554,  1205,  1742,  1332,  1730,    65,
    1335,  1309,  1566,  1305,    45,  1637,  1434,   398,  1005,   773,
     493,  1002,   493,  1038,   960,  1080,   895,   847,  1094,  1093,
     910,   298,  1098,   739,  1094,  1338,  1298,  1021,   993,   291,
      -1,  1344,   945,  1346,  1110,    -1,  1349,    -1,  1387,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1122,    -1,    -1,    -1,
      -1,    -1,   920,    -1,   922,    -1,   924,    -1,   926,   927,
      -1,    -1,  1411,    -1,  1413,    -1,   107,    -1,    -1,  1364,
     922,   112,   924,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,  1570,    -1,    -1,    -1,  1297,    -1,    -1,
      -1,  1426,    -1,    -1,    -1,  1606,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,  1736,    -1,    -1,    -1,    -1,  1319,
      -1,  1743,    -1,    -1,    -1,  1191,   157,   158,    -1,   160,
      -1,  1456,    -1,    -1,    -1,    -1,    -1,  1440,  1338,    -1,
      -1,    -1,    -1,    -1,  1344,    -1,  1346,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    45,    -1,    26,    27,    -1,  1359,
      30,  1446,  1447,    -1,    -1,    -1,    -1,    -1,  1368,    -1,
      -1,    -1,   203,    -1,    -1,    -1,  1034,    -1,    -1,  1245,
      -1,  1247,    -1,  1565,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1034,    -1,    -1,    -1,    -1,    -1,    -1,  1057,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1687,   107,    -1,    -1,    -1,
      -1,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,  1297,    -1,    -1,    -1,    -1,    -1,  1303,    -1,    -1,
    1440,    -1,    -1,  1309,  1102,  1445,  1628,    -1,    -1,    -1,
    1575,  1451,  1110,    -1,    -1,    -1,    -1,  1457,    -1,    -1,
    1118,  1119,    -1,    -1,  1122,    -1,   157,   158,    -1,   160,
      -1,  1704,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1354,    -1,
     181,    -1,    -1,  1359,    -1,    -1,    -1,    -1,  1364,    -1,
      -1,    -1,  1368,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,  1383,    -1,    -1,
      -1,  1387,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1397,    -1,    -1,    -1,    -1,    -1,    -1,  1404,    -1,
    1198,   211,    -1,    -1,    -1,  1411,    -1,  1413,   218,    -1,
      -1,    -1,    -1,  1419,   224,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   252,   253,    -1,    -1,    -1,    -1,   258,
      -1,    -1,    -1,    -1,    -1,    -1,  1442,    -1,    -1,  1445,
    1446,  1447,   252,   253,    -1,  1451,    -1,    -1,   258,  1692,
      -1,  1457,    -1,    -1,    -1,  1595,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1710,    -1,    -1,
      -1,   281,    -1,    -1,    -1,  1718,    -1,    -1,    -1,    -1,
     290,   291,    -1,  1748,    -1,    -1,    -1,   297,    -1,    -1,
      -1,  1756,    -1,    -1,   304,  1635,  1636,  1762,    -1,  1297,
    1765,    -1,  1642,    -1,    -1,   315,    -1,    -1,   337,    -1,
      -1,    -1,    -1,    26,    27,  1297,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   337,    -1,    -1,
     340,  1313,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1679,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1687,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1565,
      -1,  1359,    -1,    -1,    -1,    -1,  1364,    -1,    -1,    -1,
    1368,    -1,   382,     4,    -1,    -1,    -1,  1359,    -1,    -1,
    1586,    -1,    -1,    -1,  1590,    -1,  1368,    -1,    -1,  1595,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1604,    -1,
      -1,    -1,    -1,    -1,  1744,  1611,  1612,    -1,    -1,  1615,
    1616,  1751,    -1,    -1,    45,   425,    -1,    -1,    -1,    -1,
      -1,    -1,  1628,    -1,    -1,   435,    -1,    -1,    -1,  1635,
    1636,    -1,    -1,    -1,    -1,  1417,  1642,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   474,    -1,  1445,  1446,  1447,
      -1,    -1,    -1,  1451,    -1,    -1,    -1,    -1,    -1,  1457,
      -1,    -1,    -1,  1445,   474,   475,    -1,    -1,   478,  1451,
      -1,    -1,    -1,  1679,    -1,  1457,   107,    -1,    -1,    -1,
    1686,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1703,   211,    -1,
      -1,    -1,    -1,    -1,    -1,   218,    -1,    -1,    -1,    -1,
      -1,   224,    -1,    -1,    -1,    -1,   526,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,    -1,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1744,    -1,
      -1,    -1,   571,   572,    -1,  1751,    -1,    -1,    -1,    -1,
     181,   580,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   571,   572,    -1,    -1,    -1,    -1,    -1,   281,    -1,
     580,    -1,   203,    -1,    -1,    -1,    -1,   290,   291,    -1,
      -1,    -1,    -1,    -1,   297,    -1,    -1,    -1,    -1,    -1,
     600,   304,    -1,    10,    11,    12,    -1,  1595,    -1,    -1,
      -1,    -1,   315,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,  1595,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,  1635,  1636,    -1,
      -1,    -1,    -1,    -1,  1642,    -1,    -1,   657,    65,    -1,
      -1,    -1,    -1,  1635,  1636,    -1,    -1,    -1,    -1,    -1,
    1642,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,   382,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1679,    -1,    -1,    -1,    98,    -1,    -1,    -1,   699,
      -1,   701,    -1,    -1,    -1,    -1,    -1,  1679,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     739,   740,   425,    -1,    -1,   725,   726,    -1,    -1,    -1,
      -1,    -1,   435,    65,    -1,    -1,    -1,    -1,    -1,   739,
     740,   741,   742,   743,   744,   745,    -1,    -1,    -1,    29,
      -1,   751,    -1,    -1,   157,   158,  1744,   160,   161,   162,
      -1,    -1,    -1,  1751,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1744,    -1,    -1,    -1,    -1,   777,    -1,  1751,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,   794,    -1,   199,    77,    -1,   206,
      -1,    -1,    -1,    -1,   823,    -1,    -1,   807,    -1,   809,
     810,    26,    27,    -1,    -1,    30,    -1,    -1,    98,   838,
      -1,    -1,    -1,   823,   824,    -1,    -1,    -1,    -1,    -1,
      -1,   850,   832,    -1,    -1,    -1,    -1,    -1,   838,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     850,    -1,    -1,   133,   134,    -1,    -1,    -1,   858,    -1,
     879,   861,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,   879,
     160,   161,   162,   883,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   600,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      -1,   940,    -1,    -1,   943,    -1,   926,   927,    77,    -1,
      -1,    -1,    -1,    63,    64,    -1,    -1,    -1,    -1,    -1,
     940,    -1,    -1,   943,    -1,   945,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,   657,    -1,    -1,    -1,    -1,    -1,
     960,    -1,    -1,   963,    -1,    -1,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,    -1,    -1,    -1,   699,    -1,   701,   129,
     130,    -1,   151,   218,    -1,   154,    -1,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,  1018,    -1,
      -1,    -1,   725,    -1,    -1,    -1,    -1,    -1,  1047,    -1,
    1049,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,  1047,    -1,  1049,
      -1,    -1,    -1,    -1,   203,  1074,    -1,  1057,  1077,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,
     200,    -1,    -1,    -1,  1074,   290,   291,  1077,    -1,    -1,
      -1,    -1,   297,    -1,    -1,    -1,    -1,    -1,    -1,   304,
      -1,   794,    -1,    -1,    -1,    -1,    -1,  1097,    -1,    -1,
     315,    -1,  1102,    -1,    -1,    -1,   809,   810,    -1,    -1,
      -1,  1130,    -1,    -1,    -1,  1134,    -1,    -1,  1118,  1119,
      -1,  1121,    -1,    -1,    54,    -1,    -1,    -1,    -1,   832,
    1130,    -1,    -1,    -1,  1134,    -1,    -1,  1137,    -1,  1139,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,  1155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   382,    26,    27,
      -1,    -1,    -1,  1192,  1193,    -1,    -1,    -1,    -1,    -1,
     883,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    -1,  1192,  1193,    -1,  1195,    -1,    -1,  1198,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
     425,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   926,   927,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,  1278,
     963,  1280,    65,   478,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1278,    -1,
    1280,    -1,    -1,    -1,    -1,  1285,  1286,    -1,    -1,  1289,
      -1,  1291,    -1,    -1,  1294,  1314,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1303,  1304,    -1,    -1,  1307,    10,    11,
      12,   526,    -1,    -1,  1314,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   252,   253,    -1,    -1,    -1,    29,   258,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,   211,  1057,    -1,  1356,    -1,    -1,    -1,
     218,    -1,    -1,    65,    -1,    -1,   224,    -1,    -1,    -1,
      -1,  1371,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1381,  1382,    -1,    -1,   600,    -1,  1387,    -1,  1389,
      -1,    -1,    -1,    -1,  1097,    -1,    -1,    -1,    -1,  1102,
     203,    -1,    -1,    -1,    -1,    -1,    -1,   337,    -1,    -1,
     340,  1411,    -1,  1413,    -1,  1118,  1119,    -1,    -1,  1419,
      -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   290,   291,   292,    -1,    -1,    -1,    -1,   297,
      -1,    -1,   657,    -1,    -1,    -1,   304,    -1,    -1,  1468,
      -1,    -1,    -1,    -1,  1454,  1455,    -1,    -1,    -1,    -1,
      -1,  1461,  1462,    -1,    -1,    -1,    -1,    -1,  1468,    -1,
    1470,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   699,    -1,   701,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1198,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     725,   726,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   741,   742,   743,   744,
     745,    -1,    -1,    -1,    -1,    -1,   751,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   474,   475,    -1,    -1,   478,    -1,
    1569,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   777,    -1,    77,    -1,    -1,   425,    -1,  1569,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   435,    -1,   794,
      -1,    -1,    -1,  1286,    -1,    98,  1586,    -1,    -1,    -1,
      -1,    -1,   807,    -1,   809,   810,   526,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1604,    -1,    -1,    -1,    -1,   824,
    1610,    -1,    -1,    -1,    -1,    -1,    -1,   832,    -1,    -1,
      -1,  1621,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,
    1630,    -1,    -1,  1652,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   571,   572,   858,   157,   158,   861,   160,   161,   162,
     580,    -1,  1652,  1356,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   883,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,  1691,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1699,
      -1,    -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,    -1,
      -1,   926,   927,    -1,  1714,    -1,    -1,    -1,    -1,  1738,
      -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1738,    -1,
      -1,    -1,   600,    -1,    -1,   960,    -1,    -1,   963,    -1,
      -1,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,   988,   989,   990,   991,   992,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,   726,    -1,    -1,   657,
      -1,    -1,    -1,  1018,    -1,    -1,    -1,    -1,    -1,   739,
     740,   741,   742,   743,   744,   745,    -1,    -1,   478,    -1,
      -1,   751,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   699,  1057,   701,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,   526,   725,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,  1097,    -1,    -1,    -1,    -1,  1102,    -1,    -1,
      -1,    -1,    -1,   823,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1118,  1119,    -1,  1121,    -1,   838,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     850,    -1,  1137,    -1,  1139,    -1,    -1,    -1,   858,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   794,    -1,    -1,    -1,
    1155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   879,
      -1,   809,   810,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1195,    -1,    -1,  1198,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
     940,    -1,    -1,   943,    -1,   945,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     960,    -1,    -1,    -1,    -1,    -1,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,    -1,    -1,    -1,   726,    -1,   926,   927,
    1285,  1286,    -1,    -1,  1289,    -1,  1291,    -1,    -1,  1294,
      -1,   741,   742,   743,   744,   745,    -1,    -1,  1018,  1304,
      -1,   751,  1307,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,   478,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1047,    29,  1049,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1356,    53,    -1,  1074,    -1,    -1,  1077,    -1,    -1,
      -1,    -1,    -1,   526,    65,    -1,  1371,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1381,  1382,    -1,    -1,
      -1,    -1,    -1,    -1,  1389,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1121,    -1,    -1,    -1,    -1,    -1,    -1,   858,  1057,
    1130,    -1,    -1,    -1,  1134,    -1,    -1,  1137,    -1,  1139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1454,
    1455,    -1,    -1,    -1,  1102,    -1,  1461,  1462,    -1,    -1,
      10,    11,    12,    -1,    -1,  1470,    -1,    -1,    -1,    -1,
    1118,  1119,  1192,  1193,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   203,    53,    -1,    -1,    -1,    -1,    -1,    -1,
     960,    -1,    -1,    -1,    -1,    65,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1278,    -1,
    1280,    -1,    -1,   726,    -1,  1285,    -1,    -1,  1018,  1289,
      -1,  1291,    -1,    -1,  1294,    -1,    -1,    -1,   741,   742,
     743,   744,   745,  1303,    -1,    -1,    -1,    -1,   751,    -1,
      -1,    -1,    -1,    -1,  1314,   478,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1610,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1621,    -1,    -1,    -1,
      -1,  1626,    -1,    -1,    -1,  1630,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   526,    53,    -1,    -1,    -1,    -1,    -1,
      -1,  1371,    -1,   203,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1387,    -1,    -1,
      -1,  1121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1691,  1137,    -1,  1139,
      -1,  1411,    -1,  1413,  1699,   858,    -1,    -1,    -1,  1419,
      -1,    -1,    -1,    -1,    -1,  1155,    -1,    -1,    -1,  1714,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,  1454,  1455,    -1,    -1,    -1,    -1,
      -1,  1461,    -1,    65,    -1,    -1,    -1,    -1,  1468,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   960,    -1,    -1,
      -1,    -1,    -1,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
      35,    -1,    -1,    -1,    -1,  1285,    -1,    -1,    -1,  1289,
      -1,  1291,    -1,   726,  1294,    -1,    -1,    -1,    -1,  1569,
      -1,    -1,    -1,    -1,    -1,  1018,    -1,    -1,   741,   742,
     743,   744,    -1,    -1,    -1,    -1,  1586,    -1,   751,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1604,    -1,    -1,    -1,    -1,    -1,
    1610,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1621,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,
    1630,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,  1371,    -1,   203,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,  1652,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,  1121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1691,    -1,    -1,  1137,   858,  1139,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,  1155,    -1,   199,    -1,    -1,    -1,    -1,   204,
      -1,    -1,    -1,  1723,  1454,  1455,    10,    11,    12,    -1,
      -1,  1461,    -1,    -1,    -1,    -1,    -1,    -1,  1738,  1469,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,  1285,    -1,    -1,    -1,  1289,    -1,  1291,    -1,
      -1,  1294,    -1,    -1,    29,  1018,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
    1610,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,  1621,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,
    1630,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,  1654,    -1,    -1,    29,  1371,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,  1121,    -1,
      -1,  1691,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1137,    -1,  1139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,  1454,  1455,    -1,    -1,    -1,    -1,    -1,  1461,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   203,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1285,    -1,    -1,    -1,  1289,    -1,  1291,    -1,
      29,  1294,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,  1610,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1621,    -1,
      -1,    -1,    -1,  1626,    -1,    -1,    -1,  1630,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,  1371,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,  1691,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,  1454,  1455,    -1,    86,    87,    88,    89,  1461,    91,
      -1,    93,   201,    95,    -1,    -1,    98,    99,    -1,    -1,
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
     202,   203,   204,   205,    -1,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1610,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,  1621,    -1,
      -1,    -1,    -1,  1626,    -1,    -1,    -1,  1630,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,  1691,   103,
     104,   105,   106,   107,   108,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,   176,    -1,   178,    -1,   180,   181,    -1,   183,
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
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,
     178,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,   207,
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
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,
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
      -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    96,    -1,    98,    99,    -1,    -1,
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
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      94,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
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
      86,    87,    88,    89,    -1,    91,    92,    93,    -1,    95,
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
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
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
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
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
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
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
     158,    -1,   160,   161,   162,    -1,   164,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
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
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    11,    12,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,    10,    11,    12,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   103,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,   191,   192,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    11,    12,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
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
      -1,   188,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
      -1,    12,   204,   205,    -1,   207,   208,     3,     4,     5,
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
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,    -1,    12,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
     200,    -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    30,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
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
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
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
      -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,
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
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    32,
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
      -1,    -1,    -1,    13,    -1,    -1,    33,    34,    35,    36,
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
      -1,    10,    11,    12,   204,   205,    -1,   207,   208,     3,
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
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,   187,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    10,    11,    12,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
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
      -1,   169,    -1,   186,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
     152,   153,    -1,    -1,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,    -1,    -1,    -1,
      -1,    -1,    -1,   175,   176,    -1,   178,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
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
     200,    -1,    -1,    -1,    -1,    29,    77,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    98,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,   200,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   133,
     134,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   195,    29,   151,   199,    -1,
     154,   155,   203,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   175,    55,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    29,    -1,    77,   199,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    55,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    29,    -1,    -1,   151,   106,
      -1,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,   175,    -1,    -1,    -1,   133,   134,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    77,   151,    -1,   199,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    98,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    55,
      -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
     164,    -1,    98,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    55,    -1,    -1,   199,    -1,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    77,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,   164,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    55,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   175,   176,    -1,    -1,   133,   134,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    77,   151,    79,   199,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    30,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    46,
      47,   125,   199,    -1,    -1,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,   157,   158,    -1,   160,   161,   162,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    29,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      77,    -1,    -1,    -1,   151,   152,   153,   154,   155,    86,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,   199,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    77,   151,    79,    -1,   154,    -1,    -1,
     157,   158,    -1,   160,   161,   162,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,   155,    -1,   157,   158,    -1,   160,   161,   162,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,    46,
      47,    -1,    -1,   180,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    66,
      -1,    -1,   199,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    67,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,    65,    -1,    -1,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
     157,   158,    -1,   160,   161,   162,   114,   115,   116,   117,
     118,   119,    -1,    67,    68,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    77,    -1,    79,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,    -1,   151,    98,    -1,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    77,    -1,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    98,    -1,    -1,    -1,   204,   151,    -1,    -1,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    68,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    98,    -1,    -1,   199,    -1,    -1,   154,    -1,
     204,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,    98,    -1,   151,   202,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    98,    -1,   199,    -1,    -1,   151,    -1,   204,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    98,    -1,    -1,   199,    -1,    -1,   202,   155,   204,
     157,   158,   159,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,   132,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      98,    -1,   199,    -1,   151,    -1,    -1,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    98,
      -1,    -1,   199,    -1,    -1,    -1,    -1,   204,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,    -1,   132,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
      98,   199,   151,    -1,    -1,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    -1,
     199,    98,    -1,   151,    -1,   204,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
      -1,   199,    -1,    -1,   151,    -1,   204,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    77,
      98,    79,   199,    -1,    -1,    -1,    -1,   204,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      77,   119,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,    98,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    77,    -1,
      79,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   155,    98,
     157,   158,    -1,   160,   161,   162,   204,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    65,
      -1,    -1,    -1,    -1,    -1,   202,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    10,    11,    12,
      -1,    -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
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
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     131,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   131,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    65,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      29,   131,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    98,   131,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,    98,
     160,   161,   162,    -1,    -1,    -1,   120,   106,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,   151,    79,    80,
     154,   155,   131,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    98,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    77,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    77,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,    98,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    77,    -1,    -1,    -1,    -1,
     155,    98,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    98,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,   154,    -1,    -1,
     157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    77,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    77,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    28,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    97,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
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
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65
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
     119,   199,   224,   400,   401,   438,   441,   438,    35,   345,
     453,   454,   345,   119,   199,   224,   400,   401,   402,   437,
     445,   450,   451,   199,   336,   403,   199,   336,   352,   337,
     345,   233,   336,   199,   199,   199,   336,   201,   345,   216,
     201,   345,    29,    55,   133,   134,   155,   175,   199,   216,
     227,   467,   480,   481,   483,   182,   201,   342,   345,   369,
     371,   202,   240,   345,   106,   107,   154,   217,   220,   223,
      79,   204,   292,   293,   118,   125,   117,   125,    79,   294,
     199,   199,   199,   199,   216,   264,   468,   199,   199,   337,
      79,    85,   147,   148,   149,   459,   460,   154,   202,   223,
     223,   216,   265,   468,   155,   199,   468,   468,    79,   196,
     202,   354,   335,   345,   346,   438,   442,   229,   202,    85,
     404,   459,    85,   459,   459,    30,   154,   171,   469,   199,
       9,   201,    35,   246,   155,   263,   468,   119,   181,   247,
     327,   201,   201,   201,   201,   201,   201,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    53,    65,   201,    66,    66,   201,   202,   150,
     126,   161,   163,   176,   178,   266,   325,   326,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    63,    64,   129,   130,   430,    66,   202,   435,   199,
     199,    66,   202,   204,   446,   199,   246,   247,    14,   345,
     201,   131,    44,   216,   425,   199,   335,   438,   442,   150,
     438,   131,   206,     9,   411,   335,   438,   469,   150,   199,
     405,   430,   435,   200,   345,    30,   231,     8,   357,     9,
     201,   231,   232,   337,   338,   345,   216,   278,   235,   201,
     201,   201,   483,   483,   171,   199,   106,   483,    14,   150,
     216,    79,   201,   201,   201,   182,   183,   184,   189,   190,
     193,   194,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   384,   385,   386,   241,   110,   168,   201,   154,   218,
     221,   223,   154,   219,   222,   223,   223,     9,   201,    97,
     202,   438,     9,   201,   125,   125,    14,     9,   201,   438,
     463,   463,   335,   346,   438,   441,   442,   200,   171,   258,
     132,   438,   452,   453,   201,    66,   430,   147,   460,    78,
     345,   438,    85,   147,   460,   223,   215,   201,   202,   253,
     261,   390,   392,    86,   204,   358,   359,   361,   401,   446,
     464,    14,    97,   465,   353,   355,   356,   288,   289,   428,
     429,   200,   200,   200,   200,   203,   230,   231,   248,   255,
     260,   428,   345,   205,   207,   208,   216,   470,   471,   483,
      35,   164,   290,   291,   345,   467,   199,   468,   256,   246,
     345,   345,   345,    30,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   402,   345,   345,   448,
     448,   345,   455,   456,   125,   202,   216,   445,   446,   264,
     216,   265,   468,   468,   263,   247,    27,    35,   339,   342,
     345,   369,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   155,   202,   216,   431,   432,   433,
     434,   445,   448,   345,   290,   290,   448,   345,   452,   246,
     200,   345,   199,   424,     9,   411,   335,   200,   216,    35,
     345,    35,   345,   200,   200,   445,   290,   202,   216,   431,
     432,   445,   200,   229,   282,   202,   342,   345,   345,    89,
      30,   231,   276,   201,    28,    97,    14,     9,   200,    30,
     202,   279,   483,    29,    86,   227,   477,   478,   479,   199,
       9,    46,    47,    52,    54,    66,    86,   133,   146,   155,
     175,   199,   224,   225,   227,   366,   400,   406,   407,   408,
     216,   482,   185,    79,   345,    79,    79,   345,   381,   382,
     345,   345,   374,   384,   188,   387,   229,   199,   239,   223,
     201,     9,    97,   223,   201,     9,    97,    97,   220,   216,
     345,   293,   407,    79,     9,   200,   200,   200,   200,   200,
     200,   200,   201,    46,    47,   475,   476,   127,   269,   199,
       9,   200,   200,    79,    80,   216,   461,   216,    66,   203,
     203,   212,   214,    30,   128,   268,   170,    50,   155,   170,
     394,   131,     9,   411,   200,   150,   483,   483,    14,   357,
     288,   229,   197,     9,   412,   483,   484,   430,   435,   203,
       9,   411,   172,   438,   345,   200,     9,   412,    14,   349,
     249,   127,   267,   199,   468,   345,    30,   206,   206,   131,
     203,     9,   411,   345,   469,   199,   259,   254,   262,    14,
     465,   257,   246,    68,   438,   345,   469,   206,   203,   200,
     200,   206,   203,   200,    46,    47,    66,    74,    75,    76,
      86,   133,   146,   175,   216,   414,   416,   417,   420,   423,
     216,   438,   438,   131,   430,   435,   200,   345,   283,    71,
      72,   284,   229,   336,   229,   338,    97,    35,   132,   273,
     438,   407,   216,    30,   231,   277,   201,   280,   201,   280,
       9,   172,    86,   131,   150,     9,   411,   200,   164,   470,
     471,   472,   470,   407,   407,   407,   407,   407,   410,   413,
     199,    85,   150,   199,   407,   150,   202,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   150,   469,   345,   185,   185,    14,   191,
     192,   383,     9,   195,   387,    79,   203,   400,   202,   243,
      97,   221,   216,    97,   222,   216,   216,   203,    14,   438,
     201,     9,   172,   216,   270,   400,   202,   452,   132,   438,
      14,   206,   345,   203,   212,   483,   270,   202,   393,    14,
     345,   358,   216,   201,   483,   197,   203,    30,   473,   429,
      35,    79,   164,   431,   432,   434,   483,    35,   164,   345,
     407,   288,   199,   400,   268,   350,   250,   345,   345,   345,
     203,   199,   290,   269,    30,   268,   483,    14,   267,   468,
     402,   203,   199,    14,    74,    75,    76,   216,   415,   415,
     417,   418,   419,    48,   199,    85,   147,   199,     9,   411,
     200,   424,    35,   345,   431,   432,   203,    71,    72,   285,
     336,   231,   203,   201,    90,   201,   273,   438,   199,   131,
     272,    14,   229,   280,   100,   101,   102,   280,   203,   483,
     131,   483,   216,   477,     9,   200,   411,   131,   206,     9,
     411,   410,   216,   358,   360,   362,   200,   125,   216,   407,
     457,   458,   407,   407,   407,    30,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   407,   407,   482,
     345,   345,   345,   382,   345,   372,    79,   244,   216,   216,
     407,   476,    97,    98,   474,     9,   298,   200,   199,   339,
     342,   345,   206,   203,   465,   298,   156,   169,   202,   389,
     396,   156,   202,   395,   131,   201,   473,   483,   357,   484,
      79,   164,    14,    79,   469,   438,   345,   200,   288,   202,
     288,   199,   131,   199,   290,   200,   202,   483,   202,   201,
     483,   268,   251,   405,   290,   131,   206,     9,   411,   416,
     418,   147,   358,   421,   422,   417,   438,   336,    30,    73,
     231,   201,   338,   272,   452,   273,   200,   407,    96,   100,
     201,   345,    30,   201,   281,   203,   172,   483,   131,   164,
      30,   200,   407,   407,   200,   131,     9,   411,   200,   131,
     203,     9,   411,   407,    30,   186,   200,   229,   216,   483,
     483,   400,     4,   107,   112,   118,   120,   157,   158,   160,
     203,   299,   324,   325,   326,   331,   332,   333,   334,   428,
     452,   203,   202,   203,    50,   345,   345,   345,   357,    35,
      79,   164,    14,    79,   345,   199,   473,   200,   298,   200,
     288,   345,   290,   200,   298,   465,   298,   201,   202,   199,
     200,   417,   417,   200,   131,   200,     9,   411,    30,   229,
     201,   200,   200,   200,   236,   201,   201,   281,   229,   483,
     483,   131,   407,   358,   407,   407,   407,   345,   202,   203,
     474,   127,   128,   176,   467,   271,   400,   107,   334,    29,
     120,   133,   134,   155,   161,   308,   309,   310,   311,   400,
     159,   316,   317,   123,   199,   216,   318,   319,   300,   247,
     483,     9,   201,     9,   201,   201,   465,   325,   200,   295,
     155,   391,   203,   203,    79,   164,    14,    79,   345,   290,
     112,   347,   473,   203,   473,   200,   200,   203,   202,   203,
     298,   288,   131,   417,   358,   229,   234,   237,    30,   231,
     275,   229,   200,   407,   131,   131,   187,   229,   400,   400,
     468,    14,     9,   201,   202,   467,   465,   311,   171,   202,
       9,   201,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    53,    67,    68,    69,    70,    71,    72,
      73,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   132,   133,   135,   136,   137,   138,   139,   151,
     152,   153,   163,   165,   166,   168,   175,   176,   178,   180,
     181,   216,   397,   398,     9,   201,   155,   159,   216,   319,
     320,   321,   201,    79,   330,   246,   301,   467,   467,    14,
     247,   203,   296,   297,   467,    14,    79,   345,   200,   199,
     202,   201,   202,   322,   347,   473,   295,   203,   200,   417,
     131,    30,   231,   274,   275,   229,   407,   407,   345,   203,
     201,   201,   407,   400,   304,   483,   312,   313,   406,   309,
      14,    30,    47,   314,   317,     9,    33,   200,    29,    46,
      49,    14,     9,   201,   468,   330,    14,   483,   246,   201,
      14,   345,    35,    79,   388,   229,   229,   202,   322,   203,
     473,   417,   229,    94,   188,   242,   203,   216,   227,   305,
     306,   307,     9,   172,     9,   411,   203,   407,   398,   398,
      55,   315,   320,   320,    29,    46,    49,   407,    79,   199,
     201,   407,   468,   407,    79,     9,   412,   203,   203,   229,
     322,    92,   201,    79,   110,   238,   150,    97,   483,   406,
     162,    14,   302,   199,    35,    79,   200,   203,   201,   199,
     168,   245,   216,   325,   326,   172,   407,   286,   287,   429,
     303,    79,   400,   243,   165,   216,   201,   200,     9,   412,
     114,   115,   116,   328,   329,   286,    79,   271,   201,   473,
     429,   484,   200,   200,   201,   201,   202,   323,   328,    35,
      79,   164,   473,   202,   229,   484,    79,   164,    14,    79,
     323,   229,   203,    35,    79,   164,    14,    79,   345,   203,
      79,   164,    14,    79,   345,    14,    79,   345,   345
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
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval).reset();;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval)++;;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
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

  case 823:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
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

  case 824:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
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
#line 2856 "hphp.y"
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
#line 2865 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval).reset();;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
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

  case 866:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    {;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    {;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    {;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13923 "hphp.tab.cpp"
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
#line 3194 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

