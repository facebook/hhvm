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
     T_VEC = 376,
     T_CALLABLE = 377,
     T_CLASS_C = 378,
     T_METHOD_C = 379,
     T_FUNC_C = 380,
     T_LINE = 381,
     T_FILE = 382,
     T_COMMENT = 383,
     T_DOC_COMMENT = 384,
     T_OPEN_TAG = 385,
     T_OPEN_TAG_WITH_ECHO = 386,
     T_CLOSE_TAG = 387,
     T_WHITESPACE = 388,
     T_START_HEREDOC = 389,
     T_END_HEREDOC = 390,
     T_DOLLAR_OPEN_CURLY_BRACES = 391,
     T_CURLY_OPEN = 392,
     T_DOUBLE_COLON = 393,
     T_NAMESPACE = 394,
     T_NS_C = 395,
     T_DIR = 396,
     T_NS_SEPARATOR = 397,
     T_XHP_LABEL = 398,
     T_XHP_TEXT = 399,
     T_XHP_ATTRIBUTE = 400,
     T_XHP_CATEGORY = 401,
     T_XHP_CATEGORY_LABEL = 402,
     T_XHP_CHILDREN = 403,
     T_ENUM = 404,
     T_XHP_REQUIRED = 405,
     T_TRAIT = 406,
     T_ELLIPSIS = 407,
     T_INSTEADOF = 408,
     T_TRAIT_C = 409,
     T_HH_ERROR = 410,
     T_FINALLY = 411,
     T_XHP_TAG_LT = 412,
     T_XHP_TAG_GT = 413,
     T_TYPELIST_LT = 414,
     T_TYPELIST_GT = 415,
     T_UNRESOLVED_LT = 416,
     T_COLLECTION = 417,
     T_SHAPE = 418,
     T_TYPE = 419,
     T_UNRESOLVED_TYPE = 420,
     T_NEWTYPE = 421,
     T_UNRESOLVED_NEWTYPE = 422,
     T_COMPILER_HALT_OFFSET = 423,
     T_ASYNC = 424,
     T_LAMBDA_OP = 425,
     T_LAMBDA_CP = 426,
     T_UNRESOLVED_OP = 427
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
#line 878 "hphp.5.tab.cpp"

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
#define YYLAST   18061

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  202
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  288
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1044
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1922

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   427

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   200,     2,   197,    55,    38,   201,
     192,   193,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   194,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   199,    37,     2,   198,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   195,    36,   196,    58,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191
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
     217,   219,   221,   223,   225,   227,   229,   231,   233,   235,
     238,   242,   246,   248,   251,   253,   256,   260,   265,   269,
     271,   274,   276,   279,   282,   284,   288,   290,   294,   297,
     300,   303,   309,   314,   317,   318,   320,   322,   324,   326,
     330,   336,   345,   346,   351,   352,   359,   360,   371,   372,
     377,   380,   384,   387,   391,   394,   398,   402,   406,   410,
     414,   418,   424,   426,   428,   430,   431,   441,   442,   453,
     459,   460,   474,   475,   481,   485,   489,   492,   495,   498,
     501,   504,   507,   511,   514,   517,   521,   524,   527,   528,
     533,   543,   544,   545,   550,   553,   554,   556,   557,   559,
     560,   570,   571,   582,   583,   595,   596,   606,   607,   618,
     619,   628,   629,   639,   640,   648,   649,   658,   659,   668,
     669,   677,   678,   687,   689,   691,   693,   695,   697,   700,
     704,   708,   711,   714,   715,   718,   719,   722,   723,   725,
     729,   731,   735,   738,   739,   741,   744,   749,   751,   756,
     758,   763,   765,   770,   772,   777,   781,   787,   791,   796,
     801,   807,   813,   818,   819,   821,   823,   828,   829,   835,
     836,   839,   840,   844,   845,   853,   862,   869,   872,   878,
     885,   890,   891,   896,   902,   910,   917,   924,   932,   942,
     951,   958,   966,   972,   975,   980,   986,   990,   991,   995,
    1000,  1007,  1013,  1019,  1026,  1035,  1043,  1046,  1047,  1049,
    1052,  1055,  1059,  1064,  1069,  1073,  1075,  1077,  1080,  1085,
    1089,  1095,  1097,  1101,  1104,  1105,  1108,  1112,  1115,  1116,
    1117,  1122,  1123,  1129,  1132,  1135,  1138,  1139,  1150,  1151,
    1163,  1167,  1171,  1175,  1180,  1185,  1189,  1195,  1198,  1201,
    1202,  1209,  1215,  1220,  1224,  1226,  1228,  1232,  1237,  1239,
    1242,  1244,  1246,  1252,  1259,  1261,  1263,  1268,  1270,  1272,
    1276,  1279,  1282,  1283,  1286,  1287,  1289,  1293,  1295,  1297,
    1299,  1301,  1305,  1310,  1315,  1320,  1322,  1324,  1327,  1330,
    1333,  1337,  1341,  1343,  1345,  1347,  1349,  1353,  1355,  1359,
    1361,  1363,  1365,  1366,  1368,  1371,  1373,  1375,  1377,  1379,
    1381,  1383,  1385,  1387,  1388,  1390,  1392,  1394,  1398,  1404,
    1406,  1410,  1416,  1421,  1425,  1429,  1433,  1438,  1442,  1446,
    1450,  1453,  1456,  1458,  1460,  1464,  1468,  1470,  1472,  1473,
    1475,  1478,  1483,  1487,  1491,  1498,  1501,  1505,  1508,  1512,
    1519,  1521,  1523,  1525,  1527,  1529,  1536,  1540,  1545,  1552,
    1556,  1560,  1564,  1568,  1572,  1576,  1580,  1584,  1588,  1592,
    1596,  1600,  1603,  1606,  1609,  1612,  1616,  1620,  1624,  1628,
    1632,  1636,  1640,  1644,  1648,  1652,  1656,  1660,  1664,  1668,
    1672,  1676,  1680,  1684,  1687,  1690,  1693,  1696,  1700,  1704,
    1708,  1712,  1716,  1720,  1724,  1728,  1732,  1736,  1740,  1746,
    1751,  1755,  1757,  1760,  1763,  1766,  1769,  1772,  1775,  1778,
    1781,  1784,  1786,  1788,  1790,  1792,  1794,  1798,  1801,  1803,
    1809,  1810,  1811,  1824,  1825,  1839,  1840,  1845,  1846,  1854,
    1855,  1861,  1862,  1866,  1867,  1874,  1877,  1880,  1885,  1887,
    1889,  1895,  1899,  1905,  1909,  1912,  1913,  1916,  1917,  1922,
    1927,  1931,  1934,  1935,  1941,  1945,  1952,  1957,  1960,  1961,
    1967,  1971,  1974,  1975,  1981,  1985,  1990,  1995,  2000,  2005,
    2010,  2015,  2018,  2019,  2022,  2023,  2026,  2027,  2032,  2037,
    2042,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2063,  2067,
    2072,  2074,  2077,  2082,  2085,  2092,  2093,  2095,  2100,  2101,
    2104,  2105,  2107,  2109,  2113,  2115,  2119,  2121,  2123,  2127,
    2131,  2133,  2135,  2137,  2139,  2141,  2143,  2145,  2147,  2149,
    2151,  2153,  2155,  2157,  2159,  2161,  2163,  2165,  2167,  2169,
    2171,  2173,  2175,  2177,  2179,  2181,  2183,  2185,  2187,  2189,
    2191,  2193,  2195,  2197,  2199,  2201,  2203,  2205,  2207,  2209,
    2211,  2213,  2215,  2217,  2219,  2221,  2223,  2225,  2227,  2229,
    2231,  2233,  2235,  2237,  2239,  2241,  2243,  2245,  2247,  2249,
    2251,  2253,  2255,  2257,  2259,  2261,  2263,  2265,  2267,  2269,
    2271,  2273,  2275,  2277,  2279,  2281,  2283,  2285,  2287,  2289,
    2291,  2293,  2295,  2297,  2302,  2307,  2312,  2314,  2316,  2318,
    2320,  2322,  2324,  2328,  2330,  2334,  2336,  2338,  2342,  2344,
    2346,  2348,  2351,  2353,  2354,  2355,  2357,  2359,  2363,  2364,
    2366,  2368,  2370,  2372,  2374,  2376,  2378,  2380,  2382,  2384,
    2386,  2388,  2390,  2394,  2397,  2399,  2401,  2406,  2410,  2415,
    2417,  2419,  2421,  2423,  2427,  2431,  2435,  2439,  2443,  2447,
    2451,  2455,  2459,  2463,  2467,  2471,  2475,  2479,  2483,  2487,
    2491,  2495,  2498,  2501,  2504,  2507,  2511,  2515,  2519,  2523,
    2527,  2531,  2535,  2539,  2543,  2549,  2554,  2558,  2560,  2564,
    2568,  2572,  2574,  2576,  2578,  2580,  2584,  2588,  2592,  2595,
    2596,  2598,  2599,  2601,  2602,  2608,  2612,  2616,  2618,  2620,
    2622,  2624,  2628,  2631,  2633,  2635,  2637,  2639,  2641,  2645,
    2647,  2649,  2651,  2654,  2657,  2662,  2666,  2671,  2673,  2675,
    2679,  2681,  2684,  2685,  2691,  2695,  2699,  2701,  2705,  2707,
    2710,  2711,  2717,  2721,  2724,  2725,  2729,  2730,  2735,  2738,
    2739,  2743,  2747,  2749,  2750,  2752,  2754,  2756,  2758,  2762,
    2764,  2766,  2768,  2772,  2774,  2776,  2780,  2784,  2787,  2792,
    2795,  2800,  2806,  2812,  2818,  2824,  2826,  2828,  2830,  2832,
    2834,  2836,  2840,  2844,  2849,  2854,  2858,  2860,  2862,  2864,
    2866,  2870,  2872,  2877,  2881,  2883,  2885,  2887,  2889,  2891,
    2895,  2899,  2904,  2909,  2913,  2915,  2917,  2925,  2935,  2943,
    2950,  2959,  2961,  2964,  2969,  2974,  2976,  2978,  2980,  2985,
    2987,  2988,  2990,  2993,  2995,  2997,  2999,  3003,  3007,  3011,
    3012,  3014,  3016,  3020,  3024,  3027,  3031,  3038,  3039,  3041,
    3046,  3049,  3050,  3056,  3060,  3064,  3066,  3073,  3078,  3083,
    3086,  3089,  3090,  3096,  3100,  3104,  3106,  3109,  3110,  3116,
    3120,  3124,  3126,  3129,  3132,  3134,  3137,  3139,  3144,  3148,
    3152,  3159,  3163,  3165,  3167,  3169,  3174,  3179,  3184,  3189,
    3194,  3199,  3202,  3205,  3210,  3213,  3216,  3218,  3222,  3226,
    3230,  3231,  3234,  3240,  3247,  3254,  3262,  3264,  3267,  3269,
    3272,  3274,  3279,  3281,  3286,  3290,  3291,  3293,  3297,  3300,
    3304,  3306,  3308,  3309,  3310,  3313,  3316,  3319,  3322,  3327,
    3330,  3336,  3340,  3342,  3344,  3345,  3349,  3354,  3360,  3364,
    3366,  3369,  3370,  3375,  3377,  3381,  3384,  3389,  3395,  3398,
    3401,  3403,  3405,  3407,  3409,  3411,  3413,  3417,  3420,  3423,
    3426,  3428,  3437,  3444,  3446
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     203,     0,    -1,    -1,   204,   205,    -1,   205,   206,    -1,
      -1,   226,    -1,   243,    -1,   250,    -1,   247,    -1,   257,
      -1,   467,    -1,   129,   192,   193,   194,    -1,   158,   219,
     194,    -1,    -1,   158,   219,   195,   207,   205,   196,    -1,
      -1,   158,   195,   208,   205,   196,    -1,   117,   214,   194,
      -1,   117,   111,   214,   194,    -1,   117,   112,   214,   194,
      -1,   117,   212,   195,   217,   196,   194,    -1,   117,   111,
     212,   195,   214,   196,   194,    -1,   117,   112,   212,   195,
     214,   196,   194,    -1,   223,   194,    -1,    81,    -1,   103,
      -1,   164,    -1,   165,    -1,   167,    -1,   169,    -1,   168,
      -1,   209,    -1,   141,    -1,   170,    -1,   132,    -1,   133,
      -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,   120,
      -1,   119,    -1,   112,    -1,   101,    -1,    97,    -1,    99,
      -1,    77,    -1,    95,    -1,    12,    -1,   118,    -1,   109,
      -1,    57,    -1,   172,    -1,   131,    -1,   158,    -1,    72,
      -1,    10,    -1,    11,    -1,   114,    -1,   117,    -1,   125,
      -1,    73,    -1,   137,    -1,    71,    -1,     7,    -1,     6,
      -1,   116,    -1,   138,    -1,    13,    -1,    92,    -1,     4,
      -1,     3,    -1,   113,    -1,    76,    -1,    75,    -1,   107,
      -1,   108,    -1,   110,    -1,   104,    -1,    27,    -1,    29,
      -1,   111,    -1,    74,    -1,   105,    -1,   175,    -1,    96,
      -1,    98,    -1,   100,    -1,   106,    -1,    93,    -1,    94,
      -1,   102,    -1,   115,    -1,   126,    -1,   139,    -1,   140,
      -1,   210,    -1,   130,    -1,   219,   161,    -1,   161,   219,
     161,    -1,   213,     9,   215,    -1,   215,    -1,   213,   410,
      -1,   219,    -1,   161,   219,    -1,   219,   102,   209,    -1,
     161,   219,   102,   209,    -1,   216,     9,   218,    -1,   218,
      -1,   216,   410,    -1,   215,    -1,   111,   215,    -1,   112,
     215,    -1,   209,    -1,   219,   161,   209,    -1,   219,    -1,
     158,   161,   219,    -1,   161,   219,    -1,   220,   472,    -1,
     220,   472,    -1,   223,     9,   468,    14,   404,    -1,   112,
     468,    14,   404,    -1,   224,   225,    -1,    -1,   226,    -1,
     243,    -1,   250,    -1,   257,    -1,   195,   224,   196,    -1,
      74,   333,   226,   279,   281,    -1,    74,   333,    32,   224,
     280,   282,    77,   194,    -1,    -1,    94,   333,   227,   273,
      -1,    -1,    93,   228,   226,    94,   333,   194,    -1,    -1,
      96,   192,   335,   194,   335,   194,   335,   193,   229,   271,
      -1,    -1,   104,   333,   230,   276,    -1,   108,   194,    -1,
     108,   344,   194,    -1,   110,   194,    -1,   110,   344,   194,
      -1,   113,   194,    -1,   113,   344,   194,    -1,    27,   108,
     194,    -1,   118,   289,   194,    -1,   124,   291,   194,    -1,
      92,   334,   194,    -1,   150,   334,   194,    -1,   126,   192,
     464,   193,   194,    -1,   194,    -1,    86,    -1,    87,    -1,
      -1,    98,   192,   344,   102,   270,   269,   193,   231,   272,
      -1,    -1,    98,   192,   344,    28,   102,   270,   269,   193,
     232,   272,    -1,   100,   192,   275,   193,   274,    -1,    -1,
     114,   235,   115,   192,   395,    83,   193,   195,   224,   196,
     237,   233,   240,    -1,    -1,   114,   235,   175,   234,   238,
      -1,   116,   344,   194,    -1,   109,   209,   194,    -1,   344,
     194,    -1,   336,   194,    -1,   337,   194,    -1,   338,   194,
      -1,   339,   194,    -1,   340,   194,    -1,   113,   339,   194,
      -1,   341,   194,    -1,   342,   194,    -1,   113,   341,   194,
      -1,   343,   194,    -1,   209,    32,    -1,    -1,   195,   236,
     224,   196,    -1,   237,   115,   192,   395,    83,   193,   195,
     224,   196,    -1,    -1,    -1,   195,   239,   224,   196,    -1,
     175,   238,    -1,    -1,    38,    -1,    -1,   111,    -1,    -1,
     242,   241,   471,   244,   192,   285,   193,   476,   319,    -1,
      -1,   323,   242,   241,   471,   245,   192,   285,   193,   476,
     319,    -1,    -1,   427,   322,   242,   241,   471,   246,   192,
     285,   193,   476,   319,    -1,    -1,   168,   209,   248,    32,
     488,   466,   195,   292,   196,    -1,    -1,   427,   168,   209,
     249,    32,   488,   466,   195,   292,   196,    -1,    -1,   263,
     260,   251,   264,   265,   195,   295,   196,    -1,    -1,   427,
     263,   260,   252,   264,   265,   195,   295,   196,    -1,    -1,
     131,   261,   253,   266,   195,   295,   196,    -1,    -1,   427,
     131,   261,   254,   266,   195,   295,   196,    -1,    -1,   130,
     256,   402,   264,   265,   195,   295,   196,    -1,    -1,   170,
     262,   258,   265,   195,   295,   196,    -1,    -1,   427,   170,
     262,   259,   265,   195,   295,   196,    -1,   471,    -1,   162,
      -1,   471,    -1,   471,    -1,   130,    -1,   123,   130,    -1,
     123,   122,   130,    -1,   122,   123,   130,    -1,   122,   130,
      -1,   132,   395,    -1,    -1,   133,   267,    -1,    -1,   132,
     267,    -1,    -1,   395,    -1,   267,     9,   395,    -1,   395,
      -1,   268,     9,   395,    -1,   136,   270,    -1,    -1,   439,
      -1,    38,   439,    -1,   137,   192,   453,   193,    -1,   226,
      -1,    32,   224,    97,   194,    -1,   226,    -1,    32,   224,
      99,   194,    -1,   226,    -1,    32,   224,    95,   194,    -1,
     226,    -1,    32,   224,   101,   194,    -1,   209,    14,   404,
      -1,   275,     9,   209,    14,   404,    -1,   195,   277,   196,
      -1,   195,   194,   277,   196,    -1,    32,   277,   105,   194,
      -1,    32,   194,   277,   105,   194,    -1,   277,   106,   344,
     278,   224,    -1,   277,   107,   278,   224,    -1,    -1,    32,
      -1,   194,    -1,   279,    75,   333,   226,    -1,    -1,   280,
      75,   333,    32,   224,    -1,    -1,    76,   226,    -1,    -1,
      76,    32,   224,    -1,    -1,   284,     9,   428,   325,   489,
     171,    83,    -1,   284,     9,   428,   325,   489,    38,   171,
      83,    -1,   284,     9,   428,   325,   489,   171,    -1,   284,
     410,    -1,   428,   325,   489,   171,    83,    -1,   428,   325,
     489,    38,   171,    83,    -1,   428,   325,   489,   171,    -1,
      -1,   428,   325,   489,    83,    -1,   428,   325,   489,    38,
      83,    -1,   428,   325,   489,    38,    83,    14,   344,    -1,
     428,   325,   489,    83,    14,   344,    -1,   284,     9,   428,
     325,   489,    83,    -1,   284,     9,   428,   325,   489,    38,
      83,    -1,   284,     9,   428,   325,   489,    38,    83,    14,
     344,    -1,   284,     9,   428,   325,   489,    83,    14,   344,
      -1,   286,     9,   428,   489,   171,    83,    -1,   286,     9,
     428,   489,    38,   171,    83,    -1,   286,     9,   428,   489,
     171,    -1,   286,   410,    -1,   428,   489,   171,    83,    -1,
     428,   489,    38,   171,    83,    -1,   428,   489,   171,    -1,
      -1,   428,   489,    83,    -1,   428,   489,    38,    83,    -1,
     428,   489,    38,    83,    14,   344,    -1,   428,   489,    83,
      14,   344,    -1,   286,     9,   428,   489,    83,    -1,   286,
       9,   428,   489,    38,    83,    -1,   286,     9,   428,   489,
      38,    83,    14,   344,    -1,   286,     9,   428,   489,    83,
      14,   344,    -1,   288,   410,    -1,    -1,   344,    -1,    38,
     439,    -1,   171,   344,    -1,   288,     9,   344,    -1,   288,
       9,   171,   344,    -1,   288,     9,    38,   439,    -1,   289,
       9,   290,    -1,   290,    -1,    83,    -1,   197,   439,    -1,
     197,   195,   344,   196,    -1,   291,     9,    83,    -1,   291,
       9,    83,    14,   404,    -1,    83,    -1,    83,    14,   404,
      -1,   292,   293,    -1,    -1,   294,   194,    -1,   469,    14,
     404,    -1,   295,   296,    -1,    -1,    -1,   321,   297,   327,
     194,    -1,    -1,   323,   488,   298,   327,   194,    -1,   328,
     194,    -1,   329,   194,    -1,   330,   194,    -1,    -1,   322,
     242,   241,   470,   192,   299,   283,   193,   476,   320,    -1,
      -1,   427,   322,   242,   241,   471,   192,   300,   283,   193,
     476,   320,    -1,   164,   305,   194,    -1,   165,   313,   194,
      -1,   167,   315,   194,    -1,     4,   132,   395,   194,    -1,
       4,   133,   395,   194,    -1,   117,   268,   194,    -1,   117,
     268,   195,   301,   196,    -1,   301,   302,    -1,   301,   303,
      -1,    -1,   222,   157,   209,   172,   268,   194,    -1,   304,
     102,   322,   209,   194,    -1,   304,   102,   323,   194,    -1,
     222,   157,   209,    -1,   209,    -1,   306,    -1,   305,     9,
     306,    -1,   307,   392,   311,   312,    -1,   162,    -1,    31,
     308,    -1,   308,    -1,   138,    -1,   138,   178,   488,   409,
     179,    -1,   138,   178,   488,     9,   488,   179,    -1,   395,
      -1,   125,    -1,   168,   195,   310,   196,    -1,   141,    -1,
     403,    -1,   309,     9,   403,    -1,   309,   409,    -1,    14,
     404,    -1,    -1,    59,   169,    -1,    -1,   314,    -1,   313,
       9,   314,    -1,   166,    -1,   316,    -1,   209,    -1,   128,
      -1,   192,   317,   193,    -1,   192,   317,   193,    53,    -1,
     192,   317,   193,    31,    -1,   192,   317,   193,    50,    -1,
     316,    -1,   318,    -1,   318,    53,    -1,   318,    31,    -1,
     318,    50,    -1,   317,     9,   317,    -1,   317,    36,   317,
      -1,   209,    -1,   162,    -1,   166,    -1,   194,    -1,   195,
     224,   196,    -1,   194,    -1,   195,   224,   196,    -1,   323,
      -1,   125,    -1,   323,    -1,    -1,   324,    -1,   323,   324,
      -1,   119,    -1,   120,    -1,   121,    -1,   124,    -1,   123,
      -1,   122,    -1,   188,    -1,   326,    -1,    -1,   119,    -1,
     120,    -1,   121,    -1,   327,     9,    83,    -1,   327,     9,
      83,    14,   404,    -1,    83,    -1,    83,    14,   404,    -1,
     328,     9,   469,    14,   404,    -1,   112,   469,    14,   404,
      -1,   329,     9,   469,    -1,   123,   112,   469,    -1,   123,
     331,   466,    -1,   331,   466,    14,   488,    -1,   112,   183,
     471,    -1,   192,   332,   193,    -1,    72,   399,   402,    -1,
      72,   255,    -1,    71,   344,    -1,   384,    -1,   379,    -1,
     192,   344,   193,    -1,   334,     9,   344,    -1,   344,    -1,
     334,    -1,    -1,    27,    -1,    27,   344,    -1,    27,   344,
     136,   344,    -1,   192,   336,   193,    -1,   439,    14,   336,
      -1,   137,   192,   453,   193,    14,   336,    -1,    29,   344,
      -1,   439,    14,   339,    -1,    28,   344,    -1,   439,    14,
     341,    -1,   137,   192,   453,   193,    14,   341,    -1,   345,
      -1,   439,    -1,   332,    -1,   443,    -1,   442,    -1,   137,
     192,   453,   193,    14,   344,    -1,   439,    14,   344,    -1,
     439,    14,    38,   439,    -1,   439,    14,    38,    72,   399,
     402,    -1,   439,    26,   344,    -1,   439,    25,   344,    -1,
     439,    24,   344,    -1,   439,    23,   344,    -1,   439,    22,
     344,    -1,   439,    21,   344,    -1,   439,    20,   344,    -1,
     439,    19,   344,    -1,   439,    18,   344,    -1,   439,    17,
     344,    -1,   439,    16,   344,    -1,   439,    15,   344,    -1,
     439,    68,    -1,    68,   439,    -1,   439,    67,    -1,    67,
     439,    -1,   344,    34,   344,    -1,   344,    35,   344,    -1,
     344,    10,   344,    -1,   344,    12,   344,    -1,   344,    11,
     344,    -1,   344,    36,   344,    -1,   344,    38,   344,    -1,
     344,    37,   344,    -1,   344,    52,   344,    -1,   344,    50,
     344,    -1,   344,    51,   344,    -1,   344,    53,   344,    -1,
     344,    54,   344,    -1,   344,    69,   344,    -1,   344,    55,
     344,    -1,   344,    30,   344,    -1,   344,    49,   344,    -1,
     344,    48,   344,    -1,    50,   344,    -1,    51,   344,    -1,
      56,   344,    -1,    58,   344,    -1,   344,    40,   344,    -1,
     344,    39,   344,    -1,   344,    42,   344,    -1,   344,    41,
     344,    -1,   344,    43,   344,    -1,   344,    47,   344,    -1,
     344,    44,   344,    -1,   344,    46,   344,    -1,   344,    45,
     344,    -1,   344,    57,   399,    -1,   192,   345,   193,    -1,
     344,    31,   344,    32,   344,    -1,   344,    31,    32,   344,
      -1,   344,    33,   344,    -1,   463,    -1,    66,   344,    -1,
      65,   344,    -1,    64,   344,    -1,    63,   344,    -1,    62,
     344,    -1,    61,   344,    -1,    60,   344,    -1,    73,   400,
      -1,    59,   344,    -1,   407,    -1,   363,    -1,   370,    -1,
     373,    -1,   362,    -1,   198,   401,   198,    -1,    13,   344,
      -1,   381,    -1,   117,   192,   383,   410,   193,    -1,    -1,
      -1,   242,   241,   192,   348,   285,   193,   476,   346,   476,
     195,   224,   196,    -1,    -1,   323,   242,   241,   192,   349,
     285,   193,   476,   346,   476,   195,   224,   196,    -1,    -1,
     188,    83,   351,   356,    -1,    -1,   188,   189,   352,   285,
     190,   476,   356,    -1,    -1,   188,   195,   353,   224,   196,
      -1,    -1,    83,   354,   356,    -1,    -1,   189,   355,   285,
     190,   476,   356,    -1,     8,   344,    -1,     8,   341,    -1,
       8,   195,   224,   196,    -1,    91,    -1,   465,    -1,   358,
       9,   357,   136,   344,    -1,   357,   136,   344,    -1,   359,
       9,   357,   136,   404,    -1,   357,   136,   404,    -1,   358,
     409,    -1,    -1,   359,   409,    -1,    -1,   182,   192,   360,
     193,    -1,   138,   192,   454,   193,    -1,    70,   454,   199,
      -1,   365,   409,    -1,    -1,   365,     9,   344,   136,   344,
      -1,   344,   136,   344,    -1,   365,     9,   344,   136,    38,
     439,    -1,   344,   136,    38,   439,    -1,   367,   409,    -1,
      -1,   367,     9,   404,   136,   404,    -1,   404,   136,   404,
      -1,   369,   409,    -1,    -1,   369,     9,   415,   136,   415,
      -1,   415,   136,   415,    -1,   139,    70,   364,   199,    -1,
     139,    70,   366,   199,    -1,   139,    70,   368,   199,    -1,
     140,    70,   376,   199,    -1,   140,    70,   377,   199,    -1,
     140,    70,   378,   199,    -1,   334,   409,    -1,    -1,   405,
     409,    -1,    -1,   416,   409,    -1,    -1,   395,   195,   456,
     196,    -1,   395,   195,   458,   196,    -1,   381,    70,   449,
     199,    -1,   382,    70,   449,   199,    -1,   363,    -1,   370,
      -1,   373,    -1,   465,    -1,   442,    -1,    91,    -1,   192,
     345,   193,    -1,   383,     9,    83,    -1,   383,     9,    38,
      83,    -1,    83,    -1,    38,    83,    -1,   176,   162,   385,
     177,    -1,   387,    54,    -1,   387,   177,   388,   176,    54,
     386,    -1,    -1,   162,    -1,   387,   389,    14,   390,    -1,
      -1,   388,   391,    -1,    -1,   162,    -1,   163,    -1,   195,
     344,   196,    -1,   163,    -1,   195,   344,   196,    -1,   384,
      -1,   393,    -1,   392,    32,   393,    -1,   392,    51,   393,
      -1,   209,    -1,    73,    -1,   111,    -1,   112,    -1,   113,
      -1,    27,    -1,    29,    -1,    28,    -1,   114,    -1,   115,
      -1,   175,    -1,   116,    -1,    74,    -1,    75,    -1,    77,
      -1,    76,    -1,    94,    -1,    95,    -1,    93,    -1,    96,
      -1,    97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,
      -1,    57,    -1,   102,    -1,   104,    -1,   105,    -1,   106,
      -1,   107,    -1,   108,    -1,   110,    -1,   109,    -1,    92,
      -1,    13,    -1,   130,    -1,   131,    -1,   132,    -1,   133,
      -1,    72,    -1,    71,    -1,   125,    -1,     5,    -1,     7,
      -1,     6,    -1,     4,    -1,     3,    -1,   158,    -1,   117,
      -1,   118,    -1,   127,    -1,   128,    -1,   129,    -1,   124,
      -1,   123,    -1,   122,    -1,   121,    -1,   120,    -1,   119,
      -1,   188,    -1,   126,    -1,   137,    -1,   138,    -1,    10,
      -1,    12,    -1,    11,    -1,   142,    -1,   144,    -1,   143,
      -1,   145,    -1,   146,    -1,   160,    -1,   159,    -1,   187,
      -1,   170,    -1,   173,    -1,   172,    -1,   183,    -1,   185,
      -1,   182,    -1,   139,    -1,   140,    -1,   221,   192,   287,
     193,    -1,   139,   192,   287,   193,    -1,   140,   192,   287,
     193,    -1,   222,    -1,   162,    -1,   395,    -1,   403,    -1,
     124,    -1,   447,    -1,   192,   345,   193,    -1,   396,    -1,
     397,   157,   446,    -1,   396,    -1,   445,    -1,   398,   157,
     446,    -1,   395,    -1,   124,    -1,   451,    -1,   192,   193,
      -1,   333,    -1,    -1,    -1,    90,    -1,   460,    -1,   192,
     287,   193,    -1,    -1,    78,    -1,    79,    -1,    80,    -1,
      91,    -1,   145,    -1,   146,    -1,   160,    -1,   142,    -1,
     173,    -1,   143,    -1,   144,    -1,   159,    -1,   187,    -1,
     153,    90,   154,    -1,   153,   154,    -1,   403,    -1,   220,
      -1,   138,   192,   408,   193,    -1,    70,   408,   199,    -1,
     182,   192,   361,   193,    -1,   371,    -1,   374,    -1,   406,
      -1,   380,    -1,   192,   404,   193,    -1,   404,    34,   404,
      -1,   404,    35,   404,    -1,   404,    10,   404,    -1,   404,
      12,   404,    -1,   404,    11,   404,    -1,   404,    36,   404,
      -1,   404,    38,   404,    -1,   404,    37,   404,    -1,   404,
      52,   404,    -1,   404,    50,   404,    -1,   404,    51,   404,
      -1,   404,    53,   404,    -1,   404,    54,   404,    -1,   404,
      55,   404,    -1,   404,    49,   404,    -1,   404,    48,   404,
      -1,   404,    69,   404,    -1,    56,   404,    -1,    58,   404,
      -1,    50,   404,    -1,    51,   404,    -1,   404,    40,   404,
      -1,   404,    39,   404,    -1,   404,    42,   404,    -1,   404,
      41,   404,    -1,   404,    43,   404,    -1,   404,    47,   404,
      -1,   404,    44,   404,    -1,   404,    46,   404,    -1,   404,
      45,   404,    -1,   404,    31,   404,    32,   404,    -1,   404,
      31,    32,   404,    -1,   405,     9,   404,    -1,   404,    -1,
     222,   157,   210,    -1,   162,   157,   210,    -1,   222,   157,
     130,    -1,   220,    -1,    82,    -1,   465,    -1,   403,    -1,
     200,   460,   200,    -1,   201,   460,   201,    -1,   153,   460,
     154,    -1,   411,   409,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   411,     9,   404,   136,   404,    -1,   411,     9,
     404,    -1,   404,   136,   404,    -1,   404,    -1,    78,    -1,
      79,    -1,    80,    -1,   153,    90,   154,    -1,   153,   154,
      -1,    78,    -1,    79,    -1,    80,    -1,   209,    -1,    91,
      -1,    91,    52,   414,    -1,   412,    -1,   414,    -1,   209,
      -1,    50,   413,    -1,    51,   413,    -1,   138,   192,   417,
     193,    -1,    70,   417,   199,    -1,   182,   192,   420,   193,
      -1,   372,    -1,   375,    -1,   416,     9,   415,    -1,   415,
      -1,   418,   409,    -1,    -1,   418,     9,   415,   136,   415,
      -1,   418,     9,   415,    -1,   415,   136,   415,    -1,   415,
      -1,   419,     9,   415,    -1,   415,    -1,   421,   409,    -1,
      -1,   421,     9,   357,   136,   415,    -1,   357,   136,   415,
      -1,   419,   409,    -1,    -1,   192,   422,   193,    -1,    -1,
     424,     9,   209,   423,    -1,   209,   423,    -1,    -1,   426,
     424,   409,    -1,    49,   425,    48,    -1,   427,    -1,    -1,
     134,    -1,   135,    -1,   209,    -1,   162,    -1,   195,   344,
     196,    -1,   430,    -1,   446,    -1,   209,    -1,   195,   344,
     196,    -1,   432,    -1,   446,    -1,    70,   449,   199,    -1,
     195,   344,   196,    -1,   440,   434,    -1,   192,   332,   193,
     434,    -1,   452,   434,    -1,   192,   332,   193,   434,    -1,
     192,   332,   193,   429,   431,    -1,   192,   345,   193,   429,
     431,    -1,   192,   332,   193,   429,   430,    -1,   192,   345,
     193,   429,   430,    -1,   446,    -1,   394,    -1,   444,    -1,
     445,    -1,   435,    -1,   437,    -1,   439,   429,   431,    -1,
     398,   157,   446,    -1,   441,   192,   287,   193,    -1,   442,
     192,   287,   193,    -1,   192,   439,   193,    -1,   394,    -1,
     444,    -1,   445,    -1,   435,    -1,   439,   429,   430,    -1,
     438,    -1,   441,   192,   287,   193,    -1,   192,   439,   193,
      -1,   446,    -1,   435,    -1,   394,    -1,   363,    -1,   403,
      -1,   192,   439,   193,    -1,   192,   345,   193,    -1,   442,
     192,   287,   193,    -1,   441,   192,   287,   193,    -1,   192,
     443,   193,    -1,   347,    -1,   350,    -1,   439,   429,   433,
     472,   192,   287,   193,    -1,   192,   332,   193,   429,   433,
     472,   192,   287,   193,    -1,   398,   157,   211,   472,   192,
     287,   193,    -1,   398,   157,   446,   192,   287,   193,    -1,
     398,   157,   195,   344,   196,   192,   287,   193,    -1,   447,
      -1,   450,   447,    -1,   447,    70,   449,   199,    -1,   447,
     195,   344,   196,    -1,   448,    -1,    83,    -1,    84,    -1,
     197,   195,   344,   196,    -1,   344,    -1,    -1,   197,    -1,
     450,   197,    -1,   446,    -1,   436,    -1,   437,    -1,   451,
     429,   431,    -1,   397,   157,   446,    -1,   192,   439,   193,
      -1,    -1,   436,    -1,   438,    -1,   451,   429,   430,    -1,
     192,   439,   193,    -1,   453,     9,    -1,   453,     9,   439,
      -1,   453,     9,   137,   192,   453,   193,    -1,    -1,   439,
      -1,   137,   192,   453,   193,    -1,   455,   409,    -1,    -1,
     455,     9,   344,   136,   344,    -1,   455,     9,   344,    -1,
     344,   136,   344,    -1,   344,    -1,   455,     9,   344,   136,
      38,   439,    -1,   455,     9,    38,   439,    -1,   344,   136,
      38,   439,    -1,    38,   439,    -1,   457,   409,    -1,    -1,
     457,     9,   344,   136,   344,    -1,   457,     9,   344,    -1,
     344,   136,   344,    -1,   344,    -1,   459,   409,    -1,    -1,
     459,     9,   404,   136,   404,    -1,   459,     9,   404,    -1,
     404,   136,   404,    -1,   404,    -1,   460,   461,    -1,   460,
      90,    -1,   461,    -1,    90,   461,    -1,    83,    -1,    83,
      70,   462,   199,    -1,    83,   429,   209,    -1,   155,   344,
     196,    -1,   155,    82,    70,   344,   199,   196,    -1,   156,
     439,   196,    -1,   209,    -1,    85,    -1,    83,    -1,   127,
     192,   334,   193,    -1,   128,   192,   439,   193,    -1,   128,
     192,   345,   193,    -1,   128,   192,   443,   193,    -1,   128,
     192,   442,   193,    -1,   128,   192,   332,   193,    -1,     7,
     344,    -1,     6,   344,    -1,     5,   192,   344,   193,    -1,
       4,   344,    -1,     3,   344,    -1,   439,    -1,   464,     9,
     439,    -1,   398,   157,   210,    -1,   398,   157,   130,    -1,
      -1,   102,   488,    -1,   183,   471,    14,   488,   194,    -1,
     427,   183,   471,    14,   488,   194,    -1,   185,   471,   466,
      14,   488,   194,    -1,   427,   185,   471,   466,    14,   488,
     194,    -1,   211,    -1,   488,   211,    -1,   210,    -1,   488,
     210,    -1,   211,    -1,   211,   178,   478,   179,    -1,   209,
      -1,   209,   178,   478,   179,    -1,   178,   474,   179,    -1,
      -1,   488,    -1,   473,     9,   488,    -1,   473,   409,    -1,
     473,     9,   171,    -1,   474,    -1,   171,    -1,    -1,    -1,
      32,   488,    -1,   102,   488,    -1,   103,   488,    -1,   479,
     409,    -1,   479,     9,   480,   209,    -1,   480,   209,    -1,
     479,     9,   480,   209,   477,    -1,   480,   209,   477,    -1,
      50,    -1,    51,    -1,    -1,    91,   136,   488,    -1,    31,
      91,   136,   488,    -1,   222,   157,   209,   136,   488,    -1,
     482,     9,   481,    -1,   481,    -1,   482,   409,    -1,    -1,
     182,   192,   483,   193,    -1,   222,    -1,   209,   157,   486,
      -1,   209,   472,    -1,   178,   488,   409,   179,    -1,   178,
     488,     9,   488,   179,    -1,    31,   488,    -1,    59,   488,
      -1,   222,    -1,   138,    -1,   139,    -1,   140,    -1,   141,
      -1,   484,    -1,   485,   157,   486,    -1,   138,   487,    -1,
     139,   487,    -1,   140,   487,    -1,   162,    -1,   192,   111,
     192,   475,   193,    32,   488,   193,    -1,   192,   488,     9,
     473,   409,   193,    -1,   488,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   732,   732,   732,   741,   743,   746,   747,   748,   749,
     750,   751,   752,   755,   757,   757,   759,   759,   761,   763,
     766,   769,   773,   777,   781,   786,   787,   788,   789,   790,
     791,   792,   796,   797,   798,   799,   800,   801,   802,   803,
     804,   805,   806,   807,   808,   809,   810,   811,   812,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   823,
     824,   825,   826,   827,   828,   829,   830,   831,   832,   833,
     834,   835,   836,   837,   838,   839,   840,   841,   842,   843,
     844,   845,   846,   847,   848,   849,   850,   851,   852,   853,
     854,   855,   856,   857,   861,   862,   863,   867,   868,   872,
     873,   878,   880,   885,   890,   891,   892,   894,   899,   901,
     906,   911,   913,   915,   920,   921,   925,   926,   928,   932,
     939,   946,   950,   956,   958,   961,   962,   963,   964,   967,
     968,   972,   977,   977,   983,   983,   990,   989,   995,   995,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1018,  1016,  1025,  1023,  1030,
    1040,  1034,  1044,  1042,  1046,  1047,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1070,  1070,
    1075,  1081,  1085,  1085,  1093,  1094,  1098,  1099,  1103,  1109,
    1107,  1122,  1119,  1135,  1132,  1149,  1148,  1157,  1155,  1167,
    1166,  1185,  1183,  1202,  1201,  1210,  1208,  1219,  1219,  1226,
    1225,  1237,  1235,  1248,  1249,  1253,  1256,  1259,  1260,  1261,
    1264,  1265,  1268,  1270,  1273,  1274,  1277,  1278,  1281,  1282,
    1286,  1287,  1292,  1293,  1296,  1297,  1298,  1302,  1303,  1307,
    1308,  1312,  1313,  1317,  1318,  1323,  1324,  1330,  1331,  1332,
    1333,  1336,  1339,  1341,  1344,  1345,  1349,  1351,  1354,  1357,
    1360,  1361,  1364,  1365,  1369,  1375,  1381,  1388,  1390,  1395,
    1400,  1406,  1410,  1414,  1418,  1423,  1428,  1433,  1438,  1444,
    1453,  1458,  1463,  1469,  1471,  1475,  1479,  1484,  1488,  1491,
    1494,  1498,  1502,  1506,  1510,  1515,  1523,  1525,  1528,  1529,
    1530,  1531,  1533,  1535,  1540,  1541,  1544,  1545,  1546,  1550,
    1551,  1553,  1554,  1558,  1560,  1563,  1567,  1573,  1575,  1578,
    1578,  1582,  1581,  1585,  1587,  1590,  1593,  1591,  1607,  1603,
    1617,  1619,  1621,  1623,  1625,  1627,  1629,  1633,  1634,  1635,
    1638,  1644,  1648,  1654,  1657,  1662,  1664,  1669,  1674,  1678,
    1679,  1683,  1684,  1686,  1688,  1694,  1695,  1697,  1701,  1702,
    1707,  1711,  1712,  1716,  1717,  1721,  1723,  1729,  1734,  1735,
    1737,  1741,  1742,  1743,  1744,  1748,  1749,  1750,  1751,  1752,
    1753,  1755,  1760,  1763,  1764,  1768,  1769,  1773,  1774,  1777,
    1778,  1781,  1782,  1785,  1786,  1790,  1791,  1792,  1793,  1794,
    1795,  1796,  1800,  1801,  1804,  1805,  1806,  1809,  1811,  1813,
    1814,  1817,  1819,  1823,  1825,  1829,  1833,  1837,  1842,  1843,
    1845,  1846,  1847,  1848,  1851,  1855,  1856,  1860,  1861,  1865,
    1866,  1867,  1868,  1872,  1876,  1881,  1885,  1889,  1893,  1897,
    1902,  1903,  1904,  1905,  1906,  1910,  1912,  1913,  1914,  1917,
    1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,  1927,
    1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,  1937,
    1938,  1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,  1947,
    1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,
    1958,  1959,  1960,  1962,  1963,  1965,  1966,  1968,  1969,  1970,
    1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,  1979,  1980,
    1981,  1982,  1983,  1984,  1985,  1986,  1987,  1988,  1989,  1993,
    1997,  2002,  2001,  2016,  2014,  2032,  2031,  2050,  2049,  2068,
    2067,  2085,  2085,  2100,  2100,  2118,  2119,  2120,  2125,  2127,
    2131,  2135,  2141,  2145,  2151,  2153,  2157,  2159,  2163,  2167,
    2168,  2172,  2174,  2178,  2180,  2181,  2184,  2188,  2190,  2194,
    2197,  2202,  2204,  2208,  2211,  2216,  2220,  2224,  2228,  2232,
    2236,  2240,  2242,  2246,  2248,  2252,  2254,  2258,  2265,  2272,
    2274,  2279,  2280,  2281,  2282,  2283,  2284,  2286,  2290,  2291,
    2292,  2293,  2297,  2303,  2312,  2325,  2326,  2329,  2332,  2335,
    2336,  2339,  2343,  2346,  2349,  2356,  2357,  2361,  2362,  2364,
    2369,  2370,  2371,  2372,  2373,  2374,  2375,  2376,  2377,  2378,
    2379,  2380,  2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,
    2389,  2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,
    2399,  2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,
    2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,
    2419,  2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,
    2429,  2430,  2431,  2432,  2433,  2434,  2435,  2436,  2437,  2438,
    2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,  2447,  2448,
    2449,  2450,  2451,  2455,  2457,  2460,  2466,  2467,  2471,  2472,
    2473,  2474,  2476,  2480,  2481,  2492,  2493,  2495,  2507,  2508,
    2509,  2513,  2514,  2515,  2519,  2520,  2521,  2524,  2526,  2530,
    2531,  2532,  2533,  2535,  2536,  2537,  2538,  2539,  2540,  2541,
    2542,  2543,  2544,  2547,  2552,  2553,  2554,  2556,  2557,  2559,
    2560,  2561,  2562,  2563,  2564,  2566,  2568,  2570,  2572,  2574,
    2575,  2576,  2577,  2578,  2579,  2580,  2581,  2582,  2583,  2584,
    2585,  2586,  2587,  2588,  2589,  2590,  2592,  2594,  2596,  2598,
    2599,  2602,  2603,  2607,  2611,  2613,  2617,  2618,  2622,  2625,
    2628,  2634,  2635,  2636,  2637,  2638,  2639,  2640,  2645,  2647,
    2651,  2652,  2655,  2656,  2660,  2663,  2665,  2667,  2671,  2672,
    2673,  2674,  2677,  2681,  2682,  2683,  2684,  2688,  2690,  2697,
    2698,  2699,  2700,  2701,  2702,  2704,  2705,  2707,  2708,  2712,
    2714,  2718,  2720,  2723,  2726,  2728,  2730,  2733,  2735,  2739,
    2741,  2744,  2747,  2753,  2755,  2758,  2759,  2764,  2767,  2771,
    2771,  2776,  2779,  2780,  2784,  2785,  2789,  2790,  2791,  2795,
    2797,  2805,  2806,  2810,  2812,  2820,  2821,  2825,  2826,  2831,
    2833,  2838,  2849,  2863,  2875,  2890,  2891,  2892,  2893,  2894,
    2895,  2896,  2906,  2915,  2917,  2919,  2923,  2924,  2925,  2926,
    2927,  2943,  2944,  2946,  2955,  2956,  2957,  2958,  2959,  2960,
    2961,  2962,  2964,  2969,  2973,  2974,  2978,  2981,  2988,  2992,
    3001,  3008,  3010,  3016,  3018,  3019,  3023,  3024,  3025,  3032,
    3033,  3038,  3039,  3044,  3045,  3046,  3047,  3058,  3061,  3064,
    3065,  3066,  3067,  3078,  3082,  3083,  3084,  3086,  3087,  3088,
    3092,  3094,  3097,  3099,  3100,  3101,  3102,  3105,  3107,  3108,
    3112,  3114,  3117,  3119,  3120,  3121,  3125,  3127,  3130,  3133,
    3135,  3137,  3141,  3142,  3144,  3145,  3151,  3152,  3154,  3164,
    3166,  3168,  3171,  3172,  3173,  3177,  3178,  3179,  3180,  3181,
    3182,  3183,  3184,  3185,  3186,  3187,  3191,  3192,  3196,  3198,
    3206,  3208,  3212,  3216,  3221,  3225,  3233,  3234,  3238,  3239,
    3245,  3246,  3255,  3256,  3264,  3267,  3271,  3274,  3279,  3284,
    3286,  3287,  3288,  3292,  3293,  3297,  3298,  3301,  3306,  3309,
    3311,  3315,  3321,  3322,  3323,  3327,  3331,  3341,  3349,  3351,
    3355,  3357,  3362,  3368,  3371,  3376,  3381,  3383,  3390,  3393,
    3396,  3397,  3400,  3403,  3406,  3409,  3410,  3415,  3417,  3419,
    3421,  3425,  3431,  3441,  3442
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
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_VEC", "T_CALLABLE",
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
  "static_dict_literal", "static_dict_literal_ae", "vec_literal",
  "static_vec_literal", "static_vec_literal_ae", "vec_expr_list",
  "static_vec_expr_list", "static_vec_expr_list_ae", "collection_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
  "static_class_name_base", "static_class_name_no_calls",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_expr_list", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_string_expr_ae", "static_scalar_ae",
  "static_scalar_ae_list", "static_array_pair_list_ae",
  "non_empty_static_array_pair_list_ae", "non_empty_static_scalar_list_ae",
  "static_shape_pair_list_ae", "non_empty_static_shape_pair_list_ae",
  "static_scalar_list_ae", "attribute_static_scalar_list",
  "non_empty_user_attribute_list", "user_attribute_list", "$@37",
  "non_empty_user_attributes", "optional_user_attributes",
  "object_operator", "object_property_name_no_variables",
  "object_property_name", "object_method_name_no_variables",
  "object_method_name", "array_access", "dimmable_variable_access",
  "dimmable_variable_no_calls_access", "object_property_access_on_expr",
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
     426,   427,    40,    41,    59,   123,   125,    36,    96,    93,
      34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   202,   204,   203,   205,   205,   206,   206,   206,   206,
     206,   206,   206,   206,   207,   206,   208,   206,   206,   206,
     206,   206,   206,   206,   206,   209,   209,   209,   209,   209,
     209,   209,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   211,   211,   212,
     212,   213,   213,   214,   215,   215,   215,   215,   216,   216,
     217,   218,   218,   218,   219,   219,   220,   220,   220,   221,
     222,   223,   223,   224,   224,   225,   225,   225,   225,   226,
     226,   226,   227,   226,   228,   226,   229,   226,   230,   226,
     226,   226,   226,   226,   226,   226,   226,   226,   226,   226,
     226,   226,   226,   226,   226,   231,   226,   232,   226,   226,
     233,   226,   234,   226,   226,   226,   226,   226,   226,   226,
     226,   226,   226,   226,   226,   226,   226,   226,   236,   235,
     237,   237,   239,   238,   240,   240,   241,   241,   242,   244,
     243,   245,   243,   246,   243,   248,   247,   249,   247,   251,
     250,   252,   250,   253,   250,   254,   250,   256,   255,   258,
     257,   259,   257,   260,   260,   261,   262,   263,   263,   263,
     263,   263,   264,   264,   265,   265,   266,   266,   267,   267,
     268,   268,   269,   269,   270,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   275,   275,   276,   276,   276,
     276,   277,   277,   277,   278,   278,   279,   279,   280,   280,
     281,   281,   282,   282,   283,   283,   283,   283,   283,   283,
     283,   283,   284,   284,   284,   284,   284,   284,   284,   284,
     285,   285,   285,   285,   285,   285,   285,   285,   286,   286,
     286,   286,   286,   286,   286,   286,   287,   287,   288,   288,
     288,   288,   288,   288,   289,   289,   290,   290,   290,   291,
     291,   291,   291,   292,   292,   293,   294,   295,   295,   297,
     296,   298,   296,   296,   296,   296,   299,   296,   300,   296,
     296,   296,   296,   296,   296,   296,   296,   301,   301,   301,
     302,   303,   303,   304,   304,   305,   305,   306,   306,   307,
     307,   308,   308,   308,   308,   308,   308,   308,   309,   309,
     310,   311,   311,   312,   312,   313,   313,   314,   315,   315,
     315,   316,   316,   316,   316,   317,   317,   317,   317,   317,
     317,   317,   318,   318,   318,   319,   319,   320,   320,   321,
     321,   322,   322,   323,   323,   324,   324,   324,   324,   324,
     324,   324,   325,   325,   326,   326,   326,   327,   327,   327,
     327,   328,   328,   329,   329,   330,   330,   331,   332,   332,
     332,   332,   332,   332,   333,   334,   334,   335,   335,   336,
     336,   336,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   344,   344,   344,   344,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   346,
     346,   348,   347,   349,   347,   351,   350,   352,   350,   353,
     350,   354,   350,   355,   350,   356,   356,   356,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   361,   362,   363,
     363,   364,   364,   365,   365,   365,   365,   366,   366,   367,
     367,   368,   368,   369,   369,   370,   371,   372,   373,   374,
     375,   376,   376,   377,   377,   378,   378,   379,   380,   381,
     381,   382,   382,   382,   382,   382,   382,   382,   383,   383,
     383,   383,   384,   385,   385,   386,   386,   387,   387,   388,
     388,   389,   390,   390,   391,   391,   391,   392,   392,   392,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   394,   394,   394,   395,   395,   396,   396,
     396,   396,   396,   397,   397,   398,   398,   398,   399,   399,
     399,   400,   400,   400,   401,   401,   401,   402,   402,   403,
     403,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   405,   405,   406,   406,
     406,   407,   407,   407,   407,   407,   407,   407,   408,   408,
     409,   409,   410,   410,   411,   411,   411,   411,   412,   412,
     412,   412,   412,   413,   413,   413,   413,   414,   414,   415,
     415,   415,   415,   415,   415,   415,   415,   415,   415,   416,
     416,   417,   417,   418,   418,   418,   418,   419,   419,   420,
     420,   421,   421,   422,   422,   423,   423,   424,   424,   426,
     425,   427,   428,   428,   429,   429,   430,   430,   430,   431,
     431,   432,   432,   433,   433,   434,   434,   435,   435,   436,
     436,   437,   437,   438,   438,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   440,   440,   440,   440,
     440,   440,   440,   440,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   442,   443,   443,   444,   444,   445,   445,
     445,   446,   446,   447,   447,   447,   448,   448,   448,   449,
     449,   450,   450,   451,   451,   451,   451,   451,   451,   452,
     452,   452,   452,   452,   453,   453,   453,   453,   453,   453,
     454,   454,   455,   455,   455,   455,   455,   455,   455,   455,
     456,   456,   457,   457,   457,   457,   458,   458,   459,   459,
     459,   459,   460,   460,   460,   460,   461,   461,   461,   461,
     461,   461,   462,   462,   462,   463,   463,   463,   463,   463,
     463,   463,   463,   463,   463,   463,   464,   464,   465,   465,
     466,   466,   467,   467,   467,   467,   468,   468,   469,   469,
     470,   470,   471,   471,   472,   472,   473,   473,   474,   475,
     475,   475,   475,   476,   476,   477,   477,   478,   479,   479,
     479,   479,   480,   480,   480,   481,   481,   481,   482,   482,
     483,   483,   484,   485,   486,   486,   487,   487,   488,   488,
     488,   488,   488,   488,   488,   488,   488,   488,   488,   488,
     488,   488,   488,   489,   489
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     1,     2,     1,     2,     3,     4,     3,     1,
       2,     1,     2,     2,     1,     3,     1,     3,     2,     2,
       2,     5,     4,     2,     0,     1,     1,     1,     1,     3,
       5,     8,     0,     4,     0,     6,     0,    10,     0,     4,
       2,     3,     2,     3,     2,     3,     3,     3,     3,     3,
       3,     5,     1,     1,     1,     0,     9,     0,    10,     5,
       0,    13,     0,     5,     3,     3,     2,     2,     2,     2,
       2,     2,     3,     2,     2,     3,     2,     2,     0,     4,
       9,     0,     0,     4,     2,     0,     1,     0,     1,     0,
       9,     0,    10,     0,    11,     0,     9,     0,    10,     0,
       8,     0,     9,     0,     7,     0,     8,     0,     8,     0,
       7,     0,     8,     1,     1,     1,     1,     1,     2,     3,
       3,     2,     2,     0,     2,     0,     2,     0,     1,     3,
       1,     3,     2,     0,     1,     2,     4,     1,     4,     1,
       4,     1,     4,     1,     4,     3,     5,     3,     4,     4,
       5,     5,     4,     0,     1,     1,     4,     0,     5,     0,
       2,     0,     3,     0,     7,     8,     6,     2,     5,     6,
       4,     0,     4,     5,     7,     6,     6,     7,     9,     8,
       6,     7,     5,     2,     4,     5,     3,     0,     3,     4,
       6,     5,     5,     6,     8,     7,     2,     0,     1,     2,
       2,     3,     4,     4,     3,     1,     1,     2,     4,     3,
       5,     1,     3,     2,     0,     2,     3,     2,     0,     0,
       4,     0,     5,     2,     2,     2,     0,    10,     0,    11,
       3,     3,     3,     4,     4,     3,     5,     2,     2,     0,
       6,     5,     4,     3,     1,     1,     3,     4,     1,     2,
       1,     1,     5,     6,     1,     1,     4,     1,     1,     3,
       2,     2,     0,     2,     0,     1,     3,     1,     1,     1,
       1,     3,     4,     4,     4,     1,     1,     2,     2,     2,
       3,     3,     1,     1,     1,     1,     3,     1,     3,     1,
       1,     1,     0,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     1,     1,     3,     5,     1,
       3,     5,     4,     3,     3,     3,     4,     3,     3,     3,
       2,     2,     1,     1,     3,     3,     1,     1,     0,     1,
       2,     4,     3,     3,     6,     2,     3,     2,     3,     6,
       1,     1,     1,     1,     1,     6,     3,     4,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    12,     0,    13,     0,     4,     0,     7,     0,
       5,     0,     3,     0,     6,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     2,     0,     5,     3,     6,     4,     2,     0,     5,
       3,     2,     0,     5,     3,     4,     4,     4,     4,     4,
       4,     2,     0,     2,     0,     2,     0,     4,     4,     4,
       4,     1,     1,     1,     1,     1,     1,     3,     3,     4,
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
       1,     1,     1,     4,     4,     4,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     3,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     1,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     2,     2,     4,     3,     4,     1,     1,     3,
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
       1,     1,     1,     1,     1,     1,     3,     2,     2,     2,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   429,     0,     0,   839,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   931,
       0,   919,   713,     0,   719,   720,   721,    25,   782,   906,
     907,   153,   154,   722,     0,   134,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   188,     0,     0,     0,     0,
       0,     0,   395,   396,   397,   400,   399,   398,     0,     0,
       0,     0,   217,     0,     0,     0,     0,     0,   726,   728,
     729,   723,   724,     0,     0,     0,   730,   725,     0,   697,
      27,    28,    29,    31,    30,     0,   727,     0,     0,     0,
       0,   731,   401,   533,     0,   152,   124,   911,   714,     0,
       0,     4,   114,   116,   781,     0,   696,     0,     6,   187,
       7,     9,     8,    10,     0,     0,   393,   442,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   440,   894,   895,
     515,   512,   513,   514,   423,   518,     0,   422,   866,   698,
     705,     0,   784,   511,   392,   869,   870,   881,   441,     0,
       0,   444,   443,   867,   868,   865,   901,   905,     0,   501,
     783,    11,   400,   399,   398,     0,     0,    31,     0,   114,
     187,     0,   975,   441,   974,     0,   972,   971,   517,     0,
     430,   437,   435,     0,     0,   483,   484,   485,   486,   510,
     508,   507,   506,   505,   504,   503,   502,   906,   722,   700,
       0,     0,     0,     0,   995,   887,   698,     0,   699,   464,
       0,   462,     0,   935,     0,   791,   421,   709,   207,     0,
     995,   420,   708,   703,     0,   718,   699,   914,   915,   921,
     913,   710,     0,     0,   712,   509,     0,     0,     0,     0,
     426,     0,   132,   428,     0,     0,   138,   140,     0,     0,
     142,     0,    72,    71,    66,    65,    57,    58,    49,    69,
      80,    81,     0,    52,     0,    64,    56,    62,    83,    75,
      74,    47,    70,    90,    91,    48,    86,    45,    87,    46,
      88,    44,    92,    79,    84,    89,    76,    77,    51,    78,
      82,    43,    73,    59,    93,    67,    60,    50,    42,    41,
      40,    39,    38,    37,    61,    94,    98,    54,    35,    36,
      63,  1031,  1032,  1033,  1034,    55,  1040,    34,    53,    85,
       0,     0,   114,    97,   986,  1030,     0,  1035,     0,     0,
     144,     0,     0,     0,   178,     0,     0,     0,     0,     0,
       0,   793,     0,   102,   104,   306,     0,     0,   305,     0,
     221,     0,   218,   311,     0,     0,     0,     0,     0,   992,
     203,   215,   927,   931,   552,   297,   572,   297,     0,   956,
       0,   733,     0,     0,     0,   954,     0,    16,     0,   118,
     195,   209,   216,   598,   545,     0,   980,   525,   527,   529,
     843,   429,   442,     0,     0,   440,   441,   443,     0,     0,
     715,     0,   716,     0,     0,     0,   177,     0,     0,   120,
     297,     0,    24,   186,     0,   214,   199,   213,   398,   401,
     187,   394,   167,   168,   169,   170,   171,   173,   174,   176,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   919,     0,
     166,   910,   910,   941,     0,     0,     0,     0,     0,     0,
       0,     0,   391,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463,   461,   844,   845,
       0,   910,     0,   857,   297,   297,   910,     0,   912,   902,
     927,     0,   187,     0,     0,   146,     0,   841,   836,   791,
       0,   442,   440,     0,   939,     0,   550,   790,   930,   718,
     442,   440,   441,   120,     0,   297,   419,     0,   859,   711,
       0,   124,   257,     0,   532,     0,   149,     0,     0,   427,
       0,     0,     0,     0,     0,   141,   165,   143,  1031,  1032,
    1033,  1034,  1028,  1029,     0,  1037,  1038,  1039,  1021,     0,
       0,     0,     0,    68,    95,    96,    33,    55,    32,   987,
     172,   175,   145,   124,     0,   162,   164,     0,     0,     0,
       0,   105,     0,   792,   103,    18,     0,    99,     0,   307,
       0,   147,   220,   219,     0,     0,   148,   976,     0,     0,
     442,   440,   441,   444,   443,     0,  1014,   227,     0,   928,
       0,     0,     0,     0,   791,     0,     0,     0,   793,   298,
     791,     0,     0,   150,     0,     0,   732,   955,   782,     0,
       0,   953,   787,   952,   117,     5,    13,    14,     0,   225,
       0,     0,   538,     0,     0,     0,   791,     0,     0,   706,
     701,   539,     0,     0,     0,     0,   843,   124,     0,   793,
     842,  1044,   418,   432,   497,   875,   893,   129,   123,   125,
     126,   127,   128,   392,     0,   516,   785,   786,   115,   791,
       0,   996,     0,     0,   521,   189,   223,     0,   467,   469,
     468,   480,     0,     0,   500,   465,   466,   470,   472,   471,
     488,   487,   490,   489,   491,   493,   495,   494,   492,   482,
     481,   474,   475,   473,   476,   477,   479,   496,   478,   909,
       0,     0,   945,     0,   791,   979,     0,   978,   995,   872,
     901,   205,   197,   211,     0,   980,   201,   187,     0,   433,
     436,   438,   446,   460,   459,   458,   457,   456,   455,   454,
     453,   452,   451,   450,   449,   847,     0,   846,   849,   871,
     853,   995,   850,     0,     0,     0,     0,     0,     0,     0,
       0,   973,   431,   834,   838,   790,   840,     0,   702,     0,
     934,     0,   933,   223,     0,   702,   918,   917,     0,     0,
     846,   849,   916,   850,   424,   259,   261,   124,   536,   535,
     425,     0,   124,   241,   133,   428,     0,     0,     0,     0,
       0,   253,   253,   139,   791,     0,     0,     0,  1019,   791,
       0,  1002,     0,     0,     0,     0,     0,   789,     0,     0,
       0,   697,     0,     0,   735,   696,   739,   740,   742,     0,
     734,   122,   741,   995,  1036,     0,     0,     0,     0,    19,
       0,    20,     0,   100,     0,     0,     0,   111,   793,     0,
     109,   104,   101,   106,     0,   304,   312,   309,     0,     0,
     965,   970,   967,   966,   969,   968,    12,  1012,  1013,     0,
     791,     0,     0,     0,   927,   924,     0,   549,     0,   565,
     790,   551,   299,   300,   694,   792,   296,   790,   571,   568,
     695,   964,   963,   962,     0,   958,     0,   959,   961,     0,
       5,     0,     0,     0,   592,   593,   601,   600,     0,   440,
       0,   790,   544,   548,     0,     0,   981,     0,   526,     0,
       0,  1003,   843,   283,  1043,     0,     0,   858,     0,   908,
     790,   998,   994,   693,     0,   843,     0,     0,   225,   523,
     191,   499,     0,   579,   580,     0,   577,   790,   940,     0,
       0,   297,   227,     0,   225,     0,     0,   223,     0,   919,
     447,     0,     0,   855,   856,   873,   874,   903,   904,     0,
       0,     0,   822,   798,   799,   800,   807,     0,     0,     0,
       0,     0,   811,   817,   818,   809,   810,   828,   791,     0,
     836,   938,   937,     0,   225,     0,   860,   717,     0,   263,
       0,     0,   130,     0,     0,     0,     0,     0,     0,     0,
     233,   234,   245,     0,   124,   243,   159,   253,     0,   253,
       0,   790,     0,     0,     0,     0,   790,  1020,  1022,  1001,
     791,  1000,     0,   791,   763,   764,   761,   762,   797,     0,
     791,   789,   558,   574,     0,   547,     0,     0,   947,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,   179,     0,
     182,   163,     0,     0,   107,   112,   113,   105,   792,   110,
       0,   308,     0,   977,   151,   993,  1014,  1007,  1009,   226,
     228,   318,     0,     0,   925,     0,     0,   554,     0,     0,
       0,   301,   957,     0,    17,     0,   980,   224,   318,     0,
       0,   702,   541,     0,   707,   982,     0,  1003,   530,     0,
       0,  1044,     0,   288,   286,   849,   861,   995,   849,   862,
     997,   121,     0,   843,   222,     0,   843,     0,   498,   944,
     943,     0,   297,     0,     0,     0,     0,     0,     0,   225,
     193,   718,   848,   297,     0,   803,   804,   805,   806,   812,
     813,   826,     0,   791,     0,   822,   562,   576,     0,   802,
     830,   790,   833,   835,   837,     0,   932,     0,   848,     0,
       0,     0,     0,   260,   537,   135,     0,   428,   233,   235,
     927,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     247,     0,  1026,     0,  1015,     0,  1018,   790,     0,     0,
       0,   737,   790,   788,     0,     0,   791,     0,     0,   777,
     791,   779,     0,   791,     0,   743,   780,   778,   951,     0,
     791,   746,   748,   747,     0,     0,   744,   745,   749,   751,
     750,   766,   765,   768,   767,   769,   771,   773,   772,   770,
     759,   758,   753,   754,   752,   755,   756,   757,   760,  1024,
       0,   124,     0,     0,   108,    21,   310,     0,     0,     0,
    1011,     0,   392,   929,   927,   434,   439,   445,   556,     0,
     303,   302,     0,    15,     0,   392,   604,     0,     0,   606,
     599,   602,     0,   597,     0,   984,     0,  1004,   534,     0,
     289,     0,     0,   284,     0,  1003,     0,   318,     0,   843,
       0,   297,     0,   899,   318,   980,   318,   983,     0,     0,
       0,   448,     0,     0,   815,   790,   821,   808,     0,     0,
     791,     0,     0,   820,   791,   801,     0,     0,   791,   827,
     936,   318,     0,   124,     0,   256,   242,     0,     0,     0,
     232,   155,   246,     0,     0,   249,     0,   254,   255,   124,
     248,  1027,  1016,     0,   999,     0,  1042,   796,   795,   736,
     566,   790,   557,     0,   569,   790,   573,     0,   790,   546,
     738,     0,   578,   790,   946,   775,     0,     0,     0,    22,
      23,  1008,  1005,  1006,   229,     0,     0,     0,   399,   390,
       0,     0,     0,   204,   317,   319,     0,   389,     0,     0,
       0,   980,   392,     0,     0,   553,   960,   314,   210,   595,
       0,     0,   540,   528,     0,   292,   282,     0,   285,   291,
     297,   520,  1003,   392,  1003,     0,   942,     0,   898,   392,
       0,   392,   985,   318,   843,   896,   825,   824,   814,   567,
     790,   561,     0,   570,   790,   575,     0,   816,   790,   829,
     392,   124,   262,   131,   136,   157,   236,     0,   244,   250,
     124,   252,  1017,     0,     0,     0,   560,   776,   543,     0,
     950,   949,   774,   124,   183,  1010,     0,     0,     0,   988,
       0,     0,     0,   230,     0,   980,     0,   355,   351,   357,
     697,    31,     0,   345,     0,   350,   354,   367,     0,   365,
     370,     0,   369,     0,   368,     0,   187,   321,     0,   323,
       0,   324,   325,     0,     0,   926,   555,     0,   596,   594,
     605,   603,   293,     0,     0,   280,   290,     0,     0,  1003,
       0,   200,   520,  1003,   900,   206,   314,   212,   392,     0,
       0,     0,   564,   819,   832,     0,   208,   258,     0,     0,
     124,   239,   156,   251,  1041,   794,     0,     0,     0,     0,
       0,     0,   417,     0,   989,     0,   335,   339,   414,   415,
     349,     0,     0,     0,   330,   657,   656,   653,   655,   654,
     674,   676,   675,   645,   615,   617,   616,   635,   651,   650,
     611,   622,   623,   625,   624,   644,   628,   626,   627,   629,
     630,   631,   632,   633,   634,   636,   637,   638,   639,   640,
     641,   643,   642,   612,   613,   614,   618,   619,   621,   659,
     660,   669,   668,   667,   666,   665,   664,   652,   671,   661,
     662,   663,   646,   647,   648,   649,   672,   673,   691,   692,
     677,   679,   678,   680,   681,   658,   683,   682,   685,   687,
     686,   620,   690,   688,   689,   684,   670,   610,   362,   607,
       0,   331,   383,   384,   382,   375,     0,   376,   332,   409,
       0,     0,     0,     0,   413,     0,   187,   196,   313,     0,
       0,     0,   281,   295,   897,     0,     0,   385,   124,   190,
    1003,     0,     0,   202,  1003,   823,     0,     0,   124,   237,
     137,   158,     0,   559,   542,   948,   181,   333,   334,   412,
     231,     0,   791,   791,     0,   358,   346,     0,     0,     0,
     364,   366,     0,     0,   371,   378,   379,   377,     0,     0,
     320,   990,     0,     0,     0,   416,     0,   315,     0,   294,
       0,   590,   793,   124,     0,     0,   192,   198,     0,   563,
     831,     0,     0,   160,   336,   114,     0,   337,   338,     0,
     790,     0,   790,   360,   356,   361,   608,   609,     0,   347,
     380,   381,   373,   374,   372,   410,   407,  1014,   326,   322,
     411,     0,   316,   591,   792,     0,     0,   386,   124,   194,
       0,   240,     0,   185,     0,   392,     0,   352,   359,   363,
       0,     0,   843,   328,     0,   588,   519,   522,     0,   238,
       0,     0,   161,   343,     0,   391,   353,   408,   991,     0,
     793,   403,   843,   589,   524,     0,   184,     0,     0,   342,
    1003,   843,   267,   404,   405,   406,  1044,   402,     0,     0,
       0,   341,     0,   403,     0,  1003,     0,   340,   387,   124,
     327,  1044,     0,   272,   270,     0,   124,     0,     0,   273,
       0,     0,   268,   329,     0,   388,     0,   276,   266,     0,
     269,   275,   180,   277,     0,     0,   264,   274,     0,   265,
     279,   278
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   111,   920,   645,   179,  1519,   738,
     350,   351,   352,   353,   868,   869,   870,   113,   114,   115,
     116,   117,   408,   678,   679,   548,   251,  1588,   554,  1497,
    1589,  1833,   857,   345,   583,  1793,  1101,  1291,  1852,   424,
     180,   680,   956,  1167,  1350,   121,   648,   973,   681,   696,
     977,   617,   972,   231,   529,   682,   649,   974,   426,   370,
     391,   124,   958,   923,   893,  1119,  1522,  1222,  1030,  1740,
    1592,   814,  1036,   553,   823,  1038,  1389,   806,  1019,  1022,
    1211,  1859,  1860,   668,   669,   627,   628,   357,   358,   364,
    1557,  1718,  1719,  1302,  1434,  1545,  1712,  1842,  1862,  1751,
    1797,  1798,  1799,  1532,  1533,  1534,  1535,  1753,  1754,  1760,
    1809,  1538,  1539,  1543,  1705,  1706,  1707,  1729,  1890,  1435,
    1436,   181,   126,  1876,  1877,  1710,  1438,  1439,  1440,  1441,
     127,   244,   549,   550,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,  1569,   138,   955,  1166,   139,   665,
     666,   667,   248,   400,   544,   655,   656,  1253,   657,  1254,
     140,   141,   623,   624,  1245,  1246,  1359,  1360,   142,   846,
    1003,   143,   847,  1004,   631,  1248,  1362,   144,   848,   145,
     146,  1782,   147,   650,  1559,   651,  1139,   928,  1323,  1320,
    1698,  1699,   148,   149,   150,   234,   151,   235,   245,   411,
     536,   152,  1058,  1250,   852,   153,  1059,   951,   594,  1060,
    1005,  1189,  1006,  1191,  1364,  1192,  1193,  1008,  1367,  1368,
    1009,   784,   519,   193,   194,   683,   671,   500,  1155,  1156,
     770,   771,   947,   155,   237,   156,   157,   183,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   730,   168,   241,
     242,   620,   224,   225,   733,   734,  1259,  1260,   384,   385,
     914,   169,   608,   170,   664,   171,   336,  1720,  1772,   371,
     419,   689,   690,  1052,  1150,  1300,   889,   890,   891,   828,
     829,   830,   337,   338,   854,   565,  1521,   945
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1193
static const yytype_int16 yypact[] =
{
   -1193,   175, -1193, -1193,  5602, 13761, 13761,     6, 13761, 13761,
   13761, 11174, 13761, 13761, -1193, 13761, 13761, 13761, 13761, 13761,
   13761, 13761, 13761, 13761, 13761, 13761, 13761, 16810, 16810, 11373,
   13761, 15580,    22,    45, -1193, -1193, -1193, -1193, -1193,   179,
   -1193, -1193, -1193,   204, 13761, -1193,    45,   186,   189,   209,
   -1193,    45, 11572,   511, 11771, -1193, 14481, 10179,   236, 13761,
    1596,    14, -1193, -1193, -1193,   686,    44,    77,   301,   312,
     317,   356, -1193,   511,   364,   377,   283,   329, -1193, -1193,
   -1193, -1193, -1193, 13761,   591,   632, -1193, -1193,   511, -1193,
   -1193, -1193, -1193,   511, -1193,   511, -1193,   435,   407,   511,
     511, -1193,   394, -1193, 11970, -1193, -1193,   454,   535,   626,
     626, -1193,   619,   509,   761,   508, -1193,    85, -1193,   656,
   -1193, -1193, -1193, -1193,   935,  1104, -1193, -1193,   512,   514,
     516,   548,   550,   554,   556,   564, 15215, -1193, -1193, -1193,
   -1193,   333,   658,   693, -1193,   701,   703, -1193,    47,   580,
   -1193,   620,     8, -1193,  1622,    74, -1193, -1193,  3069,   112,
     594,   353, -1193,   180,   163,   600,   227, -1193,   330, -1193,
     745, -1193, -1193, -1193,   660,   628,   682, -1193, 13761, -1193,
     656,  1104, 17854,  3791, 17854, 13761, 17854, 17854, 14160,   652,
   16158, 14160, 17854,   800,   511,   788,   788,   627,   788,   788,
     788,   788,   788,   788,   788,   788,   788, -1193, -1193, -1193,
     667,   676,    63, 13761,   692, -1193, -1193,   708,   680,    68,
     683,    68, 16810, 16388,   685,   889, -1193,   660, -1193, 13761,
     692, -1193,   747, -1193,   773,   740, -1193,   187, -1193, -1193,
   -1193,    68,   112, 12169, -1193, -1193, 13761,  8985,   926,    89,
   17854,  9980, -1193, 13761, 13761,   511, -1193, -1193, 15263,   744,
   -1193, 15329, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193,  2735, -1193,  2735, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193,   109,   118,   137,   107,   682, -1193, -1193, -1193, -1193,
     748,  2494,   124, -1193, -1193,   793,   941, -1193,   806, 15689,
   -1193,   778,   787, 15377, -1193,    30, 15425,  1424,  1424,   511,
     796,   979,   803, -1193,    91, -1193, 16410,    92, -1193,   865,
   -1193,   872, -1193,   990,    96, 16810, 13761, 13761,   814,   833,
   -1193, -1193, 16510, 11373, 13761, 10378, 13761, 10378,    98,   477,
     446, -1193, 13960, 16810,   651, -1193,   511, -1193,   458,   509,
   -1193, -1193, -1193, -1193, 17471,  1000,   917, -1193, -1193, -1193,
      75, 13761,   828,   829, 17854,   830,  1174,   831,  5801, 13761,
      65,   832,   629,    65,   489,   484, -1193,   511,  2735,   835,
   10378, 14481, -1193, -1193,  1226, -1193, -1193, -1193, -1193, -1193,
     656, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   13761, 13761, 13761, 13761, 12368, 13761, 13761, 13761, 13761, 13761,
   13761, 13761, 13761, 13761, 13761, 13761, 13761, 13761, 13761, 13761,
   13761, 13761, 13761, 13761, 13761, 13761, 13761, 13761, 17568, 13761,
   -1193, 13761, 13761, 13761,  5004,   511,   511,   511,   511,   511,
     935,   920,   603,  4780, 13761, 13761, 13761, 13761, 13761, 13761,
   13761, 13761, 13761, 13761, 13761, 13761, -1193, -1193, -1193, -1193,
    1126, 13761, 13761, -1193, 10378, 10378, 13761, 13761,   454,   190,
   16510,   843,   656, 12567, 15491, -1193, 13761, -1193,   847,  1031,
     887,   850,   851, 14159,    68, 12766, -1193, 12965, -1193,   740,
     852,   853,  1705, -1193,   344, 10378, -1193,  1181, -1193, -1193,
   15539, -1193, -1193, 10577, -1193, 13761, -1193,   958,  9184,  1048,
     867, 17733,  1046,   102,    61, -1193, -1193, -1193,   884,   884,
     884, -1193, -1193, -1193,  2735, -1193, -1193, -1193,   504,   875,
    1061, 16310,   511, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193,   879, -1193, -1193,   877,   880,   878,
     885,   396,  1794,  1940, -1193, -1193,   511,   511, 13761,    68,
      14, -1193, -1193, -1193, 16310,   997, -1193,    68,   128,   134,
     890,   892,  1869,   323,   894,   888,   801,   963,   904,    68,
     143,   905, 16975,   906,  1097, 16810, 13761,   915,  1101, 17854,
    1103,   914,   921, -1193,  1332,   511, -1193, -1193,  1045,  3288,
      49, -1193, -1193, -1193,   509, -1193, -1193, -1193,  1085,   986,
     945,   351,   966, 13761,   454,   988,  1117,   936,   973, -1193,
     190, -1193,  2735,  2735,  1123,   926,    75, -1193,   949,  1133,
   -1193,  2735,   221, -1193,   273,   165, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193,   766,  3485, -1193, -1193, -1193, -1193,  1134,
     962, -1193,   952,  1135, -1193, -1193,  1014,  1261, 11756, 17992,
   14160, 14480, 13761, 17806, 14653,  3983, 12545,  4114,  3358,  4387,
   12344, 12344, 12344, 12344,  3579,  3579,  3579,  3579,  3579,  1303,
    1303,   932,   932,   932,   627,   627,   627, -1193,   788, 17854,
     948,   955, 17023,   959,  1139,    12, 13761,    21,   692,    15,
     190, -1193, -1193, -1193,  1142,   917, -1193,   656, 16610, -1193,
   -1193, -1193, 14160, 14160, 14160, 14160, 14160, 14160, 14160, 14160,
   14160, 14160, 14160, 14160, 14160, -1193, 13761,    23,   191, -1193,
   -1193,   692,   188,   965,  3792,   968,   976,   971,  3928,   147,
     970, -1193, 17854,  2979, -1193,   511, -1193,   221,   495, 16810,
   17854, 16810, 17079,  1014,   221,    68,   192,  1001,   978, 13761,
   -1193,   193, -1193, -1193, -1193,  8786,   502, -1193, -1193, 17854,
   17854,    45, -1193, -1193, -1193, 13761,  1070, 16190, 16310,   511,
    9383,   980,   981, -1193,  1167,  1089,  1047,  1027, -1193,  1177,
     994,  2200,  2735, 16310, 16310, 16310, 16310, 16310,  1009,  1141,
    1144,  1059,  1010, 16310,   348,  1062, -1193, -1193, -1193,  1008,
   -1193, 17948, -1193,   216, -1193,  6000,  1321,  1023,  1940, -1193,
    1940, -1193,   511,   511,  1940,  1940,   511, -1193,  1195,  1024,
   -1193,   447, -1193, -1193,  4054, -1193, 17948,  1191, 16810,  1036,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,  1052,
    1223,   511,  1321,  1039, 16510, 16710,  1224, -1193, 13164, -1193,
   13761, -1193,    68, 17854, -1193, 10776, -1193, 13761, -1193, -1193,
   -1193, -1193, -1193, -1193,  1038, -1193, 13761, -1193, -1193,  5204,
   -1193,  2735,  1321,  1049, -1193, -1193, -1193, -1193,  1229,  1054,
   13761, 17471, -1193, -1193,  5004,  1051, -1193,  2735, -1193,  1050,
    6199,  1218,    53, -1193, -1193,    84,  1126, -1193,  1181, -1193,
    2735, -1193, -1193, -1193, 16310,    51,  1060,  1321,   986, -1193,
   -1193, 14653, 13761, -1193, -1193, 13761, -1193, 13761, -1193, 11158,
    1063, 10378,   963,  1219,   986,  2735,  1239,  1014,   511, 17568,
      68, 11556,  1064, -1193, -1193,   184,  1065, -1193, -1193,  1244,
     709,   709,  2979, -1193, -1193, -1193,  1209,  1071,  1196,  1197,
      78,  1076, -1193, -1193, -1193, -1193, -1193, -1193,  1260,  1079,
     847,    68,    68, 13363,   986,  1181, -1193, -1193, 12153,   779,
      45,  9980, -1193,  6398,  1080,  6597,  1082, 16190, 16810,  1081,
    1143,    68, 17948,  1263, -1193, -1193, -1193, -1193,   815, -1193,
     210,  2735,  1099,  1149,  2735,   511,   504, -1193, -1193, -1193,
    1277, -1193,  1096,  1134,   749,   749,  1233,  1233, 17231,  1106,
    1294, 16310, 16310, 16310, 15977, 17471, 15587, 15833, 16310, 16310,
   16310, 16310, 16080, 16310, 16310, 16310, 16310, 16310, 16310, 16310,
   16310, 16310, 16310, 16310, 16310, 16310, 16310, 16310, 16310, 16310,
   16310, 16310, 16310, 16310, 16310, 16310,   511, -1193, -1193,  1231,
   -1193, -1193,  1116,  1119, -1193, -1193, -1193,   449,  1794, -1193,
    1128, -1193, 16310,    68, -1193, -1193,   119, -1193,   810,  1319,
   -1193, -1193,   149,  1138,    68, 10975, 16810, 17854, 17127, 16810,
   13761, 17854, -1193,  2368, -1193,  5403,   917,  1319, -1193,   669,
     461, -1193, 17854,  1198,  1159, -1193,  1137,  1218, -1193,  2735,
     926,  2735,   164,  1327,  1250,   194, -1193,   692,   202, -1193,
   -1193, 17948,  1168,    51, -1193,  1157,    51,  1170, 14653, 17854,
   17183,  1173, 10378,  1175,  1176,  2735,  1179,  1183,  2735,   986,
   -1193,   740,   472, 10378, 13761, -1193, -1193, -1193, -1193, -1193,
   -1193,  1243,  1182,  1360,  1289,  2979,  2979,  2979,  1230, -1193,
   17471,  2979, -1193, -1193, -1193, 16810, 17854,  1190, -1193,    45,
    1354,  1310,  9980, -1193, -1193, -1193,  1202, 13761,  1143,    68,
   16510, 16190,  1199, 16310,  6796,   821,  1204, 13761,    64,   426,
   -1193,  1210, -1193,  2735, -1193,  1264, -1193,  2588,  1367,  1208,
   16310, -1193, 16310, -1193,  1211,  1207,  1394, 17286,  1212, 17948,
    1398, -1193,  1272,  1400,  1217, -1193, -1193, -1193, 17334,  1220,
    1405,  5001, 10358, 10756, 16310, 17902,  4726, 12744, 12942,  4341,
   13139, 13338, 13338, 13338, 13338,  4444,  4444,  4444,  4444,  4444,
    1642,  1642,   749,   749,   749,  1233,  1233,  1233,  1233, -1193,
    1227, -1193,  1225,  1238, -1193, -1193, 17948,   511,  2735,  2735,
   -1193,  1321,   106, -1193, 16510, -1193, -1193, 14160,    68, 13562,
      68, 17854,  1240, -1193,  1228,   969, -1193,   272, 13761, -1193,
   -1193, -1193, 13761, -1193, 13761, -1193,   926, -1193, -1193,   133,
    1413,  1357, 13761, -1193,  1252,  1218,  1255, -1193,  1256,    51,
   13761, 10378,  1257, -1193, -1193,   917, -1193, -1193,  1262,  1259,
    1265, -1193,  1258,  2979, -1193,  2979, -1193, -1193,  1267,  1266,
    1449,  1328,  1269, -1193,  1454, -1193,  1333,  1279,  1461, -1193,
      68, -1193,  1443, -1193,  1282, -1193, -1193,  1285,  1287,   154,
   -1193, -1193, 17948,  1290,  1297, -1193, 15167, -1193, -1193, -1193,
   -1193, -1193, -1193,  2735, -1193,  2735, -1193, 17948, 17389, -1193,
   -1193, 16310, -1193, 16310, -1193, 16310, -1193, 16310, 17471, -1193,
   -1193, 16310, -1193, 16310, -1193, 11353, 16310,  1299,  6995, -1193,
   -1193,   810, -1193, -1193, -1193,   784, 14654,  1321,  1380, -1193,
     673,  1336,  1132, -1193, -1193, -1193,   920,  4157,   105,   111,
    1304,   917,   603,   155, 16810, 17854, -1193, -1193, -1193,  1341,
   13745, 13944, 17854, -1193,   434,  1492,  1425, 13761, -1193, 17854,
   10378,  1390,  1218,  1829,  1218,  1316, 17854,  1318, -1193,  1915,
    1320,  1935, -1193, -1193,    51, -1193, -1193,  1377, -1193, -1193,
    2979, -1193,  2979, -1193,  2979, -1193,  2979, -1193, 17471, -1193,
    1969, -1193,  8786, -1193, -1193, -1193, -1193,  9582, -1193, -1193,
   -1193,  8786, -1193,  1326, 16310, 17437, 17948, 17948, 17948,  1384,
   17948, 17492, 11353, -1193, -1193, -1193,  1321,  1321,   511, -1193,
    1507, 15977,    83, -1193, 14654,   917,  2109, -1193,  1344, -1193,
     117,  1335,   125, -1193, 15001, -1193, -1193, -1193,   131, -1193,
   -1193,   657, -1193,  1337, -1193,  1451,   656, -1193, 14828, -1193,
   14828, -1193, -1193,  1518,   920, -1193,    68,  4564, -1193, -1193,
   -1193, -1193,  1521,  1455, 13761, -1193, 17854,  1347,  1345,  1218,
     687, -1193,  1390,  1218, -1193, -1193, -1193, -1193,  2018,  1348,
    2979,  1407, -1193, -1193, -1193,  1409, -1193,  8786,  9781,  9582,
   -1193, -1193, -1193,  8786, -1193, 17948, 16310, 16310, 16310,  7194,
    1352,  1355, -1193, 16310, -1193,  1321, -1193, -1193, -1193, -1193,
   -1193,  2735,  1495,   673, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,   719, -1193,
    1336, -1193, -1193, -1193, -1193, -1193,   121,   808, -1193,  1537,
     138, 15689,  1451,  1539, -1193,  2735,   656, -1193, -1193,  1362,
    1543, 13761, -1193, 17854, -1193,   157,  1363, -1193, -1193, -1193,
    1218,   687, 14308, -1193,  1218, -1193,  2979,  2979, -1193, -1193,
   -1193, -1193,  7393, 17948, 17948, 17948, -1193, -1193, -1193, 17948,
   -1193,   534,  1551,  1552,  1366, -1193, -1193, 16310, 15001, 15001,
    1505, -1193,   657,   657,   813, -1193, -1193, -1193, 16310,  1482,
   -1193,  1391,  1379,   140, 16310, -1193,   511, -1193, 16310, 17854,
    1493, -1193,  1568, -1193,  7592,  1385, -1193, -1193,   687, -1193,
   -1193,  7791,  1387,  1467, -1193,  1488,  1427, -1193, -1193,  1501,
    2735,  1426,  1495, -1193, -1193, 17948, -1193, -1193,  1437, -1193,
    1573, -1193, -1193, -1193, -1193, 17948,  1599,   801, -1193, -1193,
   17948,  1422, 17948, -1193,   326,  1423,  7990, -1193, -1193, -1193,
    1421, -1193,  1429,  1444,   511,   603,  1446, -1193, -1193, -1193,
   16310,  1447,    60, -1193,  1546, -1193, -1193, -1193,  8189, -1193,
    1321,  1023, -1193,  1450,   511,  1477, -1193, 17948, -1193,  1439,
    1624,   849,    60, -1193, -1193,  1559, -1193,  1321,  1441, -1193,
    1218,    66, -1193, -1193, -1193, -1193,  2735, -1193,  1452,  1453,
     145, -1193,   751,   849,   139,  1218,  1457, -1193, -1193, -1193,
   -1193,  2735,   470,  1630,  1564,   751, -1193,  8388,   153,  1635,
    1567, 13761, -1193, -1193,  8587, -1193,   475,  1644,  1576, 13761,
   -1193, 17854, -1193,  1646,  1578, 13761, -1193, 17854, 13761, -1193,
   17854, 17854
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1193, -1193, -1193,  -561, -1193, -1193, -1193,    86,   -11,   -54,
     631, -1193,  -278,  -520, -1193, -1193,   555,    43,  1406, -1193,
    2646, -1193,  -458, -1193,    42, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193,  -187, -1193, -1193,  -149,
      25,     5, -1193, -1193, -1193, -1193, -1193, -1193,    28, -1193,
   -1193, -1193, -1193, -1193, -1193,    33, -1193, -1193,  1187,  1194,
    1193,   -92,  -664,  -879,   700,   754,  -186,   465,  -953, -1193,
      95, -1193, -1193, -1193, -1193,  -731,   300, -1193, -1193, -1193,
   -1193,  -175, -1193,  -560, -1193,  -316, -1193, -1193,  1088, -1193,
     114, -1193, -1193, -1062, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193,    88, -1193,   172, -1193, -1193, -1193,
   -1193, -1193,     2, -1193,   271,  -769, -1193, -1192,  -190, -1193,
    -131,    24,   -99,  -177, -1193,    -2, -1193, -1193, -1193,   281,
     -17,    -3,    36,  -726,   -66, -1193, -1193,     9, -1193,    -7,
   -1193, -1193,    -5,   -37,   142, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193,  -608,  -871, -1193, -1193, -1193, -1193,
   -1193,  2070, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193, -1193,   573, -1193, -1193, -1193, -1193, -1193, -1193, -1193,
   -1193,  -759, -1193,  2150,     3, -1193,  1951,  -404, -1193, -1193,
    -470,  3389,  3471, -1193, -1193, -1193,   654,   113,  -601, -1193,
   -1193,   722,   540,  -727, -1193,   541, -1193, -1193, -1193, -1193,
   -1193,   725, -1193, -1193, -1193,    73,  -891,  -128,  -429,  -425,
   -1193,   792,  -106, -1193, -1193,     4,    27,   527, -1193, -1193,
    1496,   -16, -1193,  -372,    50,   120, -1193,   141, -1193, -1193,
   -1193,  -468,  1376, -1193, -1193, -1193, -1193, -1193,   866,   352,
   -1193, -1193, -1193,  -350,  -712, -1193,  1329, -1150, -1193,   -59,
    -167,   181,   909, -1193,  -801,   334,   -63, -1193,   642,   713,
   -1193, -1193, -1193, -1193,   672,  -237,   296, -1096
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1024
static const yytype_int16 yytable[] =
{
     182,   184,   334,   186,   187,   188,   190,   191,   192,   120,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   659,   481,   223,   226,   431,   906,   125,   119,
     247,   511,   122,   976,   233,   238,   392,   123,   403,   250,
     395,   396,   779,   252,   661,   333,   118,   258,   256,   261,
     342,  1151,   343,   503,   346,  1329,  1007,   938,   239,   793,
    1143,   632,   480,   533,   727,   427,   341,   405,   943,   588,
     590,   768,   867,   872,  1218,   769,  1315,   154,   250,  1165,
     249,   240,   431,   805,   919,   566,   567,   402,   407,  1026,
     112,  1040,  1605,   821,   421,  1176,  1387,   355,   545,   404,
      14,   600,    14,   354,   692,   605,   939,   545,   801,    14,
    1425,   819,   802,   537,  1548,    14,   -33,  -876,   -68,   378,
    1550,   -33,  1152,   -68,    14,   855,  -348,   -95,   388,  1014,
    1762,   389,   -95,   -32,  1613,  1207,   538,   878,   -32,   259,
    1700,   405,   332,   545,  -879,   584,   -96,  1769,   379,  1769,
     430,   -96,   895,   520,  1605,    14,   895,  1763,   895,   369,
     363,   402,   407,   895,   895,  -699,   361,  1153,  1198,   887,
     888,  1454,  -707,   404,   362,     3,   522,  1892,   482,   390,
     514,   369,   501,   498,   499,   369,   369,  -531,   775,   776,
     -98,  1906,   531,   596,  1252,  1780,   521,   407,   185,   -97,
    -888,  -851,   498,   499,   -98,   585,   512,   971,   404,   940,
     369,   356,   530,   -97,   243,  -851,  1455,   381,  1426,   798,
     382,   383,  1893,  1427,   404,    62,    63,    64,   172,  1428,
     428,  1429,  1199,  -878,  -700,  -883,  1907,   246,   540,  -886,
    1781,   540,  -876,  -792,  -287,   918,  -792,  1330,   250,   551,
    -877,   635,   597,  -271,  -882,  1154,   822,  -920,  1388,  -792,
     506,  -880,  -923,  -922,  -863,  -287,  -885,  1109,  1380,  -879,
    1430,  1431,  -864,  1432,  -586,  1463,  1520,  1606,  1607,   422,
     518,   697,  1469,   546,  1471,   579,   601,   564,   509,   542,
     606,   501,   633,   547,   429,   820,   564,   506,  -790,  1549,
    1349,   -33,  1433,   -68,  1456,  1551,  1225,   502,  1229,  1490,
    1894,  -348,   -95,  1179,  1764,   564,  1227,  1228,   -32,  1614,
    -706,   879,   566,   567,  1908,  1701,  1449,   880,   333,  1366,
     611,   -96,  1770,   112,  1819,  1331,   896,   112,   528,  1887,
     989,   552,  1303,  -587,  1105,  1106,  1326,  1496,  1555,  1023,
     610,   614,   339,   374,  1025,   498,   499,  -889,  -878,  1135,
    -883,   250,   404,   780,  1844,   695,  -854,   334,   223,   622,
     629,   250,   629,  1096,  1608,  -877,  -892,   639,   253,  -882,
    -854,   254,  -920,   431,  -701,   507,  -880,  -923,  -922,  -863,
     354,   354,   591,  -585,   418,  1162,   190,  -864,  1713,   376,
    1714,   255,   609,  -581,   684,   925,  1230,   498,   499,  1845,
     333,  1578,   630,   207,    40,   629,   502,   749,   392,   744,
     745,   427,   507,  -585,  1314,   578,  1122,   207,    40,   644,
    -702,   344,   125,   119,   393,   698,   699,   700,   701,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   737,   728,  -890,   729,   729,   732,  1361,
    1363,   233,   238,   670,  1369,   375,   751,   397,   752,   753,
     754,   755,   756,   757,   758,   759,   760,   761,   762,   763,
     764,  1377,   750,   365,   112,   239,   729,   774,   862,   629,
     629,   729,   778,   688,   366,  -995,   747,   332,   752,   367,
     369,   782,   333,   926,   660,   505,   884,  1562,   240,  1158,
     790,   377,   792,  1159,   739,  -887,   418,   508,   927,   379,
     629,   158,  1227,  1228,  1461,   825,   808,  1509,   809,  1786,
     810,   107,  1328,  -995,   946,   505,   948,   634,   368,   596,
     772,   862,   481,  1899,   219,   221,   372,   863,  1913,   659,
     578,   369,   742,   369,   369,   369,   369,   379,   562,   373,
     563,   970,   379,   739,   641,  1181,  1224,  1020,  1021,   641,
    1102,   661,  1103,   398,   797,    37,   767,   803,   867,   399,
     813,   480,    37,   874,   740,   826,  1829,   393,   978,   394,
     636,   382,   383,  1336,   982,  1563,  1338,    50,   417,   578,
     417,   498,   499,   731,    50,    37,   929,  1585,   379,   417,
     740,   903,  1390,   800,  1321,   410,  1476,   570,  1477,   498,
     499,   406,   786,  1470,   112,   871,   871,    50,   960,   382,
     383,  1900,   773,   740,   382,   383,  1914,   777,   404,   409,
    -852,   416,   646,   647,   740,  1173,  1322,   740,   853,   946,
     948,  1570,   176,  1572,  -852,    88,  1015,   948,    90,    91,
     417,    92,   177,    94,   379,    90,    91,   533,    92,   177,
      94,   380,   873,   688,   468,   687,  1097,  -890,  1016,   686,
     382,   383,   176,   659,   423,    88,   469,   961,    90,    91,
     420,    92,   177,    94,  1526,   406,   432,   482,   433,   379,
     434,  1351,   379,    37,   691,   661,   413,   339,  1453,   641,
     913,   915,    62,    63,    64,   172,   173,   428,  -582,  1553,
    1794,   969,   637,  1757,   379,    50,   643,   901,    37,   670,
     406,   641,   435,   908,   436,   381,   382,   383,   437,   524,
     438,  1758,  1379,  1581,    37,  1582,   532,  1583,   439,  1584,
      50,   981,   637,  -583,   643,   637,   643,   643,  1726,   932,
    1759,   471,  1731,   472,   158,   473,    50,   474,   158,  1465,
    1884,   382,   383,   369,   382,   383,   504,  1185,  1186,  1187,
      37,   429,  -884,   386,  1018,  1898,    90,    91,  1527,    92,
     177,    94,  1092,  1093,  1094,   642,   382,   383,  1024,   359,
     250,  1528,    50,  1609,  1529,  -584,   360,  -700,  1095,  1702,
     510,    90,    91,  1703,    92,   177,    94,   387,   659,   125,
     119,   176,  1316,  1418,    88,  1530,  1443,    90,    91,  1765,
      92,  1531,    94,   386,  1812,  1317,   515,   968,   517,  1541,
     661,   887,   888,  1735,  1209,  1210,  1342,   469,  1766,   375,
     824,  1767,  1035,  1813,  1318,   523,  1814,  1352,   377,  1002,
     418,  1010,  -888,    90,    91,   505,    92,   177,    94,   125,
     119,  1727,  1728,   599,   526,    62,    63,    64,    65,    66,
     428,   112,   607,  1127,   612,  1128,    72,   475,   527,   619,
    1131,   871,   810,   871,  -698,  1033,   112,   871,   871,  1107,
     640,  1133,  1298,  1299,  1579,  1492,  1516,  1517,  -995,  1180,
    1226,  1227,  1228,   737,   120,  1142,  1384,  1227,  1228,  1785,
     534,  1501,   535,  1788,   543,   158,   477,  1042,   556,   418,
     568,   112,  1047,   125,   119,  1888,  1889,   122,  1104,   688,
   -1023,  1861,   123,  -995,   429,   571,  -995,  1168,   935,   936,
    1169,   118,  1170,   572,   125,   119,   629,   944,  1873,  1874,
    1875,  1861,   580,  1425,   412,   414,   415,  1118,   587,   589,
    1883,   581,   233,   238,  1144,   465,   466,   467,   593,   468,
    1334,   592,   154,  1810,  1811,   602,   772,   595,   803,  1806,
    1807,   469,   603,  1117,   604,   112,   239,   615,  1206,  1789,
    1790,   616,  1050,  1053,   662,   670,    37,  1212,    14,   663,
     578,   672,   673,   674,   676,  1467,   112,  -119,   670,   240,
     685,    55,   767,  1587,   800,   694,   659,   619,    50,   783,
     785,   636,  1593,   787,   788,   794,   795,   125,   119,   125,
     119,   660,   811,  1251,   740,  1599,  1257,   545,   661,  1305,
     818,   815,   564,  1213,   369,   803,   740,   831,   740,  1882,
     832,   856,   858,   860,   859,   158,  1188,  1188,  1002,   861,
     877,  1426,   886,   881,  1895,   882,  1427,   885,    62,    63,
      64,   172,  1428,   428,  1429,   892,   894,   425,   897,    90,
      91,   800,    92,   177,    94,   899,   900,   112,   904,   112,
     905,   112,   907,   909,   910,   916,   659,   921,  1306,   922,
    1307,  1202,   924,  -722,   930,  1311,   931,   691,   691,   933,
     934,  1235,  1742,  1430,  1431,   740,  1432,   937,   661,   941,
     120,   952,   942,   950,  1567,   953,   957,   963,   967,   954,
     578,   871,   902,   578,   964,   966,   975,   429,  -704,   125,
     119,   985,   959,   122,   983,  1448,  1239,   629,   123,   986,
     987,  1017,  1027,  1243,  1037,  1039,  1041,   118,   629,  1307,
    1043,  1825,   853,  1044,  1045,   660,  1046,  1048,   513,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,  1061,  1065,  1068,  1108,  1112,  1372,    37,   154,   207,
      40,  1062,   250,    37,  1063,    55,  1064,  1136,  1100,  1067,
    1110,   112,  1386,    62,    63,    64,   172,   173,   428,    50,
    1114,  1115,  1116,  1146,  1121,    50,   670,  1132,  1125,   670,
    1147,   496,   497,  1140,  1138,  1145,  1160,  1141,   125,   119,
    1149,  1175,  1163,  1178,  1375,  1172,  1183,  -891,  1184,  1872,
    1540,  1194,    37,  1195,   207,    40,  1196,  1197,  1200,  1201,
    1784,  1177,  1203,  1220,  1215,   980,  1217,  1223,  1232,  1221,
    1791,  1002,  1002,  1002,    50,  1233,  1237,  1002,   765,  1238,
      90,    91,   429,    92,   177,    94,    90,    91,   112,    92,
     177,    94,  1095,  1242,  1445,  1241,  1356,    37,   498,   499,
     112,  1554,  1292,  1450,  1290,  1293,  1011,  1451,  1012,  1452,
     660,   766,  1295,   107,  1541,  1826,  1437,  1459,  1301,    50,
    1304,  1325,   158,  1333,  1324,  1466,   629,  1231,   431,  1437,
    1234,  1332,    37,   765,  1031,    90,    91,   158,    92,   177,
      94,   971,  1337,   462,   463,   464,   465,   466,   467,  1402,
     468,  1335,  1339,  1406,    50,  1341,  1409,   675,  1343,  1355,
    1848,  1344,   469,  1414,  1346,  1442,   799,  1347,   107,  1353,
     996,  1354,   158,  1421,  1365,  1371,  1373,  1374,  1442,  1391,
      90,    91,  1381,    92,   177,    94,  1376,  1711,  1385,  1395,
    1393,  1396,    37,  1401,  1399,  1113,  1400,  1405,  1407,  1408,
    1410,  1404,   670,    37,  1413,   911,  1412,   912,   694,  1419,
    1417,   619,  1124,  1447,    50,    90,    91,  1457,    92,   177,
      94,  1897,  1420,   214,   214,    50,  1446,   230,  1904,  1002,
    1458,  1002,   125,   119,  1460,  1327,   158,   944,  1462,  1464,
    1468,  1475,  1566,   959,  1473,   629,  1472,  1474,  1480,  1602,
    1478,  1546,   230,  1484,  1482,  1479,   482,   158,  1483,  1486,
    1488,  1345,  1487,  1481,  1348,  1491,  1493,  1485,  1494,   176,
    1495,  1489,    88,    89,  1498,    90,    91,  1437,    92,   177,
      94,  1499,  1524,  1437,  1513,  1437,    90,    91,  1552,    92,
     177,    94,  1537,  1558,   112,    37,  1564,  1568,  1565,  1573,
    1604,  1574,   332,  1580,  1437,  1576,   125,   119,  1542,  1594,
    1597,  1603,  1611,   220,   220,   125,   119,    50,   660,  1392,
    1612,  1708,  1715,  1160,  1709,  1721,  1442,  1725,  1722,  1591,
    1724,  1734,  1442,  1736,  1442,  1737,  1747,   670,   158,  1748,
     158,  1768,   158,  1774,  1031,  1219,  1777,  1778,  1783,  1723,
    1800,  1802,  1804,  1442,  1808,  1816,  1002,  1776,  1002,  1817,
    1002,  1818,  1002,    34,    35,    36,  1823,  1824,   112,  1716,
    1828,  1831,  1832,   112,  1834,   349,   208,   112,    90,    91,
    -344,    92,   177,    94,  1422,  1423,    62,    63,    64,   172,
     173,   428,  1437,  1835,   369,  1837,  1839,   578,   660,  1763,
     332,   125,   119,  1840,  1843,  1849,  1846,   125,   119,  1851,
    1697,  1850,  1867,   125,   119,  1856,  1858,  1704,   214,  1863,
    1739,  1591,  1870,  1871,   332,  1881,   332,    78,    79,    80,
      81,    82,  1879,   332,  1901,  1885,  1886,  1902,   212,  1909,
    1910,  1442,  1896,  1308,    86,    87,  1310,  1771,  1915,  1916,
    1918,  1919,   158,  1294,  1866,   429,  1002,   746,    96,   741,
     743,  1869,  1174,   112,   112,   112,  1137,    37,   230,   112,
     230,  1880,   101,  1378,  1741,   112,  1500,  1878,   875,  1502,
    1732,  1503,  1089,  1090,  1091,  1092,  1093,  1094,  1610,    50,
     333,  1756,  1761,  1544,  1854,  1903,  1891,   347,   348,  1525,
    1773,  1095,  1319,  1190,  1730,  1244,  1779,  1821,   220,   513,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,  1370,  1547,  1357,  1204,  1358,   230,  1157,   158,
    1051,    62,    63,    64,    65,    66,   428,   619,  1031,   621,
     693,   158,    72,   475,  1841,  1515,   431,   349,  1297,  1236,
      90,    91,   214,    92,   177,    94,   125,   119,  1289,     0,
       0,   214,   496,   497,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     476,     0,   477,     0,     0,     0,     0,   578,     0,     0,
     230,     0,     0,     0,     0,   478,     0,   479,   125,   119,
     429,     0,     0,     0,     0,   125,   119,     0,   332,     0,
       0,     0,  1002,  1002,   230,     0,     0,   230,   112,     0,
       0,   619,     0,  1425,     0,     0,     0,  1795,     0,   498,
     499,     0,     0,     0,  1697,  1697,     0,     0,  1704,  1704,
     125,   119,   220,     0,     0,     0,     0,     0,     0,  1855,
       0,   220,   369,   613,     0,  1801,  1803,     0,   220,     0,
     112,     0,   125,   119,   230,    37,     0,   112,    14,   220,
       0,     0,     0,   513,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,  1911,    50,   796,     0,
       0,     0,     0,     0,  1917,   864,   865,  1752,     0,     0,
    1920,     0,   112,  1921,     0,   670,   214,     0,     0,  1425,
    1853,   125,   119,     0,     0,     0,     0,     0,   125,   119,
       0,     0,     0,     0,   112,   670,   496,   497,     0,  1425,
    1868,  1426,     0,     0,   670,   158,  1427,     0,    62,    63,
      64,   172,  1428,   428,  1429,   866,     0,     0,    90,    91,
       0,    92,   177,    94,    14,     0,     0,     0,     0,     0,
     230,  1556,     0,  1425,   230,     0,     0,   844,   217,   217,
       0,     0,     0,   112,    14,     0,     0,     0,     0,     0,
     112,     0,     0,  1430,  1431,     0,  1432,     0,     0,     0,
       0,     0,     0,   498,   499,     0,   220,     0,     0,     0,
     844,  1775,     0,     0,     0,     0,     0,   429,    14,   158,
       0,    37,  1425,     0,   158,  1571,     0,  1426,   158,     0,
       0,   214,  1427,     0,    62,    63,    64,   172,  1428,   428,
    1429,     0,     0,    50,     0,     0,     0,  1426,     0,     0,
       0,     0,  1427,     0,    62,    63,    64,   172,  1428,   428,
    1429,     0,   883,     0,     0,     0,     0,    14,   230,   230,
       0,     0,     0,     0,     0,     0,     0,   230,     0,  1430,
    1431,  1426,  1432,     0,     0,     0,  1427,     0,    62,    63,
      64,   172,  1428,   428,  1429,     0,  1836,   215,   215,  1430,
    1431,   866,  1432,   429,    90,    91,     0,    92,   177,    94,
       0,  1575,     0,     0,   158,   158,   158,     0,     0,     0,
     158,   220,     0,   429,     0,     0,   158,     0,     0,     0,
    1426,  1577,     0,  1430,  1431,  1427,  1432,    62,    63,    64,
     172,  1428,   428,  1429,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,   429,     0,     0,
       0,     0,     0,     0,     0,  1586,     0,     0,     0,     0,
       0,     0,   944,   217,     0,     0,     0,   216,   216,     0,
       0,   232,  1430,  1431,     0,  1432,     0,   944,     0,     0,
      37,     0,     0,     0,     0,   214,     0,   214,     0,     0,
       0,     0,     0,     0,     0,     0,   429,     0,     0,     0,
       0,     0,    50,     0,  1733,     0,     0,     0,     0,     0,
       0,     0,     0,   214,   844,     0,     0,     0,     0,     0,
       0,   272,     0,     0,  1527,     0,     0,   230,   230,   844,
     844,   844,   844,   844,   220,     0,     0,  1528,     0,   844,
    1529,     0,     0,     0,     0,     0,     0,     0,     0,   274,
       0,     0,   230,     0,     0,     0,     0,   176,     0,   158,
      88,    89,     0,    90,    91,     0,    92,  1531,    94,     0,
       0,    37,     0,     0,   214,   220,     0,   220,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,   230,     0,
     214,   214,     0,    50,     0,     0,     0,   217,     0,     0,
       0,   158,     0,   220,     0,     0,   217,     0,   158,     0,
       0,     0,     0,   217,     0,     0,     0,   230,   230,     0,
       0,     0,     0,     0,   217,     0,     0,   230,   558,   559,
     560,   561,     0,   230,     0,   658,     0,     0,     0,     0,
       0,     0,     0,   158,     0,     0,   230,     0,   176,     0,
     844,    88,   326,   230,    90,    91,     0,    92,   177,    94,
       0,  1049,   216,     0,   220,   158,     0,     0,   440,   441,
     442,   230,   330,     0,     0,   230,     0,     0,     0,     0,
     220,   220,   331,     0,     0,     0,     0,     0,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   158,   468,   215,     0,     0,     0,
       0,   158,     0,   214,   214,   215,     0,   469,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   230,     0,     0,
     230,     0,   230,   215,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,   844,   844,   844,
       0,   230,     0,     0,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,   216,     0,     0,   844,     0,
       0,     0,   216,   220,   220,   272,     0,     0,     0,     0,
       0,     0,   214,   216,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,   274,     0,   230,     0,   230,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1312,     0,     0,
       0,     0,     0,     0,     0,    37,   217,     0,     0,     0,
     215,   230,     0,     0,   230,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,   569,   230,     0,     0,     0,
       0,   214,     0,     0,     0,     0,     0,     0,   232,   272,
       0,     0,   220,     0,     0,   220,   214,   214,     0,   844,
       0,     0,   558,   559,   560,   561,     0,     0,     0,   230,
       0,     0,     0,   230,     0,     0,   844,   274,   844,     0,
       0,     0,   176,     0,     0,    88,   326,     0,    90,    91,
     216,    92,   177,    94,     0,     0,     0,     0,     0,    37,
     844,     0,     0,     0,     0,     0,   330,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   331,     0,     0,     0,
       0,    50,     0,     0,     0,   215,     0,     0,     0,   217,
       0,   220,   335,     0,   230,   230,     0,   230,     0,     0,
     214,     0,     0,     0,     0,     0,   220,   220,     0,     0,
       0,   849,     0,     0,     0,     0,   558,   559,   560,   561,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,   217,     0,     0,     0,   176,     0,     0,    88,
     326,     0,    90,    91,   849,    92,   177,    94,     0,  1394,
       0,     0,     0,     0,     0,     0,   272,     0,   217,     0,
     330,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     331,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,     0,     0,     0,     0,   230,
     220,   230,     0,     0,     0,     0,     0,   844,     0,   844,
       0,   844,     0,   844,   230,     0,    37,   844,   215,   844,
       0,     0,   844,     0,     0,     0,     0,     0,     0,   217,
       0,     0,   230,   230,     0,     0,   230,     0,    50,     0,
       0,     0,     0,   230,     0,   217,   217,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   558,   559,   560,   561,     0,     0,     0,
       0,     0,   658,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,   176,   230,     0,    88,   326,   216,    90,
      91,     0,    92,   177,    94,     0,     0,     0,     0,     0,
     844,     0,     0,     0,     0,     0,     0,   330,   335,     0,
     335,     0,   230,   230,     0,     0,     0,   331,     0,     0,
     230,     0,   230,     0,     0,     0,     0,     0,     0,   216,
     220,   216,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,   230,     0,   230,     0,     0,     0,
       0,     0,     0,   230,   215,   215,     0,   216,   849,     0,
       0,     0,     0,     0,     0,     0,     0,   335,   217,   217,
       0,     0,     0,   849,   849,   849,   849,   849,     0,     0,
       0,     0,     0,   849,     0,     0,     0,     0,     0,     0,
       0,     0,   844,   844,   844,     0,  1099,     0,     0,   844,
       0,   230,     0,     0,     0,     0,   658,   230,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   990,
     991,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1120,     0,   216,   216,     0,     0,     0,   992,
       0,     0,     0,     0,     0,     0,     0,   993,   994,   995,
      37,     0,     0,     0,   335,     0,     0,   335,     0,     0,
     996,     0,  1120,     0,     0,     0,     0,   217,     0,     0,
     217,   216,    50,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,     0,   215,   215,     0,
       0,     0,     0,     0,   849,     0,     0,  1164,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   997,   998,   999,
       0,   230,     0,     0,     0,     0,     0,     0,     0,   232,
       0,     0,  1000,     0,     0,     0,   496,   497,   230,     0,
       0,     0,     0,    90,    91,     0,    92,   177,    94,     0,
       0,   658,     0,     0,     0,     0,   217,   230,     0,     0,
       0,  1001,     0,   844,     0,     0,     0,     0,     0,     0,
       0,   217,   217,     0,   844,     0,     0,   216,   216,     0,
     844,     0,     0,     0,   844,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,   215,
       0,     0,     0,   498,   499,     0,   230,     0,     0,     0,
     335,   849,   849,   849,   827,   216,     0,   845,   849,   849,
     849,   849,   849,   849,   849,   849,   849,   849,   849,   849,
     849,   849,   849,   849,   849,   849,   849,   849,   849,   849,
     849,   849,   849,   849,   849,   849,   844,     0,     0,     0,
     845,     0,     0,     0,     0,   217,   230,     0,     0,     0,
       0,     0,   849,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   230,     0,   215,   216,     0,     0,   216,
       0,     0,   230,     0,     0,     0,     0,     0,     0,     0,
     215,   215,     0,     0,     0,     0,     0,   230,   440,   441,
     442,     0,     0,     0,     0,     0,     0,     0,   335,   335,
       0,     0,     0,     0,     0,     0,     0,   335,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,     0,     0,
     216,     0,     0,     0,     0,   216,     0,   469,     0,   658,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216,   216,     0,   849,   215,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     849,     0,   849,     0,     0,   217,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   849,   468,   218,   218,     0,     0,
     236,     0,     0,     0,     0,     0,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   658,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1424,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   845,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,   335,   845,
     845,   845,   845,   845,   917,     0,     0,     0,     0,   845,
       0,     0,     0,     0,     0,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   215,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,   849,     0,   849,   469,   849,     0,   849,   216,     0,
       0,   849,     0,   849,     0,     0,   849,   335,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1523,     0,     0,
    1536,     0,     0,   335,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,   335,     0,     0,     0,
     845,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335, -1024, -1024, -1024, -1024, -1024,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,   849,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1600,  1601,     0,     0,
       0,     0,     0,     0,     0,     0,  1536,     0,     0,     0,
       0,   949,     0,     0,     0,     0,     0,   335,     0,     0,
     335,     0,   827,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   845,   845,   845,
       0,     0,     0,     0,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,   845,   845,   845,   845,   845,   845,   845,   845,
     845,   845,     0,     0,     0,   218,   849,   849,   849,     0,
       0,     0,     0,   849,   218,  1750,     0,     0,   845,     0,
       0,   218,     0,  1536,     0,     0,     0,     0,     0,     0,
       0,     0,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   236,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   335,     0,   335,     0,     0,
       0,     0,   440,   441,   442,   513,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,     0,     0,
       0,   335,   443,   444,   335,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,     0,   468,
       0,     0,     0,     0,     0,     0,     0,   236,   496,   497,
       0,   469,     0,     0,     0,     0,     0,     0,     0,   845,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
       0,     0,     0,   335,     0,     0,   845,     0,   845,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,     0,     0,     0,     0,     0,     0,   849,     0,     0,
     845,     0,     0,     0,     0,     0,     0,     0,   849,     0,
       0,     0,     0,     0,   849,   498,   499,     0,   849,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   440,   441,
     442,     0,     0,     0,   335,   335,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,   444,
     850,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,   984,     0,
     849,     0,     0,   850,     0,     0,     0,   469,     0,     0,
    1865,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,  1523,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   335,
     468,   335,   851,     0,     0,     0,     0,   845,     0,   845,
       0,   845,   469,   845,     0,     0,     0,   845,     0,   845,
       0,     0,   845,     0,   440,   441,   442,     0,     0,     0,
       0,     0,   335,     0,     0,   876,     0,     0,     0,     0,
       0,     0,     0,   335,   443,   444,     0,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
       0,   468,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   469,   988,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     845,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     335,   468,     0,     0,     0,     0,     0,     0,   218,     0,
     218,     0,     0,   469,     0,     0,     0,     0,   272,     0,
       0,     0,     0,     0,   335,     0,   335,     0,     0,     0,
       0,     0,     0,   335,     0,     0,   218,   850,     0,     0,
       0,     0,     0,     0,     0,     0,   274,     0,     0,     0,
       0,     0,   850,   850,   850,   850,   850,     0,     0,     0,
       0,     0,   850,     0,     0,     0,     0,     0,    37,     0,
       0,     0,   845,   845,   845,     0,     0,     0,     0,   845,
    1111,     0,     0,     0,     0,     0,     0,   335,     0,     0,
      50,     0,     0,     0,     0,     0,     0,   218,  -391,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   172,
     173,   428,     0,   218,   218,     0,     0,     0,     0,  1032,
       0,     0,     0,     0,     0,   558,   559,   560,   561,     0,
       0,     0,     0,     0,  1054,  1055,  1056,  1057,     0,     0,
       0,     0,     0,     0,  1066,   176,     0,     0,    88,   326,
     236,    90,    91,     0,    92,   177,    94,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   330,
       0,     0,     0,   850,     0,   429,     0,     0,     0,   331,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335,     0,     0,     0,     0,     0,     0,   236,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,  1093,  1094,  1796,     0,     0,
       0,     0,     0,   845,     0,     0,     0,     0,     0,     0,
    1095,     0,     0,     0,   845,     0,   218,   218,     0,     0,
     845,     0,     0,     0,   845,  1161,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,     0,   335,     0,     0,     0,
     850,   850,   850,     0,   236,     0,   469,   850,   850,   850,
     850,   850,   850,   850,   850,   850,   850,   850,   850,   850,
     850,   850,   850,   850,   850,   850,   850,   850,   850,   850,
     850,   850,   850,   850,   850,     0,   845, -1024, -1024, -1024,
   -1024, -1024,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
       0,   850,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1095,     0,   218,     0,     0,   218,     0,
       0,     0,   335,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1247,  1249,     0,     0,   335,     0,  1258,
    1261,  1262,  1263,  1265,  1266,  1267,  1268,  1269,  1270,  1271,
    1272,  1273,  1274,  1275,  1276,  1277,  1278,  1279,  1280,  1281,
    1282,  1283,  1284,  1285,  1286,  1287,  1288,   262,   263,     0,
     264,   265,     0,     0,   266,   267,   268,   269,     0,     0,
       0,     0,     0,  1296,     0,     0,     0,     0,     0,   236,
       0,   270,     0,   271,   218,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
     218,     0,   850,     0,     0,     0,     0,     0,     0,     0,
       0,   273,     0,   274,     0,     0,     0,     0,     0,   850,
       0,   850,     0,     0,     0,   275,   276,   277,   278,   279,
     280,   281,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,   850,     0,     0,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,    50,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,     0,   218,  1382,   317,   318,   319,     0,     0,
       0,   320,   321,   322,   323,   324,     0,     0,     0,     0,
       0,  1397,     0,  1398,     0,     0,     0,     0,     0,     0,
       0,     0,   325,     0,     0,    88,   326,     0,    90,    91,
       0,    92,   177,    94,   327,  1415,   328,     0,     0,   329,
       0,     0,     0,     0,     0,     0,   330,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   331,     0,     0,     0,
    1717,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,     0,     5,     6,     7,     8,     9,     0,     0,
     850,     0,   850,    10,   850,  1095,   850,   236,     0,     0,
     850,     0,   850,     0,     0,   850,     0,   401,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   748,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   218,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,  1505,     0,  1506,     0,  1507,   236,  1508,     0,
       0,     0,  1510,    50,  1511,     0,     0,  1512,     0,     0,
       0,    55,     0,   850,     0,     0,     0,     0,     0,    62,
      63,    64,   172,   173,   174,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   175,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,     0,  1595,     0,   107,   108,     0,
     109,   110,     0,     0,     0,   850,   850,   850,     0,     0,
       0,     0,   850,     0,     0,     0,     0,     0,     0,     0,
       0,  1755,     0,     0,     0,     0,     0,   262,   263,     0,
     264,   265,  1070,  1071,   266,   267,   268,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,  1072,   271,     0,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,  1093,  1094,     0,     0,     0,
       0,   273,     0,     0,     0,     0,     0,  1743,  1744,  1745,
    1095,     0,     0,     0,  1749,   275,   276,   277,   278,   279,
     280,   281,     0,     0,     0,    37,     0,   207,    40,     0,
       0,     0,     0,     0,     0,     0,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,    50,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,     0,     0,   735,   317,   318,   319,     0,     0,
       0,   320,   573,   574,   575,   576,   850,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   850,     0,     0,
       0,     0,   577,   850,     0,     0,     0,   850,    90,    91,
       0,    92,   177,    94,   327,     0,   328,     0,     0,   329,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1838,     0,     0,     0,     0,     0,     0,     0,   736,
       0,   107,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1805,   850,
       0,    11,    12,    13,     0,     0,     0,     0,     0,  1815,
       0,     0,     0,     0,     0,  1820,     0,     0,     0,  1822,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,  1857,    52,    53,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,    83,     0,     0,    84,     0,     0,
       0,     0,    85,    86,    87,    88,    89,     0,    90,    91,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,    99,     0,   100,
       0,   101,   102,   103,     0,     0,   104,     0,   105,   106,
    1134,   107,   108,     0,   109,   110,     5,     6,     7,     8,
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
       0,    52,    53,    54,    55,    56,    57,    58,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    84,     0,     0,     0,
       0,    85,    86,    87,    88,    89,     0,    90,    91,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,    99,     0,   100,     0,
     101,   102,   103,     0,     0,   104,     0,   105,   106,  1313,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
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
      52,    53,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,    83,     0,     0,    84,     0,     0,     0,     0,
      85,    86,    87,    88,    89,     0,    90,    91,     0,    92,
      93,    94,    95,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,    99,     0,   100,     0,   101,
     102,   103,     0,     0,   104,     0,   105,   106,     0,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,    83,     0,     0,    84,     0,     0,     0,     0,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,    95,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   104,     0,   105,   106,   677,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
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
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
      83,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      95,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,   105,   106,  1098,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
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
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,    83,
       0,     0,    84,     0,     0,     0,     0,   176,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   177,    94,    95,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   104,     0,   105,   106,  1148,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    74,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,    83,     0,
       0,    84,     0,     0,     0,     0,   176,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   177,    94,    95,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     104,     0,   105,   106,  1214,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,  1216,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,    83,     0,     0,
      84,     0,     0,     0,     0,   176,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   177,    94,    95,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   104,
       0,   105,   106,     0,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1383,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    84,
       0,     0,     0,     0,   176,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   177,    94,    95,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   104,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     5,     6,
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
       0,     0,    74,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,    83,     0,     0,    84,     0,
       0,     0,     0,   176,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   177,    94,    95,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,     0,   105,
     106,  1514,   107,   108,     0,   109,   110,     5,     6,     7,
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
       0,    74,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,    83,     0,     0,    84,     0,     0,
       0,     0,   176,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   177,    94,    95,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   104,     0,   105,   106,
    1746,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,  1792,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    84,     0,     0,     0,
       0,   176,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   177,    94,    95,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   104,     0,   105,   106,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
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
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,    83,     0,     0,    84,     0,     0,     0,     0,
     176,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     177,    94,    95,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   104,     0,   105,   106,  1827,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,  1830,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,    83,     0,     0,    84,     0,     0,     0,     0,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,    95,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   104,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
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
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
      83,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      95,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,   105,   106,  1847,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
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
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,    83,
       0,     0,    84,     0,     0,     0,     0,   176,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   177,    94,    95,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   104,     0,   105,   106,  1864,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    74,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,    83,     0,
       0,    84,     0,     0,     0,     0,   176,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   177,    94,    95,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     104,     0,   105,   106,  1905,   107,   108,     0,   109,   110,
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
       0,     0,     0,     0,    74,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,    83,     0,     0,
      84,     0,     0,     0,     0,   176,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   177,    94,    95,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   104,
       0,   105,   106,  1912,   107,   108,     0,   109,   110,     5,
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
       0,     0,     0,    74,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    84,
       0,     0,     0,     0,   176,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   177,    94,    95,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   104,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,   541,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,   172,   173,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,    83,     0,     0,    84,     0,
       0,     0,     0,   176,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   177,    94,     0,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,     0,   105,
     106,     0,   107,   108,     0,   109,   110,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   812,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   172,   173,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,    83,     0,     0,    84,     0,     0,
       0,     0,   176,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   177,    94,     0,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   104,     0,   105,   106,
       0,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1034,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   172,   173,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    84,     0,     0,     0,
       0,   176,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   177,    94,     0,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   104,     0,   105,   106,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1590,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   172,   173,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,    83,     0,     0,    84,     0,     0,     0,     0,
     176,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     177,    94,     0,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   104,     0,   105,   106,     0,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1738,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   172,   173,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,    83,     0,     0,    84,     0,     0,     0,     0,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,     0,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   104,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   172,   173,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
      83,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,   105,   106,     0,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
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
      64,   172,   173,   174,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   175,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,   176,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   177,    94,     0,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
    1071,   178,     0,   340,     0,     0,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,  1072,
       0,    10,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,     0,     0,   625,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1095,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     172,   173,   174,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   175,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,   176,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   177,    94,     0,   626,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     178,     0,     0,     0,     0,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   172,
     173,   174,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   175,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,   176,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   177,    94,     0,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   178,
       0,     0,   807,     0,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,  1072,     0,    10,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,     0,     0,  1129,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1095,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   172,   173,
     174,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   175,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,   176,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   177,    94,     0,  1130,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   178,     0,
       0,     0,     0,   107,   108,     0,   109,   110,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   401,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   172,   173,   174,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   175,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,   176,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   177,    94,     0,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,   440,   441,
     442,     0,   107,   108,     0,   109,   110,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   469,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   189,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   172,   173,   174,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   175,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,   176,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   177,    94,     0,     0,     0,    96,     0,     0,
      97,     0,     0,     0,  1171,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   178,     0,     0,     0,
       0,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,     0,
       0,   222,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1095,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   172,   173,   174,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     175,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,   176,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   177,    94,     0,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   178,   440,   441,   442,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   443,   444,     0,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,     0,   468,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   469,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   172,   173,   174,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   175,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
     176,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     177,    94,     0,     0,     0,    96,     0,     0,    97,     0,
       0,     0,  1182,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   178,     0,   257,   441,   442,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   443,   444,     0,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,     0,   468,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,   469,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   172,   173,   174,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   175,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,     0,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   178,     0,   260,     0,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   401,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   172,   173,   174,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   175,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,   440,   441,   442,     0,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   443,   444,     0,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   469,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   172,   173,   174,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   175,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,   176,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   177,    94,     0,
       0,     0,    96,     0,     0,    97,     0,     0,     0,  1208,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   178,   539,     0,     0,     0,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0, -1024, -1024, -1024, -1024,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     702,   468,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   469,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     172,   173,   174,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   175,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,   176,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   177,    94,     0,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     178,     0,     0,     0,     0,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,   748,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   172,
     173,   174,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   175,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,   176,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   177,    94,     0,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   178,
       0,     0,     0,     0,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
       0,     0,     0,     0,   789,     0,     0,     0,     0,     0,
       0,     0,     0,  1095,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   172,   173,
     174,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   175,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,   176,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   177,    94,     0,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   178,     0,
       0,     0,     0,   107,   108,     0,   109,   110,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,     0,     0,
       0,     0,     0,   791,     0,     0,     0,     0,     0,     0,
       0,  1095,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   172,   173,   174,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   175,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,   176,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   177,    94,     0,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   178,     0,     0,
       0,     0,   107,   108,     0,   109,   110,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,     0,     0,     0,     0,     0,
       0,     0,  1126,     0,     0,     0,     0,     0,  1095,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   172,   173,   174,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   175,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,   176,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   177,    94,     0,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   178,     0,     0,     0,
       0,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10, -1024, -1024, -1024,
   -1024,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,     0,     0,     0,     0,     0,     0,
       0,  1205,     0,     0,     0,     0,     0,  1095,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   172,   173,   174,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     175,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,   176,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   177,    94,     0,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   178,     0,     0,     0,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   172,   173,   174,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   175,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
     176,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     177,    94,     0,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   178,   440,   441,   442,     0,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   469,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   172,   173,   174,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   175,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,     0,     0,     0,    96,     0,     0,    97,     0,     0,
       0,  1560,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   178,   440,   441,   442,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   443,   444,     0,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
       0,   468,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   469,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,   638,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   172,   173,   174,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   175,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
    1561,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   178,     0,     0,     0,     0,   107,   108,     0,
     109,   110,   262,   263,     0,   264,   265,     0,     0,   266,
     267,   268,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,     0,   271,     0,
     443,   444,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   273,   468,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   469,
     275,   276,   277,   278,   279,   280,   281,     0,     0,     0,
      37,     0,   207,    40,     0,     0,     0,     0,     0,     0,
       0,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,    50,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,     0,     0,   316,
     317,   318,   319,     0,     0,     0,   320,   573,   574,   575,
     576,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   263,     0,   264,   265,     0,   577,   266,   267,
     268,   269,     0,    90,    91,     0,    92,   177,    94,   327,
       0,   328,     0,     0,   329,   270,     0,   271,     0,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   736,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,   273,     0,   274,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   275,
     276,   277,   278,   279,   280,   281,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,    50,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,     0,     0,     0,   317,
     318,   319,     0,     0,     0,   320,   321,   322,   323,   324,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   325,     0,     0,    88,
     326,     0,    90,    91,     0,    92,   177,    94,   327,     0,
     328,     0,     0,   329,   262,   263,     0,   264,   265,     0,
     330,   266,   267,   268,   269,     0,     0,     0,     0,     0,
     331,     0,     0,     0,  1787,     0,     0,     0,   270,     0,
     271,   444,   272,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,     0,   468,   273,     0,
     274,     0,     0,     0,     0,     0,     0,     0,     0,   469,
       0,     0,   275,   276,   277,   278,   279,   280,   281,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,    50,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,     0,
       0,   316,   317,   318,   319,     0,     0,     0,   320,   321,
     322,   323,   324,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   325,
       0,     0,    88,   326,     0,    90,    91,     0,    92,   177,
      94,   327,     0,   328,     0,     0,   329,   262,   263,     0,
     264,   265,     0,   330,   266,   267,   268,   269,     0,     0,
       0,     0,     0,   331,     0,     0,     0,     0,     0,     0,
       0,   270,     0,   271,     0,   272,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,   273,     0,   274,     0,     0,     0,     0,     0,     0,
       0,     0,   469,     0,     0,   275,   276,   277,   278,   279,
     280,   281,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,    50,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,     0,     0,     0,   317,   318,   319,     0,     0,
       0,   320,   321,   322,   323,   324,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   325,     0,     0,    88,   326,     0,    90,    91,
       0,    92,   177,    94,   327,     0,   328,     0,     0,   329,
       0,   262,   263,     0,   264,   265,   330,  1518,   266,   267,
     268,   269,     0,     0,     0,     0,   331,     0,     0,     0,
       0,     0,     0,     0,     0,   270,     0,   271,     0,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   273,     0,   274,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   275,
     276,   277,   278,   279,   280,   281,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,    50,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,     0,     0,     0,   317,
     318,   319,     0,     0,     0,   320,   321,   322,   323,   324,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   325,     0,     0,    88,
     326,     0,    90,    91,     0,    92,   177,    94,   327,     0,
     328,     0,     0,   329,  1615,  1616,  1617,  1618,  1619,     0,
     330,  1620,  1621,  1622,  1623,     0,     0,     0,     0,     0,
     331,     0,     0,     0,     0,     0,     0,     0,  1624,  1625,
    1626,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1627,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1628,  1629,  1630,  1631,  1632,  1633,  1634,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1635,  1636,  1637,  1638,  1639,  1640,  1641,
    1642,  1643,  1644,  1645,    50,  1646,  1647,  1648,  1649,  1650,
    1651,  1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,
    1661,  1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,
    1671,  1672,  1673,  1674,  1675,     0,     0,     0,  1676,  1677,
    1678,  1679,     0,  1680,  1681,  1682,  1683,  1684,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1685,
    1686,  1687,     0,     0,     0,    90,    91,     0,    92,   177,
      94,  1688,     0,  1689,  1690,     0,  1691,   440,   441,   442,
       0,     0,     0,  1692,  1693,     0,  1694,     0,  1695,  1696,
       0,     0,     0,     0,     0,     0,     0,   443,   444,  1387,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,   440,   441,   442,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,     0,     0,   443,   444,     0,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   469,     0,     0,     0,     0,     0,     0,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     444,  1388,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,   440,   441,   442,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,   470,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,   555,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,   440,   441,   442,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   557,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,     0,   468,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     444,   582,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,   586,
       0,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,  1094,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1095,     0,    34,    35,
      36,    37,     0,   207,    40,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,   781,     0,     0,     0,     0,     0,
       0,     0,   262,   263,     0,   264,   265,     0,     0,   266,
     267,   268,   269,     0,   227,     0,     0,     0,     0,     0,
     228,     0,     0,     0,     0,     0,   270,     0,   271,     0,
       0,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,   804,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,   273,    92,   177,    94,
       0,     0,     0,    96,     0,     0,     0,     0,     0,     0,
     275,   276,   277,   278,   279,   280,   281,   101,     0,     0,
      37,     0,   229,     0,     0,     0,     0,   107,     0,     0,
    1255,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,    50,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,     0,     0,   316,
     317,   318,   319,     0,     0,     0,   320,   573,   574,   575,
     576,     0,     0,     0,     0,     0,   262,   263,     0,   264,
     265,     0,     0,   266,   267,   268,   269,   577,     0,     0,
       0,     0,     0,    90,    91,     0,    92,   177,    94,   327,
     270,   328,   271,     0,   329,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,    50,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
       0,     0,     0,  1256,   317,   318,   319,     0,     0,     0,
     320,   573,   574,   575,   576,     0,     0,     0,     0,     0,
     262,   263,     0,   264,   265,     0,     0,   266,   267,   268,
     269,   577,     0,     0,     0,     0,     0,    90,    91,     0,
      92,   177,    94,   327,   270,   328,   271,     0,   329,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   273,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   275,   276,
     277,   278,   279,   280,   281,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
      50,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,     0,     0,     0,   317,   318,
     319,     0,  1264,     0,   320,   573,   574,   575,   576,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     833,   834,     0,     0,     0,   577,   835,     0,   836,     0,
       0,    90,    91,     0,    92,   177,    94,   327,     0,   328,
     837,     0,   329,     0,     0,     0,     0,     0,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,   440,   441,
     442,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,   838,   839,
     840,     0,    78,    79,    80,    81,    82,   469,  1028,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,   841,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,     0,     0,     0,     0,     0,
      29,     0,   842,     0,     0,     0,     0,   101,    34,    35,
      36,    37,   843,   207,    40,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,   516,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1029,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
     833,   834,     0,    96,     0,     0,   835,     0,   836,     0,
       0,     0,     0,     0,     0,     0,     0,   101,     0,     0,
     837,     0,   213,     0,     0,     0,     0,   107,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,   440,   441,
     442,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,   838,   839,
     840,     0,    78,    79,    80,    81,    82,   469,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,   841,     0,    90,    91,     0,    92,   177,    94,
      29,     0,     0,    96,     0,     0,     0,     0,    34,    35,
      36,    37,   842,   207,    40,     0,     0,   101,     0,     0,
       0,   208,   843,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   525,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      29,     0,     0,    96,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   207,    40,     0,     0,   101,     0,     0,
       0,   208,   213,     0,     0,   598,     0,   107,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   618,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      29,     0,   979,    96,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   207,    40,     0,     0,   101,     0,     0,
       0,   208,   213,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      29,     0,     0,    96,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   207,    40,     0,     0,   101,     0,     0,
       0,   208,   213,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1123,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
      29,     0,     0,    96,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   207,    40,     0,     0,   101,     0,     0,
       0,   208,   213,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   210,
     211,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   212,     0,     0,     0,     0,   176,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   177,    94,
       0,     0,     0,    96,     0,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   213,     0,     0,   443,   444,   107,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,   440,   441,   442,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,     0,     0,   443,   444,     0,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,     0,     0,     0,     0,     0,     0,     0,     0,   440,
     441,   442,   469,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     444,   898,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,   440,   441,   442,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,   965,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,     0,     0,     0,     0,     0,
       0,     0,     0,   440,   441,   442,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   443,   444,  1013,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,  1069,  1070,  1071,     0,     0,     0,     0,     0,     0,
       0,     0,   469,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1072,  1309,     0,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,  1093,  1094,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1069,  1070,  1071,     0,
    1095,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1072,     0,  1340,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,     0,     0,  1069,  1070,  1071,     0,     0,     0,
       0,     0,     0,     0,     0,  1095,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1072,     0,  1240,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1069,
    1070,  1071,     0,  1095,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1072,     0,  1403,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,     0,     0,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,  1095,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,     0,
    1411,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,  1093,  1094,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1069,  1070,  1071,     0,  1095,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1072,     0,  1504,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,     0,    34,
      35,    36,    37,     0,   207,    40,     0,     0,     0,     0,
       0,  1095,   652,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1596,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,  1598,   176,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   177,
      94,     0,     0,     0,    96,     0,    34,    35,    36,    37,
       0,   207,    40,     0,     0,     0,     0,     0,   101,   208,
       0,     0,     0,   653,     0,     0,     0,     0,   654,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,   176,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   177,    94,     0,     0,
       0,    96,     0,   440,   441,   442,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     229,   816,     0,   443,   444,   107,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   469,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   440,   441,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   817,   443,   444,   962,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,     0,   468,   440,   441,   442,     0,     0,     0,
       0,     0,     0,     0,     0,   469,     0,     0,     0,     0,
       0,     0,     0,     0,   443,   444,     0,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
       0,   468,  1069,  1070,  1071,     0,     0,     0,     0,     0,
       0,     0,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1072,  1416,     0,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1069,  1070,
    1071,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1095,     0,     0,     0,     0,     0,     0,     0,  1072,
       0,     0,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,   442,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1095,     0,     0,
       0,     0,   443,   444,     0,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,     0,   468,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   469
};

static const yytype_int16 yycheck[] =
{
       5,     6,    56,     8,     9,    10,    11,    12,    13,     4,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   394,   154,    29,    30,   125,   628,     4,     4,
      33,   180,     4,   745,    31,    31,    95,     4,   104,    44,
      99,   100,   510,    46,   394,    56,     4,    52,    51,    54,
      57,   942,    57,   159,    59,  1151,   783,   665,    31,   529,
     931,   377,   154,   230,   468,   124,    57,   104,   669,   347,
     348,   500,   592,   593,  1027,   500,  1138,     4,    83,   958,
      44,    31,   181,   541,   645,   322,   323,   104,   104,   815,
       4,   822,     9,    32,     9,   974,    32,    83,     9,   104,
      49,     9,    49,    60,   420,     9,   666,     9,   537,    49,
       4,     9,   537,   241,     9,    49,     9,    70,     9,    83,
       9,    14,    38,    14,    49,   583,     9,     9,    85,   793,
       9,    88,    14,     9,     9,  1014,   242,     9,    14,    53,
       9,   178,    56,     9,    70,   115,     9,     9,    83,     9,
     125,    14,     9,    90,     9,    49,     9,    36,     9,    73,
      83,   178,   178,     9,     9,   157,   122,    83,    90,    50,
      51,    38,   157,   178,   130,     0,   213,    38,   154,    93,
     185,    95,    70,   134,   135,    99,   100,     8,   504,   505,
     178,    38,   229,   102,  1065,    38,   213,   213,   192,   178,
     192,   178,   134,   135,   192,   175,   181,   192,   213,   667,
     124,   197,   229,   192,   192,   192,    83,   154,   112,   535,
     155,   156,    83,   117,   229,   119,   120,   121,   122,   123,
     124,   125,   154,    70,   157,    70,    83,   192,   243,   192,
      83,   246,   195,   190,   193,   196,   193,    83,   253,   254,
      70,   379,   161,   193,    70,   171,   195,    70,   194,   193,
      70,    70,    70,    70,    70,   190,   192,   868,  1221,   195,
     164,   165,    70,   167,    70,  1337,  1426,   194,   195,   194,
     194,   430,  1344,   194,  1346,   339,   194,   178,   168,   247,
     194,    70,   194,   251,   188,   193,   178,    70,   179,   194,
    1179,   194,   196,   194,   171,   194,  1037,   195,  1039,  1371,
     171,   194,   194,   977,   193,   178,   106,   107,   194,   194,
     157,   193,   559,   560,   171,   194,    54,   193,   339,  1200,
     367,   194,   194,   247,   194,   171,   193,   251,   225,   194,
     193,   255,   193,    70,   864,   865,  1147,   193,   193,   807,
     367,   367,    56,    70,   812,   134,   135,   192,   195,   920,
     195,   366,   367,   512,    38,   424,   178,   421,   373,   374,
     375,   376,   377,   157,  1524,   195,   192,   382,   192,   195,
     192,   192,   195,   482,   157,   195,   195,   195,   195,   195,
     347,   348,   349,    70,   178,   955,   401,   195,  1548,    70,
    1550,   192,   366,    70,   409,    54,   196,   134,   135,    83,
     421,  1473,   376,    83,    84,   420,   195,   483,   477,   478,
     479,   480,   195,    70,  1136,   339,   894,    83,    84,   386,
     157,   195,   408,   408,   162,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   474,   469,   192,   471,   472,   473,  1196,
    1197,   468,   468,   400,  1201,   192,   483,    83,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,  1217,   483,   192,   408,   468,   501,   502,   102,   504,
     505,   506,   507,   417,   192,   157,   481,   421,   513,   192,
     424,   516,   523,   162,   394,   192,   193,    83,   468,   948,
     525,   192,   527,   948,   474,   192,   178,   197,   177,    83,
     535,     4,   106,   107,  1335,    31,   543,  1408,   543,  1731,
     545,   197,  1150,   195,   672,   192,   674,    70,   192,   102,
     500,   102,   683,    83,    27,    28,   192,   161,    83,   931,
     474,   475,   476,   477,   478,   479,   480,    83,   272,   192,
     274,   738,    83,   523,    90,   979,  1034,    75,    76,    90,
     858,   931,   860,   189,   534,    81,   500,   537,  1108,   195,
     548,   683,    81,   598,   474,    91,  1788,   162,   747,   192,
     154,   155,   156,  1163,   771,   171,  1166,   103,   161,   523,
     161,   134,   135,   472,   103,    81,   653,  1488,    83,   161,
     500,   626,   196,   537,   163,    90,  1353,   331,  1355,   134,
     135,   104,   519,  1345,   548,   592,   593,   103,   697,   155,
     156,   171,   501,   523,   155,   156,   171,   506,   653,   195,
     178,    32,   194,   195,   534,   971,   195,   537,   572,   787,
     788,  1462,   158,  1464,   192,   161,   794,   795,   164,   165,
     161,   167,   168,   169,    83,   164,   165,   844,   167,   168,
     169,    90,   596,   597,    57,   201,   853,   192,   794,   200,
     155,   156,   158,  1065,    38,   161,    69,   702,   164,   165,
     192,   167,   168,   169,    31,   178,   194,   683,   194,    83,
     194,  1181,    83,    81,   418,  1065,    90,   421,  1326,    90,
     634,   635,   119,   120,   121,   122,   123,   124,    70,  1441,
     196,   736,   380,    14,    83,   103,   384,   624,    81,   666,
     213,    90,   194,   630,   194,   154,   155,   156,   194,   222,
     194,    32,  1220,  1480,    81,  1482,   229,  1484,   194,  1486,
     103,   766,   410,    70,   412,   413,   414,   415,  1569,   656,
      51,    70,  1573,    70,   247,   195,   103,   157,   251,  1339,
    1876,   155,   156,   697,   155,   156,   192,    78,    79,    80,
      81,   188,   192,   161,   799,  1891,   164,   165,   125,   167,
     168,   169,    53,    54,    55,   154,   155,   156,   811,   123,
     815,   138,   103,  1525,   141,    70,   130,   157,    69,   162,
     192,   164,   165,   166,   167,   168,   169,   195,  1200,   805,
     805,   158,   163,  1291,   161,   162,  1304,   164,   165,    31,
     167,   168,   169,   161,    31,   176,   194,   734,    48,   192,
    1200,    50,    51,  1580,    75,    76,  1172,    69,    50,   192,
     564,    53,   820,    50,   195,   157,    53,  1183,   192,   783,
     178,   785,   192,   164,   165,   192,   167,   168,   169,   855,
     855,   194,   195,   356,   199,   119,   120,   121,   122,   123,
     124,   805,   365,   898,   367,   900,   130,   131,     9,   372,
     905,   858,   907,   860,   157,   819,   820,   864,   865,   866,
     383,   916,   102,   103,  1474,  1373,   132,   133,   157,   978,
     105,   106,   107,   934,   919,   930,   105,   106,   107,  1730,
     157,  1389,   192,  1734,     8,   408,   170,   824,   194,   178,
     192,   855,   829,   919,   919,   194,   195,   919,   862,   863,
     157,  1842,   919,   192,   188,    14,   195,   962,   662,   663,
     965,   919,   967,   157,   940,   940,   971,   671,   119,   120,
     121,  1862,   194,     4,   108,   109,   110,   891,   347,   348,
    1871,   194,   979,   979,   934,    53,    54,    55,     9,    57,
    1157,   195,   919,  1762,  1763,   130,   946,   194,   948,  1758,
    1759,    69,   130,   890,    14,   919,   979,   193,  1013,  1736,
    1737,   178,   831,   832,    14,   942,    81,  1020,    49,   102,
     934,   193,   193,   193,   193,  1341,   940,   192,   955,   979,
     198,   111,   946,  1491,   948,   192,  1408,   510,   103,   192,
       9,   154,  1500,   193,   193,   193,   193,  1023,  1023,  1025,
    1025,   931,    94,  1064,   934,  1513,  1067,     9,  1408,  1125,
      14,   194,   178,  1021,   978,  1015,   946,   192,   948,  1870,
       9,   192,   195,   195,   194,   548,   990,   991,   992,   194,
      83,   112,   194,   193,  1885,   193,   117,   193,   119,   120,
     121,   122,   123,   124,   125,   132,   192,   162,   193,   164,
     165,  1015,   167,   168,   169,   199,     9,  1021,   193,  1023,
       9,  1025,     9,   199,   193,    70,  1488,    32,  1125,   133,
    1125,  1008,   177,   157,   136,  1130,     9,   831,   832,   193,
     157,  1045,  1590,   164,   165,  1015,   167,    14,  1488,   190,
    1135,   179,     9,     9,  1460,   193,   132,   199,     9,    14,
    1064,  1108,   625,  1067,   199,   196,    14,   188,   157,  1135,
    1135,   193,   192,  1135,   199,   196,  1053,  1172,  1135,   193,
     199,   193,   102,  1060,   194,   194,     9,  1135,  1183,  1184,
      91,  1782,  1096,   136,   157,  1065,     9,   193,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   192,   192,   195,     9,    14,  1209,    81,  1135,    83,
      84,    70,  1217,    81,    70,   111,   157,   921,   195,   157,
     196,  1135,  1227,   119,   120,   121,   122,   123,   124,   103,
     194,   179,     9,   937,   195,   103,  1163,   199,    14,  1166,
     190,    67,    68,    14,   195,   194,   950,   193,  1224,  1224,
      32,    32,   192,    14,  1212,   192,   192,   192,    14,  1860,
     128,    52,    81,   192,    83,    84,    70,    70,   192,     9,
    1728,   975,   193,   192,   194,   748,   194,    14,   179,   136,
    1738,  1195,  1196,  1197,   103,   136,     9,  1201,   162,   193,
     164,   165,   188,   167,   168,   169,   164,   165,  1212,   167,
     168,   169,    69,     9,  1309,   199,  1193,    81,   134,   135,
    1224,  1442,   196,  1318,    83,   196,   789,  1322,   791,  1324,
    1200,   195,   194,   197,   192,  1783,  1302,  1332,     9,   103,
     192,   194,   805,    83,   136,  1340,  1341,  1041,  1437,  1315,
    1044,    14,    81,   162,   817,   164,   165,   820,   167,   168,
     169,   192,   195,    50,    51,    52,    53,    54,    55,  1246,
      57,   193,   192,  1250,   103,   192,  1253,   193,   193,     9,
    1828,   195,    69,  1260,   195,  1302,   195,   194,   197,   136,
      91,   199,   855,  1297,   154,   195,    32,    77,  1315,   179,
     164,   165,   193,   167,   168,   169,   194,  1546,   194,    32,
     136,   193,    81,     9,   193,   878,   199,     9,   136,     9,
     193,   199,  1339,    81,     9,    83,   196,    85,   192,   194,
     193,   894,   895,   195,   103,   164,   165,    14,   167,   168,
     169,  1889,   194,    27,    28,   103,   196,    31,  1896,  1353,
      83,  1355,  1418,  1418,   192,  1149,   919,  1151,   193,   193,
     193,   193,  1457,   192,   195,  1460,   194,   192,     9,  1518,
     193,  1436,    56,     9,   136,   199,  1442,   940,   199,   136,
       9,  1175,   193,  1360,  1178,    32,   194,  1364,   193,   158,
     193,  1368,   161,   162,   194,   164,   165,  1463,   167,   168,
     169,   194,   112,  1469,   195,  1471,   164,   165,   194,   167,
     168,   169,   166,   162,  1418,    81,    14,   117,    83,   193,
    1521,   193,  1426,   136,  1490,   195,  1492,  1492,  1432,   193,
     136,    14,   178,    27,    28,  1501,  1501,   103,  1408,  1233,
     195,   194,    14,  1237,    83,    14,  1463,   192,    83,  1497,
     193,   193,  1469,   136,  1471,   136,   194,  1474,  1021,   194,
    1023,    14,  1025,    14,  1027,  1028,   194,    14,   195,  1564,
       9,     9,   196,  1490,    59,    83,  1480,  1716,  1482,   178,
    1484,   192,  1486,    78,    79,    80,    83,     9,  1492,  1554,
     195,   194,   115,  1497,   157,   161,    91,  1501,   164,   165,
     102,   167,   168,   169,  1298,  1299,   119,   120,   121,   122,
     123,   124,  1578,   102,  1518,   179,   169,  1521,  1488,    36,
    1524,  1587,  1587,    14,   192,   194,   193,  1593,  1593,   175,
    1534,   192,   172,  1599,  1599,   179,   179,  1541,   222,    83,
    1588,  1589,   193,     9,  1548,   194,  1550,   142,   143,   144,
     145,   146,    83,  1557,    14,   193,   193,    83,   153,    14,
      83,  1578,   195,  1126,   159,   160,  1129,  1711,    14,    83,
      14,    83,  1135,  1108,  1851,   188,  1580,   480,   173,   475,
     477,   194,   972,  1587,  1588,  1589,   922,    81,   272,  1593,
     274,  1867,   187,  1218,  1589,  1599,  1386,  1862,   600,  1393,
    1576,  1395,    50,    51,    52,    53,    54,    55,  1526,   103,
    1711,  1613,  1700,  1432,  1835,  1895,  1883,   111,   112,  1428,
    1712,    69,  1139,   991,  1572,  1061,  1721,  1776,   222,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,  1205,  1437,  1194,  1010,  1195,   331,   946,  1212,
     831,   119,   120,   121,   122,   123,   124,  1220,  1221,   373,
     421,  1224,   130,   131,  1817,  1421,  1855,   161,  1116,  1046,
     164,   165,   356,   167,   168,   169,  1742,  1742,  1096,    -1,
      -1,   365,    67,    68,    -1,    -1,    -1,    -1,   372,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   383,
     168,    -1,   170,    -1,    -1,    -1,    -1,  1711,    -1,    -1,
     394,    -1,    -1,    -1,    -1,   183,    -1,   185,  1784,  1784,
     188,    -1,    -1,    -1,    -1,  1791,  1791,    -1,  1732,    -1,
      -1,    -1,  1736,  1737,   418,    -1,    -1,   421,  1742,    -1,
      -1,  1304,    -1,     4,    -1,    -1,    -1,  1751,    -1,   134,
     135,    -1,    -1,    -1,  1758,  1759,    -1,    -1,  1762,  1763,
    1826,  1826,   356,    -1,    -1,    -1,    -1,    -1,    -1,  1835,
      -1,   365,  1776,   367,    -1,  1752,  1753,    -1,   372,    -1,
    1784,    -1,  1848,  1848,   468,    81,    -1,  1791,    49,   383,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,  1901,   103,   193,    -1,
      -1,    -1,    -1,    -1,  1909,   111,   112,  1611,    -1,    -1,
    1915,    -1,  1826,  1918,    -1,  1842,   510,    -1,    -1,     4,
    1834,  1897,  1897,    -1,    -1,    -1,    -1,    -1,  1904,  1904,
      -1,    -1,    -1,    -1,  1848,  1862,    67,    68,    -1,     4,
    1854,   112,    -1,    -1,  1871,  1418,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,   161,    -1,    -1,   164,   165,
      -1,   167,   168,   169,    49,    -1,    -1,    -1,    -1,    -1,
     564,  1444,    -1,     4,   568,    -1,    -1,   571,    27,    28,
      -1,    -1,    -1,  1897,    49,    -1,    -1,    -1,    -1,    -1,
    1904,    -1,    -1,   164,   165,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   510,    -1,    -1,    -1,
     604,  1715,    -1,    -1,    -1,    -1,    -1,   188,    49,  1492,
      -1,    81,     4,    -1,  1497,   196,    -1,   112,  1501,    -1,
      -1,   625,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,   103,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   193,    -1,    -1,    -1,    -1,    49,   662,   663,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   671,    -1,   164,
     165,   112,   167,    -1,    -1,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,  1800,    27,    28,   164,
     165,   161,   167,   188,   164,   165,    -1,   167,   168,   169,
      -1,   196,    -1,    -1,  1587,  1588,  1589,    -1,    -1,    -1,
    1593,   625,    -1,   188,    -1,    -1,  1599,    -1,    -1,    -1,
     112,   196,    -1,   164,   165,   117,   167,   119,   120,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   748,    -1,    -1,   188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,
      -1,    -1,  1876,   222,    -1,    -1,    -1,    27,    28,    -1,
      -1,    31,   164,   165,    -1,   167,    -1,  1891,    -1,    -1,
      81,    -1,    -1,    -1,    -1,   789,    -1,   791,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,   103,    -1,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   817,   818,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,   125,    -1,    -1,   831,   832,   833,
     834,   835,   836,   837,   748,    -1,    -1,   138,    -1,   843,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      -1,    -1,   856,    -1,    -1,    -1,    -1,   158,    -1,  1742,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,    81,    -1,    -1,   878,   789,    -1,   791,    -1,    -1,
      -1,    -1,   222,    -1,    -1,    -1,    -1,    -1,   892,    -1,
     894,   895,    -1,   103,    -1,    -1,    -1,   356,    -1,    -1,
      -1,  1784,    -1,   817,    -1,    -1,   365,    -1,  1791,    -1,
      -1,    -1,    -1,   372,    -1,    -1,    -1,   921,   922,    -1,
      -1,    -1,    -1,    -1,   383,    -1,    -1,   931,   138,   139,
     140,   141,    -1,   937,    -1,   394,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1826,    -1,    -1,   950,    -1,   158,    -1,
     954,   161,   162,   957,   164,   165,    -1,   167,   168,   169,
      -1,   171,   222,    -1,   878,  1848,    -1,    -1,    10,    11,
      12,   975,   182,    -1,    -1,   979,    -1,    -1,    -1,    -1,
     894,   895,   192,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1897,    57,   356,    -1,    -1,    -1,
      -1,  1904,    -1,  1027,  1028,   365,    -1,    69,    -1,    -1,
      -1,    -1,   372,    -1,    -1,    -1,    -1,  1041,    -1,    -1,
    1044,    -1,  1046,   383,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   510,    -1,    -1,    -1,    -1,    -1,  1061,  1062,  1063,
      -1,  1065,    -1,    -1,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
    1094,  1095,    -1,    -1,    -1,    -1,   356,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   365,    -1,    -1,  1112,    -1,
      -1,    -1,   372,  1027,  1028,    31,    -1,    -1,    -1,    -1,
      -1,    -1,  1126,   383,    -1,  1129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   394,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    -1,  1149,    -1,  1151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,   625,    -1,    -1,    -1,
     510,  1175,    -1,    -1,  1178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,  1200,    -1,    -1,    -1,
      -1,  1205,    -1,    -1,    -1,    -1,    -1,    -1,   468,    31,
      -1,    -1,  1126,    -1,    -1,  1129,  1220,  1221,    -1,  1223,
      -1,    -1,   138,   139,   140,   141,    -1,    -1,    -1,  1233,
      -1,    -1,    -1,  1237,    -1,    -1,  1240,    59,  1242,    -1,
      -1,    -1,   158,    -1,    -1,   161,   162,    -1,   164,   165,
     510,   167,   168,   169,    -1,    -1,    -1,    -1,    -1,    81,
    1264,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,   625,    -1,    -1,    -1,   748,
      -1,  1205,    56,    -1,  1298,  1299,    -1,  1301,    -1,    -1,
    1304,    -1,    -1,    -1,    -1,    -1,  1220,  1221,    -1,    -1,
      -1,   571,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     789,    -1,   791,    -1,    -1,    -1,   158,    -1,    -1,   161,
     162,    -1,   164,   165,   604,   167,   168,   169,    -1,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   817,    -1,
     182,    -1,    -1,    -1,    -1,   625,    -1,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,  1393,
    1304,  1395,    -1,    -1,    -1,    -1,    -1,  1401,    -1,  1403,
      -1,  1405,    -1,  1407,  1408,    -1,    81,  1411,   748,  1413,
      -1,    -1,  1416,    -1,    -1,    -1,    -1,    -1,    -1,   878,
      -1,    -1,  1426,  1427,    -1,    -1,  1430,    -1,   103,    -1,
      -1,    -1,    -1,  1437,    -1,   894,   895,    -1,    -1,    -1,
    1444,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   789,
      -1,   791,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,   931,    -1,    -1,    -1,    -1,   817,    -1,    -1,
      -1,    -1,    -1,   158,  1488,    -1,   161,   162,   748,   164,
     165,    -1,   167,   168,   169,    -1,    -1,    -1,    -1,    -1,
    1504,    -1,    -1,    -1,    -1,    -1,    -1,   182,   272,    -1,
     274,    -1,  1516,  1517,    -1,    -1,    -1,   192,    -1,    -1,
    1524,    -1,  1526,    -1,    -1,    -1,    -1,    -1,    -1,   789,
    1444,   791,    -1,    -1,    -1,    -1,    -1,    -1,   878,    -1,
      -1,    -1,    -1,    -1,  1548,    -1,  1550,    -1,    -1,    -1,
      -1,    -1,    -1,  1557,   894,   895,    -1,   817,   818,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   331,  1027,  1028,
      -1,    -1,    -1,   833,   834,   835,   836,   837,    -1,    -1,
      -1,    -1,    -1,   843,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1596,  1597,  1598,    -1,   856,    -1,    -1,  1603,
      -1,  1605,    -1,    -1,    -1,    -1,  1065,  1611,    -1,  1613,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   878,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   892,    -1,   894,   895,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,   418,    -1,    -1,   421,    -1,    -1,
      91,    -1,   922,    -1,    -1,    -1,    -1,  1126,    -1,    -1,
    1129,   931,   103,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,  1027,  1028,    -1,
      -1,    -1,    -1,    -1,   954,    -1,    -1,   957,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
      -1,  1715,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   979,
      -1,    -1,   153,    -1,    -1,    -1,    67,    68,  1732,    -1,
      -1,    -1,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,  1200,    -1,    -1,    -1,    -1,  1205,  1751,    -1,    -1,
      -1,   182,    -1,  1757,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1220,  1221,    -1,  1768,    -1,    -1,  1027,  1028,    -1,
    1774,    -1,    -1,    -1,  1778,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1126,    -1,    -1,  1129,
      -1,    -1,    -1,   134,   135,    -1,  1800,    -1,    -1,    -1,
     564,  1061,  1062,  1063,   568,  1065,    -1,   571,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,  1093,  1094,  1095,  1840,    -1,    -1,    -1,
     604,    -1,    -1,    -1,    -1,  1304,  1850,    -1,    -1,    -1,
      -1,    -1,  1112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1867,    -1,  1205,  1126,    -1,    -1,  1129,
      -1,    -1,  1876,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1220,  1221,    -1,    -1,    -1,    -1,    -1,  1891,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   662,   663,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   671,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
    1200,    -1,    -1,    -1,    -1,  1205,    -1,    69,    -1,  1408,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1220,  1221,    -1,  1223,  1304,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1240,    -1,  1242,    -1,    -1,  1444,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1264,    57,    27,    28,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1488,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1301,    -1,    -1,  1304,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   818,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   831,   832,   833,
     834,   835,   836,   837,   196,    -1,    -1,    -1,    -1,   843,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1444,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1401,    -1,  1403,    69,  1405,    -1,  1407,  1408,    -1,
      -1,  1411,    -1,  1413,    -1,    -1,  1416,   921,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1427,    -1,    -1,
    1430,    -1,    -1,   937,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1444,    -1,   950,    -1,    -1,    -1,
     954,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   975,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,  1488,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,  1504,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1516,  1517,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1526,    -1,    -1,    -1,
      -1,   196,    -1,    -1,    -1,    -1,    -1,  1041,    -1,    -1,
    1044,    -1,  1046,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1061,  1062,  1063,
      -1,    -1,    -1,    -1,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
    1094,  1095,    -1,    -1,    -1,   356,  1596,  1597,  1598,    -1,
      -1,    -1,    -1,  1603,   365,  1605,    -1,    -1,  1112,    -1,
      -1,   372,    -1,  1613,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   394,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1149,    -1,  1151,    -1,    -1,
      -1,    -1,    10,    11,    12,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,  1175,    30,    31,  1178,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   468,    67,    68,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1223,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1233,
      -1,    -1,    -1,  1237,    -1,    -1,  1240,    -1,  1242,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   510,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1757,    -1,    -1,
    1264,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1768,    -1,
      -1,    -1,    -1,    -1,  1774,   134,   135,    -1,  1778,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,  1298,  1299,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     571,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   196,    -1,
    1840,    -1,    -1,   604,    -1,    -1,    -1,    69,    -1,    -1,
    1850,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   625,    -1,    -1,  1867,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1393,
      57,  1395,   571,    -1,    -1,    -1,    -1,  1401,    -1,  1403,
      -1,  1405,    69,  1407,    -1,    -1,    -1,  1411,    -1,  1413,
      -1,    -1,  1416,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,  1426,    -1,    -1,   604,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1437,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   748,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1504,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1524,    57,    -1,    -1,    -1,    -1,    -1,    -1,   789,    -1,
     791,    -1,    -1,    69,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,  1548,    -1,  1550,    -1,    -1,    -1,
      -1,    -1,    -1,  1557,    -1,    -1,   817,   818,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,   833,   834,   835,   836,   837,    -1,    -1,    -1,
      -1,    -1,   843,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,  1596,  1597,  1598,    -1,    -1,    -1,    -1,  1603,
     196,    -1,    -1,    -1,    -1,    -1,    -1,  1611,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,   878,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,   894,   895,    -1,    -1,    -1,    -1,   818,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,   833,   834,   835,   836,    -1,    -1,
      -1,    -1,    -1,    -1,   843,   158,    -1,    -1,   161,   162,
     931,   164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,   954,    -1,   188,    -1,    -1,    -1,   192,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1715,    -1,    -1,    -1,    -1,    -1,    -1,   979,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1732,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1751,    -1,    -1,
      -1,    -1,    -1,  1757,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,  1768,    -1,  1027,  1028,    -1,    -1,
    1774,    -1,    -1,    -1,  1778,   954,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,  1800,    -1,    -1,    -1,
    1061,  1062,  1063,    -1,  1065,    -1,    69,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,  1094,  1095,    -1,  1840,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,  1112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,  1126,    -1,    -1,  1129,    -1,
      -1,    -1,  1876,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1062,  1063,    -1,    -1,  1891,    -1,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1093,  1094,  1095,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,  1112,    -1,    -1,    -1,    -1,    -1,  1200,
      -1,    27,    -1,    29,  1205,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,
    1221,    -1,  1223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,  1240,
      -1,  1242,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1264,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,  1304,  1223,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,  1240,    -1,  1242,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,   170,  1264,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,
     196,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,     3,     4,     5,     6,     7,    -1,    -1,
    1401,    -1,  1403,    13,  1405,    69,  1407,  1408,    -1,    -1,
    1411,    -1,  1413,    -1,    -1,  1416,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,  1444,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,  1401,    -1,  1403,    -1,  1405,  1488,  1407,    -1,
      -1,    -1,  1411,   103,  1413,    -1,    -1,  1416,    -1,    -1,
      -1,   111,    -1,  1504,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,    -1,  1504,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,    -1,  1596,  1597,  1598,    -1,    -1,
      -1,    -1,  1603,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1612,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    11,    12,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    31,    29,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,  1596,  1597,  1598,
      69,    -1,    -1,    -1,  1603,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,  1757,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1768,    -1,    -1,
      -1,    -1,   158,  1774,    -1,    -1,    -1,  1778,   164,   165,
      -1,   167,   168,   169,   170,    -1,   172,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1802,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,   197,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1757,  1840,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,  1768,
      -1,    -1,    -1,    -1,    -1,  1774,    -1,    -1,    -1,  1778,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,  1840,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
      -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,   195,
     196,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
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
      -1,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,   194,   195,   196,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
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
     108,   109,   110,   111,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,    -1,   187,
     188,   189,    -1,    -1,   192,    -1,   194,   195,    -1,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,   170,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    -1,   194,   195,   196,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
     170,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,   194,   195,   196,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,   150,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,   170,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      -1,   192,    -1,   194,   195,   196,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,   194,   195,   196,   197,   198,    -1,   200,   201,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,   170,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,   194,   195,    -1,   197,   198,    -1,   200,   201,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,   170,    -1,    -1,   173,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,
     194,   195,    -1,   197,   198,    -1,   200,   201,     3,     4,
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
      -1,    -1,   137,   138,   139,   140,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,
      -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,   164,
     165,    -1,   167,   168,   169,   170,    -1,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,
     195,   196,   197,   198,    -1,   200,   201,     3,     4,     5,
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
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,   195,
     196,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    99,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,   194,   195,    -1,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
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
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    -1,   192,    -1,   194,   195,   196,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    97,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,   170,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    -1,   194,   195,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
     170,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,   194,   195,   196,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,   150,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,   170,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      -1,   192,    -1,   194,   195,   196,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,   194,   195,   196,   197,   198,    -1,   200,   201,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,   170,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,   194,   195,   196,   197,   198,    -1,   200,   201,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,   170,    -1,    -1,   173,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,
     194,   195,    -1,   197,   198,    -1,   200,   201,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
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
      -1,    -1,   137,   138,   139,   140,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,
      -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,   164,
     165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,
     195,    -1,   197,   198,    -1,   200,   201,     3,     4,     5,
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
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,   195,
      -1,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,   194,   195,    -1,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
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
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    -1,   192,    -1,   194,   195,    -1,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
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
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    -1,   194,   195,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,   194,   195,    -1,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      12,   192,    -1,   194,    -1,    -1,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    31,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,    -1,   171,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,    -1,    -1,    -1,   197,   198,    -1,   200,   201,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,    -1,   171,    -1,   173,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,
      -1,    -1,    -1,   197,   198,    -1,   200,   201,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,
      -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,   164,
     165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    10,    11,
      12,    -1,   197,   198,    -1,   200,   201,     3,     4,     5,
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
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,
     176,    -1,    -1,    -1,   196,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,    -1,   192,    -1,    -1,    -1,
      -1,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    10,    11,    12,    -1,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,   196,    -1,   182,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    -1,   192,    -1,   194,    11,    12,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    69,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    -1,   194,    -1,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    10,    11,    12,    -1,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    31,    -1,    33,    34,    35,    36,
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
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,   196,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      -1,   192,   193,    -1,    -1,    -1,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,    -1,    -1,    -1,   197,   198,    -1,   200,   201,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,    -1,    -1,    -1,   197,   198,    -1,   200,   201,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,
      -1,    -1,    -1,   197,   198,    -1,   200,   201,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,
      -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,   164,
     165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,    -1,
      -1,    -1,   197,   198,    -1,   200,   201,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,    -1,   192,    -1,    -1,    -1,
      -1,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,    -1,    -1,    -1,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    -1,   192,    10,    11,    12,    -1,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
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
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,   196,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    10,    11,    12,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
     196,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,
     200,   201,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   158,    10,    11,
      12,    13,    -1,   164,   165,    -1,   167,   168,   169,   170,
      -1,   172,    -1,    -1,   175,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,   197,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
     172,    -1,    -1,   175,     3,     4,    -1,     6,     7,    -1,
     182,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     192,    -1,    -1,    -1,   196,    -1,    -1,    -1,    27,    -1,
      29,    31,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,   170,    -1,   172,    -1,    -1,   175,     3,     4,    -1,
       6,     7,    -1,   182,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,   170,    -1,   172,    -1,    -1,   175,
      -1,     3,     4,    -1,     6,     7,   182,   183,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
     172,    -1,    -1,   175,     3,     4,     5,     6,     7,    -1,
     182,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
     159,   160,    -1,    -1,    -1,   164,   165,    -1,   167,   168,
     169,   170,    -1,   172,   173,    -1,   175,    10,    11,    12,
      -1,    -1,    -1,   182,   183,    -1,   185,    -1,   187,   188,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   194,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   194,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   194,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   194,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   194,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   194,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   193,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,   124,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,   193,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    57,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,   187,    -1,    -1,
      81,    -1,   192,    -1,    -1,    -1,    -1,   197,    -1,    -1,
     193,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   158,    -1,    -1,
      -1,    -1,    -1,   164,   165,    -1,   167,   168,   169,   170,
      27,   172,    29,    -1,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   158,    -1,    -1,    -1,    -1,    -1,   164,   165,    -1,
     167,   168,   169,   170,    27,   172,    29,    -1,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,
     133,    -1,    32,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,   158,    56,    -1,    58,    -1,
      -1,   164,   165,    -1,   167,   168,   169,   170,    -1,   172,
      70,    -1,   175,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    69,    38,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    -1,   182,    -1,    -1,    -1,    -1,   187,    78,    79,
      80,    81,   192,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      50,    51,    -1,   173,    -1,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      70,    -1,   192,    -1,    -1,    -1,    -1,   197,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    69,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      70,    -1,    -1,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,   182,    83,    84,    -1,    -1,   187,    -1,    -1,
      -1,    91,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      70,    -1,    -1,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   187,    -1,    -1,
      -1,    91,   192,    -1,    -1,   195,    -1,   197,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      70,    -1,    72,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   187,    -1,    -1,
      -1,    91,   192,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      70,    -1,    -1,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   187,    -1,    -1,
      -1,    91,   192,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      70,    -1,    -1,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   187,    -1,    -1,
      -1,    91,   192,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    30,    31,   197,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   136,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,   136,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
     136,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,   136,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,    -1,    -1,   173,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   187,    91,
      -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,   173,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,    28,    -1,    30,    31,   197,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   203,   204,     0,   205,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   142,   143,
     144,   145,   146,   150,   153,   158,   159,   160,   161,   162,
     164,   165,   167,   168,   169,   170,   173,   176,   182,   183,
     185,   187,   188,   189,   192,   194,   195,   197,   198,   200,
     201,   206,   209,   219,   220,   221,   222,   223,   226,   242,
     243,   247,   250,   257,   263,   323,   324,   332,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   347,   350,
     362,   363,   370,   373,   379,   381,   382,   384,   394,   395,
     396,   398,   403,   407,   427,   435,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   450,   463,
     465,   467,   122,   123,   124,   137,   158,   168,   192,   209,
     242,   323,   344,   439,   344,   192,   344,   344,   344,   108,
     344,   344,   344,   425,   426,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,    83,    91,   124,
     139,   140,   153,   192,   220,   363,   395,   398,   403,   439,
     442,   439,    38,   344,   454,   455,   344,   124,   130,   192,
     220,   255,   395,   396,   397,   399,   403,   436,   437,   438,
     446,   451,   452,   192,   333,   400,   192,   333,   354,   334,
     344,   228,   333,   192,   192,   192,   333,   194,   344,   209,
     194,   344,     3,     4,     6,     7,    10,    11,    12,    13,
      27,    29,    31,    57,    59,    71,    72,    73,    74,    75,
      76,    77,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   130,   131,   132,   133,
     137,   138,   139,   140,   141,   158,   162,   170,   172,   175,
     182,   192,   209,   210,   211,   222,   468,   484,   485,   488,
     194,   339,   341,   344,   195,   235,   344,   111,   112,   161,
     212,   213,   214,   215,   219,    83,   197,   289,   290,   123,
     130,   122,   130,    83,   291,   192,   192,   192,   192,   209,
     261,   471,   192,   192,    70,   192,    70,   192,   334,    83,
      90,   154,   155,   156,   460,   461,   161,   195,   219,   219,
     209,   262,   471,   162,   192,   471,   471,    83,   189,   195,
     355,    27,   332,   336,   344,   345,   439,   443,   224,   195,
      90,   401,   460,    90,   460,   460,    32,   161,   178,   472,
     192,     9,   194,    38,   241,   162,   260,   471,   124,   188,
     242,   324,   194,   194,   194,   194,   194,   194,   194,   194,
      10,    11,    12,    30,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    69,
     194,    70,    70,   195,   157,   131,   168,   170,   183,   185,
     263,   322,   323,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    67,    68,   134,   135,
     429,    70,   195,   434,   192,   192,    70,   195,   197,   447,
     192,   241,   242,    14,   344,   194,   136,    48,   209,   424,
      90,   332,   345,   157,   439,   136,   199,     9,   409,   256,
     332,   345,   439,   472,   157,   192,   402,   429,   434,   193,
     344,    32,   226,     8,   356,     9,   194,   226,   227,   334,
     335,   344,   209,   275,   230,   194,   194,   194,   138,   139,
     140,   141,   488,   488,   178,   487,   487,   487,   192,   111,
     488,    14,   157,   138,   139,   140,   141,   158,   209,   211,
     194,   194,   194,   236,   115,   175,   194,   212,   214,   212,
     214,   219,   195,     9,   410,   194,   102,   161,   195,   439,
       9,   194,   130,   130,    14,     9,   194,   439,   464,   334,
     332,   345,   439,   442,   443,   193,   178,   253,   137,   439,
     453,   454,   344,   364,   365,    38,   171,   287,   288,   344,
     334,   376,   287,   194,    70,   429,   154,   461,    82,   344,
     439,    90,   154,   461,   219,   208,   194,   195,   248,   258,
     385,   387,    91,   192,   197,   357,   358,   360,   398,   445,
     447,   465,    14,   102,   466,   351,   352,   353,   285,   286,
     427,   428,   193,   193,   193,   193,   193,   196,   225,   226,
     243,   250,   257,   427,   344,   198,   200,   201,   209,   473,
     474,   488,   287,   468,   192,   471,   251,   241,   344,   344,
     344,   344,    32,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   399,   344,   344,
     449,   449,   344,   456,   457,   130,   195,   210,   211,   446,
     447,   261,   209,   262,   471,   471,   260,   242,    38,   336,
     339,   341,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   162,   195,   209,   430,   431,
     432,   433,   446,   449,   344,   287,   287,   449,   344,   453,
     241,   193,   344,   192,   423,     9,   409,   193,   193,    38,
     344,    38,   344,   402,   193,   193,   193,   446,   287,   195,
     209,   430,   431,   446,   193,   224,   279,   195,   341,   344,
     344,    94,    32,   226,   273,   194,    28,   102,    14,     9,
     193,    32,   195,   276,   488,    31,    91,   222,   481,   482,
     483,   192,     9,    50,    51,    56,    58,    70,   138,   139,
     140,   162,   182,   192,   220,   222,   371,   374,   380,   395,
     403,   404,   406,   209,   486,   224,   192,   234,   195,   194,
     195,   194,   102,   161,   111,   112,   161,   215,   216,   217,
     218,   219,   215,   209,   344,   290,   404,    83,     9,   193,
     193,   193,   193,   193,   193,   193,   194,    50,    51,   478,
     479,   480,   132,   266,   192,     9,   193,   193,   136,   199,
       9,   409,   439,   344,   193,     9,   410,     9,   409,   199,
     193,    83,    85,   209,   462,   209,    70,   196,   196,   205,
     207,    32,   133,   265,   177,    54,   162,   177,   389,   345,
     136,     9,   409,   193,   157,   488,   488,    14,   356,   285,
     224,   190,     9,   410,   488,   489,   429,   434,   429,   196,
       9,   409,   179,   193,    14,   348,   244,   132,   264,   192,
     471,   344,    32,   199,   199,   136,   196,     9,   409,   344,
     472,   192,   254,   249,   259,    14,   466,   252,   241,    72,
     439,   344,   472,   199,   196,   193,   193,   199,   196,   193,
      50,    51,    70,    78,    79,    80,    91,   138,   139,   140,
     153,   182,   209,   372,   375,   412,   414,   415,   419,   422,
     209,   439,   439,   136,   264,   429,   434,   193,   344,   280,
      75,    76,   281,   224,   333,   224,   335,   102,    38,   137,
     270,   439,   404,   209,    32,   226,   274,   194,   277,   194,
     277,     9,   409,    91,   136,   157,     9,   409,   193,   171,
     473,   474,   475,   473,   404,   404,   404,   404,   404,   408,
     411,   192,    70,    70,   157,   192,   404,   157,   195,    10,
      11,    12,    31,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    69,   157,   472,   196,   395,
     195,   238,   214,   214,   209,   215,   215,   219,     9,   410,
     196,   196,    14,   439,   194,   179,     9,   409,   209,   267,
     395,   195,   453,   137,   439,    14,    38,   344,   344,    38,
     171,   344,   199,   344,   196,   205,   488,   267,   195,   388,
      14,   193,   344,   357,   446,   194,   488,   190,   196,    32,
     476,   428,    38,    83,   171,   430,   431,   433,   430,   431,
     488,   404,   285,   192,   395,   265,   349,   245,   344,   344,
     344,   196,   192,   287,   266,    32,   265,   488,    14,   264,
     471,   399,   196,   192,    14,    78,    79,    80,   209,   413,
     413,   415,   417,   418,    52,   192,    70,    70,    90,   154,
     192,     9,   409,   193,   423,    38,   344,   265,   196,    75,
      76,   282,   333,   226,   196,   194,    95,   194,   270,   439,
     192,   136,   269,    14,   224,   277,   105,   106,   107,   277,
     196,   488,   179,   136,   488,   209,   481,     9,   193,   409,
     136,   199,     9,   409,   408,   366,   367,   404,   377,   404,
     405,   210,   357,   359,   361,   193,   130,   210,   404,   458,
     459,   404,   404,   404,    32,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   486,
      83,   239,   196,   196,   218,   194,   404,   480,   102,   103,
     477,     9,   295,   193,   192,   336,   341,   344,   439,   136,
     439,   344,   199,   196,   466,   295,   163,   176,   195,   384,
     391,   163,   195,   390,   136,   194,   476,   488,   356,   489,
      83,   171,    14,    83,   472,   193,   285,   195,   285,   192,
     136,   192,   287,   193,   195,   488,   195,   194,   488,   265,
     246,   402,   287,   136,   199,     9,   409,   414,   417,   368,
     369,   415,   378,   415,   416,   154,   357,   420,   421,   415,
     439,   195,   333,    32,    77,   226,   194,   335,   269,   453,
     270,   193,   404,   101,   105,   194,   344,    32,   194,   278,
     196,   179,   488,   136,   171,    32,   193,   404,   404,   193,
     199,     9,   409,   136,   199,     9,   409,   136,     9,   409,
     193,   136,   196,     9,   409,   404,    32,   193,   224,   194,
     194,   209,   488,   488,   395,     4,   112,   117,   123,   125,
     164,   165,   167,   196,   296,   321,   322,   323,   328,   329,
     330,   331,   427,   453,    38,   344,   196,   195,   196,    54,
     344,   344,   344,   356,    38,    83,   171,    14,    83,   344,
     192,   476,   193,   295,   193,   285,   344,   287,   193,   295,
     466,   295,   194,   195,   192,   193,   415,   415,   193,   199,
       9,   409,   136,   199,     9,   409,   136,   193,     9,   409,
     295,    32,   224,   194,   193,   193,   193,   231,   194,   194,
     278,   224,   488,   488,   136,   404,   404,   404,   404,   357,
     404,   404,   404,   195,   196,   477,   132,   133,   183,   210,
     469,   488,   268,   395,   112,   331,    31,   125,   138,   141,
     162,   168,   305,   306,   307,   308,   395,   166,   313,   314,
     128,   192,   209,   315,   316,   297,   242,   488,     9,   194,
       9,   194,   194,   466,   322,   193,   439,   292,   162,   386,
     196,   196,    83,   171,    14,    83,   344,   287,   117,   346,
     476,   196,   476,   193,   193,   196,   195,   196,   295,   285,
     136,   415,   415,   415,   415,   357,   196,   224,   229,   232,
      32,   226,   272,   224,   193,   404,   136,   136,   136,   224,
     395,   395,   471,    14,   210,     9,   194,   195,   469,   466,
     308,   178,   195,     9,   194,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    29,    57,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   137,   138,   139,   140,
     142,   143,   144,   145,   146,   158,   159,   160,   170,   172,
     173,   175,   182,   183,   185,   187,   188,   209,   392,   393,
       9,   194,   162,   166,   209,   316,   317,   318,   194,    83,
     327,   241,   298,   469,   469,    14,   242,   196,   293,   294,
     469,    14,    83,   344,   193,   192,   476,   194,   195,   319,
     346,   476,   292,   196,   193,   415,   136,   136,    32,   226,
     271,   272,   224,   404,   404,   404,   196,   194,   194,   404,
     395,   301,   488,   309,   310,   403,   306,    14,    32,    51,
     311,   314,     9,    36,   193,    31,    50,    53,    14,     9,
     194,   211,   470,   327,    14,   488,   241,   194,    14,   344,
      38,    83,   383,   195,   224,   476,   319,   196,   476,   415,
     415,   224,    99,   237,   196,   209,   222,   302,   303,   304,
       9,   409,     9,   409,   196,   404,   393,   393,    59,   312,
     317,   317,    31,    50,    53,   404,    83,   178,   192,   194,
     404,   471,   404,    83,     9,   410,   224,   196,   195,   319,
      97,   194,   115,   233,   157,   102,   488,   179,   403,   169,
      14,   478,   299,   192,    38,    83,   193,   196,   224,   194,
     192,   175,   240,   209,   322,   323,   179,   404,   179,   283,
     284,   428,   300,    83,   196,   395,   238,   172,   209,   194,
     193,     9,   410,   119,   120,   121,   325,   326,   283,    83,
     268,   194,   476,   428,   489,   193,   193,   194,   194,   195,
     320,   325,    38,    83,   171,   476,   195,   224,   489,    83,
     171,    14,    83,   320,   224,   196,    38,    83,   171,    14,
      83,   344,   196,    83,   171,    14,    83,   344,    14,    83,
     344,   344
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
#line 732 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 735 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { (yyval).reset();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { (yyval).reset();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
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

  case 201:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
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

  case 203:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = T_FINAL;;}
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
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval).reset();;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { (yyval).reset();;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { (yyval).reset();;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { (yyval).reset();;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
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
#line 1687 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval).reset();;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval).reset();;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval).reset();;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval).reset();;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
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
#line 2040 "hphp.y"
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
#line 2050 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
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

  case 531:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval).reset();;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval).reset();;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval).reset();;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval).reset();;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
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

  case 594:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
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

  case 595:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval).reset();;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval).reset();;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("dict");
                                         _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("vec");
                                         _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval).reset();;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval).reset();;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval).reset();;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval).reset();;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval).reset();;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
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

  case 862:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
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

  case 863:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
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

  case 864:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
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

  case 865:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
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

  case 872:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
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

  case 881:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval).reset();;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { (yyval)++;;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
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

  case 917:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
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

  case 923:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval).reset();;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
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

  case 959:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3274 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3287 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3314 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    {;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    {;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3323 "hphp.y"
    {;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3334 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3350 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3356 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3373 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3390 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3393 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3397 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3400 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3403 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3406 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3412 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3415 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3419 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3421 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3427 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3433 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3442 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14502 "hphp.5.tab.cpp"
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
#line 3445 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

