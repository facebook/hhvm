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
     T_CALLABLE = 375,
     T_CLASS_C = 376,
     T_METHOD_C = 377,
     T_FUNC_C = 378,
     T_LINE = 379,
     T_FILE = 380,
     T_COMMENT = 381,
     T_DOC_COMMENT = 382,
     T_OPEN_TAG = 383,
     T_OPEN_TAG_WITH_ECHO = 384,
     T_CLOSE_TAG = 385,
     T_WHITESPACE = 386,
     T_START_HEREDOC = 387,
     T_END_HEREDOC = 388,
     T_DOLLAR_OPEN_CURLY_BRACES = 389,
     T_CURLY_OPEN = 390,
     T_DOUBLE_COLON = 391,
     T_NAMESPACE = 392,
     T_NS_C = 393,
     T_DIR = 394,
     T_NS_SEPARATOR = 395,
     T_XHP_LABEL = 396,
     T_XHP_TEXT = 397,
     T_XHP_ATTRIBUTE = 398,
     T_XHP_CATEGORY = 399,
     T_XHP_CATEGORY_LABEL = 400,
     T_XHP_CHILDREN = 401,
     T_ENUM = 402,
     T_XHP_REQUIRED = 403,
     T_TRAIT = 404,
     T_ELLIPSIS = 405,
     T_INSTEADOF = 406,
     T_TRAIT_C = 407,
     T_HH_ERROR = 408,
     T_FINALLY = 409,
     T_XHP_TAG_LT = 410,
     T_XHP_TAG_GT = 411,
     T_TYPELIST_LT = 412,
     T_TYPELIST_GT = 413,
     T_UNRESOLVED_LT = 414,
     T_COLLECTION = 415,
     T_SHAPE = 416,
     T_TYPE = 417,
     T_UNRESOLVED_TYPE = 418,
     T_NEWTYPE = 419,
     T_UNRESOLVED_NEWTYPE = 420,
     T_COMPILER_HALT_OFFSET = 421,
     T_ASYNC = 422,
     T_LAMBDA_OP = 423,
     T_LAMBDA_CP = 424,
     T_UNRESOLVED_OP = 425
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
#line 876 "hphp.5.tab.cpp"

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
#define YYLAST   17092

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  200
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  270
/* YYNRULES -- Number of rules.  */
#define YYNRULES  995
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1829

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   425

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   198,     2,   195,    55,    38,   199,
     190,   191,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   192,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   197,    37,     2,   196,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   193,    36,   194,    58,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189
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
    1676,  1680,  1683,  1686,  1689,  1692,  1696,  1700,  1704,  1708,
    1712,  1716,  1720,  1724,  1728,  1732,  1736,  1742,  1747,  1751,
    1753,  1756,  1759,  1762,  1765,  1768,  1771,  1774,  1777,  1780,
    1782,  1784,  1786,  1790,  1793,  1795,  1801,  1802,  1803,  1816,
    1817,  1831,  1832,  1837,  1838,  1846,  1847,  1853,  1854,  1858,
    1859,  1866,  1869,  1872,  1877,  1879,  1881,  1887,  1891,  1897,
    1901,  1904,  1905,  1908,  1909,  1914,  1919,  1923,  1928,  1933,
    1938,  1943,  1945,  1947,  1949,  1951,  1955,  1959,  1964,  1966,
    1969,  1974,  1977,  1984,  1985,  1987,  1992,  1993,  1996,  1997,
    1999,  2001,  2005,  2007,  2011,  2013,  2015,  2019,  2023,  2025,
    2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,  2045,
    2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,  2065,
    2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,  2085,
    2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,
    2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,  2123,  2125,
    2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,  2143,  2145,
    2147,  2149,  2151,  2153,  2155,  2157,  2159,  2161,  2163,  2165,
    2167,  2169,  2171,  2173,  2175,  2177,  2179,  2181,  2183,  2185,
    2190,  2192,  2194,  2196,  2198,  2200,  2202,  2206,  2208,  2212,
    2214,  2216,  2220,  2222,  2224,  2226,  2229,  2231,  2232,  2233,
    2235,  2237,  2241,  2242,  2244,  2246,  2248,  2250,  2252,  2254,
    2256,  2258,  2260,  2262,  2264,  2266,  2268,  2272,  2275,  2277,
    2279,  2284,  2288,  2293,  2295,  2297,  2301,  2305,  2309,  2313,
    2317,  2321,  2325,  2329,  2333,  2337,  2341,  2345,  2349,  2353,
    2357,  2361,  2365,  2369,  2372,  2375,  2378,  2381,  2385,  2389,
    2393,  2397,  2401,  2405,  2409,  2413,  2417,  2423,  2428,  2432,
    2436,  2440,  2442,  2444,  2446,  2448,  2452,  2456,  2460,  2463,
    2464,  2466,  2467,  2469,  2470,  2476,  2480,  2484,  2486,  2488,
    2490,  2492,  2496,  2499,  2501,  2503,  2505,  2507,  2509,  2513,
    2515,  2517,  2519,  2522,  2525,  2530,  2534,  2539,  2542,  2543,
    2549,  2553,  2557,  2559,  2563,  2565,  2568,  2569,  2575,  2579,
    2582,  2583,  2587,  2588,  2593,  2596,  2597,  2601,  2605,  2607,
    2608,  2610,  2612,  2614,  2616,  2620,  2622,  2624,  2626,  2630,
    2632,  2634,  2638,  2642,  2645,  2650,  2653,  2658,  2664,  2670,
    2676,  2682,  2684,  2686,  2688,  2690,  2692,  2694,  2698,  2702,
    2707,  2712,  2716,  2718,  2720,  2722,  2724,  2728,  2730,  2735,
    2739,  2741,  2743,  2745,  2747,  2749,  2753,  2757,  2762,  2767,
    2771,  2773,  2775,  2783,  2793,  2801,  2808,  2817,  2819,  2822,
    2827,  2832,  2834,  2836,  2838,  2843,  2845,  2846,  2848,  2851,
    2853,  2855,  2857,  2861,  2865,  2869,  2870,  2872,  2874,  2878,
    2882,  2885,  2889,  2896,  2897,  2899,  2904,  2907,  2908,  2914,
    2918,  2922,  2924,  2931,  2936,  2941,  2944,  2947,  2948,  2954,
    2958,  2962,  2964,  2967,  2968,  2974,  2978,  2982,  2984,  2987,
    2990,  2992,  2995,  2997,  3002,  3006,  3010,  3017,  3021,  3023,
    3025,  3027,  3032,  3037,  3042,  3047,  3052,  3057,  3060,  3063,
    3068,  3071,  3074,  3076,  3080,  3084,  3088,  3089,  3092,  3098,
    3105,  3112,  3120,  3122,  3125,  3127,  3130,  3132,  3137,  3139,
    3144,  3148,  3149,  3151,  3155,  3158,  3162,  3164,  3166,  3167,
    3168,  3171,  3174,  3177,  3180,  3185,  3188,  3194,  3198,  3200,
    3202,  3203,  3207,  3212,  3218,  3222,  3224,  3227,  3228,  3233,
    3235,  3239,  3242,  3245,  3248,  3250,  3252,  3254,  3256,  3260,
    3266,  3273,  3275,  3284,  3291,  3293
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     201,     0,    -1,    -1,   202,   203,    -1,   203,   204,    -1,
      -1,   224,    -1,   241,    -1,   248,    -1,   245,    -1,   255,
      -1,   448,    -1,   129,   190,   191,   192,    -1,   156,   217,
     192,    -1,    -1,   156,   217,   193,   205,   203,   194,    -1,
      -1,   156,   193,   206,   203,   194,    -1,   117,   212,   192,
      -1,   117,   111,   212,   192,    -1,   117,   112,   212,   192,
      -1,   117,   210,   193,   215,   194,   192,    -1,   117,   111,
     210,   193,   212,   194,   192,    -1,   117,   112,   210,   193,
     212,   194,   192,    -1,   221,   192,    -1,    81,    -1,   103,
      -1,   162,    -1,   163,    -1,   165,    -1,   167,    -1,   166,
      -1,   207,    -1,   139,    -1,   168,    -1,   132,    -1,   133,
      -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,   120,
      -1,   119,    -1,   112,    -1,   101,    -1,    97,    -1,    99,
      -1,    77,    -1,    95,    -1,    12,    -1,   118,    -1,   109,
      -1,    57,    -1,   170,    -1,   131,    -1,   156,    -1,    72,
      -1,    10,    -1,    11,    -1,   114,    -1,   117,    -1,   125,
      -1,    73,    -1,   137,    -1,    71,    -1,     7,    -1,     6,
      -1,   116,    -1,   138,    -1,    13,    -1,    92,    -1,     4,
      -1,     3,    -1,   113,    -1,    76,    -1,    75,    -1,   107,
      -1,   108,    -1,   110,    -1,   104,    -1,    27,    -1,    29,
      -1,   111,    -1,    74,    -1,   105,    -1,   173,    -1,    96,
      -1,    98,    -1,   100,    -1,   106,    -1,    93,    -1,    94,
      -1,   102,    -1,   115,    -1,   126,    -1,   208,    -1,   130,
      -1,   217,   159,    -1,   159,   217,   159,    -1,   211,     9,
     213,    -1,   213,    -1,   211,   392,    -1,   217,    -1,   159,
     217,    -1,   217,   102,   207,    -1,   159,   217,   102,   207,
      -1,   214,     9,   216,    -1,   216,    -1,   214,   392,    -1,
     213,    -1,   111,   213,    -1,   112,   213,    -1,   207,    -1,
     217,   159,   207,    -1,   217,    -1,   156,   159,   217,    -1,
     159,   217,    -1,   218,   453,    -1,   218,   453,    -1,   221,
       9,   449,    14,   387,    -1,   112,   449,    14,   387,    -1,
     222,   223,    -1,    -1,   224,    -1,   241,    -1,   248,    -1,
     255,    -1,   193,   222,   194,    -1,    74,   331,   224,   277,
     279,    -1,    74,   331,    32,   222,   278,   280,    77,   192,
      -1,    -1,    94,   331,   225,   271,    -1,    -1,    93,   226,
     224,    94,   331,   192,    -1,    -1,    96,   190,   333,   192,
     333,   192,   333,   191,   227,   269,    -1,    -1,   104,   331,
     228,   274,    -1,   108,   192,    -1,   108,   342,   192,    -1,
     110,   192,    -1,   110,   342,   192,    -1,   113,   192,    -1,
     113,   342,   192,    -1,    27,   108,   192,    -1,   118,   287,
     192,    -1,   124,   289,   192,    -1,    92,   332,   192,    -1,
     148,   332,   192,    -1,   126,   190,   445,   191,   192,    -1,
     192,    -1,    86,    -1,    87,    -1,    -1,    98,   190,   342,
     102,   268,   267,   191,   229,   270,    -1,    -1,    98,   190,
     342,    28,   102,   268,   267,   191,   230,   270,    -1,   100,
     190,   273,   191,   272,    -1,    -1,   114,   233,   115,   190,
     378,    83,   191,   193,   222,   194,   235,   231,   238,    -1,
      -1,   114,   233,   173,   232,   236,    -1,   116,   342,   192,
      -1,   109,   207,   192,    -1,   342,   192,    -1,   334,   192,
      -1,   335,   192,    -1,   336,   192,    -1,   337,   192,    -1,
     338,   192,    -1,   113,   337,   192,    -1,   339,   192,    -1,
     340,   192,    -1,   113,   339,   192,    -1,   341,   192,    -1,
     207,    32,    -1,    -1,   193,   234,   222,   194,    -1,   235,
     115,   190,   378,    83,   191,   193,   222,   194,    -1,    -1,
      -1,   193,   237,   222,   194,    -1,   173,   236,    -1,    -1,
      38,    -1,    -1,   111,    -1,    -1,   240,   239,   452,   242,
     190,   283,   191,   457,   317,    -1,    -1,   321,   240,   239,
     452,   243,   190,   283,   191,   457,   317,    -1,    -1,   408,
     320,   240,   239,   452,   244,   190,   283,   191,   457,   317,
      -1,    -1,   166,   207,   246,    32,   468,   447,   193,   290,
     194,    -1,    -1,   408,   166,   207,   247,    32,   468,   447,
     193,   290,   194,    -1,    -1,   261,   258,   249,   262,   263,
     193,   293,   194,    -1,    -1,   408,   261,   258,   250,   262,
     263,   193,   293,   194,    -1,    -1,   131,   259,   251,   264,
     193,   293,   194,    -1,    -1,   408,   131,   259,   252,   264,
     193,   293,   194,    -1,    -1,   130,   254,   385,   262,   263,
     193,   293,   194,    -1,    -1,   168,   260,   256,   263,   193,
     293,   194,    -1,    -1,   408,   168,   260,   257,   263,   193,
     293,   194,    -1,   452,    -1,   160,    -1,   452,    -1,   452,
      -1,   130,    -1,   123,   130,    -1,   123,   122,   130,    -1,
     122,   123,   130,    -1,   122,   130,    -1,   132,   378,    -1,
      -1,   133,   265,    -1,    -1,   132,   265,    -1,    -1,   378,
      -1,   265,     9,   378,    -1,   378,    -1,   266,     9,   378,
      -1,   136,   268,    -1,    -1,   420,    -1,    38,   420,    -1,
     137,   190,   434,   191,    -1,   224,    -1,    32,   222,    97,
     192,    -1,   224,    -1,    32,   222,    99,   192,    -1,   224,
      -1,    32,   222,    95,   192,    -1,   224,    -1,    32,   222,
     101,   192,    -1,   207,    14,   387,    -1,   273,     9,   207,
      14,   387,    -1,   193,   275,   194,    -1,   193,   192,   275,
     194,    -1,    32,   275,   105,   192,    -1,    32,   192,   275,
     105,   192,    -1,   275,   106,   342,   276,   222,    -1,   275,
     107,   276,   222,    -1,    -1,    32,    -1,   192,    -1,   277,
      75,   331,   224,    -1,    -1,   278,    75,   331,    32,   222,
      -1,    -1,    76,   224,    -1,    -1,    76,    32,   222,    -1,
      -1,   282,     9,   409,   323,   469,   169,    83,    -1,   282,
       9,   409,   323,   469,    38,   169,    83,    -1,   282,     9,
     409,   323,   469,   169,    -1,   282,   392,    -1,   409,   323,
     469,   169,    83,    -1,   409,   323,   469,    38,   169,    83,
      -1,   409,   323,   469,   169,    -1,    -1,   409,   323,   469,
      83,    -1,   409,   323,   469,    38,    83,    -1,   409,   323,
     469,    38,    83,    14,   342,    -1,   409,   323,   469,    83,
      14,   342,    -1,   282,     9,   409,   323,   469,    83,    -1,
     282,     9,   409,   323,   469,    38,    83,    -1,   282,     9,
     409,   323,   469,    38,    83,    14,   342,    -1,   282,     9,
     409,   323,   469,    83,    14,   342,    -1,   284,     9,   409,
     469,   169,    83,    -1,   284,     9,   409,   469,    38,   169,
      83,    -1,   284,     9,   409,   469,   169,    -1,   284,   392,
      -1,   409,   469,   169,    83,    -1,   409,   469,    38,   169,
      83,    -1,   409,   469,   169,    -1,    -1,   409,   469,    83,
      -1,   409,   469,    38,    83,    -1,   409,   469,    38,    83,
      14,   342,    -1,   409,   469,    83,    14,   342,    -1,   284,
       9,   409,   469,    83,    -1,   284,     9,   409,   469,    38,
      83,    -1,   284,     9,   409,   469,    38,    83,    14,   342,
      -1,   284,     9,   409,   469,    83,    14,   342,    -1,   286,
     392,    -1,    -1,   342,    -1,    38,   420,    -1,   169,   342,
      -1,   286,     9,   342,    -1,   286,     9,   169,   342,    -1,
     286,     9,    38,   420,    -1,   287,     9,   288,    -1,   288,
      -1,    83,    -1,   195,   420,    -1,   195,   193,   342,   194,
      -1,   289,     9,    83,    -1,   289,     9,    83,    14,   387,
      -1,    83,    -1,    83,    14,   387,    -1,   290,   291,    -1,
      -1,   292,   192,    -1,   450,    14,   387,    -1,   293,   294,
      -1,    -1,    -1,   319,   295,   325,   192,    -1,    -1,   321,
     468,   296,   325,   192,    -1,   326,   192,    -1,   327,   192,
      -1,   328,   192,    -1,    -1,   320,   240,   239,   451,   190,
     297,   281,   191,   457,   318,    -1,    -1,   408,   320,   240,
     239,   452,   190,   298,   281,   191,   457,   318,    -1,   162,
     303,   192,    -1,   163,   311,   192,    -1,   165,   313,   192,
      -1,     4,   132,   378,   192,    -1,     4,   133,   378,   192,
      -1,   117,   266,   192,    -1,   117,   266,   193,   299,   194,
      -1,   299,   300,    -1,   299,   301,    -1,    -1,   220,   155,
     207,   170,   266,   192,    -1,   302,   102,   320,   207,   192,
      -1,   302,   102,   321,   192,    -1,   220,   155,   207,    -1,
     207,    -1,   304,    -1,   303,     9,   304,    -1,   305,   375,
     309,   310,    -1,   160,    -1,    31,   306,    -1,   306,    -1,
     138,    -1,   138,   176,   468,   391,   177,    -1,   138,   176,
     468,     9,   468,   177,    -1,   378,    -1,   125,    -1,   166,
     193,   308,   194,    -1,   139,    -1,   386,    -1,   307,     9,
     386,    -1,   307,   391,    -1,    14,   387,    -1,    -1,    59,
     167,    -1,    -1,   312,    -1,   311,     9,   312,    -1,   164,
      -1,   314,    -1,   207,    -1,   128,    -1,   190,   315,   191,
      -1,   190,   315,   191,    53,    -1,   190,   315,   191,    31,
      -1,   190,   315,   191,    50,    -1,   314,    -1,   316,    -1,
     316,    53,    -1,   316,    31,    -1,   316,    50,    -1,   315,
       9,   315,    -1,   315,    36,   315,    -1,   207,    -1,   160,
      -1,   164,    -1,   192,    -1,   193,   222,   194,    -1,   192,
      -1,   193,   222,   194,    -1,   321,    -1,   125,    -1,   321,
      -1,    -1,   322,    -1,   321,   322,    -1,   119,    -1,   120,
      -1,   121,    -1,   124,    -1,   123,    -1,   122,    -1,   186,
      -1,   324,    -1,    -1,   119,    -1,   120,    -1,   121,    -1,
     325,     9,    83,    -1,   325,     9,    83,    14,   387,    -1,
      83,    -1,    83,    14,   387,    -1,   326,     9,   450,    14,
     387,    -1,   112,   450,    14,   387,    -1,   327,     9,   450,
      -1,   123,   112,   450,    -1,   123,   329,   447,    -1,   329,
     447,    14,   468,    -1,   112,   181,   452,    -1,   190,   330,
     191,    -1,    72,   382,   385,    -1,    72,   253,    -1,    71,
     342,    -1,   367,    -1,   362,    -1,   190,   342,   191,    -1,
     332,     9,   342,    -1,   342,    -1,   332,    -1,    -1,    27,
      -1,    27,   342,    -1,    27,   342,   136,   342,    -1,   190,
     334,   191,    -1,   420,    14,   334,    -1,   137,   190,   434,
     191,    14,   334,    -1,    29,   342,    -1,   420,    14,   337,
      -1,    28,   342,    -1,   420,    14,   339,    -1,   137,   190,
     434,   191,    14,   339,    -1,   343,    -1,   420,    -1,   330,
      -1,   424,    -1,   423,    -1,   137,   190,   434,   191,    14,
     342,    -1,   420,    14,   342,    -1,   420,    14,    38,   420,
      -1,   420,    14,    38,    72,   382,   385,    -1,   420,    26,
     342,    -1,   420,    25,   342,    -1,   420,    24,   342,    -1,
     420,    23,   342,    -1,   420,    22,   342,    -1,   420,    21,
     342,    -1,   420,    20,   342,    -1,   420,    19,   342,    -1,
     420,    18,   342,    -1,   420,    17,   342,    -1,   420,    16,
     342,    -1,   420,    15,   342,    -1,   420,    68,    -1,    68,
     420,    -1,   420,    67,    -1,    67,   420,    -1,   342,    34,
     342,    -1,   342,    35,   342,    -1,   342,    10,   342,    -1,
     342,    12,   342,    -1,   342,    11,   342,    -1,   342,    36,
     342,    -1,   342,    38,   342,    -1,   342,    37,   342,    -1,
     342,    52,   342,    -1,   342,    50,   342,    -1,   342,    51,
     342,    -1,   342,    53,   342,    -1,   342,    54,   342,    -1,
     342,    69,   342,    -1,   342,    55,   342,    -1,   342,    30,
     342,    -1,   342,    49,   342,    -1,   342,    48,   342,    -1,
      50,   342,    -1,    51,   342,    -1,    56,   342,    -1,    58,
     342,    -1,   342,    40,   342,    -1,   342,    39,   342,    -1,
     342,    42,   342,    -1,   342,    41,   342,    -1,   342,    43,
     342,    -1,   342,    47,   342,    -1,   342,    44,   342,    -1,
     342,    46,   342,    -1,   342,    45,   342,    -1,   342,    57,
     382,    -1,   190,   343,   191,    -1,   342,    31,   342,    32,
     342,    -1,   342,    31,    32,   342,    -1,   342,    33,   342,
      -1,   444,    -1,    66,   342,    -1,    65,   342,    -1,    64,
     342,    -1,    63,   342,    -1,    62,   342,    -1,    61,   342,
      -1,    60,   342,    -1,    73,   383,    -1,    59,   342,    -1,
     389,    -1,   361,    -1,   360,    -1,   196,   384,   196,    -1,
      13,   342,    -1,   364,    -1,   117,   190,   366,   392,   191,
      -1,    -1,    -1,   240,   239,   190,   346,   283,   191,   457,
     344,   457,   193,   222,   194,    -1,    -1,   321,   240,   239,
     190,   347,   283,   191,   457,   344,   457,   193,   222,   194,
      -1,    -1,   186,    83,   349,   354,    -1,    -1,   186,   187,
     350,   283,   188,   457,   354,    -1,    -1,   186,   193,   351,
     222,   194,    -1,    -1,    83,   352,   354,    -1,    -1,   187,
     353,   283,   188,   457,   354,    -1,     8,   342,    -1,     8,
     339,    -1,     8,   193,   222,   194,    -1,    91,    -1,   446,
      -1,   356,     9,   355,   136,   342,    -1,   355,   136,   342,
      -1,   357,     9,   355,   136,   387,    -1,   355,   136,   387,
      -1,   356,   391,    -1,    -1,   357,   391,    -1,    -1,   180,
     190,   358,   191,    -1,   138,   190,   435,   191,    -1,    70,
     435,   197,    -1,   378,   193,   437,   194,    -1,   378,   193,
     439,   194,    -1,   364,    70,   430,   197,    -1,   365,    70,
     430,   197,    -1,   361,    -1,   446,    -1,   423,    -1,    91,
      -1,   190,   343,   191,    -1,   366,     9,    83,    -1,   366,
       9,    38,    83,    -1,    83,    -1,    38,    83,    -1,   174,
     160,   368,   175,    -1,   370,    54,    -1,   370,   175,   371,
     174,    54,   369,    -1,    -1,   160,    -1,   370,   372,    14,
     373,    -1,    -1,   371,   374,    -1,    -1,   160,    -1,   161,
      -1,   193,   342,   194,    -1,   161,    -1,   193,   342,   194,
      -1,   367,    -1,   376,    -1,   375,    32,   376,    -1,   375,
      51,   376,    -1,   207,    -1,    73,    -1,   111,    -1,   112,
      -1,   113,    -1,    27,    -1,    29,    -1,    28,    -1,   114,
      -1,   115,    -1,   173,    -1,   116,    -1,    74,    -1,    75,
      -1,    77,    -1,    76,    -1,    94,    -1,    95,    -1,    93,
      -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,   100,
      -1,   101,    -1,    57,    -1,   102,    -1,   104,    -1,   105,
      -1,   106,    -1,   107,    -1,   108,    -1,   110,    -1,   109,
      -1,    92,    -1,    13,    -1,   130,    -1,   131,    -1,   132,
      -1,   133,    -1,    72,    -1,    71,    -1,   125,    -1,     5,
      -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,   156,
      -1,   117,    -1,   118,    -1,   127,    -1,   128,    -1,   129,
      -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,   120,
      -1,   119,    -1,   186,    -1,   126,    -1,   137,    -1,   138,
      -1,    10,    -1,    12,    -1,    11,    -1,   140,    -1,   142,
      -1,   141,    -1,   143,    -1,   144,    -1,   158,    -1,   157,
      -1,   185,    -1,   168,    -1,   171,    -1,   170,    -1,   181,
      -1,   183,    -1,   180,    -1,   219,   190,   285,   191,    -1,
     220,    -1,   160,    -1,   378,    -1,   386,    -1,   124,    -1,
     428,    -1,   190,   343,   191,    -1,   379,    -1,   380,   155,
     427,    -1,   379,    -1,   426,    -1,   381,   155,   427,    -1,
     378,    -1,   124,    -1,   432,    -1,   190,   191,    -1,   331,
      -1,    -1,    -1,    90,    -1,   441,    -1,   190,   285,   191,
      -1,    -1,    78,    -1,    79,    -1,    80,    -1,    91,    -1,
     143,    -1,   144,    -1,   158,    -1,   140,    -1,   171,    -1,
     141,    -1,   142,    -1,   157,    -1,   185,    -1,   151,    90,
     152,    -1,   151,   152,    -1,   386,    -1,   218,    -1,   138,
     190,   390,   191,    -1,    70,   390,   197,    -1,   180,   190,
     359,   191,    -1,   388,    -1,   363,    -1,   190,   387,   191,
      -1,   387,    34,   387,    -1,   387,    35,   387,    -1,   387,
      10,   387,    -1,   387,    12,   387,    -1,   387,    11,   387,
      -1,   387,    36,   387,    -1,   387,    38,   387,    -1,   387,
      37,   387,    -1,   387,    52,   387,    -1,   387,    50,   387,
      -1,   387,    51,   387,    -1,   387,    53,   387,    -1,   387,
      54,   387,    -1,   387,    55,   387,    -1,   387,    49,   387,
      -1,   387,    48,   387,    -1,   387,    69,   387,    -1,    56,
     387,    -1,    58,   387,    -1,    50,   387,    -1,    51,   387,
      -1,   387,    40,   387,    -1,   387,    39,   387,    -1,   387,
      42,   387,    -1,   387,    41,   387,    -1,   387,    43,   387,
      -1,   387,    47,   387,    -1,   387,    44,   387,    -1,   387,
      46,   387,    -1,   387,    45,   387,    -1,   387,    31,   387,
      32,   387,    -1,   387,    31,    32,   387,    -1,   220,   155,
     208,    -1,   160,   155,   208,    -1,   220,   155,   130,    -1,
     218,    -1,    82,    -1,   446,    -1,   386,    -1,   198,   441,
     198,    -1,   199,   441,   199,    -1,   151,   441,   152,    -1,
     393,   391,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,
     393,     9,   387,   136,   387,    -1,   393,     9,   387,    -1,
     387,   136,   387,    -1,   387,    -1,    78,    -1,    79,    -1,
      80,    -1,   151,    90,   152,    -1,   151,   152,    -1,    78,
      -1,    79,    -1,    80,    -1,   207,    -1,    91,    -1,    91,
      52,   396,    -1,   394,    -1,   396,    -1,   207,    -1,    50,
     395,    -1,    51,   395,    -1,   138,   190,   398,   191,    -1,
      70,   398,   197,    -1,   180,   190,   401,   191,    -1,   399,
     391,    -1,    -1,   399,     9,   397,   136,   397,    -1,   399,
       9,   397,    -1,   397,   136,   397,    -1,   397,    -1,   400,
       9,   397,    -1,   397,    -1,   402,   391,    -1,    -1,   402,
       9,   355,   136,   397,    -1,   355,   136,   397,    -1,   400,
     391,    -1,    -1,   190,   403,   191,    -1,    -1,   405,     9,
     207,   404,    -1,   207,   404,    -1,    -1,   407,   405,   391,
      -1,    49,   406,    48,    -1,   408,    -1,    -1,   134,    -1,
     135,    -1,   207,    -1,   160,    -1,   193,   342,   194,    -1,
     411,    -1,   427,    -1,   207,    -1,   193,   342,   194,    -1,
     413,    -1,   427,    -1,    70,   430,   197,    -1,   193,   342,
     194,    -1,   421,   415,    -1,   190,   330,   191,   415,    -1,
     433,   415,    -1,   190,   330,   191,   415,    -1,   190,   330,
     191,   410,   412,    -1,   190,   343,   191,   410,   412,    -1,
     190,   330,   191,   410,   411,    -1,   190,   343,   191,   410,
     411,    -1,   427,    -1,   377,    -1,   425,    -1,   426,    -1,
     416,    -1,   418,    -1,   420,   410,   412,    -1,   381,   155,
     427,    -1,   422,   190,   285,   191,    -1,   423,   190,   285,
     191,    -1,   190,   420,   191,    -1,   377,    -1,   425,    -1,
     426,    -1,   416,    -1,   420,   410,   411,    -1,   419,    -1,
     422,   190,   285,   191,    -1,   190,   420,   191,    -1,   427,
      -1,   416,    -1,   377,    -1,   361,    -1,   386,    -1,   190,
     420,   191,    -1,   190,   343,   191,    -1,   423,   190,   285,
     191,    -1,   422,   190,   285,   191,    -1,   190,   424,   191,
      -1,   345,    -1,   348,    -1,   420,   410,   414,   453,   190,
     285,   191,    -1,   190,   330,   191,   410,   414,   453,   190,
     285,   191,    -1,   381,   155,   209,   453,   190,   285,   191,
      -1,   381,   155,   427,   190,   285,   191,    -1,   381,   155,
     193,   342,   194,   190,   285,   191,    -1,   428,    -1,   431,
     428,    -1,   428,    70,   430,   197,    -1,   428,   193,   342,
     194,    -1,   429,    -1,    83,    -1,    84,    -1,   195,   193,
     342,   194,    -1,   342,    -1,    -1,   195,    -1,   431,   195,
      -1,   427,    -1,   417,    -1,   418,    -1,   432,   410,   412,
      -1,   380,   155,   427,    -1,   190,   420,   191,    -1,    -1,
     417,    -1,   419,    -1,   432,   410,   411,    -1,   190,   420,
     191,    -1,   434,     9,    -1,   434,     9,   420,    -1,   434,
       9,   137,   190,   434,   191,    -1,    -1,   420,    -1,   137,
     190,   434,   191,    -1,   436,   391,    -1,    -1,   436,     9,
     342,   136,   342,    -1,   436,     9,   342,    -1,   342,   136,
     342,    -1,   342,    -1,   436,     9,   342,   136,    38,   420,
      -1,   436,     9,    38,   420,    -1,   342,   136,    38,   420,
      -1,    38,   420,    -1,   438,   391,    -1,    -1,   438,     9,
     342,   136,   342,    -1,   438,     9,   342,    -1,   342,   136,
     342,    -1,   342,    -1,   440,   391,    -1,    -1,   440,     9,
     387,   136,   387,    -1,   440,     9,   387,    -1,   387,   136,
     387,    -1,   387,    -1,   441,   442,    -1,   441,    90,    -1,
     442,    -1,    90,   442,    -1,    83,    -1,    83,    70,   443,
     197,    -1,    83,   410,   207,    -1,   153,   342,   194,    -1,
     153,    82,    70,   342,   197,   194,    -1,   154,   420,   194,
      -1,   207,    -1,    85,    -1,    83,    -1,   127,   190,   332,
     191,    -1,   128,   190,   420,   191,    -1,   128,   190,   343,
     191,    -1,   128,   190,   424,   191,    -1,   128,   190,   423,
     191,    -1,   128,   190,   330,   191,    -1,     7,   342,    -1,
       6,   342,    -1,     5,   190,   342,   191,    -1,     4,   342,
      -1,     3,   342,    -1,   420,    -1,   445,     9,   420,    -1,
     381,   155,   208,    -1,   381,   155,   130,    -1,    -1,   102,
     468,    -1,   181,   452,    14,   468,   192,    -1,   408,   181,
     452,    14,   468,   192,    -1,   183,   452,   447,    14,   468,
     192,    -1,   408,   183,   452,   447,    14,   468,   192,    -1,
     209,    -1,   468,   209,    -1,   208,    -1,   468,   208,    -1,
     209,    -1,   209,   176,   459,   177,    -1,   207,    -1,   207,
     176,   459,   177,    -1,   176,   455,   177,    -1,    -1,   468,
      -1,   454,     9,   468,    -1,   454,   391,    -1,   454,     9,
     169,    -1,   455,    -1,   169,    -1,    -1,    -1,    32,   468,
      -1,   102,   468,    -1,   103,   468,    -1,   460,   391,    -1,
     460,     9,   461,   207,    -1,   461,   207,    -1,   460,     9,
     461,   207,   458,    -1,   461,   207,   458,    -1,    50,    -1,
      51,    -1,    -1,    91,   136,   468,    -1,    31,    91,   136,
     468,    -1,   220,   155,   207,   136,   468,    -1,   463,     9,
     462,    -1,   462,    -1,   463,   391,    -1,    -1,   180,   190,
     464,   191,    -1,   220,    -1,   207,   155,   467,    -1,   207,
     453,    -1,    31,   468,    -1,    59,   468,    -1,   220,    -1,
     138,    -1,   139,    -1,   465,    -1,   466,   155,   467,    -1,
     138,   176,   468,   391,   177,    -1,   138,   176,   468,     9,
     468,   177,    -1,   160,    -1,   190,   111,   190,   456,   191,
      32,   468,   191,    -1,   190,   468,     9,   454,   391,   191,
      -1,   468,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   730,   730,   730,   739,   741,   744,   745,   746,   747,
     748,   749,   750,   753,   755,   755,   757,   757,   759,   761,
     764,   767,   771,   775,   779,   784,   785,   786,   787,   788,
     789,   790,   794,   795,   796,   797,   798,   799,   800,   801,
     802,   803,   804,   805,   806,   807,   808,   809,   810,   811,
     812,   813,   814,   815,   816,   817,   818,   819,   820,   821,
     822,   823,   824,   825,   826,   827,   828,   829,   830,   831,
     832,   833,   834,   835,   836,   837,   838,   839,   840,   841,
     842,   843,   844,   845,   846,   847,   848,   849,   850,   851,
     852,   853,   854,   855,   859,   863,   864,   868,   869,   874,
     876,   881,   886,   887,   888,   890,   895,   897,   902,   907,
     909,   911,   916,   917,   921,   922,   924,   928,   935,   942,
     946,   952,   954,   957,   958,   959,   960,   963,   964,   968,
     973,   973,   979,   979,   986,   985,   991,   991,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1014,  1012,  1021,  1019,  1026,  1036,  1030,
    1040,  1038,  1042,  1043,  1047,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1066,  1066,  1071,  1077,
    1081,  1081,  1089,  1090,  1094,  1095,  1099,  1105,  1103,  1118,
    1115,  1131,  1128,  1145,  1144,  1153,  1151,  1163,  1162,  1181,
    1179,  1198,  1197,  1206,  1204,  1215,  1215,  1222,  1221,  1233,
    1231,  1244,  1245,  1249,  1252,  1255,  1256,  1257,  1260,  1261,
    1264,  1266,  1269,  1270,  1273,  1274,  1277,  1278,  1282,  1283,
    1288,  1289,  1292,  1293,  1294,  1298,  1299,  1303,  1304,  1308,
    1309,  1313,  1314,  1319,  1320,  1326,  1327,  1328,  1329,  1332,
    1335,  1337,  1340,  1341,  1345,  1347,  1350,  1353,  1356,  1357,
    1360,  1361,  1365,  1371,  1377,  1384,  1386,  1391,  1396,  1402,
    1406,  1410,  1414,  1419,  1424,  1429,  1434,  1440,  1449,  1454,
    1459,  1465,  1467,  1471,  1475,  1480,  1484,  1487,  1490,  1494,
    1498,  1502,  1506,  1511,  1519,  1521,  1524,  1525,  1526,  1527,
    1529,  1531,  1536,  1537,  1540,  1541,  1542,  1546,  1547,  1549,
    1550,  1554,  1556,  1559,  1563,  1569,  1571,  1574,  1574,  1578,
    1577,  1581,  1583,  1586,  1589,  1587,  1603,  1599,  1613,  1615,
    1617,  1619,  1621,  1623,  1625,  1629,  1630,  1631,  1634,  1640,
    1644,  1650,  1653,  1658,  1660,  1665,  1670,  1674,  1675,  1679,
    1680,  1682,  1684,  1690,  1691,  1693,  1697,  1698,  1703,  1707,
    1708,  1712,  1713,  1717,  1719,  1725,  1730,  1731,  1733,  1737,
    1738,  1739,  1740,  1744,  1745,  1746,  1747,  1748,  1749,  1751,
    1756,  1759,  1760,  1764,  1765,  1769,  1770,  1773,  1774,  1777,
    1778,  1781,  1782,  1786,  1787,  1788,  1789,  1790,  1791,  1792,
    1796,  1797,  1800,  1801,  1802,  1805,  1807,  1809,  1810,  1813,
    1815,  1819,  1821,  1825,  1829,  1833,  1838,  1839,  1841,  1842,
    1843,  1844,  1847,  1851,  1852,  1856,  1857,  1861,  1862,  1863,
    1864,  1868,  1872,  1877,  1881,  1885,  1889,  1893,  1898,  1899,
    1900,  1901,  1902,  1906,  1908,  1909,  1910,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,
    1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,
    1936,  1937,  1938,  1939,  1940,  1941,  1942,  1943,  1944,  1945,
    1946,  1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,
    1956,  1958,  1959,  1961,  1962,  1964,  1965,  1966,  1967,  1968,
    1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,
    1979,  1980,  1981,  1982,  1983,  1987,  1991,  1996,  1995,  2010,
    2008,  2026,  2025,  2044,  2043,  2062,  2061,  2079,  2079,  2094,
    2094,  2112,  2113,  2114,  2119,  2121,  2125,  2129,  2135,  2139,
    2145,  2147,  2151,  2153,  2157,  2161,  2162,  2166,  2173,  2180,
    2182,  2187,  2188,  2189,  2190,  2192,  2196,  2197,  2198,  2199,
    2203,  2209,  2218,  2231,  2232,  2235,  2238,  2241,  2242,  2245,
    2249,  2252,  2255,  2262,  2263,  2267,  2268,  2270,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,
    2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,
    2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,
    2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,
    2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2325,
    2326,  2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,  2335,
    2336,  2337,  2338,  2339,  2340,  2341,  2342,  2343,  2344,  2345,
    2346,  2347,  2348,  2349,  2350,  2351,  2352,  2353,  2354,  2358,
    2363,  2364,  2368,  2369,  2370,  2371,  2373,  2377,  2378,  2389,
    2390,  2392,  2404,  2405,  2406,  2410,  2411,  2412,  2416,  2417,
    2418,  2421,  2423,  2427,  2428,  2429,  2430,  2432,  2433,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2444,  2449,  2450,
    2451,  2453,  2454,  2456,  2457,  2458,  2459,  2461,  2463,  2465,
    2467,  2469,  2470,  2471,  2472,  2473,  2474,  2475,  2476,  2477,
    2478,  2479,  2480,  2481,  2482,  2483,  2484,  2485,  2487,  2489,
    2491,  2493,  2494,  2497,  2498,  2502,  2506,  2508,  2512,  2515,
    2518,  2524,  2525,  2526,  2527,  2528,  2529,  2530,  2535,  2537,
    2541,  2542,  2545,  2546,  2550,  2553,  2555,  2557,  2561,  2562,
    2563,  2564,  2567,  2571,  2572,  2573,  2574,  2578,  2580,  2587,
    2588,  2589,  2590,  2591,  2592,  2594,  2595,  2600,  2602,  2605,
    2608,  2610,  2612,  2615,  2617,  2621,  2623,  2626,  2629,  2635,
    2637,  2640,  2641,  2646,  2649,  2653,  2653,  2658,  2661,  2662,
    2666,  2667,  2671,  2672,  2673,  2677,  2679,  2687,  2688,  2692,
    2694,  2702,  2703,  2707,  2708,  2713,  2715,  2720,  2731,  2745,
    2757,  2772,  2773,  2774,  2775,  2776,  2777,  2778,  2788,  2797,
    2799,  2801,  2805,  2806,  2807,  2808,  2809,  2825,  2826,  2828,
    2837,  2838,  2839,  2840,  2841,  2842,  2843,  2844,  2846,  2851,
    2855,  2856,  2860,  2863,  2870,  2874,  2883,  2890,  2892,  2898,
    2900,  2901,  2905,  2906,  2907,  2914,  2915,  2920,  2921,  2926,
    2927,  2928,  2929,  2940,  2943,  2946,  2947,  2948,  2949,  2960,
    2964,  2965,  2966,  2968,  2969,  2970,  2974,  2976,  2979,  2981,
    2982,  2983,  2984,  2987,  2989,  2990,  2994,  2996,  2999,  3001,
    3002,  3003,  3007,  3009,  3012,  3015,  3017,  3019,  3023,  3024,
    3026,  3027,  3033,  3034,  3036,  3046,  3048,  3050,  3053,  3054,
    3055,  3059,  3060,  3061,  3062,  3063,  3064,  3065,  3066,  3067,
    3068,  3069,  3073,  3074,  3078,  3080,  3088,  3090,  3094,  3098,
    3103,  3107,  3115,  3116,  3120,  3121,  3127,  3128,  3137,  3138,
    3146,  3149,  3153,  3156,  3161,  3166,  3168,  3169,  3170,  3174,
    3175,  3179,  3180,  3183,  3188,  3191,  3193,  3197,  3203,  3204,
    3205,  3209,  3213,  3223,  3231,  3233,  3237,  3239,  3244,  3250,
    3253,  3258,  3266,  3269,  3272,  3273,  3276,  3279,  3280,  3285,
    3288,  3292,  3296,  3302,  3312,  3313
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
      40,    41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   200,   202,   201,   203,   203,   204,   204,   204,   204,
     204,   204,   204,   204,   205,   204,   206,   204,   204,   204,
     204,   204,   204,   204,   204,   207,   207,   207,   207,   207,
     207,   207,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   209,   209,   210,   210,   211,
     211,   212,   213,   213,   213,   213,   214,   214,   215,   216,
     216,   216,   217,   217,   218,   218,   218,   219,   220,   221,
     221,   222,   222,   223,   223,   223,   223,   224,   224,   224,
     225,   224,   226,   224,   227,   224,   228,   224,   224,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   229,   224,   230,   224,   224,   231,   224,
     232,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   224,   224,   224,   234,   233,   235,   235,
     237,   236,   238,   238,   239,   239,   240,   242,   241,   243,
     241,   244,   241,   246,   245,   247,   245,   249,   248,   250,
     248,   251,   248,   252,   248,   254,   253,   256,   255,   257,
     255,   258,   258,   259,   260,   261,   261,   261,   261,   261,
     262,   262,   263,   263,   264,   264,   265,   265,   266,   266,
     267,   267,   268,   268,   268,   269,   269,   270,   270,   271,
     271,   272,   272,   273,   273,   274,   274,   274,   274,   275,
     275,   275,   276,   276,   277,   277,   278,   278,   279,   279,
     280,   280,   281,   281,   281,   281,   281,   281,   281,   281,
     282,   282,   282,   282,   282,   282,   282,   282,   283,   283,
     283,   283,   283,   283,   283,   283,   284,   284,   284,   284,
     284,   284,   284,   284,   285,   285,   286,   286,   286,   286,
     286,   286,   287,   287,   288,   288,   288,   289,   289,   289,
     289,   290,   290,   291,   292,   293,   293,   295,   294,   296,
     294,   294,   294,   294,   297,   294,   298,   294,   294,   294,
     294,   294,   294,   294,   294,   299,   299,   299,   300,   301,
     301,   302,   302,   303,   303,   304,   304,   305,   305,   306,
     306,   306,   306,   306,   306,   306,   307,   307,   308,   309,
     309,   310,   310,   311,   311,   312,   313,   313,   313,   314,
     314,   314,   314,   315,   315,   315,   315,   315,   315,   315,
     316,   316,   316,   317,   317,   318,   318,   319,   319,   320,
     320,   321,   321,   322,   322,   322,   322,   322,   322,   322,
     323,   323,   324,   324,   324,   325,   325,   325,   325,   326,
     326,   327,   327,   328,   328,   329,   330,   330,   330,   330,
     330,   330,   331,   332,   332,   333,   333,   334,   334,   334,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   342,
     342,   342,   342,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   344,   344,   346,   345,   347,
     345,   349,   348,   350,   348,   351,   348,   352,   348,   353,
     348,   354,   354,   354,   355,   355,   356,   356,   357,   357,
     358,   358,   359,   359,   360,   361,   361,   362,   363,   364,
     364,   365,   365,   365,   365,   365,   366,   366,   366,   366,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     373,   373,   374,   374,   374,   375,   375,   375,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   377,
     378,   378,   379,   379,   379,   379,   379,   380,   380,   381,
     381,   381,   382,   382,   382,   383,   383,   383,   384,   384,
     384,   385,   385,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   388,   388,
     388,   389,   389,   389,   389,   389,   389,   389,   390,   390,
     391,   391,   392,   392,   393,   393,   393,   393,   394,   394,
     394,   394,   394,   395,   395,   395,   395,   396,   396,   397,
     397,   397,   397,   397,   397,   397,   397,   398,   398,   399,
     399,   399,   399,   400,   400,   401,   401,   402,   402,   403,
     403,   404,   404,   405,   405,   407,   406,   408,   409,   409,
     410,   410,   411,   411,   411,   412,   412,   413,   413,   414,
     414,   415,   415,   416,   416,   417,   417,   418,   418,   419,
     419,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   421,   421,   421,   421,   421,   421,   421,   421,
     422,   422,   422,   422,   422,   422,   422,   422,   422,   423,
     424,   424,   425,   425,   426,   426,   426,   427,   427,   428,
     428,   428,   429,   429,   429,   430,   430,   431,   431,   432,
     432,   432,   432,   432,   432,   433,   433,   433,   433,   433,
     434,   434,   434,   434,   434,   434,   435,   435,   436,   436,
     436,   436,   436,   436,   436,   436,   437,   437,   438,   438,
     438,   438,   439,   439,   440,   440,   440,   440,   441,   441,
     441,   441,   442,   442,   442,   442,   442,   442,   443,   443,
     443,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   445,   445,   446,   446,   447,   447,   448,   448,
     448,   448,   449,   449,   450,   450,   451,   451,   452,   452,
     453,   453,   454,   454,   455,   456,   456,   456,   456,   457,
     457,   458,   458,   459,   460,   460,   460,   460,   461,   461,
     461,   462,   462,   462,   463,   463,   464,   464,   465,   466,
     467,   467,   468,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   469,   469
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
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     3,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     3,     2,     1,     5,     0,     0,    12,     0,
      13,     0,     4,     0,     7,     0,     5,     0,     3,     0,
       6,     2,     2,     4,     1,     1,     5,     3,     5,     3,
       2,     0,     2,     0,     4,     4,     3,     4,     4,     4,
       4,     1,     1,     1,     1,     3,     3,     4,     1,     2,
       4,     2,     6,     0,     1,     4,     0,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       1,     3,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       4,     3,     4,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     2,     2,     4,     3,     4,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     4,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     2,     2,     2,     4,     2,     5,     3,     1,     1,
       0,     3,     4,     5,     3,     1,     2,     0,     4,     1,
       3,     2,     2,     2,     1,     1,     1,     1,     3,     5,
       6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   427,     0,     0,   795,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   887,
       0,   875,   677,     0,   683,   684,   685,    25,   742,   862,
     863,   151,   152,   686,     0,   132,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   186,     0,     0,     0,     0,
       0,     0,   393,   394,   395,   398,   397,   396,     0,     0,
       0,     0,   215,     0,     0,     0,   690,   692,   693,   687,
     688,     0,     0,     0,   694,   689,     0,   661,    27,    28,
      29,    31,    30,     0,   691,     0,     0,     0,     0,   695,
     399,   529,     0,   150,   122,   867,   678,     0,     0,     4,
     112,   114,   741,     0,   660,     0,     6,   185,     7,     9,
       8,    10,     0,     0,   391,   440,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   438,   850,   851,   511,   510,
     421,   514,     0,   420,   822,   662,   669,     0,   744,   509,
     390,   825,   826,   837,   439,     0,     0,   442,   441,   823,
     824,   821,   857,   861,     0,   499,   743,    11,   398,   397,
     396,     0,     0,    31,     0,   112,   185,     0,   931,   439,
     930,     0,   928,   927,   513,     0,   428,   435,   433,     0,
       0,   481,   482,   483,   484,   508,   506,   505,   504,   503,
     502,   501,   500,   862,   686,   664,     0,     0,   951,   843,
     662,     0,   663,   462,     0,   460,     0,   891,     0,   751,
     419,   673,   205,     0,   951,   418,   672,   667,     0,   682,
     663,   870,   871,   877,   869,   674,     0,     0,   676,   507,
       0,     0,     0,     0,   424,     0,   130,   426,     0,     0,
     136,   138,     0,     0,   140,     0,    72,    71,    66,    65,
      57,    58,    49,    69,    80,    81,     0,    52,     0,    64,
      56,    62,    83,    75,    74,    47,    70,    90,    91,    48,
      86,    45,    87,    46,    88,    44,    92,    79,    84,    89,
      76,    77,    51,    78,    82,    43,    73,    59,    93,    67,
      60,    50,    42,    41,    40,    39,    38,    37,    61,    94,
      96,    54,    35,    36,    63,   985,   986,    55,   991,    34,
      53,    85,     0,     0,   112,    95,   942,   984,     0,   987,
       0,     0,   142,     0,     0,     0,   176,     0,     0,     0,
       0,     0,     0,   753,     0,   100,   102,   304,     0,     0,
     303,     0,   219,     0,   216,   309,     0,     0,     0,     0,
       0,   948,   201,   213,   883,   887,     0,   912,     0,   697,
       0,     0,     0,   910,     0,    16,     0,   116,   193,   207,
     214,   566,   541,     0,   936,   521,   523,   525,   799,   427,
     440,     0,     0,   438,   439,   441,     0,     0,   679,     0,
     680,     0,     0,     0,   175,     0,     0,   118,   295,     0,
      24,   184,     0,   212,   197,   211,   396,   399,   185,   392,
     165,   166,   167,   168,   169,   171,   172,   174,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   875,     0,   164,   866,
     866,   897,     0,     0,     0,     0,     0,     0,     0,     0,
     389,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   461,   459,   800,   801,     0,   866,
       0,   813,   295,   295,   866,     0,   868,   858,   883,     0,
     185,     0,     0,   144,     0,   797,   792,   751,     0,   440,
     438,     0,   895,     0,   546,   750,   886,   682,   440,   438,
     439,   118,     0,   295,   417,     0,   815,   675,     0,   122,
     255,     0,   528,     0,   147,     0,     0,   425,     0,     0,
       0,     0,     0,   139,   163,   141,   985,   986,   982,   983,
       0,   977,     0,     0,     0,     0,    68,    33,    55,    32,
     943,   170,   173,   143,   122,     0,   160,   162,     0,     0,
       0,     0,   103,     0,   752,   101,    18,     0,    97,     0,
     305,     0,   145,   218,   217,     0,     0,   146,   932,     0,
       0,   440,   438,   439,   442,   441,     0,   970,   225,     0,
     884,     0,     0,   148,     0,     0,   696,   911,   742,     0,
       0,   909,   747,   908,   115,     5,    13,    14,     0,   223,
       0,     0,   534,     0,     0,     0,   751,     0,     0,   670,
     665,   535,     0,     0,     0,     0,   799,   122,     0,   753,
     798,   995,   416,   430,   495,   831,   849,   127,   121,   123,
     124,   125,   126,   390,     0,   512,   745,   746,   113,   751,
       0,   952,     0,     0,     0,   753,   296,     0,   517,   187,
     221,     0,   465,   467,   466,   478,     0,     0,   498,   463,
     464,   468,   470,   469,   486,   485,   488,   487,   489,   491,
     493,   492,   490,   480,   479,   472,   473,   471,   474,   475,
     477,   494,   476,   865,     0,     0,   901,     0,   751,   935,
       0,   934,   951,   828,   857,   203,   195,   209,     0,   936,
     199,   185,     0,   431,   434,   436,   444,   458,   457,   456,
     455,   454,   453,   452,   451,   450,   449,   448,   447,   803,
       0,   802,   805,   827,   809,   951,   806,     0,     0,     0,
       0,     0,     0,     0,     0,   929,   429,   790,   794,   750,
     796,     0,   666,     0,   890,     0,   889,   221,     0,   666,
     874,   873,     0,     0,   802,   805,   872,   806,   422,   257,
     259,   122,   532,   531,   423,     0,   122,   239,   131,   426,
       0,     0,     0,     0,     0,   251,   251,   137,   751,     0,
       0,     0,   975,   751,     0,   958,     0,     0,     0,     0,
       0,   749,     0,   661,     0,     0,   699,   660,   704,     0,
     698,   120,   703,   951,   988,     0,     0,     0,     0,    19,
       0,    20,     0,    98,     0,     0,     0,   109,   753,     0,
     107,   102,    99,   104,     0,   302,   310,   307,     0,     0,
     921,   926,   923,   922,   925,   924,    12,   968,   969,     0,
     751,     0,     0,     0,   883,   880,     0,   545,   920,   919,
     918,     0,   914,     0,   915,   917,     0,     5,     0,     0,
       0,   560,   561,   569,   568,     0,   438,     0,   750,   540,
     544,     0,     0,   937,     0,   522,     0,     0,   959,   799,
     281,   994,     0,     0,   814,     0,   864,   750,   954,   950,
     297,   298,   659,   752,   294,     0,   799,     0,     0,   223,
     519,   189,   497,     0,   549,   550,     0,   547,   750,   896,
       0,     0,   295,   225,     0,   223,     0,     0,   221,     0,
     875,   445,     0,     0,   811,   812,   829,   830,   859,   860,
       0,     0,     0,   778,   758,   759,   760,   767,     0,     0,
       0,   771,   769,   770,   784,   751,     0,   792,   894,   893,
       0,   223,     0,   816,   681,     0,   261,     0,     0,   128,
       0,     0,     0,     0,     0,     0,     0,   231,   232,   243,
       0,   122,   241,   157,   251,     0,   251,     0,   750,     0,
       0,     0,     0,   750,   976,   978,   957,   751,   956,     0,
     751,   725,   726,   723,   724,   757,     0,   751,   749,     0,
     543,     0,     0,   903,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   981,   177,     0,   180,   161,     0,     0,   105,
     110,   111,   103,   752,   108,     0,   306,     0,   933,   149,
     949,   970,   963,   965,   224,   226,   316,     0,     0,   881,
       0,   913,     0,    17,     0,   936,   222,   316,     0,     0,
     666,   537,     0,   671,   938,     0,   959,   526,     0,     0,
     995,     0,   286,   284,   805,   817,   951,   805,   818,   953,
       0,     0,   299,   119,     0,   799,   220,     0,   799,     0,
     496,   900,   899,     0,   295,     0,     0,     0,     0,     0,
       0,   223,   191,   682,   804,   295,     0,   763,   764,   765,
     766,   772,   773,   782,     0,   751,     0,   778,     0,   762,
     786,   750,   789,   791,   793,     0,   888,     0,   804,     0,
       0,     0,     0,   258,   533,   133,     0,   426,   231,   233,
     883,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,   989,     0,   971,     0,   974,   750,     0,     0,
       0,   701,   750,   748,     0,   739,     0,   751,     0,   705,
     740,   738,   907,     0,   751,   708,   710,   709,     0,     0,
     706,   707,   711,   713,   712,   728,   727,   730,   729,   731,
     733,   735,   734,   732,   721,   720,   715,   716,   714,   717,
     718,   719,   722,   980,     0,   122,     0,     0,   106,    21,
     308,     0,     0,     0,   967,     0,   390,   885,   883,   432,
     437,   443,     0,    15,     0,   390,   572,     0,     0,   574,
     567,   570,     0,   565,     0,   940,     0,   960,   530,     0,
     287,     0,     0,   282,     0,   301,   300,   959,     0,   316,
       0,   799,     0,   295,     0,   855,   316,   936,   316,   939,
       0,     0,     0,   446,     0,     0,   775,   750,   777,   768,
       0,   761,     0,     0,   751,   783,   892,   316,     0,   122,
       0,   254,   240,     0,     0,     0,   230,   153,   244,     0,
       0,   247,     0,   252,   253,   122,   246,   990,   972,     0,
     955,     0,   993,   756,   755,   700,     0,   750,   542,   702,
       0,   548,   750,   902,   737,     0,     0,     0,    22,    23,
     964,   961,   962,   227,     0,     0,     0,   397,   388,     0,
       0,     0,   202,   315,   317,     0,   387,     0,     0,     0,
     936,   390,     0,   916,   312,   208,   563,     0,     0,   536,
     524,     0,   290,   280,     0,   283,   289,   295,   516,   959,
     390,   959,     0,   898,     0,   854,   390,     0,   390,   941,
     316,   799,   852,   781,   780,   774,     0,   776,   750,   785,
     390,   122,   260,   129,   134,   155,   234,     0,   242,   248,
     122,   250,   973,     0,     0,   539,     0,   906,   905,   736,
     122,   181,   966,     0,     0,     0,   944,     0,     0,     0,
     228,     0,   936,     0,   353,   349,   355,   661,    31,     0,
     343,     0,   348,   352,   365,     0,   363,   368,     0,   367,
       0,   366,     0,   185,   319,     0,   321,     0,   322,   323,
       0,     0,   882,     0,   564,   562,   573,   571,   291,     0,
       0,   278,   288,     0,     0,   959,     0,   198,   516,   959,
     856,   204,   312,   210,   390,     0,     0,   788,     0,   206,
     256,     0,     0,   122,   237,   154,   249,   992,   754,     0,
       0,     0,     0,     0,   415,     0,   945,     0,   333,   337,
     412,   413,   347,     0,     0,     0,   328,   625,   624,   621,
     623,   622,   642,   644,   643,   613,   583,   585,   584,   603,
     619,   618,   579,   590,   591,   593,   592,   612,   596,   594,
     595,   597,   598,   599,   600,   601,   602,   604,   605,   606,
     607,   608,   609,   611,   610,   580,   581,   582,   586,   587,
     589,   627,   628,   637,   636,   635,   634,   633,   632,   620,
     639,   629,   630,   631,   614,   615,   616,   617,   640,   641,
     645,   647,   646,   648,   649,   626,   651,   650,   653,   655,
     654,   588,   658,   656,   657,   652,   638,   578,   360,   575,
       0,   329,   381,   382,   380,   373,     0,   374,   330,   407,
       0,     0,     0,     0,   411,     0,   185,   194,   311,     0,
       0,     0,   279,   293,   853,     0,     0,   383,   122,   188,
     959,     0,     0,   200,   959,   779,     0,   122,   235,   135,
     156,     0,   538,   904,   179,   331,   332,   410,   229,     0,
     751,   751,     0,   356,   344,     0,     0,     0,   362,   364,
       0,     0,   369,   376,   377,   375,     0,     0,   318,   946,
       0,     0,     0,   414,     0,   313,     0,   292,     0,   558,
     753,   122,     0,     0,   190,   196,     0,   787,     0,     0,
     158,   334,   112,     0,   335,   336,     0,   750,     0,   750,
     358,   354,   359,   576,   577,     0,   345,   378,   379,   371,
     372,   370,   408,   405,   970,   324,   320,   409,     0,   314,
     559,   752,     0,     0,   384,   122,   192,     0,   238,     0,
     183,     0,   390,     0,   350,   357,   361,     0,     0,   799,
     326,     0,   556,   515,   518,     0,   236,     0,     0,   159,
     341,     0,   389,   351,   406,   947,     0,   753,   401,   799,
     557,   520,     0,   182,     0,     0,   340,   959,   799,   265,
     402,   403,   404,   995,   400,     0,     0,     0,   339,     0,
     401,     0,   959,     0,   338,   385,   122,   325,   995,     0,
     270,   268,     0,   122,     0,     0,   271,     0,     0,   266,
     327,     0,   386,     0,   274,   264,     0,   267,   273,   178,
     275,     0,     0,   262,   272,     0,   263,   277,   276
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   109,   877,   615,   175,  1436,   712,
     342,   343,   344,   345,   838,   839,   840,   111,   112,   113,
     114,   115,   396,   648,   649,   536,   245,  1501,   542,  1417,
    1502,  1740,   827,   337,   564,  1700,  1056,  1235,  1759,   412,
     176,   650,   917,  1119,  1292,   119,   618,   934,   651,   670,
     938,   598,   933,   225,   517,   652,   619,   935,   414,   362,
     379,   122,   919,   880,   863,  1074,  1439,  1172,   987,  1649,
    1505,   788,   993,   541,   797,   995,  1325,   780,   976,   979,
    1161,  1766,  1767,   638,   639,   664,   665,   349,   350,   356,
    1473,  1628,  1629,  1246,  1363,  1462,  1622,  1749,  1769,  1659,
    1704,  1705,  1706,  1449,  1450,  1451,  1452,  1661,  1662,  1668,
    1716,  1455,  1456,  1460,  1615,  1616,  1617,  1639,  1797,  1364,
    1365,   177,   124,  1783,  1784,  1620,  1367,  1368,  1369,  1370,
     125,   238,   537,   538,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,  1485,   136,   916,  1118,   137,   635,
     636,   637,   242,   388,   532,   625,   626,  1197,   627,  1198,
     138,   139,   140,   818,   141,   142,  1690,   143,   620,  1475,
     621,  1088,   885,  1263,  1260,  1608,  1609,   144,   145,   146,
     228,   147,   229,   239,   399,   524,   148,  1015,   822,   149,
    1016,   908,   575,  1017,   962,  1141,   963,  1143,  1144,  1145,
     965,  1303,  1304,   966,   758,   507,   189,   190,   653,   641,
     488,  1104,  1105,   744,   745,   904,   151,   231,   152,   153,
     179,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     704,   164,   235,   236,   601,   218,   219,   707,   708,  1203,
    1204,   372,   373,   871,   165,   589,   166,   634,   167,   328,
    1630,  1680,   363,   407,   659,   660,  1009,  1099,  1244,   859,
     860,   861,   802,   803,   804,   329,   330,   824,  1438,   902
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1486
static const yytype_int16 yypact[] =
{
   -1486,   166, -1486, -1486,  5338, 13021, 13021,   -11, 13021, 13021,
   13021, 10854, 13021, 13021, -1486, 13021, 13021, 13021, 13021, 13021,
   13021, 13021, 13021, 13021, 13021, 13021, 13021, 16411, 16411, 11051,
   13021,  3944,    -8,   178, -1486, -1486, -1486, -1486, -1486,   177,
   -1486, -1486, -1486,   122, 13021, -1486,   178,   198,   223,   226,
   -1486,   178, 11248,  1058, 11445, -1486, 14046,  9869,    10, 13021,
    1157,    60, -1486, -1486, -1486,    78,   267,    58,   228,   230,
     310,   327, -1486,  1058,   329,   344, -1486, -1486, -1486, -1486,
   -1486, 13021,   499,  1821, -1486, -1486,  1058, -1486, -1486, -1486,
   -1486,  1058, -1486,  1058, -1486,    45,   360,  1058,  1058, -1486,
     311, -1486, 11642, -1486, -1486,   304,    68,   494,   494, -1486,
     206,   402,   517,   376, -1486,    83, -1486,   543, -1486, -1486,
   -1486, -1486,   579,   479, -1486, -1486,   428,   437,   443,   451,
     458,   465,   471,   514, 13005, -1486, -1486, -1486, -1486,   156,
   -1486,   561,   647, -1486,    57,   531, -1486,   581,   -13, -1486,
    2247,   154, -1486, -1486,  3541,   124,   550,   160, -1486,   128,
      52,   553,    84, -1486,   298, -1486,   680, -1486, -1486, -1486,
     592,   575,   594, -1486, 13021, -1486,   543,   479, 16885,  3575,
   16885, 13021, 16885, 16885, 13416,   578, 15567, 13416, 16885,   729,
    1058,   714,   714,   338,   714,   714,   714,   714,   714,   714,
     714,   714,   714, -1486, -1486, -1486,   265, 13021,   609, -1486,
   -1486,   636,   603,    53,   604,    53, 16411, 15674,   598,   789,
   -1486,   592, -1486, 13021,   609, -1486,   644, -1486,   646,   612,
   -1486,   144, -1486, -1486, -1486,    53,   124, 11839, -1486, -1486,
   13021,  4657,   796,    91, 16885,  9475, -1486, 13021, 13021,  1058,
   -1486, -1486, 13202,   614, -1486, 14724, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486,  2899, -1486,  2899, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486,    80,    89,   594, -1486, -1486,
   -1486, -1486,   615,  1145,    90, -1486, -1486,   652,   795, -1486,
     655, 14987, -1486,   621,   630, 14772, -1486,    50, 14820,   899,
     899,  1058,   633,   826,   651, -1486,    82, -1486, 16019,    92,
   -1486,   706, -1486,   721, -1486,   838,    96, 16411, 13021, 13021,
     662,   681, -1486, -1486, 16117, 11051,    98,   478,   379, -1486,
   13218, 16411,   549, -1486,  1058, -1486,   394,   402, -1486, -1486,
   -1486, -1486, 16506,   842,   756, -1486, -1486, -1486,   103, 13021,
     674,   675, 16885,   676,  1327,   679,  5535, 13021,   422,   682,
     607,   422,   439,   426, -1486,  1058,  2899,   685, 10066, 14046,
   -1486, -1486,  1125, -1486, -1486, -1486, -1486, -1486,   543, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, 13021, 13021,
   13021, 13021, 12036, 13021, 13021, 13021, 13021, 13021, 13021, 13021,
   13021, 13021, 13021, 13021, 13021, 13021, 13021, 13021, 13021, 13021,
   13021, 13021, 13021, 13021, 13021, 13021, 16601, 13021, -1486, 13021,
   13021, 13021, 13415,  1058,  1058,  1058,  1058,  1058,   579,   768,
     809,  9672, 13021, 13021, 13021, 13021, 13021, 13021, 13021, 13021,
   13021, 13021, 13021, 13021, -1486, -1486, -1486, -1486,  1339, 13021,
   13021, -1486, 10066, 10066, 13021, 13021,   304,   150, 16117,   690,
     543, 12233, 14884, -1486, 13021, -1486,   691,   875,   733,   695,
     698, 13557,    53, 12430, -1486, 12627, -1486,   612,   701,   707,
    2188, -1486,   307, 10066, -1486,  1656, -1486, -1486, 14932, -1486,
   -1486, 10263, -1486, 13021, -1486,   803,  8687,   892,   710, 16764,
     889,    73,    56, -1486, -1486, -1486,   731, -1486, -1486, -1486,
    2899,  1171,   719,   901, 15491,  1058, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486,   722, -1486, -1486,   727,   723,
     742,   732,   102,  1639,  1357, -1486, -1486,  1058,  1058, 13021,
      53,    60, -1486, -1486, -1486, 15491,   831, -1486,    53,   123,
     129,   735,   745,  2844,    63,   749,   750,   486,   813,   751,
      53,   135,   757, -1486,   788,  1058, -1486, -1486,   882,  2768,
      25, -1486, -1486, -1486,   402, -1486, -1486, -1486,   925,   827,
     784,   326,   806, 13021,   304,   828,   953,   776,   817, -1486,
     150, -1486,  2899,  2899,   959,   796,   103, -1486,   791,   967,
   -1486,  2899,    93, -1486,   433,   174, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486,   709,  3080, -1486, -1486, -1486, -1486,   976,
     810, -1486, 16411, 13021,   797,   977, 16885,   979, -1486, -1486,
     858,  1525, 11430, 17023, 13416, 13874, 13021, 16837, 14045, 12212,
    4541, 12604, 12012, 13550, 14210, 14210, 14210, 14210,  2592,  2592,
    2592,  2592,  2592,  1540,  1540,   807,   807,   807,   338,   338,
     338, -1486,   714, 16885,   800,   804, 15722,   814,   985,     5,
   13021,   172,   609,     6,   150, -1486, -1486, -1486,   989,   756,
   -1486,   543, 16215, -1486, -1486, -1486, 13416, 13416, 13416, 13416,
   13416, 13416, 13416, 13416, 13416, 13416, 13416, 13416, 13416, -1486,
   13021,   197,   159, -1486, -1486,   609,   330,   812,  3458,   819,
     821,   818,  3806,   137,   832, -1486, 16885,  1581, -1486,  1058,
   -1486,    93,   421, 16411, 16885, 16411, 15778,   858,    93,    53,
     163,   849,   825, 13021, -1486,   164, -1486, -1486, -1486,  8490,
     519, -1486, -1486, 16885, 16885,   178, -1486, -1486, -1486, 13021,
     915, 15373, 15491,  1058,  8884,   833,   840, -1486,  1024,   944,
     902,   886, -1486,  1033,   852,  1753,  2899, 15491, 15491, 15491,
   15491, 15491,   856,   894,   860, 15491,   370,   897, -1486,   861,
   -1486, 16979, -1486,    13, -1486,  5732,  1950,   863,  1357, -1486,
    1357, -1486,  1058,  1058,  1357,  1357,  1058, -1486,  1048,   865,
   -1486,   392, -1486, -1486,  3932, -1486, 16979,  1049, 16411,   876,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,   890,
    1061,  1058,  1950,   883, 16117, 16313,  1063, -1486, -1486, -1486,
   -1486,   881, -1486, 13021, -1486, -1486,  4927, -1486,  2899,  1950,
     888, -1486, -1486, -1486, -1486,  1065,   891, 13021, 16506, -1486,
   -1486, 13415,   893, -1486,  2899, -1486,   895,  5929,  1052,    71,
   -1486, -1486,    79,  1339, -1486,  1656, -1486,  2899, -1486, -1486,
      53, 16885, -1486, 10460, -1486, 15491,    61,   900,  1950,   827,
   -1486, -1486, 14045, 13021, -1486, -1486, 13021, -1486, 13021, -1486,
    4141,   904, 10066,   813,  1055,   827,  2899,  1081,   858,  1058,
   16601,    53,  4237,   908, -1486, -1486,   176,   909, -1486, -1486,
    1090,  1882,  1882,  1581, -1486, -1486, -1486,  1053,   917,   269,
     924, -1486, -1486, -1486, -1486,  1109,   931,   691,    53,    53,
   12824,   827,  1656, -1486, -1486,  4858,   530,   178,  9475, -1486,
    6126,   932,  6323,   934, 15373, 16411,   937,   992,    53, 16979,
    1115, -1486, -1486, -1486, -1486,   509, -1486,    66,  2899,   955,
     998,  2899,  1058,  1171, -1486, -1486, -1486,  1132, -1486,   962,
     976,   324,   324,  1088,  1088, 15881,   970,  1141, 15491, 15271,
   16506,  4333, 15129, 15491, 15491, 15491, 15491,  3590, 15491, 15491,
   15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491,
   15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491, 15491,
   15491,  1058, -1486, -1486,  1086, -1486, -1486,   984,   986, -1486,
   -1486, -1486,   393,  1639, -1486,   978, -1486, 15491,    53, -1486,
   -1486,    86, -1486,   586,  1162, -1486, -1486,   139,   983,    53,
   10657, -1486,  2495, -1486,  5141,   756,  1162, -1486,   366,    17,
   -1486, 16885,  1043,   991, -1486,   990,  1052, -1486,  2899,   796,
    2899,    48,  1175,  1112,   170, -1486,   609,   181, -1486, -1486,
   16411, 13021, 16885, 16979,  1005,    61, -1486,  1004,    61,  1008,
   14045, 16885, 15826,  1010, 10066,  1012,  1014,  2899,  1015,  1009,
    2899,   827, -1486,   612,   331, 10066, 13021, -1486, -1486, -1486,
   -1486, -1486, -1486,  1069,  1013,  1202,  1122,  1581,  1064, -1486,
   16506,  1581, -1486, -1486, -1486, 16411, 16885,  1022, -1486,   178,
    1186,  1142,  9475, -1486, -1486, -1486,  1030, 13021,   992,    53,
   16117, 15373,  1038, 15491,  6520,   535,  1040, 13021,    74,   264,
   -1486,  1056, -1486,  2899, -1486,  1100, -1486,  2321,  1205,  1051,
   15491, -1486, 15491, -1486,  1059, -1486,  1103,  1234,  1060, -1486,
   -1486, -1486, 15929,  1050,  1237,  4403, 10046, 10440, 15491, 16933,
   12409,  2802,  3411, 12800, 14382, 14551, 14551, 14551, 14551,  2672,
    2672,  2672,  2672,  2672,  1967,  1967,   324,   324,   324,  1088,
    1088,  1088,  1088, -1486,  1067, -1486,  1057,  1062, -1486, -1486,
   16979,  1058,  2899,  2899, -1486,  1950,   725, -1486, 16117, -1486,
   -1486, 13416,  1071, -1486,  1054,  1023, -1486,   146, 13021, -1486,
   -1486, -1486, 13021, -1486, 13021, -1486,   796, -1486, -1486,    88,
    1241,  1183, 13021, -1486,  1080,    53, 16885,  1052,  1084, -1486,
    1085,    61, 13021, 10066,  1091, -1486, -1486,   756, -1486, -1486,
    1079,  1092,  1082, -1486,  1095,  1581, -1486,  1581, -1486, -1486,
    1102, -1486,  1158,  1104,  1272, -1486,    53, -1486,  1265, -1486,
    1108, -1486, -1486,  1111,  1118,   140, -1486, -1486, 16979,  1121,
    1126, -1486, 11823, -1486, -1486, -1486, -1486, -1486, -1486,  2899,
   -1486,  2899, -1486, 16979, 15984, -1486, 15491, 16506, -1486, -1486,
   15491, -1486, 15491, -1486, 11031, 15491,  1135,  6717, -1486, -1486,
     586, -1486, -1486, -1486,   563, 14217,  1950,  1217, -1486,   613,
    1167,  1289, -1486, -1486, -1486,   768,  3194,   100,   107,  1140,
     756,   809,   141, -1486, -1486, -1486,  1179, 10838, 11232, 16885,
   -1486,    62,  1340,  1273, 13021, -1486, 16885, 10066,  1238,  1052,
    1942,  1052,  1166, 16885,  1169, -1486,  2029,  1165,  2076, -1486,
   -1486,    61, -1486, -1486,  1225, -1486,  1581, -1486, 16506, -1486,
    2110, -1486,  8490, -1486, -1486, -1486, -1486,  9081, -1486, -1486,
   -1486,  8490, -1486,  1174, 15491, 16979,  1230, 16979, 16032, 11031,
   -1486, -1486, -1486,  1950,  1950,  1058, -1486,  1353, 15271,    87,
   -1486, 14217,   756,  2005, -1486,  1195, -1486,   109,  1180,   110,
   -1486, 14560, -1486, -1486, -1486,   112, -1486, -1486,  1414, -1486,
    1184, -1486,  1292,   543, -1486, 14389, -1486, 14389, -1486, -1486,
    1358,   768, -1486, 13704, -1486, -1486, -1486, -1486,  1363,  1295,
   13021, -1486, 16885,  1189,  1194,  1052,   506, -1486,  1238,  1052,
   -1486, -1486, -1486, -1486,  2170,  1196,  1581, -1486,  1249, -1486,
    8490,  9278,  9081, -1486, -1486, -1486,  8490, -1486, 16979, 15491,
   15491,  6914,  1199,  1201, -1486, 15491, -1486,  1950, -1486, -1486,
   -1486, -1486, -1486,  2899,  2514,   613, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486, -1486, -1486,   161, -1486,
    1167, -1486, -1486, -1486, -1486, -1486,   104,   677, -1486,  1374,
     115, 14987,  1292,  1382, -1486,  2899,   543, -1486, -1486,  1207,
    1386, 13021, -1486, 16885, -1486,   126,  1208, -1486, -1486, -1486,
    1052,   506, 13875, -1486,  1052, -1486,  1581, -1486, -1486, -1486,
   -1486,  7111, 16979, 16979, -1486, -1486, -1486, 16979, -1486,   993,
    1393,  1395,  1212, -1486, -1486, 15491, 14560, 14560,  1348, -1486,
    1414,  1414,   678, -1486, -1486, -1486, 15491,  1328, -1486,  1239,
    1220,   116, 15491, -1486,  1058, -1486, 15491, 16885,  1330, -1486,
    1409, -1486,  7308,  1231, -1486, -1486,   506, -1486,  7505,  1233,
    1311, -1486,  1325,  1274, -1486, -1486,  1329,  2899,  1256,  2514,
   -1486, -1486, 16979, -1486, -1486,  1267, -1486,  1400, -1486, -1486,
   -1486, -1486, 16979,  1425,   486, -1486, -1486, 16979,  1251, 16979,
   -1486,   132,  1257,  7702, -1486, -1486, -1486,  1258, -1486,  1259,
    1284,  1058,   809,  1282, -1486, -1486, -1486, 15491,  1288,   106,
   -1486,  1385, -1486, -1486, -1486,  7899, -1486,  1950,   863, -1486,
    1300,  1058,   635, -1486, 16979, -1486,  1280,  1463,   565,   106,
   -1486, -1486,  1390, -1486,  1950,  1286, -1486,  1052,   125, -1486,
   -1486, -1486, -1486,  2899, -1486,  1285,  1293,   121, -1486,   520,
     565,   142,  1052,  1287, -1486, -1486, -1486, -1486,  2899,   246,
    1472,  1406,   520, -1486,  8096,   153,  1477,  1410, 13021, -1486,
   -1486,  8293, -1486,   253,  1482,  1415, 13021, -1486, 16885, -1486,
    1483,  1417, 13021, -1486, 16885, 13021, -1486, 16885, 16885
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1486, -1486, -1486,  -542, -1486, -1486, -1486,   212,     2,   -33,
     382, -1486,  -268,  -494, -1486, -1486,   440,   -16,  1584, -1486,
    2890, -1486,   454, -1486,    29, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486,  -250, -1486, -1486,  -149,
     195,    24, -1486, -1486, -1486, -1486, -1486, -1486,    28, -1486,
   -1486, -1486, -1486, -1486, -1486,    37, -1486, -1486,  1042,  1066,
    1046,   -99,  -680,  -842,   580,   642,  -249,   358,  -898, -1486,
      26, -1486, -1486, -1486, -1486,  -718,   205, -1486, -1486, -1486,
   -1486,  -234, -1486,  -577, -1486,  -409, -1486, -1486,   956, -1486,
      44, -1486, -1486,  -992, -1486, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486,    19, -1486,    95, -1486, -1486, -1486,
   -1486, -1486,   -70, -1486,   182,  -938, -1486, -1485,  -257, -1486,
    -141,   143,  -121,  -244, -1486,   -71, -1486, -1486, -1486,   200,
     -17,    15,    47,  -687,   -57, -1486, -1486,    11, -1486,   -20,
   -1486, -1486,    -5,   -40,    64, -1486, -1486, -1486, -1486, -1486,
   -1486, -1486, -1486, -1486,  -575,  -823, -1486, -1486, -1486, -1486,
   -1486,  1073, -1486, -1486, -1486, -1486, -1486,   466, -1486, -1486,
   -1486, -1486, -1486, -1486, -1486, -1486,  -885, -1486,  2380,     3,
   -1486,   599,  -393, -1486, -1486,  -467,  3451,  1860, -1486, -1486,
     540,  -111,  -601, -1486, -1486,   610,   415,  -688,   416, -1486,
   -1486, -1486, -1486, -1486,   601, -1486, -1486, -1486,    18,  -853,
    -100,  -414,  -413, -1486,   663,  -113, -1486, -1486,     4,     9,
     618, -1486, -1486,  1110,   -21, -1486,  -353,   108,   148, -1486,
     -85, -1486, -1486, -1486,  -445,  1200, -1486, -1486, -1486, -1486,
   -1486,   656,   628, -1486, -1486, -1486,  -339,  -604, -1486,  1160,
   -1055, -1486,   -67,  -167,   -18,   765, -1486, -1006,   222,  -142,
   -1486,   504,   582, -1486, -1486, -1486, -1486,   536,   305, -1064
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -980
static const yytype_int16 yytable[] =
{
     178,   180,   419,   182,   183,   184,   186,   187,   188,   469,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   150,   326,   217,   220,   380,   499,   118,   629,
     383,   384,   120,   116,   227,   232,  1269,   334,   900,   244,
     233,   121,   491,   631,   346,   391,  1100,   252,   241,   255,
     767,   468,   335,   753,   338,   415,   419,   521,   325,   896,
     895,   246,   393,   701,   914,  1092,   250,   376,   333,   964,
     377,   569,   571,   876,   742,   743,   244,  1117,   997,   837,
     842,   395,   793,   749,   750,   390,  1168,   971,   795,   -68,
    1266,   243,   409,  1128,   -68,  1255,  1517,   392,   -33,   -32,
     533,   581,   983,   -33,   -32,   586,  1323,   533,   516,  1465,
      14,   775,   776,  1670,   772,   937,  1467,  1101,  -346,  1525,
      14,  1610,  -834,   526,  1677,  1677,  1381,  -832,   366,  1157,
    1517,  1270,   848,  -553,   393,   525,   857,   858,   533,   234,
    1671,   355,  -663,   347,   865,  1478,   865,   123,   865,   865,
     865,   367,    14,   395,   494,    14,  1694,   390,   398,   486,
     487,  -671,  1102,   489,  1688,   565,     3,   510,  1051,   392,
    1751,  1382,  1177,  1178,    14,  1665,   502,  -844,  1261,   181,
    1799,   -96,   237,   519,   577,  -527,   395,   486,   487,   406,
     509,  1813,  -554,  1666,   489,   -96,   932,  1196,  -833,   117,
    1376,   351,   392,   336,   832,   381,   518,  -670,   352,  1689,
    1262,  1736,  1667,  -664,  -876,  1752,   110,  1271,   392,   875,
     494,   370,   371,   566,  -835,  1800,  -551,   486,   487,  -836,
    -553,  1479,   528,  -879,  -878,   528,  1814,  1064,   404,  -665,
    -819,   578,   244,   539,  -839,  -834,  -838,  -842,  1103,   796,
    -832,  -820,  -285,   493,   854,   348,   550,  1383,  1131,  -752,
    1180,   833,  -752,  -750,   794,   253,  1324,   605,   324,   671,
     530,  1388,   -68,  1316,   535,   410,  1175,   495,  1179,  1518,
    1519,   -33,   -32,   534,   582,   361,   490,  1390,   587,  1291,
     603,  -285,  1466,   470,  1396,  1672,  1398,  -269,   560,  1468,
    1437,  -346,  1526,   378,  1611,   361,   381,  1678,  1726,   361,
     361,  1801,   497,  1794,   849,  1410,  -752,   490,   418,   592,
     850,  -833,  1815,   346,   346,   572,   866,  1302,   950,  1806,
    1247,  1416,  1472,   325,   361,  1084,  1820,  -876,   595,  1114,
    1060,  1061,   591,   495,  -841,   669,  -843,  -835,   -95,   419,
     493,   754,  -836,   244,   392,   508,  -879,  -878,   614,  1148,
     217,   331,   -95,  -819,  -845,   609,  -848,  -839,   240,  -838,
    1177,  1178,   500,  -807,  -820,   705,   326,  1047,  1048,  1049,
     882,   203,    40,  1486,   186,  1488,  1520,  -807,   247,   353,
     203,    40,   654,  1050,   385,   456,   760,   354,   380,   718,
     719,   415,   506,   666,   747,   590,   640,   457,  1494,   751,
    1623,   325,  1624,   248,   723,  1807,   249,   369,   357,  1077,
     358,  1149,  1821,   672,   673,   674,   675,   677,   678,   679,
     680,   681,   682,   683,   684,   685,   686,   687,   688,   689,
     690,   691,   692,   693,   694,   695,   696,   697,   698,   699,
     700,   725,   702,   110,   703,   703,   706,   110,  1326,   227,
     232,   540,   367,  1305,   711,   233,   726,   727,   728,   729,
     730,   731,   732,   733,   734,   735,   736,   737,   738,  1636,
    1313,  1254,   724,  1641,   703,   748,   883,   666,   666,   703,
     752,  1107,  1108,   496,   577,   832,   726,   397,   386,   756,
     359,   884,   105,  -555,   387,   367,  -810,  -808,   764,   367,
     766,   782,   469,   325,  1426,   889,   611,   360,   666,   364,
    -810,  -808,   367,  1125,  1268,  -951,   783,  1256,   784,   611,
     630,   606,   370,   371,   365,   629,   857,   858,  1278,   123,
    1257,  1280,   903,   559,   905,   931,   406,  1133,   604,   631,
     382,   405,   405,   405,   468,   486,   487,   841,   841,  1258,
    1057,   405,  1058,  -951,   234,   787,   408,   486,   487,   837,
     713,   548,   939,   549,   844,   370,   371,   367,   943,   370,
     371,   411,   367,   886,   401,  1498,   616,   617,  -666,   368,
      55,   117,   370,   371,   977,   978,   746,   929,    62,    63,
      64,   168,   169,   416,   921,  1159,  1160,  1403,   110,  1404,
     714,  -846,   486,   487,  1176,  1177,  1178,   658,   392,   713,
     420,   324,   154,  -846,   361,   657,   211,   211,   553,   421,
     771,   459,   367,   777,  1693,   422,   714,   656,  1696,   611,
    1320,  1177,  1178,   423,  1443,   213,   215,   370,   371,   521,
     424,   369,   370,   371,   640,   973,  1052,   425,   911,   714,
      37,   903,   905,   426,   721,   417,  1293,   629,   972,   905,
     714,   922,  -951,   714,   559,   361,   716,   361,   361,   361,
     361,   631,    50,  1397,  1780,  1781,  1782,   999,  1242,  1243,
     367,  1380,  1004,   406,    37,  1433,  1434,   611,  1637,  1638,
     741,   612,   370,   371,  1392,   930,   427,  -951,  1673,  1719,
    -951,   661,  1795,  1796,   331,  1284,    50,   460,  1497,  1791,
     394,   568,   570,   559,   461,  1315,  1294,  1674,  1720,  1354,
    1675,  1721,  1717,  1718,  1805,   942,   462,   774,  1444,   413,
     492,    88,    89,  -840,    90,   173,    92,  -664,   110,  1072,
    -552,  1445,  1446,   374,    62,    63,    64,   168,   169,   416,
     370,   371,   400,   402,   403,   498,  1470,   823,   975,   172,
     503,  1789,    86,  1447,    14,    88,    89,   505,    90,  1448,
      92,  1713,  1714,   457,   244,   406,  1802,  1007,  1010,   843,
     658,   511,   394,  -844,   493,   514,   470,   629,   515,  -662,
     981,   522,   523,  1372,   531,   551,   544,  -979,  1645,   554,
     555,   631,   841,   561,   841,   211,   870,   872,   841,   841,
    1062,   417,   562,   992,  1495,   394,   573,  1776,    62,    63,
      64,    65,    66,   416,   512,   574,   583,  1355,  1521,    72,
     463,   520,  1356,   576,    62,    63,    64,   168,  1357,   416,
    1358,   584,   585,   596,  1152,   798,   632,   597,   633,   154,
     453,   454,   455,   154,   456,   642,   643,   644,  1082,    37,
     646,   868,  1132,   869,  1394,  -117,   457,   465,   655,    55,
     668,   757,  1091,   361,   759,   606,   761,  1359,  1360,   762,
    1361,    50,   768,   711,   150,   417,  1768,   785,   769,  1189,
     118,   533,   789,   792,   120,   116,  1193,   550,  1112,   805,
     806,   417,   826,   121,   847,   829,  1768,   640,  1120,  1362,
     828,  1121,   123,  1122,   831,  1790,   851,   666,    62,    63,
      64,   168,   169,   416,   640,   830,   852,   892,   893,  1274,
     855,   864,   856,   227,   232,   862,   901,   211,   867,   233,
      88,    89,   873,    90,   173,    92,   211,   878,  1697,   881,
     879,  -686,   888,   211,   887,  1156,   580,   890,   123,   961,
     211,   967,   891,   894,   117,   588,   899,   593,  1483,   898,
      37,   628,   600,   779,   629,   907,   913,   909,   912,   610,
     918,   110,  1162,   915,   928,   417,   607,   924,   631,  1093,
     613,   925,    50,   936,  -668,   990,   110,  1163,   927,   944,
     946,   746,   947,   777,   154,   948,   974,   984,   825,   123,
     117,  1195,   920,  1249,  1201,   994,   607,  1354,   613,   607,
     613,   613,   996,   998,  1298,  1000,   630,   110,  1001,   714,
     123,  1002,  1003,  1005,  1059,   658,  1018,   841,   234,  1019,
    1020,   714,  1022,   714,  1023,   629,  1055,  1063,   341,  1065,
    1250,    88,    89,  1067,    90,   173,    92,  1070,  1069,   631,
    1071,   117,    14,  1073,    37,  1251,  1076,  1080,  1081,  1089,
     777,  1087,  1090,  1096,  1098,  1094,  1338,  1127,   110,  1732,
    1115,   897,   117,  1343,  1124,  1130,    50,   211,  1135,  -847,
     209,   209,   150,   559,  1136,  1146,  1276,  1147,   118,   110,
     661,   661,   120,   116,  1150,   741,   600,   774,  1151,   666,
     714,   121,  1153,   123,  1165,   123,  1167,  1170,  1171,  1173,
     666,  1251,  1182,   640,  1183,  1355,   640,   214,   214,    37,
    1356,  1187,    62,    63,    64,   168,  1357,   416,  1358,   172,
    1192,   361,    86,  1188,   154,    88,    89,  1050,    90,   173,
      92,    50,   244,  1140,  1140,   961,  1779,  1191,   630,  1234,
    1239,  1245,  1322,  1248,  1308,   117,   266,   117,  1236,  1264,
    1237,   932,  1265,  1085,   774,  1359,  1360,  1701,  1361,  1272,
     110,  1311,   110,  1409,   110,  1273,  1277,  1279,  1281,  1095,
    1283,  1289,   799,  1285,   268,  1295,    37,  1286,  1288,   417,
    1296,  1297,  1109,   957,  1185,  1307,  1301,  1375,  1309,  1310,
      88,    89,  1312,    90,   173,    92,    37,   123,    50,  1317,
    1471,   559,  1321,  1327,   559,   980,  1329,  1331,    37,  1336,
     982,  1129,  1332,  1337,  1341,   419,  1342,  1374,    50,  1348,
    1335,  1339,    37,  1377,  1349,  1384,   552,  1378,  1346,  1379,
      50,   211,   800,   823,  1371,  1373,  1385,  1386,   339,   340,
    1387,  1399,  1401,  1371,    50,  1389,  1391,  1393,   666,   117,
     910,  1408,  1395,   546,   547,  1400,  1402,    88,    89,   209,
      90,   173,    92,  1405,  1406,  1407,   110,  1411,   630,   640,
    1413,   172,  1414,  1181,    86,   318,  1184,    88,    89,  1415,
      90,   173,    92,  1418,  1621,   668,   341,   123,  1419,    88,
      89,   211,    90,   173,    92,   322,   214,   172,  1430,  1441,
      86,  1454,  1469,    88,    89,   323,    90,   173,    92,  1474,
     941,   501,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,  1480,  1484,  1481,  1489,  1492,   961,
    1490,  1496,   211,   961,   211,  1507,  1509,  1515,  1514,   117,
      37,  1523,  1625,  1524,   110,  1619,  1618,  1631,  1632,  1482,
    1634,   968,   666,   969,  1635,  1646,   110,  1644,  1676,  1366,
     211,  1655,    50,  1656,   484,   485,  1682,   154,  1366,  1685,
    1686,  1691,  1707,  1267,  1709,   901,  1711,  1715,  1371,   988,
    1725,  1723,   154,  1730,  1371,  1724,  1371,  1457,  1731,   640,
      37,   209,   203,    40,  1735,  1738,  1739,  -342,  1371,  1741,
     209,  1742,  1287,  1744,  1746,  1290,  1671,   209,    37,  1747,
    1516,  1750,    50,   154,   209,  1174,  1504,   211,  1753,  1757,
    1756,    88,    89,  1350,    90,   173,    92,  1758,   214,  1763,
      50,   486,   487,   211,   211,  1765,  1068,   214,  1770,   594,
    1774,  1777,  1778,  1786,   214,  1633,  1792,  1684,  1788,  1458,
    1803,   214,   600,  1079,  1793,   630,  1808,   628,  1328,  1809,
     123,  1816,  1109,  1817,   154,    37,  1822,  1825,  1823,   739,
    1826,    88,    89,  1238,    90,   173,    92,   961,  1773,   961,
     720,   717,  1371,  1126,   470,   154,   836,    50,   645,    88,
      89,  1086,    90,   173,    92,  1787,  1314,  1420,  1650,   715,
    1648,  1504,   740,  1366,   105,  1785,  1642,   845,  1522,  1366,
    1669,  1366,   117,  1461,  1664,  1810,  1798,  1351,  1352,  1708,
    1710,  1681,  1640,  1366,  1259,   123,   630,  1442,  1194,   110,
    1463,  1299,  1142,  1300,   123,   602,  1106,   324,  1154,   667,
    1008,   209,  1432,  1459,  1612,  1241,    88,    89,  1613,    90,
     173,    92,  1748,   211,   211,  1186,     0,  1233,  1679,     0,
     450,   451,   452,   453,   454,   455,   154,   456,   154,     0,
     154,  1761,   988,  1169,  1458,     0,    37,   117,   214,   457,
       0,   208,   208,     0,     0,   224,   117,  1728,   961,   628,
       0,     0,     0,   325,   110,     0,  1687,     0,    50,   110,
       0,   951,   952,   110,  1422,     0,  1423,  1366,     0,     0,
     224,   419,     0,   123,     0,     0,     0,   361,     0,   123,
     559,   953,     0,   324,   123,     0,     0,     0,     0,   954,
     955,   956,    37,  1607,     0,     0,  1626,     0,     0,     0,
    1614,  1464,   957,     0,     0,     0,     0,   324,     0,   324,
       0,     0,     0,     0,    50,   324,     0,    88,    89,  1347,
      90,   173,    92,     0,     0,   117,     0,     0,     0,     0,
       0,   117,   154,     0,     0,     0,   117,     0,   961,   211,
       0,     0,   110,   110,   110,   920,     0,     0,   110,   958,
      37,     0,     0,   110,     0,     0,     0,     0,  1275,     0,
       0,     0,   959,     0,     0,   209,     0,    37,     0,   203,
      40,     0,    50,    88,    89,     0,    90,   173,    92,   628,
     834,   835,     0,     0,   211,     0,     0,     0,     0,    50,
       0,   960,     0,  1412,     0,     0,     0,   640,     0,   211,
     211,     0,   214,  1306,     0,     0,     0,     0,     0,  1421,
     154,     0,     0,     0,   266,     0,     0,   640,   600,   988,
       0,     0,   154,     0,   123,   209,   640,     0,   836,     0,
     208,    88,    89,  1818,    90,   173,    92,     0,     0,     0,
       0,  1824,   268,     0,     0,     0,   739,  1827,    88,    89,
    1828,    90,   173,    92,     0,     0,     0,     0,  1660,     0,
       0,     0,   214,   559,    37,   123,   209,     0,   209,     0,
       0,   123,     0,     0,     0,     0,   117,   211,     0,   773,
     224,   105,   224,     0,   324,     0,    50,     0,   961,     0,
       0,     0,     0,   110,   209,  1500,   600,     0,     0,     0,
       0,  1702,     0,   214,  1506,   214,   123,     0,  1607,  1607,
       0,     0,  1614,  1614,  1511,  1762,     0,   117,     0,     0,
       0,   546,   547,   117,     0,     0,   361,     0,   123,     0,
       0,   214,    37,     0,   110,     0,     0,   224,     0,   172,
     110,     0,    86,   318,     0,    88,    89,     0,    90,   173,
      92,   209,  1006,     0,    50,     0,     0,     0,   117,     0,
    1683,     0,   208,   322,     0,     0,   628,   209,   209,     0,
       0,   208,     0,   323,     0,   110,  1354,   123,   208,     0,
     117,     0,     0,  1760,   123,   208,     0,  1651,   214,     0,
    1137,  1138,  1139,    37,     0,   154,   224,   110,     0,     0,
       0,     0,     0,  1775,   214,   214,     0,     0,     0,     0,
     374,     0,     0,    88,    89,    50,    90,   173,    92,     0,
     224,    14,     0,   224,     0,     0,     0,     0,     0,   117,
       0,     0,     0,     0,     0,     0,   117,   628,     0,     0,
       0,     0,  1743,     0,   375,     0,   110,  1044,  1045,  1046,
    1047,  1048,  1049,   110,     0,     0,     0,     0,     0,     0,
     154,    37,     0,  1354,     0,   154,  1050,     0,     0,   154,
     224,     0,     0,     0,    88,    89,     0,    90,   173,    92,
       0,     0,     0,    50,  1355,     0,     0,   209,   209,  1356,
       0,    62,    63,    64,   168,  1357,   416,  1358,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
    1354,     0,   208,     0,     0,     0,    37,     0,   901,     0,
       0,     0,  1692,     0,   214,   214,     0,     0,     0,     0,
       0,  1698,     0,   901,  1359,  1360,   172,  1361,    50,    86,
      87,     0,    88,    89,  1354,    90,   173,    92,   154,   154,
     154,     0,     0,     0,   154,    14,     0,     0,   417,   154,
    1444,     0,     0,     0,   224,   224,  1487,     0,   816,     0,
       0,  1355,     0,  1445,  1446,  1733,  1356,     0,    62,    63,
      64,   168,  1357,   416,  1358,     0,     0,     0,     0,    14,
       0,   172,     0,     0,    86,    87,     0,    88,    89,   816,
      90,  1448,    92,     0,  1354,     0,     0,     0,     0,     0,
       0,     0,     0,   209,     0,     0,     0,     0,  1355,  1755,
       0,  1359,  1360,  1356,  1361,    62,    63,    64,   168,  1357,
     416,  1358,   501,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   417,   224,   224,     0,    14,
     214,     0,  1355,  1491,     0,   224,     0,  1356,   209,    62,
      63,    64,   168,  1357,   416,  1358,     0,     0,  1359,  1360,
       0,  1361,     0,   209,   209,     0,   208,     0,     0,     0,
    1804,     0,     0,     0,     0,   484,   485,  1811,     0,     0,
       0,     0,   417,     0,     0,   214,     0,     0,     0,   154,
    1493,     0,  1359,  1360,     0,  1361,     0,     0,     0,     0,
     214,   214,  1355,     0,     0,     0,     0,  1356,     0,    62,
      63,    64,   168,  1357,   416,  1358,   417,     0,     0,     0,
       0,     0,     0,     0,  1499,     0,   208,     0,     0,     0,
     154,     0,     0,     0,     0,     0,   154,     0,     0,     0,
       0,   209,   486,   487,     0,     0,     0,     0,     0,     0,
       0,     0,  1359,  1360,     0,  1361,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   208,     0,   208,
       0,   154,   266,     0,     0,     0,   417,     0,   214,     0,
       0,     0,     0,     0,  1643,     0,    62,    63,    64,    65,
      66,   416,     0,   154,     0,   208,   816,    72,   463,   770,
     268,     0,     0,     0,     0,     0,     0,     0,     0,   224,
     224,   816,   816,   816,   816,   816,     0,     0,     0,   816,
       0,     0,    37,     0,     0,     0,     0,   210,   210,     0,
     224,   226,     0,   464,   821,   465,     0,     0,     0,     0,
       0,     0,   154,     0,    50,     0,     0,     0,   466,   154,
     467,     0,   208,   417,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   846,   224,     0,   208,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   546,
     547,     0,   224,   224,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   172,   224,     0,
      86,   318,     0,    88,    89,     0,    90,   173,    92,     0,
    1330,   224,     0,     0,     0,     0,     0,     0,     0,   816,
       0,   322,   224,     0,     0,   428,   429,   430,     0,     0,
       0,   323,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,   224,   431,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   457,     0,     0,     0,   208,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,   224,     0,   224,     0,     0,
       0,     0,    34,    35,    36,     0,   210,     0,     0,     0,
       0,     0,   816,     0,   224,   204,     0,   816,   816,   816,
     816,   816,   816,   816,   816,   816,   816,   816,   816,   816,
     816,   816,   816,   816,   816,   816,   816,   816,   816,   816,
     816,   816,   816,   816,   816,  -980,  -980,  -980,  -980,  -980,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,   816,   989,     0,    76,    77,    78,    79,    80,     0,
       0,   457,     0,     0,     0,   206,     0,  1011,  1012,  1013,
    1014,    84,    85,     0,     0,  1021,     0,     0,     0,     0,
       0,     0,   224,     0,   224,    94,     0,     0,     0,     0,
       0,     0,  1252,     0,   208,     0,     0,     0,     0,    99,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,   224,  -980,  -980,  -980,  -980,  -980,
    1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,   210,     0,
       0,     0,     0,     0,   224,     0,     0,   210,     0,   208,
       0,  1050,     0,     0,   210,     0,     0,     0,     0,     0,
       0,   210,     0,     0,   208,   208,     0,   816,     0,     0,
       0,     0,   210,     0,     0,     0,     0,   224,     0,     0,
       0,   224,     0,     0,   816,  1113,   816,     0,   428,   429,
     430,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   816,     0,     0,     0,     0,     0,   431,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   224,   224,     0,   224,
       0,     0,   208,     0,     0,     0,   226,   457,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,   501,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,  1050,     0,     0,     0,     0,     0,     0,   210,     0,
       0,     0,     0,  1202,  1205,  1206,  1207,  1209,  1210,  1211,
    1212,  1213,  1214,  1215,  1216,  1217,  1218,  1219,  1220,  1221,
    1222,  1223,  1224,  1225,  1226,  1227,  1228,  1229,  1230,  1231,
    1232,   484,   485,   224,     0,   224,     0,     0,     0,     0,
     816,   224,     0,     0,   816,     0,   816,  1240,     0,   816,
     266,     0,     0,     0,   819,     0,     0,     0,     0,   224,
     224,     0,     0,   224,     0,     0,   327,     0,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,   268,     0,
       0,     0,   874,     0,     0,   819,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   486,   487,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,   816,     0,
       0,     0,     0,     0,     0,     0,     0,   224,   224,     0,
       0,     0,     0,     0,     0,   224,     0,   224,     0,     0,
       0,     0,     0,  1318,     0,   853,     0,   546,   547,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,   224,
    1333,   224,  1334,     0,     0,   172,     0,   224,    86,   318,
       0,    88,    89,     0,    90,   173,    92,     0,  1344,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   322,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   323,
     428,   429,   430,   816,   816,     0,     0,     0,     0,   816,
       0,   224,   210,     0,     0,     0,     0,   224,     0,   224,
     431,   432,     0,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,     0,     0,
       0,     0,     0,   210,     0,   210,     0,     0,     0,   457,
       0,     0,     0,     0,     0,     0,   327,     0,   327,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   819,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   819,   819,   819,
     819,   819,     0,     0,     0,   819,  1425,     0,     0,     0,
    1427,     0,  1428,     0,     0,  1429,  1054,     0,     0,   224,
       0,     0,     0,   327,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   266,   224,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1075,   224,   210,   210,     0,     0,     0,   816,
       0,     0,     0,   268,     0,     0,     0,     0,     0,  1075,
     816,     0,     0,     0,     0,     0,   816,     0,   210,     0,
     816,     0,     0,     0,   906,    37,     0,     0,     0,     0,
       0,     0,     0,     0,  1508,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,   819,   327,    50,  1116,   327,
       0,     0,     0,     0,     0,  -389,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   168,   169,   416,     0,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   816,   546,   547,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
     172,     0,     0,    86,   318,     0,    88,    89,   224,    90,
     173,    92,     0,     0,   210,   210,     0,   224,     0,  1652,
    1653,     0,     0,     0,   322,  1657,     0,     0,     0,     0,
     417,     0,   224,     0,   323,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   819,     0,
     210,     0,     0,   819,   819,   819,   819,   819,   819,   819,
     819,   819,   819,   819,   819,   819,   819,   819,   819,   819,
     819,   819,   819,   819,   819,   819,   819,   819,   819,   819,
     819,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     327,   801,     0,     0,   817,     0,     0,   819,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,   428,   429,
     430,     0,     0,     0,     0,   817,     0,     0,   212,   212,
    1050,     0,   230,     0,     0,     0,     0,     0,   431,   432,
     210,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,     0,     0,     0,     0,
       0,     0,   327,   327,     0,  1712,     0,   457,     0,     0,
     210,   327,     0,     0,     0,   210,  1722,     0,     0,     0,
       0,     0,  1727,     0,     0,     0,  1729,     0,     0,     0,
     210,   210,     0,   819,     0,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,     0,     0,
     819,     0,   819,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   819,   501,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,     0,     0,     0,     0,     0,  1764,   484,   485,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1208,     0,     0,  1353,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     807,   808,   484,   485,     0,     0,   809,     0,   810,     0,
       0,     0,   945,     0,     0,     0,     0,     0,     0,     0,
     811,     0,     0,     0,     0,     0,     0,   212,    34,    35,
      36,    37,     0,     0,     0,   486,   487,     0,     0,     0,
       0,   204,   817,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,   327,   327,   817,   817,   817,
     817,   817,     0,     0,     0,   817,     0,     0,     0,   486,
     487,     0,     0,     0,     0,     0,   819,   210,     0,     0,
     819,     0,   819,     0,     0,   819,     0,     0,   812,     0,
      76,    77,    78,    79,    80,     0,  1440,     0,     0,  1453,
       0,   206,     0,     0,     0,     0,   172,    84,    85,    86,
     813,     0,    88,    89,     0,    90,   173,    92,     0,     0,
       0,    94,     0,     0,     0,     0,     0,     0,   327,     0,
     814,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     815,     0,     0,     0,   327,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,   327,     0,   212,
       0,     0,     0,     0,   819,   817,     0,     0,   212,     0,
       0,     0,     0,  1512,  1513,   212,   428,   429,   430,     0,
       0,     0,   212,  1453,     0,     0,   327,     0,     0,     0,
       0,     0,     0,   230,     0,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   327,   819,
     819,   327,     0,   801,     0,   819,     0,  1658,     0,     0,
       0,     0,     0,     0,     0,  1453,     0,   230,   817,     0,
       0,     0,     0,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     817,     0,   428,   429,   430,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,   817,     0,     0,
       0,     0,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   327,   456,
     327,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     949,   457,     0,     0,     0,   820,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   327,     0,     0,
     327,     0,    34,    35,    36,    37,     0,   203,    40,     0,
       0,     0,     0,     0,     0,   204,   820,     0,     0,     0,
       0,     0,     0,     0,     0,   819,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   819,     0,     0,     0,
       0,     0,   819,   817,     0,     0,   819,     0,   221,     0,
       0,     0,     0,   327,   222,     0,     0,   327,     0,     0,
     817,     0,   817,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,   206,     0,     0,   817,     0,
     172,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     173,    92,     0,   212,     0,    94,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1066,   819,     0,    99,
       0,     0,   327,   327,   223,     0,     0,  1772,     0,   105,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   428,   429,   430,  1440,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   431,   432,   212,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,     0,   456,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     457,     0,     0,     0,   212,     0,   212,     0,     0,   327,
       0,   327,     0,     0,     0,     0,   817,     0,     0,     0,
     817,     0,   817,     0,     0,   817,     0,     0,     0,     0,
       0,     0,   212,   820,     0,   327,     0,   428,   429,   430,
       0,     0,     0,     0,     0,     0,   327,     0,   820,   820,
     820,   820,   820,     0,     0,     0,   820,   431,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,     0,   456,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,   457,     0,     0,     0,
       0,     0,     0,     0,   817,   212,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   327,     0,     0,     0,  1123,     0,     0,     0,   230,
       0,     0,     0,  1024,  1025,  1026,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   327,     0,   327,     0,     0,
       0,     0,     0,   327,  1027,     0,   820,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,
       0,   230,     0,     0,     0,     0,     0,     0,     0,   817,
     817,     0,  1050,     0,     0,   817,     0,     0,     0,     0,
       0,     0,     0,   327,  1025,  1026,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1134,     0,     0,  1027,   212,   212,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   820,
       0,   230,  1050,     0,   820,   820,   820,   820,   820,   820,
     820,   820,   820,   820,   820,   820,   820,   820,   820,   820,
     820,   820,   820,   820,   820,   820,   820,   820,   820,   820,
     820,   820,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   327,     0,     0,   820,     0,
       0,     0,     0,     0,  1199,     0,     0,     0,     0,     0,
       0,     0,   327,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1703,
       0,     0,     0,     0,     0,   817,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,   817,     0,     0,     0,
       0,     0,   817,     0,     0,     0,   817,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   327,   456,     0,
       0,   230,     0,     0,     0,     0,   212,     0,     0,     0,
     457,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   212,   212,     0,   820,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   817,     0,     0,
       0,   820,     0,   820,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   820,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,   327,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,   327,   529,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   168,
     169,    67,     0,    68,    69,    70,     0,   820,   230,     0,
       0,   820,     0,   820,    74,    75,   820,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,     0,   105,   106,     0,   107,   108,     0,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,   428,   429,
     430,     0,     0,     0,     0,   820,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   431,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
     820,   820,     0,     0,     0,     0,   820,     0,     0,     0,
       0,     0,     0,     0,     0,  1663,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,  1158,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,     0,    88,
      89,     0,    90,    91,    92,    93,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,    97,     0,
      98,     0,    99,   100,   101,     0,   820,   102,     0,   103,
     104,  1083,   105,   106,     0,   107,   108,   820,     0,     0,
       0,     0,     0,   820,     0,     0,     0,   820,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
    1745,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,   820,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,     0,    88,    89,     0,    90,    91,    92,    93,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,    97,     0,    98,     0,    99,   100,   101,     0,
       0,   102,     0,   103,   104,  1253,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    74,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,    81,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,     0,
      88,    89,     0,    90,    91,    92,    93,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,    97,
       0,    98,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,     0,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,    81,     0,     0,    82,     0,     0,     0,
       0,   172,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   173,    92,    93,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   102,     0,   103,   104,   647,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
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
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
      81,     0,     0,    82,     0,     0,     0,     0,   172,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   173,    92,
      93,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,     0,   103,   104,  1053,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,    81,     0,     0,
      82,     0,     0,     0,     0,   172,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   173,    92,    93,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   102,
       0,   103,   104,  1097,   105,   106,     0,   107,   108,     5,
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
       0,     0,     0,    74,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,    81,     0,     0,    82,     0,     0,
       0,     0,   172,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   173,    92,    93,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   102,     0,   103,   104,
    1164,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,  1166,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,    81,     0,     0,    82,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,    93,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   102,     0,   103,   104,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,  1319,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,     0,   172,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   173,    92,    93,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     102,     0,   103,   104,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,    93,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,  1431,   105,   106,     0,   107,   108,     5,     6,     7,
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
       0,    74,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,    81,     0,     0,    82,     0,     0,     0,     0,
     172,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     173,    92,    93,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   102,     0,   103,   104,  1654,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
    1699,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,    93,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   102,     0,   103,   104,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    74,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,    81,     0,     0,    82,
       0,     0,     0,     0,   172,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   173,    92,    93,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,  1734,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,  1737,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,    81,     0,     0,    82,     0,     0,     0,
       0,   172,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   173,    92,    93,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   102,     0,   103,   104,     0,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
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
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
      81,     0,     0,    82,     0,     0,     0,     0,   172,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   173,    92,
      93,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,     0,   103,   104,  1754,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,    81,     0,     0,
      82,     0,     0,     0,     0,   172,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   173,    92,    93,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   102,
       0,   103,   104,  1771,   105,   106,     0,   107,   108,     5,
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
       0,     0,     0,    74,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,    81,     0,     0,    82,     0,     0,
       0,     0,   172,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   173,    92,    93,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   102,     0,   103,   104,
    1812,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,    81,     0,     0,    82,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,    93,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   102,     0,   103,   104,  1819,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
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
      72,    73,     0,     0,     0,     0,     0,    74,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,     0,   172,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   173,    92,    93,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     102,     0,   103,   104,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   786,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   168,
     169,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,     0,   105,   106,     0,   107,   108,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   991,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   168,   169,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,    81,     0,     0,    82,     0,     0,     0,     0,
     172,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     173,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   102,     0,   103,   104,     0,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1503,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   168,   169,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   102,     0,   103,   104,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1647,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     168,   169,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,    81,     0,     0,    82,
       0,     0,     0,     0,   172,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   173,    92,     0,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,     0,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,   168,   169,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,    81,     0,     0,    82,     0,     0,     0,
       0,   172,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   173,    92,     0,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   102,     0,   103,   104,     0,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   389,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
     722,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   168,   169,   170,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   171,
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,   172,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   173,    92,
       0,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,     0,     0,     0,     0,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   168,   169,   170,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   171,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,   172,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   173,    92,     0,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,  1026,   174,
       0,   332,     0,     0,   105,   106,     0,   107,   108,     5,
       6,     7,     8,     9,     0,     0,     0,  1027,     0,    10,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
    1048,  1049,     0,     0,   662,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1050,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   168,   169,
     170,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   171,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,   172,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   173,    92,     0,   663,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   174,     0,     0,     0,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   168,   169,   170,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     171,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,     0,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   174,     0,     0,   781,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,  1027,     0,    10,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,  1048,  1049,     0,     0,  1110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1050,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   168,   169,   170,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   171,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,   172,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   173,    92,     0,  1111,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     174,     0,     0,     0,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   389,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   168,
     169,   170,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   171,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,   428,   429,
     430,     0,   105,   106,     0,   107,   108,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   431,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   457,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   185,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   168,   169,   170,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   171,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
     172,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     173,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,  1476,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   174,     0,     0,     0,     0,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,     0,   216,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1050,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   168,   169,   170,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   171,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   174,   428,   429,   430,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   457,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     168,   169,   170,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   171,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,     0,   172,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   173,    92,     0,     0,     0,    94,
       0,     0,    95,     0,     0,     0,  1477,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   174,     0,
     251,   429,   430,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
     431,   432,     0,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,   457,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   168,   169,   170,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   171,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,   172,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   173,    92,     0,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   174,     0,   254,     0,     0,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   389,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   168,   169,   170,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   171,
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,   172,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   173,    92,
       0,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,   428,   429,   430,     0,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   431,   432,  1323,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,     0,
     456,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   457,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   168,   169,   170,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   171,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,   172,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   173,    92,     0,     0,     0,
      94,     0,     0,    95,     0,  1324,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   174,
     527,     0,     0,     0,   105,   106,     0,   107,   108,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   676,   456,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   457,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   168,   169,
     170,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   171,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,   172,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   173,    92,     0,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   174,     0,     0,     0,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,   722,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   457,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   168,   169,   170,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     171,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,     0,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   174,     0,     0,     0,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,     0,     0,     0,   763,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1050,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   168,   169,   170,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   171,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,   172,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   173,    92,     0,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     174,     0,     0,     0,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,   765,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   168,
     169,   170,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   171,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   174,     0,     0,
       0,     0,   105,   106,     0,   107,   108,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,  1048,  1049,     0,     0,     0,     0,
       0,     0,  1155,     0,     0,     0,     0,     0,     0,  1050,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   168,   169,   170,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   171,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
     172,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     173,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   174,   428,   429,   430,     0,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   431,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   457,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   168,   169,   170,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   171,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,     0,    95,     0,   458,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   174,   428,   429,   430,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   457,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
     608,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     168,   169,   170,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   171,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,     0,   172,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   173,    92,     0,     0,     0,    94,
       0,     0,    95,     0,   543,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   174,     0,
       0,     0,     0,   105,   106,     0,   107,   108,   256,   257,
       0,   258,   259,     0,     0,   260,   261,   262,   263,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   264,     0,   265,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   267,   456,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   457,   269,   270,   271,   272,
     273,   274,   275,     0,     0,     0,    37,     0,   203,    40,
       0,     0,     0,     0,     0,     0,     0,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,    50,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,     0,     0,     0,   709,   311,   312,   313,     0,
       0,     0,   314,   556,   557,     0,     0,     0,     0,     0,
     256,   257,     0,   258,   259,     0,     0,   260,   261,   262,
     263,   558,     0,     0,     0,     0,     0,    88,    89,     0,
      90,   173,    92,   319,   264,   320,   265,     0,   321,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,   710,     0,
     105,     0,     0,     0,   267,     0,     0,     0,     0,   457,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   270,
     271,   272,   273,   274,   275,     0,     0,     0,    37,     0,
     203,    40,     0,     0,     0,     0,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
      50,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,     0,     0,     0,   310,   311,   312,
     313,     0,     0,     0,   314,   556,   557,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   256,   257,     0,
     258,   259,     0,   558,   260,   261,   262,   263,     0,    88,
      89,     0,    90,   173,    92,   319,     0,   320,     0,     0,
     321,   264,     0,   265,     0,   266,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     710,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,   267,     0,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   272,   273,
     274,   275,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,    50,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,     0,     0,     0,     0,   311,   312,   313,     0,     0,
       0,   314,   315,   316,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     317,     0,     0,    86,   318,     0,    88,    89,     0,    90,
     173,    92,   319,     0,   320,     0,     0,   321,   256,   257,
       0,   258,   259,     0,   322,   260,   261,   262,   263,     0,
       0,     0,     0,     0,   323,     0,     0,     0,  1627,     0,
       0,     0,   264,     0,   265,   432,   266,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,   267,     0,   268,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,   269,   270,   271,   272,
     273,   274,   275,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,    50,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,     0,     0,     0,     0,   311,   312,   313,     0,
       0,     0,   314,   315,   316,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   317,     0,     0,    86,   318,     0,    88,    89,     0,
      90,   173,    92,   319,     0,   320,     0,     0,   321,   256,
     257,     0,   258,   259,     0,   322,   260,   261,   262,   263,
       0,     0,     0,     0,     0,   323,     0,     0,     0,  1695,
       0,     0,     0,   264,     0,   265,     0,   266,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,   267,     0,   268,     0,     0,     0,     0,
       0,     0,     0,     0,   457,     0,     0,   269,   270,   271,
     272,   273,   274,   275,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,    50,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,     0,     0,     0,   310,   311,   312,   313,
       0,     0,     0,   314,   315,   316,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   317,     0,     0,    86,   318,     0,    88,    89,
       0,    90,   173,    92,   319,     0,   320,     0,     0,   321,
     256,   257,     0,   258,   259,     0,   322,   260,   261,   262,
     263,     0,     0,     0,     0,     0,   323,     0,     0,     0,
       0,     0,     0,     0,   264,     0,   265,     0,   266,  -980,
    -980,  -980,  -980,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,     0,     0,
       0,     0,     0,     0,   267,     0,   268,     0,     0,   457,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   270,
     271,   272,   273,   274,   275,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
      50,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,     0,     0,     0,     0,   311,   312,
     313,     0,     0,     0,   314,   315,   316,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   317,     0,     0,    86,   318,     0,    88,
      89,     0,    90,   173,    92,   319,     0,   320,     0,     0,
     321,     0,   256,   257,     0,   258,   259,   322,  1435,   260,
     261,   262,   263,     0,     0,     0,     0,   323,     0,     0,
       0,     0,     0,     0,     0,     0,   264,     0,   265,     0,
     266,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,     0,
       0,     0,     0,     0,     0,     0,   267,     0,   268,     0,
       0,  1050,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    50,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,     0,     0,     0,     0,
     311,   312,   313,     0,     0,     0,   314,   315,   316,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   317,     0,     0,    86,   318,
       0,    88,    89,     0,    90,   173,    92,   319,     0,   320,
       0,     0,   321,  1527,  1528,  1529,  1530,  1531,     0,   322,
    1532,  1533,  1534,  1535,     0,     0,     0,     0,     0,   323,
       0,     0,     0,     0,     0,     0,     0,  1536,  1537,  1538,
    -980,  -980,  -980,  -980,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1539,     0,     0,
    1050,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1540,  1541,  1542,  1543,  1544,  1545,  1546,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1547,  1548,  1549,  1550,  1551,  1552,  1553,  1554,
    1555,  1556,  1557,    50,  1558,  1559,  1560,  1561,  1562,  1563,
    1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,  1573,
    1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,  1582,  1583,
    1584,  1585,  1586,  1587,     0,     0,     0,  1588,  1589,     0,
    1590,  1591,  1592,  1593,  1594,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1595,  1596,  1597,     0,
       0,     0,    88,    89,     0,    90,   173,    92,  1598,     0,
    1599,  1600,     0,  1601,   428,   429,   430,     0,     0,     0,
    1602,  1603,     0,  1604,     0,  1605,  1606,     0,     0,     0,
       0,     0,     0,     0,   431,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,   428,   429,   430,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
       0,     0,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
     428,   429,   430,     0,     0,     0,     0,     0,     0,     0,
       0,   457,     0,     0,     0,     0,     0,     0,     0,     0,
     431,   432,     0,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   457,
       0,     0,     0,     0,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   431,   432,   545,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,   428,   429,   430,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
       0,     0,   431,   432,   563,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
     256,   257,     0,   258,   259,     0,     0,   260,   261,   262,
     263,   457,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   567,     0,   264,     0,   265,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   270,
     271,   272,   273,   274,   275,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,   755,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
      50,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,     0,     0,     0,   310,   311,   312,
     313,     0,     0,   778,   314,   556,   557,     0,     0,     0,
       0,     0,   256,   257,     0,   258,   259,     0,     0,   260,
     261,   262,   263,   558,     0,     0,     0,     0,     0,    88,
      89,     0,    90,   173,    92,   319,   264,   320,   265,     0,
     321,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   267,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    50,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,     0,     0,     0,  1200,
     311,   312,   313,     0,     0,     0,   314,   556,   557,     0,
       0,     0,     0,     0,   256,   257,     0,   258,   259,     0,
       0,   260,   261,   262,   263,   558,     0,     0,     0,     0,
       0,    88,    89,     0,    90,   173,    92,   319,   264,   320,
     265,     0,   321,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,    50,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,     0,     0,
       0,     0,   311,   312,   313,     0,     0,     0,   314,   556,
     557,   985,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   558,     0,     0,
       0,     0,     0,    88,    89,     0,    90,   173,    92,   319,
       0,   320,     0,    29,   321,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,     0,   203,    40,     0,     0,
       0,     0,     0,     0,   204,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     986,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,   206,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,   807,   808,     0,    94,     0,     0,   809,     0,   810,
       0,     0,     0,     0,     0,     0,     0,     0,    99,     0,
       0,   811,     0,   207,     0,     0,     0,     0,   105,    34,
      35,    36,    37,     0,     0,     0,     0,   428,   429,   430,
       0,     0,   204,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,   431,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,     0,   456,     0,     0,     0,     0,   812,
       0,    76,    77,    78,    79,    80,   457,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,   172,    84,    85,
      86,   813,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,     0,     0,     0,     0,     0,     0,
       0,   814,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   815,     0,     0,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   504,   431,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,   428,   429,   430,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
       0,     0,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,     0,     0,     0,     0,     0,     0,     0,   428,   429,
     430,   457,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   431,   432,
     513,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   428,   429,   430,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,     0,     0,     0,     0,   431,   432,   926,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,     0,     0,     0,     0,     0,
       0,  1024,  1025,  1026,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1027,     0,   970,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,     0,  1024,
    1025,  1026,     0,     0,     0,     0,     0,     0,     0,     0,
    1050,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1027,     0,  1282,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1024,  1025,  1026,     0,  1050,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1027,     0,  1190,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,
       0,     0,  1024,  1025,  1026,     0,     0,     0,     0,     0,
       0,     0,     0,  1050,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1027,     0,  1340,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,     0,    29,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,  1050,   203,    40,     0,     0,     0,     0,     0,     0,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1424,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,  1510,     0,
     206,     0,     0,     0,     0,   172,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   173,    92,    29,     0,     0,
      94,     0,     0,     0,     0,    34,    35,    36,    37,     0,
     203,    40,     0,     0,    99,     0,     0,     0,   204,   207,
       0,     0,   579,     0,   105,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   599,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,   206,     0,
       0,     0,     0,   172,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   173,    92,    29,     0,   940,    94,     0,
       0,     0,     0,    34,    35,    36,    37,     0,   203,    40,
       0,     0,    99,     0,     0,     0,   204,   207,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,   206,     0,     0,     0,
       0,   172,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   173,    92,    29,     0,     0,    94,     0,     0,     0,
       0,    34,    35,    36,    37,     0,   203,    40,     0,     0,
      99,     0,     0,     0,   204,   207,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1078,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,   206,     0,     0,     0,     0,   172,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   173,
      92,    29,     0,     0,    94,     0,     0,     0,     0,    34,
      35,    36,    37,     0,   203,    40,     0,     0,    99,     0,
       0,     0,   204,   207,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,    34,    35,    36,    37,     0,   203,
      40,     0,     0,     0,     0,     0,    99,   622,     0,     0,
       0,   207,     0,     0,     0,     0,   105,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,   206,     0,     0,
       0,     0,   172,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   173,    92,     0,     0,     0,    94,     0,    34,
      35,    36,    37,     0,   203,    40,     0,     0,     0,     0,
       0,    99,   204,     0,     0,     0,   623,     0,     0,     0,
       0,   624,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,   172,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   173,    92,     0,
       0,     0,    94,     0,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,   223,   790,     0,   431,   432,   105,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   428,   429,   430,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   791,   431,   432,   923,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,     0,   456,   428,   429,   430,     0,     0,
       0,     0,     0,     0,     0,     0,   457,     0,     0,     0,
       0,     0,     0,     0,     0,   431,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,  1024,  1025,  1026,     0,     0,     0,     0,
       0,     0,     0,     0,   457,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1027,  1345,     0,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,  1024,
    1025,  1026,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1050,     0,     0,     0,     0,     0,     0,     0,
    1027,     0,     0,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,   430,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1050,     0,
       0,     0,     0,   431,   432,     0,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,     0,
     456,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   457
};

static const yytype_int16 yycheck[] =
{
       5,     6,   123,     8,     9,    10,    11,    12,    13,   150,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     4,    56,    29,    30,    93,   176,     4,   382,
      97,    98,     4,     4,    31,    31,  1100,    57,   639,    44,
      31,     4,   155,   382,    60,   102,   899,    52,    33,    54,
     517,   150,    57,   498,    59,   122,   177,   224,    56,   636,
     635,    46,   102,   456,   665,   888,    51,    83,    57,   757,
      86,   339,   340,   615,   488,   488,    81,   919,   796,   573,
     574,   102,     9,   492,   493,   102,   984,   767,    32,     9,
    1096,    44,     9,   935,    14,  1087,     9,   102,     9,     9,
       9,     9,   789,    14,    14,     9,    32,     9,   219,     9,
      49,   525,   525,     9,   523,   719,     9,    38,     9,     9,
      49,     9,    70,   236,     9,     9,    38,    70,    81,   971,
       9,    83,     9,    70,   174,   235,    50,    51,     9,    31,
      36,    83,   155,    83,     9,    83,     9,     4,     9,     9,
       9,    83,    49,   174,    70,    49,  1641,   174,    90,   134,
     135,   155,    83,    70,    38,   115,     0,   207,   155,   174,
      38,    83,   106,   107,    49,    14,   181,   190,   161,   190,
      38,   176,   190,   223,   102,     8,   207,   134,   135,   176,
     207,    38,    70,    32,    70,   190,   190,  1020,    70,     4,
      54,   123,   207,   193,   102,   160,   223,   155,   130,    83,
     193,  1696,    51,   155,    70,    83,     4,   169,   223,   194,
      70,   153,   154,   173,    70,    83,    70,   134,   135,    70,
      70,   169,   237,    70,    70,   240,    83,   838,    32,   155,
      70,   159,   247,   248,    70,   193,    70,   190,   169,   193,
     193,    70,   191,   190,   191,   195,   176,   169,   938,   188,
     194,   159,   191,   177,   191,    53,   192,   367,    56,   418,
     241,  1277,   192,  1171,   245,   192,   994,   193,   996,   192,
     193,   192,   192,   192,   192,    73,   193,  1279,   192,  1131,
     192,   188,   192,   150,  1286,   191,  1288,   191,   331,   192,
    1355,   192,   192,    91,   192,    93,   160,   192,   192,    97,
      98,   169,   164,   192,   191,  1307,   191,   193,   123,   359,
     191,   193,   169,   339,   340,   341,   191,  1150,   191,    83,
     191,   191,   191,   331,   122,   877,    83,   193,   359,   916,
     834,   835,   359,   193,   190,   412,   190,   193,   176,   470,
     190,   500,   193,   358,   359,    90,   193,   193,   374,    90,
     365,    56,   190,   193,   190,   370,   190,   193,   190,   193,
     106,   107,   177,   176,   193,   460,   409,    53,    54,    55,
      54,    83,    84,  1389,   389,  1391,  1441,   190,   190,   122,
      83,    84,   397,    69,    83,    57,   507,   130,   465,   466,
     467,   468,   190,   408,   489,   358,   388,    69,  1400,   494,
    1465,   409,  1467,   190,   471,   169,   190,   152,   190,   864,
     190,   152,   169,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   471,   457,   241,   459,   460,   461,   245,   194,   456,
     456,   249,    83,  1151,   462,   456,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,  1485,
    1167,  1085,   471,  1489,   489,   490,   160,   492,   493,   494,
     495,   905,   905,   195,   102,   102,   501,   193,   187,   504,
     190,   175,   195,    70,   193,    83,   176,   176,   513,    83,
     515,   531,   653,   511,  1337,   626,    90,   190,   523,   190,
     190,   190,    83,   932,  1099,   155,   531,   161,   533,    90,
     382,   152,   153,   154,   190,   888,    50,    51,  1115,   396,
     174,  1118,   642,   331,   644,   712,   176,   940,    70,   888,
     190,   159,   159,   159,   653,   134,   135,   573,   574,   193,
     828,   159,   830,   193,   456,   536,   190,   134,   135,  1063,
     462,   266,   721,   268,   579,   153,   154,    83,   745,   153,
     154,    38,    83,   623,    90,  1408,   192,   193,   155,    90,
     111,   396,   153,   154,    75,    76,   488,   708,   119,   120,
     121,   122,   123,   124,   671,    75,    76,  1295,   396,  1297,
     462,   190,   134,   135,   105,   106,   107,   405,   623,   511,
     192,   409,     4,   190,   412,   199,    27,    28,   323,   192,
     522,    70,    83,   525,  1640,   192,   488,   198,  1644,    90,
     105,   106,   107,   192,    31,    27,    28,   153,   154,   816,
     192,   152,   153,   154,   636,   768,   823,   192,   663,   511,
      81,   761,   762,   192,   469,   186,  1133,  1020,   768,   769,
     522,   676,   155,   525,   462,   463,   464,   465,   466,   467,
     468,  1020,   103,  1287,   119,   120,   121,   798,   102,   103,
      83,  1266,   803,   176,    81,   132,   133,    90,   192,   193,
     488,   152,   153,   154,  1281,   710,   192,   190,    31,    31,
     193,   406,   192,   193,   409,  1124,   103,    70,  1406,  1783,
     102,   339,   340,   511,   193,  1170,  1135,    50,    50,     4,
      53,    53,  1670,  1671,  1798,   740,   155,   525,   125,   160,
     190,   162,   163,   190,   165,   166,   167,   155,   536,   860,
      70,   138,   139,   159,   119,   120,   121,   122,   123,   124,
     153,   154,   106,   107,   108,   190,  1370,   555,   773,   156,
     192,  1777,   159,   160,    49,   162,   163,    48,   165,   166,
     167,  1666,  1667,    69,   789,   176,  1792,   805,   806,   577,
     578,   155,   174,   190,   190,   197,   653,  1150,     9,   155,
     785,   155,   190,  1248,     8,   190,   192,   155,  1496,    14,
     155,  1150,   828,   192,   830,   216,   604,   605,   834,   835,
     836,   186,   192,   794,  1401,   207,   193,   192,   119,   120,
     121,   122,   123,   124,   216,     9,   130,   112,  1442,   130,
     131,   223,   117,   192,   119,   120,   121,   122,   123,   124,
     125,   130,    14,   191,   965,   550,    14,   176,   102,   241,
      53,    54,    55,   245,    57,   191,   191,   191,   873,    81,
     191,    83,   939,    85,  1283,   190,    69,   168,   196,   111,
     190,   190,   887,   671,     9,   152,   191,   162,   163,   191,
     165,   103,   191,   891,   876,   186,  1749,    94,   191,  1010,
     876,     9,   192,    14,   876,   876,  1017,   176,   913,   190,
       9,   186,   190,   876,    83,   192,  1769,   899,   923,   194,
     193,   926,   779,   928,   192,  1778,   191,   932,   119,   120,
     121,   122,   123,   124,   916,   193,   191,   632,   633,  1106,
     191,   190,   192,   940,   940,   132,   641,   348,   191,   940,
     162,   163,    70,   165,   166,   167,   357,    32,  1646,   175,
     133,   155,     9,   364,   136,   970,   348,   191,   825,   757,
     371,   759,   155,    14,   779,   357,     9,   359,  1387,   188,
      81,   382,   364,   529,  1337,     9,     9,   177,   191,   371,
     132,   779,   977,    14,     9,   186,   368,   197,  1337,   891,
     372,   197,   103,    14,   155,   793,   794,   978,   194,   197,
     191,   903,   191,   905,   396,   197,   191,   102,   564,   876,
     825,  1019,   190,  1080,  1022,   192,   398,     4,   400,   401,
     402,   403,   192,     9,  1145,    91,   888,   825,   136,   891,
     897,   155,     9,   191,   832,   833,   190,  1063,   940,   155,
     190,   903,   155,   905,   193,  1408,   193,     9,   159,   194,
    1080,   162,   163,    14,   165,   166,   167,   177,   192,  1408,
       9,   876,    49,   861,    81,  1080,   193,    14,   197,    14,
     972,   193,   191,   188,    32,   192,  1197,    32,   876,  1690,
     190,   637,   897,  1204,   190,    14,   103,   498,   190,   190,
      27,    28,  1084,   891,    14,    52,  1111,   190,  1084,   897,
     805,   806,  1084,  1084,   190,   903,   498,   905,     9,  1124,
     972,  1084,   191,   980,   192,   982,   192,   190,   136,    14,
    1135,  1136,   177,  1115,   136,   112,  1118,    27,    28,    81,
     117,     9,   119,   120,   121,   122,   123,   124,   125,   156,
       9,   939,   159,   191,   536,   162,   163,    69,   165,   166,
     167,   103,  1167,   951,   952,   953,  1767,   197,  1020,    83,
     192,     9,  1177,   190,  1159,   980,    31,   982,   194,   136,
     194,   190,   192,   878,   972,   162,   163,   194,   165,    14,
     978,  1162,   980,  1304,   982,    83,   191,   193,   190,   894,
     190,   192,    31,   191,    59,   136,    81,   193,   193,   186,
     197,     9,   907,    91,  1002,   193,   152,   194,    32,    77,
     162,   163,   192,   165,   166,   167,    81,  1084,   103,   191,
    1371,  1019,   192,   177,  1022,   781,   136,    32,    81,   136,
     786,   936,   191,     9,   194,  1366,     9,   193,   103,   192,
     191,   191,    81,  1258,   192,    14,   111,  1262,   191,  1264,
     103,   662,    91,  1051,  1246,   194,    83,  1272,   111,   112,
     190,   192,   190,  1255,   103,   191,   191,  1282,  1283,  1084,
     662,     9,   191,   138,   139,   193,   191,   162,   163,   216,
     165,   166,   167,   191,   136,   191,  1084,    32,  1150,  1281,
     192,   156,   191,   998,   159,   160,  1001,   162,   163,   191,
     165,   166,   167,   192,  1463,   190,   159,  1174,   192,   162,
     163,   722,   165,   166,   167,   180,   216,   156,   193,   112,
     159,   164,   192,   162,   163,   190,   165,   166,   167,   160,
     722,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    14,   117,    83,   191,   193,  1147,
     191,   136,   763,  1151,   765,   191,   136,    14,  1435,  1174,
      81,   176,    14,   193,  1162,    83,   192,    14,    83,  1384,
     191,   763,  1387,   765,   190,   136,  1174,   191,    14,  1246,
     791,   192,   103,   192,    67,    68,    14,   779,  1255,   192,
      14,   193,     9,  1098,     9,  1100,   194,    59,  1390,   791,
     190,    83,   794,    83,  1396,   176,  1398,   128,     9,  1401,
      81,   348,    83,    84,   193,   192,   115,   102,  1410,   155,
     357,   102,  1127,   177,   167,  1130,    36,   364,    81,    14,
    1438,   190,   103,   825,   371,   991,  1417,   848,   191,   190,
     192,   162,   163,  1241,   165,   166,   167,   173,   348,   177,
     103,   134,   135,   864,   865,   177,   848,   357,    83,   359,
     170,   191,     9,    83,   364,  1480,   191,  1626,   192,   190,
     193,   371,   864,   865,   191,  1337,    14,   888,  1183,    83,
    1347,    14,  1187,    83,   876,    81,    14,    14,    83,   160,
      83,   162,   163,  1063,   165,   166,   167,  1295,  1758,  1297,
     468,   465,  1494,   933,  1371,   897,   159,   103,   191,   162,
     163,   879,   165,   166,   167,  1774,  1168,  1322,  1502,   463,
    1501,  1502,   193,  1390,   195,  1769,  1492,   581,  1443,  1396,
    1610,  1398,  1347,  1361,  1525,  1802,  1790,  1242,  1243,  1660,
    1661,  1622,  1488,  1410,  1088,  1412,  1408,  1357,  1018,  1347,
    1365,  1146,   952,  1147,  1421,   365,   903,  1355,   967,   409,
     805,   498,  1350,  1361,   160,  1071,   162,   163,   164,   165,
     166,   167,  1724,   984,   985,  1003,    -1,  1051,  1621,    -1,
      50,    51,    52,    53,    54,    55,   978,    57,   980,    -1,
     982,  1742,   984,   985,   190,    -1,    81,  1412,   498,    69,
      -1,    27,    28,    -1,    -1,    31,  1421,  1684,  1406,  1020,
      -1,    -1,    -1,  1621,  1412,    -1,  1631,    -1,   103,  1417,
      -1,    50,    51,  1421,  1329,    -1,  1331,  1494,    -1,    -1,
      56,  1762,    -1,  1500,    -1,    -1,    -1,  1435,    -1,  1506,
    1438,    70,    -1,  1441,  1511,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,  1451,    -1,    -1,  1471,    -1,    -1,    -1,
    1458,  1366,    91,    -1,    -1,    -1,    -1,  1465,    -1,  1467,
      -1,    -1,    -1,    -1,   103,  1473,    -1,   162,   163,  1235,
     165,   166,   167,    -1,    -1,  1500,    -1,    -1,    -1,    -1,
      -1,  1506,  1084,    -1,    -1,    -1,  1511,    -1,  1496,  1110,
      -1,    -1,  1500,  1501,  1502,   190,    -1,    -1,  1506,   138,
      81,    -1,    -1,  1511,    -1,    -1,    -1,    -1,  1110,    -1,
      -1,    -1,   151,    -1,    -1,   662,    -1,    81,    -1,    83,
      84,    -1,   103,   162,   163,    -1,   165,   166,   167,  1150,
     111,   112,    -1,    -1,  1155,    -1,    -1,    -1,    -1,   103,
      -1,   180,    -1,  1309,    -1,    -1,    -1,  1749,    -1,  1170,
    1171,    -1,   662,  1155,    -1,    -1,    -1,    -1,    -1,  1325,
    1162,    -1,    -1,    -1,    31,    -1,    -1,  1769,  1170,  1171,
      -1,    -1,  1174,    -1,  1651,   722,  1778,    -1,   159,    -1,
     216,   162,   163,  1808,   165,   166,   167,    -1,    -1,    -1,
      -1,  1816,    59,    -1,    -1,    -1,   160,  1822,   162,   163,
    1825,   165,   166,   167,    -1,    -1,    -1,    -1,  1523,    -1,
      -1,    -1,   722,  1621,    81,  1692,   763,    -1,   765,    -1,
      -1,  1698,    -1,    -1,    -1,    -1,  1651,  1248,    -1,   193,
     266,   195,   268,    -1,  1642,    -1,   103,    -1,  1646,    -1,
      -1,    -1,    -1,  1651,   791,  1411,  1248,    -1,    -1,    -1,
      -1,  1659,    -1,   763,  1420,   765,  1733,    -1,  1666,  1667,
      -1,    -1,  1670,  1671,  1430,  1742,    -1,  1692,    -1,    -1,
      -1,   138,   139,  1698,    -1,    -1,  1684,    -1,  1755,    -1,
      -1,   791,    81,    -1,  1692,    -1,    -1,   323,    -1,   156,
    1698,    -1,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,   848,   169,    -1,   103,    -1,    -1,    -1,  1733,    -1,
    1625,    -1,   348,   180,    -1,    -1,  1337,   864,   865,    -1,
      -1,   357,    -1,   190,    -1,  1733,     4,  1804,   364,    -1,
    1755,    -1,    -1,  1741,  1811,   371,    -1,  1503,   848,    -1,
      78,    79,    80,    81,    -1,  1347,   382,  1755,    -1,    -1,
      -1,    -1,    -1,  1761,   864,   865,    -1,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,   103,   165,   166,   167,    -1,
     406,    49,    -1,   409,    -1,    -1,    -1,    -1,    -1,  1804,
      -1,    -1,    -1,    -1,    -1,    -1,  1811,  1408,    -1,    -1,
      -1,    -1,  1707,    -1,   193,    -1,  1804,    50,    51,    52,
      53,    54,    55,  1811,    -1,    -1,    -1,    -1,    -1,    -1,
    1412,    81,    -1,     4,    -1,  1417,    69,    -1,    -1,  1421,
     456,    -1,    -1,    -1,   162,   163,    -1,   165,   166,   167,
      -1,    -1,    -1,   103,   112,    -1,    -1,   984,   985,   117,
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
       4,    -1,   498,    -1,    -1,    -1,    81,    -1,  1783,    -1,
      -1,    -1,  1638,    -1,   984,   985,    -1,    -1,    -1,    -1,
      -1,  1647,    -1,  1798,   162,   163,   156,   165,   103,   159,
     160,    -1,   162,   163,     4,   165,   166,   167,  1500,  1501,
    1502,    -1,    -1,    -1,  1506,    49,    -1,    -1,   186,  1511,
     125,    -1,    -1,    -1,   550,   551,   194,    -1,   554,    -1,
      -1,   112,    -1,   138,   139,  1691,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    49,
      -1,   156,    -1,    -1,   159,   160,    -1,   162,   163,   585,
     165,   166,   167,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1110,    -1,    -1,    -1,    -1,   112,  1735,
      -1,   162,   163,   117,   165,   119,   120,   121,   122,   123,
     124,   125,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   186,   632,   633,    -1,    49,
    1110,    -1,   112,   194,    -1,   641,    -1,   117,  1155,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,   162,   163,
      -1,   165,    -1,  1170,  1171,    -1,   662,    -1,    -1,    -1,
    1796,    -1,    -1,    -1,    -1,    67,    68,  1803,    -1,    -1,
      -1,    -1,   186,    -1,    -1,  1155,    -1,    -1,    -1,  1651,
     194,    -1,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,
    1170,  1171,   112,    -1,    -1,    -1,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   194,    -1,   722,    -1,    -1,    -1,
    1692,    -1,    -1,    -1,    -1,    -1,  1698,    -1,    -1,    -1,
      -1,  1248,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   763,    -1,   765,
      -1,  1733,    31,    -1,    -1,    -1,   186,    -1,  1248,    -1,
      -1,    -1,    -1,    -1,   194,    -1,   119,   120,   121,   122,
     123,   124,    -1,  1755,    -1,   791,   792,   130,   131,   191,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   805,
     806,   807,   808,   809,   810,   811,    -1,    -1,    -1,   815,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    27,    28,    -1,
     826,    31,    -1,   166,   554,   168,    -1,    -1,    -1,    -1,
      -1,    -1,  1804,    -1,   103,    -1,    -1,    -1,   181,  1811,
     183,    -1,   848,   186,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   585,   862,    -1,   864,   865,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,    -1,   878,   879,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   888,    -1,    -1,    -1,    -1,   156,   894,    -1,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
     169,   907,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   915,
      -1,   180,   918,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     936,    -1,    -1,    -1,   940,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,   984,   985,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   998,    -1,    -1,  1001,    -1,  1003,    -1,    -1,
      -1,    -1,    78,    79,    80,    -1,   216,    -1,    -1,    -1,
      -1,    -1,  1018,    -1,  1020,    91,    -1,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,  1048,  1049,  1050,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,  1067,   792,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    69,    -1,    -1,    -1,   151,    -1,   807,   808,   809,
     810,   157,   158,    -1,    -1,   815,    -1,    -1,    -1,    -1,
      -1,    -1,  1098,    -1,  1100,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   197,    -1,  1110,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1127,    -1,    -1,  1130,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   348,    -1,
      -1,    -1,    -1,    -1,  1150,    -1,    -1,   357,    -1,  1155,
      -1,    69,    -1,    -1,   364,    -1,    -1,    -1,    -1,    -1,
      -1,   371,    -1,    -1,  1170,  1171,    -1,  1173,    -1,    -1,
      -1,    -1,   382,    -1,    -1,    -1,    -1,  1183,    -1,    -1,
      -1,  1187,    -1,    -1,  1190,   915,  1192,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1208,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,  1242,  1243,    -1,  1245,
      -1,    -1,  1248,    -1,    -1,    -1,   456,    69,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    69,    -1,    -1,    -1,    -1,    -1,    -1,   498,    -1,
      -1,    -1,    -1,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,
    1050,    67,    68,  1329,    -1,  1331,    -1,    -1,    -1,    -1,
    1336,  1337,    -1,    -1,  1340,    -1,  1342,  1067,    -1,  1345,
      31,    -1,    -1,    -1,   554,    -1,    -1,    -1,    -1,  1355,
    1356,    -1,    -1,  1359,    -1,    -1,    56,    -1,    -1,    -1,
    1366,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,
      -1,    -1,   194,    -1,    -1,   585,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1408,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,  1424,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1433,  1434,    -1,
      -1,    -1,    -1,    -1,    -1,  1441,    -1,  1443,    -1,    -1,
      -1,    -1,    -1,  1173,    -1,   191,    -1,   138,   139,    -1,
      -1,    -1,   662,    -1,    -1,    -1,    -1,    -1,    -1,  1465,
    1190,  1467,  1192,    -1,    -1,   156,    -1,  1473,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,    -1,  1208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,
      10,    11,    12,  1509,  1510,    -1,    -1,    -1,    -1,  1515,
      -1,  1517,   722,    -1,    -1,    -1,    -1,  1523,    -1,  1525,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,   763,    -1,   765,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,   266,    -1,   268,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   791,   792,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   807,   808,   809,
     810,   811,    -1,    -1,    -1,   815,  1336,    -1,    -1,    -1,
    1340,    -1,  1342,    -1,    -1,  1345,   826,    -1,    -1,  1625,
      -1,    -1,    -1,   323,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,  1642,    -1,   848,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   862,  1659,   864,   865,    -1,    -1,    -1,  1665,
      -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,   879,
    1676,    -1,    -1,    -1,    -1,    -1,  1682,    -1,   888,    -1,
    1686,    -1,    -1,    -1,   194,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1424,    -1,    -1,    -1,    -1,    -1,
      -1,  1707,    -1,    -1,    -1,   915,   406,   103,   918,   409,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
     940,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1747,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1757,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,   159,   160,    -1,   162,   163,  1774,   165,
     166,   167,    -1,    -1,   984,   985,    -1,  1783,    -1,  1509,
    1510,    -1,    -1,    -1,   180,  1515,    -1,    -1,    -1,    -1,
     186,    -1,  1798,    -1,   190,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,    -1,
    1020,    -1,    -1,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,
    1050,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     550,   551,    -1,    -1,   554,    -1,    -1,  1067,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,   585,    -1,    -1,    27,    28,
      69,    -1,    31,    -1,    -1,    -1,    -1,    -1,    30,    31,
    1110,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,   632,   633,    -1,  1665,    -1,    69,    -1,    -1,
    1150,   641,    -1,    -1,    -1,  1155,  1676,    -1,    -1,    -1,
      -1,    -1,  1682,    -1,    -1,    -1,  1686,    -1,    -1,    -1,
    1170,  1171,    -1,  1173,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
    1190,    -1,  1192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1208,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,  1747,    67,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,  1245,    -1,    -1,  1248,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    67,    68,    -1,    -1,    56,    -1,    58,    -1,
      -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,   216,    78,    79,
      80,    81,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,
      -1,    91,   792,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,   805,   806,   807,   808,   809,
     810,   811,    -1,    -1,    -1,   815,    -1,    -1,    -1,   134,
     135,    -1,    -1,    -1,    -1,    -1,  1336,  1337,    -1,    -1,
    1340,    -1,  1342,    -1,    -1,  1345,    -1,    -1,   138,    -1,
     140,   141,   142,   143,   144,    -1,  1356,    -1,    -1,  1359,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,   878,    -1,
     180,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,   894,    -1,    -1,    -1,  1408,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   907,    -1,   348,
      -1,    -1,    -1,    -1,  1424,   915,    -1,    -1,   357,    -1,
      -1,    -1,    -1,  1433,  1434,   364,    10,    11,    12,    -1,
      -1,    -1,   371,  1443,    -1,    -1,   936,    -1,    -1,    -1,
      -1,    -1,    -1,   382,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   998,  1509,
    1510,  1001,    -1,  1003,    -1,  1515,    -1,  1517,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1525,    -1,   456,  1018,    -1,
      -1,    -1,    -1,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,  1049,
    1050,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,   498,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1067,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1098,    57,
    1100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     194,    69,    -1,    -1,    -1,   554,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1127,    -1,    -1,
    1130,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,   585,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1665,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1676,    -1,    -1,    -1,
      -1,    -1,  1682,  1173,    -1,    -1,  1686,    -1,   124,    -1,
      -1,    -1,    -1,  1183,   130,    -1,    -1,  1187,    -1,    -1,
    1190,    -1,  1192,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,  1208,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,   662,    -1,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   194,  1747,    -1,   185,
      -1,    -1,  1242,  1243,   190,    -1,    -1,  1757,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,  1774,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   722,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,   763,    -1,   765,    -1,    -1,  1329,
      -1,  1331,    -1,    -1,    -1,    -1,  1336,    -1,    -1,    -1,
    1340,    -1,  1342,    -1,    -1,  1345,    -1,    -1,    -1,    -1,
      -1,    -1,   791,   792,    -1,  1355,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,  1366,    -1,   807,   808,
     809,   810,   811,    -1,    -1,    -1,   815,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,   848,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1424,   864,   865,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1441,    -1,    -1,    -1,   194,    -1,    -1,    -1,   888,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1465,    -1,  1467,    -1,    -1,
      -1,    -1,    -1,  1473,    31,    -1,   915,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,   940,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1509,
    1510,    -1,    69,    -1,    -1,  1515,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1523,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   194,    -1,    -1,    31,   984,   985,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,
      -1,  1020,    69,    -1,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,
    1049,  1050,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1625,    -1,    -1,  1067,    -1,
      -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1642,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1659,
      -1,    -1,    -1,    -1,    -1,  1665,    -1,    -1,    -1,    -1,
      -1,  1110,    -1,    -1,    -1,    -1,  1676,    -1,    -1,    -1,
      -1,    -1,  1682,    -1,    -1,    -1,  1686,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1707,    57,    -1,
      -1,  1150,    -1,    -1,    -1,    -1,  1155,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1170,  1171,    -1,  1173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1747,    -1,    -1,
      -1,  1190,    -1,  1192,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,  1783,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,  1798,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1248,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,  1336,  1337,    -1,
      -1,  1340,    -1,  1342,   137,   138,  1345,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,
     193,    -1,   195,   196,    -1,   198,   199,    -1,    -1,  1408,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,  1424,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
    1509,  1510,    -1,    -1,    -1,    -1,  1515,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1524,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   194,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,    -1,   185,   186,   187,    -1,  1665,   190,    -1,   192,
     193,   194,   195,   196,    -1,   198,   199,  1676,    -1,    -1,
      -1,    -1,    -1,  1682,    -1,    -1,    -1,  1686,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
    1709,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,  1747,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,    -1,   116,   117,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,   148,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,    -1,   185,   186,   187,    -1,
      -1,   190,    -1,   192,   193,   194,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,   168,    -1,    -1,   171,
      -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,    -1,   185,   186,   187,    -1,    -1,   190,    -1,
     192,   193,    -1,   195,   196,    -1,   198,   199,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,   168,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    -1,    -1,   190,    -1,   192,   193,   194,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
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
     138,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
     148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,
     158,   159,   160,    -1,   162,   163,    -1,   165,   166,   167,
     168,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
      -1,    -1,   190,    -1,   192,   193,   194,   195,   196,    -1,
     198,   199,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,   168,    -1,    -1,
     171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,
      -1,   192,   193,   194,   195,   196,    -1,   198,   199,     3,
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
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,   193,
     194,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,   168,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   190,    -1,   192,   193,    -1,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,   101,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,   168,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,   192,   193,    -1,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,
     193,   194,   195,   196,    -1,   198,   199,     3,     4,     5,
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
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,   168,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,   192,   193,   194,   195,
     196,    -1,   198,   199,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,   148,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
      -1,   190,    -1,   192,   193,    -1,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,   168,    -1,    -1,   171,
      -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,
      -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,
     192,   193,   194,   195,   196,    -1,   198,   199,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,   168,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    -1,    -1,   190,    -1,   192,   193,    -1,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
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
     138,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
     148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,
     158,   159,   160,    -1,   162,   163,    -1,   165,   166,   167,
     168,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
      -1,    -1,   190,    -1,   192,   193,   194,   195,   196,    -1,
     198,   199,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,   168,    -1,    -1,
     171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,
      -1,   192,   193,   194,   195,   196,    -1,   198,   199,     3,
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
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,   193,
     194,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,   168,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   190,    -1,   192,   193,   194,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
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
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,   168,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,   192,   193,    -1,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,
     193,    -1,   195,   196,    -1,   198,   199,     3,     4,     5,
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
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,   192,   193,    -1,   195,
     196,    -1,   198,   199,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,   148,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
      -1,   190,    -1,   192,   193,    -1,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,
      -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,
      -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,
     192,   193,    -1,   195,   196,    -1,   198,   199,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    -1,    -1,   190,    -1,   192,   193,    -1,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
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
     138,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,
     158,   159,   160,    -1,   162,   163,    -1,   165,   166,   167,
      -1,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
      -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,
     198,   199,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,
     171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    12,   190,
      -1,   192,    -1,    -1,   195,   196,    -1,   198,   199,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,
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
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,    -1,   169,    -1,   171,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   185,   186,   187,    -1,    -1,   190,    -1,    -1,    -1,
      -1,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   190,    -1,    -1,   193,    -1,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    31,    -1,    13,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,   169,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    10,    11,
      12,    -1,   195,   196,    -1,   198,   199,     3,     4,     5,
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
      -1,    -1,   108,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,   194,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,    -1,    -1,    -1,   195,
     196,    -1,   198,   199,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
      -1,   190,    10,    11,    12,    -1,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,
      -1,    -1,   174,    -1,    -1,    -1,   194,    -1,   180,    -1,
      -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,
     192,    11,    12,   195,   196,    -1,   198,   199,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    69,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    -1,    -1,   190,    -1,   192,    -1,    -1,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
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
     138,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,
     158,   159,   160,    -1,   162,   163,    -1,   165,   166,   167,
      -1,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
      -1,    -1,   190,    10,    11,    12,    -1,   195,   196,    -1,
     198,   199,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,
     171,    -1,    -1,   174,    -1,   192,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,
     191,    -1,    -1,    -1,   195,   196,    -1,   198,   199,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    32,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   185,   186,   187,    -1,    -1,   190,    -1,    -1,    -1,
      -1,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,    -1,
      -1,    -1,   195,   196,    -1,   198,   199,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    10,    11,    12,    -1,   195,
     196,    -1,   198,   199,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,   174,    -1,   192,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
      -1,   190,    10,    11,    12,    -1,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,
      -1,    -1,   174,    -1,   192,    -1,    -1,    -1,   180,    -1,
      -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,
      -1,    -1,    -1,   195,   196,    -1,   198,   199,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   156,    -1,    -1,    -1,    -1,    -1,   162,   163,    -1,
     165,   166,   167,   168,    27,   170,    29,    -1,   173,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   193,    -1,
     195,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   156,    10,    11,    12,    13,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,   170,    -1,    -1,
     173,    27,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,   168,    -1,   170,    -1,    -1,   173,     3,     4,
      -1,     6,     7,    -1,   180,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    27,    -1,    29,    31,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,   168,    -1,   170,    -1,    -1,   173,     3,
       4,    -1,     6,     7,    -1,   180,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,   194,
      -1,    -1,    -1,    27,    -1,    29,    -1,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,   168,    -1,   170,    -1,    -1,   173,
       3,     4,    -1,     6,     7,    -1,   180,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,
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
     133,    -1,    -1,    -1,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,   170,    -1,    -1,
     173,    -1,     3,     4,    -1,     6,     7,   180,   181,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,   190,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      31,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,   168,    -1,   170,
      -1,    -1,   173,     3,     4,     5,     6,     7,    -1,   180,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   156,   157,   158,    -1,
      -1,    -1,   162,   163,    -1,   165,   166,   167,   168,    -1,
     170,   171,    -1,   173,    10,    11,    12,    -1,    -1,    -1,
     180,   181,    -1,   183,    -1,   185,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   192,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   192,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    27,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,    -1,   191,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   156,    -1,    -1,    -1,    -1,    -1,   162,
     163,    -1,   165,   166,   167,   168,    27,   170,    29,    -1,
     173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   156,    -1,    -1,    -1,    -1,
      -1,   162,   163,    -1,   165,   166,   167,   168,    27,   170,
      29,    -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,   170,    -1,    70,   173,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,    50,    51,    -1,   171,    -1,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    70,    -1,   190,    -1,    -1,    -1,    -1,   195,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,   138,
      -1,   140,   141,   142,   143,   144,    69,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     136,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
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
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    69,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,   136,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,    70,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   185,    -1,    -1,    -1,    91,   190,
      -1,    -1,   193,    -1,   195,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    70,    -1,    72,   171,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   185,    -1,    -1,    -1,    91,   190,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    70,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     185,    -1,    -1,    -1,    91,   190,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,    70,    -1,    -1,   171,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   185,    -1,
      -1,    -1,    91,   190,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,   185,    91,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,   195,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,   185,    91,    -1,    -1,    -1,   190,    -1,    -1,    -1,
      -1,   195,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,    28,    -1,    30,    31,   195,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   201,   202,     0,   203,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   140,   141,   142,   143,
     144,   148,   151,   156,   157,   158,   159,   160,   162,   163,
     165,   166,   167,   168,   171,   174,   180,   181,   183,   185,
     186,   187,   190,   192,   193,   195,   196,   198,   199,   204,
     207,   217,   218,   219,   220,   221,   224,   240,   241,   245,
     248,   255,   261,   321,   322,   330,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   345,   348,   360,   361,
     362,   364,   365,   367,   377,   378,   379,   381,   386,   389,
     408,   416,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   431,   444,   446,   448,   122,   123,
     124,   137,   156,   166,   190,   207,   240,   321,   342,   420,
     342,   190,   342,   342,   342,   108,   342,   342,   342,   406,
     407,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,    83,    91,   124,   151,   190,   218,   361,
     378,   381,   386,   420,   423,   420,    38,   342,   435,   436,
     342,   124,   130,   190,   218,   253,   378,   379,   380,   382,
     386,   417,   418,   419,   427,   432,   433,   190,   331,   383,
     190,   331,   352,   332,   342,   226,   331,   190,   190,   190,
     331,   192,   342,   207,   192,   342,     3,     4,     6,     7,
      10,    11,    12,    13,    27,    29,    31,    57,    59,    71,
      72,    73,    74,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     130,   131,   132,   133,   137,   138,   139,   156,   160,   168,
     170,   173,   180,   190,   207,   208,   209,   220,   449,   465,
     466,   468,   192,   337,   339,   342,   193,   233,   342,   111,
     112,   159,   210,   211,   212,   213,   217,    83,   195,   287,
     288,   123,   130,   122,   130,    83,   289,   190,   190,   190,
     190,   207,   259,   452,   190,   190,   332,    83,    90,   152,
     153,   154,   441,   442,   159,   193,   217,   217,   207,   260,
     452,   160,   190,   452,   452,    83,   187,   193,   353,    27,
     330,   334,   342,   343,   420,   424,   222,   193,    90,   384,
     441,    90,   441,   441,    32,   159,   176,   453,   190,     9,
     192,    38,   239,   160,   258,   452,   124,   186,   240,   322,
     192,   192,   192,   192,   192,   192,   192,   192,    10,    11,
      12,    30,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    69,   192,    70,
      70,   193,   155,   131,   166,   168,   181,   183,   261,   320,
     321,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    67,    68,   134,   135,   410,    70,
     193,   415,   190,   190,    70,   193,   195,   428,   190,   239,
     240,    14,   342,   192,   136,    48,   207,   405,    90,   330,
     343,   155,   420,   136,   197,     9,   391,   254,   330,   343,
     420,   453,   155,   190,   385,   410,   415,   191,   342,    32,
     224,     8,   354,     9,   192,   224,   225,   332,   333,   342,
     207,   273,   228,   192,   192,   192,   138,   139,   468,   468,
     176,   190,   111,   468,    14,   155,   138,   139,   156,   207,
     209,   192,   192,   192,   234,   115,   173,   192,   210,   212,
     210,   212,   217,   193,     9,   392,   192,   102,   159,   193,
     420,     9,   192,   130,   130,    14,     9,   192,   420,   445,
     332,   330,   343,   420,   423,   424,   191,   176,   251,   137,
     420,   434,   435,   192,    70,   410,   152,   442,    82,   342,
     420,    90,   152,   442,   217,   206,   192,   193,   246,   256,
     368,   370,    91,   190,   195,   355,   356,   358,   381,   426,
     428,   446,    14,   102,   447,   349,   350,   351,   283,   284,
     408,   409,   191,   191,   191,   191,   191,   194,   223,   224,
     241,   248,   255,   408,   342,   196,   198,   199,   207,   454,
     455,   468,    38,   169,   285,   286,   342,   449,   190,   452,
     249,   239,   342,   342,   342,   342,    32,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   382,   342,   342,   430,   430,   342,   437,   438,   130,
     193,   208,   209,   427,   428,   259,   207,   260,   452,   452,
     258,   240,    38,   334,   337,   339,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   160,
     193,   207,   411,   412,   413,   414,   427,   430,   342,   285,
     285,   430,   342,   434,   239,   191,   342,   190,   404,     9,
     391,   191,   191,    38,   342,    38,   342,   385,   191,   191,
     191,   427,   285,   193,   207,   411,   412,   427,   191,   222,
     277,   193,   339,   342,   342,    94,    32,   224,   271,   192,
      28,   102,    14,     9,   191,    32,   193,   274,   468,    31,
      91,   220,   462,   463,   464,   190,     9,    50,    51,    56,
      58,    70,   138,   160,   180,   190,   218,   220,   363,   378,
     386,   387,   388,   207,   467,   222,   190,   232,   193,   192,
     193,   192,   102,   159,   111,   112,   159,   213,   214,   215,
     216,   217,   213,   207,   342,   288,   387,    83,     9,   191,
     191,   191,   191,   191,   191,   191,   192,    50,    51,   459,
     460,   461,   132,   264,   190,     9,   191,   191,    83,    85,
     207,   443,   207,    70,   194,   194,   203,   205,    32,   133,
     263,   175,    54,   160,   175,   372,   343,   136,     9,   391,
     191,   155,   468,   468,    14,   354,   283,   222,   188,     9,
     392,   468,   469,   410,   415,   410,   194,     9,   391,   177,
     420,   342,   191,     9,   392,    14,   346,   242,   132,   262,
     190,   452,   342,    32,   197,   197,   136,   194,     9,   391,
     342,   453,   190,   252,   247,   257,    14,   447,   250,   239,
      72,   420,   342,   453,   197,   194,   191,   191,   197,   194,
     191,    50,    51,    70,    78,    79,    80,    91,   138,   151,
     180,   207,   394,   396,   397,   400,   403,   207,   420,   420,
     136,   262,   410,   415,   191,   342,   278,    75,    76,   279,
     222,   331,   222,   333,   102,    38,   137,   268,   420,   387,
     207,    32,   224,   272,   192,   275,   192,   275,     9,   391,
      91,   136,   155,     9,   391,   191,   169,   454,   455,   456,
     454,   387,   387,   387,   387,   387,   390,   393,   190,   155,
     190,   387,   155,   193,    10,    11,    12,    31,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      69,   155,   453,   194,   378,   193,   236,   212,   212,   207,
     213,   213,   217,     9,   392,   194,   194,    14,   420,   192,
     177,     9,   391,   207,   265,   378,   193,   434,   137,   420,
      14,   197,   342,   194,   203,   468,   265,   193,   371,    14,
     191,   342,   355,   427,   192,   468,   188,   194,    32,   457,
     409,    38,    83,   169,   411,   412,   414,   411,   412,   468,
      38,   169,   342,   387,   283,   190,   378,   263,   347,   243,
     342,   342,   342,   194,   190,   285,   264,    32,   263,   468,
      14,   262,   452,   382,   194,   190,    14,    78,    79,    80,
     207,   395,   395,   397,   398,   399,    52,   190,    90,   152,
     190,     9,   391,   191,   404,    38,   342,   263,   194,    75,
      76,   280,   331,   224,   194,   192,    95,   192,   268,   420,
     190,   136,   267,    14,   222,   275,   105,   106,   107,   275,
     194,   468,   177,   136,   468,   207,   462,     9,   191,   391,
     136,   197,     9,   391,   390,   208,   355,   357,   359,   191,
     130,   208,   387,   439,   440,   387,   387,   387,    32,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   467,    83,   237,   194,   194,   216,   192,
     387,   461,   102,   103,   458,     9,   293,   191,   190,   334,
     339,   342,   197,   194,   447,   293,   161,   174,   193,   367,
     374,   161,   193,   373,   136,   192,   457,   468,   354,   469,
      83,   169,    14,    83,   453,   420,   342,   191,   283,   193,
     283,   190,   136,   190,   285,   191,   193,   468,   193,   192,
     468,   263,   244,   385,   285,   136,   197,     9,   391,   396,
     398,   152,   355,   401,   402,   397,   420,   193,   331,    32,
      77,   224,   192,   333,   267,   434,   268,   191,   387,   101,
     105,   192,   342,    32,   192,   276,   194,   177,   468,   136,
     169,    32,   191,   387,   387,   191,   136,     9,   391,   191,
     136,   194,     9,   391,   387,    32,   191,   222,   192,   192,
     207,   468,   468,   378,     4,   112,   117,   123,   125,   162,
     163,   165,   194,   294,   319,   320,   321,   326,   327,   328,
     329,   408,   434,   194,   193,   194,    54,   342,   342,   342,
     354,    38,    83,   169,    14,    83,   342,   190,   457,   191,
     293,   191,   283,   342,   285,   191,   293,   447,   293,   192,
     193,   190,   191,   397,   397,   191,   136,   191,     9,   391,
     293,    32,   222,   192,   191,   191,   191,   229,   192,   192,
     276,   222,   468,   468,   136,   387,   355,   387,   387,   387,
     193,   194,   458,   132,   133,   181,   208,   450,   468,   266,
     378,   112,   329,    31,   125,   138,   139,   160,   166,   303,
     304,   305,   306,   378,   164,   311,   312,   128,   190,   207,
     313,   314,   295,   240,   468,     9,   192,     9,   192,   192,
     447,   320,   191,   290,   160,   369,   194,   194,    83,   169,
      14,    83,   342,   285,   117,   344,   457,   194,   457,   191,
     191,   194,   193,   194,   293,   283,   136,   397,   355,   194,
     222,   227,   230,    32,   224,   270,   222,   191,   387,   136,
     136,   222,   378,   378,   452,    14,   208,     9,   192,   193,
     450,   447,   306,   176,   193,     9,   192,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   137,   138,
     140,   141,   142,   143,   144,   156,   157,   158,   168,   170,
     171,   173,   180,   181,   183,   185,   186,   207,   375,   376,
       9,   192,   160,   164,   207,   314,   315,   316,   192,    83,
     325,   239,   296,   450,   450,    14,   240,   194,   291,   292,
     450,    14,    83,   342,   191,   190,   457,   192,   193,   317,
     344,   457,   290,   194,   191,   397,   136,    32,   224,   269,
     270,   222,   387,   387,   194,   192,   192,   387,   378,   299,
     468,   307,   308,   386,   304,    14,    32,    51,   309,   312,
       9,    36,   191,    31,    50,    53,    14,     9,   192,   209,
     451,   325,    14,   468,   239,   192,    14,   342,    38,    83,
     366,   193,   222,   457,   317,   194,   457,   397,   222,    99,
     235,   194,   207,   220,   300,   301,   302,     9,   391,     9,
     391,   194,   387,   376,   376,    59,   310,   315,   315,    31,
      50,    53,   387,    83,   176,   190,   192,   387,   452,   387,
      83,     9,   392,   222,   194,   193,   317,    97,   192,   115,
     231,   155,   102,   468,   177,   386,   167,    14,   459,   297,
     190,    38,    83,   191,   194,   222,   192,   190,   173,   238,
     207,   320,   321,   177,   387,   177,   281,   282,   409,   298,
      83,   194,   378,   236,   170,   207,   192,   191,     9,   392,
     119,   120,   121,   323,   324,   281,    83,   266,   192,   457,
     409,   469,   191,   191,   192,   192,   193,   318,   323,    38,
      83,   169,   457,   193,   222,   469,    83,   169,    14,    83,
     318,   222,   194,    38,    83,   169,    14,    83,   342,   194,
      83,   169,    14,    83,   342,    14,    83,   342,   342
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
#line 730 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 733 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { (yyval).reset();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1095 "hphp.y"
    { (yyval).reset();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1163 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
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
#line 1181 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
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
#line 1198 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset();;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval).reset();;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { (yyval).reset();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { (yyval).reset();;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval).reset();;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { (yyval).reset();;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = 4;;}
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
#line 1684 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval).reset();;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval).reset();;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval).reset();;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval).reset();;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval).reset();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
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

  case 523:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
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

  case 525:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
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

  case 527:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 561:

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

  case 562:

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

  case 563:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 572:

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

  case 573:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval).reset();;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval).reset();;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval).reset();;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval).reset();;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval).reset();;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { (yyval).reset();;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval).reset();;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval).reset();;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval).reset();;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval).reset();;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval).reset();;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
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
#line 2733 "hphp.y"
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
#line 2748 "hphp.y"
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

  case 821:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
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

  case 828:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
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
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval).reset();;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval)++;;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
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

  case 873:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
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

  case 879:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { (yyval).reset();;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
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

  case 915:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    {;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    {;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    {;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3238 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3273 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { (yyvsp[(1) - (5)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)])); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3289 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3313 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13895 "hphp.5.tab.cpp"
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
#line 3316 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

