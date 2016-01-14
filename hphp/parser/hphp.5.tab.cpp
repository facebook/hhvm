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
#define yyparse         Compiler5parse
#define yylex           Compiler5lex
#define yyerror         Compiler5error
#define yylval          Compiler5lval
#define yychar          Compiler5char
#define yydebug         Compiler5debug
#define yynerrs         Compiler5nerrs
#define yylloc          Compiler5lloc

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
#line 651 "hphp.5.tab.cpp"

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
     T_YIELD_FROM = 282,
     T_COALESCE = 283,
     T_BOOLEAN_OR = 284,
     T_BOOLEAN_AND = 285,
     T_IS_NOT_IDENTICAL = 286,
     T_IS_IDENTICAL = 287,
     T_IS_NOT_EQUAL = 288,
     T_IS_EQUAL = 289,
     T_SPACESHIP = 290,
     T_IS_GREATER_OR_EQUAL = 291,
     T_IS_SMALLER_OR_EQUAL = 292,
     T_SR = 293,
     T_SL = 294,
     T_INSTANCEOF = 295,
     T_UNSET_CAST = 296,
     T_BOOL_CAST = 297,
     T_OBJECT_CAST = 298,
     T_ARRAY_CAST = 299,
     T_STRING_CAST = 300,
     T_DOUBLE_CAST = 301,
     T_INT_CAST = 302,
     T_DEC = 303,
     T_INC = 304,
     T_POW = 305,
     T_CLONE = 306,
     T_NEW = 307,
     T_EXIT = 308,
     T_IF = 309,
     T_ELSEIF = 310,
     T_ELSE = 311,
     T_ENDIF = 312,
     T_LNUMBER = 313,
     T_DNUMBER = 314,
     T_ONUMBER = 315,
     T_STRING = 316,
     T_STRING_VARNAME = 317,
     T_VARIABLE = 318,
     T_NUM_STRING = 319,
     T_INLINE_HTML = 320,
     T_HASHBANG = 321,
     T_CHARACTER = 322,
     T_BAD_CHARACTER = 323,
     T_ENCAPSED_AND_WHITESPACE = 324,
     T_CONSTANT_ENCAPSED_STRING = 325,
     T_ECHO = 326,
     T_DO = 327,
     T_WHILE = 328,
     T_ENDWHILE = 329,
     T_FOR = 330,
     T_ENDFOR = 331,
     T_FOREACH = 332,
     T_ENDFOREACH = 333,
     T_DECLARE = 334,
     T_ENDDECLARE = 335,
     T_AS = 336,
     T_SUPER = 337,
     T_SWITCH = 338,
     T_ENDSWITCH = 339,
     T_CASE = 340,
     T_DEFAULT = 341,
     T_BREAK = 342,
     T_GOTO = 343,
     T_CONTINUE = 344,
     T_FUNCTION = 345,
     T_CONST = 346,
     T_RETURN = 347,
     T_TRY = 348,
     T_CATCH = 349,
     T_THROW = 350,
     T_USE = 351,
     T_GLOBAL = 352,
     T_PUBLIC = 353,
     T_PROTECTED = 354,
     T_PRIVATE = 355,
     T_FINAL = 356,
     T_ABSTRACT = 357,
     T_STATIC = 358,
     T_VAR = 359,
     T_UNSET = 360,
     T_ISSET = 361,
     T_EMPTY = 362,
     T_HALT_COMPILER = 363,
     T_CLASS = 364,
     T_INTERFACE = 365,
     T_EXTENDS = 366,
     T_IMPLEMENTS = 367,
     T_OBJECT_OPERATOR = 368,
     T_NULLSAFE_OBJECT_OPERATOR = 369,
     T_DOUBLE_ARROW = 370,
     T_LIST = 371,
     T_ARRAY = 372,
     T_CALLABLE = 373,
     T_CLASS_C = 374,
     T_METHOD_C = 375,
     T_FUNC_C = 376,
     T_LINE = 377,
     T_FILE = 378,
     T_COMMENT = 379,
     T_DOC_COMMENT = 380,
     T_OPEN_TAG = 381,
     T_OPEN_TAG_WITH_ECHO = 382,
     T_CLOSE_TAG = 383,
     T_WHITESPACE = 384,
     T_START_HEREDOC = 385,
     T_END_HEREDOC = 386,
     T_DOLLAR_OPEN_CURLY_BRACES = 387,
     T_CURLY_OPEN = 388,
     T_DOUBLE_COLON = 389,
     T_NAMESPACE = 390,
     T_NS_C = 391,
     T_DIR = 392,
     T_NS_SEPARATOR = 393,
     T_XHP_LABEL = 394,
     T_XHP_TEXT = 395,
     T_XHP_ATTRIBUTE = 396,
     T_XHP_CATEGORY = 397,
     T_XHP_CATEGORY_LABEL = 398,
     T_XHP_CHILDREN = 399,
     T_ENUM = 400,
     T_XHP_REQUIRED = 401,
     T_TRAIT = 402,
     T_ELLIPSIS = 403,
     T_INSTEADOF = 404,
     T_TRAIT_C = 405,
     T_HH_ERROR = 406,
     T_FINALLY = 407,
     T_XHP_TAG_LT = 408,
     T_XHP_TAG_GT = 409,
     T_TYPELIST_LT = 410,
     T_TYPELIST_GT = 411,
     T_UNRESOLVED_LT = 412,
     T_COLLECTION = 413,
     T_SHAPE = 414,
     T_TYPE = 415,
     T_UNRESOLVED_TYPE = 416,
     T_NEWTYPE = 417,
     T_UNRESOLVED_NEWTYPE = 418,
     T_COMPILER_HALT_OFFSET = 419,
     T_ASYNC = 420,
     T_LAMBDA_OP = 421,
     T_LAMBDA_CP = 422,
     T_UNRESOLVED_OP = 423
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
#line 874 "hphp.5.tab.cpp"

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
#define YYLAST   16747

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  198
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  270
/* YYNRULES -- Number of rules.  */
#define YYNRULES  993
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1826

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   423

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    55,   196,     2,   193,    54,    37,   197,
     188,   189,    52,    49,     9,    50,    51,    53,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    31,   190,
      42,    14,    43,    30,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    69,     2,   195,    36,     2,   194,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   191,    35,   192,    57,     2,     2,     2,
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
      27,    28,    29,    32,    33,    34,    38,    39,    40,    41,
      44,    45,    46,    47,    48,    56,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    70,    71,    72,    73,
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
     184,   185,   186,   187
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    23,    28,    32,    33,    40,    41,    47,    51,
      56,    61,    68,    76,    84,    87,    89,    91,    93,    95,
      97,    99,   101,   103,   105,   107,   109,   111,   113,   115,
     117,   119,   121,   123,   125,   127,   129,   131,   133,   135,
     137,   139,   141,   143,   145,   147,   149,   151,   153,   155,
     157,   159,   161,   163,   165,   167,   169,   171,   173,   175,
     177,   179,   181,   183,   185,   187,   189,   191,   193,   195,
     197,   199,   201,   203,   205,   207,   209,   211,   213,   215,
     217,   219,   221,   223,   225,   227,   229,   231,   234,   238,
     242,   244,   247,   249,   252,   256,   261,   265,   267,   270,
     272,   275,   278,   280,   284,   286,   290,   293,   296,   299,
     305,   310,   313,   314,   316,   318,   320,   322,   326,   332,
     341,   342,   347,   348,   355,   356,   367,   368,   373,   376,
     380,   383,   387,   390,   394,   398,   402,   406,   410,   414,
     420,   422,   424,   426,   427,   437,   438,   449,   455,   456,
     470,   471,   477,   481,   485,   488,   491,   494,   497,   500,
     503,   507,   510,   513,   517,   520,   523,   524,   529,   539,
     540,   541,   546,   549,   550,   552,   553,   555,   556,   566,
     567,   578,   579,   591,   592,   602,   603,   614,   615,   624,
     625,   635,   636,   644,   645,   654,   655,   664,   665,   673,
     674,   683,   685,   687,   689,   691,   693,   696,   700,   704,
     707,   710,   711,   714,   715,   718,   719,   721,   725,   727,
     731,   734,   735,   737,   740,   745,   747,   752,   754,   759,
     761,   766,   768,   773,   777,   783,   787,   792,   797,   803,
     809,   814,   815,   817,   819,   824,   825,   831,   832,   835,
     836,   840,   841,   849,   858,   865,   868,   874,   881,   886,
     887,   892,   898,   906,   913,   920,   928,   938,   947,   954,
     962,   968,   971,   976,   982,   986,   987,   991,   996,  1003,
    1009,  1015,  1022,  1031,  1039,  1042,  1043,  1045,  1048,  1051,
    1055,  1060,  1065,  1069,  1071,  1073,  1076,  1081,  1085,  1091,
    1093,  1097,  1100,  1101,  1104,  1108,  1111,  1112,  1113,  1118,
    1119,  1125,  1128,  1131,  1134,  1135,  1146,  1147,  1159,  1163,
    1167,  1171,  1176,  1181,  1185,  1191,  1194,  1197,  1198,  1205,
    1211,  1216,  1220,  1222,  1224,  1228,  1233,  1235,  1238,  1240,
    1242,  1248,  1255,  1257,  1259,  1264,  1266,  1268,  1272,  1275,
    1278,  1279,  1282,  1283,  1285,  1289,  1291,  1293,  1295,  1297,
    1301,  1306,  1311,  1316,  1318,  1320,  1323,  1326,  1329,  1333,
    1337,  1339,  1341,  1343,  1345,  1349,  1351,  1355,  1357,  1359,
    1361,  1362,  1364,  1367,  1369,  1371,  1373,  1375,  1377,  1379,
    1381,  1383,  1384,  1386,  1388,  1390,  1394,  1400,  1402,  1406,
    1412,  1417,  1421,  1425,  1429,  1434,  1438,  1442,  1446,  1449,
    1452,  1454,  1456,  1460,  1464,  1466,  1468,  1469,  1471,  1474,
    1479,  1483,  1487,  1494,  1497,  1501,  1504,  1508,  1515,  1517,
    1519,  1521,  1523,  1525,  1532,  1536,  1541,  1548,  1552,  1556,
    1560,  1564,  1568,  1572,  1576,  1580,  1584,  1588,  1592,  1596,
    1599,  1602,  1605,  1608,  1612,  1616,  1620,  1624,  1628,  1632,
    1636,  1640,  1644,  1648,  1652,  1656,  1660,  1664,  1668,  1672,
    1676,  1679,  1682,  1685,  1688,  1692,  1696,  1700,  1704,  1708,
    1712,  1716,  1720,  1724,  1728,  1732,  1738,  1743,  1747,  1749,
    1752,  1755,  1758,  1761,  1764,  1767,  1770,  1773,  1776,  1778,
    1780,  1782,  1786,  1789,  1791,  1797,  1798,  1799,  1812,  1813,
    1827,  1828,  1833,  1834,  1842,  1843,  1849,  1850,  1854,  1855,
    1862,  1865,  1868,  1873,  1875,  1877,  1883,  1887,  1893,  1897,
    1900,  1901,  1904,  1905,  1910,  1915,  1919,  1924,  1929,  1934,
    1939,  1941,  1943,  1945,  1947,  1951,  1955,  1960,  1962,  1965,
    1970,  1973,  1980,  1981,  1983,  1988,  1989,  1992,  1993,  1995,
    1997,  2001,  2003,  2007,  2009,  2011,  2015,  2019,  2021,  2023,
    2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,
    2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,
    2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,
    2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,
    2105,  2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,  2123,
    2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,  2143,
    2145,  2147,  2149,  2151,  2153,  2155,  2157,  2159,  2161,  2163,
    2165,  2167,  2169,  2171,  2173,  2175,  2177,  2179,  2181,  2186,
    2188,  2190,  2192,  2194,  2196,  2198,  2202,  2204,  2208,  2210,
    2212,  2216,  2218,  2220,  2222,  2225,  2227,  2228,  2229,  2231,
    2233,  2237,  2238,  2240,  2242,  2244,  2246,  2248,  2250,  2252,
    2254,  2256,  2258,  2260,  2262,  2264,  2268,  2271,  2273,  2275,
    2280,  2284,  2289,  2291,  2293,  2297,  2301,  2305,  2309,  2313,
    2317,  2321,  2325,  2329,  2333,  2337,  2341,  2345,  2349,  2353,
    2357,  2361,  2365,  2368,  2371,  2374,  2377,  2381,  2385,  2389,
    2393,  2397,  2401,  2405,  2409,  2413,  2419,  2424,  2428,  2432,
    2436,  2438,  2440,  2442,  2444,  2448,  2452,  2456,  2459,  2460,
    2462,  2463,  2465,  2466,  2472,  2476,  2480,  2482,  2484,  2486,
    2488,  2492,  2495,  2497,  2499,  2501,  2503,  2505,  2509,  2511,
    2513,  2515,  2518,  2521,  2526,  2530,  2535,  2538,  2539,  2545,
    2549,  2553,  2555,  2559,  2561,  2564,  2565,  2571,  2575,  2578,
    2579,  2583,  2584,  2589,  2592,  2593,  2597,  2601,  2603,  2604,
    2606,  2608,  2610,  2612,  2616,  2618,  2620,  2622,  2626,  2628,
    2630,  2634,  2638,  2641,  2646,  2649,  2654,  2660,  2666,  2672,
    2678,  2680,  2682,  2684,  2686,  2688,  2690,  2694,  2698,  2703,
    2708,  2712,  2714,  2716,  2718,  2720,  2724,  2726,  2731,  2735,
    2737,  2739,  2741,  2743,  2745,  2749,  2753,  2758,  2763,  2767,
    2769,  2771,  2779,  2789,  2797,  2804,  2813,  2815,  2818,  2823,
    2828,  2830,  2832,  2837,  2839,  2840,  2842,  2845,  2847,  2849,
    2851,  2855,  2859,  2863,  2864,  2866,  2868,  2872,  2876,  2879,
    2883,  2890,  2891,  2893,  2898,  2901,  2902,  2908,  2912,  2916,
    2918,  2925,  2930,  2935,  2938,  2941,  2942,  2948,  2952,  2956,
    2958,  2961,  2962,  2968,  2972,  2976,  2978,  2981,  2984,  2986,
    2989,  2991,  2996,  3000,  3004,  3011,  3015,  3017,  3019,  3021,
    3026,  3031,  3036,  3041,  3046,  3051,  3054,  3057,  3062,  3065,
    3068,  3070,  3074,  3078,  3082,  3083,  3086,  3092,  3099,  3106,
    3114,  3116,  3119,  3121,  3124,  3126,  3131,  3133,  3138,  3142,
    3143,  3145,  3149,  3152,  3156,  3158,  3160,  3161,  3162,  3165,
    3168,  3171,  3174,  3179,  3182,  3188,  3192,  3194,  3196,  3197,
    3201,  3206,  3212,  3216,  3218,  3221,  3222,  3227,  3229,  3233,
    3236,  3239,  3242,  3244,  3246,  3248,  3250,  3254,  3260,  3267,
    3269,  3278,  3285,  3287
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     199,     0,    -1,    -1,   200,   201,    -1,   201,   202,    -1,
      -1,   222,    -1,   239,    -1,   246,    -1,   243,    -1,   253,
      -1,   446,    -1,   127,   188,   189,   190,    -1,   154,   215,
     190,    -1,    -1,   154,   215,   191,   203,   201,   192,    -1,
      -1,   154,   191,   204,   201,   192,    -1,   115,   210,   190,
      -1,   115,   109,   210,   190,    -1,   115,   110,   210,   190,
      -1,   115,   208,   191,   213,   192,   190,    -1,   115,   109,
     208,   191,   210,   192,   190,    -1,   115,   110,   208,   191,
     210,   192,   190,    -1,   219,   190,    -1,    80,    -1,   101,
      -1,   160,    -1,   161,    -1,   163,    -1,   165,    -1,   164,
      -1,   205,    -1,   137,    -1,   166,    -1,   130,    -1,   131,
      -1,   122,    -1,   121,    -1,   120,    -1,   119,    -1,   118,
      -1,   117,    -1,   110,    -1,    99,    -1,    95,    -1,    97,
      -1,    76,    -1,    93,    -1,    12,    -1,   116,    -1,   107,
      -1,    56,    -1,   168,    -1,   129,    -1,   154,    -1,    71,
      -1,    10,    -1,    11,    -1,   112,    -1,   115,    -1,   123,
      -1,    72,    -1,   135,    -1,    70,    -1,     7,    -1,     6,
      -1,   114,    -1,   136,    -1,    13,    -1,    90,    -1,     4,
      -1,     3,    -1,   111,    -1,    75,    -1,    74,    -1,   105,
      -1,   106,    -1,   108,    -1,   102,    -1,    27,    -1,    29,
      -1,   109,    -1,    73,    -1,   103,    -1,   171,    -1,    94,
      -1,    96,    -1,    98,    -1,   104,    -1,    91,    -1,    92,
      -1,   100,    -1,   113,    -1,   124,    -1,   206,    -1,   128,
      -1,   215,   157,    -1,   157,   215,   157,    -1,   209,     9,
     211,    -1,   211,    -1,   209,   390,    -1,   215,    -1,   157,
     215,    -1,   215,   100,   205,    -1,   157,   215,   100,   205,
      -1,   212,     9,   214,    -1,   214,    -1,   212,   390,    -1,
     211,    -1,   109,   211,    -1,   110,   211,    -1,   205,    -1,
     215,   157,   205,    -1,   215,    -1,   154,   157,   215,    -1,
     157,   215,    -1,   216,   451,    -1,   216,   451,    -1,   219,
       9,   447,    14,   385,    -1,   110,   447,    14,   385,    -1,
     220,   221,    -1,    -1,   222,    -1,   239,    -1,   246,    -1,
     253,    -1,   191,   220,   192,    -1,    73,   329,   222,   275,
     277,    -1,    73,   329,    31,   220,   276,   278,    76,   190,
      -1,    -1,    92,   329,   223,   269,    -1,    -1,    91,   224,
     222,    92,   329,   190,    -1,    -1,    94,   188,   331,   190,
     331,   190,   331,   189,   225,   267,    -1,    -1,   102,   329,
     226,   272,    -1,   106,   190,    -1,   106,   340,   190,    -1,
     108,   190,    -1,   108,   340,   190,    -1,   111,   190,    -1,
     111,   340,   190,    -1,    27,   106,   190,    -1,   116,   285,
     190,    -1,   122,   287,   190,    -1,    90,   330,   190,    -1,
     146,   330,   190,    -1,   124,   188,   443,   189,   190,    -1,
     190,    -1,    84,    -1,    85,    -1,    -1,    96,   188,   340,
     100,   266,   265,   189,   227,   268,    -1,    -1,    96,   188,
     340,    28,   100,   266,   265,   189,   228,   268,    -1,    98,
     188,   271,   189,   270,    -1,    -1,   112,   231,   113,   188,
     376,    82,   189,   191,   220,   192,   233,   229,   236,    -1,
      -1,   112,   231,   171,   230,   234,    -1,   114,   340,   190,
      -1,   107,   205,   190,    -1,   340,   190,    -1,   332,   190,
      -1,   333,   190,    -1,   334,   190,    -1,   335,   190,    -1,
     336,   190,    -1,   111,   335,   190,    -1,   337,   190,    -1,
     338,   190,    -1,   111,   337,   190,    -1,   339,   190,    -1,
     205,    31,    -1,    -1,   191,   232,   220,   192,    -1,   233,
     113,   188,   376,    82,   189,   191,   220,   192,    -1,    -1,
      -1,   191,   235,   220,   192,    -1,   171,   234,    -1,    -1,
      37,    -1,    -1,   109,    -1,    -1,   238,   237,   450,   240,
     188,   281,   189,   455,   315,    -1,    -1,   319,   238,   237,
     450,   241,   188,   281,   189,   455,   315,    -1,    -1,   406,
     318,   238,   237,   450,   242,   188,   281,   189,   455,   315,
      -1,    -1,   164,   205,   244,    31,   466,   445,   191,   288,
     192,    -1,    -1,   406,   164,   205,   245,    31,   466,   445,
     191,   288,   192,    -1,    -1,   259,   256,   247,   260,   261,
     191,   291,   192,    -1,    -1,   406,   259,   256,   248,   260,
     261,   191,   291,   192,    -1,    -1,   129,   257,   249,   262,
     191,   291,   192,    -1,    -1,   406,   129,   257,   250,   262,
     191,   291,   192,    -1,    -1,   128,   252,   383,   260,   261,
     191,   291,   192,    -1,    -1,   166,   258,   254,   261,   191,
     291,   192,    -1,    -1,   406,   166,   258,   255,   261,   191,
     291,   192,    -1,   450,    -1,   158,    -1,   450,    -1,   450,
      -1,   128,    -1,   121,   128,    -1,   121,   120,   128,    -1,
     120,   121,   128,    -1,   120,   128,    -1,   130,   376,    -1,
      -1,   131,   263,    -1,    -1,   130,   263,    -1,    -1,   376,
      -1,   263,     9,   376,    -1,   376,    -1,   264,     9,   376,
      -1,   134,   266,    -1,    -1,   418,    -1,    37,   418,    -1,
     135,   188,   432,   189,    -1,   222,    -1,    31,   220,    95,
     190,    -1,   222,    -1,    31,   220,    97,   190,    -1,   222,
      -1,    31,   220,    93,   190,    -1,   222,    -1,    31,   220,
      99,   190,    -1,   205,    14,   385,    -1,   271,     9,   205,
      14,   385,    -1,   191,   273,   192,    -1,   191,   190,   273,
     192,    -1,    31,   273,   103,   190,    -1,    31,   190,   273,
     103,   190,    -1,   273,   104,   340,   274,   220,    -1,   273,
     105,   274,   220,    -1,    -1,    31,    -1,   190,    -1,   275,
      74,   329,   222,    -1,    -1,   276,    74,   329,    31,   220,
      -1,    -1,    75,   222,    -1,    -1,    75,    31,   220,    -1,
      -1,   280,     9,   407,   321,   467,   167,    82,    -1,   280,
       9,   407,   321,   467,    37,   167,    82,    -1,   280,     9,
     407,   321,   467,   167,    -1,   280,   390,    -1,   407,   321,
     467,   167,    82,    -1,   407,   321,   467,    37,   167,    82,
      -1,   407,   321,   467,   167,    -1,    -1,   407,   321,   467,
      82,    -1,   407,   321,   467,    37,    82,    -1,   407,   321,
     467,    37,    82,    14,   340,    -1,   407,   321,   467,    82,
      14,   340,    -1,   280,     9,   407,   321,   467,    82,    -1,
     280,     9,   407,   321,   467,    37,    82,    -1,   280,     9,
     407,   321,   467,    37,    82,    14,   340,    -1,   280,     9,
     407,   321,   467,    82,    14,   340,    -1,   282,     9,   407,
     467,   167,    82,    -1,   282,     9,   407,   467,    37,   167,
      82,    -1,   282,     9,   407,   467,   167,    -1,   282,   390,
      -1,   407,   467,   167,    82,    -1,   407,   467,    37,   167,
      82,    -1,   407,   467,   167,    -1,    -1,   407,   467,    82,
      -1,   407,   467,    37,    82,    -1,   407,   467,    37,    82,
      14,   340,    -1,   407,   467,    82,    14,   340,    -1,   282,
       9,   407,   467,    82,    -1,   282,     9,   407,   467,    37,
      82,    -1,   282,     9,   407,   467,    37,    82,    14,   340,
      -1,   282,     9,   407,   467,    82,    14,   340,    -1,   284,
     390,    -1,    -1,   340,    -1,    37,   418,    -1,   167,   340,
      -1,   284,     9,   340,    -1,   284,     9,   167,   340,    -1,
     284,     9,    37,   418,    -1,   285,     9,   286,    -1,   286,
      -1,    82,    -1,   193,   418,    -1,   193,   191,   340,   192,
      -1,   287,     9,    82,    -1,   287,     9,    82,    14,   385,
      -1,    82,    -1,    82,    14,   385,    -1,   288,   289,    -1,
      -1,   290,   190,    -1,   448,    14,   385,    -1,   291,   292,
      -1,    -1,    -1,   317,   293,   323,   190,    -1,    -1,   319,
     466,   294,   323,   190,    -1,   324,   190,    -1,   325,   190,
      -1,   326,   190,    -1,    -1,   318,   238,   237,   449,   188,
     295,   279,   189,   455,   316,    -1,    -1,   406,   318,   238,
     237,   450,   188,   296,   279,   189,   455,   316,    -1,   160,
     301,   190,    -1,   161,   309,   190,    -1,   163,   311,   190,
      -1,     4,   130,   376,   190,    -1,     4,   131,   376,   190,
      -1,   115,   264,   190,    -1,   115,   264,   191,   297,   192,
      -1,   297,   298,    -1,   297,   299,    -1,    -1,   218,   153,
     205,   168,   264,   190,    -1,   300,   100,   318,   205,   190,
      -1,   300,   100,   319,   190,    -1,   218,   153,   205,    -1,
     205,    -1,   302,    -1,   301,     9,   302,    -1,   303,   373,
     307,   308,    -1,   158,    -1,    30,   304,    -1,   304,    -1,
     136,    -1,   136,   174,   466,   389,   175,    -1,   136,   174,
     466,     9,   466,   175,    -1,   376,    -1,   123,    -1,   164,
     191,   306,   192,    -1,   137,    -1,   384,    -1,   305,     9,
     384,    -1,   305,   389,    -1,    14,   385,    -1,    -1,    58,
     165,    -1,    -1,   310,    -1,   309,     9,   310,    -1,   162,
      -1,   312,    -1,   205,    -1,   126,    -1,   188,   313,   189,
      -1,   188,   313,   189,    52,    -1,   188,   313,   189,    30,
      -1,   188,   313,   189,    49,    -1,   312,    -1,   314,    -1,
     314,    52,    -1,   314,    30,    -1,   314,    49,    -1,   313,
       9,   313,    -1,   313,    35,   313,    -1,   205,    -1,   158,
      -1,   162,    -1,   190,    -1,   191,   220,   192,    -1,   190,
      -1,   191,   220,   192,    -1,   319,    -1,   123,    -1,   319,
      -1,    -1,   320,    -1,   319,   320,    -1,   117,    -1,   118,
      -1,   119,    -1,   122,    -1,   121,    -1,   120,    -1,   184,
      -1,   322,    -1,    -1,   117,    -1,   118,    -1,   119,    -1,
     323,     9,    82,    -1,   323,     9,    82,    14,   385,    -1,
      82,    -1,    82,    14,   385,    -1,   324,     9,   448,    14,
     385,    -1,   110,   448,    14,   385,    -1,   325,     9,   448,
      -1,   121,   110,   448,    -1,   121,   327,   445,    -1,   327,
     445,    14,   466,    -1,   110,   179,   450,    -1,   188,   328,
     189,    -1,    71,   380,   383,    -1,    71,   251,    -1,    70,
     340,    -1,   365,    -1,   360,    -1,   188,   340,   189,    -1,
     330,     9,   340,    -1,   340,    -1,   330,    -1,    -1,    27,
      -1,    27,   340,    -1,    27,   340,   134,   340,    -1,   188,
     332,   189,    -1,   418,    14,   332,    -1,   135,   188,   432,
     189,    14,   332,    -1,    29,   340,    -1,   418,    14,   335,
      -1,    28,   340,    -1,   418,    14,   337,    -1,   135,   188,
     432,   189,    14,   337,    -1,   341,    -1,   418,    -1,   328,
      -1,   422,    -1,   421,    -1,   135,   188,   432,   189,    14,
     340,    -1,   418,    14,   340,    -1,   418,    14,    37,   418,
      -1,   418,    14,    37,    71,   380,   383,    -1,   418,    26,
     340,    -1,   418,    25,   340,    -1,   418,    24,   340,    -1,
     418,    23,   340,    -1,   418,    22,   340,    -1,   418,    21,
     340,    -1,   418,    20,   340,    -1,   418,    19,   340,    -1,
     418,    18,   340,    -1,   418,    17,   340,    -1,   418,    16,
     340,    -1,   418,    15,   340,    -1,   418,    67,    -1,    67,
     418,    -1,   418,    66,    -1,    66,   418,    -1,   340,    33,
     340,    -1,   340,    34,   340,    -1,   340,    10,   340,    -1,
     340,    12,   340,    -1,   340,    11,   340,    -1,   340,    35,
     340,    -1,   340,    37,   340,    -1,   340,    36,   340,    -1,
     340,    51,   340,    -1,   340,    49,   340,    -1,   340,    50,
     340,    -1,   340,    52,   340,    -1,   340,    53,   340,    -1,
     340,    68,   340,    -1,   340,    54,   340,    -1,   340,    48,
     340,    -1,   340,    47,   340,    -1,    49,   340,    -1,    50,
     340,    -1,    55,   340,    -1,    57,   340,    -1,   340,    39,
     340,    -1,   340,    38,   340,    -1,   340,    41,   340,    -1,
     340,    40,   340,    -1,   340,    42,   340,    -1,   340,    46,
     340,    -1,   340,    43,   340,    -1,   340,    45,   340,    -1,
     340,    44,   340,    -1,   340,    56,   380,    -1,   188,   341,
     189,    -1,   340,    30,   340,    31,   340,    -1,   340,    30,
      31,   340,    -1,   340,    32,   340,    -1,   442,    -1,    65,
     340,    -1,    64,   340,    -1,    63,   340,    -1,    62,   340,
      -1,    61,   340,    -1,    60,   340,    -1,    59,   340,    -1,
      72,   381,    -1,    58,   340,    -1,   387,    -1,   359,    -1,
     358,    -1,   194,   382,   194,    -1,    13,   340,    -1,   362,
      -1,   115,   188,   364,   390,   189,    -1,    -1,    -1,   238,
     237,   188,   344,   281,   189,   455,   342,   455,   191,   220,
     192,    -1,    -1,   319,   238,   237,   188,   345,   281,   189,
     455,   342,   455,   191,   220,   192,    -1,    -1,   184,    82,
     347,   352,    -1,    -1,   184,   185,   348,   281,   186,   455,
     352,    -1,    -1,   184,   191,   349,   220,   192,    -1,    -1,
      82,   350,   352,    -1,    -1,   185,   351,   281,   186,   455,
     352,    -1,     8,   340,    -1,     8,   337,    -1,     8,   191,
     220,   192,    -1,    89,    -1,   444,    -1,   354,     9,   353,
     134,   340,    -1,   353,   134,   340,    -1,   355,     9,   353,
     134,   385,    -1,   353,   134,   385,    -1,   354,   389,    -1,
      -1,   355,   389,    -1,    -1,   178,   188,   356,   189,    -1,
     136,   188,   433,   189,    -1,    69,   433,   195,    -1,   376,
     191,   435,   192,    -1,   376,   191,   437,   192,    -1,   362,
      69,   428,   195,    -1,   363,    69,   428,   195,    -1,   359,
      -1,   444,    -1,   421,    -1,    89,    -1,   188,   341,   189,
      -1,   364,     9,    82,    -1,   364,     9,    37,    82,    -1,
      82,    -1,    37,    82,    -1,   172,   158,   366,   173,    -1,
     368,    53,    -1,   368,   173,   369,   172,    53,   367,    -1,
      -1,   158,    -1,   368,   370,    14,   371,    -1,    -1,   369,
     372,    -1,    -1,   158,    -1,   159,    -1,   191,   340,   192,
      -1,   159,    -1,   191,   340,   192,    -1,   365,    -1,   374,
      -1,   373,    31,   374,    -1,   373,    50,   374,    -1,   205,
      -1,    72,    -1,   109,    -1,   110,    -1,   111,    -1,    27,
      -1,    29,    -1,    28,    -1,   112,    -1,   113,    -1,   171,
      -1,   114,    -1,    73,    -1,    74,    -1,    76,    -1,    75,
      -1,    92,    -1,    93,    -1,    91,    -1,    94,    -1,    95,
      -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,    56,
      -1,   100,    -1,   102,    -1,   103,    -1,   104,    -1,   105,
      -1,   106,    -1,   108,    -1,   107,    -1,    90,    -1,    13,
      -1,   128,    -1,   129,    -1,   130,    -1,   131,    -1,    71,
      -1,    70,    -1,   123,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   154,    -1,   115,    -1,   116,
      -1,   125,    -1,   126,    -1,   127,    -1,   122,    -1,   121,
      -1,   120,    -1,   119,    -1,   118,    -1,   117,    -1,   184,
      -1,   124,    -1,   135,    -1,   136,    -1,    10,    -1,    12,
      -1,    11,    -1,   138,    -1,   140,    -1,   139,    -1,   141,
      -1,   142,    -1,   156,    -1,   155,    -1,   183,    -1,   166,
      -1,   169,    -1,   168,    -1,   179,    -1,   181,    -1,   178,
      -1,   217,   188,   283,   189,    -1,   218,    -1,   158,    -1,
     376,    -1,   384,    -1,   122,    -1,   426,    -1,   188,   341,
     189,    -1,   377,    -1,   378,   153,   425,    -1,   377,    -1,
     424,    -1,   379,   153,   425,    -1,   376,    -1,   122,    -1,
     430,    -1,   188,   189,    -1,   329,    -1,    -1,    -1,    88,
      -1,   439,    -1,   188,   283,   189,    -1,    -1,    77,    -1,
      78,    -1,    79,    -1,    89,    -1,   141,    -1,   142,    -1,
     156,    -1,   138,    -1,   169,    -1,   139,    -1,   140,    -1,
     155,    -1,   183,    -1,   149,    88,   150,    -1,   149,   150,
      -1,   384,    -1,   216,    -1,   136,   188,   388,   189,    -1,
      69,   388,   195,    -1,   178,   188,   357,   189,    -1,   386,
      -1,   361,    -1,   188,   385,   189,    -1,   385,    33,   385,
      -1,   385,    34,   385,    -1,   385,    10,   385,    -1,   385,
      12,   385,    -1,   385,    11,   385,    -1,   385,    35,   385,
      -1,   385,    37,   385,    -1,   385,    36,   385,    -1,   385,
      51,   385,    -1,   385,    49,   385,    -1,   385,    50,   385,
      -1,   385,    52,   385,    -1,   385,    53,   385,    -1,   385,
      54,   385,    -1,   385,    48,   385,    -1,   385,    47,   385,
      -1,   385,    68,   385,    -1,    55,   385,    -1,    57,   385,
      -1,    49,   385,    -1,    50,   385,    -1,   385,    39,   385,
      -1,   385,    38,   385,    -1,   385,    41,   385,    -1,   385,
      40,   385,    -1,   385,    42,   385,    -1,   385,    46,   385,
      -1,   385,    43,   385,    -1,   385,    45,   385,    -1,   385,
      44,   385,    -1,   385,    30,   385,    31,   385,    -1,   385,
      30,    31,   385,    -1,   218,   153,   206,    -1,   158,   153,
     206,    -1,   218,   153,   128,    -1,   216,    -1,    81,    -1,
     444,    -1,   384,    -1,   196,   439,   196,    -1,   197,   439,
     197,    -1,   149,   439,   150,    -1,   391,   389,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   391,     9,   385,   134,
     385,    -1,   391,     9,   385,    -1,   385,   134,   385,    -1,
     385,    -1,    77,    -1,    78,    -1,    79,    -1,   149,    88,
     150,    -1,   149,   150,    -1,    77,    -1,    78,    -1,    79,
      -1,   205,    -1,    89,    -1,    89,    51,   394,    -1,   392,
      -1,   394,    -1,   205,    -1,    49,   393,    -1,    50,   393,
      -1,   136,   188,   396,   189,    -1,    69,   396,   195,    -1,
     178,   188,   399,   189,    -1,   397,   389,    -1,    -1,   397,
       9,   395,   134,   395,    -1,   397,     9,   395,    -1,   395,
     134,   395,    -1,   395,    -1,   398,     9,   395,    -1,   395,
      -1,   400,   389,    -1,    -1,   400,     9,   353,   134,   395,
      -1,   353,   134,   395,    -1,   398,   389,    -1,    -1,   188,
     401,   189,    -1,    -1,   403,     9,   205,   402,    -1,   205,
     402,    -1,    -1,   405,   403,   389,    -1,    48,   404,    47,
      -1,   406,    -1,    -1,   132,    -1,   133,    -1,   205,    -1,
     158,    -1,   191,   340,   192,    -1,   409,    -1,   425,    -1,
     205,    -1,   191,   340,   192,    -1,   411,    -1,   425,    -1,
      69,   428,   195,    -1,   191,   340,   192,    -1,   419,   413,
      -1,   188,   328,   189,   413,    -1,   431,   413,    -1,   188,
     328,   189,   413,    -1,   188,   328,   189,   408,   410,    -1,
     188,   341,   189,   408,   410,    -1,   188,   328,   189,   408,
     409,    -1,   188,   341,   189,   408,   409,    -1,   425,    -1,
     375,    -1,   423,    -1,   424,    -1,   414,    -1,   416,    -1,
     418,   408,   410,    -1,   379,   153,   425,    -1,   420,   188,
     283,   189,    -1,   421,   188,   283,   189,    -1,   188,   418,
     189,    -1,   375,    -1,   423,    -1,   424,    -1,   414,    -1,
     418,   408,   409,    -1,   417,    -1,   420,   188,   283,   189,
      -1,   188,   418,   189,    -1,   425,    -1,   414,    -1,   375,
      -1,   359,    -1,   384,    -1,   188,   418,   189,    -1,   188,
     341,   189,    -1,   421,   188,   283,   189,    -1,   420,   188,
     283,   189,    -1,   188,   422,   189,    -1,   343,    -1,   346,
      -1,   418,   408,   412,   451,   188,   283,   189,    -1,   188,
     328,   189,   408,   412,   451,   188,   283,   189,    -1,   379,
     153,   207,   451,   188,   283,   189,    -1,   379,   153,   425,
     188,   283,   189,    -1,   379,   153,   191,   340,   192,   188,
     283,   189,    -1,   426,    -1,   429,   426,    -1,   426,    69,
     428,   195,    -1,   426,   191,   340,   192,    -1,   427,    -1,
      82,    -1,   193,   191,   340,   192,    -1,   340,    -1,    -1,
     193,    -1,   429,   193,    -1,   425,    -1,   415,    -1,   416,
      -1,   430,   408,   410,    -1,   378,   153,   425,    -1,   188,
     418,   189,    -1,    -1,   415,    -1,   417,    -1,   430,   408,
     409,    -1,   188,   418,   189,    -1,   432,     9,    -1,   432,
       9,   418,    -1,   432,     9,   135,   188,   432,   189,    -1,
      -1,   418,    -1,   135,   188,   432,   189,    -1,   434,   389,
      -1,    -1,   434,     9,   340,   134,   340,    -1,   434,     9,
     340,    -1,   340,   134,   340,    -1,   340,    -1,   434,     9,
     340,   134,    37,   418,    -1,   434,     9,    37,   418,    -1,
     340,   134,    37,   418,    -1,    37,   418,    -1,   436,   389,
      -1,    -1,   436,     9,   340,   134,   340,    -1,   436,     9,
     340,    -1,   340,   134,   340,    -1,   340,    -1,   438,   389,
      -1,    -1,   438,     9,   385,   134,   385,    -1,   438,     9,
     385,    -1,   385,   134,   385,    -1,   385,    -1,   439,   440,
      -1,   439,    88,    -1,   440,    -1,    88,   440,    -1,    82,
      -1,    82,    69,   441,   195,    -1,    82,   408,   205,    -1,
     151,   340,   192,    -1,   151,    81,    69,   340,   195,   192,
      -1,   152,   418,   192,    -1,   205,    -1,    83,    -1,    82,
      -1,   125,   188,   330,   189,    -1,   126,   188,   418,   189,
      -1,   126,   188,   341,   189,    -1,   126,   188,   422,   189,
      -1,   126,   188,   421,   189,    -1,   126,   188,   328,   189,
      -1,     7,   340,    -1,     6,   340,    -1,     5,   188,   340,
     189,    -1,     4,   340,    -1,     3,   340,    -1,   418,    -1,
     443,     9,   418,    -1,   379,   153,   206,    -1,   379,   153,
     128,    -1,    -1,   100,   466,    -1,   179,   450,    14,   466,
     190,    -1,   406,   179,   450,    14,   466,   190,    -1,   181,
     450,   445,    14,   466,   190,    -1,   406,   181,   450,   445,
      14,   466,   190,    -1,   207,    -1,   466,   207,    -1,   206,
      -1,   466,   206,    -1,   207,    -1,   207,   174,   457,   175,
      -1,   205,    -1,   205,   174,   457,   175,    -1,   174,   453,
     175,    -1,    -1,   466,    -1,   452,     9,   466,    -1,   452,
     389,    -1,   452,     9,   167,    -1,   453,    -1,   167,    -1,
      -1,    -1,    31,   466,    -1,   100,   466,    -1,   101,   466,
      -1,   458,   389,    -1,   458,     9,   459,   205,    -1,   459,
     205,    -1,   458,     9,   459,   205,   456,    -1,   459,   205,
     456,    -1,    49,    -1,    50,    -1,    -1,    89,   134,   466,
      -1,    30,    89,   134,   466,    -1,   218,   153,   205,   134,
     466,    -1,   461,     9,   460,    -1,   460,    -1,   461,   389,
      -1,    -1,   178,   188,   462,   189,    -1,   218,    -1,   205,
     153,   465,    -1,   205,   451,    -1,    30,   466,    -1,    58,
     466,    -1,   218,    -1,   136,    -1,   137,    -1,   463,    -1,
     464,   153,   465,    -1,   136,   174,   466,   389,   175,    -1,
     136,   174,   466,     9,   466,   175,    -1,   158,    -1,   188,
     109,   188,   454,   189,    31,   466,   189,    -1,   188,   466,
       9,   452,   389,   189,    -1,   466,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   728,   728,   728,   737,   739,   742,   743,   744,   745,
     746,   747,   748,   751,   753,   753,   755,   755,   757,   759,
     762,   765,   769,   773,   777,   782,   783,   784,   785,   786,
     787,   788,   792,   793,   794,   795,   796,   797,   798,   799,
     800,   801,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   829,
     830,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     840,   841,   842,   843,   844,   845,   846,   847,   848,   849,
     850,   851,   852,   853,   857,   861,   862,   866,   867,   872,
     874,   879,   884,   885,   886,   888,   893,   895,   900,   905,
     907,   909,   914,   915,   919,   920,   922,   926,   933,   940,
     944,   950,   952,   955,   956,   957,   958,   961,   962,   966,
     971,   971,   977,   977,   984,   983,   989,   989,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1012,  1010,  1019,  1017,  1024,  1034,  1028,
    1038,  1036,  1040,  1041,  1045,  1046,  1047,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1064,  1064,  1069,  1075,
    1079,  1079,  1087,  1088,  1092,  1093,  1097,  1103,  1101,  1116,
    1113,  1129,  1126,  1143,  1142,  1151,  1149,  1161,  1160,  1179,
    1177,  1196,  1195,  1204,  1202,  1213,  1213,  1220,  1219,  1231,
    1229,  1242,  1243,  1247,  1250,  1253,  1254,  1255,  1258,  1259,
    1262,  1264,  1267,  1268,  1271,  1272,  1275,  1276,  1280,  1281,
    1286,  1287,  1290,  1291,  1292,  1296,  1297,  1301,  1302,  1306,
    1307,  1311,  1312,  1317,  1318,  1324,  1325,  1326,  1327,  1330,
    1333,  1335,  1338,  1339,  1343,  1345,  1348,  1351,  1354,  1355,
    1358,  1359,  1363,  1369,  1375,  1382,  1384,  1389,  1394,  1400,
    1404,  1408,  1412,  1417,  1422,  1427,  1432,  1438,  1447,  1452,
    1457,  1463,  1465,  1469,  1473,  1478,  1482,  1485,  1488,  1492,
    1496,  1500,  1504,  1509,  1517,  1519,  1522,  1523,  1524,  1525,
    1527,  1529,  1534,  1535,  1538,  1539,  1540,  1544,  1545,  1547,
    1548,  1552,  1554,  1557,  1561,  1567,  1569,  1572,  1572,  1576,
    1575,  1579,  1581,  1584,  1587,  1585,  1601,  1597,  1611,  1613,
    1615,  1617,  1619,  1621,  1623,  1627,  1628,  1629,  1632,  1638,
    1642,  1648,  1651,  1656,  1658,  1663,  1668,  1672,  1673,  1677,
    1678,  1680,  1682,  1688,  1689,  1691,  1695,  1696,  1701,  1705,
    1706,  1710,  1711,  1715,  1717,  1723,  1728,  1729,  1731,  1735,
    1736,  1737,  1738,  1742,  1743,  1744,  1745,  1746,  1747,  1749,
    1754,  1757,  1758,  1762,  1763,  1767,  1768,  1771,  1772,  1775,
    1776,  1779,  1780,  1784,  1785,  1786,  1787,  1788,  1789,  1790,
    1794,  1795,  1798,  1799,  1800,  1803,  1805,  1807,  1808,  1811,
    1813,  1817,  1819,  1823,  1827,  1831,  1836,  1837,  1839,  1840,
    1841,  1842,  1845,  1849,  1850,  1854,  1855,  1859,  1860,  1861,
    1862,  1866,  1870,  1875,  1879,  1883,  1887,  1891,  1896,  1897,
    1898,  1899,  1900,  1904,  1906,  1907,  1908,  1911,  1912,  1913,
    1914,  1915,  1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,
    1924,  1925,  1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1937,  1938,  1939,  1940,  1941,  1942,  1943,
    1944,  1945,  1946,  1947,  1948,  1949,  1950,  1951,  1952,  1953,
    1955,  1956,  1958,  1959,  1961,  1962,  1963,  1964,  1965,  1966,
    1967,  1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,
    1977,  1978,  1979,  1980,  1984,  1988,  1993,  1992,  2007,  2005,
    2023,  2022,  2041,  2040,  2059,  2058,  2076,  2076,  2091,  2091,
    2109,  2110,  2111,  2116,  2118,  2122,  2126,  2132,  2136,  2142,
    2144,  2148,  2150,  2154,  2158,  2159,  2163,  2170,  2177,  2179,
    2184,  2185,  2186,  2187,  2189,  2193,  2194,  2195,  2196,  2200,
    2206,  2215,  2228,  2229,  2232,  2235,  2238,  2239,  2242,  2246,
    2249,  2252,  2259,  2260,  2264,  2265,  2267,  2271,  2272,  2273,
    2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,
    2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,
    2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,
    2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,
    2314,  2315,  2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,
    2324,  2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,  2333,
    2334,  2335,  2336,  2337,  2338,  2339,  2340,  2341,  2342,  2343,
    2344,  2345,  2346,  2347,  2348,  2349,  2350,  2351,  2355,  2360,
    2361,  2365,  2366,  2367,  2368,  2370,  2374,  2375,  2386,  2387,
    2389,  2401,  2402,  2403,  2407,  2408,  2409,  2413,  2414,  2415,
    2418,  2420,  2424,  2425,  2426,  2427,  2429,  2430,  2431,  2432,
    2433,  2434,  2435,  2436,  2437,  2438,  2441,  2446,  2447,  2448,
    2450,  2451,  2453,  2454,  2455,  2456,  2458,  2460,  2462,  2464,
    2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,  2475,
    2476,  2477,  2478,  2479,  2480,  2481,  2482,  2484,  2486,  2488,
    2490,  2491,  2494,  2495,  2499,  2503,  2505,  2509,  2512,  2515,
    2521,  2522,  2523,  2524,  2525,  2526,  2527,  2532,  2534,  2538,
    2539,  2542,  2543,  2547,  2550,  2552,  2554,  2558,  2559,  2560,
    2561,  2564,  2568,  2569,  2570,  2571,  2575,  2577,  2584,  2585,
    2586,  2587,  2588,  2589,  2591,  2592,  2597,  2599,  2602,  2605,
    2607,  2609,  2612,  2614,  2618,  2620,  2623,  2626,  2632,  2634,
    2637,  2638,  2643,  2646,  2650,  2650,  2655,  2658,  2659,  2663,
    2664,  2668,  2669,  2670,  2674,  2676,  2684,  2685,  2689,  2691,
    2699,  2700,  2704,  2705,  2710,  2712,  2717,  2728,  2742,  2754,
    2769,  2770,  2771,  2772,  2773,  2774,  2775,  2785,  2794,  2796,
    2798,  2802,  2803,  2804,  2805,  2806,  2822,  2823,  2825,  2834,
    2835,  2836,  2837,  2838,  2839,  2840,  2841,  2843,  2848,  2852,
    2853,  2857,  2860,  2867,  2871,  2880,  2887,  2889,  2895,  2897,
    2898,  2902,  2903,  2910,  2911,  2916,  2917,  2922,  2923,  2924,
    2925,  2936,  2939,  2942,  2943,  2944,  2945,  2956,  2960,  2961,
    2962,  2964,  2965,  2966,  2970,  2972,  2975,  2977,  2978,  2979,
    2980,  2983,  2985,  2986,  2990,  2992,  2995,  2997,  2998,  2999,
    3003,  3005,  3008,  3011,  3013,  3015,  3019,  3020,  3022,  3023,
    3029,  3030,  3032,  3042,  3044,  3046,  3049,  3050,  3051,  3055,
    3056,  3057,  3058,  3059,  3060,  3061,  3062,  3063,  3064,  3065,
    3069,  3070,  3074,  3076,  3084,  3086,  3090,  3094,  3099,  3103,
    3111,  3112,  3116,  3117,  3123,  3124,  3133,  3134,  3142,  3145,
    3149,  3152,  3157,  3162,  3164,  3165,  3166,  3170,  3171,  3175,
    3176,  3179,  3184,  3187,  3189,  3193,  3199,  3200,  3201,  3205,
    3209,  3219,  3227,  3229,  3233,  3235,  3240,  3246,  3249,  3254,
    3262,  3265,  3268,  3269,  3272,  3275,  3276,  3281,  3284,  3288,
    3292,  3298,  3308,  3309
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
  "T_MINUS_EQUAL", "T_PLUS_EQUAL", "T_YIELD", "T_AWAIT", "T_YIELD_FROM",
  "'?'", "':'", "\"??\"", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'", "'^'",
  "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_SPACESHIP", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "T_POW", "'['",
  "T_CLONE", "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF",
  "T_LNUMBER", "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME",
  "T_VARIABLE", "T_NUM_STRING", "T_INLINE_HTML", "T_HASHBANG",
  "T_CHARACTER", "T_BAD_CHARACTER", "T_ENCAPSED_AND_WHITESPACE",
  "T_CONSTANT_ENCAPSED_STRING", "T_ECHO", "T_DO", "T_WHILE", "T_ENDWHILE",
  "T_FOR", "T_ENDFOR", "T_FOREACH", "T_ENDFOREACH", "T_DECLARE",
  "T_ENDDECLARE", "T_AS", "T_SUPER", "T_SWITCH", "T_ENDSWITCH", "T_CASE",
  "T_DEFAULT", "T_BREAK", "T_GOTO", "T_CONTINUE", "T_FUNCTION", "T_CONST",
  "T_RETURN", "T_TRY", "T_CATCH", "T_THROW", "T_USE", "T_GLOBAL",
  "T_PUBLIC", "T_PROTECTED", "T_PRIVATE", "T_FINAL", "T_ABSTRACT",
  "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET", "T_EMPTY", "T_HALT_COMPILER",
  "T_CLASS", "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS",
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
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_LAMBDA_OP", "T_LAMBDA_CP",
  "T_UNRESOLVED_OP", "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'",
  "']'", "'\"'", "'\\''", "$accept", "start", "$@1", "top_statement_list",
  "top_statement", "$@2", "$@3", "ident_no_semireserved",
  "ident_for_class_const", "ident", "group_use_prefix",
  "non_empty_use_declarations", "use_declarations", "use_declaration",
  "non_empty_mixed_use_declarations", "mixed_use_declarations",
  "mixed_use_declaration", "namespace_name", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "try_statement_list", "$@12", "additional_catches",
  "finally_statement_list", "$@13", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@14", "$@15", "$@16",
  "enum_declaration_statement", "$@17", "$@18",
  "class_declaration_statement", "$@19", "$@20", "$@21", "$@22",
  "class_expression", "$@23", "trait_declaration_statement", "$@24",
  "$@25", "class_decl_name", "interface_decl_name", "trait_decl_name",
  "class_entry_type", "extends_from", "implements_list",
  "interface_extends_list", "interface_list", "trait_list",
  "foreach_optional_arg", "foreach_variable", "for_statement",
  "foreach_statement", "while_statement", "declare_statement",
  "declare_list", "switch_case_list", "case_list", "case_separator",
  "elseif_list", "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@26", "$@27", "$@28",
  "$@29", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
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
  "yield_list_assign_expr", "yield_from_expr", "yield_from_assign_expr",
  "await_expr", "await_assign_expr", "await_list_assign_expr", "expr",
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@30",
  "$@31", "lambda_expression", "$@32", "$@33", "$@34", "$@35", "$@36",
  "lambda_body", "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name_base", "static_class_name_no_calls",
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
  "user_attribute_list", "$@37", "non_empty_user_attributes",
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
  "opt_return_type", "hh_constraint", "hh_typevar_list",
  "hh_non_empty_typevar_list", "hh_typevar_variance",
  "hh_shape_member_type", "hh_non_empty_shape_member_list",
  "hh_shape_member_list", "hh_shape_type", "hh_access_type_start",
  "hh_access_type", "hh_type", "hh_type_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
      63,    58,   283,   284,   285,   124,    94,    38,   286,   287,
     288,   289,    60,    62,   290,   291,   292,   293,   294,    43,
      45,    46,    42,    47,    37,    33,   295,   126,    64,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,    91,
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
     416,   417,   418,   419,   420,   421,   422,   423,    40,    41,
      59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   198,   200,   199,   201,   201,   202,   202,   202,   202,
     202,   202,   202,   202,   203,   202,   204,   202,   202,   202,
     202,   202,   202,   202,   202,   205,   205,   205,   205,   205,
     205,   205,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   207,   207,   208,   208,   209,
     209,   210,   211,   211,   211,   211,   212,   212,   213,   214,
     214,   214,   215,   215,   216,   216,   216,   217,   218,   219,
     219,   220,   220,   221,   221,   221,   221,   222,   222,   222,
     223,   222,   224,   222,   225,   222,   226,   222,   222,   222,
     222,   222,   222,   222,   222,   222,   222,   222,   222,   222,
     222,   222,   222,   227,   222,   228,   222,   222,   229,   222,
     230,   222,   222,   222,   222,   222,   222,   222,   222,   222,
     222,   222,   222,   222,   222,   222,   232,   231,   233,   233,
     235,   234,   236,   236,   237,   237,   238,   240,   239,   241,
     239,   242,   239,   244,   243,   245,   243,   247,   246,   248,
     246,   249,   246,   250,   246,   252,   251,   254,   253,   255,
     253,   256,   256,   257,   258,   259,   259,   259,   259,   259,
     260,   260,   261,   261,   262,   262,   263,   263,   264,   264,
     265,   265,   266,   266,   266,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   272,   272,   273,
     273,   273,   274,   274,   275,   275,   276,   276,   277,   277,
     278,   278,   279,   279,   279,   279,   279,   279,   279,   279,
     280,   280,   280,   280,   280,   280,   280,   280,   281,   281,
     281,   281,   281,   281,   281,   281,   282,   282,   282,   282,
     282,   282,   282,   282,   283,   283,   284,   284,   284,   284,
     284,   284,   285,   285,   286,   286,   286,   287,   287,   287,
     287,   288,   288,   289,   290,   291,   291,   293,   292,   294,
     292,   292,   292,   292,   295,   292,   296,   292,   292,   292,
     292,   292,   292,   292,   292,   297,   297,   297,   298,   299,
     299,   300,   300,   301,   301,   302,   302,   303,   303,   304,
     304,   304,   304,   304,   304,   304,   305,   305,   306,   307,
     307,   308,   308,   309,   309,   310,   311,   311,   311,   312,
     312,   312,   312,   313,   313,   313,   313,   313,   313,   313,
     314,   314,   314,   315,   315,   316,   316,   317,   317,   318,
     318,   319,   319,   320,   320,   320,   320,   320,   320,   320,
     321,   321,   322,   322,   322,   323,   323,   323,   323,   324,
     324,   325,   325,   326,   326,   327,   328,   328,   328,   328,
     328,   328,   329,   330,   330,   331,   331,   332,   332,   332,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   340,
     340,   340,   340,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   342,   342,   344,   343,   345,   343,
     347,   346,   348,   346,   349,   346,   350,   346,   351,   346,
     352,   352,   352,   353,   353,   354,   354,   355,   355,   356,
     356,   357,   357,   358,   359,   359,   360,   361,   362,   362,
     363,   363,   363,   363,   363,   364,   364,   364,   364,   365,
     366,   366,   367,   367,   368,   368,   369,   369,   370,   371,
     371,   372,   372,   372,   373,   373,   373,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   375,   376,
     376,   377,   377,   377,   377,   377,   378,   378,   379,   379,
     379,   380,   380,   380,   381,   381,   381,   382,   382,   382,
     383,   383,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   386,   386,   386,
     387,   387,   387,   387,   387,   387,   387,   388,   388,   389,
     389,   390,   390,   391,   391,   391,   391,   392,   392,   392,
     392,   392,   393,   393,   393,   393,   394,   394,   395,   395,
     395,   395,   395,   395,   395,   395,   396,   396,   397,   397,
     397,   397,   398,   398,   399,   399,   400,   400,   401,   401,
     402,   402,   403,   403,   405,   404,   406,   407,   407,   408,
     408,   409,   409,   409,   410,   410,   411,   411,   412,   412,
     413,   413,   414,   414,   415,   415,   416,   416,   417,   417,
     418,   418,   418,   418,   418,   418,   418,   418,   418,   418,
     418,   419,   419,   419,   419,   419,   419,   419,   419,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   421,   422,
     422,   423,   423,   424,   424,   424,   425,   425,   426,   426,
     426,   427,   427,   428,   428,   429,   429,   430,   430,   430,
     430,   430,   430,   431,   431,   431,   431,   431,   432,   432,
     432,   432,   432,   432,   433,   433,   434,   434,   434,   434,
     434,   434,   434,   434,   435,   435,   436,   436,   436,   436,
     437,   437,   438,   438,   438,   438,   439,   439,   439,   439,
     440,   440,   440,   440,   440,   440,   441,   441,   441,   442,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     443,   443,   444,   444,   445,   445,   446,   446,   446,   446,
     447,   447,   448,   448,   449,   449,   450,   450,   451,   451,
     452,   452,   453,   454,   454,   454,   454,   455,   455,   456,
     456,   457,   458,   458,   458,   458,   459,   459,   459,   460,
     460,   460,   461,   461,   462,   462,   463,   464,   465,   465,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   467,   467
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     6,     7,     7,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     3,
       1,     2,     1,     2,     3,     4,     3,     1,     2,     1,
       2,     2,     1,     3,     1,     3,     2,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     3,     5,
       1,     1,     1,     0,     9,     0,    10,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     3,     2,     2,     0,     4,     9,     0,
       0,     4,     2,     0,     1,     0,     1,     0,     9,     0,
      10,     0,    11,     0,     9,     0,    10,     0,     8,     0,
       9,     0,     7,     0,     8,     0,     8,     0,     7,     0,
       8,     1,     1,     1,     1,     1,     2,     3,     3,     2,
       2,     0,     2,     0,     2,     0,     1,     3,     1,     3,
       2,     0,     1,     2,     4,     1,     4,     1,     4,     1,
       4,     1,     4,     3,     5,     3,     4,     4,     5,     5,
       4,     0,     1,     1,     4,     0,     5,     0,     2,     0,
       3,     0,     7,     8,     6,     2,     5,     6,     4,     0,
       4,     5,     7,     6,     6,     7,     9,     8,     6,     7,
       5,     2,     4,     5,     3,     0,     3,     4,     6,     5,
       5,     6,     8,     7,     2,     0,     1,     2,     2,     3,
       4,     4,     3,     1,     1,     2,     4,     3,     5,     1,
       3,     2,     0,     2,     3,     2,     0,     0,     4,     0,
       5,     2,     2,     2,     0,    10,     0,    11,     3,     3,
       3,     4,     4,     3,     5,     2,     2,     0,     6,     5,
       4,     3,     1,     1,     3,     4,     1,     2,     1,     1,
       5,     6,     1,     1,     4,     1,     1,     3,     2,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     3,     4,     3,     3,     3,     2,     2,
       1,     1,     3,     3,     1,     1,     0,     1,     2,     4,
       3,     3,     6,     2,     3,     2,     3,     6,     1,     1,
       1,     1,     1,     6,     3,     4,     6,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     3,     2,     1,     5,     0,     0,    12,     0,    13,
       0,     4,     0,     7,     0,     5,     0,     3,     0,     6,
       2,     2,     4,     1,     1,     5,     3,     5,     3,     2,
       0,     2,     0,     4,     4,     3,     4,     4,     4,     4,
       1,     1,     1,     1,     3,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     4,     0,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       3,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     4,
       3,     4,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     2,     2,     4,     3,     4,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     4,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     1,     1,     3,     1,     4,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     1,
       3,     3,     3,     0,     1,     1,     3,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     6,     7,
       1,     2,     1,     2,     1,     4,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     2,     2,
       2,     2,     4,     2,     5,     3,     1,     1,     0,     3,
       4,     5,     3,     1,     2,     0,     4,     1,     3,     2,
       2,     2,     1,     1,     1,     1,     3,     5,     6,     1,
       8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   427,     0,     0,   794,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   885,
       0,   873,   676,     0,   682,   683,   684,    25,   741,   861,
     151,   152,   685,     0,   132,     0,     0,     0,     0,    26,
       0,     0,     0,     0,   186,     0,     0,     0,     0,     0,
       0,   393,   394,   395,   398,   397,   396,     0,     0,     0,
       0,   215,     0,     0,     0,   689,   691,   692,   686,   687,
       0,     0,     0,   693,   688,     0,   660,    27,    28,    29,
      31,    30,     0,   690,     0,     0,     0,     0,   694,   399,
     528,     0,   150,   122,   865,   677,     0,     0,     4,   112,
     114,   740,     0,   659,     0,     6,   185,     7,     9,     8,
      10,     0,     0,   391,   440,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   438,   849,   850,   510,   509,   421,
     513,     0,   420,   821,   661,   668,     0,   743,   508,   390,
     824,   825,   836,   439,     0,     0,   442,   441,   822,   823,
     820,   856,   860,     0,   498,   742,    11,   398,   397,   396,
       0,     0,    31,     0,   112,   185,     0,   929,   439,   928,
       0,   926,   925,   512,     0,   428,   435,   433,     0,     0,
     480,   481,   482,   483,   507,   505,   504,   503,   502,   501,
     500,   499,   861,   685,   663,     0,     0,   949,   842,   661,
       0,   662,   462,     0,   460,     0,   889,     0,   750,   419,
     672,   205,     0,   949,   418,   671,   666,     0,   681,   662,
     868,   869,   875,   867,   673,     0,     0,   675,   506,     0,
       0,     0,     0,   424,     0,   130,   426,     0,     0,   136,
     138,     0,     0,   140,     0,    72,    71,    66,    65,    57,
      58,    49,    69,    80,    81,     0,    52,     0,    64,    56,
      62,    83,    75,    74,    47,    70,    90,    91,    48,    86,
      45,    87,    46,    88,    44,    92,    79,    84,    89,    76,
      77,    51,    78,    82,    43,    73,    59,    93,    67,    60,
      50,    42,    41,    40,    39,    38,    37,    61,    94,    96,
      54,    35,    36,    63,   983,   984,    55,   989,    34,    53,
      85,     0,     0,   112,    95,   940,   982,     0,   985,     0,
       0,   142,     0,     0,     0,   176,     0,     0,     0,     0,
       0,     0,   752,     0,   100,   102,   304,     0,     0,   303,
       0,   219,     0,   216,   309,     0,     0,     0,     0,     0,
     946,   201,   213,   881,   885,     0,   910,     0,   696,     0,
       0,     0,   908,     0,    16,     0,   116,   193,   207,   214,
     565,   540,     0,   934,   520,   522,   524,   798,   427,   440,
       0,     0,   438,   439,   441,     0,     0,   678,     0,   679,
       0,     0,     0,   175,     0,     0,   118,   295,     0,    24,
     184,     0,   212,   197,   211,   396,   399,   185,   392,   165,
     166,   167,   168,   169,   171,   172,   174,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   873,     0,   164,   864,   864,   895,
       0,     0,     0,     0,     0,     0,     0,     0,   389,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   461,   459,   799,   800,     0,   864,     0,   812,
     295,   295,   864,     0,   866,   857,   881,     0,   185,     0,
       0,   144,     0,   796,   791,   750,     0,   440,   438,     0,
     893,     0,   545,   749,   884,   681,   440,   438,   439,   118,
       0,   295,   417,     0,   814,   674,     0,   122,   255,     0,
     527,     0,   147,     0,     0,   425,     0,     0,     0,     0,
       0,   139,   163,   141,   983,   984,   980,   981,     0,   975,
       0,     0,     0,     0,    68,    33,    55,    32,   941,   170,
     173,   143,   122,     0,   160,   162,     0,     0,     0,     0,
     103,     0,   751,   101,    18,     0,    97,     0,   305,     0,
     145,   218,   217,     0,     0,   146,   930,     0,     0,   440,
     438,   439,   442,   441,     0,   968,   225,     0,   882,     0,
       0,   148,     0,     0,   695,   909,   741,     0,     0,   907,
     746,   906,   115,     5,    13,    14,     0,   223,     0,     0,
     533,     0,     0,     0,   750,     0,     0,   669,   664,   534,
       0,     0,     0,     0,   798,   122,     0,   752,   797,   993,
     416,   430,   494,   830,   848,   127,   121,   123,   124,   125,
     126,   390,     0,   511,   744,   745,   113,   750,     0,   950,
       0,     0,     0,   752,   296,     0,   516,   187,   221,     0,
     465,   467,   466,     0,     0,   497,   463,   464,   468,   470,
     469,   485,   484,   487,   486,   488,   490,   492,   491,   489,
     479,   478,   472,   473,   471,   474,   475,   477,   493,   476,
     863,     0,     0,   899,     0,   750,   933,     0,   932,   949,
     827,   856,   203,   195,   209,     0,   934,   199,   185,     0,
     431,   434,   436,   444,   458,   457,   456,   455,   454,   453,
     452,   451,   450,   449,   448,   447,   802,     0,   801,   804,
     826,   808,   949,   805,     0,     0,     0,     0,     0,     0,
       0,     0,   927,   429,   789,   793,   749,   795,     0,   665,
       0,   888,     0,   887,   221,     0,   665,   872,   871,     0,
       0,   801,   804,   870,   805,   422,   257,   259,   122,   531,
     530,   423,     0,   122,   239,   131,   426,     0,     0,     0,
       0,     0,   251,   251,   137,   750,     0,     0,     0,   973,
     750,     0,   956,     0,     0,     0,     0,     0,   748,     0,
     660,     0,     0,   698,   659,   703,     0,   697,   120,   702,
     949,   986,     0,     0,     0,     0,    19,     0,    20,     0,
      98,     0,     0,     0,   109,   752,     0,   107,   102,    99,
     104,     0,   302,   310,   307,     0,     0,   919,   924,   921,
     920,   923,   922,    12,   966,   967,     0,   750,     0,     0,
       0,   881,   878,     0,   544,   918,   917,   916,     0,   912,
       0,   913,   915,     0,     5,     0,     0,     0,   559,   560,
     568,   567,     0,   438,     0,   749,   539,   543,     0,     0,
     935,     0,   521,     0,     0,   957,   798,   281,   992,     0,
       0,   813,     0,   862,   749,   952,   948,   297,   298,   658,
     751,   294,     0,   798,     0,     0,   223,   518,   189,   496,
       0,   548,   549,     0,   546,   749,   894,     0,     0,   295,
     225,     0,   223,     0,     0,   221,     0,   873,   445,     0,
       0,   810,   811,   828,   829,   858,   859,     0,     0,     0,
     777,   757,   758,   759,   766,     0,     0,     0,   770,   768,
     769,   783,   750,     0,   791,   892,   891,     0,   223,     0,
     815,   680,     0,   261,     0,     0,   128,     0,     0,     0,
       0,     0,     0,     0,   231,   232,   243,     0,   122,   241,
     157,   251,     0,   251,     0,   749,     0,     0,     0,     0,
     749,   974,   976,   955,   750,   954,     0,   750,   724,   725,
     722,   723,   756,     0,   750,   748,     0,   542,     0,     0,
     901,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   979,
     177,     0,   180,   161,     0,     0,   105,   110,   111,   103,
     751,   108,     0,   306,     0,   931,   149,   947,   968,   961,
     963,   224,   226,   316,     0,     0,   879,     0,   911,     0,
      17,     0,   934,   222,   316,     0,     0,   665,   536,     0,
     670,   936,     0,   957,   525,     0,     0,   993,     0,   286,
     284,   804,   816,   949,   804,   817,   951,     0,     0,   299,
     119,     0,   798,   220,     0,   798,     0,   495,   898,   897,
       0,   295,     0,     0,     0,     0,     0,     0,   223,   191,
     681,   803,   295,     0,   762,   763,   764,   765,   771,   772,
     781,     0,   750,     0,   777,     0,   761,   785,   749,   788,
     790,   792,     0,   886,     0,   803,     0,     0,     0,     0,
     258,   532,   133,     0,   426,   231,   233,   881,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,   987,
       0,   969,     0,   972,   749,     0,     0,     0,   700,   749,
     747,     0,   738,     0,   750,     0,   704,   739,   737,   905,
       0,   750,   707,   709,   708,     0,     0,   705,   706,   710,
     712,   711,   727,   726,   729,   728,   730,   732,   734,   733,
     731,   720,   719,   714,   715,   713,   716,   717,   718,   721,
     978,     0,   122,     0,     0,   106,    21,   308,     0,     0,
       0,   965,     0,   390,   883,   881,   432,   437,   443,     0,
      15,     0,   390,   571,     0,     0,   573,   566,   569,     0,
     564,     0,   938,     0,   958,   529,     0,   287,     0,     0,
     282,     0,   301,   300,   957,     0,   316,     0,   798,     0,
     295,     0,   854,   316,   934,   316,   937,     0,     0,     0,
     446,     0,     0,   774,   749,   776,   767,     0,   760,     0,
       0,   750,   782,   890,   316,     0,   122,     0,   254,   240,
       0,     0,     0,   230,   153,   244,     0,     0,   247,     0,
     252,   253,   122,   246,   988,   970,     0,   953,     0,   991,
     755,   754,   699,     0,   749,   541,   701,     0,   547,   749,
     900,   736,     0,     0,     0,    22,    23,   962,   959,   960,
     227,     0,     0,     0,   397,   388,     0,     0,     0,   202,
     315,   317,     0,   387,     0,     0,     0,   934,   390,     0,
     914,   312,   208,   562,     0,     0,   535,   523,     0,   290,
     280,     0,   283,   289,   295,   515,   957,   390,   957,     0,
     896,     0,   853,   390,     0,   390,   939,   316,   798,   851,
     780,   779,   773,     0,   775,   749,   784,   390,   122,   260,
     129,   134,   155,   234,     0,   242,   248,   122,   250,   971,
       0,     0,   538,     0,   904,   903,   735,   122,   181,   964,
       0,     0,     0,   942,     0,     0,     0,   228,     0,   934,
       0,   353,   349,   355,   660,    31,     0,   343,     0,   348,
     352,   365,     0,   363,   368,     0,   367,     0,   366,     0,
     185,   319,     0,   321,     0,   322,   323,     0,     0,   880,
       0,   563,   561,   572,   570,   291,     0,     0,   278,   288,
       0,     0,   957,     0,   198,   515,   957,   855,   204,   312,
     210,   390,     0,     0,   787,     0,   206,   256,     0,     0,
     122,   237,   154,   249,   990,   753,     0,     0,     0,     0,
       0,   415,     0,   943,     0,   333,   337,   412,   413,   347,
       0,     0,     0,   328,   624,   623,   620,   622,   621,   641,
     643,   642,   612,   582,   584,   583,   602,   618,   617,   578,
     589,   590,   592,   591,   611,   595,   593,   594,   596,   597,
     598,   599,   600,   601,   603,   604,   605,   606,   607,   608,
     610,   609,   579,   580,   581,   585,   586,   588,   626,   627,
     636,   635,   634,   633,   632,   631,   619,   638,   628,   629,
     630,   613,   614,   615,   616,   639,   640,   644,   646,   645,
     647,   648,   625,   650,   649,   652,   654,   653,   587,   657,
     655,   656,   651,   637,   577,   360,   574,     0,   329,   381,
     382,   380,   373,     0,   374,   330,   407,     0,     0,     0,
       0,   411,     0,   185,   194,   311,     0,     0,     0,   279,
     293,   852,     0,     0,   383,   122,   188,   957,     0,     0,
     200,   957,   778,     0,   122,   235,   135,   156,     0,   537,
     902,   179,   331,   332,   410,   229,     0,   750,   750,     0,
     356,   344,     0,     0,     0,   362,   364,     0,     0,   369,
     376,   377,   375,     0,     0,   318,   944,     0,     0,     0,
     414,     0,   313,     0,   292,     0,   557,   752,   122,     0,
       0,   190,   196,     0,   786,     0,     0,   158,   334,   112,
       0,   335,   336,     0,   749,     0,   749,   358,   354,   359,
     575,   576,     0,   345,   378,   379,   371,   372,   370,   408,
     405,   968,   324,   320,   409,     0,   314,   558,   751,     0,
       0,   384,   122,   192,     0,   238,     0,   183,     0,   390,
       0,   350,   357,   361,     0,     0,   798,   326,     0,   555,
     514,   517,     0,   236,     0,     0,   159,   341,     0,   389,
     351,   406,   945,     0,   752,   401,   798,   556,   519,     0,
     182,     0,     0,   340,   957,   798,   265,   402,   403,   404,
     993,   400,     0,     0,     0,   339,     0,   401,     0,   957,
       0,   338,   385,   122,   325,   993,     0,   270,   268,     0,
     122,     0,     0,   271,     0,     0,   266,   327,     0,   386,
       0,   274,   264,     0,   267,   273,   178,   275,     0,     0,
     262,   272,     0,   263,   277,   276
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   108,   874,   613,   174,  1433,   709,
     341,   342,   343,   344,   835,   836,   837,   110,   111,   112,
     113,   114,   395,   646,   647,   534,   244,  1498,   540,  1414,
    1499,  1737,   824,   336,   562,  1697,  1053,  1232,  1756,   411,
     175,   648,   914,  1116,  1289,   118,   616,   931,   649,   668,
     935,   596,   930,   224,   515,   650,   617,   932,   413,   361,
     378,   121,   916,   877,   860,  1071,  1436,  1169,   984,  1646,
    1502,   785,   990,   539,   794,   992,  1322,   777,   973,   976,
    1158,  1763,  1764,   636,   637,   662,   663,   348,   349,   355,
    1470,  1625,  1626,  1243,  1360,  1459,  1619,  1746,  1766,  1656,
    1701,  1702,  1703,  1446,  1447,  1448,  1449,  1658,  1659,  1665,
    1713,  1452,  1453,  1457,  1612,  1613,  1614,  1636,  1794,  1361,
    1362,   176,   123,  1780,  1781,  1617,  1364,  1365,  1366,  1367,
     124,   237,   535,   536,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,  1482,   135,   913,  1115,   136,   633,
     634,   635,   241,   387,   530,   623,   624,  1194,   625,  1195,
     137,   138,   139,   815,   140,   141,  1687,   142,   618,  1472,
     619,  1085,   882,  1260,  1257,  1605,  1606,   143,   144,   145,
     227,   146,   228,   238,   398,   522,   147,  1012,   819,   148,
    1013,   905,   573,  1014,   959,  1138,   960,  1140,  1141,  1142,
     962,  1300,  1301,   963,   755,   505,   188,   189,   651,   639,
     486,  1101,  1102,   741,   742,   901,   150,   230,   151,   152,
     178,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     701,   163,   234,   235,   599,   217,   218,   704,   705,  1200,
    1201,   371,   372,   868,   164,   587,   165,   632,   166,   327,
    1627,  1677,   362,   406,   657,   658,  1006,  1096,  1241,   856,
     857,   858,   799,   800,   801,   328,   329,   821,  1435,   899
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1283
static const yytype_int16 yypact[] =
{
   -1283,   154, -1283, -1283,  5723, 13133, 13133,   -30, 13133, 13133,
   13133, 10988, 13133, 13133, -1283, 13133, 13133, 13133, 13133, 13133,
   13133, 13133, 13133, 13133, 13133, 13133, 13133, 15627, 15627, 11183,
   13133, 16080,   -25,    -1, -1283, -1283, -1283, -1283, -1283,   175,
   -1283, -1283,   128, 13133, -1283,    -1,   201,   233,   329, -1283,
      -1, 11378,  1521, 11573, -1283, 13943,  3657,    -6, 13133,  1303,
      65, -1283, -1283, -1283,   248,   440,    68,   333,   363,   374,
     396, -1283,  1521,   398,   405, -1283, -1283, -1283, -1283, -1283,
   13133,   584,   869, -1283, -1283,  1521, -1283, -1283, -1283, -1283,
    1521, -1283,  1521, -1283,   189,   420,  1521,  1521, -1283,   448,
   -1283, 11768, -1283, -1283,   343,   493,   512,   512, -1283,   579,
     460,   391,   432, -1283,    92, -1283,   576, -1283, -1283, -1283,
   -1283,  1677,   737, -1283, -1283,   441,   462,   481,   483,   485,
     490,   498,   503, 10972, -1283, -1283, -1283, -1283,    47, -1283,
     629,   634, -1283,    42,   515, -1283,   563,   213, -1283,  2422,
      90, -1283, -1283,  2377,   104,   535,   145, -1283,   150,   102,
     537,   155, -1283,   300, -1283,   658, -1283, -1283, -1283,   580,
     541,   590, -1283, 13133, -1283,   576,   737, 16546,  3559, 16546,
   13133, 16546, 16546, 13521,   574, 15120, 13521, 16546,   719,  1521,
     707,   707,   531,   707,   707,   707,   707,   707,   707,   707,
     707,   707, -1283, -1283, -1283,    49, 13133,   610, -1283, -1283,
     638,   599,   456,   608,   456, 15627, 15330,   605,   792, -1283,
     580, -1283, 13133,   610, -1283,   651, -1283,   657,   624, -1283,
     153, -1283, -1283, -1283,   456,   104, 11963, -1283, -1283, 13133,
    9038,   805,    93, 16546, 10013, -1283, 13133, 13133,  1521, -1283,
   -1283, 11362,   627, -1283, 11947, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283,   465, -1283,   465, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283,    85,    74,   590, -1283, -1283, -1283,
   -1283,   631,  2272,    86, -1283, -1283,   662,   806, -1283,   671,
   14664, -1283,   636,   639, 13117, -1283,   226, 13312,  1744,  1744,
    1521,   644,   818,   650, -1283,    56, -1283, 15348,    95, -1283,
     714, -1283,   720, -1283,   829,    97, 15627, 13133, 13133,   661,
     673, -1283, -1283, 15441, 11183,   106,   558,   263, -1283, 13328,
   15627,   594, -1283,  1521, -1283,    -2,   460, -1283, -1283, -1283,
   -1283, 16173,   839,   762, -1283, -1283, -1283,    34, 13133,   677,
     678, 16546,   679,  2323,   680,  5918, 13133,   425,   681,   642,
     425,   455,   453, -1283,  1521,   465,   683, 10208, 13943, -1283,
   -1283,  1154, -1283, -1283, -1283, -1283, -1283,   576, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, 13133, 13133, 13133,
   12158, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133,
   13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133,
   13133, 13133, 13133, 13133, 16266, 13133, -1283, 13133, 13133, 13133,
    5125,  1521,  1521,  1521,  1521,  1521,  1677,   763,  1309,  4898,
   13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133, 13133,
   13133, 13133, -1283, -1283, -1283, -1283,   625, 13133, 13133, -1283,
   10208, 10208, 13133, 13133,   343,   158, 15441,   685,   576, 12353,
   14554, -1283, 13133, -1283,   689,   877,   728,   699,   700, 13523,
     456, 12548, -1283, 12743, -1283,   624,   704,   706,  2710, -1283,
     308, 10208, -1283,   857, -1283, -1283, 14601, -1283, -1283, 10403,
   -1283, 13133, -1283,   808,  9233,   889,   717, 16428,   900,   101,
      66, -1283, -1283, -1283,   743, -1283, -1283, -1283,   465,  1196,
     734,   914, 15255,  1521, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283,   738, -1283, -1283,   736,   744,   742,   751,
      61,  1544,  1785, -1283, -1283,  1521,  1521, 13133,   456,    65,
   -1283, -1283, -1283, 15255,   860, -1283,   456,   121,   122,   755,
     756,  3201,   229,   758,   766,   346,   837,   771,   456,   124,
     786, -1283,   800,  1521, -1283, -1283,   909,  2526,    20, -1283,
   -1283, -1283,   460, -1283, -1283, -1283,   952,   853,   816,   214,
     842, 13133,   343,   856,   983,   809,   844, -1283,   158, -1283,
     465,   465,   987,   805,    34, -1283,   820,   998, -1283,   465,
     170, -1283,   458,   146, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283,   963,  2654, -1283, -1283, -1283, -1283,  1000,   836, -1283,
   15627, 13133,   824,  1005, 16546,  1009, -1283, -1283,   897,  1285,
   11558, 16636, 13521, 13133, 16499, 14390, 10188,  4525,  5261, 12525,
   12134, 12914, 12914, 12914, 12914,  2420,  2420,  2420,  2420,  2420,
    1481,  1481,   724,   724,   724,   531,   531,   531, -1283,   707,
   16546,   833,   840, 15789,   827,  1027,   -26, 13133,   -14,   610,
     217,   158, -1283, -1283, -1283,  1023,   762, -1283,   576,  4726,
   -1283, -1283, -1283, 13521, 13521, 13521, 13521, 13521, 13521, 13521,
   13521, 13521, 13521, 13521, 13521, 13521, -1283, 13133,   205,   159,
   -1283, -1283,   610,   375,   845,  3057,   850,   852,   848,  3140,
     126,   858, -1283, 16546,  1599, -1283,  1521, -1283,   170,   421,
   15627, 16546, 15627, 15836,   897,   170,   456,   167,   894,   862,
   13133, -1283,   174, -1283, -1283, -1283,  8843,   561, -1283, -1283,
   16546, 16546,    -1, -1283, -1283, -1283, 13133,   954, 15138, 15255,
    1521,  9428,   865,   866, -1283,  1043,   969,   931,   915, -1283,
    1060,   881,  3701,   465, 15255, 15255, 15255, 15255, 15255,   883,
     921,   887, 15255,   387,   924, -1283,   895, -1283,  3402, -1283,
     246, -1283,  6113,  1904,   902,  1785, -1283,  1785, -1283,  1521,
    1521,  1785,  1785,  1521, -1283,  1085,   904, -1283,    72, -1283,
   -1283,  3498, -1283,  3402,  1087, 15627,   912, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283,   934,  1090,  1521,  1904,
     922, 15441, 15534,  1098, -1283, -1283, -1283, -1283,   919, -1283,
   13133, -1283, -1283,  5333, -1283,   465,  1904,   928, -1283, -1283,
   -1283, -1283,  1106,   932, 13133, 16173, -1283, -1283,  5125,   935,
   -1283,   465, -1283,   936,  6308,  1100,   130, -1283, -1283,   112,
     625, -1283,   857, -1283,   465, -1283, -1283,   456, 16546, -1283,
   10598, -1283, 15255,    55,   944,  1904,   853, -1283, -1283, 14390,
   13133, -1283, -1283, 13133, -1283, 13133, -1283,  3945,   946, 10208,
     837,  1104,   853,   465,  1123,   897,  1521, 16266,   456,  4056,
     951, -1283, -1283,   147,   953, -1283, -1283,  1128,  1321,  1321,
    1599, -1283, -1283, -1283,  1093,   957,    54,   960, -1283, -1283,
   -1283, -1283,  1141,   965,   689,   456,   456, 12938,   853,   857,
   -1283, -1283,  4226,   566,    -1, 10013, -1283,  6503,   966,  6698,
     968, 15138, 15627,   967,  1026,   456,  3402,  1148, -1283, -1283,
   -1283, -1283,   615, -1283,    62,   465,   988,  1031,   465,  1521,
    1196, -1283, -1283, -1283,  1164, -1283,   986,  1000,   505,   505,
    1109,  1109, 15942,   984,  1171, 15255, 14944, 16173,  2842, 14804,
   15255, 15255, 15255, 15255, 15045, 15255, 15255, 15255, 15255, 15255,
   15255, 15255, 15255, 15255, 15255, 15255, 15255, 15255, 15255, 15255,
   15255, 15255, 15255, 15255, 15255, 15255, 15255, 15255,  1521, -1283,
   -1283,  1099, -1283, -1283,   991,   993, -1283, -1283, -1283,   255,
    1544, -1283,   996, -1283, 15255,   456, -1283, -1283,    91, -1283,
     554,  1178, -1283, -1283,   129,  1003,   456, 10793, -1283,  2247,
   -1283,  5528,   762,  1178, -1283,     5,   215, -1283, 16546,  1058,
    1006, -1283,  1008,  1100, -1283,   465,   805,   465,   428,  1179,
    1115,   176, -1283,   610,   177, -1283, -1283, 15627, 13133, 16546,
    3402,  1011,    55, -1283,  1012,    55,  1014, 14390, 16546, 15895,
    1022, 10208,  1024,  1020,   465,  1021,  1029,   465,   853, -1283,
     624,   409, 10208, 13133, -1283, -1283, -1283, -1283, -1283, -1283,
    1082,  1025,  1208,  1132,  1599,  1072, -1283, 16173,  1599, -1283,
   -1283, -1283, 15627, 16546,  1032, -1283,    -1,  1194,  1151, 10013,
   -1283, -1283, -1283,  1039, 13133,  1026,   456, 15441, 15138,  1041,
   15255,  6893,   703,  1042, 13133,    60,   224, -1283,  1056, -1283,
     465, -1283,  1101, -1283,  3982,  1202,  1050, 15255, -1283, 15255,
   -1283,  1052, -1283,  1108,  1235,  1057, -1283, -1283, -1283, 15997,
    1053,  1238,  2948, 16679,  5123, 15255, 16593, 10578, 11162, 12331,
   12720, 13657, 14798, 14798, 14798, 14798,  3544,  3544,  3544,  3544,
    3544,   901,   901,   505,   505,   505,  1109,  1109,  1109,  1109,
   -1283,  1059, -1283,  1061,  1062, -1283, -1283,  3402,  1521,   465,
     465, -1283,  1904,    88, -1283, 15441, -1283, -1283, 13521,  1067,
   -1283,  1069,  1210, -1283,   261, 13133, -1283, -1283, -1283, 13133,
   -1283, 13133, -1283,   805, -1283, -1283,   156,  1247,  1180, 13133,
   -1283,  1078,   456, 16546,  1100,  1079, -1283,  1080,    55, 13133,
   10208,  1081, -1283, -1283,   762, -1283, -1283,  1091,  1086,  1083,
   -1283,  1094,  1599, -1283,  1599, -1283, -1283,  1095, -1283,  1138,
    1097,  1271, -1283,   456, -1283,  1251, -1283,  1102, -1283, -1283,
    1107,  1110,   135, -1283, -1283,  3402,  1103,  1105, -1283,  4710,
   -1283, -1283, -1283, -1283, -1283, -1283,   465, -1283,   465, -1283,
    3402, 16044, -1283, 15255, 16173, -1283, -1283, 15255, -1283, 15255,
   -1283, 14661, 15255,  1111,  7088, -1283, -1283,   554, -1283, -1283,
   -1283,   547, 14083,  1904,  1181, -1283,  1669,  1136,  1007, -1283,
   -1283, -1283,   763,  3221,   109,   110,  1113,   762,  1309,   136,
   -1283, -1283, -1283,  1142,  4317,  4403, 16546, -1283,   436,  1291,
    1224, 13133, -1283, 16546, 10208,  1197,  1100,  1435,  1100,  1122,
   16546,  1124, -1283,  1736,  1130,  1891, -1283, -1283,    55, -1283,
   -1283,  1188, -1283,  1599, -1283, 16173, -1283,  1913, -1283,  8843,
   -1283, -1283, -1283, -1283,  9623, -1283, -1283, -1283,  8843, -1283,
    1134, 15255,  3402,  1190,  3402, 16099, 14661, -1283, -1283, -1283,
    1904,  1904,  1521, -1283,  1312, 14944,    71, -1283, 14083,   762,
    2674, -1283,  1160, -1283,   111,  1146,   114, -1283, 14392, -1283,
   -1283, -1283,   115, -1283, -1283,   748, -1283,  1149, -1283,  1256,
     576, -1283, 14223, -1283, 14223, -1283, -1283,  1326,   763, -1283,
   13663, -1283, -1283, -1283, -1283,  1327,  1262, 13133, -1283, 16546,
    1156,  1159,  1100,   510, -1283,  1197,  1100, -1283, -1283, -1283,
   -1283,  2033,  1163,  1599, -1283,  1214, -1283,  8843,  9818,  9623,
   -1283, -1283, -1283,  8843, -1283,  3402, 15255, 15255,  7283,  1168,
    1172, -1283, 15255, -1283,  1904, -1283, -1283, -1283, -1283, -1283,
     465,  2081,  1669, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283,   357, -1283,  1136, -1283, -1283,
   -1283, -1283, -1283,    99,   351, -1283,  1340,   116, 14664,  1256,
    1349, -1283,   465,   576, -1283, -1283,  1174,  1352, 13133, -1283,
   16546, -1283,   143,  1177, -1283, -1283, -1283,  1100,   510, 13803,
   -1283,  1100, -1283,  1599, -1283, -1283, -1283, -1283,  7478,  3402,
    3402, -1283, -1283, -1283,  3402, -1283,  1354,  1363,  1368,  1186,
   -1283, -1283, 15255, 14392, 14392,  1324, -1283,   748,   748,   637,
   -1283, -1283, -1283, 15255,  1302, -1283,  1211,  1200,   117, 15255,
   -1283,  1521, -1283, 15255, 16546,  1307, -1283,  1382, -1283,  7673,
    1204, -1283, -1283,   510, -1283,  7868,  1203,  1283, -1283,  1297,
    1250, -1283, -1283,  1305,   465,  1232,  2081, -1283, -1283,  3402,
   -1283, -1283,  1243, -1283,  1375, -1283, -1283, -1283, -1283,  3402,
    1397,   346, -1283, -1283,  3402,  1226,  3402, -1283,   326,  1227,
    8063, -1283, -1283, -1283,  1229, -1283,  1245,  1246,  1521,  1309,
    1248, -1283, -1283, -1283, 15255,  1260,    64, -1283,  1338, -1283,
   -1283, -1283,  8258, -1283,  1904,   902, -1283,  1269,  1521,   641,
   -1283,  3402, -1283,  1249,  1431,   715,    64, -1283, -1283,  1359,
   -1283,  1904,  1252, -1283,  1100,    79, -1283, -1283, -1283, -1283,
     465, -1283,  1254,  1264,   119, -1283,   519,   715,   165,  1100,
    1263, -1283, -1283, -1283, -1283,   465,   447,  1437,  1374,   519,
   -1283,  8453,   195,  1443,  1376, 13133, -1283, -1283,  8648, -1283,
     457,  1447,  1383, 13133, -1283, 16546, -1283,  1455,  1388, 13133,
   -1283, 16546, 13133, -1283, 16546, 16546
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1283, -1283, -1283,  -544, -1283, -1283, -1283,   288,    43,   -33,
     400, -1283,  -277,  -505, -1283, -1283,   416,   141,  1616, -1283,
    2876, -1283,  -492, -1283,    30, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283,  -278, -1283, -1283,  -144,
     118,    19, -1283, -1283, -1283, -1283, -1283, -1283,    23, -1283,
   -1283, -1283, -1283, -1283, -1283,    24, -1283, -1283,  1033,  1028,
    1034,   -95,  -683,  -839,   557,   612,  -271,   337,  -908, -1283,
       7, -1283, -1283, -1283, -1283,  -722,   185, -1283, -1283, -1283,
   -1283,  -261, -1283,  -593, -1283,  -404, -1283, -1283,   930, -1283,
      18, -1283, -1283, -1020, -1283, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283,   -10, -1283,    73, -1283, -1283, -1283,
   -1283, -1283,   -87, -1283,   163,  -912, -1283, -1282,  -276, -1283,
    -140,   172,  -120,  -262, -1283,   -81, -1283, -1283, -1283,   187,
     -27,    -3,    29,  -702,   -75, -1283, -1283,    22, -1283,   -20,
   -1283, -1283,    -5,   -41,    57, -1283, -1283, -1283, -1283, -1283,
   -1283, -1283, -1283, -1283,  -576,  -835, -1283, -1283, -1283, -1283,
   -1283,  1767, -1283, -1283, -1283, -1283, -1283,   454, -1283, -1283,
   -1283, -1283, -1283, -1283, -1283, -1283,  -895, -1283,  2452,     1,
   -1283,  1347,  -391, -1283, -1283,  -470,  3672,  3604, -1283, -1283,
     528,  -163,  -598, -1283, -1283,   595,   404,  -633,   407, -1283,
   -1283, -1283, -1283, -1283,   596, -1283, -1283, -1283,   103,  -847,
     -44,  -418,  -410, -1283,   648,   -96, -1283, -1283,     6,     9,
     630, -1283, -1283,   142,   -22, -1283,  -352,   161,   188, -1283,
    -301, -1283, -1283, -1283,  -463,  1199, -1283, -1283, -1283, -1283,
   -1283,   732,   807, -1283, -1283, -1283,  -338,  -672, -1283,  1153,
    -960, -1283,    -7,  -171,   -31,   764, -1283,  -501,   221,  -157,
   -1283,   501,   570, -1283, -1283, -1283, -1283,   524,   476, -1038
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -978
static const yytype_int16 yytable[] =
{
     177,   179,   418,   181,   182,   183,   185,   186,   187,   467,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   325,   117,   216,   219,   390,   119,   120,   627,
     240,   497,   226,   750,   115,   776,   333,   231,   243,   897,
     232,   893,   245,   629,   934,   764,   251,   249,   254,  1097,
    1089,   334,   519,   337,   466,   514,   418,   892,   489,  1266,
     392,   567,   569,   698,  1252,   911,   834,   839,   739,   873,
     822,   994,   242,  1165,   389,   243,   740,  1114,   332,   394,
    1514,   968,    14,   -33,   980,   379,   746,   747,   -33,   382,
     383,  1320,  1351,  1125,   -68,   -32,   391,   792,   324,   -68,
     -32,   408,   531,    14,   579,   772,   584,   149,  1667,   365,
     790,  -831,    14,   773,   414,   531,  -550,   769,  1462,  1464,
    -346,   961,   116,  1522,  1607,  1674,  1674,    14,  1514,  1154,
     845,   531,   392,   862,  1668,   862,    14,   506,   862,   524,
     854,   855,  1145,   894,   862,   862,   389,   346,   -96,  1098,
     354,   394,   484,   485,     3,   404,   575,   702,   180,  -834,
     -95,   829,   -96,   236,  1253,   508,  1174,  1175,   391,   213,
     213,  -833,   575,   487,   -95,   500,   122,  1254,    14,   507,
    1685,   517,  1193,  -526,   394,   335,   744,   239,   614,   615,
     523,   748,   233,  1378,  1099,   516,  1255,  -553,  1352,   368,
     345,   391,  1796,  1353,  1146,    61,    62,    63,   167,  1354,
     415,  1355,   872,   576,  -552,  -838,  -837,   391,   830,  -832,
    -285,  -663,  -874,   375,   492,  1686,   376,   492,  -835,   404,
    -841,   526,  1810,  -831,   526,  -842,  -877,  1061,  1379,   487,
     417,   243,   537,  -876,  -285,  -818,  -819,  1797,  1356,  1357,
    1321,  1358,  1128,  -269,  1177,  -669,  1387,   793,   347,   548,
    1313,  1515,  1516,  1393,   -33,  1395,  -749,   879,  -751,  1172,
     528,  1176,   416,   669,   533,   -68,   -32,  1811,  -840,  1100,
    1359,  -834,   409,   532,  1407,   580,   977,   585,  1669,  1288,
     791,   979,   109,  -833,   498,   488,   601,   558,  -552,  1463,
    1465,  -346,   484,   485,  1523,  1608,  1675,  1723,  -664,  1791,
     846,   847,  1299,   863,  1373,   947,  -751,   590,  1244,  -751,
    1111,   468,   603,  1380,  1413,  1469,  1057,  1058,  1174,  1175,
    1081,   589,  1798,   491,  -844,  -847,   593,  -838,  -837,   563,
     252,  -832,   757,   323,  -874,   366,   493,   380,   418,   493,
    -835,   495,   243,   391,   751,   829,  1691,   213,  -877,   216,
     360,   488,  1812,  1748,   607,  -876,  -662,  -818,  -819,   350,
    -670,  1662,   880,   324,  1258,   325,   351,  1491,   377,  -806,
     360,  1670,   202,   185,   360,   360,   588,   881,  1663,   246,
     202,   652,  1434,  -806,   720,   854,   855,   564,  1074,  1048,
    1671,  -843,   664,  1672,   667,   929,  1259,  1664,  1749,   360,
    1251,  1733,   404,   604,   369,   370,  1323,   491,   851,   380,
     405,   247,   670,   671,   672,   674,   675,   676,   677,   678,
     679,   680,   681,   682,   683,   684,   685,   686,   687,   688,
     689,   690,   691,   692,   693,   694,   695,   696,   697,   722,
     699,   324,   700,   700,   703,   226,   379,   715,   716,   414,
     231,   886,  1310,   232,   723,   724,   725,   726,   727,   728,
     729,   730,   731,   732,   733,   734,   735,   504,  1517,   345,
     345,   570,   700,   745,  1104,   664,   664,   700,   749,   213,
     638,   721,  1105,   494,   723,   265,  1171,   753,   213,  1423,
     592,   104,  1620,   708,  1621,   213,   761,   366,   763,   779,
    1267,   467,   213,   116,   612,  1302,   664,   248,  1475,  1275,
    1265,   356,  1277,   267,   780,  1122,   781,  -554,   109,  1803,
     384,   330,   109,   627,   396,   366,   538,   366,   928,  1817,
    -949,   609,   926,   609,  -949,    37,  1130,   629,  1054,  -809,
    1055,   357,   324,   484,   485,   834,   466,  1044,  1045,  1046,
     352,   405,   358,  -809,   784,   405,    49,   122,   353,   628,
    1495,   940,   841,  1047,   936,   366,   369,   370,  -949,  -949,
     883,   397,  -949,  -807,   359,   718,   363,   454,   484,   485,
     484,   485,  1263,   364,   366,  1268,   900,  -807,   902,   455,
     400,   544,   545,  1476,   369,   370,   369,   370,   381,  -845,
     403,  -665,  1394,   410,  1804,   233,   391,   404,   557,   171,
     407,   710,    85,   317,  1818,    87,    88,   602,    89,   172,
      91,   419,   996,   385,   153,   974,   975,  1001,   213,   386,
    1156,  1157,   519,   321,   369,   370,  -845,   743,   711,  1049,
     655,   654,   420,   322,  1239,  1240,   908,   212,   214,  1400,
    1290,  1401,   918,   369,   370,   627,   366,  1716,   919,   970,
     710,   421,   367,   422,   711,   423,   366,  1430,  1431,   629,
     424,   768,   609,   109,   774,  1389,  1717,  1377,   425,  1718,
     484,   485,   656,   426,  1069,  1467,   323,   711,   457,   360,
    1634,  1635,   927,   458,  1312,    37,   459,   202,   711,  1792,
    1793,   711,   838,   838,   900,   902,   460,  1281,  1173,  1174,
    1175,   969,   902,   490,   366,  -839,    49,  -551,  1291,   496,
     609,   393,   939,  -663,   368,   369,   370,   638,   566,   568,
    1344,   546,  1788,   547,   610,   369,   370,   373,   557,   360,
     713,   360,   360,   360,   360,  1714,  1715,  1802,    61,    62,
      63,   167,   168,   415,   501,   972,   503,  1518,  1710,  1711,
    1494,  1004,  1007,  1385,   738,   455,   451,   452,   453,   978,
     454,   243,  1369,   736,   405,    87,    88,  -843,    89,   172,
      91,   509,   455,   369,   370,   627,   491,   557,   551,  1149,
     512,   513,   213,   393,  -661,  1492,  1317,  1174,  1175,   629,
     520,   771,   521,   529,  1409,  -977,   737,   542,   104,   549,
     552,   989,   109,   468,   553,   416,   559,   572,    37,   560,
    1418,  1773,  1777,  1778,  1779,   571,   393,   399,   401,   402,
     574,   820,   581,   583,  1186,   510,    54,   595,   582,    49,
     594,  1190,   518,   630,    61,    62,    63,   167,   168,   415,
    1642,   213,   631,   840,   656,  1079,   640,   641,   642,   644,
     153,  -117,    54,   666,   153,   653,  1391,   754,   604,  1088,
      37,   659,   865,   866,   330,  1483,   756,  1485,   758,   759,
     867,   869,   117,   765,   116,   766,   119,   120,   531,  1765,
     782,    49,   213,   115,   213,  1109,  1609,   786,    87,    88,
    1610,    89,   172,    91,   789,  1117,  1497,   548,  1118,  1765,
    1119,   416,   802,   803,   664,  1503,   823,   825,  1787,  1129,
     213,   708,  1271,   827,   826,  1508,  1455,    37,   226,   202,
     116,   828,   844,   231,   848,   849,   232,   852,   122,    37,
    1041,  1042,  1043,  1044,  1045,  1046,   853,   360,    49,   861,
      87,    88,  1153,    89,   172,    91,   838,   859,   838,  1047,
      49,  1159,   838,   838,  1059,   864,   149,   578,   870,  1295,
    1480,  1633,   627,   875,   876,  1638,   586,   213,   591,   878,
     884,   116,   885,   598,   122,  -685,   629,   888,   887,   638,
     608,   891,  1246,   213,   213,  1160,   895,   896,  1648,   904,
    1694,   906,   116,   909,   910,   736,   638,    87,    88,   924,
      89,   172,    91,   912,   795,   153,   373,   915,   921,    87,
      88,  1335,    89,   172,    91,   922,   925,   933,  1340,   943,
     941,   944,   958,   945,   964,   122,   917,  -667,   770,  1090,
     104,   971,   995,   627,   981,   991,   993,  1247,   997,  1192,
     374,   743,  1198,   774,   109,   998,   122,   629,   999,  1000,
    1002,  1015,  1248,   628,  1016,  1017,   711,  1019,   987,   109,
      61,    62,    63,    64,    65,   415,  1020,    37,   711,  1729,
     711,    71,   461,  1052,  1060,   116,  1062,   116,   233,  1068,
     117,  1064,  1066,  1273,   119,   120,   889,   890,    49,  1067,
     109,   115,  1077,  1073,  1078,   898,   664,  1056,   656,  1084,
    1086,  1087,  1093,   213,   213,  1091,   598,   664,  1248,   463,
     774,  1095,  1112,  1454,  1121,  1124,  1690,  1127,  1406,  1132,
    1693,  -846,  1133,  1689,  1143,  1144,  1070,   416,  1147,   122,
    1148,   122,  1695,  1305,  1150,  1167,  1162,   711,  1164,   243,
    1168,   109,  1170,  1179,   153,  1180,  1776,    87,    88,  1319,
      89,   172,    91,  1184,   605,  1185,   557,  1047,   611,  1188,
    1189,  1231,   109,  1233,   149,  1234,  1236,  1242,   738,  1308,
     771,  1245,  1261,  1269,   929,  1455,  1730,  1270,  1262,   116,
    1274,   838,  1278,  1276,   605,   628,   611,   605,   611,   611,
    1280,  1283,  1285,  1282,  1351,   638,  1292,  1294,   638,  1286,
    1293,   954,  1298,  1304,   360,  1306,   796,  1307,  1468,  1309,
    1314,  1324,  1318,  1328,    37,  1326,  1137,  1137,   958,  1329,
    1752,  1332,  1333,   418,  1334,  1338,  1336,  1339,  1343,   213,
    1374,  1345,  1346,   122,  1375,    49,  1376,   771,    14,  1370,
    1371,  1381,  1382,   109,  1383,   109,  1384,   109,  1386,  1388,
    1392,  1398,  1403,  1786,  1390,   664,    37,  1397,   659,   659,
    1405,  1396,  1408,  1399,  1402,   797,  1404,  1182,  1799,   116,
     907,  1438,  1410,  1415,   213,  1416,  1411,    49,  1451,  1412,
    1471,  1801,  1427,  1466,   557,  1477,  1478,   557,  1808,   213,
     213,  1486,  1481,  1487,    87,    88,  1618,    89,   172,    91,
    1352,  1489,  1493,  1504,  1506,  1353,  1512,    61,    62,    63,
     167,  1354,   415,  1355,  1520,   628,   820,  1521,  1616,  1615,
    1622,  1628,   666,   122,  1629,  1631,  1368,  1632,  1643,   938,
     171,  1082,  1641,    85,  1673,  1368,    87,    88,  1652,    89,
     172,    91,  1653,  1679,  1682,    37,  1683,  1092,  1688,   109,
    1356,  1357,  1704,  1358,   210,   210,  1479,  1706,  1708,   664,
    1106,   638,  1712,    37,  1720,  1721,    49,   213,  1722,  1727,
     965,  1728,   966,  1735,   416,  1732,  1736,  -342,  1134,  1135,
    1136,    37,  1372,  1738,    49,  1739,   153,  1741,  1743,  1126,
    1668,  1744,   338,   339,  1747,  1363,  1750,  1755,   985,  1753,
    1767,   153,    49,  1760,  1363,  1511,    61,    62,    63,   167,
     168,   415,   958,  1754,    37,  1762,   958,  1771,  1774,  1351,
    1775,  1783,  1785,  1789,  1501,    87,    88,   109,    89,   172,
      91,  1805,   153,  1790,  1800,    49,  1806,  1813,  1814,   109,
     340,  1819,   116,    87,    88,  1820,    89,   172,    91,  1822,
    1823,  1178,  1630,   917,  1181,  1065,  1235,  1770,  1513,  1681,
    1460,    87,    88,    14,    89,   172,    91,  1123,  1083,   712,
    1368,   598,  1076,   416,  1705,  1707,  1368,   714,  1368,   717,
    1784,   638,  1311,   153,  1417,  1782,  1647,  1639,   171,   842,
    1368,    85,  1661,  1519,    87,    88,   122,    89,   172,    91,
    1666,  1458,   628,  1807,   153,  1795,  1347,   116,  1645,  1501,
     448,   449,   450,   451,   452,   453,   116,   454,  1678,  1256,
     468,  1439,  1637,  1191,  1139,  1352,  1698,  1296,  1103,   455,
    1353,  1297,    61,    62,    63,   167,  1354,   415,  1355,  1363,
    1151,   665,   210,   600,  1745,  1363,  1005,  1363,  1429,  1238,
    1183,  1264,  1230,   898,     0,     0,     0,     0,     0,  1363,
     958,   122,   958,     0,     0,  1676,  1623,     0,     0,     0,
     122,     0,     0,   628,  1368,  1356,  1357,     0,  1358,  1758,
    1284,    37,     0,  1287,     0,   153,     0,   153,     0,   153,
       0,   985,  1166,     0,     0,   116,     0,     0,     0,   416,
       0,   116,    49,  1684,    37,     0,   116,  1484,     0,     0,
       0,     0,   109,     0,     0,     0,     0,     0,     0,   418,
     323,     0,     0,   207,   207,    49,  1456,   223,   948,   949,
       0,     0,     0,   831,   832,     0,  1325,     0,     0,     0,
    1106,   324,     0,  1363,     0,     0,     0,     0,   950,   122,
       0,   223,     0,     0,  1725,   122,   951,   952,   953,    37,
     122,    87,    88,     0,    89,   172,    91,     0,   954,     0,
       0,   958,     0,     0,   210,     0,     0,   109,     0,  1440,
      49,   833,   109,   210,    87,    88,   109,    89,   172,    91,
     210,   153,     0,     0,     0,  1348,  1349,   210,     0,     0,
     360,     0,     0,   557,     0,     0,   323,     0,   626,     0,
       0,     0,     0,     0,     0,   955,  1604,  1272,     0,     0,
    1351,     0,     0,  1611,     0,     0,     0,     0,   956,    37,
     323,     0,   323,     0,     0,     0,     0,    37,   323,    87,
      88,     0,    89,   172,    91,     0,   116,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,   957,    49,     0,
       0,   958,  1303,     0,    14,   109,   109,   109,     0,   153,
       0,   109,  1441,     0,   208,   208,   109,   598,   985,     0,
    1815,   153,  1419,     0,  1420,  1442,  1443,   116,  1821,     0,
       0,     0,     0,   116,  1824,     0,     0,  1825,     0,     0,
     122,     0,     0,   171,    37,     0,    85,  1444,     0,    87,
      88,   207,    89,  1445,    91,   412,     0,    87,    88,  1461,
      89,   172,    91,   210,     0,    49,  1352,     0,   116,   638,
       0,  1353,     0,    61,    62,    63,   167,  1354,   415,  1355,
       0,   122,     0,     0,     0,    37,     0,   122,     0,   638,
     116,     0,     0,     0,     0,   598,     0,     0,   638,     0,
       0,   223,     0,   223,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,  1351,  1356,  1357,     0,  1358,
       0,   340,   122,     0,    87,    88,   557,    89,   172,    91,
       0,  1759,     0,     0,     0,     0,     0,  1351,     0,   116,
     416,     0,     0,     0,   122,     0,   116,   323,  1488,     0,
       0,   958,     0,     0,     0,     0,   109,     0,   223,    14,
       0,     0,   833,     0,  1699,    87,    88,     0,    89,   172,
      91,  1604,  1604,     0,     0,  1611,  1611,     0,     0,     0,
       0,    14,     0,   207,     0,     0,     0,     0,     0,   360,
       0,     0,   207,   122,   153,     0,     0,   109,     0,   207,
     122,     0,   208,   109,    37,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1657,   223,     0,     0,
       0,  1352,     0,     0,     0,    49,  1353,   210,    61,    62,
      63,   167,  1354,   415,  1355,     0,     0,     0,   109,     0,
       0,   223,     0,  1352,   223,     0,  1757,     0,  1353,     0,
      61,    62,    63,   167,  1354,   415,  1355,  1351,     0,   153,
     109,     0,     0,     0,   153,     0,  1772,     0,   153,     0,
       0,  1356,  1357,     0,  1358,     0,     0,     0,   171,     0,
       0,    85,    86,     0,    87,    88,   210,    89,   172,    91,
     223,     0,     0,  1356,  1357,   416,  1358,     0,     0,     0,
       0,    14,     0,  1490,     0,     0,     0,     0,     0,   109,
       0,     0,     0,     0,     0,     0,   109,   416,  1680,     0,
       0,     0,     0,     0,     0,  1496,     0,   210,     0,   210,
       0,     0,   207,     0,   208,     0,     0,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,   153,   153,   153,
     208,     0,     0,   153,     0,   210,     0,   208,   153,     0,
       0,     0,     0,  1352,     0,     0,     0,     0,  1353,     0,
      61,    62,    63,   167,  1354,   415,  1355,     0,    34,    35,
      36,     0,     0,     0,   223,   223,     0,     0,   813,     0,
     203,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1740,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   210,  1356,  1357,     0,  1358,     0,     0,   813,
       0,     0,     0,     0,     0,     0,     0,     0,   210,   210,
       0,     0,     0,     0,     0,     0,     0,   416,     0,    75,
      76,    77,    78,    79,     0,  1640,     0,     0,     0,     0,
     205,     0,   626,     0,     0,     0,    83,    84,     0,     0,
       0,     0,     0,     0,     0,     0,   223,   223,     0,     0,
      93,     0,     0,     0,     0,   223,   898,   427,   428,   429,
       0,     0,     0,   208,    98,     0,     0,     0,     0,     0,
       0,   898,     0,     0,     0,     0,   207,   430,   153,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   265,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   455,     0,     0,     0,   153,
       0,     0,     0,     0,     0,   153,     0,     0,   210,   210,
     267,     0,     0,     0,     0,   207,     0,   499,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
     153,     0,     0,     0,   626,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,   207,     0,   207,     0,
       0,   550,   153,     0,     0,     0,     0,     0,     0,   482,
     483,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   207,   813,     0,     0,   544,   545,
       0,     0,     0,     0,     0,     0,     0,     0,   223,   223,
     813,   813,   813,   813,   813,     0,   171,   208,   813,    85,
     317,   153,    87,    88,     0,    89,   172,    91,   153,   223,
       0,     0,  1249,   482,   483,     0,     0,     0,     0,     0,
     321,     0,     0,     0,   210,   484,   485,     0,     0,     0,
     322,   207,  -978,  -978,  -978,  -978,  -978,   446,   447,   448,
     449,   450,   451,   452,   453,   223,   454,   207,   207,   209,
     209,     0,     0,   225,     0,     0,   208,     0,   455,     0,
       0,   223,   223,     0,   626,     0,     0,     0,     0,   210,
       0,   223,     0,     0,     0,     0,     0,   223,     0,   484,
     485,     0,   643,     0,   210,   210,     0,     0,     0,     0,
     223,     0,     0,     0,     0,     0,     0,   208,   813,   208,
       0,   223,     0,     0,     0,     0,   427,   428,   429,    61,
      62,    63,    64,    65,   415,     0,     0,     0,     0,   223,
      71,   461,     0,   223,     0,   208,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,     0,     0,   462,     0,   463,     0,
       0,     0,   210,     0,   455,     0,     0,   207,   207,     0,
       0,   464,     0,   465,     0,     0,   416,     0,     0,     0,
       0,   223,   208,     0,   223,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,   208,
       0,   813,     0,   223,     0,     0,   813,   813,   813,   813,
     813,   813,   813,   813,   813,   813,   813,   813,   813,   813,
     813,   813,   813,   813,   813,   813,   813,   813,   813,   813,
     813,   813,   813,   813,   427,   428,   429,   209,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     813,   626,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,   223,     0,   223,     0,     0,     0,     0,   871,     0,
       0,     0,   455,   207,   499,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,     0,     0,
     223,     0,     0,   223,     0,     0,     0,     0,   208,   208,
       0,     0,   626,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,   223,     0,     0,     0,     0,   207,     0,
       0,     0,     0,     0,     0,    49,   482,   483,     0,     0,
       0,     0,     0,   207,   207,     0,   813,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,  1441,     0,   209,
     223,     0,     0,   813,     0,   813,     0,     0,   209,     0,
    1442,  1443,     0,     0,     0,   209,     0,     0,     0,     0,
       0,   813,   209,     0,     0,     0,     0,     0,   171,     0,
       0,    85,    86,   209,    87,    88,     0,    89,  1445,    91,
       0,     0,   484,   485,     0,     0,   903,     0,     0,     0,
       0,     0,  1021,  1022,  1023,   223,   223,     0,   223,     0,
       0,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1024,     0,   208,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,     0,     0,   767,
       0,     0,     0,     0,     0,     0,   225,     0,     0,     0,
    1047,     0,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   326,     0,     0,   208,   208,     0,     0,     0,     0,
       0,     0,   223,     0,   223,     0,     0,     0,   209,   813,
     223,     0,     0,   813,     0,   813,     0,     0,   813,  1022,
    1023,     0,     0,     0,     0,     0,     0,     0,   223,   223,
       0,     0,   223,     0,     0,     0,     0,     0,  1024,   223,
       0,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,     0,   816,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,  1047,     0,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1196,     0,     0,     0,   816,     0,   813,     0,     0,
       0,     0,     0,     0,     0,     0,   223,   223,     0,     0,
       0,     0,     0,     0,   223,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
     223,     0,     0,     0,     0,     0,   223,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   209,   454,     0,     0,     0,     0,     0,     0,
       0,     0,   813,   813,     0,   455,     0,     0,   813,     0,
     223,     0,     0,     0,     0,     0,   223,     0,   223,     0,
       0,   326,     0,   326,     0,     0,     0,     0,     0,     0,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     430,   209,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,   209,     0,   209,   499,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
     209,   816,     0,     0,     0,     0,     0,     0,     0,   942,
       0,   265,     0,     0,     0,   223,   816,   816,   816,   816,
     816,     0,     0,     0,   816,     0,     0,   482,   483,     0,
       0,     0,   223,     0,     0,  1051,     0,     0,   813,   267,
       0,   326,     0,     0,   326,     0,     0,     0,     0,   813,
       0,     0,     0,     0,     0,   813,     0,   209,     0,   813,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1072,     0,   209,   209,     0,     0,     0,     0,     0,
     223,     0,    49,     0,     0,     0,     0,     0,  1072,     0,
    -389,     0,   946,   484,   485,     0,     0,   209,    61,    62,
      63,   167,   168,   415,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   544,   545,     0,
     813,     0,     0,     0,   816,     0,     0,  1113,     0,     0,
     223,     0,     0,     0,     0,   171,     0,     0,    85,   317,
       0,    87,    88,     0,    89,   172,    91,   223,     0,   225,
     850,     0,     0,     0,     0,     0,   223,     0,     0,   321,
       0,     0,     0,     0,     0,   416,     0,     0,     0,   322,
       0,   223,  1021,  1022,  1023,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,   798,     0,     0,   814,     0,
       0,     0,  1024,   209,   209,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,     0,     0,   814,
       0,     0,     0,     0,     0,     0,     0,   816,     0,   209,
    1047,     0,   816,   816,   816,   816,   816,   816,   816,   816,
     816,   816,   816,   816,   816,   816,   816,   816,   816,   816,
     816,   816,   816,   816,   816,   816,   816,   816,   816,   816,
       0,     0,     0,     0,     0,     0,   326,   326,   427,   428,
     429,     0,     0,     0,     0,   326,   816,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,   455,     0,     0,     0,
       0,     0,     0,   499,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,  -978,  -978,  -978,  -978,
    -978,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,   209,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,  1047,     0,     0,     0,     0,     0,     0,   209,
     209,     0,   816,     0,     0,   482,   483,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   816,
       0,   816,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   816,     0,     0,
       5,     6,     7,     8,     9,   814,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,   326,   326,
     814,   814,   814,   814,   814,    12,    13,     0,   814,     0,
    1063,   484,   485,     0,  1350,     0,     0,   209,     0,   211,
     211,     0,     0,   229,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,   265,     0,     0,    34,    35,    36,    37,    38,    39,
       0,     0,     0,     0,     0,     0,    42,     0,     0,     0,
       0,   326,     0,     0,     0,     0,     0,     0,    49,   267,
       0,     0,     0,     0,     0,     0,    54,   326,     0,     0,
       0,     0,     0,     0,    61,    62,    63,   167,   168,   169,
     326,    37,    68,    69,     0,   816,   209,     0,   814,   816,
       0,   816,   170,    74,   816,    75,    76,    77,    78,    79,
       0,     0,    49,     0,     0,  1437,    81,     0,  1450,   326,
       0,   171,    83,    84,    85,    86,     0,    87,    88,     0,
      89,   172,    91,     0,     0,     0,    93,     0,     0,    94,
       0,     0,     0,     0,     0,    95,     0,   544,   545,     0,
      98,    99,   100,     0,     0,   173,     0,   331,     0,     0,
     104,   105,     0,   106,   107,   171,     0,   209,    85,   317,
       0,    87,    88,     0,    89,   172,    91,     0,  1003,     0,
       0,   326,     0,   816,   326,     0,   798,     0,     0,   321,
       0,     0,  1509,  1510,     0,     0,     0,   211,     0,   322,
       0,   814,  1450,     0,     0,     0,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     814,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   427,   428,   429,   816,   816,
       0,     0,     0,     0,   816,     0,  1655,     0,     0,     0,
       0,   326,     0,   326,  1450,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     326,   454,     0,   326,     0,     0,     0,     0,     0,     0,
       0,     0,   265,   455,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,   211,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     267,     0,   211,     0,     0,     0,   814,     0,     0,     0,
       0,     0,     0,   229,     0,     0,   326,     0,     0,     0,
     326,     0,    37,   814,     0,   814,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   814,     0,    49,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,   816,   326,   326,     0,   544,   545,
       0,     0,     0,     0,   455,   816,   229,     0,     0,     0,
       0,   816,     0,     0,     0,   816,   171,  1120,     0,    85,
     317,     0,    87,    88,     0,    89,   172,    91,     0,  1327,
       0,     0,     0,     0,     0,     0,   818,     0,     0,     0,
     321,     0,     0,     0,     0,     0,     0,     0,   211,     0,
     322,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   843,     0,     0,
       0,     0,     0,     0,     0,     0,   816,     0,     0,     0,
       0,     0,   326,     0,   326,     0,  1769,     0,     0,   814,
       0,     0,     0,   814,     0,   814,     0,     0,   814,     0,
       0,     0,     0,  1437,   817,     0,     0,     0,   326,     0,
       0,     0,     0,     0,     0,     0,   427,   428,   429,   326,
       0,     0,     0,     0,     0,     0,     0,     0,  1131,     0,
       0,     0,     0,     0,     0,   817,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,     0,     0,   814,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,   211,     0,     0,     0,     0,     0,   326,     0,
     326,     0,     0,     0,     0,     0,   326,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,     0,   814,   814,     0,   455,     0,     0,   814,     0,
       0,   211,     0,   986,     0,     0,   326,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1008,  1009,
    1010,  1011,     0,   427,   428,   429,  1018,     0,  1155,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,   430,   211,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
     211,   817,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,     0,     0,     0,     0,   817,   817,   817,   817,
     817,     0,     0,     0,   817,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1473,
       0,     0,     0,     0,     0,   326,  1110,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1700,   211,   211,     0,     0,     0,   814,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   814,
       0,     0,     0,     0,     0,   814,     0,   229,     0,   814,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     326,   454,     0,     0,   817,     0,     0,     0,     0,     0,
       0,     0,     0,   455,     0,  1474,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     814,     0,     0,     0,  1199,  1202,  1203,  1204,  1206,  1207,
    1208,  1209,  1210,  1211,  1212,  1213,  1214,  1215,  1216,  1217,
    1218,  1219,  1220,  1221,  1222,  1223,  1224,  1225,  1226,  1227,
    1228,  1229,     0,   211,   211,     0,   326,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1237,     0,
       0,   326,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   817,     0,   229,
       0,     0,   817,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   817,     0,     0,     0,
     430,  1320,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,     0,     0,
       0,     0,     0,     0,  1315,     0,     0,     0,   455,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1330,     0,  1331,     0,    29,     0,   937,     0,     0,
       0,     0,     0,    34,    35,    36,    37,     0,   202,  1341,
       0,     0,     0,     0,     0,   203,     0,     0,     0,   229,
       0,     0,     0,     0,   211,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
     211,     0,   817,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   817,
       0,   817,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   205,     0,   817,     0,     0,
     171,    83,    84,    85,    86,     0,    87,    88,     0,    89,
     172,    91,     0,     0,     0,    93,     0,     0,     0,     0,
    1321,     5,     6,     7,     8,     9,     0,     0,     0,    98,
       0,    10,     0,     0,   206,     0,     0,   211,     0,   104,
       0,     0,     0,     0,     0,   388,    12,    13,     0,     0,
       0,     0,     0,     0,     0,   719,     0,  1422,     0,     0,
       0,  1424,     0,  1425,     0,     0,  1426,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,   817,   229,    54,     0,   817,
       0,   817,     0,     0,   817,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,  1505,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,   229,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   817,   106,   107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1649,  1650,     0,     0,     0,     0,  1654,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,   256,
       0,   257,   258,     0,     0,   259,   260,   261,   262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,  1024,   264,     0,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,   817,   817,
       0,   266,     0,     0,   817,     0,     0,     0,     0,     0,
       0,  1047,     0,  1660,     0,   268,   269,   270,   271,   272,
     273,   274,     0,     0,     0,    37,     0,   202,     0,     0,
       0,     0,     0,     0,     0,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,    49,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
       0,     0,     0,   706,   310,   311,   312,     0,     0,     0,
     313,   554,   555,     0,     0,     0,  1709,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1719,     0,   556,
       0,     0,     0,  1724,     0,    87,    88,  1726,    89,   172,
      91,   318,     0,   319,     0,     0,   320,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   707,   454,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,     0,     0,   817,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   817,    10,     0,  1761,     0,
       0,   817,     0,     0,     0,   817,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1742,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,   817,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,    55,    56,    57,     0,    58,    59,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,    82,    83,    84,
      85,    86,     0,    87,    88,     0,    89,    90,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,    96,     0,    97,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1080,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,    82,    83,    84,    85,    86,     0,    87,    88,
       0,    89,    90,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,    96,     0,    97,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1250,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,    55,    56,    57,     0,    58,    59,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,    82,    83,    84,
      85,    86,     0,    87,    88,     0,    89,    90,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,    96,     0,    97,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
     645,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1050,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1094,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1161,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,  1163,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,  1316,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1428,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1651,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,  1696,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1731,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,  1734,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1751,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1768,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1809,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1816,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   167,   168,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,   783,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,   167,   168,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,   988,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   167,   168,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,  1500,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,   167,   168,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,  1644,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   167,   168,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,   167,   168,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,   660,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   455,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,   661,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,     0,     0,   778,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,     0,     0,  1107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1047,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,  1108,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     388,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,   427,   428,   429,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     455,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,   184,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,   456,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,     0,     0,     0,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1047,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,   427,   428,   429,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     455,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,   541,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,   250,   428,
     429,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,   455,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,     0,   253,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   388,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,   427,   428,   429,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   455,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,   543,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,   525,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   673,
     454,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,     0,     0,     0,     0,
     719,     0,     0,     0,     0,     0,     0,     0,     0,  1047,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,     0,     0,     0,   760,     0,     0,     0,     0,
       0,     0,     0,   455,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,     0,     0,     0,     0,     0,
     762,     0,     0,     0,     0,     0,     0,     0,  1047,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  -978,  -978,  -978,  -978,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,     0,     0,     0,     0,  1152,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,   427,   428,   429,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   455,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   167,   168,   169,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,     0,     0,    94,     0,   561,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   173,   427,   428,   429,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     455,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,   606,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   167,   168,
     169,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,     0,     0,     0,    93,     0,     0,
      94,     0,   565,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   173,     0,     0,     0,
       0,   104,   105,     0,   106,   107,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   430,   264,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,   266,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,   202,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,   309,   310,   311,   312,     0,     0,     0,   313,   554,
     555,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   556,     0,     0,
       0,     0,     0,    87,    88,     0,    89,   172,    91,   318,
     263,   319,   264,   265,   320,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,     0,     0,   707,     0,   104,     0,     0,   266,
       0,   267,     0,     0,     0,  1047,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,     0,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   316,     0,     0,
      85,   317,     0,    87,    88,     0,    89,   172,    91,   318,
     263,   319,   264,   265,   320,     0,     0,     0,     0,     0,
       0,   321,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   322,     0,     0,     0,  1624,     0,     0,     0,   266,
       0,   267,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,     0,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   316,     0,     0,
      85,   317,     0,    87,    88,     0,    89,   172,    91,   318,
     263,   319,   264,   265,   320,     0,     0,     0,     0,     0,
       0,   321,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   322,     0,     0,     0,  1692,     0,     0,     0,   266,
       0,   267,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,   309,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   316,     0,     0,
      85,   317,     0,    87,    88,     0,    89,   172,    91,   318,
     263,   319,   264,   265,   320,     0,     0,     0,     0,     0,
       0,   321,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   322,     0,     0,     0,     0,     0,     0,     0,   266,
       0,   267,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,     0,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   316,     0,     0,
      85,   317,     0,    87,    88,     0,    89,   172,    91,   318,
     263,   319,   264,   265,   320,     0,     0,     0,     0,     0,
       0,   321,  1432,     0,     0,     0,     0,     0,     0,     0,
       0,   322,     0,     0,     0,     0,     0,     0,     0,   266,
       0,   267,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    49,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,     0,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   316,     0,     0,
      85,   317,     0,    87,    88,     0,    89,   172,    91,   318,
       0,   319,     0,     0,   320,  1524,  1525,  1526,  1527,  1528,
       0,   321,  1529,  1530,  1531,  1532,     0,     0,     0,     0,
       0,   322,     0,     0,     0,     0,     0,     0,     0,  1533,
    1534,  1535,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,  1536,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,  1537,  1538,  1539,  1540,  1541,  1542,  1543,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1544,  1545,  1546,  1547,  1548,  1549,  1550,  1551,
    1552,  1553,  1554,    49,  1555,  1556,  1557,  1558,  1559,  1560,
    1561,  1562,  1563,  1564,  1565,  1566,  1567,  1568,  1569,  1570,
    1571,  1572,  1573,  1574,  1575,  1576,  1577,  1578,  1579,  1580,
    1581,  1582,  1583,  1584,     0,     0,     0,  1585,  1586,     0,
    1587,  1588,  1589,  1590,  1591,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1592,  1593,  1594,     0,
       0,     0,    87,    88,     0,    89,   172,    91,  1595,     0,
    1596,  1597,     0,  1598,   427,   428,   429,     0,     0,     0,
    1599,  1600,     0,  1601,     0,  1602,  1603,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   255,   256,   455,
     257,   258,     0,     0,   259,   260,   261,   262,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   263,     0,   264,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,     0,     0,     0,     0,
     266,     0,     0,     0,     0,     0,     0,     0,     0,  1047,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,     0,     0,   752,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,    49,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,     0,
     775,     0,   309,   310,   311,   312,     0,     0,     0,   313,
     554,   555,     0,     0,     0,     0,     0,   255,   256,     0,
     257,   258,     0,     0,   259,   260,   261,   262,   556,     0,
       0,     0,     0,     0,    87,    88,     0,    89,   172,    91,
     318,   263,   319,   264,     0,   320,  -978,  -978,  -978,  -978,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,     0,     0,     0,     0,     0,     0,     0,
     266,     0,     0,     0,     0,     0,  1047,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,    49,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,     0,
       0,     0,  1197,   310,   311,   312,     0,     0,     0,   313,
     554,   555,     0,     0,     0,     0,     0,   255,   256,     0,
     257,   258,     0,     0,   259,   260,   261,   262,   556,     0,
       0,     0,     0,     0,    87,    88,     0,    89,   172,    91,
     318,   263,   319,   264,     0,   320,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     266,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,    49,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,     0,
       0,     0,     0,   310,   311,   312,  1205,     0,     0,   313,
     554,   555,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   804,   805,     0,     0,   556,     0,
     806,     0,   807,     0,    87,    88,     0,    89,   172,    91,
     318,     0,   319,     0,   808,   320,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,     0,     0,     0,     0,
     427,   428,   429,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   982,   454,     0,     0,     0,
       0,   809,     0,    75,    76,    77,    78,    79,   455,     0,
       0,     0,     0,     0,   205,     0,     0,     0,     0,   171,
      83,    84,    85,   810,     0,    87,    88,    29,    89,   172,
      91,     0,     0,     0,    93,    34,    35,    36,    37,     0,
     202,     0,     0,   811,     0,     0,     0,   203,    98,     0,
       0,     0,     0,   812,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   502,     0,     0,     0,     0,     0,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   983,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   172,    91,   804,   805,     0,    93,     0,     0,
     806,     0,   807,     0,     0,     0,     0,     0,     0,     0,
       0,    98,     0,     0,   808,     0,   206,     0,     0,     0,
       0,   104,    34,    35,    36,    37,     0,     0,     0,     0,
     427,   428,   429,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,     0,     0,
       0,   809,     0,    75,    76,    77,    78,    79,   455,     0,
       0,     0,     0,     0,   205,     0,     0,     0,     0,   171,
      83,    84,    85,   810,     0,    87,    88,    29,    89,   172,
      91,     0,     0,     0,    93,    34,    35,    36,    37,     0,
     202,     0,     0,   811,     0,     0,     0,   203,    98,     0,
       0,     0,     0,   812,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   511,     0,     0,     0,     0,     0,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,   171,    83,    84,    85,    86,     0,    87,    88,
      29,    89,   172,    91,     0,     0,     0,    93,    34,    35,
      36,    37,     0,   202,     0,     0,     0,     0,     0,     0,
     203,    98,     0,     0,     0,     0,   206,     0,     0,   577,
       0,   104,    49,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   597,    74,     0,    75,
      76,    77,    78,    79,     0,     0,     0,     0,     0,     0,
     205,     0,     0,     0,     0,   171,    83,    84,    85,    86,
       0,    87,    88,    29,    89,   172,    91,     0,     0,     0,
      93,    34,    35,    36,    37,     0,   202,     0,     0,     0,
       0,     0,     0,   203,    98,     0,     0,     0,     0,   206,
       0,     0,     0,     0,   104,    49,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   204,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1075,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,   205,     0,     0,     0,     0,   171,    83,
      84,    85,    86,     0,    87,    88,    29,    89,   172,    91,
       0,     0,     0,    93,    34,    35,    36,    37,     0,   202,
       0,     0,     0,     0,     0,     0,   203,    98,     0,     0,
       0,     0,   206,     0,     0,     0,     0,   104,    49,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   204,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,     0,    75,    76,    77,    78,    79,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,   171,    83,    84,    85,    86,     0,    87,    88,     0,
      89,   172,    91,     0,     0,     0,    93,     0,     0,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   206,     0,     0,     0,   430,
     104,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,     0,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   923,     0,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,  1021,  1022,  1023,     0,     0,     0,     0,     0,
       0,     0,     0,   455,     0,     0,     0,     0,     0,     0,
     967,     0,  1024,     0,     0,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1021,  1022,  1023,
    1047,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1024,     0,  1279,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,     0,     0,  1021,  1022,  1023,     0,     0,     0,
       0,     0,     0,     0,     0,  1047,     0,     0,     0,     0,
       0,     0,     0,     0,  1024,     0,  1187,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1021,
    1022,  1023,  1047,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1024,
       0,  1337,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,     0,     0,     0,    34,    35,    36,
      37,     0,   202,     0,     0,     0,     0,  1047,     0,   203,
       0,     0,     0,     0,     0,     0,     0,     0,  1421,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,     0,     0,     0,     0,     0,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,   205,
       0,     0,     0,  1507,   171,    83,    84,    85,    86,     0,
      87,    88,     0,    89,   172,    91,     0,     0,     0,    93,
      34,    35,    36,    37,     0,   202,     0,     0,     0,     0,
       0,     0,   620,    98,     0,     0,     0,     0,   222,     0,
       0,     0,     0,   104,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,   171,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   172,    91,     0,
       0,     0,    93,    34,    35,    36,    37,     0,   202,     0,
       0,     0,     0,     0,     0,   203,    98,     0,     0,     0,
       0,   621,     0,     0,     0,     0,   622,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
     171,    83,    84,    85,    86,     0,    87,    88,     0,    89,
     172,    91,     0,     0,     0,    93,     0,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   222,     0,   787,     0,   430,   104,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   788,   430,
     920,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,     0,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,  1021,  1022,  1023,     0,     0,     0,     0,
       0,     0,     0,     0,   455,     0,     0,     0,     0,     0,
       0,     0,     0,  1024,  1342,     0,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,   429,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1047,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,  1023,   454,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,     0,     0,     0,     0,  1024,
       0,     0,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1047
};

static const yytype_int16 yycheck[] =
{
       5,     6,   122,     8,     9,    10,    11,    12,    13,   149,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    55,     4,    29,    30,   101,     4,     4,   381,
      33,   175,    31,   496,     4,   527,    56,    31,    43,   637,
      31,   634,    45,   381,   716,   515,    51,    50,    53,   896,
     885,    56,   223,    58,   149,   218,   176,   633,   154,  1097,
     101,   338,   339,   454,  1084,   663,   571,   572,   486,   613,
     562,   793,    43,   981,   101,    80,   486,   916,    56,   101,
       9,   764,    48,     9,   786,    92,   490,   491,    14,    96,
      97,    31,     4,   932,     9,     9,   101,    31,    55,    14,
      14,     9,     9,    48,     9,   523,     9,     4,     9,    80,
       9,    69,    48,   523,   121,     9,    69,   521,     9,     9,
       9,   754,     4,     9,     9,     9,     9,    48,     9,   968,
       9,     9,   173,     9,    35,     9,    48,    88,     9,   235,
      49,    50,    88,   635,     9,     9,   173,    82,   174,    37,
      82,   173,   132,   133,     0,   157,   100,   458,   188,    69,
     174,   100,   188,   188,   159,   206,   104,   105,   173,    27,
      28,    69,   100,    69,   188,   180,     4,   172,    48,   206,
      37,   222,  1017,     8,   206,   191,   487,   188,   190,   191,
     234,   492,    31,    37,    82,   222,   191,    69,   110,   150,
      59,   206,    37,   115,   150,   117,   118,   119,   120,   121,
     122,   123,   192,   157,    69,    69,    69,   222,   157,    69,
     186,   153,    69,    82,    69,    82,    85,    69,    69,   157,
     188,   236,    37,   191,   239,   188,    69,   835,    82,    69,
     122,   246,   247,    69,   189,    69,    69,    82,   160,   161,
     190,   163,   935,   189,   192,   153,  1276,   191,   193,   174,
    1168,   190,   191,  1283,   190,  1285,   175,    53,   189,   991,
     240,   993,   184,   417,   244,   190,   190,    82,   188,   167,
     192,   191,   190,   190,  1304,   190,   778,   190,   189,  1128,
     189,   783,     4,   191,   176,   191,   190,   330,    69,   190,
     190,   190,   132,   133,   190,   190,   190,   190,   153,   190,
     189,   189,  1147,   189,    53,   189,   186,   358,   189,   189,
     913,   149,   366,   167,   189,   189,   831,   832,   104,   105,
     874,   358,   167,   188,   188,   188,   358,   191,   191,   113,
      52,   191,   505,    55,   191,    82,   191,   158,   468,   191,
     191,   163,   357,   358,   498,   100,  1638,   215,   191,   364,
      72,   191,   167,    37,   369,   191,   153,   191,   191,   121,
     153,    14,   158,   330,   159,   408,   128,  1397,    90,   174,
      92,    30,    82,   388,    96,    97,   357,   173,    31,   188,
      82,   396,  1352,   188,   469,    49,    50,   171,   861,   153,
      49,   188,   407,    52,   411,   188,   191,    50,    82,   121,
    1082,  1693,   157,   150,   151,   152,   192,   188,   189,   158,
     174,   188,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   469,
     455,   408,   457,   458,   459,   454,   463,   464,   465,   466,
     454,   624,  1164,   454,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   189,  1438,   338,
     339,   340,   487,   488,   902,   490,   491,   492,   493,   347,
     387,   469,   902,   193,   499,    30,   988,   502,   356,  1334,
     358,   193,  1462,   460,  1464,   363,   511,    82,   513,   529,
      82,   651,   370,   395,   373,  1148,   521,   188,    82,  1112,
    1096,   188,  1115,    58,   529,   929,   531,    69,   240,    82,
      82,    55,   244,   885,   191,    82,   248,    82,   709,    82,
     153,    88,   705,    88,   153,    80,   937,   885,   825,   174,
     827,   188,   509,   132,   133,  1060,   651,    52,    53,    54,
     120,   174,   188,   188,   534,   174,   101,   395,   128,   381,
    1405,   742,   577,    68,   718,    82,   151,   152,   191,   188,
     621,    88,   191,   174,   188,   467,   188,    56,   132,   133,
     132,   133,  1093,   188,    82,   167,   640,   188,   642,    68,
      88,   136,   137,   167,   151,   152,   151,   152,   188,   188,
      31,   153,  1284,    37,   167,   454,   621,   157,   330,   154,
     188,   460,   157,   158,   167,   160,   161,    69,   163,   164,
     165,   190,   795,   185,     4,    74,    75,   800,   496,   191,
      74,    75,   813,   178,   151,   152,   188,   486,   460,   820,
     197,   196,   190,   188,   100,   101,   661,    27,    28,  1292,
    1130,  1294,   669,   151,   152,  1017,    82,    30,   673,   765,
     509,   190,    88,   190,   486,   190,    82,   130,   131,  1017,
     190,   520,    88,   395,   523,  1278,    49,  1263,   190,    52,
     132,   133,   404,   190,   857,  1367,   408,   509,    69,   411,
     190,   191,   707,    69,  1167,    80,   191,    82,   520,   190,
     191,   523,   571,   572,   758,   759,   153,  1121,   103,   104,
     105,   765,   766,   188,    82,   188,   101,    69,  1132,   188,
      88,   101,   737,   153,   150,   151,   152,   634,   338,   339,
    1232,   265,  1780,   267,   150,   151,   152,   157,   460,   461,
     462,   463,   464,   465,   466,  1667,  1668,  1795,   117,   118,
     119,   120,   121,   122,   190,   770,    47,  1439,  1663,  1664,
    1403,   802,   803,  1274,   486,    68,    52,    53,    54,   782,
      56,   786,  1245,   158,   174,   160,   161,   188,   163,   164,
     165,   153,    68,   151,   152,  1147,   188,   509,   322,   962,
     195,     9,   660,   173,   153,  1398,   103,   104,   105,  1147,
     153,   523,   188,     8,  1306,   153,   191,   190,   193,   188,
      14,   791,   534,   651,   153,   184,   190,     9,    80,   190,
    1322,   190,   117,   118,   119,   191,   206,   105,   106,   107,
     190,   553,   128,    14,  1007,   215,   109,   174,   128,   101,
     189,  1014,   222,    14,   117,   118,   119,   120,   121,   122,
    1493,   719,   100,   575,   576,   870,   189,   189,   189,   189,
     240,   188,   109,   188,   244,   194,  1280,   188,   150,   884,
      80,   405,    82,    83,   408,  1386,     9,  1388,   189,   189,
     602,   603,   873,   189,   776,   189,   873,   873,     9,  1746,
      92,   101,   760,   873,   762,   910,   158,   190,   160,   161,
     162,   163,   164,   165,    14,   920,  1408,   174,   923,  1766,
     925,   184,   188,     9,   929,  1417,   188,   191,  1775,   936,
     788,   888,  1103,   191,   190,  1427,   188,    80,   937,    82,
     822,   190,    82,   937,   189,   189,   937,   189,   776,    80,
      49,    50,    51,    52,    53,    54,   190,   669,   101,   188,
     160,   161,   967,   163,   164,   165,   825,   130,   827,    68,
     101,   974,   831,   832,   833,   189,   873,   347,    69,  1142,
    1384,  1482,  1334,    31,   131,  1486,   356,   845,   358,   173,
     134,   873,     9,   363,   822,   153,  1334,   153,   189,   896,
     370,    14,  1077,   861,   862,   975,   186,     9,  1500,     9,
    1643,   175,   894,   189,     9,   158,   913,   160,   161,   192,
     163,   164,   165,    14,   548,   395,   157,   130,   195,   160,
     161,  1194,   163,   164,   165,   195,     9,    14,  1201,   189,
     195,   189,   754,   195,   756,   873,   188,   153,   191,   888,
     193,   189,     9,  1405,   100,   190,   190,  1077,    89,  1016,
     191,   900,  1019,   902,   776,   134,   894,  1405,   153,     9,
     189,   188,  1077,   885,   153,   188,   888,   153,   790,   791,
     117,   118,   119,   120,   121,   122,   191,    80,   900,  1687,
     902,   128,   129,   191,     9,   977,   192,   979,   937,     9,
    1081,    14,   190,  1108,  1081,  1081,   630,   631,   101,   175,
     822,  1081,    14,   191,   195,   639,  1121,   829,   830,   191,
      14,   189,   186,   981,   982,   190,   496,  1132,  1133,   166,
     969,    31,   188,   126,   188,    31,  1637,    14,  1301,   188,
    1641,   188,    14,  1635,    51,   188,   858,   184,   188,   977,
       9,   979,  1644,  1156,   189,   188,   190,   969,   190,  1164,
     134,   873,    14,   175,   534,   134,  1764,   160,   161,  1174,
     163,   164,   165,     9,   367,   189,   888,    68,   371,   195,
       9,    82,   894,   192,  1081,   192,   190,     9,   900,  1159,
     902,   188,   134,    14,   188,   188,  1688,    82,   190,  1081,
     189,  1060,   188,   191,   397,  1017,   399,   400,   401,   402,
     188,   191,   191,   189,     4,  1112,   134,     9,  1115,   190,
     195,    89,   150,   191,   936,    31,    30,    76,  1368,   190,
     189,   175,   190,    31,    80,   134,   948,   949,   950,   189,
    1732,   189,   134,  1363,     9,   192,   189,     9,   189,  1107,
    1255,   190,   190,  1081,  1259,   101,  1261,   969,    48,   192,
     191,    14,    82,   975,  1269,   977,   188,   979,   189,   189,
     189,   188,   134,  1774,  1279,  1280,    80,   191,   802,   803,
       9,   190,    31,   189,   189,    89,   189,   999,  1789,  1171,
     660,   110,   190,   190,  1152,   190,   189,   101,   162,   189,
     158,  1793,   191,   190,  1016,    14,    82,  1019,  1800,  1167,
    1168,   189,   115,   189,   160,   161,  1460,   163,   164,   165,
     110,   191,   134,   189,   134,   115,    14,   117,   118,   119,
     120,   121,   122,   123,   174,  1147,  1048,   191,    82,   190,
      14,    14,   188,  1171,    82,   189,  1243,   188,   134,   719,
     154,   875,   189,   157,    14,  1252,   160,   161,   190,   163,
     164,   165,   190,    14,   190,    80,    14,   891,   191,  1081,
     160,   161,     9,   163,    27,    28,  1381,     9,   192,  1384,
     904,  1278,    58,    80,    82,   174,   101,  1245,   188,    82,
     760,     9,   762,   190,   184,   191,   113,   100,    77,    78,
      79,    80,   192,   153,   101,   100,   776,   175,   165,   933,
      35,    14,   109,   110,   188,  1243,   189,   171,   788,   190,
      82,   791,   101,   175,  1252,  1432,   117,   118,   119,   120,
     121,   122,  1144,   188,    80,   175,  1148,   168,   189,     4,
       9,    82,   190,   189,  1414,   160,   161,  1159,   163,   164,
     165,    14,   822,   189,   191,   101,    82,    14,    82,  1171,
     157,    14,  1344,   160,   161,    82,   163,   164,   165,    14,
      82,   995,  1477,   188,   998,   845,  1060,  1755,  1435,  1623,
    1362,   160,   161,    48,   163,   164,   165,   930,   876,   461,
    1387,   861,   862,   184,  1657,  1658,  1393,   463,  1395,   466,
    1771,  1398,  1165,   873,  1319,  1766,  1499,  1489,   154,   579,
    1407,   157,  1522,  1440,   160,   161,  1344,   163,   164,   165,
    1607,  1358,  1334,  1799,   894,  1787,  1238,  1409,  1498,  1499,
      49,    50,    51,    52,    53,    54,  1418,    56,  1619,  1085,
    1368,  1354,  1485,  1015,   949,   110,   192,  1143,   900,    68,
     115,  1144,   117,   118,   119,   120,   121,   122,   123,  1387,
     964,   408,   215,   364,  1721,  1393,   802,  1395,  1347,  1068,
    1000,  1095,  1048,  1097,    -1,    -1,    -1,    -1,    -1,  1407,
    1292,  1409,  1294,    -1,    -1,  1618,  1468,    -1,    -1,    -1,
    1418,    -1,    -1,  1405,  1491,   160,   161,    -1,   163,  1739,
    1124,    80,    -1,  1127,    -1,   975,    -1,   977,    -1,   979,
      -1,   981,   982,    -1,    -1,  1497,    -1,    -1,    -1,   184,
      -1,  1503,   101,  1628,    80,    -1,  1508,   192,    -1,    -1,
      -1,    -1,  1344,    -1,    -1,    -1,    -1,    -1,    -1,  1759,
    1352,    -1,    -1,    27,    28,   101,  1358,    31,    49,    50,
      -1,    -1,    -1,   109,   110,    -1,  1180,    -1,    -1,    -1,
    1184,  1618,    -1,  1491,    -1,    -1,    -1,    -1,    69,  1497,
      -1,    55,    -1,    -1,  1681,  1503,    77,    78,    79,    80,
    1508,   160,   161,    -1,   163,   164,   165,    -1,    89,    -1,
      -1,  1403,    -1,    -1,   347,    -1,    -1,  1409,    -1,    30,
     101,   157,  1414,   356,   160,   161,  1418,   163,   164,   165,
     363,  1081,    -1,    -1,    -1,  1239,  1240,   370,    -1,    -1,
    1432,    -1,    -1,  1435,    -1,    -1,  1438,    -1,   381,    -1,
      -1,    -1,    -1,    -1,    -1,   136,  1448,  1107,    -1,    -1,
       4,    -1,    -1,  1455,    -1,    -1,    -1,    -1,   149,    80,
    1462,    -1,  1464,    -1,    -1,    -1,    -1,    80,  1470,   160,
     161,    -1,   163,   164,   165,    -1,  1648,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,   178,   101,    -1,
      -1,  1493,  1152,    -1,    48,  1497,  1498,  1499,    -1,  1159,
      -1,  1503,   123,    -1,    27,    28,  1508,  1167,  1168,    -1,
    1805,  1171,  1326,    -1,  1328,   136,   137,  1689,  1813,    -1,
      -1,    -1,    -1,  1695,  1819,    -1,    -1,  1822,    -1,    -1,
    1648,    -1,    -1,   154,    80,    -1,   157,   158,    -1,   160,
     161,   215,   163,   164,   165,   158,    -1,   160,   161,  1363,
     163,   164,   165,   496,    -1,   101,   110,    -1,  1730,  1746,
      -1,   115,    -1,   117,   118,   119,   120,   121,   122,   123,
      -1,  1689,    -1,    -1,    -1,    80,    -1,  1695,    -1,  1766,
    1752,    -1,    -1,    -1,    -1,  1245,    -1,    -1,  1775,    -1,
      -1,   265,    -1,   267,    -1,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,   160,   161,    -1,   163,
      -1,   157,  1730,    -1,   160,   161,  1618,   163,   164,   165,
      -1,  1739,    -1,    -1,    -1,    -1,    -1,     4,    -1,  1801,
     184,    -1,    -1,    -1,  1752,    -1,  1808,  1639,   192,    -1,
      -1,  1643,    -1,    -1,    -1,    -1,  1648,    -1,   322,    48,
      -1,    -1,   157,    -1,  1656,   160,   161,    -1,   163,   164,
     165,  1663,  1664,    -1,    -1,  1667,  1668,    -1,    -1,    -1,
      -1,    48,    -1,   347,    -1,    -1,    -1,    -1,    -1,  1681,
      -1,    -1,   356,  1801,  1344,    -1,    -1,  1689,    -1,   363,
    1808,    -1,   215,  1695,    80,    -1,   370,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1520,   381,    -1,    -1,
      -1,   110,    -1,    -1,    -1,   101,   115,   660,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,  1730,    -1,
      -1,   405,    -1,   110,   408,    -1,  1738,    -1,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,     4,    -1,  1409,
    1752,    -1,    -1,    -1,  1414,    -1,  1758,    -1,  1418,    -1,
      -1,   160,   161,    -1,   163,    -1,    -1,    -1,   154,    -1,
      -1,   157,   158,    -1,   160,   161,   719,   163,   164,   165,
     454,    -1,    -1,   160,   161,   184,   163,    -1,    -1,    -1,
      -1,    48,    -1,   192,    -1,    -1,    -1,    -1,    -1,  1801,
      -1,    -1,    -1,    -1,    -1,    -1,  1808,   184,  1622,    -1,
      -1,    -1,    -1,    -1,    -1,   192,    -1,   760,    -1,   762,
      -1,    -1,   496,    -1,   347,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   356,    -1,    -1,    -1,  1497,  1498,  1499,
     363,    -1,    -1,  1503,    -1,   788,    -1,   370,  1508,    -1,
      -1,    -1,    -1,   110,    -1,    -1,    -1,    -1,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,    -1,    77,    78,
      79,    -1,    -1,    -1,   548,   549,    -1,    -1,   552,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1704,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   845,   160,   161,    -1,   163,    -1,    -1,   583,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   861,   862,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,    -1,   138,
     139,   140,   141,   142,    -1,   192,    -1,    -1,    -1,    -1,
     149,    -1,   885,    -1,    -1,    -1,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   630,   631,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   639,  1780,    10,    11,    12,
      -1,    -1,    -1,   496,   183,    -1,    -1,    -1,    -1,    -1,
      -1,  1795,    -1,    -1,    -1,    -1,   660,    30,  1648,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    30,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,  1689,
      -1,    -1,    -1,    -1,    -1,  1695,    -1,    -1,   981,   982,
      58,    -1,    -1,    -1,    -1,   719,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1730,    -1,    -1,    -1,  1017,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   101,    -1,    -1,   760,    -1,   762,    -1,
      -1,   109,  1752,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   788,   789,    -1,    -1,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   802,   803,
     804,   805,   806,   807,   808,    -1,   154,   660,   812,   157,
     158,  1801,   160,   161,    -1,   163,   164,   165,  1808,   823,
      -1,    -1,   195,    66,    67,    -1,    -1,    -1,    -1,    -1,
     178,    -1,    -1,    -1,  1107,   132,   133,    -1,    -1,    -1,
     188,   845,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,   859,    56,   861,   862,    27,
      28,    -1,    -1,    31,    -1,    -1,   719,    -1,    68,    -1,
      -1,   875,   876,    -1,  1147,    -1,    -1,    -1,    -1,  1152,
      -1,   885,    -1,    -1,    -1,    -1,    -1,   891,    -1,   132,
     133,    -1,   189,    -1,  1167,  1168,    -1,    -1,    -1,    -1,
     904,    -1,    -1,    -1,    -1,    -1,    -1,   760,   912,   762,
      -1,   915,    -1,    -1,    -1,    -1,    10,    11,    12,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,   933,
     128,   129,    -1,   937,    -1,   788,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    -1,    -1,   164,    -1,   166,    -1,
      -1,    -1,  1245,    -1,    68,    -1,    -1,   981,   982,    -1,
      -1,   179,    -1,   181,    -1,    -1,   184,    -1,    -1,    -1,
      -1,   995,   845,    -1,   998,    -1,  1000,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   861,   862,
      -1,  1015,    -1,  1017,    -1,    -1,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,    10,    11,    12,   215,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1064,  1334,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,  1095,    -1,  1097,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    68,  1107,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
    1124,    -1,    -1,  1127,    -1,    -1,    -1,    -1,   981,   982,
      -1,    -1,  1405,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1147,    -1,    -1,    -1,    -1,  1152,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    66,    67,    -1,    -1,
      -1,    -1,    -1,  1167,  1168,    -1,  1170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1180,   123,    -1,   347,
    1184,    -1,    -1,  1187,    -1,  1189,    -1,    -1,   356,    -1,
     136,   137,    -1,    -1,    -1,   363,    -1,    -1,    -1,    -1,
      -1,  1205,   370,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,   157,   158,   381,   160,   161,    -1,   163,   164,   165,
      -1,    -1,   132,   133,    -1,    -1,   192,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,  1239,  1240,    -1,  1242,    -1,
      -1,  1245,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,  1107,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,   189,
      -1,    -1,    -1,    -1,    -1,    -1,   454,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,  1167,  1168,    -1,    -1,    -1,    -1,
      -1,    -1,  1326,    -1,  1328,    -1,    -1,    -1,   496,  1333,
    1334,    -1,    -1,  1337,    -1,  1339,    -1,    -1,  1342,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1352,  1353,
      -1,    -1,  1356,    -1,    -1,    -1,    -1,    -1,    30,  1363,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,   552,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1245,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,  1405,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   189,    -1,    -1,    -1,   583,    -1,  1421,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1430,  1431,    -1,    -1,
      -1,    -1,    -1,    -1,  1438,    -1,  1440,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1462,    -1,
    1464,    -1,    -1,    -1,    -1,    -1,  1470,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,   660,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1506,  1507,    -1,    68,    -1,    -1,  1512,    -1,
    1514,    -1,    -1,    -1,    -1,    -1,  1520,    -1,  1522,    -1,
      -1,   265,    -1,   267,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,   719,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,   322,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,   760,    -1,   762,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1622,    -1,
     788,   789,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      -1,    30,    -1,    -1,    -1,  1639,   804,   805,   806,   807,
     808,    -1,    -1,    -1,   812,    -1,    -1,    66,    67,    -1,
      -1,    -1,  1656,    -1,    -1,   823,    -1,    -1,  1662,    58,
      -1,   405,    -1,    -1,   408,    -1,    -1,    -1,    -1,  1673,
      -1,    -1,    -1,    -1,    -1,  1679,    -1,   845,    -1,  1683,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   859,    -1,   861,   862,    -1,    -1,    -1,    -1,    -1,
    1704,    -1,   101,    -1,    -1,    -1,    -1,    -1,   876,    -1,
     109,    -1,   192,   132,   133,    -1,    -1,   885,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,    -1,
    1744,    -1,    -1,    -1,   912,    -1,    -1,   915,    -1,    -1,
    1754,    -1,    -1,    -1,    -1,   154,    -1,    -1,   157,   158,
      -1,   160,   161,    -1,   163,   164,   165,  1771,    -1,   937,
     189,    -1,    -1,    -1,    -1,    -1,  1780,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    -1,   184,    -1,    -1,    -1,   188,
      -1,  1795,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   548,   549,    -1,    -1,   552,    -1,
      -1,    -1,    30,   981,   982,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,   583,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1015,    -1,  1017,
      68,    -1,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
      -1,    -1,    -1,    -1,    -1,    -1,   630,   631,    10,    11,
      12,    -1,    -1,    -1,    -1,   639,  1064,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,  1107,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,  1147,
      -1,    -1,    -1,    -1,  1152,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,  1167,
    1168,    -1,  1170,    -1,    -1,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1187,
      -1,  1189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1205,    -1,    -1,
       3,     4,     5,     6,     7,   789,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   802,   803,
     804,   805,   806,   807,   808,    28,    29,    -1,   812,    -1,
     192,   132,   133,    -1,  1242,    -1,    -1,  1245,    -1,    27,
      28,    -1,    -1,    31,    -1,    -1,    49,    50,    -1,    -1,
      -1,    -1,    55,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      -1,    30,    -1,    -1,    77,    78,    79,    80,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,
      -1,   875,    -1,    -1,    -1,    -1,    -1,    -1,   101,    58,
      -1,    -1,    -1,    -1,    -1,    -1,   109,   891,    -1,    -1,
      -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,   122,
     904,    80,   125,   126,    -1,  1333,  1334,    -1,   912,  1337,
      -1,  1339,   135,   136,  1342,   138,   139,   140,   141,   142,
      -1,    -1,   101,    -1,    -1,  1353,   149,    -1,  1356,   933,
      -1,   154,   155,   156,   157,   158,    -1,   160,   161,    -1,
     163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,
      -1,    -1,    -1,    -1,    -1,   178,    -1,   136,   137,    -1,
     183,   184,   185,    -1,    -1,   188,    -1,   190,    -1,    -1,
     193,   194,    -1,   196,   197,   154,    -1,  1405,   157,   158,
      -1,   160,   161,    -1,   163,   164,   165,    -1,   167,    -1,
      -1,   995,    -1,  1421,   998,    -1,  1000,    -1,    -1,   178,
      -1,    -1,  1430,  1431,    -1,    -1,    -1,   215,    -1,   188,
      -1,  1015,  1440,    -1,    -1,    -1,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1064,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,  1506,  1507,
      -1,    -1,    -1,    -1,  1512,    -1,  1514,    -1,    -1,    -1,
      -1,  1095,    -1,  1097,  1522,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
    1124,    56,    -1,  1127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    68,    -1,    -1,    -1,    -1,    -1,   347,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   356,    -1,
      -1,    -1,    -1,    -1,    -1,   363,    -1,    -1,    -1,    -1,
      58,    -1,   370,    -1,    -1,    -1,  1170,    -1,    -1,    -1,
      -1,    -1,    -1,   381,    -1,    -1,  1180,    -1,    -1,    -1,
    1184,    -1,    80,  1187,    -1,  1189,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1205,    -1,   101,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,  1662,  1239,  1240,    -1,   136,   137,
      -1,    -1,    -1,    -1,    68,  1673,   454,    -1,    -1,    -1,
      -1,  1679,    -1,    -1,    -1,  1683,   154,   192,    -1,   157,
     158,    -1,   160,   161,    -1,   163,   164,   165,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,   552,    -1,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   496,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   583,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1744,    -1,    -1,    -1,
      -1,    -1,  1326,    -1,  1328,    -1,  1754,    -1,    -1,  1333,
      -1,    -1,    -1,  1337,    -1,  1339,    -1,    -1,  1342,    -1,
      -1,    -1,    -1,  1771,   552,    -1,    -1,    -1,  1352,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,  1363,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    -1,    -1,    -1,   583,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,  1421,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1438,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,   660,    -1,    -1,    -1,    -1,    -1,  1462,    -1,
    1464,    -1,    -1,    -1,    -1,    -1,  1470,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1506,  1507,    -1,    68,    -1,    -1,  1512,    -1,
      -1,   719,    -1,   789,    -1,    -1,  1520,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   804,   805,
     806,   807,    -1,    10,    11,    12,   812,    -1,   192,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   760,    30,   762,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
     788,   789,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,   804,   805,   806,   807,
     808,    -1,    -1,    -1,   812,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1622,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      -1,    -1,    -1,    -1,    -1,  1639,   912,   845,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1656,   861,   862,    -1,    -1,    -1,  1662,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1673,
      -1,    -1,    -1,    -1,    -1,  1679,    -1,   885,    -1,  1683,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
    1704,    56,    -1,    -1,   912,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,   192,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   937,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1744,    -1,    -1,    -1,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,    -1,   981,   982,    -1,  1780,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1064,    -1,
      -1,  1795,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1015,    -1,  1017,
      -1,    -1,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1064,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1170,    -1,    -1,    -1,    68,  1107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1187,    -1,  1189,    -1,    69,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    77,    78,    79,    80,    -1,    82,  1205,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,  1147,
      -1,    -1,    -1,    -1,  1152,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1167,
    1168,    -1,  1170,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1187,
      -1,  1189,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,  1205,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     190,     3,     4,     5,     6,     7,    -1,    -1,    -1,   183,
      -1,    13,    -1,    -1,   188,    -1,    -1,  1245,    -1,   193,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    -1,  1333,    -1,    -1,
      -1,  1337,    -1,  1339,    -1,    -1,  1342,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,  1333,  1334,   109,    -1,  1337,
      -1,  1339,    -1,    -1,  1342,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,  1421,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,  1405,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,  1421,   196,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1506,  1507,    -1,    -1,    -1,    -1,  1512,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    30,    29,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,  1506,  1507,
      -1,    56,    -1,    -1,  1512,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,  1521,    -1,    70,    71,    72,    73,    74,
      75,    76,    -1,    -1,    -1,    80,    -1,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,   128,   129,   130,   131,    -1,    -1,    -1,
     135,   136,   137,    -1,    -1,    -1,  1662,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1673,    -1,   154,
      -1,    -1,    -1,  1679,    -1,   160,   161,  1683,   163,   164,
     165,   166,    -1,   168,    -1,    -1,   171,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,   191,    56,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,  1662,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,  1673,    13,    -1,  1744,    -1,
      -1,  1679,    -1,    -1,    -1,  1683,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1706,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,  1744,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,   179,    -1,   181,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
     112,    -1,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,   179,    -1,   181,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,   179,    -1,   181,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    99,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    97,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,   111,
     112,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    84,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,
     107,   108,   109,    -1,   111,   112,    -1,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,   167,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,   191,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,   167,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    10,    11,    12,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      68,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,   106,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    10,    11,    12,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      68,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,    11,
      12,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    68,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,   190,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    10,    11,    12,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    68,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   190,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,   189,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    31,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    10,    11,    12,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    68,    -1,    -1,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   190,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    10,    11,    12,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      68,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    30,    29,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,
      -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,   166,
      27,   168,    29,    30,   171,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,   191,    -1,   193,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,   192,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,   192,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,   179,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,   168,    -1,    -1,   171,     3,     4,     5,     6,     7,
      -1,   178,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,    -1,    -1,    -1,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,   161,    -1,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,    10,    11,    12,    -1,    -1,    -1,
     178,   179,    -1,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    68,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,   189,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     189,    -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,    -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    -1,   171,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,    -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    -1,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,   131,    31,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    -1,    -1,   154,    -1,
      55,    -1,    57,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,   168,    -1,    69,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    37,    56,    -1,    -1,    -1,
      -1,   136,    -1,   138,   139,   140,   141,   142,    68,    -1,
      -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,
     155,   156,   157,   158,    -1,   160,   161,    69,   163,   164,
     165,    -1,    -1,    -1,   169,    77,    78,    79,    80,    -1,
      82,    -1,    -1,   178,    -1,    -1,    -1,    89,   183,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    49,    50,    -1,   169,    -1,    -1,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    69,    -1,   188,    -1,    -1,    -1,
      -1,   193,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,   136,    -1,   138,   139,   140,   141,   142,    68,    -1,
      -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,
     155,   156,   157,   158,    -1,   160,   161,    69,   163,   164,
     165,    -1,    -1,    -1,   169,    77,    78,    79,    80,    -1,
      82,    -1,    -1,   178,    -1,    -1,    -1,    89,   183,    -1,
      -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      69,   163,   164,   165,    -1,    -1,    -1,   169,    77,    78,
      79,    80,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      89,   183,    -1,    -1,    -1,    -1,   188,    -1,    -1,   191,
      -1,   193,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,    -1,    -1,    -1,   154,   155,   156,   157,   158,
      -1,   160,   161,    69,   163,   164,   165,    -1,    -1,    -1,
     169,    77,    78,    79,    80,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,   183,    -1,    -1,    -1,    -1,   188,
      -1,    -1,    -1,    -1,   193,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    69,   163,   164,   165,
      -1,    -1,    -1,   169,    77,    78,    79,    80,    -1,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    89,   183,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   101,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,
      -1,   154,   155,   156,   157,   158,    -1,   160,   161,    -1,
     163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    30,
     193,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,   134,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,   134,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,   134,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    77,    78,    79,
      80,    -1,    82,    -1,    -1,    -1,    -1,    68,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,   134,   154,   155,   156,   157,   158,    -1,
     160,   161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,
      77,    78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,   183,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,   193,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    77,    78,    79,    80,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    89,   183,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,    -1,    28,    -1,    30,   193,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    12,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   199,   200,     0,   201,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    49,    50,    55,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    69,
      70,    71,    72,    73,    77,    78,    79,    80,    81,    82,
      84,    85,    89,    90,    91,    92,    94,    96,    98,   101,
     102,   106,   107,   108,   109,   110,   111,   112,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   124,   125,   126,
     127,   128,   129,   135,   136,   138,   139,   140,   141,   142,
     146,   149,   154,   155,   156,   157,   158,   160,   161,   163,
     164,   165,   166,   169,   172,   178,   179,   181,   183,   184,
     185,   188,   190,   191,   193,   194,   196,   197,   202,   205,
     215,   216,   217,   218,   219,   222,   238,   239,   243,   246,
     253,   259,   319,   320,   328,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   343,   346,   358,   359,   360,
     362,   363,   365,   375,   376,   377,   379,   384,   387,   406,
     414,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   429,   442,   444,   446,   120,   121,   122,
     135,   154,   164,   188,   205,   238,   319,   340,   418,   340,
     188,   340,   340,   340,   106,   340,   340,   340,   404,   405,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,    82,    89,   122,   149,   188,   216,   359,   376,
     379,   384,   418,   421,   418,    37,   340,   433,   434,   340,
     122,   128,   188,   216,   251,   376,   377,   378,   380,   384,
     415,   416,   417,   425,   430,   431,   188,   329,   381,   188,
     329,   350,   330,   340,   224,   329,   188,   188,   188,   329,
     190,   340,   205,   190,   340,     3,     4,     6,     7,    10,
      11,    12,    13,    27,    29,    30,    56,    58,    70,    71,
      72,    73,    74,    75,    76,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   128,
     129,   130,   131,   135,   136,   137,   154,   158,   166,   168,
     171,   178,   188,   205,   206,   207,   218,   447,   463,   464,
     466,   190,   335,   337,   340,   191,   231,   340,   109,   110,
     157,   208,   209,   210,   211,   215,    82,   193,   285,   286,
     121,   128,   120,   128,    82,   287,   188,   188,   188,   188,
     205,   257,   450,   188,   188,   330,    82,    88,   150,   151,
     152,   439,   440,   157,   191,   215,   215,   205,   258,   450,
     158,   188,   450,   450,    82,   185,   191,   351,    27,   328,
     332,   340,   341,   418,   422,   220,   191,    88,   382,   439,
      88,   439,   439,    31,   157,   174,   451,   188,     9,   190,
      37,   237,   158,   256,   450,   122,   184,   238,   320,   190,
     190,   190,   190,   190,   190,   190,   190,    10,    11,    12,
      30,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    56,    68,   190,    69,    69,   191,
     153,   129,   164,   166,   179,   181,   259,   318,   319,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    66,    67,   132,   133,   408,    69,   191,   413,
     188,   188,    69,   191,   193,   426,   188,   237,   238,    14,
     340,   190,   134,    47,   205,   403,    88,   328,   341,   153,
     418,   134,   195,     9,   389,   252,   328,   341,   418,   451,
     153,   188,   383,   408,   413,   189,   340,    31,   222,     8,
     352,     9,   190,   222,   223,   330,   331,   340,   205,   271,
     226,   190,   190,   190,   136,   137,   466,   466,   174,   188,
     109,   466,    14,   153,   136,   137,   154,   205,   207,   190,
     190,   190,   232,   113,   171,   190,   208,   210,   208,   210,
     215,   191,     9,   390,   190,   100,   157,   191,   418,     9,
     190,   128,   128,    14,     9,   190,   418,   443,   330,   328,
     341,   418,   421,   422,   189,   174,   249,   135,   418,   432,
     433,   190,    69,   408,   150,   440,    81,   340,   418,    88,
     150,   440,   215,   204,   190,   191,   244,   254,   366,   368,
      89,   188,   193,   353,   354,   356,   379,   424,   426,   444,
      14,   100,   445,   347,   348,   349,   281,   282,   406,   407,
     189,   189,   189,   189,   189,   192,   221,   222,   239,   246,
     253,   406,   340,   194,   196,   197,   205,   452,   453,   466,
      37,   167,   283,   284,   340,   447,   188,   450,   247,   237,
     340,   340,   340,    31,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   380,   340,
     340,   428,   428,   340,   435,   436,   128,   191,   206,   207,
     425,   426,   257,   205,   258,   450,   450,   256,   238,    37,
     332,   335,   337,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   158,   191,   205,   409,
     410,   411,   412,   425,   428,   340,   283,   283,   428,   340,
     432,   237,   189,   340,   188,   402,     9,   389,   189,   189,
      37,   340,    37,   340,   383,   189,   189,   189,   425,   283,
     191,   205,   409,   410,   425,   189,   220,   275,   191,   337,
     340,   340,    92,    31,   222,   269,   190,    28,   100,    14,
       9,   189,    31,   191,   272,   466,    30,    89,   218,   460,
     461,   462,   188,     9,    49,    50,    55,    57,    69,   136,
     158,   178,   188,   216,   218,   361,   376,   384,   385,   386,
     205,   465,   220,   188,   230,   191,   190,   191,   190,   100,
     157,   109,   110,   157,   211,   212,   213,   214,   215,   211,
     205,   340,   286,   385,    82,     9,   189,   189,   189,   189,
     189,   189,   189,   190,    49,    50,   457,   458,   459,   130,
     262,   188,     9,   189,   189,    82,    83,   205,   441,   205,
      69,   192,   192,   201,   203,    31,   131,   261,   173,    53,
     158,   173,   370,   341,   134,     9,   389,   189,   153,   466,
     466,    14,   352,   281,   220,   186,     9,   390,   466,   467,
     408,   413,   408,   192,     9,   389,   175,   418,   340,   189,
       9,   390,    14,   344,   240,   130,   260,   188,   450,   340,
      31,   195,   195,   134,   192,     9,   389,   340,   451,   188,
     250,   245,   255,    14,   445,   248,   237,    71,   418,   340,
     451,   195,   192,   189,   189,   195,   192,   189,    49,    50,
      69,    77,    78,    79,    89,   136,   149,   178,   205,   392,
     394,   395,   398,   401,   205,   418,   418,   134,   260,   408,
     413,   189,   340,   276,    74,    75,   277,   220,   329,   220,
     331,   100,    37,   135,   266,   418,   385,   205,    31,   222,
     270,   190,   273,   190,   273,     9,   389,    89,   134,   153,
       9,   389,   189,   167,   452,   453,   454,   452,   385,   385,
     385,   385,   385,   388,   391,   188,   153,   188,   385,   153,
     191,    10,    11,    12,    30,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    68,   153,   451,
     192,   376,   191,   234,   210,   210,   205,   211,   211,   215,
       9,   390,   192,   192,    14,   418,   190,   175,     9,   389,
     205,   263,   376,   191,   432,   135,   418,    14,   195,   340,
     192,   201,   466,   263,   191,   369,    14,   189,   340,   353,
     425,   190,   466,   186,   192,    31,   455,   407,    37,    82,
     167,   409,   410,   412,   409,   410,   466,    37,   167,   340,
     385,   281,   188,   376,   261,   345,   241,   340,   340,   340,
     192,   188,   283,   262,    31,   261,   466,    14,   260,   450,
     380,   192,   188,    14,    77,    78,    79,   205,   393,   393,
     395,   396,   397,    51,   188,    88,   150,   188,     9,   389,
     189,   402,    37,   340,   261,   192,    74,    75,   278,   329,
     222,   192,   190,    93,   190,   266,   418,   188,   134,   265,
      14,   220,   273,   103,   104,   105,   273,   192,   466,   175,
     134,   466,   205,   460,     9,   189,   389,   134,   195,     9,
     389,   388,   206,   353,   355,   357,   189,   128,   206,   385,
     437,   438,   385,   385,   385,    31,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     465,    82,   235,   192,   192,   214,   190,   385,   459,   100,
     101,   456,     9,   291,   189,   188,   332,   337,   340,   195,
     192,   445,   291,   159,   172,   191,   365,   372,   159,   191,
     371,   134,   190,   455,   466,   352,   467,    82,   167,    14,
      82,   451,   418,   340,   189,   281,   191,   281,   188,   134,
     188,   283,   189,   191,   466,   191,   190,   466,   261,   242,
     383,   283,   134,   195,     9,   389,   394,   396,   150,   353,
     399,   400,   395,   418,   191,   329,    31,    76,   222,   190,
     331,   265,   432,   266,   189,   385,    99,   103,   190,   340,
      31,   190,   274,   192,   175,   466,   134,   167,    31,   189,
     385,   385,   189,   134,     9,   389,   189,   134,   192,     9,
     389,   385,    31,   189,   220,   190,   190,   205,   466,   466,
     376,     4,   110,   115,   121,   123,   160,   161,   163,   192,
     292,   317,   318,   319,   324,   325,   326,   327,   406,   432,
     192,   191,   192,    53,   340,   340,   340,   352,    37,    82,
     167,    14,    82,   340,   188,   455,   189,   291,   189,   281,
     340,   283,   189,   291,   445,   291,   190,   191,   188,   189,
     395,   395,   189,   134,   189,     9,   389,   291,    31,   220,
     190,   189,   189,   189,   227,   190,   190,   274,   220,   466,
     466,   134,   385,   353,   385,   385,   385,   191,   192,   456,
     130,   131,   179,   206,   448,   466,   264,   376,   110,   327,
      30,   123,   136,   137,   158,   164,   301,   302,   303,   304,
     376,   162,   309,   310,   126,   188,   205,   311,   312,   293,
     238,   466,     9,   190,     9,   190,   190,   445,   318,   189,
     288,   158,   367,   192,   192,    82,   167,    14,    82,   340,
     283,   115,   342,   455,   192,   455,   189,   189,   192,   191,
     192,   291,   281,   134,   395,   353,   192,   220,   225,   228,
      31,   222,   268,   220,   189,   385,   134,   134,   220,   376,
     376,   450,    14,   206,     9,   190,   191,   448,   445,   304,
     174,   191,     9,   190,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    29,    56,    70,    71,    72,
      73,    74,    75,    76,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   135,   136,   138,   139,   140,
     141,   142,   154,   155,   156,   166,   168,   169,   171,   178,
     179,   181,   183,   184,   205,   373,   374,     9,   190,   158,
     162,   205,   312,   313,   314,   190,    82,   323,   237,   294,
     448,   448,    14,   238,   192,   289,   290,   448,    14,    82,
     340,   189,   188,   455,   190,   191,   315,   342,   455,   288,
     192,   189,   395,   134,    31,   222,   267,   268,   220,   385,
     385,   192,   190,   190,   385,   376,   297,   466,   305,   306,
     384,   302,    14,    31,    50,   307,   310,     9,    35,   189,
      30,    49,    52,    14,     9,   190,   207,   449,   323,    14,
     466,   237,   190,    14,   340,    37,    82,   364,   191,   220,
     455,   315,   192,   455,   395,   220,    97,   233,   192,   205,
     218,   298,   299,   300,     9,   389,     9,   389,   192,   385,
     374,   374,    58,   308,   313,   313,    30,    49,    52,   385,
      82,   174,   188,   190,   385,   450,   385,    82,     9,   390,
     220,   192,   191,   315,    95,   190,   113,   229,   153,   100,
     466,   175,   384,   165,    14,   457,   295,   188,    37,    82,
     189,   192,   220,   190,   188,   171,   236,   205,   318,   319,
     175,   385,   175,   279,   280,   407,   296,    82,   192,   376,
     234,   168,   205,   190,   189,     9,   390,   117,   118,   119,
     321,   322,   279,    82,   264,   190,   455,   407,   467,   189,
     189,   190,   190,   191,   316,   321,    37,    82,   167,   455,
     191,   220,   467,    82,   167,    14,    82,   316,   220,   192,
      37,    82,   167,    14,    82,   340,   192,    82,   167,    14,
      82,   340,    14,    82,   340,   340
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
#line 728 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 731 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
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

  case 97:

/* Line 1455 of yacc.c  */
#line 866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyval).reset();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { (yyval).reset();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
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

  case 199:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
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

  case 201:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval).reset();;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { (yyval).reset();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { (yyval).reset();;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval).reset();;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { (yyval).reset();;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval).reset();;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval).reset();;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval).reset();;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval).reset();;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval).reset();;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval).reset();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 520:

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

  case 521:

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

  case 522:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
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

  case 524:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
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

  case 526:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
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

  case 561:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
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

  case 562:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval).reset();;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval).reset();;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
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
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval).reset();;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval).reset();;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval).reset();;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { (yyval).reset();;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval).reset();;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { (yyval).reset();;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval).reset();;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { (yyval).reset();;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
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

  case 817:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
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

  case 818:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
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

  case 819:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
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

  case 820:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
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

  case 827:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
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

  case 836:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval).reset();;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval)++;;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
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

  case 871:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
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

  case 877:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval).reset();;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
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

  case 913:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    {;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    {;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    {;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { (yyvsp[(1) - (5)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)])); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3294 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3309 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13807 "hphp.5.tab.cpp"
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
#line 3312 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

