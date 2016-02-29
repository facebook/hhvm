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
    Token params, ret, ref; ref = 0;
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
    Token params, ret, ref; ref = 0;
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
    Token params, ret, ref; ref = 0;
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
     T_PIPE = 283,
     T_COALESCE = 284,
     T_BOOLEAN_OR = 285,
     T_BOOLEAN_AND = 286,
     T_IS_NOT_IDENTICAL = 287,
     T_IS_IDENTICAL = 288,
     T_IS_NOT_EQUAL = 289,
     T_IS_EQUAL = 290,
     T_SPACESHIP = 291,
     T_IS_GREATER_OR_EQUAL = 292,
     T_IS_SMALLER_OR_EQUAL = 293,
     T_SR = 294,
     T_SL = 295,
     T_INSTANCEOF = 296,
     T_UNSET_CAST = 297,
     T_BOOL_CAST = 298,
     T_OBJECT_CAST = 299,
     T_ARRAY_CAST = 300,
     T_STRING_CAST = 301,
     T_DOUBLE_CAST = 302,
     T_INT_CAST = 303,
     T_DEC = 304,
     T_INC = 305,
     T_POW = 306,
     T_CLONE = 307,
     T_NEW = 308,
     T_EXIT = 309,
     T_IF = 310,
     T_ELSEIF = 311,
     T_ELSE = 312,
     T_ENDIF = 313,
     T_LNUMBER = 314,
     T_DNUMBER = 315,
     T_ONUMBER = 316,
     T_STRING = 317,
     T_STRING_VARNAME = 318,
     T_VARIABLE = 319,
     T_PIPE_VAR = 320,
     T_NUM_STRING = 321,
     T_INLINE_HTML = 322,
     T_HASHBANG = 323,
     T_CHARACTER = 324,
     T_BAD_CHARACTER = 325,
     T_ENCAPSED_AND_WHITESPACE = 326,
     T_CONSTANT_ENCAPSED_STRING = 327,
     T_ECHO = 328,
     T_DO = 329,
     T_WHILE = 330,
     T_ENDWHILE = 331,
     T_FOR = 332,
     T_ENDFOR = 333,
     T_FOREACH = 334,
     T_ENDFOREACH = 335,
     T_DECLARE = 336,
     T_ENDDECLARE = 337,
     T_AS = 338,
     T_SUPER = 339,
     T_SWITCH = 340,
     T_ENDSWITCH = 341,
     T_CASE = 342,
     T_DEFAULT = 343,
     T_BREAK = 344,
     T_GOTO = 345,
     T_CONTINUE = 346,
     T_FUNCTION = 347,
     T_CONST = 348,
     T_RETURN = 349,
     T_TRY = 350,
     T_CATCH = 351,
     T_THROW = 352,
     T_USE = 353,
     T_GLOBAL = 354,
     T_PUBLIC = 355,
     T_PROTECTED = 356,
     T_PRIVATE = 357,
     T_FINAL = 358,
     T_ABSTRACT = 359,
     T_STATIC = 360,
     T_VAR = 361,
     T_UNSET = 362,
     T_ISSET = 363,
     T_EMPTY = 364,
     T_HALT_COMPILER = 365,
     T_CLASS = 366,
     T_INTERFACE = 367,
     T_EXTENDS = 368,
     T_IMPLEMENTS = 369,
     T_OBJECT_OPERATOR = 370,
     T_NULLSAFE_OBJECT_OPERATOR = 371,
     T_DOUBLE_ARROW = 372,
     T_LIST = 373,
     T_ARRAY = 374,
     T_DICT = 375,
     T_CALLABLE = 376,
     T_CLASS_C = 377,
     T_METHOD_C = 378,
     T_FUNC_C = 379,
     T_LINE = 380,
     T_FILE = 381,
     T_COMMENT = 382,
     T_DOC_COMMENT = 383,
     T_OPEN_TAG = 384,
     T_OPEN_TAG_WITH_ECHO = 385,
     T_CLOSE_TAG = 386,
     T_WHITESPACE = 387,
     T_START_HEREDOC = 388,
     T_END_HEREDOC = 389,
     T_DOLLAR_OPEN_CURLY_BRACES = 390,
     T_CURLY_OPEN = 391,
     T_DOUBLE_COLON = 392,
     T_NAMESPACE = 393,
     T_NS_C = 394,
     T_DIR = 395,
     T_NS_SEPARATOR = 396,
     T_XHP_LABEL = 397,
     T_XHP_TEXT = 398,
     T_XHP_ATTRIBUTE = 399,
     T_XHP_CATEGORY = 400,
     T_XHP_CATEGORY_LABEL = 401,
     T_XHP_CHILDREN = 402,
     T_ENUM = 403,
     T_XHP_REQUIRED = 404,
     T_TRAIT = 405,
     T_ELLIPSIS = 406,
     T_INSTEADOF = 407,
     T_TRAIT_C = 408,
     T_HH_ERROR = 409,
     T_FINALLY = 410,
     T_XHP_TAG_LT = 411,
     T_XHP_TAG_GT = 412,
     T_TYPELIST_LT = 413,
     T_TYPELIST_GT = 414,
     T_UNRESOLVED_LT = 415,
     T_COLLECTION = 416,
     T_SHAPE = 417,
     T_TYPE = 418,
     T_UNRESOLVED_TYPE = 419,
     T_NEWTYPE = 420,
     T_UNRESOLVED_NEWTYPE = 421,
     T_COMPILER_HALT_OFFSET = 422,
     T_ASYNC = 423,
     T_LAMBDA_OP = 424,
     T_LAMBDA_CP = 425,
     T_UNRESOLVED_OP = 426
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
#line 877 "hphp.5.tab.cpp"

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
#define YYLAST   17886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  201
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  280
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1022
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1885

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   426

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   199,     2,   196,    55,    38,   200,
     191,   192,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   193,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   198,    37,     2,   197,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   194,    36,   195,    58,     2,     2,     2,
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
      27,    28,    29,    30,    33,    34,    35,    39,    40,    41,
      42,    45,    46,    47,    48,    49,    57,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    71,    72,    73,
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
     184,   185,   186,   187,   188,   189,   190
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
     217,   219,   221,   223,   225,   227,   229,   231,   233,   236,
     240,   244,   246,   249,   251,   254,   258,   263,   267,   269,
     272,   274,   277,   280,   282,   286,   288,   292,   295,   298,
     301,   307,   312,   315,   316,   318,   320,   322,   324,   328,
     334,   343,   344,   349,   350,   357,   358,   369,   370,   375,
     378,   382,   385,   389,   392,   396,   400,   404,   408,   412,
     416,   422,   424,   426,   428,   429,   439,   440,   451,   457,
     458,   472,   473,   479,   483,   487,   490,   493,   496,   499,
     502,   505,   509,   512,   515,   519,   522,   525,   526,   531,
     541,   542,   543,   548,   551,   552,   554,   555,   557,   558,
     568,   569,   580,   581,   593,   594,   604,   605,   616,   617,
     626,   627,   637,   638,   646,   647,   656,   657,   666,   667,
     675,   676,   685,   687,   689,   691,   693,   695,   698,   702,
     706,   709,   712,   713,   716,   717,   720,   721,   723,   727,
     729,   733,   736,   737,   739,   742,   747,   749,   754,   756,
     761,   763,   768,   770,   775,   779,   785,   789,   794,   799,
     805,   811,   816,   817,   819,   821,   826,   827,   833,   834,
     837,   838,   842,   843,   851,   860,   867,   870,   876,   883,
     888,   889,   894,   900,   908,   915,   922,   930,   940,   949,
     956,   964,   970,   973,   978,   984,   988,   989,   993,   998,
    1005,  1011,  1017,  1024,  1033,  1041,  1044,  1045,  1047,  1050,
    1053,  1057,  1062,  1067,  1071,  1073,  1075,  1078,  1083,  1087,
    1093,  1095,  1099,  1102,  1103,  1106,  1110,  1113,  1114,  1115,
    1120,  1121,  1127,  1130,  1133,  1136,  1137,  1148,  1149,  1161,
    1165,  1169,  1173,  1178,  1183,  1187,  1193,  1196,  1199,  1200,
    1207,  1213,  1218,  1222,  1224,  1226,  1230,  1235,  1237,  1240,
    1242,  1244,  1250,  1257,  1259,  1261,  1266,  1268,  1270,  1274,
    1277,  1280,  1281,  1284,  1285,  1287,  1291,  1293,  1295,  1297,
    1299,  1303,  1308,  1313,  1318,  1320,  1322,  1325,  1328,  1331,
    1335,  1339,  1341,  1343,  1345,  1347,  1351,  1353,  1357,  1359,
    1361,  1363,  1364,  1366,  1369,  1371,  1373,  1375,  1377,  1379,
    1381,  1383,  1385,  1386,  1388,  1390,  1392,  1396,  1402,  1404,
    1408,  1414,  1419,  1423,  1427,  1431,  1436,  1440,  1444,  1448,
    1451,  1454,  1456,  1458,  1462,  1466,  1468,  1470,  1471,  1473,
    1476,  1481,  1485,  1489,  1496,  1499,  1503,  1506,  1510,  1517,
    1519,  1521,  1523,  1525,  1527,  1534,  1538,  1543,  1550,  1554,
    1558,  1562,  1566,  1570,  1574,  1578,  1582,  1586,  1590,  1594,
    1598,  1601,  1604,  1607,  1610,  1614,  1618,  1622,  1626,  1630,
    1634,  1638,  1642,  1646,  1650,  1654,  1658,  1662,  1666,  1670,
    1674,  1678,  1682,  1685,  1688,  1691,  1694,  1698,  1702,  1706,
    1710,  1714,  1718,  1722,  1726,  1730,  1734,  1738,  1744,  1749,
    1753,  1755,  1758,  1761,  1764,  1767,  1770,  1773,  1776,  1779,
    1782,  1784,  1786,  1788,  1790,  1794,  1797,  1799,  1805,  1806,
    1807,  1820,  1821,  1835,  1836,  1841,  1842,  1850,  1851,  1857,
    1858,  1862,  1863,  1870,  1873,  1876,  1881,  1883,  1885,  1891,
    1895,  1901,  1905,  1908,  1909,  1912,  1913,  1918,  1923,  1927,
    1930,  1931,  1937,  1941,  1948,  1953,  1956,  1957,  1963,  1967,
    1970,  1971,  1977,  1981,  1986,  1991,  1996,  2001,  2006,  2011,
    2016,  2018,  2020,  2022,  2024,  2026,  2030,  2034,  2039,  2041,
    2044,  2049,  2052,  2059,  2060,  2062,  2067,  2068,  2071,  2072,
    2074,  2076,  2080,  2082,  2086,  2088,  2090,  2094,  2098,  2100,
    2102,  2104,  2106,  2108,  2110,  2112,  2114,  2116,  2118,  2120,
    2122,  2124,  2126,  2128,  2130,  2132,  2134,  2136,  2138,  2140,
    2142,  2144,  2146,  2148,  2150,  2152,  2154,  2156,  2158,  2160,
    2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,  2180,
    2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,  2200,
    2202,  2204,  2206,  2208,  2210,  2212,  2214,  2216,  2218,  2220,
    2222,  2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,  2240,
    2242,  2244,  2246,  2248,  2250,  2252,  2254,  2256,  2258,  2260,
    2262,  2267,  2272,  2274,  2276,  2278,  2280,  2282,  2284,  2288,
    2290,  2294,  2296,  2298,  2302,  2304,  2306,  2308,  2311,  2313,
    2314,  2315,  2317,  2319,  2323,  2324,  2326,  2328,  2330,  2332,
    2334,  2336,  2338,  2340,  2342,  2344,  2346,  2348,  2350,  2354,
    2357,  2359,  2361,  2366,  2370,  2375,  2377,  2379,  2381,  2385,
    2389,  2393,  2397,  2401,  2405,  2409,  2413,  2417,  2421,  2425,
    2429,  2433,  2437,  2441,  2445,  2449,  2453,  2456,  2459,  2462,
    2465,  2469,  2473,  2477,  2481,  2485,  2489,  2493,  2497,  2501,
    2507,  2512,  2516,  2520,  2524,  2526,  2528,  2530,  2532,  2536,
    2540,  2544,  2547,  2548,  2550,  2551,  2553,  2554,  2560,  2564,
    2568,  2570,  2572,  2574,  2576,  2580,  2583,  2585,  2587,  2589,
    2591,  2593,  2597,  2599,  2601,  2603,  2606,  2609,  2614,  2618,
    2623,  2625,  2628,  2629,  2635,  2639,  2643,  2645,  2649,  2651,
    2654,  2655,  2661,  2665,  2668,  2669,  2673,  2674,  2679,  2682,
    2683,  2687,  2691,  2693,  2694,  2696,  2698,  2700,  2702,  2706,
    2708,  2710,  2712,  2716,  2718,  2720,  2724,  2728,  2731,  2736,
    2739,  2744,  2750,  2756,  2762,  2768,  2770,  2772,  2774,  2776,
    2778,  2780,  2784,  2788,  2793,  2798,  2802,  2804,  2806,  2808,
    2810,  2814,  2816,  2821,  2825,  2827,  2829,  2831,  2833,  2835,
    2839,  2843,  2848,  2853,  2857,  2859,  2861,  2869,  2879,  2887,
    2894,  2903,  2905,  2908,  2913,  2918,  2920,  2922,  2924,  2929,
    2931,  2932,  2934,  2937,  2939,  2941,  2943,  2947,  2951,  2955,
    2956,  2958,  2960,  2964,  2968,  2971,  2975,  2982,  2983,  2985,
    2990,  2993,  2994,  3000,  3004,  3008,  3010,  3017,  3022,  3027,
    3030,  3033,  3034,  3040,  3044,  3048,  3050,  3053,  3054,  3060,
    3064,  3068,  3070,  3073,  3076,  3078,  3081,  3083,  3088,  3092,
    3096,  3103,  3107,  3109,  3111,  3113,  3118,  3123,  3128,  3133,
    3138,  3143,  3146,  3149,  3154,  3157,  3160,  3162,  3166,  3170,
    3174,  3175,  3178,  3184,  3191,  3198,  3206,  3208,  3211,  3213,
    3216,  3218,  3223,  3225,  3230,  3234,  3235,  3237,  3241,  3244,
    3248,  3250,  3252,  3253,  3254,  3257,  3260,  3263,  3266,  3271,
    3274,  3280,  3284,  3286,  3288,  3289,  3293,  3298,  3304,  3308,
    3310,  3313,  3314,  3319,  3321,  3325,  3328,  3333,  3339,  3342,
    3345,  3347,  3349,  3351,  3353,  3355,  3359,  3362,  3365,  3367,
    3376,  3383,  3385
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     202,     0,    -1,    -1,   203,   204,    -1,   204,   205,    -1,
      -1,   225,    -1,   242,    -1,   249,    -1,   246,    -1,   256,
      -1,   458,    -1,   129,   191,   192,   193,    -1,   157,   218,
     193,    -1,    -1,   157,   218,   194,   206,   204,   195,    -1,
      -1,   157,   194,   207,   204,   195,    -1,   117,   213,   193,
      -1,   117,   111,   213,   193,    -1,   117,   112,   213,   193,
      -1,   117,   211,   194,   216,   195,   193,    -1,   117,   111,
     211,   194,   213,   195,   193,    -1,   117,   112,   211,   194,
     213,   195,   193,    -1,   222,   193,    -1,    81,    -1,   103,
      -1,   163,    -1,   164,    -1,   166,    -1,   168,    -1,   167,
      -1,   208,    -1,   140,    -1,   169,    -1,   132,    -1,   133,
      -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,   120,
      -1,   119,    -1,   112,    -1,   101,    -1,    97,    -1,    99,
      -1,    77,    -1,    95,    -1,    12,    -1,   118,    -1,   109,
      -1,    57,    -1,   171,    -1,   131,    -1,   157,    -1,    72,
      -1,    10,    -1,    11,    -1,   114,    -1,   117,    -1,   125,
      -1,    73,    -1,   137,    -1,    71,    -1,     7,    -1,     6,
      -1,   116,    -1,   138,    -1,    13,    -1,    92,    -1,     4,
      -1,     3,    -1,   113,    -1,    76,    -1,    75,    -1,   107,
      -1,   108,    -1,   110,    -1,   104,    -1,    27,    -1,    29,
      -1,   111,    -1,    74,    -1,   105,    -1,   174,    -1,    96,
      -1,    98,    -1,   100,    -1,   106,    -1,    93,    -1,    94,
      -1,   102,    -1,   115,    -1,   126,    -1,   139,    -1,   209,
      -1,   130,    -1,   218,   160,    -1,   160,   218,   160,    -1,
     212,     9,   214,    -1,   214,    -1,   212,   402,    -1,   218,
      -1,   160,   218,    -1,   218,   102,   208,    -1,   160,   218,
     102,   208,    -1,   215,     9,   217,    -1,   217,    -1,   215,
     402,    -1,   214,    -1,   111,   214,    -1,   112,   214,    -1,
     208,    -1,   218,   160,   208,    -1,   218,    -1,   157,   160,
     218,    -1,   160,   218,    -1,   219,   463,    -1,   219,   463,
      -1,   222,     9,   459,    14,   397,    -1,   112,   459,    14,
     397,    -1,   223,   224,    -1,    -1,   225,    -1,   242,    -1,
     249,    -1,   256,    -1,   194,   223,   195,    -1,    74,   332,
     225,   278,   280,    -1,    74,   332,    32,   223,   279,   281,
      77,   193,    -1,    -1,    94,   332,   226,   272,    -1,    -1,
      93,   227,   225,    94,   332,   193,    -1,    -1,    96,   191,
     334,   193,   334,   193,   334,   192,   228,   270,    -1,    -1,
     104,   332,   229,   275,    -1,   108,   193,    -1,   108,   343,
     193,    -1,   110,   193,    -1,   110,   343,   193,    -1,   113,
     193,    -1,   113,   343,   193,    -1,    27,   108,   193,    -1,
     118,   288,   193,    -1,   124,   290,   193,    -1,    92,   333,
     193,    -1,   149,   333,   193,    -1,   126,   191,   455,   192,
     193,    -1,   193,    -1,    86,    -1,    87,    -1,    -1,    98,
     191,   343,   102,   269,   268,   192,   230,   271,    -1,    -1,
      98,   191,   343,    28,   102,   269,   268,   192,   231,   271,
      -1,   100,   191,   274,   192,   273,    -1,    -1,   114,   234,
     115,   191,   388,    83,   192,   194,   223,   195,   236,   232,
     239,    -1,    -1,   114,   234,   174,   233,   237,    -1,   116,
     343,   193,    -1,   109,   208,   193,    -1,   343,   193,    -1,
     335,   193,    -1,   336,   193,    -1,   337,   193,    -1,   338,
     193,    -1,   339,   193,    -1,   113,   338,   193,    -1,   340,
     193,    -1,   341,   193,    -1,   113,   340,   193,    -1,   342,
     193,    -1,   208,    32,    -1,    -1,   194,   235,   223,   195,
      -1,   236,   115,   191,   388,    83,   192,   194,   223,   195,
      -1,    -1,    -1,   194,   238,   223,   195,    -1,   174,   237,
      -1,    -1,    38,    -1,    -1,   111,    -1,    -1,   241,   240,
     462,   243,   191,   284,   192,   467,   318,    -1,    -1,   322,
     241,   240,   462,   244,   191,   284,   192,   467,   318,    -1,
      -1,   418,   321,   241,   240,   462,   245,   191,   284,   192,
     467,   318,    -1,    -1,   167,   208,   247,    32,   479,   457,
     194,   291,   195,    -1,    -1,   418,   167,   208,   248,    32,
     479,   457,   194,   291,   195,    -1,    -1,   262,   259,   250,
     263,   264,   194,   294,   195,    -1,    -1,   418,   262,   259,
     251,   263,   264,   194,   294,   195,    -1,    -1,   131,   260,
     252,   265,   194,   294,   195,    -1,    -1,   418,   131,   260,
     253,   265,   194,   294,   195,    -1,    -1,   130,   255,   395,
     263,   264,   194,   294,   195,    -1,    -1,   169,   261,   257,
     264,   194,   294,   195,    -1,    -1,   418,   169,   261,   258,
     264,   194,   294,   195,    -1,   462,    -1,   161,    -1,   462,
      -1,   462,    -1,   130,    -1,   123,   130,    -1,   123,   122,
     130,    -1,   122,   123,   130,    -1,   122,   130,    -1,   132,
     388,    -1,    -1,   133,   266,    -1,    -1,   132,   266,    -1,
      -1,   388,    -1,   266,     9,   388,    -1,   388,    -1,   267,
       9,   388,    -1,   136,   269,    -1,    -1,   430,    -1,    38,
     430,    -1,   137,   191,   444,   192,    -1,   225,    -1,    32,
     223,    97,   193,    -1,   225,    -1,    32,   223,    99,   193,
      -1,   225,    -1,    32,   223,    95,   193,    -1,   225,    -1,
      32,   223,   101,   193,    -1,   208,    14,   397,    -1,   274,
       9,   208,    14,   397,    -1,   194,   276,   195,    -1,   194,
     193,   276,   195,    -1,    32,   276,   105,   193,    -1,    32,
     193,   276,   105,   193,    -1,   276,   106,   343,   277,   223,
      -1,   276,   107,   277,   223,    -1,    -1,    32,    -1,   193,
      -1,   278,    75,   332,   225,    -1,    -1,   279,    75,   332,
      32,   223,    -1,    -1,    76,   225,    -1,    -1,    76,    32,
     223,    -1,    -1,   283,     9,   419,   324,   480,   170,    83,
      -1,   283,     9,   419,   324,   480,    38,   170,    83,    -1,
     283,     9,   419,   324,   480,   170,    -1,   283,   402,    -1,
     419,   324,   480,   170,    83,    -1,   419,   324,   480,    38,
     170,    83,    -1,   419,   324,   480,   170,    -1,    -1,   419,
     324,   480,    83,    -1,   419,   324,   480,    38,    83,    -1,
     419,   324,   480,    38,    83,    14,   343,    -1,   419,   324,
     480,    83,    14,   343,    -1,   283,     9,   419,   324,   480,
      83,    -1,   283,     9,   419,   324,   480,    38,    83,    -1,
     283,     9,   419,   324,   480,    38,    83,    14,   343,    -1,
     283,     9,   419,   324,   480,    83,    14,   343,    -1,   285,
       9,   419,   480,   170,    83,    -1,   285,     9,   419,   480,
      38,   170,    83,    -1,   285,     9,   419,   480,   170,    -1,
     285,   402,    -1,   419,   480,   170,    83,    -1,   419,   480,
      38,   170,    83,    -1,   419,   480,   170,    -1,    -1,   419,
     480,    83,    -1,   419,   480,    38,    83,    -1,   419,   480,
      38,    83,    14,   343,    -1,   419,   480,    83,    14,   343,
      -1,   285,     9,   419,   480,    83,    -1,   285,     9,   419,
     480,    38,    83,    -1,   285,     9,   419,   480,    38,    83,
      14,   343,    -1,   285,     9,   419,   480,    83,    14,   343,
      -1,   287,   402,    -1,    -1,   343,    -1,    38,   430,    -1,
     170,   343,    -1,   287,     9,   343,    -1,   287,     9,   170,
     343,    -1,   287,     9,    38,   430,    -1,   288,     9,   289,
      -1,   289,    -1,    83,    -1,   196,   430,    -1,   196,   194,
     343,   195,    -1,   290,     9,    83,    -1,   290,     9,    83,
      14,   397,    -1,    83,    -1,    83,    14,   397,    -1,   291,
     292,    -1,    -1,   293,   193,    -1,   460,    14,   397,    -1,
     294,   295,    -1,    -1,    -1,   320,   296,   326,   193,    -1,
      -1,   322,   479,   297,   326,   193,    -1,   327,   193,    -1,
     328,   193,    -1,   329,   193,    -1,    -1,   321,   241,   240,
     461,   191,   298,   282,   192,   467,   319,    -1,    -1,   418,
     321,   241,   240,   462,   191,   299,   282,   192,   467,   319,
      -1,   163,   304,   193,    -1,   164,   312,   193,    -1,   166,
     314,   193,    -1,     4,   132,   388,   193,    -1,     4,   133,
     388,   193,    -1,   117,   267,   193,    -1,   117,   267,   194,
     300,   195,    -1,   300,   301,    -1,   300,   302,    -1,    -1,
     221,   156,   208,   171,   267,   193,    -1,   303,   102,   321,
     208,   193,    -1,   303,   102,   322,   193,    -1,   221,   156,
     208,    -1,   208,    -1,   305,    -1,   304,     9,   305,    -1,
     306,   385,   310,   311,    -1,   161,    -1,    31,   307,    -1,
     307,    -1,   138,    -1,   138,   177,   479,   401,   178,    -1,
     138,   177,   479,     9,   479,   178,    -1,   388,    -1,   125,
      -1,   167,   194,   309,   195,    -1,   140,    -1,   396,    -1,
     308,     9,   396,    -1,   308,   401,    -1,    14,   397,    -1,
      -1,    59,   168,    -1,    -1,   313,    -1,   312,     9,   313,
      -1,   165,    -1,   315,    -1,   208,    -1,   128,    -1,   191,
     316,   192,    -1,   191,   316,   192,    53,    -1,   191,   316,
     192,    31,    -1,   191,   316,   192,    50,    -1,   315,    -1,
     317,    -1,   317,    53,    -1,   317,    31,    -1,   317,    50,
      -1,   316,     9,   316,    -1,   316,    36,   316,    -1,   208,
      -1,   161,    -1,   165,    -1,   193,    -1,   194,   223,   195,
      -1,   193,    -1,   194,   223,   195,    -1,   322,    -1,   125,
      -1,   322,    -1,    -1,   323,    -1,   322,   323,    -1,   119,
      -1,   120,    -1,   121,    -1,   124,    -1,   123,    -1,   122,
      -1,   187,    -1,   325,    -1,    -1,   119,    -1,   120,    -1,
     121,    -1,   326,     9,    83,    -1,   326,     9,    83,    14,
     397,    -1,    83,    -1,    83,    14,   397,    -1,   327,     9,
     460,    14,   397,    -1,   112,   460,    14,   397,    -1,   328,
       9,   460,    -1,   123,   112,   460,    -1,   123,   330,   457,
      -1,   330,   457,    14,   479,    -1,   112,   182,   462,    -1,
     191,   331,   192,    -1,    72,   392,   395,    -1,    72,   254,
      -1,    71,   343,    -1,   377,    -1,   372,    -1,   191,   343,
     192,    -1,   333,     9,   343,    -1,   343,    -1,   333,    -1,
      -1,    27,    -1,    27,   343,    -1,    27,   343,   136,   343,
      -1,   191,   335,   192,    -1,   430,    14,   335,    -1,   137,
     191,   444,   192,    14,   335,    -1,    29,   343,    -1,   430,
      14,   338,    -1,    28,   343,    -1,   430,    14,   340,    -1,
     137,   191,   444,   192,    14,   340,    -1,   344,    -1,   430,
      -1,   331,    -1,   434,    -1,   433,    -1,   137,   191,   444,
     192,    14,   343,    -1,   430,    14,   343,    -1,   430,    14,
      38,   430,    -1,   430,    14,    38,    72,   392,   395,    -1,
     430,    26,   343,    -1,   430,    25,   343,    -1,   430,    24,
     343,    -1,   430,    23,   343,    -1,   430,    22,   343,    -1,
     430,    21,   343,    -1,   430,    20,   343,    -1,   430,    19,
     343,    -1,   430,    18,   343,    -1,   430,    17,   343,    -1,
     430,    16,   343,    -1,   430,    15,   343,    -1,   430,    68,
      -1,    68,   430,    -1,   430,    67,    -1,    67,   430,    -1,
     343,    34,   343,    -1,   343,    35,   343,    -1,   343,    10,
     343,    -1,   343,    12,   343,    -1,   343,    11,   343,    -1,
     343,    36,   343,    -1,   343,    38,   343,    -1,   343,    37,
     343,    -1,   343,    52,   343,    -1,   343,    50,   343,    -1,
     343,    51,   343,    -1,   343,    53,   343,    -1,   343,    54,
     343,    -1,   343,    69,   343,    -1,   343,    55,   343,    -1,
     343,    30,   343,    -1,   343,    49,   343,    -1,   343,    48,
     343,    -1,    50,   343,    -1,    51,   343,    -1,    56,   343,
      -1,    58,   343,    -1,   343,    40,   343,    -1,   343,    39,
     343,    -1,   343,    42,   343,    -1,   343,    41,   343,    -1,
     343,    43,   343,    -1,   343,    47,   343,    -1,   343,    44,
     343,    -1,   343,    46,   343,    -1,   343,    45,   343,    -1,
     343,    57,   392,    -1,   191,   344,   192,    -1,   343,    31,
     343,    32,   343,    -1,   343,    31,    32,   343,    -1,   343,
      33,   343,    -1,   454,    -1,    66,   343,    -1,    65,   343,
      -1,    64,   343,    -1,    63,   343,    -1,    62,   343,    -1,
      61,   343,    -1,    60,   343,    -1,    73,   393,    -1,    59,
     343,    -1,   399,    -1,   362,    -1,   369,    -1,   361,    -1,
     197,   394,   197,    -1,    13,   343,    -1,   374,    -1,   117,
     191,   376,   402,   192,    -1,    -1,    -1,   241,   240,   191,
     347,   284,   192,   467,   345,   467,   194,   223,   195,    -1,
      -1,   322,   241,   240,   191,   348,   284,   192,   467,   345,
     467,   194,   223,   195,    -1,    -1,   187,    83,   350,   355,
      -1,    -1,   187,   188,   351,   284,   189,   467,   355,    -1,
      -1,   187,   194,   352,   223,   195,    -1,    -1,    83,   353,
     355,    -1,    -1,   188,   354,   284,   189,   467,   355,    -1,
       8,   343,    -1,     8,   340,    -1,     8,   194,   223,   195,
      -1,    91,    -1,   456,    -1,   357,     9,   356,   136,   343,
      -1,   356,   136,   343,    -1,   358,     9,   356,   136,   397,
      -1,   356,   136,   397,    -1,   357,   401,    -1,    -1,   358,
     401,    -1,    -1,   181,   191,   359,   192,    -1,   138,   191,
     445,   192,    -1,    70,   445,   198,    -1,   364,   401,    -1,
      -1,   364,     9,   343,   136,   343,    -1,   343,   136,   343,
      -1,   364,     9,   343,   136,    38,   430,    -1,   343,   136,
      38,   430,    -1,   366,   401,    -1,    -1,   366,     9,   397,
     136,   397,    -1,   397,   136,   397,    -1,   368,   401,    -1,
      -1,   368,     9,   407,   136,   407,    -1,   407,   136,   407,
      -1,   139,    70,   363,   198,    -1,   139,    70,   365,   198,
      -1,   139,    70,   367,   198,    -1,   388,   194,   447,   195,
      -1,   388,   194,   449,   195,    -1,   374,    70,   440,   198,
      -1,   375,    70,   440,   198,    -1,   362,    -1,   369,    -1,
     456,    -1,   433,    -1,    91,    -1,   191,   344,   192,    -1,
     376,     9,    83,    -1,   376,     9,    38,    83,    -1,    83,
      -1,    38,    83,    -1,   175,   161,   378,   176,    -1,   380,
      54,    -1,   380,   176,   381,   175,    54,   379,    -1,    -1,
     161,    -1,   380,   382,    14,   383,    -1,    -1,   381,   384,
      -1,    -1,   161,    -1,   162,    -1,   194,   343,   195,    -1,
     162,    -1,   194,   343,   195,    -1,   377,    -1,   386,    -1,
     385,    32,   386,    -1,   385,    51,   386,    -1,   208,    -1,
      73,    -1,   111,    -1,   112,    -1,   113,    -1,    27,    -1,
      29,    -1,    28,    -1,   114,    -1,   115,    -1,   174,    -1,
     116,    -1,    74,    -1,    75,    -1,    77,    -1,    76,    -1,
      94,    -1,    95,    -1,    93,    -1,    96,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   101,    -1,    57,    -1,
     102,    -1,   104,    -1,   105,    -1,   106,    -1,   107,    -1,
     108,    -1,   110,    -1,   109,    -1,    92,    -1,    13,    -1,
     130,    -1,   131,    -1,   132,    -1,   133,    -1,    72,    -1,
      71,    -1,   125,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   157,    -1,   117,    -1,   118,    -1,
     127,    -1,   128,    -1,   129,    -1,   124,    -1,   123,    -1,
     122,    -1,   121,    -1,   120,    -1,   119,    -1,   187,    -1,
     126,    -1,   137,    -1,   138,    -1,    10,    -1,    12,    -1,
      11,    -1,   141,    -1,   143,    -1,   142,    -1,   144,    -1,
     145,    -1,   159,    -1,   158,    -1,   186,    -1,   169,    -1,
     172,    -1,   171,    -1,   182,    -1,   184,    -1,   181,    -1,
     139,    -1,   220,   191,   286,   192,    -1,   139,   191,   286,
     192,    -1,   221,    -1,   161,    -1,   388,    -1,   396,    -1,
     124,    -1,   438,    -1,   191,   344,   192,    -1,   389,    -1,
     390,   156,   437,    -1,   389,    -1,   436,    -1,   391,   156,
     437,    -1,   388,    -1,   124,    -1,   442,    -1,   191,   192,
      -1,   332,    -1,    -1,    -1,    90,    -1,   451,    -1,   191,
     286,   192,    -1,    -1,    78,    -1,    79,    -1,    80,    -1,
      91,    -1,   144,    -1,   145,    -1,   159,    -1,   141,    -1,
     172,    -1,   142,    -1,   143,    -1,   158,    -1,   186,    -1,
     152,    90,   153,    -1,   152,   153,    -1,   396,    -1,   219,
      -1,   138,   191,   400,   192,    -1,    70,   400,   198,    -1,
     181,   191,   360,   192,    -1,   370,    -1,   398,    -1,   373,
      -1,   191,   397,   192,    -1,   397,    34,   397,    -1,   397,
      35,   397,    -1,   397,    10,   397,    -1,   397,    12,   397,
      -1,   397,    11,   397,    -1,   397,    36,   397,    -1,   397,
      38,   397,    -1,   397,    37,   397,    -1,   397,    52,   397,
      -1,   397,    50,   397,    -1,   397,    51,   397,    -1,   397,
      53,   397,    -1,   397,    54,   397,    -1,   397,    55,   397,
      -1,   397,    49,   397,    -1,   397,    48,   397,    -1,   397,
      69,   397,    -1,    56,   397,    -1,    58,   397,    -1,    50,
     397,    -1,    51,   397,    -1,   397,    40,   397,    -1,   397,
      39,   397,    -1,   397,    42,   397,    -1,   397,    41,   397,
      -1,   397,    43,   397,    -1,   397,    47,   397,    -1,   397,
      44,   397,    -1,   397,    46,   397,    -1,   397,    45,   397,
      -1,   397,    31,   397,    32,   397,    -1,   397,    31,    32,
     397,    -1,   221,   156,   209,    -1,   161,   156,   209,    -1,
     221,   156,   130,    -1,   219,    -1,    82,    -1,   456,    -1,
     396,    -1,   199,   451,   199,    -1,   200,   451,   200,    -1,
     152,   451,   153,    -1,   403,   401,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   403,     9,   397,   136,   397,    -1,
     403,     9,   397,    -1,   397,   136,   397,    -1,   397,    -1,
      78,    -1,    79,    -1,    80,    -1,   152,    90,   153,    -1,
     152,   153,    -1,    78,    -1,    79,    -1,    80,    -1,   208,
      -1,    91,    -1,    91,    52,   406,    -1,   404,    -1,   406,
      -1,   208,    -1,    50,   405,    -1,    51,   405,    -1,   138,
     191,   408,   192,    -1,    70,   408,   198,    -1,   181,   191,
     411,   192,    -1,   371,    -1,   409,   401,    -1,    -1,   409,
       9,   407,   136,   407,    -1,   409,     9,   407,    -1,   407,
     136,   407,    -1,   407,    -1,   410,     9,   407,    -1,   407,
      -1,   412,   401,    -1,    -1,   412,     9,   356,   136,   407,
      -1,   356,   136,   407,    -1,   410,   401,    -1,    -1,   191,
     413,   192,    -1,    -1,   415,     9,   208,   414,    -1,   208,
     414,    -1,    -1,   417,   415,   401,    -1,    49,   416,    48,
      -1,   418,    -1,    -1,   134,    -1,   135,    -1,   208,    -1,
     161,    -1,   194,   343,   195,    -1,   421,    -1,   437,    -1,
     208,    -1,   194,   343,   195,    -1,   423,    -1,   437,    -1,
      70,   440,   198,    -1,   194,   343,   195,    -1,   431,   425,
      -1,   191,   331,   192,   425,    -1,   443,   425,    -1,   191,
     331,   192,   425,    -1,   191,   331,   192,   420,   422,    -1,
     191,   344,   192,   420,   422,    -1,   191,   331,   192,   420,
     421,    -1,   191,   344,   192,   420,   421,    -1,   437,    -1,
     387,    -1,   435,    -1,   436,    -1,   426,    -1,   428,    -1,
     430,   420,   422,    -1,   391,   156,   437,    -1,   432,   191,
     286,   192,    -1,   433,   191,   286,   192,    -1,   191,   430,
     192,    -1,   387,    -1,   435,    -1,   436,    -1,   426,    -1,
     430,   420,   421,    -1,   429,    -1,   432,   191,   286,   192,
      -1,   191,   430,   192,    -1,   437,    -1,   426,    -1,   387,
      -1,   362,    -1,   396,    -1,   191,   430,   192,    -1,   191,
     344,   192,    -1,   433,   191,   286,   192,    -1,   432,   191,
     286,   192,    -1,   191,   434,   192,    -1,   346,    -1,   349,
      -1,   430,   420,   424,   463,   191,   286,   192,    -1,   191,
     331,   192,   420,   424,   463,   191,   286,   192,    -1,   391,
     156,   210,   463,   191,   286,   192,    -1,   391,   156,   437,
     191,   286,   192,    -1,   391,   156,   194,   343,   195,   191,
     286,   192,    -1,   438,    -1,   441,   438,    -1,   438,    70,
     440,   198,    -1,   438,   194,   343,   195,    -1,   439,    -1,
      83,    -1,    84,    -1,   196,   194,   343,   195,    -1,   343,
      -1,    -1,   196,    -1,   441,   196,    -1,   437,    -1,   427,
      -1,   428,    -1,   442,   420,   422,    -1,   390,   156,   437,
      -1,   191,   430,   192,    -1,    -1,   427,    -1,   429,    -1,
     442,   420,   421,    -1,   191,   430,   192,    -1,   444,     9,
      -1,   444,     9,   430,    -1,   444,     9,   137,   191,   444,
     192,    -1,    -1,   430,    -1,   137,   191,   444,   192,    -1,
     446,   401,    -1,    -1,   446,     9,   343,   136,   343,    -1,
     446,     9,   343,    -1,   343,   136,   343,    -1,   343,    -1,
     446,     9,   343,   136,    38,   430,    -1,   446,     9,    38,
     430,    -1,   343,   136,    38,   430,    -1,    38,   430,    -1,
     448,   401,    -1,    -1,   448,     9,   343,   136,   343,    -1,
     448,     9,   343,    -1,   343,   136,   343,    -1,   343,    -1,
     450,   401,    -1,    -1,   450,     9,   397,   136,   397,    -1,
     450,     9,   397,    -1,   397,   136,   397,    -1,   397,    -1,
     451,   452,    -1,   451,    90,    -1,   452,    -1,    90,   452,
      -1,    83,    -1,    83,    70,   453,   198,    -1,    83,   420,
     208,    -1,   154,   343,   195,    -1,   154,    82,    70,   343,
     198,   195,    -1,   155,   430,   195,    -1,   208,    -1,    85,
      -1,    83,    -1,   127,   191,   333,   192,    -1,   128,   191,
     430,   192,    -1,   128,   191,   344,   192,    -1,   128,   191,
     434,   192,    -1,   128,   191,   433,   192,    -1,   128,   191,
     331,   192,    -1,     7,   343,    -1,     6,   343,    -1,     5,
     191,   343,   192,    -1,     4,   343,    -1,     3,   343,    -1,
     430,    -1,   455,     9,   430,    -1,   391,   156,   209,    -1,
     391,   156,   130,    -1,    -1,   102,   479,    -1,   182,   462,
      14,   479,   193,    -1,   418,   182,   462,    14,   479,   193,
      -1,   184,   462,   457,    14,   479,   193,    -1,   418,   184,
     462,   457,    14,   479,   193,    -1,   210,    -1,   479,   210,
      -1,   209,    -1,   479,   209,    -1,   210,    -1,   210,   177,
     469,   178,    -1,   208,    -1,   208,   177,   469,   178,    -1,
     177,   465,   178,    -1,    -1,   479,    -1,   464,     9,   479,
      -1,   464,   401,    -1,   464,     9,   170,    -1,   465,    -1,
     170,    -1,    -1,    -1,    32,   479,    -1,   102,   479,    -1,
     103,   479,    -1,   470,   401,    -1,   470,     9,   471,   208,
      -1,   471,   208,    -1,   470,     9,   471,   208,   468,    -1,
     471,   208,   468,    -1,    50,    -1,    51,    -1,    -1,    91,
     136,   479,    -1,    31,    91,   136,   479,    -1,   221,   156,
     208,   136,   479,    -1,   473,     9,   472,    -1,   472,    -1,
     473,   401,    -1,    -1,   181,   191,   474,   192,    -1,   221,
      -1,   208,   156,   477,    -1,   208,   463,    -1,   177,   479,
     401,   178,    -1,   177,   479,     9,   479,   178,    -1,    31,
     479,    -1,    59,   479,    -1,   221,    -1,   138,    -1,   139,
      -1,   140,    -1,   475,    -1,   476,   156,   477,    -1,   138,
     478,    -1,   139,   478,    -1,   161,    -1,   191,   111,   191,
     466,   192,    32,   479,   192,    -1,   191,   479,     9,   464,
     401,   192,    -1,   479,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   731,   731,   731,   740,   742,   745,   746,   747,   748,
     749,   750,   751,   754,   756,   756,   758,   758,   760,   762,
     765,   768,   772,   776,   780,   785,   786,   787,   788,   789,
     790,   791,   795,   796,   797,   798,   799,   800,   801,   802,
     803,   804,   805,   806,   807,   808,   809,   810,   811,   812,
     813,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     853,   854,   855,   856,   860,   861,   865,   866,   870,   871,
     876,   878,   883,   888,   889,   890,   892,   897,   899,   904,
     909,   911,   913,   918,   919,   923,   924,   926,   930,   937,
     944,   948,   954,   956,   959,   960,   961,   962,   965,   966,
     970,   975,   975,   981,   981,   988,   987,   993,   993,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1016,  1014,  1023,  1021,  1028,  1038,
    1032,  1042,  1040,  1044,  1045,  1049,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1068,  1068,  1073,
    1079,  1083,  1083,  1091,  1092,  1096,  1097,  1101,  1107,  1105,
    1120,  1117,  1133,  1130,  1147,  1146,  1155,  1153,  1165,  1164,
    1183,  1181,  1200,  1199,  1208,  1206,  1217,  1217,  1224,  1223,
    1235,  1233,  1246,  1247,  1251,  1254,  1257,  1258,  1259,  1262,
    1263,  1266,  1268,  1271,  1272,  1275,  1276,  1279,  1280,  1284,
    1285,  1290,  1291,  1294,  1295,  1296,  1300,  1301,  1305,  1306,
    1310,  1311,  1315,  1316,  1321,  1322,  1328,  1329,  1330,  1331,
    1334,  1337,  1339,  1342,  1343,  1347,  1349,  1352,  1355,  1358,
    1359,  1362,  1363,  1367,  1373,  1379,  1386,  1388,  1393,  1398,
    1404,  1408,  1412,  1416,  1421,  1426,  1431,  1436,  1442,  1451,
    1456,  1461,  1467,  1469,  1473,  1477,  1482,  1486,  1489,  1492,
    1496,  1500,  1504,  1508,  1513,  1521,  1523,  1526,  1527,  1528,
    1529,  1531,  1533,  1538,  1539,  1542,  1543,  1544,  1548,  1549,
    1551,  1552,  1556,  1558,  1561,  1565,  1571,  1573,  1576,  1576,
    1580,  1579,  1583,  1585,  1588,  1591,  1589,  1605,  1601,  1615,
    1617,  1619,  1621,  1623,  1625,  1627,  1631,  1632,  1633,  1636,
    1642,  1646,  1652,  1655,  1660,  1662,  1667,  1672,  1676,  1677,
    1681,  1682,  1684,  1686,  1692,  1693,  1695,  1699,  1700,  1705,
    1709,  1710,  1714,  1715,  1719,  1721,  1727,  1732,  1733,  1735,
    1739,  1740,  1741,  1742,  1746,  1747,  1748,  1749,  1750,  1751,
    1753,  1758,  1761,  1762,  1766,  1767,  1771,  1772,  1775,  1776,
    1779,  1780,  1783,  1784,  1788,  1789,  1790,  1791,  1792,  1793,
    1794,  1798,  1799,  1802,  1803,  1804,  1807,  1809,  1811,  1812,
    1815,  1817,  1821,  1823,  1827,  1831,  1835,  1840,  1841,  1843,
    1844,  1845,  1846,  1849,  1853,  1854,  1858,  1859,  1863,  1864,
    1865,  1866,  1870,  1874,  1879,  1883,  1887,  1891,  1895,  1900,
    1901,  1902,  1903,  1904,  1908,  1910,  1911,  1912,  1915,  1916,
    1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,
    1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,
    1937,  1938,  1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,
    1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,
    1957,  1958,  1960,  1961,  1963,  1964,  1966,  1967,  1968,  1969,
    1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,  1979,
    1980,  1981,  1982,  1983,  1984,  1985,  1986,  1990,  1994,  1999,
    1998,  2013,  2011,  2029,  2028,  2047,  2046,  2065,  2064,  2082,
    2082,  2097,  2097,  2115,  2116,  2117,  2122,  2124,  2128,  2132,
    2138,  2142,  2148,  2150,  2154,  2156,  2160,  2164,  2165,  2169,
    2171,  2175,  2177,  2178,  2181,  2185,  2187,  2191,  2194,  2199,
    2201,  2205,  2208,  2213,  2217,  2221,  2225,  2232,  2239,  2241,
    2246,  2247,  2248,  2249,  2250,  2252,  2256,  2257,  2258,  2259,
    2263,  2269,  2278,  2291,  2292,  2295,  2298,  2301,  2302,  2305,
    2309,  2312,  2315,  2322,  2323,  2327,  2328,  2330,  2334,  2335,
    2336,  2337,  2338,  2339,  2340,  2341,  2342,  2343,  2344,  2345,
    2346,  2347,  2348,  2349,  2350,  2351,  2352,  2353,  2354,  2355,
    2356,  2357,  2358,  2359,  2360,  2361,  2362,  2363,  2364,  2365,
    2366,  2367,  2368,  2369,  2370,  2371,  2372,  2373,  2374,  2375,
    2376,  2377,  2378,  2379,  2380,  2381,  2382,  2383,  2384,  2385,
    2386,  2387,  2388,  2389,  2390,  2391,  2392,  2393,  2394,  2395,
    2396,  2397,  2398,  2399,  2400,  2401,  2402,  2403,  2404,  2405,
    2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,
    2419,  2421,  2427,  2428,  2432,  2433,  2434,  2435,  2437,  2441,
    2442,  2453,  2454,  2456,  2468,  2469,  2470,  2474,  2475,  2476,
    2480,  2481,  2482,  2485,  2487,  2491,  2492,  2493,  2494,  2496,
    2497,  2498,  2499,  2500,  2501,  2502,  2503,  2504,  2505,  2508,
    2513,  2514,  2515,  2517,  2518,  2520,  2521,  2522,  2523,  2524,
    2526,  2528,  2530,  2532,  2534,  2535,  2536,  2537,  2538,  2539,
    2540,  2541,  2542,  2543,  2544,  2545,  2546,  2547,  2548,  2549,
    2550,  2552,  2554,  2556,  2558,  2559,  2562,  2563,  2567,  2571,
    2573,  2577,  2580,  2583,  2589,  2590,  2591,  2592,  2593,  2594,
    2595,  2600,  2602,  2606,  2607,  2610,  2611,  2615,  2618,  2620,
    2622,  2626,  2627,  2628,  2629,  2632,  2636,  2637,  2638,  2639,
    2643,  2645,  2652,  2653,  2654,  2655,  2656,  2657,  2659,  2660,
    2662,  2666,  2668,  2671,  2674,  2676,  2678,  2681,  2683,  2687,
    2689,  2692,  2695,  2701,  2703,  2706,  2707,  2712,  2715,  2719,
    2719,  2724,  2727,  2728,  2732,  2733,  2737,  2738,  2739,  2743,
    2745,  2753,  2754,  2758,  2760,  2768,  2769,  2773,  2774,  2779,
    2781,  2786,  2797,  2811,  2823,  2838,  2839,  2840,  2841,  2842,
    2843,  2844,  2854,  2863,  2865,  2867,  2871,  2872,  2873,  2874,
    2875,  2891,  2892,  2894,  2903,  2904,  2905,  2906,  2907,  2908,
    2909,  2910,  2912,  2917,  2921,  2922,  2926,  2929,  2936,  2940,
    2949,  2956,  2958,  2964,  2966,  2967,  2971,  2972,  2973,  2980,
    2981,  2986,  2987,  2992,  2993,  2994,  2995,  3006,  3009,  3012,
    3013,  3014,  3015,  3026,  3030,  3031,  3032,  3034,  3035,  3036,
    3040,  3042,  3045,  3047,  3048,  3049,  3050,  3053,  3055,  3056,
    3060,  3062,  3065,  3067,  3068,  3069,  3073,  3075,  3078,  3081,
    3083,  3085,  3089,  3090,  3092,  3093,  3099,  3100,  3102,  3112,
    3114,  3116,  3119,  3120,  3121,  3125,  3126,  3127,  3128,  3129,
    3130,  3131,  3132,  3133,  3134,  3135,  3139,  3140,  3144,  3146,
    3154,  3156,  3160,  3164,  3169,  3173,  3181,  3182,  3186,  3187,
    3193,  3194,  3203,  3204,  3212,  3215,  3219,  3222,  3227,  3232,
    3234,  3235,  3236,  3240,  3241,  3245,  3246,  3249,  3254,  3257,
    3259,  3263,  3269,  3270,  3271,  3275,  3279,  3289,  3297,  3299,
    3303,  3305,  3310,  3316,  3319,  3324,  3329,  3331,  3338,  3341,
    3344,  3345,  3348,  3351,  3354,  3355,  3360,  3362,  3364,  3368,
    3374,  3384,  3385
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
  "T_PIPE", "'?'", "':'", "\"??\"", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_SPACESHIP", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "T_POW", "'['",
  "T_CLONE", "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF",
  "T_LNUMBER", "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME",
  "T_VARIABLE", "T_PIPE_VAR", "T_NUM_STRING", "T_INLINE_HTML",
  "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE",
  "T_FINAL", "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET",
  "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS",
  "T_IMPLEMENTS", "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_CALLABLE",
  "T_CLASS_C", "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
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
  "dict_pair_list", "non_empty_dict_pair_list", "static_dict_pair_list",
  "non_empty_static_dict_pair_list", "static_dict_pair_list_ae",
  "non_empty_static_dict_pair_list_ae", "dict_literal",
  "static_dict_literal", "static_dict_literal_ae", "collection_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
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
  "hh_access_type", "array_typelist", "hh_type", "hh_type_opt", 0
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
     283,    63,    58,   284,   285,   286,   124,    94,    38,   287,
     288,   289,   290,    60,    62,   291,   292,   293,   294,   295,
      43,    45,    46,    42,    47,    37,    33,   296,   126,    64,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
      91,   307,   308,   309,   310,   311,   312,   313,   314,   315,
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
     426,    40,    41,    59,   123,   125,    36,    96,    93,    34,
      39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   201,   203,   202,   204,   204,   205,   205,   205,   205,
     205,   205,   205,   205,   206,   205,   207,   205,   205,   205,
     205,   205,   205,   205,   205,   208,   208,   208,   208,   208,
     208,   208,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   210,   210,   211,   211,
     212,   212,   213,   214,   214,   214,   214,   215,   215,   216,
     217,   217,   217,   218,   218,   219,   219,   219,   220,   221,
     222,   222,   223,   223,   224,   224,   224,   224,   225,   225,
     225,   226,   225,   227,   225,   228,   225,   229,   225,   225,
     225,   225,   225,   225,   225,   225,   225,   225,   225,   225,
     225,   225,   225,   225,   230,   225,   231,   225,   225,   232,
     225,   233,   225,   225,   225,   225,   225,   225,   225,   225,
     225,   225,   225,   225,   225,   225,   225,   235,   234,   236,
     236,   238,   237,   239,   239,   240,   240,   241,   243,   242,
     244,   242,   245,   242,   247,   246,   248,   246,   250,   249,
     251,   249,   252,   249,   253,   249,   255,   254,   257,   256,
     258,   256,   259,   259,   260,   261,   262,   262,   262,   262,
     262,   263,   263,   264,   264,   265,   265,   266,   266,   267,
     267,   268,   268,   269,   269,   269,   270,   270,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   275,   275,
     276,   276,   276,   277,   277,   278,   278,   279,   279,   280,
     280,   281,   281,   282,   282,   282,   282,   282,   282,   282,
     282,   283,   283,   283,   283,   283,   283,   283,   283,   284,
     284,   284,   284,   284,   284,   284,   284,   285,   285,   285,
     285,   285,   285,   285,   285,   286,   286,   287,   287,   287,
     287,   287,   287,   288,   288,   289,   289,   289,   290,   290,
     290,   290,   291,   291,   292,   293,   294,   294,   296,   295,
     297,   295,   295,   295,   295,   298,   295,   299,   295,   295,
     295,   295,   295,   295,   295,   295,   300,   300,   300,   301,
     302,   302,   303,   303,   304,   304,   305,   305,   306,   306,
     307,   307,   307,   307,   307,   307,   307,   308,   308,   309,
     310,   310,   311,   311,   312,   312,   313,   314,   314,   314,
     315,   315,   315,   315,   316,   316,   316,   316,   316,   316,
     316,   317,   317,   317,   318,   318,   319,   319,   320,   320,
     321,   321,   322,   322,   323,   323,   323,   323,   323,   323,
     323,   324,   324,   325,   325,   325,   326,   326,   326,   326,
     327,   327,   328,   328,   329,   329,   330,   331,   331,   331,
     331,   331,   331,   332,   333,   333,   334,   334,   335,   335,
     335,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     343,   343,   343,   343,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   345,   345,   347,
     346,   348,   346,   350,   349,   351,   349,   352,   349,   353,
     349,   354,   349,   355,   355,   355,   356,   356,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   362,   362,   363,
     363,   364,   364,   364,   364,   365,   365,   366,   366,   367,
     367,   368,   368,   369,   370,   371,   372,   373,   374,   374,
     375,   375,   375,   375,   375,   375,   376,   376,   376,   376,
     377,   378,   378,   379,   379,   380,   380,   381,   381,   382,
     383,   383,   384,   384,   384,   385,   385,   385,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     387,   387,   388,   388,   389,   389,   389,   389,   389,   390,
     390,   391,   391,   391,   392,   392,   392,   393,   393,   393,
     394,   394,   394,   395,   395,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   398,   398,   398,   399,   399,   399,   399,   399,   399,
     399,   400,   400,   401,   401,   402,   402,   403,   403,   403,
     403,   404,   404,   404,   404,   404,   405,   405,   405,   405,
     406,   406,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   408,   408,   409,   409,   409,   409,   410,   410,   411,
     411,   412,   412,   413,   413,   414,   414,   415,   415,   417,
     416,   418,   419,   419,   420,   420,   421,   421,   421,   422,
     422,   423,   423,   424,   424,   425,   425,   426,   426,   427,
     427,   428,   428,   429,   429,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   431,   431,   431,   431,
     431,   431,   431,   431,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   433,   434,   434,   435,   435,   436,   436,
     436,   437,   437,   438,   438,   438,   439,   439,   439,   440,
     440,   441,   441,   442,   442,   442,   442,   442,   442,   443,
     443,   443,   443,   443,   444,   444,   444,   444,   444,   444,
     445,   445,   446,   446,   446,   446,   446,   446,   446,   446,
     447,   447,   448,   448,   448,   448,   449,   449,   450,   450,
     450,   450,   451,   451,   451,   451,   452,   452,   452,   452,
     452,   452,   453,   453,   453,   454,   454,   454,   454,   454,
     454,   454,   454,   454,   454,   454,   455,   455,   456,   456,
     457,   457,   458,   458,   458,   458,   459,   459,   460,   460,
     461,   461,   462,   462,   463,   463,   464,   464,   465,   466,
     466,   466,   466,   467,   467,   468,   468,   469,   470,   470,
     470,   470,   471,   471,   471,   472,   472,   472,   473,   473,
     474,   474,   475,   476,   477,   477,   478,   478,   479,   479,
     479,   479,   479,   479,   479,   479,   479,   479,   479,   479,
     479,   480,   480
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
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       3,     1,     2,     1,     2,     3,     4,     3,     1,     2,
       1,     2,     2,     1,     3,     1,     3,     2,     2,     2,
       5,     4,     2,     0,     1,     1,     1,     1,     3,     5,
       8,     0,     4,     0,     6,     0,    10,     0,     4,     2,
       3,     2,     3,     2,     3,     3,     3,     3,     3,     3,
       5,     1,     1,     1,     0,     9,     0,    10,     5,     0,
      13,     0,     5,     3,     3,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     3,     2,     2,     0,     4,     9,
       0,     0,     4,     2,     0,     1,     0,     1,     0,     9,
       0,    10,     0,    11,     0,     9,     0,    10,     0,     8,
       0,     9,     0,     7,     0,     8,     0,     8,     0,     7,
       0,     8,     1,     1,     1,     1,     1,     2,     3,     3,
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
       0,     5,     2,     2,     2,     0,    10,     0,    11,     3,
       3,     3,     4,     4,     3,     5,     2,     2,     0,     6,
       5,     4,     3,     1,     1,     3,     4,     1,     2,     1,
       1,     5,     6,     1,     1,     4,     1,     1,     3,     2,
       2,     0,     2,     0,     1,     3,     1,     1,     1,     1,
       3,     4,     4,     4,     1,     1,     2,     2,     2,     3,
       3,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     1,     1,     3,     5,     1,     3,
       5,     4,     3,     3,     3,     4,     3,     3,     3,     2,
       2,     1,     1,     3,     3,     1,     1,     0,     1,     2,
       4,     3,     3,     6,     2,     3,     2,     3,     6,     1,
       1,     1,     1,     1,     6,     3,     4,     6,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      12,     0,    13,     0,     4,     0,     7,     0,     5,     0,
       3,     0,     6,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     2,
       0,     5,     3,     6,     4,     2,     0,     5,     3,     2,
       0,     5,     3,     4,     4,     4,     4,     4,     4,     4,
       1,     1,     1,     1,     1,     3,     3,     4,     1,     2,
       4,     2,     6,     0,     1,     4,     0,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     4,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     3,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     4,     3,     4,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     2,     0,     1,     0,     1,     0,     5,     3,     3,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     2,     2,     4,     3,     4,
       1,     2,     0,     5,     3,     3,     1,     3,     1,     2,
       0,     5,     3,     2,     0,     3,     0,     4,     2,     0,
       3,     3,     1,     0,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     3,     3,     2,     4,     2,
       4,     5,     5,     5,     5,     1,     1,     1,     1,     1,
       1,     3,     3,     4,     4,     3,     1,     1,     1,     1,
       3,     1,     4,     3,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     7,     9,     7,     6,
       8,     1,     2,     4,     4,     1,     1,     1,     4,     1,
       0,     1,     2,     1,     1,     1,     3,     3,     3,     0,
       1,     1,     3,     3,     2,     3,     6,     0,     1,     4,
       2,     0,     5,     3,     3,     1,     6,     4,     4,     2,
       2,     0,     5,     3,     3,     1,     2,     0,     5,     3,
       3,     1,     2,     2,     1,     2,     1,     4,     3,     3,
       6,     3,     1,     1,     1,     4,     4,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     6,     7,     1,     2,     1,     2,
       1,     4,     1,     4,     3,     0,     1,     3,     2,     3,
       1,     1,     0,     0,     2,     2,     2,     2,     4,     2,
       5,     3,     1,     1,     0,     3,     4,     5,     3,     1,
       2,     0,     4,     1,     3,     2,     4,     5,     2,     2,
       1,     1,     1,     1,     1,     3,     2,     2,     1,     8,
       6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   428,     0,     0,   819,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   911,
       0,   899,   699,     0,   705,   706,   707,    25,   765,   886,
     887,   152,   153,   708,     0,   133,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   187,     0,     0,     0,     0,
       0,     0,   394,   395,   396,   399,   398,   397,     0,     0,
       0,     0,   216,     0,     0,     0,     0,   712,   714,   715,
     709,   710,     0,     0,     0,   716,   711,     0,   683,    27,
      28,    29,    31,    30,     0,   713,     0,     0,     0,     0,
     717,   400,   531,     0,   151,   123,   891,   700,     0,     0,
       4,   113,   115,   764,     0,   682,     0,     6,   186,     7,
       9,     8,    10,     0,     0,   392,   441,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   439,   874,   875,   513,
     511,   512,   422,   516,     0,   421,   846,   684,   691,     0,
     767,   510,   391,   849,   850,   861,   440,     0,     0,   443,
     442,   847,   848,   845,   881,   885,     0,   500,   766,    11,
     399,   398,   397,     0,     0,    31,     0,   113,   186,     0,
     955,   440,   954,     0,   952,   951,   515,     0,   429,   436,
     434,     0,     0,   482,   483,   484,   485,   509,   507,   506,
     505,   504,   503,   502,   501,   886,   708,   686,     0,     0,
       0,   975,   867,   684,     0,   685,   463,     0,   461,     0,
     915,     0,   774,   420,   695,   206,     0,   975,   419,   694,
     689,     0,   704,   685,   894,   895,   901,   893,   696,     0,
       0,   698,   508,     0,     0,     0,     0,   425,     0,   131,
     427,     0,     0,   137,   139,     0,     0,   141,     0,    72,
      71,    66,    65,    57,    58,    49,    69,    80,    81,     0,
      52,     0,    64,    56,    62,    83,    75,    74,    47,    70,
      90,    91,    48,    86,    45,    87,    46,    88,    44,    92,
      79,    84,    89,    76,    77,    51,    78,    82,    43,    73,
      59,    93,    67,    60,    50,    42,    41,    40,    39,    38,
      37,    61,    94,    97,    54,    35,    36,    63,  1011,  1012,
    1013,    55,  1018,    34,    53,    85,     0,     0,   113,    96,
     966,  1010,     0,  1014,     0,     0,   143,     0,     0,     0,
     177,     0,     0,     0,     0,     0,     0,   776,     0,   101,
     103,   305,     0,     0,   304,     0,   220,     0,   217,   310,
       0,     0,     0,     0,     0,   972,   202,   214,   907,   911,
     550,   296,     0,   936,     0,   719,     0,     0,     0,   934,
       0,    16,     0,   117,   194,   208,   215,   586,   543,     0,
     960,   523,   525,   527,   823,   428,   441,     0,     0,   439,
     440,   442,     0,     0,   701,     0,   702,     0,     0,     0,
     176,     0,     0,   119,   296,     0,    24,   185,     0,   213,
     198,   212,   397,   400,   186,   393,   166,   167,   168,   169,
     170,   172,   173,   175,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   899,     0,   165,   890,   890,   921,     0,     0,
       0,     0,     0,     0,     0,     0,   390,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     462,   460,   824,   825,     0,   890,     0,   837,   296,   296,
     890,     0,   892,   882,   907,     0,   186,     0,     0,   145,
       0,   821,   816,   774,     0,   441,   439,     0,   919,     0,
     548,   773,   910,   704,   441,   439,   440,   119,     0,   296,
     418,     0,   839,   697,     0,   123,   256,     0,   530,     0,
     148,     0,     0,   426,     0,     0,     0,     0,     0,   140,
     164,   142,  1011,  1012,  1013,  1008,  1009,     0,  1016,  1017,
    1001,     0,     0,     0,     0,    68,    95,    33,    55,    32,
     967,   171,   174,   144,   123,     0,   161,   163,     0,     0,
       0,     0,   104,     0,   775,   102,    18,     0,    98,     0,
     306,     0,   146,   219,   218,     0,     0,   147,   956,     0,
       0,   441,   439,   440,   443,   442,     0,   994,   226,     0,
     908,     0,     0,     0,     0,   774,     0,     0,     0,   776,
     297,   149,     0,     0,   718,   935,   765,     0,     0,   933,
     770,   932,   116,     5,    13,    14,     0,   224,     0,     0,
     536,     0,     0,     0,   774,     0,     0,   692,   687,   537,
       0,     0,     0,     0,   823,   123,     0,   776,   822,  1022,
     417,   431,   496,   855,   873,   128,   122,   124,   125,   126,
     127,   391,     0,   514,   768,   769,   114,   774,     0,   976,
       0,     0,   519,   188,   222,     0,   466,   468,   467,   479,
       0,     0,   499,   464,   465,   469,   471,   470,   487,   486,
     489,   488,   490,   492,   494,   493,   491,   481,   480,   473,
     474,   472,   475,   476,   478,   495,   477,   889,     0,     0,
     925,     0,   774,   959,     0,   958,   975,   852,   881,   204,
     196,   210,     0,   960,   200,   186,     0,   432,   435,   437,
     445,   459,   458,   457,   456,   455,   454,   453,   452,   451,
     450,   449,   448,   827,     0,   826,   829,   851,   833,   975,
     830,     0,     0,     0,     0,     0,     0,     0,     0,   953,
     430,   814,   818,   773,   820,     0,   688,     0,   914,     0,
     913,   222,     0,   688,   898,   897,     0,     0,   826,   829,
     896,   830,   423,   258,   260,   123,   534,   533,   424,     0,
     123,   240,   132,   427,     0,     0,     0,     0,     0,   252,
     252,   138,   774,     0,     0,     0,   999,   774,     0,   982,
       0,     0,     0,     0,     0,   772,     0,     0,   683,     0,
       0,   721,   682,   725,   727,     0,   720,   121,   726,   975,
    1015,     0,     0,     0,     0,    19,     0,    20,     0,    99,
       0,     0,     0,   110,   776,     0,   108,   103,   100,   105,
       0,   303,   311,   308,     0,     0,   945,   950,   947,   946,
     949,   948,    12,   992,   993,     0,   774,     0,     0,     0,
     907,   904,     0,   547,     0,   563,   773,   549,   298,   299,
     681,   775,   295,   944,   943,   942,     0,   938,     0,   939,
     941,     0,     5,     0,     0,     0,   580,   581,   589,   588,
       0,   439,     0,   773,   542,   546,     0,     0,   961,     0,
     524,     0,     0,   983,   823,   282,  1021,     0,     0,   838,
       0,   888,   773,   978,   974,   680,     0,   823,     0,     0,
     224,   521,   190,   498,     0,   568,   569,     0,   566,   773,
     920,     0,     0,   296,   226,     0,   224,     0,     0,   222,
       0,   899,   446,     0,     0,   835,   836,   853,   854,   883,
     884,     0,     0,     0,   802,   781,   782,   783,   790,     0,
       0,     0,     0,   794,   800,   792,   793,   808,   774,     0,
     816,   918,   917,     0,   224,     0,   840,   703,     0,   262,
       0,     0,   129,     0,     0,     0,     0,     0,     0,     0,
     232,   233,   244,     0,   123,   242,   158,   252,     0,   252,
       0,   773,     0,     0,     0,     0,   773,  1000,  1002,   981,
     774,   980,     0,   774,   748,   749,   746,   747,   780,     0,
     774,   772,   556,     0,   545,     0,     0,   927,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1005,   178,     0,   181,
     162,     0,     0,   106,   111,   112,   104,   775,   109,     0,
     307,     0,   957,   150,   973,   994,   987,   989,   225,   227,
     317,     0,     0,   905,     0,     0,   552,     0,     0,     0,
     300,   937,     0,    17,     0,   960,   223,   317,     0,     0,
     688,   539,     0,   693,   962,     0,   983,   528,     0,     0,
    1022,     0,   287,   285,   829,   841,   975,   829,   842,   977,
     120,     0,   823,   221,     0,   823,     0,   497,   924,   923,
       0,   296,     0,     0,     0,     0,     0,     0,   224,   192,
     704,   828,   296,     0,   786,   787,   788,   789,   795,   796,
     806,     0,   774,     0,   802,   560,     0,   785,   810,   773,
     813,   815,   817,     0,   912,     0,   828,     0,     0,     0,
       0,   259,   535,   134,     0,   427,   232,   234,   907,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   246,     0,
    1006,     0,   995,     0,   998,   773,     0,     0,     0,   723,
     773,   771,     0,     0,   774,     0,   762,     0,   774,     0,
     728,   763,   761,   931,     0,   774,   731,   733,   732,     0,
       0,   729,   730,   734,   736,   735,   751,   750,   753,   752,
     754,   756,   758,   757,   755,   744,   743,   738,   739,   737,
     740,   741,   742,   745,  1004,     0,   123,     0,     0,   107,
      21,   309,     0,     0,     0,   991,     0,   391,   909,   907,
     433,   438,   444,   554,     0,   302,   301,     0,    15,     0,
     391,   592,     0,     0,   594,   587,   590,     0,   585,     0,
     964,     0,   984,   532,     0,   288,     0,     0,   283,     0,
     983,     0,   317,     0,   823,     0,   296,     0,   879,   317,
     960,   317,   963,     0,     0,     0,   447,     0,     0,   798,
     773,   801,   791,     0,     0,   774,     0,   784,     0,     0,
     774,   807,   916,   317,     0,   123,     0,   255,   241,     0,
       0,     0,   231,   154,   245,     0,     0,   248,     0,   253,
     254,   123,   247,  1007,   996,     0,   979,     0,  1020,   779,
     778,   722,   564,   773,   555,     0,     0,   773,   544,   724,
       0,   567,   773,   926,   760,     0,     0,     0,    22,    23,
     988,   985,   986,   228,     0,     0,     0,   398,   389,     0,
       0,     0,   203,   316,   318,     0,   388,     0,     0,     0,
     960,   391,     0,     0,   551,   940,   313,   209,   583,     0,
       0,   538,   526,     0,   291,   281,     0,   284,   290,   296,
     518,   983,   391,   983,     0,   922,     0,   878,   391,     0,
     391,   965,   317,   823,   876,   805,   804,   797,   565,   773,
     559,     0,     0,   799,   773,   809,   391,   123,   261,   130,
     135,   156,   235,     0,   243,   249,   123,   251,   997,     0,
       0,     0,   558,   541,     0,   930,   929,   759,   123,   182,
     990,     0,     0,     0,   968,     0,     0,     0,   229,     0,
     960,     0,   354,   350,   356,   683,    31,     0,   344,     0,
     349,   353,   366,     0,   364,   369,     0,   368,     0,   367,
       0,   186,   320,     0,   322,     0,   323,   324,     0,     0,
     906,   553,     0,   584,   582,   593,   591,   292,     0,     0,
     279,   289,     0,     0,   983,     0,   199,   518,   983,   880,
     205,   313,   211,   391,     0,     0,     0,   562,   812,     0,
     207,   257,     0,     0,   123,   238,   155,   250,  1019,   777,
       0,     0,     0,     0,     0,     0,   416,     0,   969,     0,
     334,   338,   413,   414,   348,     0,     0,     0,   329,   645,
     644,   641,   643,   642,   662,   664,   663,   633,   603,   605,
     604,   623,   639,   638,   599,   610,   611,   613,   612,   632,
     616,   614,   615,   617,   618,   619,   620,   621,   622,   624,
     625,   626,   627,   628,   629,   631,   630,   600,   601,   602,
     606,   607,   609,   647,   648,   657,   656,   655,   654,   653,
     652,   640,   659,   649,   650,   651,   634,   635,   636,   637,
     660,   661,   679,   665,   667,   666,   668,   669,   646,   671,
     670,   673,   675,   674,   608,   678,   676,   677,   672,   658,
     598,   361,   595,     0,   330,   382,   383,   381,   374,     0,
     375,   331,   408,     0,     0,     0,     0,   412,     0,   186,
     195,   312,     0,     0,     0,   280,   294,   877,     0,     0,
     384,   123,   189,   983,     0,     0,   201,   983,   803,     0,
       0,   123,   236,   136,   157,     0,   557,   540,   928,   180,
     332,   333,   411,   230,     0,   774,   774,     0,   357,   345,
       0,     0,     0,   363,   365,     0,     0,   370,   377,   378,
     376,     0,     0,   319,   970,     0,     0,     0,   415,     0,
     314,     0,   293,     0,   578,   776,   123,     0,     0,   191,
     197,     0,   561,   811,     0,     0,   159,   335,   113,     0,
     336,   337,     0,   773,     0,   773,   359,   355,   360,   596,
     597,     0,   346,   379,   380,   372,   373,   371,   409,   406,
     994,   325,   321,   410,     0,   315,   579,   775,     0,     0,
     385,   123,   193,     0,   239,     0,   184,     0,   391,     0,
     351,   358,   362,     0,     0,   823,   327,     0,   576,   517,
     520,     0,   237,     0,     0,   160,   342,     0,   390,   352,
     407,   971,     0,   776,   402,   823,   577,   522,     0,   183,
       0,     0,   341,   983,   823,   266,   403,   404,   405,  1022,
     401,     0,     0,     0,   340,     0,   402,     0,   983,     0,
     339,   386,   123,   326,  1022,     0,   271,   269,     0,   123,
       0,     0,   272,     0,     0,   267,   328,     0,   387,     0,
     275,   265,     0,   268,   274,   179,   276,     0,     0,   263,
     273,     0,   264,   278,   277
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   110,   902,   633,   177,  1484,   726,
     346,   347,   348,   349,   854,   855,   856,   112,   113,   114,
     115,   116,   402,   666,   667,   542,   248,  1552,   548,  1463,
    1553,  1796,   843,   341,   574,  1756,  1080,  1266,  1815,   418,
     178,   668,   938,  1146,  1325,   120,   636,   955,   669,   684,
     959,   608,   954,   228,   523,   670,   637,   956,   420,   366,
     385,   123,   940,   905,   879,  1098,  1487,  1200,  1010,  1703,
    1556,   802,  1016,   547,   811,  1018,  1361,   794,   999,  1002,
    1189,  1822,  1823,   656,   657,   618,   619,   353,   354,   360,
    1522,  1681,  1682,  1277,  1403,  1510,  1675,  1805,  1825,  1714,
    1760,  1761,  1762,  1497,  1498,  1499,  1500,  1716,  1717,  1723,
    1772,  1503,  1504,  1508,  1668,  1669,  1670,  1692,  1853,  1404,
    1405,   179,   125,  1839,  1840,  1673,  1407,  1408,  1409,  1410,
     126,   241,   543,   544,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,  1534,   137,   937,  1145,   138,   653,
     654,   655,   245,   394,   538,   643,   644,  1228,   645,  1229,
     139,   140,   614,   615,  1223,  1224,  1334,  1335,   141,   833,
     984,   142,   834,   143,   144,  1745,   145,   638,  1524,   639,
    1118,   910,  1298,  1295,  1661,  1662,   146,   147,   148,   231,
     149,   232,   242,   405,   530,   150,  1038,   838,   151,  1039,
     933,   585,  1040,   985,  1168,   986,  1170,  1171,  1172,   988,
    1339,  1340,   989,   772,   513,   191,   192,   671,   659,   494,
    1134,  1135,   758,   759,   929,   153,   234,   154,   155,   181,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   718,
     166,   238,   239,   611,   221,   222,   721,   722,  1234,  1235,
     378,   379,   896,   167,   599,   168,   652,   169,   332,  1683,
    1735,   367,   413,   677,   678,  1032,  1129,  1275,   875,   876,
     877,   816,   817,   818,   333,   334,   840,   558,  1486,   927
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1539
static const yytype_int16 yypact[] =
{
   -1539,   190, -1539, -1539,  5498, 13814, 13814,    -8, 13814, 13814,
   13814, 11240, 13814, 13814, -1539, 13814, 13814, 13814, 13814, 13814,
   13814, 13814, 13814, 13814, 13814, 13814, 13814, 16597, 16597, 11438,
   13814, 17202,     5,    36, -1539, -1539, -1539, -1539, -1539,   192,
   -1539, -1539, -1539,   147, 13814, -1539,    36,   191,   193,   195,
   -1539,    36, 11636,   406, 11834, -1539, 14702, 10250,   197, 13814,
     950,    70, -1539, -1539, -1539,   213,    43,    76,   212,   217,
     232,   274, -1539,   406,   278,   280,    68, -1539, -1539, -1539,
   -1539, -1539, 13814,   520,   669, -1539, -1539,   406, -1539, -1539,
   -1539, -1539,   406, -1539,   406, -1539,   350,   326,   406,   406,
   -1539,   343, -1539, 12032, -1539, -1539,   327,   610,   721,   721,
   -1539,   516,   428,   589,   418, -1539,    85, -1539,   584, -1539,
   -1539, -1539, -1539,   704,   583, -1539, -1539,   435,   441,   446,
     461,   483,   485,   487,   490,  4992, -1539, -1539, -1539, -1539,
      81,   541, -1539,   567,   616, -1539,    60,   504, -1539,   582,
       4, -1539,  2303,   144, -1539, -1539,  2661,   140,   565,    99,
   -1539,   158,    63,   580,   115, -1539,   317, -1539,   647, -1539,
   -1539, -1539,   620,   590,   626, -1539, 13814, -1539,   584,   583,
   17679,  3872, 17679, 13814, 17679, 17679, 15220,   596,  4203, 15220,
   17679,   748,   406,   741,   741,   318,   741,   741,   741,   741,
     741,   741,   741,   741,   741, -1539, -1539, -1539,   639,    62,
   13814,   668, -1539, -1539,   667,   657,    52,   665,    52, 16597,
   15961,   661,   860, -1539,   620, -1539, 13814,   668, -1539,   710,
   -1539,   717,   707, -1539,   166, -1539, -1539, -1539,    52,   140,
   12230, -1539, -1539, 13814,  8864,   898,    94, 17679,  9854, -1539,
   13814, 13814,   406, -1539, -1539, 11620,   718, -1539, 12214, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,   576,
   -1539,   576, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,    88,    92,
      86,   626, -1539, -1539, -1539, -1539,   724,  3598,    95, -1539,
   -1539,   760,   908, -1539,   767, 15496, -1539,   719,   732, 13798,
   -1539,    46, 13996,  1887,  1887,   406,   742,   932,   750, -1539,
     225, -1539, 16201,    98, -1539,   816, -1539,   819, -1539,   944,
     101, 16597, 13814, 13814,   771,   783, -1539, -1539, 16300, 11438,
   13814, 10448,   103,   535,   259, -1539, 14012, 16597,   573, -1539,
     406, -1539,   423,   428, -1539, -1539, -1539, -1539, 17298,   960,
     881, -1539, -1539, -1539,    71, 13814,   792,   794, 17679,   811,
    1053,   812,  5696, 13814,    79,   814,   813,    79,   460,   312,
   -1539,   406,   576,   821, 10448, 14702, -1539, -1539,  1452, -1539,
   -1539, -1539, -1539, -1539,   584, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, 13814, 13814, 13814, 13814, 12428, 13814,
   13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814,
   13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814,
   13814, 13814, 17394, 13814, -1539, 13814, 13814, 13814,  4754,   406,
     406,   406,   406,   406,   704,   902,   768, 10052, 13814, 13814,
   13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814, 13814,
   -1539, -1539, -1539, -1539,  1190, 13814, 13814, -1539, 10448, 10448,
   13814, 13814,   327,   183, 16300,   823,   584, 12626, 15384, -1539,
   13814, -1539,   825,  1008,   865,   829,   831, 14210,    52, 12824,
   -1539, 13022, -1539,   707,   837,   846,  2313, -1539,   344, 10448,
   -1539,  1334, -1539, -1539, 15432, -1539, -1539, 10646, -1539, 13814,
   -1539,   945,  9062,  1032,   852, 17558,  1033,   123,    64, -1539,
   -1539, -1539,   871,   871, -1539, -1539, -1539,   576, -1539, -1539,
     627,   864,  1042, 16102,   406, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539,   867, -1539, -1539,   862,   866,
     869,   887,   402,  1925,  2081, -1539, -1539,   406,   406, 13814,
      52,    70, -1539, -1539, -1539, 16102,   977, -1539,    52,   128,
     130,   872,   889,  2498,   297,   890,   892,   503,   951,   895,
      52,   131,   896, 16179,   891,  1078, 16597, 13814,   903,  1085,
   17679, -1539,   694,   406, -1539, -1539,  1035,  2168,   466, -1539,
   -1539, -1539,   428, -1539, -1539, -1539,  1074,   975,   946,   368,
     967, 13814,   327,   992,  1122,   940,   980, -1539,   183, -1539,
     576,   576,  1134,   898,    71, -1539,   961,  1142, -1539,   576,
      54, -1539,   486,   159, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539,  1505,  3378, -1539, -1539, -1539, -1539,  1144,   976, -1539,
     963,  1152, -1539, -1539,  1024,  1601, 11819, 17817, 15220, 14529,
   13814, 17631, 14701, 12803, 13198, 13593,  3645, 12404, 15040, 15040,
   15040, 15040,  3422,  3422,  3422,  3422,  3422,  1313,  1313,   765,
     765,   765,   318,   318,   318, -1539,   741, 17679,   970,   973,
   16761,   979,  1173,    16, 13814,   178,   668,   233,   183, -1539,
   -1539, -1539,  1169,   881, -1539,   584, 16399, -1539, -1539, -1539,
   15220, 15220, 15220, 15220, 15220, 15220, 15220, 15220, 15220, 15220,
   15220, 15220, 15220, -1539, 13814,   453,   185, -1539, -1539,   668,
     532,   986,  3474,   993,   994,   991,  3568,   133,  1000, -1539,
   17679,  2094, -1539,   406, -1539,    54,   561, 16597, 17679, 16597,
   16809,  1024,    54,    52,   186,  1037,  1003, 13814, -1539,   194,
   -1539, -1539, -1539,  8666,   482, -1539, -1539, 17679, 17679,    36,
   -1539, -1539, -1539, 13814,  1095, 15983, 16102,   406,  9260,  1005,
    1009, -1539,  1192,  1113,  1069,  1050, -1539,  1198,  1018,  3914,
     576, 16102, 16102, 16102, 16102, 16102,  1020,  1153,  1056,  1031,
   16102,   412,  1068, -1539, -1539,  1034, -1539, 17773, -1539,    14,
   -1539,  5894,  1779,  1038,  2081, -1539,  2081, -1539,   406,   406,
    2081,  2081,   406, -1539,  1218,  1036, -1539,   431, -1539, -1539,
    3816, -1539, 17773,  1215, 16597,  1041, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539,  1058,  1230,   406,  1779,  1047,
   16300, 16498,  1228, -1539, 13220, -1539, 13814, -1539,    52, 17679,
   -1539, 10844, -1539, -1539, -1539, -1539,  1045, -1539, 13814, -1539,
   -1539,  5084, -1539,   576,  1779,  1052, -1539, -1539, -1539, -1539,
    1233,  1057, 13814, 17298, -1539, -1539,  4754,  1055, -1539,   576,
   -1539,  1062,  6092,  1220,   125, -1539, -1539,   116,  1190, -1539,
    1334, -1539,   576, -1539, -1539, -1539, 16102,   100,  1065,  1779,
     975, -1539, -1539, 14701, 13814, -1539, -1539, 13814, -1539, 13814,
   -1539,  3991,  1066, 10448,   951,  1221,   975,   576,  1244,  1024,
     406, 17394,    52,  4154,  1070, -1539, -1539,   179,  1072, -1539,
   -1539,  1250,  2325,  2325,  2094, -1539, -1539, -1539,  1213,  1075,
    1197,    77,  1079, -1539, -1539, -1539, -1539, -1539,  1260,  1080,
     825,    52,    52, 13418,   975,  1334, -1539, -1539,  4483,   489,
      36,  9854, -1539,  6290,  1083,  6488,  1084, 15983, 16597,  1087,
    1145,    52, 17773,  1268, -1539, -1539, -1539, -1539,   745, -1539,
     313,   576,  1102,  1149,   576,   406,   627, -1539, -1539, -1539,
    1277, -1539,  1097,  1144,   759,   759,  1227,  1227, 16968,  1089,
    1281, 16102, 16102, 15782, 17298,  2793, 15639, 16102, 16102, 16102,
   16102, 15884, 16102, 16102, 16102, 16102, 16102, 16102, 16102, 16102,
   16102, 16102, 16102, 16102, 16102, 16102, 16102, 16102, 16102, 16102,
   16102, 16102, 16102, 16102, 16102,   406, -1539, -1539,  1214, -1539,
   -1539,  1105,  1106, -1539, -1539, -1539,   444,  1925, -1539,  1112,
   -1539, 16102,    52, -1539, -1539,   161, -1539,   632,  1297, -1539,
   -1539,   136,  1116,    52, 11042, 16597, 17679, 16865, 16597, 13814,
   17679, -1539,  2068, -1539,  5300,   881,  1297, -1539,   579,    47,
   -1539, 17679,  1172,  1118, -1539,  1119,  1220, -1539,   576,   898,
     576,    67,  1299,  1232,   198, -1539,   668,   200, -1539, -1539,
   17773,  1124,   100, -1539,  1125,   100,  1127, 14701, 17679, 16913,
    1129, 10448,  1130,  1131,   576,  1135,  1128,   576,   975, -1539,
     707,   578, 10448, 13814, -1539, -1539, -1539, -1539, -1539, -1539,
    1188,  1132,  1319,  1240,  2094,  2094,  1179, -1539, 17298,  2094,
   -1539, -1539, -1539, 16597, 17679,  1139, -1539,    36,  1303,  1259,
    9854, -1539, -1539, -1539,  1146, 13814,  1145,    52, 16300, 15983,
    1150, 16102,  6686,   828,  1157, 13814,    90,   413, -1539,  1159,
   -1539,   576, -1539,  1202, -1539, 14087,  1311,  1160, 16102, -1539,
   16102, -1539,  1163,  1147,  1350, 17016, -1539,  1224,  1353,  1181,
   -1539, -1539, -1539, 17071,  1176,  1365, 10428, 10824, 11418, 16102,
   17727, 13001, 13396, 14869,  4404, 15487, 15632, 15632, 15632, 15632,
    2488,  2488,  2488,  2488,  2488,  1462,  1462,   759,   759,   759,
    1227,  1227,  1227,  1227, -1539,  1184, -1539,  1185,  1186, -1539,
   -1539, 17773,   406,   576,   576, -1539,  1779,   526, -1539, 16300,
   -1539, -1539, 15220,    52, 13616,    52, 17679,  1193, -1539,  1187,
     870, -1539,   171, 13814, -1539, -1539, -1539, 13814, -1539, 13814,
   -1539,   898, -1539, -1539,   141,  1366,  1300, 13814, -1539,  1194,
    1220,  1200, -1539,  1201,   100, 13814, 10448,  1203, -1539, -1539,
     881, -1539, -1539,  1196,  1205,  1199, -1539,  1208,  2094, -1539,
    2094, -1539, -1539,  1210,  1206,  1385,  1261, -1539,  1269,  1222,
    1398, -1539,    52, -1539,  1380, -1539,  1223, -1539, -1539,  1239,
    1246,   137, -1539, -1539, 17773,  1226,  1229, -1539, 11224, -1539,
   -1539, -1539, -1539, -1539, -1539,   576, -1539,   576, -1539, 17773,
   17119, -1539, -1539, 16102, -1539, 16102, 16102, 17298, -1539, -1539,
   16102, -1539, 16102, -1539, 12606, 16102,  1219,  6884, -1539, -1539,
     632, -1539, -1539, -1539,   628, 14874,  1779,  1308, -1539,  1763,
    1271,  1563, -1539, -1539, -1539,   902,  3098,   104,   105,  1248,
     881,   768,   139, 16597, 17679, -1539, -1539, -1539,  1264,  4646,
    4941, 17679, -1539,    72,  1435,  1367, 13814, -1539, 17679, 10448,
    1336,  1220,  1096,  1220,  1263, 17679,  1265, -1539,  1323,  1257,
    1596, -1539, -1539,   100, -1539, -1539,  1320, -1539, -1539,  2094,
   -1539,  2094,  2094, -1539, 17298, -1539,  1838, -1539,  8666, -1539,
   -1539, -1539, -1539,  9458, -1539, -1539, -1539,  8666, -1539,  1272,
   16102, 17174, 17773, 17773,  1322, 17773, 17222, 12606, -1539, -1539,
   -1539,  1779,  1779,   406, -1539,  1446, 15782,    83, -1539, 14874,
     881,  1906, -1539,  1286, -1539,   106,  1274,   109, -1539, 15219,
   -1539, -1539, -1539,   110, -1539, -1539,  1012, -1539,  1273, -1539,
    1386,   584, -1539, 15047, -1539, 15047, -1539, -1539,  1457,   902,
   -1539,    52, 14358, -1539, -1539, -1539, -1539,  1458,  1390, 13814,
   -1539, 17679,  1282,  1285,  1220,   609, -1539,  1336,  1220, -1539,
   -1539, -1539, -1539,  1853,  1288,  2094,  1341, -1539, -1539,  1348,
   -1539,  8666,  9656,  9458, -1539, -1539, -1539,  8666, -1539, 17773,
   16102, 16102, 16102,  7082,  1292,  1295, -1539, 16102, -1539,  1779,
   -1539, -1539, -1539, -1539, -1539,   576,  2294,  1763, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539,   150, -1539,  1271, -1539, -1539, -1539, -1539, -1539,   108,
     679, -1539,  1477,   112, 15496,  1386,  1478, -1539,   576,   584,
   -1539, -1539,  1301,  1479, 13814, -1539, 17679, -1539,   143,  1302,
   -1539, -1539, -1539,  1220,   609, 14530, -1539,  1220, -1539,  2094,
    2094, -1539, -1539, -1539, -1539,  7280, 17773, 17773, 17773, -1539,
   -1539, -1539, 17773, -1539,  1266,  1494,  1495,  1310, -1539, -1539,
   16102, 15219, 15219,  1447, -1539,  1012,  1012,   868, -1539, -1539,
   -1539, 16102,  1424, -1539,  1332,  1330,   117, 16102, -1539,   406,
   -1539, 16102, 17679,  1439, -1539,  1514, -1539,  7478,  1333, -1539,
   -1539,   609, -1539, -1539,  7676,  1342,  1411, -1539,  1432,  1381,
   -1539, -1539,  1434,   576,  1360,  2294, -1539, -1539, 17773, -1539,
   -1539,  1371, -1539,  1504, -1539, -1539, -1539, -1539, 17773,  1527,
     503, -1539, -1539, 17773,  1352, 17773, -1539,   295,  1355,  7874,
   -1539, -1539, -1539,  1351, -1539,  1357,  1375,   406,   768,  1372,
   -1539, -1539, -1539, 16102,  1373,   126, -1539,  1469, -1539, -1539,
   -1539,  8072, -1539,  1779,  1038, -1539,  1383,   406,   857, -1539,
   17773, -1539,  1368,  1548,   850,   126, -1539, -1539,  1475, -1539,
    1779,  1369, -1539,  1220,   127, -1539, -1539, -1539, -1539,   576,
   -1539,  1374,  1377,   120, -1539,   646,   850,   156,  1220,  1384,
   -1539, -1539, -1539, -1539,   576,    74,  1549,  1484,   646, -1539,
    8270,   160,  1567,  1500, 13814, -1539, -1539,  8468, -1539,   229,
    1570,  1502, 13814, -1539, 17679, -1539,  1575,  1507, 13814, -1539,
   17679, 13814, -1539, 17679, 17679
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1539, -1539, -1539,  -551, -1539, -1539, -1539,   124,   -11,   -34,
     534, -1539,  -268,  -510, -1539, -1539,   505,     3,  1537, -1539,
    2746, -1539,  -451, -1539,    19, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539,  -220, -1539, -1539,  -150,
      23,    25, -1539, -1539, -1539, -1539, -1539, -1539,    28, -1539,
   -1539, -1539, -1539, -1539, -1539,    29, -1539, -1539,  1121,  1133,
    1126,   -90,  -670,  -851,   642,   700,  -232,   415,  -919, -1539,
      56, -1539, -1539, -1539, -1539,  -724,   254, -1539, -1539, -1539,
   -1539,  -211, -1539,  -563, -1539,  -363, -1539, -1539,  1026, -1539,
      80, -1539, -1539, -1037, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539,    45, -1539,   142, -1539, -1539, -1539,
   -1539, -1539,   -32, -1539,   237,  -843, -1539, -1538,  -226, -1539,
    -143,   121,  -122,  -212, -1539,   -33, -1539, -1539, -1539,   244,
     -18,     7,    49,  -725,   -67, -1539, -1539,    26, -1539,     8,
   -1539, -1539,    -5,   -42,   111, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539,  -619,  -841, -1539, -1539, -1539, -1539,
   -1539,  1775, -1539, -1539, -1539, -1539, -1539, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539, -1539,   529, -1539, -1539, -1539,
   -1539, -1539, -1539, -1539, -1539,  -820, -1539,  2263,    35, -1539,
    1518,  -402, -1539, -1539,  -473,  3554,  3338, -1539, -1539,   608,
       9,  -593, -1539, -1539,   677,   478,  -608,   479, -1539, -1539,
   -1539, -1539, -1539,   662, -1539, -1539, -1539,    27,  -878,  -111,
    -423,  -415, -1539,   726,   -98, -1539, -1539,    37,    38,   598,
   -1539, -1539,  1666,    -4, -1539,  -353,    24,   231, -1539,   167,
   -1539, -1539, -1539,  -474,  1287, -1539, -1539, -1539, -1539, -1539,
     901,   756, -1539, -1539, -1539,  -347,  -652, -1539,  1242,  -988,
   -1539,   -56,  -179,    89,   839, -1539, -1070,   270,  -119, -1539,
     570,   636, -1539, -1539, -1539, -1539,   592,  -249,   -19, -1086
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1004
static const yytype_int16 yytable[] =
{
     180,   182,   425,   184,   185,   186,   188,   189,   190,   475,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   330,   117,   220,   223,   892,   118,   505,   119,
     767,   152,   121,   122,   920,   647,   397,   335,   386,   247,
     244,   649,   389,   390,  1304,   329,  1130,   255,   527,   258,
     781,   680,   339,   249,   342,   237,  1301,   425,   253,   497,
     715,   399,   474,   350,   925,   338,   230,   421,   235,   236,
     559,   756,  1122,   853,   858,   579,   581,   247,  1006,   757,
    1290,   958,   901,   337,   793,   396,  1020,   382,  1196,  1144,
     383,   921,  1569,   246,   415,   -33,   809,   -68,   398,   401,
     -33,   -95,   -68,   539,   -32,  1155,   -95,   591,   789,   -32,
     596,   994,   539,  1513,  1515,  -347,   790,  1725,  1577,  1663,
      14,  1732,  1359,   841,   495,   124,  1732,   531,   111,  1569,
    -856,   372,   807,  -858,   399,   763,   764,   864,   370,   539,
     881,   532,   881,  1185,  1726,   881,   881,   424,   881,    14,
    1305,  -570,   514,   351,  1131,  1527,  1749,  1862,   396,   359,
    -685,   575,   373,   987,  1720,   357,   786,  1176,   516,  -573,
    1075,   398,   401,   358,    14,    14,    14,   256,   508,  1423,
     328,  1743,  1721,   183,   525,   500,   492,   493,   492,   493,
       3,   412,   515,   -97,  1855,  -868,   240,   365,  1869,  1132,
    -529,  1722,   506,  1227,   922,   398,   401,   -97,   524,  1296,
     495,   873,   874,  1792,  -859,   375,   384,  -574,   365,  -692,
     576,   398,   365,   365,  1424,  1418,  1744,   243,  -857,  -863,
    1177,   522,  -686,   376,   377,   534,  -900,  1306,   534,  1856,
    1430,  1297,  1528,  1870,  1863,   247,   545,   365,   496,  -862,
     555,  -866,   556,   500,  -856,  -860,  -903,  -858,   810,   371,
    -286,  1088,   623,   536,  -902,   557,   352,   541,  -843,   557,
    -844,  -687,  -867,   476,   685,  1432,  1570,  1571,   416,   -33,
    1352,   -68,  1438,  1360,  1440,   -95,  1133,   540,   -32,  1158,
     499,   592,  -286,  1203,   597,  1207,   621,  1514,  1516,  -347,
    1727,   570,  1578,  1664,   559,  1733,  1456,  1324,   562,   501,
    1782,  1425,  1876,  1850,  -775,   808,   512,  -775,  -270,  -775,
     865,   602,   866,   882,   329,   971,  1857,   587,  1278,  1462,
    1871,  1520,   387,  1807,   496,  -865,   355,  1338,  -859,  -773,
    1084,  1085,   373,   356,  1003,   601,   350,   350,   582,  1005,
    -869,  1114,  -857,  -863,   425,   -96,   768,   247,   398,   605,
    -900,  1535,   683,  1537,   220,   613,   620,  -573,   111,   -96,
    -872,   627,   111,  -862,  1141,   462,   546,   501,  1808,  -860,
    -903,   330,   250,   632,   251,   588,   252,   463,  -902,  -693,
     188,   340,  -843,   679,  -844,   373,   335,   503,   672,  1877,
     205,    40,   629,   361,   329,  1543,  1101,  1485,   362,   620,
     737,   600,   624,   376,   377,   386,   732,   733,   421,  1205,
    1206,   658,   907,   363,   953,   118,   391,   205,    40,   686,
     687,   688,   689,   691,   692,   693,   694,   695,   696,   697,
     698,   699,   700,   701,   702,   703,   704,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   714,   725,   716,   569,
     717,   717,   720,  1289,  1689,   364,   376,   377,  1694,   368,
    1349,   369,   740,   741,   742,   743,   744,   745,   746,   747,
     748,   749,   750,   751,   752,   739,   237,    37,   499,   870,
     717,   762,   727,   620,   620,   717,   766,   230,   735,   235,
     236,  1572,   740,   738,   848,   770,   329,  1137,  1208,    50,
    1303,   387,   675,   502,   778,  1138,   780,   388,   760,  1205,
    1206,   403,   774,   124,   620,  1676,   111,  1677,   475,   908,
    1394,   392,   797,   587,   798,   676,  1474,   393,   812,   328,
     106,   727,   365,   373,   909,   796,   848,   952,   410,   928,
     629,   930,   785,   873,   874,   791,  -575,  1000,  1001,  1160,
     647,   801,   849,  1202,  1187,  1188,   649,  1336,  -975,    89,
      90,  1341,    91,   175,    93,    14,  1081,   853,  1082,  1311,
     964,   474,  1313,   411,   860,   960,   857,   857,   411,   412,
    1152,   411,   569,   365,   730,   365,   365,   365,   365,   911,
     492,   493,   156,   373,   411,   622,  -975,   269,  1362,   414,
     374,  -571,   889,  1549,   376,   377,   634,   635,   755,   648,
     492,   493,   417,  1748,   887,   216,   218,  1751,   426,   942,
    -831,   917,   918,   719,   427,   271,   398,   465,  1395,   428,
     926,   569,  -688,  1396,  -831,    62,    63,    64,   170,  1397,
     422,  1398,   527,   914,   429,   788,   373,    37,   813,   674,
    1076,   900,   761,   629,   928,   930,   111,   765,  1439,   492,
     493,   995,   930,   375,   376,   377,   430,  -870,   431,    50,
     432,   658,  1422,   433,   996,   943,   466,  1326,   839,  1399,
    1400,   647,  1401,   373,    55,   492,   493,   649,   467,   728,
     404,   400,    62,    63,    64,   170,   171,   422,    37,  -834,
    1728,   859,   676,   423,   552,   553,   554,  -572,   814,   951,
    1445,  1402,  1446,  -834,  1351,   728,   630,   376,   377,  1729,
      50,   950,  1730,   174,  1273,  1274,    87,   322,   468,    89,
      90,  1291,    91,   175,    93,  -975,   895,   897,   728,   963,
      37,  1434,  -870,  1847,  1292,  -832,   498,   326,  1518,   728,
    1481,  1482,   728,  1845,   376,   377,   412,   327,  1861,  -832,
     423,  -864,    50,  1293,   400,    37,  -686,   893,  1858,   894,
    -975,   504,   998,  -975,   174,    37,   380,    87,  1317,   509,
      89,    90,   476,    91,   175,    93,   511,    50,   247,  1327,
     679,   679,  1690,  1691,   373,  1412,  1004,    50,   400,   365,
     463,   407,  1071,  1072,  1073,  1387,   118,   518,   459,   460,
     461,  1022,   462,   517,   526,   647,  1027,  1015,  1074,   380,
     371,   649,    89,    90,   463,    91,   175,    93,  1573,  1851,
    1852,  1546,   156,  1547,  1548,   412,   156,   857,  -868,   857,
    1204,  1205,  1206,   857,   857,  1086,   499,    89,    90,   520,
      91,   175,    93,   381,   118,   419,  -684,    89,    90,   521,
      91,   175,    93,   528,  1394,   376,   377,   578,   580,  1106,
    1544,  1107,  1773,  1774,  1115,  1096,  1110,    62,    63,    64,
     170,   171,   422,  1112,  1458,   983,   373,   990,   529,  1775,
    1125,  1769,  1770,   629,  1159,   725,   537,  1121,  1030,  1033,
    1467,   550,   571,  1139,   124,   560, -1003,   111,  1776,    14,
     117,  1777,   563,   564,   118,   572,   119,  1824,   152,   121,
     122,  1013,   111,  1356,  1205,  1206,   583,  1698,  1156,  1147,
    1123,   584,  1148,   586,  1149,   118,   593,  1824,   620,   594,
     590,   658,   760,  1436,   791,   423,  1846,  1309,   595,   598,
     607,   603,   124,   606,   658,   111,   610,   376,   377,  1836,
    1837,  1838,  1083,   676,   650,   628,    62,    63,    64,   170,
     171,   422,  1395,   651,   660,   237,   661,  1396,  1184,    62,
      63,    64,   170,  1397,   422,  1398,   230,  1180,   235,   236,
     156,  1097,  1209,   662,   664,  1212,  1551,  1190,   406,   408,
     409,   673,  -118,    55,   682,  1557,   771,   773,   624,   791,
    1191,   775,   124,   776,   647,   111,   118,  1563,   118,   782,
     649,    37,  1226,  1399,  1400,  1232,  1401,  1280,   783,   799,
     569,   539,  1217,   124,   423,   803,   111,   806,   557,  1221,
    1832,   820,   755,    50,   788,   819,   844,   423,   842,   845,
     863,   343,   344,   846,   867,  1417,  1532,   507,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     847,   868,   871,   878,   365,   872,   880,   886,   883,   885,
     857,  1752,  1753,    37,   891,   890,  1167,  1167,   983,  1282,
    1394,   647,   610,  1705,  1286,   898,   903,   649,   904,  1302,
     345,   926,  1281,    89,    90,    50,    91,   175,    93,   788,
     490,   491,   906,  -708,   124,   111,   124,   111,   912,   111,
     625,   913,   915,   117,   631,  1320,   916,   118,  1323,   119,
     156,   152,   121,   122,   648,    14,   620,   728,   919,  1213,
     923,   924,  1788,   932,   934,   935,   939,   620,  1282,   728,
     625,   728,   631,   625,   631,   631,   936,   569,   945,   658,
     569,   946,   658,  1665,   948,    89,    90,  1666,    91,   175,
      93,  1331,   949,   957,   965,   967,   968,   492,   493,   969,
     247,   941,  1364,  -690,  1344,   997,  1139,  1007,  1017,   839,
    1358,  1021,  1019,  1506,  1023,  1024,  1025,  1026,  1395,  1347,
    1028,  1041,  1043,  1396,   888,    62,    63,    64,   170,  1397,
     422,  1398,  1044,  1042,  1046,   118,   728,  1087,  1047,  1091,
    1835,  1089,  1079,  1374,  1093,   124,  1094,  1378,   111,  1095,
    1747,  1100,  1104,  1111,  1383,   663,  1117,  1119,  1124,  1120,
    1754,  1126,  1128,  1154,  1391,  1392,  1142,  1151,  1157,  1399,
    1400,  1162,  1401,  -871,  1163,  1173,  1174,  1175,  1519,  1179,
    1178,    37,  1181,   205,    40,   648,  1193,  1195,  1198,  1414,
    1210,  1199,  1201,   423,   425,  1211,  1215,  1219,  1419,  1216,
    1220,  1536,  1420,    50,  1421,  1789,  1074,  1265,   983,   983,
    1267,  1268,  1428,   983,  1411,  1270,  1276,  1279,  1299,   953,
    1435,   620,  1300,  1307,   111,  1308,  1310,  1411,  1314,  1312,
    1316,  1322,  1318,   124,  1328,  1319,   111,  1394,  1330,  1321,
    1329,   978,  1337,  1343,   962,  1345,  1346,  1363,  1365,  1348,
    1811,   658,  1353,  1367,  1450,  1372,  1468,    37,  1469,  1455,
    1357,   753,  1368,    89,    90,  1371,    91,   175,    93,  1373,
    1376,  1674,  1377,   456,   457,   458,   459,   460,   461,    50,
     462,  1381,    14,  1379,  1382,   991,  1386,   992,  1388,  1389,
    1426,  1416,   463,  1427,   754,  1429,   106,  1512,  1415,  1441,
    1443,   156,  1431,  1433,  1449,  1437,  1390,  1451,  1406,  1442,
    1444,  1860,  1447,  1011,  1448,  1452,   156,  1454,  1867,   648,
     118,  1406,  1457,  1478,  1453,    37,  1459,   205,    40,  1464,
    1489,  1531,  1465,   174,   620,  1523,    87,  1566,  1511,    89,
      90,  1460,    91,   175,    93,  1395,  1502,    50,  1461,   156,
    1396,  1517,    62,    63,    64,   170,  1397,   422,  1398,  1529,
    1530,  1541,   983,  1533,   983,  1538,  1545,  1539,  1561,  1411,
    1567,  1757,  1092,  1575,  1558,  1411,  1671,  1411,  1576,  1672,
     658,  1678,  1684,  1685,  1687,  1568,  1688,  1699,   610,  1103,
    1697,   118,  1555,  1411,  1700,  1710,  1399,  1400,  1711,  1401,
     118,  1731,  1737,  1741,  1740,   753,  1746,    89,    90,   156,
      91,   175,    93,  1763,  1765,  1767,  1771,  1779,   124,  1780,
     423,   111,  1068,  1069,  1070,  1071,  1072,  1073,  1540,   328,
     156,  1781,  1786,  1787,  1686,  1507,  1795,  1791,   787,  1739,
     106,  1074,   476,    37,  -343,  1794,  1798,  1797,  1800,  1802,
    1726,  1803,  1679,  1806,  1812,   214,   214,  1809,  1813,  1814,
    1819,  1821,  1826,  1406,  1830,    50,  1715,  1834,  1842,  1406,
    1833,  1406,  1844,  1864,   211,   211,  1848,  1865,   227,  1849,
    1411,  1702,  1555,   983,   118,   983,   983,  1406,  1859,   124,
     118,  1872,   111,  1873,  1878,  1879,   118,   111,   124,  1881,
    1882,   111,  1269,   227,  1829,   734,  1153,   731,  1843,   156,
    1394,   156,   729,   156,  1116,  1011,  1197,   365,   648,  1704,
     569,  1350,  1466,   328,  1841,    89,    90,   861,    91,   175,
      93,  1695,  1719,  1660,    62,    63,    64,    65,    66,   422,
    1667,  1724,  1866,  1574,  1854,    72,   469,   328,  1509,   328,
    1734,  1490,  1736,   682,    37,    14,   328,  1294,  1693,  1222,
    1169,  1332,  1182,  1333,  1136,  1817,   612,   681,  1031,  1738,
    1480,  1804,  1214,   329,  1406,  1272,    50,  1264,     0,   983,
       0,     0,   124,     0,   471,   111,   111,   111,   124,  1742,
       0,   111,    37,  1784,   124,   648,     0,   111,     0,     0,
       0,  1505,   423,   217,   217,     0,   425,     0,     0,     0,
       0,     0,     0,  1283,    50,     0,  1285,     0,  1395,     0,
       0,     0,   156,  1396,     0,    62,    63,    64,   170,  1397,
     422,  1398,     0,     0,  1764,  1766,    89,    90,   118,    91,
     175,    93,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,  1799,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1506,     0,   211,     0,     0,  1399,
    1400,     0,  1401,     0,    89,    90,     0,    91,   175,    93,
     118,     0,     0,     0,     0,     0,     0,   118,     0,     0,
       0,  1342,     0,   423,     0,     0,     0,     0,   156,     0,
       0,  1542,   941,     0,  1491,     0,   610,  1011,   569,     0,
     156,     0,   212,   212,     0,     0,   227,     0,   227,     0,
       0,     0,   118,     0,     0,     0,     0,     0,     0,   328,
     926,     0,     0,   983,   983,     0,   124,     0,     0,   111,
       0,     0,   658,     0,   118,   926,     0,     0,  1758,     0,
       0,     0,  1394,     0,    37,  1660,  1660,     0,     0,  1667,
    1667,     0,   658,     0,     0,     0,     0,  1394,     0,  1874,
      37,   658,     0,   365,   227,     0,    50,  1880,   124,     0,
     214,   111,     0,  1883,     0,   124,  1884,   610,   111,   214,
       0,     0,    50,   118,     0,   217,   214,    14,  1492,   211,
     118,     0,     0,     0,     0,   214,     0,     0,   211,     0,
       0,  1493,    14,  1494,     0,   211,   646,     0,     0,     0,
     124,     0,     0,   111,   211,     0,     0,     0,     0,  1818,
     174,  1816,     0,    87,  1495,   227,    89,    90,     0,    91,
    1496,    93,   124,     0,     0,   111,   174,     0,     0,    87,
      88,  1831,    89,    90,     0,    91,   175,    93,     0,   227,
    1395,     0,   227,     0,     0,  1396,     0,    62,    63,    64,
     170,  1397,   422,  1398,     0,  1395,     0,     0,    37,     0,
    1396,     0,    62,    63,    64,   170,  1397,   422,  1398,     0,
       0,   124,     0,     0,   111,   156,     0,    37,   124,     0,
      50,   111,     0,     0,   212,     0,     0,     0,     0,   227,
       0,  1399,  1400,     0,  1401,     0,    37,     0,     0,    50,
       0,  1521,     0,     0,     0,     0,  1399,  1400,   217,  1401,
       0,     0,   214,     0,     0,   423,     0,   217,    50,   604,
       0,  1492,     0,  1550,   217,     0,   850,   851,     0,     0,
     423,   211,     0,   217,  1493,     0,  1494,   345,  1696,     0,
      89,    90,     0,    91,   175,    93,   156,     0,     0,     0,
       0,   156,     0,   174,     0,   156,    87,    88,     0,    89,
      90,     0,    91,  1496,    93,     0,     0,     0,   434,   435,
     436,     0,     0,     0,     0,   852,     0,     0,    89,    90,
       0,    91,   175,    93,   227,     0,     0,   227,   437,   438,
     831,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,   212,     0,     0,
       0,     0,   831,     0,   214,     0,   212,   463,     0,     0,
       0,     0,     0,   212,   972,   973,     0,     0,     0,   156,
     156,   156,   212,   211,     0,   156,     0,     0,     0,     0,
       0,   156,    37,     0,   974,     0,     0,     0,     0,     0,
     217,     0,   975,   976,   977,    37,     0,     0,   434,   435,
     436,     0,     0,     0,    50,   978,     0,   227,   227,     0,
       0,     0,     0,     0,     0,     0,   227,    50,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,     0,
       0,     0,   979,   980,     0,     0,     0,   463,     0,     0,
       0,   852,     0,     0,    89,    90,   981,    91,   175,    93,
       0,     0,     0,     0,   214,     0,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,  1287,     0,     0,     0,
       0,     0,     0,   211,     0,   982,     0,     0,     0,   212,
       0,     0,   217,     0,     0,     0,     0,     0,     0,     0,
     213,   213,     0,     0,   229,   214,     0,   214,     0,     0,
       0,     0,     0,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   211,     0,   211,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,   507,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
       0,     0,   211,   831,     0,   156,     0,     0,     0,     0,
       0,     0,   156,     0,     0,     0,   227,   227,   831,   831,
     831,   831,   831,   899,     0,     0,     0,   831,     0,     0,
       0,     0,    34,    35,    36,     0,     0,     0,     0,   227,
     490,   491,   214,     0,     0,   206,     0,   156,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,   214,   214,
       0,   211,   217,  1164,  1165,  1166,    37,     0,     0,   156,
       0,     0,     0,     0,     0,   227,     0,   211,   211,     0,
       0,     0,    62,    63,    64,    65,    66,   422,    50,     0,
       0,   646,     0,    72,   469,    77,    78,    79,    80,    81,
     227,   227,     0,   217,     0,   217,   209,   492,   493,     0,
     227,     0,    85,    86,     0,     0,   227,     0,   156,     0,
       0,     0,     0,     0,     0,   156,    95,     0,     0,   227,
     470,   217,   471,   831,     0,     0,   227,     0,     0,     0,
     100,     0,   213,     0,     0,   472,     0,   473,    89,    90,
     423,    91,   175,    93,   227,     0,     0,     0,   227,     0,
       0,     0,     0,     0,     0,   784,     0,     0,     0,     0,
       0,   212,   507,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   214,   214,     0,     0,     0,
     217, -1004, -1004, -1004, -1004, -1004,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,   211,   211,   217,   217,     0,     0,
       0,     0,   212,     0,   212,     0,     0,  1074,   227,     0,
       0,   227,   646,   227,     0,   490,   491,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   831,   831,
     212,   227,     0,     0,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,   214,   213,     0,   214,     0,   831,     0,
       0,   213,   492,   493,     0,     0,     0,     0,     0,   212,
     213,     0,   211,     0,     0,   211,     0,     0,     0,     0,
       0,   213,     0,     0,     0,   212,   212,     0,     0,     0,
       0,     0,     0,     0,     0,   227,     0,   227,     0,     0,
       0,     0,     0,   217,   217,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,     0,     0,
     869,   227,     0,     0,   227,     0,   646,     0,     0,     0,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   214,   214,     0,     0,
     211,     0,     0,     0,     0,   229,     0,     0,   490,   491,
       0,     0,     0,     0,     0,   211,   211,     0,   831,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   227,     0,
       0,     0,   227,     0,     0,   831,     0,   831,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,   217,     0,     0,   217,     0,   831,     0,     0,     0,
       0,     0,   212,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   492,   493,   214,     0,     0,
       0,     0,   331,  1048,  1049,  1050,     0,     0,     0,     0,
     227,   227,     0,   227,     0,     0,   211,     0,     0,     0,
       0,     0,     0,     0,  1051,     0,   835,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,   217,
       0,     0,     0,     0,     0,     0,     0,     0,   835,     0,
       0,     0,  1074,     0,   217,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
     212,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   646,     0,     0,     0,     0,
       0,     0,   227,     0,   227,     0,     0,     0,     0,     0,
     831,     0,   831,   831,   227,     0,     0,   831,     0,   831,
       0,     0,   831,     0,     0,     0,     0,     0,     0,     0,
       0,   214,   227,   227,     0,     0,   227,     0,     0,     0,
       0,     0,     0,   227,     0,   217,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   646,   212,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1230,     0,     0,     0,     0,
       0,   227,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,   831,     0,     0,
       0,     0,     0,     0,     0,   331,     0,   331,   227,   227,
       0,     0,     0,     0,     0,     0,   227,     0,   227,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,   213,     0,     0,     0,     0,     0,     0,     0,
     227,     0,   227,     0,   212,     0,     0,     0,     0,   227,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   835,
       0,     0,     0,   331,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,   835,   835,   835,   835,   835,     0,
       0,     0,     0,   835,     0,     0,     0,   831,   831,   831,
       0,     0,     0,     0,   831,  1078,   227,     0,     0,     0,
       0,     0,   227,     0,   227,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1099,     0,   213,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   271,   331,     0,
       0,   331,     0,     0,     0,     0,     0,  1099,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   835,
       0,    50,  1143,     0,     0,     0,     0,     0,     0,  -390,
       0,     0,     0,     0,     0,   227,     0,    62,    63,    64,
     170,   171,   422,     0,   229,     0,     0,     0,     0,     0,
       0,     0,   227,     0,     0,     0,   552,   553,   554,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   227,     0,     0,     0,   174,     0,   831,    87,   322,
       0,    89,    90,     0,    91,   175,    93,     0,   831,     0,
     213,   213,     0,     0,   831,     0,     0,     0,   831,   326,
       0,     0,     0,     0,     0,   423,     0,     0,     0,   327,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,     0,   331,   835,   835,   815,   213,     0,   832,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,     0,     0,
     831,   832,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,     0,     0,   835,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,   213,     0,
       0,   213,     0,     0,     0,     0,   227,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   434,   435,
     436,   227,     0,     0,     0,     0,   331,   331,     0,     0,
       0,     0,     0,     0,     0,   331,     0,     0,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,   213,   463,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,   213,     0,   835, -1004, -1004, -1004, -1004, -1004,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,   835,     0,   835,   434,   435,   436,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   835,     0,   437,   438,     0,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,     0,  1393,
       0,     0,   213,   463,     0,     0,     0,     0,     0,     0,
       0,     0,   832,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   331,   331,   832,   832,   832,
     832,   832,     0,   931,     0,     0,   832,     0,   434,   435,
     436,   215,   215,     0,     0,   233,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,   269,
       0,     0,     0,     0,     0,     0,   835,   463,   835,   835,
     213,     0,     0,   835,     0,   835,     0,     0,   835,   331,
       0,     0,     0,     0,     0,     0,     0,   271,     0,  1488,
       0,     0,  1501,     0,     0,   331,     0,     0,     0,   966,
       0,     0,     0,     0,     0,     0,   213,     0,   331,    37,
       0,     0,   832,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,    50,   462,   331,     0,     0,     0,     0,     0,   561,
       0,     0,     0,     0,   463,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   835,     0,     0,   552,   553,   554,     0,
       0,     0,     0,     0,  1564,  1565,     0,     0,     0,     0,
       0,     0,     0,     0,  1501,   174,     0,     0,    87,   322,
       0,    89,    90,   970,    91,   175,    93,   331,     0,     0,
     331,     0,   815,   215,     0,     0,     0,     0,     0,   326,
       0,     0,     0,     0,     0,     0,     0,   832,   832,   327,
       0,     0,     0,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,     0,     0,   835,   835,   835,   434,   435,   436,     0,
     835,     0,  1713,     0,     0,     0,     0,   832,     0,     0,
    1501,     0,     0,     0,     0,     0,   437,   438,     0,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,   331,     0,   331,     0,     0,     0,
       0,     0,     0,     0,     0,   463,   507,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,     0,
     331,   837,     0,   331,     0,     0,   215,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,   862,     0,     0,     0,     0,     0,   490,
     491,     0,   233,     0,     0,   269,     0,   832,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   331,     0,     0,
       0,   331,     0,     0,   832,     0,   832,     0,     0,     0,
       0,     0,     0,   271,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   835,     0,   832,     0,     0,     0,     0,
       0,     0,     0,     0,   835,    37,     0,     0,     0,     0,
     835,   434,   435,   436,   835,     0,   492,   493,     0,     0,
       0,  1090,     0,     0,     0,     0,   233,    50,     0,   331,
     331,   437,   438,     0,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,     0,
       0,     0,   552,   553,   554,     0,     0,     0,   215,     0,
     463,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,   174,     0,     0,    87,   322,  1828,    89,    90,     0,
      91,   175,    93,     0,  1029,     0,     0,     0,     0,     0,
       0,     0,     0,  1488,     0,   326,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,   331,     0,   331,     0,     0,     0,   836,     0,   832,
       0,   832,   832,     0,     0,     0,   832,     0,   832,     0,
       0,   832,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   331,     0,     0,  1012,     0,     0,     0,     0,   836,
       0,     0,   331,     0,     0,     0,     0,     0,     0,  1034,
    1035,  1036,  1037,     0,   434,   435,   436,     0,  1045,     0,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   437,   438,  1150,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,   434,   435,   436,   832,     0,     0,     0,
       0,     0,     0,   463,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   437,   438,   331,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   331,
     462,   331,     0,     0,     0,     0,     0,     0,   331,     0,
       0,     0,   463,     0,  1140,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   832,   832,   832,     0,
       0,     0,     0,   832,     0,     0,     0,     0,     0,     0,
       0,   331,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,   215,     0,     0,     0,     0,     0,   510,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1161,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
     836,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   836,   836,   836,   836,   836,
    1225,     0,     0,     0,   836,  1233,  1236,  1237,  1238,  1240,
    1241,  1242,  1243,  1244,  1245,  1246,  1247,  1248,  1249,  1250,
    1251,  1252,  1253,  1254,  1255,  1256,  1257,  1258,  1259,  1260,
    1261,  1262,  1263,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,   331,     0,     0,     0,     0,  1271,
       0,     0,     0,     0,   215,   215,     0,     0,     0,     0,
       0,   331,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1759,     0,     0,     0,     0,     0,   832,   233,     0,     0,
       0,     0,     0,  1074,     0,     0,     0,   832,     0,     0,
       0,     0,     0,   832,     0,     0,     0,   832,     0,     0,
     836,     0,     0,   434,   435,   436,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   331,
       0,     0,     0,   437,   438,   233,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,  1354,
     462,     0,     0,     0,     0,     0,     0,     0,     0,   832,
       0,     0,   463,     0,     0,     0,  1369,     0,  1370,     0,
       0,   215,   215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1384,     0,     0,
       0,     0,     0,     0,     0,   331,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   836,   836,     0,   233,     0,
     331,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   836,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   434,   435,   436,   215,
       0,     0,   215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   437,   438,  1186,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,     0,     0,     0,     0,     0,     0,
       0,  1471,     0,  1472,  1473,   463,     0,     0,  1475,     0,
    1476,     0,     0,  1477,     0,     0,     0,     0,     0,     0,
       0,     0,   233,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,   215,     0,   836,     0,   259,   260,     0,
     261,   262,     0,     0,   263,   264,   265,   266,     0,     0,
       0,     0,   836,     0,   836,     0,     0,     0,     0,     0,
       0,   267,     0,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   836,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1559,     0,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   272,   273,   274,   275,   276,
     277,   278,     0,   215,     0,    37,     0,   205,    40,     0,
       0,  1525,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,    50,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,     0,     0,     0,   723,   314,   315,   316,     0,     0,
       0,   317,   565,   566,   567,     0,     0,     0,  1706,  1707,
    1708,     0,     0,     0,     0,  1712,     0,     0,     0,     0,
       0,   568,     0,     0,     0,     0,     0,    89,    90,     0,
      91,   175,    93,   323,     0,   324,     0,   836,   325,   836,
     836,   233,     0,     0,   836,     0,   836,     0,     0,   836,
       0,     0,     0,     0,     0,     0,     0,     0,   724,     0,
     106,   434,   435,   436,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,   437,   438,     0,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,     0,
       0,     0,   434,   435,   436,     0,     0,     0,   233,     0,
     463,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   437,   438,   836,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,     0,     0,     0,     0,     0,     0,     0,  1768,     0,
       0,   463,     0,     0,     0,     0,     0,     0,     0,  1778,
       0,     0,     0,     0,     0,  1783,     0,     0,     0,  1785,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,   836,   836,   836,     0,     0,     0,
       0,   836,     0,     0,     0,     0,     0,     0,     0,     0,
    1718,     0,     0,    14,    15,    16,  1526,     0,     0,     0,
      17,  1820,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,   464,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,    88,     0,    89,    90,     0,
      91,    92,    93,    94,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,    98,     0,    99,     0,
     100,   101,   102,     0,   836,   103,     0,   104,   105,  1113,
     106,   107,     0,   108,   109,   836,     0,     0,     0,     0,
       0,   836,     0,     0,     0,   836,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,  1801,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,   836,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,     0,    89,    90,     0,    91,    92,    93,    94,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,    98,     0,    99,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,  1288,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,    82,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
       0,    89,    90,     0,    91,    92,    93,    94,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
      98,     0,    99,     0,   100,   101,   102,     0,     0,   103,
       0,   104,   105,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,    82,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,    94,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   103,     0,   104,
     105,   665,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,    94,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   103,     0,   104,   105,  1077,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,    82,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,    94,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,   104,   105,  1127,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,    94,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,  1192,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,  1194,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,    82,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,    94,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   103,
       0,   104,   105,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1355,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,    82,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,    94,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   103,     0,   104,
     105,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,    94,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   103,     0,   104,   105,  1479,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,    82,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,    94,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,   104,   105,  1709,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1755,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,    94,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,    82,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,    94,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   103,
       0,   104,   105,  1790,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  1793,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,    82,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,    94,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   103,     0,   104,
     105,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,    94,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   103,     0,   104,   105,  1810,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,    82,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,    94,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,   104,   105,  1827,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,    94,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,  1868,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,    82,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,    94,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   103,
       0,   104,   105,  1875,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,    82,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,    94,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   103,     0,   104,
     105,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   535,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   170,   171,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   103,     0,   104,   105,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,   800,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   170,   171,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,    82,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,   104,   105,     0,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1014,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   170,   171,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,     0,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1554,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     170,   171,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,    82,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   103,
       0,   104,   105,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1701,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   170,   171,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,    82,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,     0,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   103,     0,   104,
     105,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   170,   171,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,    82,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   103,     0,   104,   105,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   395,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     736,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   170,   171,   172,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   173,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,     0,     0,     0,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   170,   171,   172,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   173,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,     0,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,  1049,
    1050,   176,     0,   336,     0,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,  1051,
       0,    10,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,     0,     0,   616,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1074,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     170,   171,   172,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   173,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,   617,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   176,
       0,     0,     0,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   170,   171,
     172,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   173,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,     0,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,  1050,   176,     0,     0,
     795,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,  1051,     0,    10,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
       0,     0,  1108,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1074,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   170,   171,   172,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   173,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,  1109,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   176,     0,     0,     0,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   395,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   170,   171,   172,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   173,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,   434,   435,   436,     0,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   437,   438,  1359,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   463,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   187,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   170,   171,   172,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   173,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,     0,
       0,     0,    95,     0,     0,    96,     0,  1360,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   176,     0,     0,     0,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,  1051,
       0,    10,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,     0,     0,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1074,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     170,   171,   172,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   173,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   176,
     434,   435,   436,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     437,   438,     0,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   463,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   170,   171,
     172,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   173,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,     0,     0,     0,    95,     0,
       0,    96,     0,   549,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   176,     0,   254,
     435,   436,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,   463,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   170,   171,   172,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   173,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   176,     0,   257,     0,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   395,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   170,   171,   172,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   173,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,   434,   435,   436,     0,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   437,   438,     0,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   463,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   170,   171,   172,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   173,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,     0,
       0,     0,    95,     0,     0,    96,     0,   551,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   176,   533,     0,     0,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     690,   462,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   463,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     170,   171,   172,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   173,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   176,
       0,     0,     0,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,     0,     0,   736,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1074,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   170,   171,
     172,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   173,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,     0,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   176,     0,     0,
       0,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,     0,   777,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   463,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   170,   171,   172,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   173,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,     0,    96,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   176,     0,     0,     0,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,     0,     0,     0,
     779,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1074,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   170,   171,   172,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   173,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   176,     0,     0,     0,     0,   106,   107,
       0,   108,   109,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,  1105,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   170,   171,   172,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   173,    75,    76,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,     0,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   100,   101,   102,     0,
       0,   176,     0,     0,     0,     0,   106,   107,     0,   108,
     109,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,     0,     0,     0,     0,  1183,     0,     0,     0,
       0,     0,     0,     0,     0,  1074,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     170,   171,   172,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   173,    75,    76,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,     0,     0,
      95,     0,     0,    96,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,   100,   101,   102,     0,     0,   176,
       0,     0,     0,     0,   106,   107,     0,   108,   109,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,     0,     0,     0,  1413,     0,     0,     0,     0,     0,
       0,     0,   463,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   170,   171,
     172,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   173,    75,    76,     0,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,   174,    85,    86,    87,    88,     0,    89,
      90,     0,    91,   175,    93,     0,     0,     0,    95,     0,
       0,    96,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   100,   101,   102,     0,     0,   176,   434,   435,
     436,     0,   106,   107,     0,   108,   109,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   463,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   170,   171,   172,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   173,    75,    76,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,     0,    96,
       0,   573,     0,     0,     0,    97,     0,     0,     0,     0,
     100,   101,   102,     0,     0,   176,   434,   435,   436,     0,
     106,   107,     0,   108,   109,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   437,   438,     0,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   463,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   626,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,   269,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   170,   171,   172,     0,     0,    69,
      70,     0,     0,     0,     0,     0,   271,     0,     0,   173,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,    37,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,     0,    96,     0,   577,
      50,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   176,     0,     0,     0,     0,   106,   107,
       0,   108,   109,   259,   260,     0,   261,   262,     0,     0,
     263,   264,   265,   266,     0,   552,   553,   554,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   267,     0,   268,
       0,     0,     0,     0,   174,     0,     0,    87,   322,     0,
      89,    90,     0,    91,   175,    93,     0,  1366,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   327,     0,
       0,   272,   273,   274,   275,   276,   277,   278,     0,     0,
       0,    37,     0,   205,    40,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,    50,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     0,     0,     0,
     313,   314,   315,   316,     0,     0,     0,   317,   565,   566,
     567,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   259,   260,     0,   261,   262,     0,   568,   263,   264,
     265,   266,     0,    89,    90,     0,    91,   175,    93,   323,
       0,   324,     0,     0,   325,   267,     0,   268,     0,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   724,     0,   106,     0,     0,     0,
       0,     0,     0,     0,     0,   270,     0,   271,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   272,
     273,   274,   275,   276,   277,   278,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,    50,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,     0,     0,     0,     0,   314,
     315,   316,     0,     0,     0,   317,   318,   319,   320,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   321,     0,     0,    87,   322,
       0,    89,    90,     0,    91,   175,    93,   323,     0,   324,
       0,     0,   325,   259,   260,     0,   261,   262,     0,   326,
     263,   264,   265,   266,     0,     0,     0,     0,     0,   327,
       0,     0,     0,  1680,     0,     0,     0,   267,     0,   268,
     438,   269,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,   270,     0,   271,
       0,     0,     0,     0,     0,     0,     0,     0,   463,     0,
       0,   272,   273,   274,   275,   276,   277,   278,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,    50,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     0,     0,     0,
       0,   314,   315,   316,     0,     0,     0,   317,   318,   319,
     320,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   321,     0,     0,
      87,   322,     0,    89,    90,     0,    91,   175,    93,   323,
       0,   324,     0,     0,   325,   259,   260,     0,   261,   262,
       0,   326,   263,   264,   265,   266,     0,     0,     0,     0,
       0,   327,     0,     0,     0,  1750,     0,     0,     0,   267,
       0,   268,     0,   269,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,   270,
       0,   271,     0,     0,     0,     0,     0,     0,     0,     0,
     463,     0,     0,   272,   273,   274,   275,   276,   277,   278,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,    50,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,     0,
       0,     0,   313,   314,   315,   316,     0,     0,     0,   317,
     318,   319,   320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,    87,   322,     0,    89,    90,     0,    91,   175,
      93,   323,     0,   324,     0,     0,   325,   259,   260,     0,
     261,   262,     0,   326,   263,   264,   265,   266,     0,     0,
       0,     0,     0,   327,     0,     0,     0,     0,     0,     0,
       0,   267,     0,   268,     0,   269,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,     0,     0,     0,     0,     0,
       0,   270,     0,   271,     0,     0,     0,     0,  1074,     0,
       0,     0,     0,     0,     0,   272,   273,   274,   275,   276,
     277,   278,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,    50,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,     0,     0,     0,     0,   314,   315,   316,     0,     0,
       0,   317,   318,   319,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   321,     0,     0,    87,   322,     0,    89,    90,     0,
      91,   175,    93,   323,     0,   324,     0,     0,   325,     0,
     259,   260,     0,   261,   262,   326,  1483,   263,   264,   265,
     266,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,     0,     0,     0,   267,     0,   268,     0,   269, -1004,
   -1004, -1004, -1004,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,     0,     0,
       0,     0,     0,     0,   270,     0,   271,     0,     0,   463,
       0,     0,     0,     0,     0,     0,     0,     0,   272,   273,
     274,   275,   276,   277,   278,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
      50,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,     0,     0,     0,     0,   314,   315,
     316,     0,     0,     0,   317,   318,   319,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   321,     0,     0,    87,   322,     0,
      89,    90,     0,    91,   175,    93,   323,     0,   324,     0,
       0,   325,  1579,  1580,  1581,  1582,  1583,     0,   326,  1584,
    1585,  1586,  1587,     0,     0,     0,     0,     0,   327,     0,
       0,     0,     0,     0,     0,     0,  1588,  1589,  1590,     0,
     437,   438,     0,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,  1591,   462,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   463,
    1592,  1593,  1594,  1595,  1596,  1597,  1598,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1599,  1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,
    1608,  1609,    50,  1610,  1611,  1612,  1613,  1614,  1615,  1616,
    1617,  1618,  1619,  1620,  1621,  1622,  1623,  1624,  1625,  1626,
    1627,  1628,  1629,  1630,  1631,  1632,  1633,  1634,  1635,  1636,
    1637,  1638,  1639,     0,     0,     0,  1640,  1641,  1642,     0,
    1643,  1644,  1645,  1646,  1647,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1648,  1649,  1650,     0,
       0,     0,    89,    90,     0,    91,   175,    93,  1651,     0,
    1652,  1653,     0,  1654,   434,   435,   436,     0,     0,     0,
    1655,  1656,     0,  1657,     0,  1658,  1659,     0,     0,     0,
       0,     0,     0,     0,   437,   438,     0,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,   434,   435,   436,     0,     0,     0,     0,     0,
       0,     0,     0,   463,     0,     0,     0,     0,     0,     0,
       0,     0,   437,   438,     0,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   259,
     260,   463,   261,   262,     0,     0,   263,   264,   265,   266,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   267,     0,   268,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   270,     0,     0,  1074,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   272,   273,   274,
     275,   276,   277,   278,     0,     0,   769,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,    50,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,     0,   792,     0,   313,   314,   315,   316,
       0,     0,     0,   317,   565,   566,   567,     0,     0,     0,
       0,     0,   259,   260,     0,   261,   262,     0,     0,   263,
     264,   265,   266,   568,     0,     0,     0,     0,     0,    89,
      90,     0,    91,   175,    93,   323,   267,   324,   268,     0,
     325, -1004, -1004, -1004, -1004,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,     0,     0,
       0,     0,     0,     0,     0,     0,   270,     0,     0,     0,
       0,  1074,     0,     0,     0,     0,     0,     0,     0,     0,
     272,   273,   274,   275,   276,   277,   278,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,    50,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,     0,     0,     0,  1231,
     314,   315,   316,     0,     0,     0,   317,   565,   566,   567,
       0,     0,     0,     0,     0,   259,   260,     0,   261,   262,
       0,     0,   263,   264,   265,   266,   568,     0,     0,     0,
       0,     0,    89,    90,     0,    91,   175,    93,   323,   267,
     324,   268,     0,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   272,   273,   274,   275,   276,   277,   278,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,    50,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,     0,
       0,     0,     0,   314,   315,   316,  1239,     0,     0,   317,
     565,   566,   567,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   821,   822,     0,     0,     0,   568,
     823,     0,   824,     0,     0,    89,    90,     0,    91,   175,
      93,   323,     0,   324,   825,     0,   325,     0,     0,     0,
       0,     0,    34,    35,    36,    37,     0,     0,     0,     0,
       0,   434,   435,   436,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,   437,   438,     0,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,     0,
       0,  1008,   826,   827,     0,    77,    78,    79,    80,    81,
     463,     0,     0,     0,     0,     0,   209,     0,     0,     0,
       0,   174,    85,    86,    87,   828,     0,    89,    90,     0,
      91,   175,    93,    29,     0,     0,    95,     0,     0,     0,
       0,    34,    35,    36,    37,   829,   205,    40,     0,     0,
     100,     0,     0,     0,   206,   830,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   519,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1009,    75,   208,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
     174,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     175,    93,   821,   822,     0,    95,     0,     0,   823,     0,
     824,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,   825,     0,   210,     0,     0,     0,     0,   106,
      34,    35,    36,    37,     0,     0,     0,     0,     0,   434,
     435,   436,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,     0,     0,     0,
     826,   827,     0,    77,    78,    79,    80,    81,   463,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,   174,
      85,    86,    87,   828,     0,    89,    90,     0,    91,   175,
      93,    29,     0,     0,    95,     0,     0,     0,     0,    34,
      35,    36,    37,   829,   205,    40,     0,     0,   100,     0,
       0,     0,   206,   830,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   884,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     208,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,   209,     0,     0,     0,     0,   174,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   175,    93,
      29,     0,     0,    95,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   205,    40,     0,     0,   100,     0,     0,
       0,   206,   210,     0,     0,   589,     0,   106,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   609,    75,   208,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,   209,     0,     0,     0,     0,   174,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   175,    93,    29,
       0,   961,    95,     0,     0,     0,     0,    34,    35,    36,
      37,     0,   205,    40,     0,     0,   100,     0,     0,     0,
     206,   210,     0,     0,     0,     0,   106,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   208,     0,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,     0,   174,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   175,    93,    29,     0,
       0,    95,     0,     0,     0,     0,    34,    35,    36,    37,
       0,   205,    40,     0,     0,   100,     0,     0,     0,   206,
     210,     0,     0,     0,     0,   106,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1102,    75,   208,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,    29,     0,     0,
      95,     0,     0,     0,     0,    34,    35,    36,    37,     0,
     205,    40,     0,     0,   100,     0,     0,     0,   206,   210,
       0,     0,     0,     0,   106,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,   208,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,   209,
       0,     0,     0,     0,   174,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   175,    93,     0,     0,     0,    95,
       0,   434,   435,   436,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   100,     0,     0,     0,     0,   210,     0,
       0,   437,   438,   106,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,   434,
     435,   436,     0,     0,     0,     0,     0,     0,     0,     0,
     463,     0,     0,     0,     0,     0,     0,     0,     0,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,     0,     0,     0,
       0,     0,     0,     0,     0,   434,   435,   436,   463,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   437,   438,   947,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,     0,   462,   434,   435,   436,     0,     0,     0,     0,
       0,     0,     0,     0,   463,     0,     0,     0,     0,     0,
       0,     0,     0,   437,   438,   993,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,     0,     0,     0,     0,     0,     0,     0,  1048,  1049,
    1050,     0,   463,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1051,
       0,  1284,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,     0,     0,  1048,  1049,  1050,     0,
       0,     0,     0,     0,     0,     0,     0,  1074,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1051,     0,  1315,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1048,  1049,  1050,     0,  1074,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1051,     0,  1218,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,     0,     0,  1048,
    1049,  1050,     0,     0,     0,     0,     0,     0,     0,     0,
    1074,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1051,     0,  1375,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1048,  1049,  1050,     0,  1074,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1051,     0,  1380,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
       0,     0,  1048,  1049,  1050,     0,     0,     0,     0,     0,
       0,     0,     0,  1074,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1051,     0,  1470,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,     0,     0,
      34,    35,    36,    37,     0,   205,    40,     0,     0,     0,
       0,  1074,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
    1560,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,     0,   225,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,  1562,   174,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   175,
      93,     0,     0,     0,    95,     0,    34,    35,    36,    37,
       0,   205,    40,     0,     0,     0,     0,     0,   100,   640,
       0,     0,     0,   226,     0,     0,     0,     0,   106,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,   174,    85,    86,    87,    88,
       0,    89,    90,     0,    91,   175,    93,     0,     0,     0,
      95,     0,    34,    35,    36,    37,     0,   205,    40,     0,
       0,     0,     0,     0,   100,   206,     0,     0,     0,   641,
       0,     0,     0,     0,   642,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,   209,     0,     0,     0,
       0,   174,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   175,    93,     0,     0,     0,    95,     0,   434,   435,
     436,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     100,     0,     0,     0,     0,   226,   804,     0,   437,   438,
     106,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   434,   435,   436,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     805,   437,   438,   944,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,   434,
     435,   436,     0,     0,     0,     0,     0,     0,     0,     0,
     463,     0,     0,     0,     0,     0,     0,     0,     0,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,  1048,  1049,  1050,
       0,     0,     0,     0,     0,     0,     0,     0,   463,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1051,  1385,
       0,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1048,  1049,  1050,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1074,     0,     0,     0,
       0,     0,     0,     0,  1051,     0,     0,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,   436,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1074,     0,     0,     0,     0,   437,   438,     0,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,     0,   462,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463
};

static const yytype_int16 yycheck[] =
{
       5,     6,   124,     8,     9,    10,    11,    12,    13,   152,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,     4,    29,    30,   619,     4,   178,     4,
     504,     4,     4,     4,   653,   388,   103,    56,    94,    44,
      33,   388,    98,    99,  1130,    56,   924,    52,   227,    54,
     523,   414,    57,    46,    59,    31,  1126,   179,    51,   157,
     462,   103,   152,    60,   657,    57,    31,   123,    31,    31,
     319,   494,   913,   583,   584,   343,   344,    82,   803,   494,
    1117,   733,   633,    57,   535,   103,   810,    84,  1007,   940,
      87,   654,     9,    44,     9,     9,    32,     9,   103,   103,
      14,     9,    14,     9,     9,   956,    14,     9,   531,    14,
       9,   781,     9,     9,     9,     9,   531,     9,     9,     9,
      49,     9,    32,   574,    70,     4,     9,   238,     4,     9,
      70,    82,     9,    70,   176,   498,   499,     9,    70,     9,
       9,   239,     9,   994,    36,     9,     9,   124,     9,    49,
      83,    70,    90,    83,    38,    83,  1694,    83,   176,    83,
     156,   115,    83,   771,    14,   122,   529,    90,   210,    70,
     156,   176,   176,   130,    49,    49,    49,    53,   183,    38,
      56,    38,    32,   191,   226,    70,   134,   135,   134,   135,
       0,   177,   210,   177,    38,   191,   191,    73,    38,    83,
       8,    51,   179,  1044,   655,   210,   210,   191,   226,   162,
      70,    50,    51,  1751,    70,   153,    92,    70,    94,   156,
     174,   226,    98,    99,    83,    54,    83,   191,    70,    70,
     153,   222,   156,   154,   155,   240,    70,   170,   243,    83,
    1310,   194,   170,    83,   170,   250,   251,   123,   194,    70,
     269,   191,   271,    70,   194,    70,    70,   194,   194,   191,
     189,   854,   373,   244,    70,   177,   196,   248,    70,   177,
      70,   156,   191,   152,   424,  1312,   193,   194,   193,   193,
    1199,   193,  1319,   193,  1321,   193,   170,   193,   193,   959,
     191,   193,   192,  1017,   193,  1019,   193,   193,   193,   193,
     192,   335,   193,   193,   553,   193,  1343,  1158,   327,   194,
     193,   170,    83,   193,   189,   192,   192,   192,   192,   192,
     192,   363,   192,   192,   335,   192,   170,   102,   192,   192,
     170,   192,   161,    38,   194,   191,   123,  1178,   194,   178,
     850,   851,    83,   130,   795,   363,   343,   344,   345,   800,
     191,   902,   194,   194,   476,   177,   506,   362,   363,   363,
     194,  1431,   418,  1433,   369,   370,   371,    70,   244,   191,
     191,   376,   248,   194,   937,    57,   252,   194,    83,   194,
     194,   415,   191,   380,   191,   160,   191,    69,   194,   156,
     395,   194,   194,   412,   194,    83,   415,   166,   403,   170,
      83,    84,    90,   191,   415,  1442,   880,  1395,   191,   414,
     477,   362,   153,   154,   155,   471,   472,   473,   474,   106,
     107,   394,    54,   191,   191,   402,    83,    83,    84,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   468,   463,   335,
     465,   466,   467,  1115,  1534,   191,   154,   155,  1538,   191,
    1195,   191,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   477,   462,    81,   191,   192,
     495,   496,   468,   498,   499,   500,   501,   462,   475,   462,
     462,  1489,   507,   477,   102,   510,   517,   930,   195,   103,
    1129,   161,   200,   196,   519,   930,   521,   191,   494,   106,
     107,   194,   513,   402,   529,  1513,   402,  1515,   671,   161,
       4,   188,   537,   102,   539,   411,  1377,   194,   557,   415,
     196,   517,   418,    83,   176,   537,   102,   726,    32,   660,
      90,   662,   528,    50,    51,   531,    70,    75,    76,   961,
     913,   542,   160,  1014,    75,    76,   913,  1175,   156,   163,
     164,  1179,   166,   167,   168,    49,   844,  1087,   846,  1142,
     759,   671,  1145,   160,   589,   735,   583,   584,   160,   177,
     953,   160,   468,   469,   470,   471,   472,   473,   474,   641,
     134,   135,     4,    83,   160,    70,   194,    31,   195,   191,
      90,    70,   617,  1454,   154,   155,   193,   194,   494,   388,
     134,   135,    38,  1693,   615,    27,    28,  1697,   193,   685,
     177,   650,   651,   466,   193,    59,   641,    70,   112,   193,
     659,   517,   156,   117,   191,   119,   120,   121,   122,   123,
     124,   125,   831,   644,   193,   531,    83,    81,    31,   199,
     839,   195,   495,    90,   775,   776,   542,   500,  1320,   134,
     135,   782,   783,   153,   154,   155,   193,   191,   193,   103,
     193,   654,  1301,   193,   782,   690,    70,  1160,   564,   163,
     164,  1044,   166,    83,   111,   134,   135,  1044,   194,   468,
      90,   103,   119,   120,   121,   122,   123,   124,    81,   177,
      31,   587,   588,   187,   138,   139,   140,    70,    91,   724,
    1328,   195,  1330,   191,  1198,   494,   153,   154,   155,    50,
     103,   722,    53,   157,   102,   103,   160,   161,   156,   163,
     164,   162,   166,   167,   168,   156,   622,   623,   517,   754,
      81,  1314,   191,  1839,   175,   177,   191,   181,  1410,   528,
     132,   133,   531,  1833,   154,   155,   177,   191,  1854,   191,
     187,   191,   103,   194,   176,    81,   156,    83,  1848,    85,
     191,   191,   787,   194,   157,    81,   160,   160,  1151,   193,
     163,   164,   671,   166,   167,   168,    48,   103,   803,  1162,
     819,   820,   193,   194,    83,  1279,   799,   103,   210,   685,
      69,    90,    53,    54,    55,  1266,   793,   219,    53,    54,
      55,   812,    57,   156,   226,  1178,   817,   808,    69,   160,
     191,  1178,   163,   164,    69,   166,   167,   168,  1490,   193,
     194,  1449,   244,  1451,  1452,   177,   248,   844,   191,   846,
     105,   106,   107,   850,   851,   852,   191,   163,   164,   198,
     166,   167,   168,   194,   841,   161,   156,   163,   164,     9,
     166,   167,   168,   156,     4,   154,   155,   343,   344,   884,
    1443,   886,  1725,  1726,   903,   876,   891,   119,   120,   121,
     122,   123,   124,   898,  1345,   771,    83,   773,   191,    31,
     919,  1721,  1722,    90,   960,   916,     8,   912,   819,   820,
    1361,   193,   193,   932,   793,   191,   156,   793,    50,    49,
     901,    53,    14,   156,   901,   193,   901,  1805,   901,   901,
     901,   807,   808,   105,   106,   107,   194,  1545,   957,   944,
     916,     9,   947,   193,   949,   922,   130,  1825,   953,   130,
     352,   924,   928,  1316,   930,   187,  1834,  1136,    14,   361,
     177,   363,   841,   192,   937,   841,   368,   154,   155,   119,
     120,   121,   848,   849,    14,   377,   119,   120,   121,   122,
     123,   124,   112,   102,   192,   961,   192,   117,   993,   119,
     120,   121,   122,   123,   124,   125,   961,   988,   961,   961,
     402,   877,  1021,   192,   192,  1024,  1457,  1000,   107,   108,
     109,   197,   191,   111,   191,  1466,   191,     9,   153,   995,
    1001,   192,   901,   192,  1377,   901,  1003,  1478,  1005,   192,
    1377,    81,  1043,   163,   164,  1046,   166,  1104,   192,    94,
     916,     9,  1033,   922,   187,   193,   922,    14,   177,  1040,
     193,     9,   928,   103,   930,   191,   194,   187,   191,   193,
      83,   111,   112,   194,   192,   195,  1429,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     193,   192,   192,   132,   960,   193,   191,     9,   192,   198,
    1087,  1699,  1700,    81,     9,   192,   972,   973,   974,  1104,
       4,  1454,   504,  1554,  1109,    70,    32,  1454,   133,  1128,
     160,  1130,  1104,   163,   164,   103,   166,   167,   168,   995,
      67,    68,   176,   156,  1003,  1001,  1005,  1003,   136,  1005,
     374,     9,   192,  1114,   378,  1154,   156,  1114,  1157,  1114,
     542,  1114,  1114,  1114,   913,    49,  1151,   916,    14,  1025,
     189,     9,  1745,     9,   178,   192,   132,  1162,  1163,   928,
     404,   930,   406,   407,   408,   409,    14,  1043,   198,  1142,
    1046,   198,  1145,   161,   195,   163,   164,   165,   166,   167,
     168,  1172,     9,    14,   198,   192,   192,   134,   135,   198,
    1195,   191,  1211,   156,  1187,   192,  1215,   102,   193,  1075,
    1205,     9,   193,   191,    91,   136,   156,     9,   112,  1190,
     192,   191,   156,   117,   616,   119,   120,   121,   122,   123,
     124,   125,   191,    70,   156,  1202,   995,     9,   194,    14,
    1823,   195,   194,  1224,   193,  1114,   178,  1228,  1114,     9,
    1691,   194,    14,   198,  1235,   192,   194,    14,   193,   192,
    1701,   189,    32,    32,  1273,  1274,   191,   191,    14,   163,
     164,   191,   166,   191,    14,    52,   191,    70,  1411,     9,
     191,    81,   192,    83,    84,  1044,   193,   193,   191,  1284,
     178,   136,    14,   187,  1406,   136,     9,   198,  1293,   192,
       9,   195,  1297,   103,  1299,  1746,    69,    83,  1174,  1175,
     195,   195,  1307,  1179,  1277,   193,     9,   191,   136,   191,
    1315,  1316,   193,    14,  1190,    83,   192,  1290,   191,   194,
     191,   193,   192,  1202,   136,   194,  1202,     4,     9,   194,
     198,    91,   153,   194,   736,    32,    77,   178,   136,   193,
    1791,  1314,   192,    32,  1335,   198,  1365,    81,  1367,  1340,
     193,   161,   192,   163,   164,   192,   166,   167,   168,     9,
     136,  1511,     9,    50,    51,    52,    53,    54,    55,   103,
      57,   195,    49,   192,     9,   777,   192,   779,   193,   193,
      14,   194,    69,    83,   194,   191,   196,  1406,   195,   193,
     191,   793,   192,   192,     9,   192,  1272,   136,  1277,   194,
     192,  1852,   192,   805,   198,   136,   808,     9,  1859,  1178,
    1387,  1290,    32,   194,   192,    81,   193,    83,    84,   193,
     112,  1426,   193,   157,  1429,   161,   160,  1483,  1405,   163,
     164,   192,   166,   167,   168,   112,   165,   103,   192,   841,
     117,   193,   119,   120,   121,   122,   123,   124,   125,    14,
      83,   194,  1328,   117,  1330,   192,   136,   192,   136,  1432,
      14,   195,   864,   177,   192,  1438,   193,  1440,   194,    83,
    1443,    14,    14,    83,   192,  1486,   191,   136,   880,   881,
     192,  1458,  1463,  1456,   136,   193,   163,   164,   193,   166,
    1467,    14,    14,    14,   193,   161,   194,   163,   164,   901,
     166,   167,   168,     9,     9,   195,    59,    83,  1387,   177,
     187,  1387,    50,    51,    52,    53,    54,    55,   195,  1395,
     922,   191,    83,     9,  1529,  1401,   115,   194,   194,  1679,
     196,    69,  1411,    81,   102,   193,   102,   156,   178,   168,
      36,    14,  1519,   191,   193,    27,    28,   192,   191,   174,
     178,   178,    83,  1432,   171,   103,  1575,     9,    83,  1438,
     192,  1440,   193,    14,    27,    28,   192,    83,    31,   192,
    1543,  1552,  1553,  1449,  1551,  1451,  1452,  1456,   194,  1458,
    1557,    14,  1458,    83,    14,    83,  1563,  1463,  1467,    14,
      83,  1467,  1087,    56,  1814,   474,   954,   471,  1830,  1001,
       4,  1003,   469,  1005,   904,  1007,  1008,  1483,  1377,  1553,
    1486,  1196,  1358,  1489,  1825,   163,   164,   591,   166,   167,
     168,  1541,  1577,  1499,   119,   120,   121,   122,   123,   124,
    1506,  1663,  1858,  1491,  1846,   130,   131,  1513,  1401,  1515,
    1674,  1397,  1675,   191,    81,    49,  1522,  1118,  1537,  1041,
     973,  1173,   990,  1174,   928,  1798,   369,   415,   819,  1678,
    1390,  1780,  1026,  1674,  1543,  1095,   103,  1075,    -1,  1545,
      -1,    -1,  1551,    -1,   169,  1551,  1552,  1553,  1557,  1684,
      -1,  1557,    81,  1739,  1563,  1454,    -1,  1563,    -1,    -1,
      -1,   128,   187,    27,    28,    -1,  1818,    -1,    -1,    -1,
      -1,    -1,    -1,  1105,   103,    -1,  1108,    -1,   112,    -1,
      -1,    -1,  1114,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,  1715,  1716,   163,   164,  1705,   166,
     167,   168,    -1,    -1,    -1,    -1,    -1,   219,    -1,    -1,
      -1,    -1,    -1,    -1,  1763,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   191,    -1,   219,    -1,    -1,   163,
     164,    -1,   166,    -1,   163,   164,    -1,   166,   167,   168,
    1747,    -1,    -1,    -1,    -1,    -1,    -1,  1754,    -1,    -1,
      -1,  1183,    -1,   187,    -1,    -1,    -1,    -1,  1190,    -1,
      -1,   195,   191,    -1,    31,    -1,  1198,  1199,  1674,    -1,
    1202,    -1,    27,    28,    -1,    -1,   269,    -1,   271,    -1,
      -1,    -1,  1789,    -1,    -1,    -1,    -1,    -1,    -1,  1695,
    1839,    -1,    -1,  1699,  1700,    -1,  1705,    -1,    -1,  1705,
      -1,    -1,  1805,    -1,  1811,  1854,    -1,    -1,  1714,    -1,
      -1,    -1,     4,    -1,    81,  1721,  1722,    -1,    -1,  1725,
    1726,    -1,  1825,    -1,    -1,    -1,    -1,     4,    -1,  1864,
      81,  1834,    -1,  1739,   327,    -1,   103,  1872,  1747,    -1,
     352,  1747,    -1,  1878,    -1,  1754,  1881,  1279,  1754,   361,
      -1,    -1,   103,  1860,    -1,   219,   368,    49,   125,   352,
    1867,    -1,    -1,    -1,    -1,   377,    -1,    -1,   361,    -1,
      -1,   138,    49,   140,    -1,   368,   388,    -1,    -1,    -1,
    1789,    -1,    -1,  1789,   377,    -1,    -1,    -1,    -1,  1798,
     157,  1797,    -1,   160,   161,   388,   163,   164,    -1,   166,
     167,   168,  1811,    -1,    -1,  1811,   157,    -1,    -1,   160,
     161,  1817,   163,   164,    -1,   166,   167,   168,    -1,   412,
     112,    -1,   415,    -1,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   112,    -1,    -1,    81,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,  1860,    -1,    -1,  1860,  1387,    -1,    81,  1867,    -1,
     103,  1867,    -1,    -1,   219,    -1,    -1,    -1,    -1,   462,
      -1,   163,   164,    -1,   166,    -1,    81,    -1,    -1,   103,
      -1,  1413,    -1,    -1,    -1,    -1,   163,   164,   352,   166,
      -1,    -1,   504,    -1,    -1,   187,    -1,   361,   103,   363,
      -1,   125,    -1,   195,   368,    -1,   111,   112,    -1,    -1,
     187,   504,    -1,   377,   138,    -1,   140,   160,   195,    -1,
     163,   164,    -1,   166,   167,   168,  1458,    -1,    -1,    -1,
      -1,  1463,    -1,   157,    -1,  1467,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,   160,    -1,    -1,   163,   164,
      -1,   166,   167,   168,   557,    -1,    -1,   560,    30,    31,
     563,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,   352,    -1,    -1,
      -1,    -1,   595,    -1,   616,    -1,   361,    69,    -1,    -1,
      -1,    -1,    -1,   368,    50,    51,    -1,    -1,    -1,  1551,
    1552,  1553,   377,   616,    -1,  1557,    -1,    -1,    -1,    -1,
      -1,  1563,    81,    -1,    70,    -1,    -1,    -1,    -1,    -1,
     504,    -1,    78,    79,    80,    81,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,   103,    91,    -1,   650,   651,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   659,   103,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,    -1,    -1,    -1,    69,    -1,    -1,
      -1,   160,    -1,    -1,   163,   164,   152,   166,   167,   168,
      -1,    -1,    -1,    -1,   736,    -1,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,   736,    -1,   181,    -1,    -1,    -1,   504,
      -1,    -1,   616,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    31,   777,    -1,   779,    -1,    -1,
      -1,    -1,    -1,  1705,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   777,    -1,   779,    -1,    -1,    -1,
      -1,    -1,    -1,   805,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   805,   806,    -1,  1747,    -1,    -1,    -1,    -1,
      -1,    -1,  1754,    -1,    -1,    -1,   819,   820,   821,   822,
     823,   824,   825,   195,    -1,    -1,    -1,   830,    -1,    -1,
      -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,   842,
      67,    68,   864,    -1,    -1,    91,    -1,  1789,    -1,    -1,
      -1,   616,    -1,    -1,    -1,    -1,    -1,    -1,   880,   881,
      -1,   864,   736,    78,    79,    80,    81,    -1,    -1,  1811,
      -1,    -1,    -1,    -1,    -1,   878,    -1,   880,   881,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,   103,    -1,
      -1,   913,    -1,   130,   131,   141,   142,   143,   144,   145,
     903,   904,    -1,   777,    -1,   779,   152,   134,   135,    -1,
     913,    -1,   158,   159,    -1,    -1,   919,    -1,  1860,    -1,
      -1,    -1,    -1,    -1,    -1,  1867,   172,    -1,    -1,   932,
     167,   805,   169,   936,    -1,    -1,   939,    -1,    -1,    -1,
     186,    -1,   219,    -1,    -1,   182,    -1,   184,   163,   164,
     187,   166,   167,   168,   957,    -1,    -1,    -1,   961,    -1,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
      -1,   736,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1007,  1008,    -1,    -1,    -1,
     864,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1007,  1008,   880,   881,    -1,    -1,
      -1,    -1,   777,    -1,   779,    -1,    -1,    69,  1021,    -1,
      -1,  1024,  1044,  1026,    -1,    67,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,  1042,
     805,  1044,    -1,    -1,  1047,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,    -1,    -1,    -1,   352,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1105,   361,    -1,  1108,    -1,  1091,    -1,
      -1,   368,   134,   135,    -1,    -1,    -1,    -1,    -1,   864,
     377,    -1,  1105,    -1,    -1,  1108,    -1,    -1,    -1,    -1,
      -1,   388,    -1,    -1,    -1,   880,   881,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1128,    -1,  1130,    -1,    -1,
      -1,    -1,    -1,  1007,  1008,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
     192,  1154,    -1,    -1,  1157,    -1,  1178,    -1,    -1,    -1,
      -1,  1183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1178,  1198,  1199,    -1,    -1,
    1183,    -1,    -1,    -1,    -1,   462,    -1,    -1,    67,    68,
      -1,    -1,    -1,    -1,    -1,  1198,  1199,    -1,  1201,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1211,    -1,
      -1,    -1,  1215,    -1,    -1,  1218,    -1,  1220,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   504,    -1,    -1,
      -1,  1105,    -1,    -1,  1108,    -1,  1239,    -1,    -1,    -1,
      -1,    -1,  1007,  1008,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,  1279,    -1,    -1,
      -1,    -1,    56,    10,    11,    12,    -1,    -1,    -1,    -1,
    1273,  1274,    -1,  1276,    -1,    -1,  1279,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   563,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   595,    -1,
      -1,    -1,    69,    -1,  1198,  1199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   616,
    1105,    -1,    -1,  1108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1377,    -1,    -1,    -1,    -1,
      -1,    -1,  1365,    -1,  1367,    -1,    -1,    -1,    -1,    -1,
    1373,    -1,  1375,  1376,  1377,    -1,    -1,  1380,    -1,  1382,
      -1,    -1,  1385,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1413,  1395,  1396,    -1,    -1,  1399,    -1,    -1,    -1,
      -1,    -1,    -1,  1406,    -1,  1279,    -1,    -1,    -1,    -1,
    1413,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1183,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1454,  1198,  1199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
      -1,  1454,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   736,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1470,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   269,    -1,   271,  1481,  1482,
      -1,    -1,    -1,    -1,    -1,    -1,  1489,    -1,  1491,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     777,    -1,   779,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1513,    -1,  1515,    -1,  1279,    -1,    -1,    -1,    -1,  1522,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   805,   806,
      -1,    -1,    -1,   327,    -1,    -1,    -1,    -1,    -1,  1413,
      -1,    -1,    -1,    -1,   821,   822,   823,   824,   825,    -1,
      -1,    -1,    -1,   830,    -1,    -1,    -1,  1560,  1561,  1562,
      -1,    -1,    -1,    -1,  1567,   842,  1569,    -1,    -1,    -1,
      -1,    -1,  1575,    -1,  1577,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   864,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   878,    -1,   880,   881,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,   412,    -1,
      -1,   415,    -1,    -1,    -1,    -1,    -1,   904,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   913,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1413,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,
      -1,   103,   939,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,  1678,    -1,   119,   120,   121,
     122,   123,   124,    -1,   961,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1695,    -1,    -1,    -1,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1714,    -1,    -1,    -1,   157,    -1,  1720,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,  1731,    -1,
    1007,  1008,    -1,    -1,  1737,    -1,    -1,    -1,  1741,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1763,    -1,    -1,   557,  1041,  1042,   560,  1044,    -1,   563,
    1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,    -1,    -1,
    1803,   595,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1813,    -1,    -1,    -1,  1091,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1830,  1105,    -1,
      -1,  1108,    -1,    -1,    -1,    -1,  1839,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,  1854,    -1,    -1,    -1,    -1,   650,   651,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   659,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,  1178,    -1,    -1,    -1,    -1,  1183,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1198,  1199,    -1,  1201,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,  1218,    -1,  1220,    10,    11,    12,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1239,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1276,
      -1,    -1,  1279,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   806,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   819,   820,   821,   822,   823,
     824,   825,    -1,   195,    -1,    -1,   830,    -1,    10,    11,
      12,    27,    28,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,  1373,    69,  1375,  1376,
    1377,    -1,    -1,  1380,    -1,  1382,    -1,    -1,  1385,   903,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,  1396,
      -1,    -1,  1399,    -1,    -1,   919,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,  1413,    -1,   932,    81,
      -1,    -1,   936,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   103,    57,   957,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    69,    -1,    -1,  1454,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1470,    -1,    -1,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,  1481,  1482,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1491,   157,    -1,    -1,   160,   161,
      -1,   163,   164,   195,   166,   167,   168,  1021,    -1,    -1,
    1024,    -1,  1026,   219,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,  1042,   191,
      -1,    -1,    -1,  1047,  1048,  1049,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,    -1,    -1,  1560,  1561,  1562,    10,    11,    12,    -1,
    1567,    -1,  1569,    -1,    -1,    -1,    -1,  1091,    -1,    -1,
    1577,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,  1128,    -1,  1130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
    1154,   563,    -1,  1157,    -1,    -1,   352,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,
      -1,    -1,   368,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   377,    -1,   595,    -1,    -1,    -1,    -1,    -1,    67,
      68,    -1,   388,    -1,    -1,    31,    -1,  1201,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1211,    -1,    -1,
      -1,  1215,    -1,    -1,  1218,    -1,  1220,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1720,    -1,  1239,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1731,    81,    -1,    -1,    -1,    -1,
    1737,    10,    11,    12,  1741,    -1,   134,   135,    -1,    -1,
      -1,   195,    -1,    -1,    -1,    -1,   462,   103,    -1,  1273,
    1274,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,   138,   139,   140,    -1,    -1,    -1,   504,    -1,
      69,    -1,    -1,    -1,    -1,    -1,  1803,    -1,    -1,    -1,
      -1,   157,    -1,    -1,   160,   161,  1813,   163,   164,    -1,
     166,   167,   168,    -1,   170,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1830,    -1,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,
      -1,  1365,    -1,  1367,    -1,    -1,    -1,   563,    -1,  1373,
      -1,  1375,  1376,    -1,    -1,    -1,  1380,    -1,  1382,    -1,
      -1,  1385,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1395,    -1,    -1,   806,    -1,    -1,    -1,    -1,   595,
      -1,    -1,  1406,    -1,    -1,    -1,    -1,    -1,    -1,   821,
     822,   823,   824,    -1,    10,    11,    12,    -1,   830,    -1,
     616,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   195,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    10,    11,    12,  1470,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,  1489,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1513,
      57,  1515,    -1,    -1,    -1,    -1,    -1,    -1,  1522,    -1,
      -1,    -1,    69,    -1,   936,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     736,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1560,  1561,  1562,    -1,
      -1,    -1,    -1,  1567,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   777,    -1,   779,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   805,
     806,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   821,   822,   823,   824,   825,
    1042,    -1,    -1,    -1,   830,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,    -1,    -1,    -1,    -1,    -1,   864,    -1,
      -1,    -1,    -1,    -1,  1678,    -1,    -1,    -1,    -1,  1091,
      -1,    -1,    -1,    -1,   880,   881,    -1,    -1,    -1,    -1,
      -1,  1695,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1714,    -1,    -1,    -1,    -1,    -1,  1720,   913,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,  1731,    -1,    -1,
      -1,    -1,    -1,  1737,    -1,    -1,    -1,  1741,    -1,    -1,
     936,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1763,
      -1,    -1,    -1,    30,    31,   961,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1201,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1803,
      -1,    -1,    69,    -1,    -1,    -1,  1218,    -1,  1220,    -1,
      -1,  1007,  1008,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1239,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1839,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1041,  1042,    -1,  1044,    -1,
    1854,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1091,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,  1105,
      -1,    -1,  1108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   195,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1373,    -1,  1375,  1376,    69,    -1,    -1,  1380,    -1,
    1382,    -1,    -1,  1385,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1178,    -1,    -1,    -1,    -1,  1183,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1198,  1199,    -1,  1201,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,  1218,    -1,  1220,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1239,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1470,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,  1279,    -1,    81,    -1,    83,    84,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,    -1,    -1,  1560,  1561,
    1562,    -1,    -1,    -1,    -1,  1567,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,    -1,  1373,   174,  1375,
    1376,  1377,    -1,    -1,  1380,    -1,  1382,    -1,    -1,  1385,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,    -1,
     196,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1413,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,  1454,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,  1470,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1720,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1731,
      -1,    -1,    -1,    -1,    -1,  1737,    -1,    -1,    -1,  1741,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,  1560,  1561,  1562,    -1,    -1,    -1,
      -1,  1567,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1576,    -1,    -1,    49,    50,    51,   195,    -1,    -1,    -1,
      56,  1803,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,   193,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,    -1,
     186,   187,   188,    -1,  1720,   191,    -1,   193,   194,   195,
     196,   197,    -1,   199,   200,  1731,    -1,    -1,    -1,    -1,
      -1,  1737,    -1,    -1,    -1,  1741,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,  1765,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,  1803,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,   195,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,   193,   194,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
     194,   195,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,   194,   195,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,   193,   194,   195,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,   195,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,   193,   194,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,   194,   195,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,   193,   194,   195,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    99,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,   193,   194,   195,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,   194,   195,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,   193,   194,   195,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,   195,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,   193,   194,   195,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,   194,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,   193,   194,    -1,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,   193,   194,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
     194,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,   194,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    11,
      12,   191,    -1,   193,    -1,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    31,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,   170,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    12,   191,    -1,    -1,
     194,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,   170,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,    -1,    -1,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    10,    11,    12,    -1,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    -1,   175,    -1,   193,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,    -1,    -1,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    31,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      10,    11,    12,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,
      -1,   175,    -1,   193,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,
      11,    12,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    69,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,   193,    -1,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    10,    11,    12,    -1,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    -1,   175,    -1,   193,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,   192,    -1,    -1,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      32,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,    -1,
      -1,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    -1,    -1,    -1,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,   197,
      -1,   199,   200,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,    -1,    -1,    -1,   196,   197,    -1,   199,
     200,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,
      -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,   187,   188,    -1,    -1,   191,    10,    11,
      12,    -1,   196,   197,    -1,   199,   200,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,   193,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,
     186,   187,   188,    -1,    -1,   191,    10,    11,    12,    -1,
     196,   197,    -1,   199,   200,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    31,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    81,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   193,
     103,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,   197,
      -1,   199,   200,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      -1,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   157,    10,    11,
      12,    13,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,   171,    -1,    -1,   174,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   194,    -1,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,   171,
      -1,    -1,   174,     3,     4,    -1,     6,     7,    -1,   181,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    27,    -1,    29,
      31,    31,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,   171,    -1,    -1,   174,     3,     4,    -1,     6,     7,
      -1,   181,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   191,    -1,    -1,    -1,   195,    -1,    -1,    -1,    27,
      -1,    29,    -1,    31,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,   171,    -1,    -1,   174,     3,     4,    -1,
       6,     7,    -1,   181,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    31,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,    -1,    -1,   174,    -1,
       3,     4,    -1,     6,     7,   181,   182,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    31,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,   169,    -1,   171,    -1,
      -1,   174,     3,     4,     5,     6,     7,    -1,   181,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,   164,    -1,   166,   167,   168,   169,    -1,
     171,   172,    -1,   174,    10,    11,    12,    -1,    -1,    -1,
     181,   182,    -1,   184,    -1,   186,   187,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    69,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,   192,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   192,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   157,    -1,    -1,    -1,    -1,    -1,   163,
     164,    -1,   166,   167,   168,   169,    27,   171,    29,    -1,
     174,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   157,    -1,    -1,    -1,
      -1,    -1,   163,   164,    -1,   166,   167,   168,   169,    27,
     171,    29,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,   131,   132,   133,    32,    -1,    -1,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,   157,
      56,    -1,    58,    -1,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,   171,    70,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    38,   138,   139,    -1,   141,   142,   143,   144,   145,
      69,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    70,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,   181,    83,    84,    -1,    -1,
     186,    -1,    -1,    -1,    91,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    50,    51,    -1,   172,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    70,    -1,   191,    -1,    -1,    -1,    -1,   196,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
     138,   139,    -1,   141,   142,   143,   144,   145,    69,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    70,    -1,    -1,   172,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,   181,    83,    84,    -1,    -1,   186,    -1,
      -1,    -1,    91,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      70,    -1,    -1,   172,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   186,    -1,    -1,
      -1,    91,   191,    -1,    -1,   194,    -1,   196,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    70,
      -1,    72,   172,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   186,    -1,    -1,    -1,
      91,   191,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    70,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   186,    -1,    -1,    -1,    91,
     191,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    70,    -1,    -1,
     172,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   186,    -1,    -1,    -1,    91,   191,
      -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    30,    31,   196,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   136,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    69,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,   136,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   186,    91,
      -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,
     172,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,   186,    91,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,   196,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    -1,    -1,    -1,   172,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,    28,    -1,    30,    31,
     196,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   202,   203,     0,   204,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   141,   142,   143,
     144,   145,   149,   152,   157,   158,   159,   160,   161,   163,
     164,   166,   167,   168,   169,   172,   175,   181,   182,   184,
     186,   187,   188,   191,   193,   194,   196,   197,   199,   200,
     205,   208,   218,   219,   220,   221,   222,   225,   241,   242,
     246,   249,   256,   262,   322,   323,   331,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   346,   349,   361,
     362,   369,   372,   374,   375,   377,   387,   388,   389,   391,
     396,   399,   418,   426,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   441,   454,   456,   458,
     122,   123,   124,   137,   157,   167,   191,   208,   241,   322,
     343,   430,   343,   191,   343,   343,   343,   108,   343,   343,
     343,   416,   417,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,    83,    91,   124,   139,   152,
     191,   219,   362,   388,   391,   396,   430,   433,   430,    38,
     343,   445,   446,   343,   124,   130,   191,   219,   254,   388,
     389,   390,   392,   396,   427,   428,   429,   437,   442,   443,
     191,   332,   393,   191,   332,   353,   333,   343,   227,   332,
     191,   191,   191,   332,   193,   343,   208,   193,   343,     3,
       4,     6,     7,    10,    11,    12,    13,    27,    29,    31,
      57,    59,    71,    72,    73,    74,    75,    76,    77,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   130,   131,   132,   133,   137,   138,   139,
     140,   157,   161,   169,   171,   174,   181,   191,   208,   209,
     210,   221,   459,   475,   476,   479,   193,   338,   340,   343,
     194,   234,   343,   111,   112,   160,   211,   212,   213,   214,
     218,    83,   196,   288,   289,   123,   130,   122,   130,    83,
     290,   191,   191,   191,   191,   208,   260,   462,   191,   191,
      70,   191,   333,    83,    90,   153,   154,   155,   451,   452,
     160,   194,   218,   218,   208,   261,   462,   161,   191,   462,
     462,    83,   188,   194,   354,    27,   331,   335,   343,   344,
     430,   434,   223,   194,    90,   394,   451,    90,   451,   451,
      32,   160,   177,   463,   191,     9,   193,    38,   240,   161,
     259,   462,   124,   187,   241,   323,   193,   193,   193,   193,
     193,   193,   193,   193,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   193,    70,    70,   194,   156,   131,
     167,   169,   182,   184,   262,   321,   322,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      67,    68,   134,   135,   420,    70,   194,   425,   191,   191,
      70,   194,   196,   438,   191,   240,   241,    14,   343,   193,
     136,    48,   208,   415,    90,   331,   344,   156,   430,   136,
     198,     9,   401,   255,   331,   344,   430,   463,   156,   191,
     395,   420,   425,   192,   343,    32,   225,     8,   355,     9,
     193,   225,   226,   333,   334,   343,   208,   274,   229,   193,
     193,   193,   138,   139,   140,   479,   479,   177,   478,   478,
     191,   111,   479,    14,   156,   138,   139,   140,   157,   208,
     210,   193,   193,   193,   235,   115,   174,   193,   211,   213,
     211,   213,   218,   194,     9,   402,   193,   102,   160,   194,
     430,     9,   193,   130,   130,    14,     9,   193,   430,   455,
     333,   331,   344,   430,   433,   434,   192,   177,   252,   137,
     430,   444,   445,   343,   363,   364,    38,   170,   286,   287,
     343,   193,    70,   420,   153,   452,    82,   343,   430,    90,
     153,   452,   218,   207,   193,   194,   247,   257,   378,   380,
      91,   191,   196,   356,   357,   359,   391,   436,   438,   456,
      14,   102,   457,   350,   351,   352,   284,   285,   418,   419,
     192,   192,   192,   192,   192,   195,   224,   225,   242,   249,
     256,   418,   343,   197,   199,   200,   208,   464,   465,   479,
     286,   459,   191,   462,   250,   240,   343,   343,   343,   343,
      32,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   392,   343,   343,   440,   440,
     343,   447,   448,   130,   194,   209,   210,   437,   438,   260,
     208,   261,   462,   462,   259,   241,    38,   335,   338,   340,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   161,   194,   208,   421,   422,   423,   424,
     437,   440,   343,   286,   286,   440,   343,   444,   240,   192,
     343,   191,   414,     9,   401,   192,   192,    38,   343,    38,
     343,   395,   192,   192,   192,   437,   286,   194,   208,   421,
     422,   437,   192,   223,   278,   194,   340,   343,   343,    94,
      32,   225,   272,   193,    28,   102,    14,     9,   192,    32,
     194,   275,   479,    31,    91,   221,   472,   473,   474,   191,
       9,    50,    51,    56,    58,    70,   138,   139,   161,   181,
     191,   219,   221,   370,   373,   388,   396,   397,   398,   208,
     477,   223,   191,   233,   194,   193,   194,   193,   102,   160,
     111,   112,   160,   214,   215,   216,   217,   218,   214,   208,
     343,   289,   397,    83,     9,   192,   192,   192,   192,   192,
     192,   192,   193,    50,    51,   469,   470,   471,   132,   265,
     191,     9,   192,   192,   136,   198,     9,   401,   430,   343,
     192,     9,   402,    83,    85,   208,   453,   208,    70,   195,
     195,   204,   206,    32,   133,   264,   176,    54,   161,   176,
     382,   344,   136,     9,   401,   192,   156,   479,   479,    14,
     355,   284,   223,   189,     9,   402,   479,   480,   420,   425,
     420,   195,     9,   401,   178,   192,    14,   347,   243,   132,
     263,   191,   462,   343,    32,   198,   198,   136,   195,     9,
     401,   343,   463,   191,   253,   248,   258,    14,   457,   251,
     240,    72,   430,   343,   463,   198,   195,   192,   192,   198,
     195,   192,    50,    51,    70,    78,    79,    80,    91,   138,
     139,   152,   181,   208,   371,   404,   406,   407,   410,   413,
     208,   430,   430,   136,   263,   420,   425,   192,   343,   279,
      75,    76,   280,   223,   332,   223,   334,   102,    38,   137,
     269,   430,   397,   208,    32,   225,   273,   193,   276,   193,
     276,     9,   401,    91,   136,   156,     9,   401,   192,   170,
     464,   465,   466,   464,   397,   397,   397,   397,   397,   400,
     403,   191,    70,   156,   191,   397,   156,   194,    10,    11,
      12,    31,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    69,   156,   463,   195,   388,   194,
     237,   213,   213,   208,   214,   214,   218,     9,   402,   195,
     195,    14,   430,   193,   178,     9,   401,   208,   266,   388,
     194,   444,   137,   430,    14,    38,   343,   343,    38,   170,
     343,   198,   343,   195,   204,   479,   266,   194,   381,    14,
     192,   343,   356,   437,   193,   479,   189,   195,    32,   467,
     419,    38,    83,   170,   421,   422,   424,   421,   422,   479,
     397,   284,   191,   388,   264,   348,   244,   343,   343,   343,
     195,   191,   286,   265,    32,   264,   479,    14,   263,   462,
     392,   195,   191,    14,    78,    79,    80,   208,   405,   405,
     407,   408,   409,    52,   191,    70,    90,   153,   191,     9,
     401,   192,   414,    38,   343,   264,   195,    75,    76,   281,
     332,   225,   195,   193,    95,   193,   269,   430,   191,   136,
     268,    14,   223,   276,   105,   106,   107,   276,   195,   479,
     178,   136,   479,   208,   472,     9,   192,   401,   136,   198,
       9,   401,   400,   365,   366,   397,   209,   356,   358,   360,
     192,   130,   209,   397,   449,   450,   397,   397,   397,    32,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   477,    83,   238,   195,   195,   217,
     193,   397,   471,   102,   103,   468,     9,   294,   192,   191,
     335,   340,   343,   430,   136,   430,   343,   198,   195,   457,
     294,   162,   175,   194,   377,   384,   162,   194,   383,   136,
     193,   467,   479,   355,   480,    83,   170,    14,    83,   463,
     192,   284,   194,   284,   191,   136,   191,   286,   192,   194,
     479,   194,   193,   479,   264,   245,   395,   286,   136,   198,
       9,   401,   406,   408,   367,   368,   407,   153,   356,   411,
     412,   407,   430,   194,   332,    32,    77,   225,   193,   334,
     268,   444,   269,   192,   397,   101,   105,   193,   343,    32,
     193,   277,   195,   178,   479,   136,   170,    32,   192,   397,
     397,   192,   198,     9,   401,   136,   136,     9,   401,   192,
     136,   195,     9,   401,   397,    32,   192,   223,   193,   193,
     208,   479,   479,   388,     4,   112,   117,   123,   125,   163,
     164,   166,   195,   295,   320,   321,   322,   327,   328,   329,
     330,   418,   444,    38,   343,   195,   194,   195,    54,   343,
     343,   343,   355,    38,    83,   170,    14,    83,   343,   191,
     467,   192,   294,   192,   284,   343,   286,   192,   294,   457,
     294,   193,   194,   191,   192,   407,   407,   192,   198,     9,
     401,   136,   136,   192,     9,   401,   294,    32,   223,   193,
     192,   192,   192,   230,   193,   193,   277,   223,   479,   479,
     136,   397,   397,   397,   356,   397,   397,   397,   194,   195,
     468,   132,   133,   182,   209,   460,   479,   267,   388,   112,
     330,    31,   125,   138,   140,   161,   167,   304,   305,   306,
     307,   388,   165,   312,   313,   128,   191,   208,   314,   315,
     296,   241,   479,     9,   193,     9,   193,   193,   457,   321,
     192,   430,   291,   161,   379,   195,   195,    83,   170,    14,
      83,   343,   286,   117,   345,   467,   195,   467,   192,   192,
     195,   194,   195,   294,   284,   136,   407,   407,   407,   356,
     195,   223,   228,   231,    32,   225,   271,   223,   192,   397,
     136,   136,   136,   223,   388,   388,   462,    14,   209,     9,
     193,   194,   460,   457,   307,   177,   194,     9,   193,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      29,    57,    71,    72,    73,    74,    75,    76,    77,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     137,   138,   139,   141,   142,   143,   144,   145,   157,   158,
     159,   169,   171,   172,   174,   181,   182,   184,   186,   187,
     208,   385,   386,     9,   193,   161,   165,   208,   315,   316,
     317,   193,    83,   326,   240,   297,   460,   460,    14,   241,
     195,   292,   293,   460,    14,    83,   343,   192,   191,   467,
     193,   194,   318,   345,   467,   291,   195,   192,   407,   136,
     136,    32,   225,   270,   271,   223,   397,   397,   397,   195,
     193,   193,   397,   388,   300,   479,   308,   309,   396,   305,
      14,    32,    51,   310,   313,     9,    36,   192,    31,    50,
      53,    14,     9,   193,   210,   461,   326,    14,   479,   240,
     193,    14,   343,    38,    83,   376,   194,   223,   467,   318,
     195,   467,   407,   407,   223,    99,   236,   195,   208,   221,
     301,   302,   303,     9,   401,     9,   401,   195,   397,   386,
     386,    59,   311,   316,   316,    31,    50,    53,   397,    83,
     177,   191,   193,   397,   462,   397,    83,     9,   402,   223,
     195,   194,   318,    97,   193,   115,   232,   156,   102,   479,
     178,   396,   168,    14,   469,   298,   191,    38,    83,   192,
     195,   223,   193,   191,   174,   239,   208,   321,   322,   178,
     397,   178,   282,   283,   419,   299,    83,   195,   388,   237,
     171,   208,   193,   192,     9,   402,   119,   120,   121,   324,
     325,   282,    83,   267,   193,   467,   419,   480,   192,   192,
     193,   193,   194,   319,   324,    38,    83,   170,   467,   194,
     223,   480,    83,   170,    14,    83,   319,   223,   195,    38,
      83,   170,    14,    83,   343,   195,    83,   170,    14,    83,
     343,    14,    83,   343,   343
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
#line 731 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 734 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { (yyval).reset();;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval).reset();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 199:

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

  case 200:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 201:

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

  case 202:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { (yyval).reset();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { (yyval).reset();;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval).reset();;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { (yyval).reset();;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval).reset();;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval).reset();;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval).reset();;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval).reset();;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval).reset();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
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

  case 525:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
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

  case 527:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval).reset();;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
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

  case 582:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
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

  case 583:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval).reset();;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval).reset();;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("dict");
                                         _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval).reset();;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval).reset();;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval).reset();;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { (yyval).reset();;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval).reset();;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval).reset();;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval).reset();;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval).reset();;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval).reset();;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { (yyval).reset();;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
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

  case 842:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
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

  case 843:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
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

  case 844:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
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

  case 845:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
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

  case 852:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
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

  case 861:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { (yyval).reset();;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { (yyval)++;;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
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

  case 897:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
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

  case 903:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { (yyval).reset();;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
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

  case 939:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    {;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3270 "hphp.y"
    {;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    {;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3325 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3330 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3332 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3341 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3360 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3362 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3364 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14284 "hphp.5.tab.cpp"
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
#line 3388 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

