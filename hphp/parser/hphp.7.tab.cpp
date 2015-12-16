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
#define yyparse         Compiler7parse
#define yylex           Compiler7lex
#define yyerror         Compiler7error
#define yylval          Compiler7lval
#define yychar          Compiler7char
#define yydebug         Compiler7debug
#define yynerrs         Compiler7nerrs
#define yylloc          Compiler7lloc

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
#line 651 "hphp.7.tab.cpp"

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
#line 874 "hphp.7.tab.cpp"

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
#define YYLAST   16727

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  198
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  268
/* YYNRULES -- Number of rules.  */
#define YYNRULES  991
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1820

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
    1242,  1247,  1254,  1256,  1258,  1263,  1265,  1267,  1271,  1274,
    1277,  1278,  1281,  1282,  1284,  1288,  1290,  1292,  1294,  1296,
    1300,  1305,  1310,  1315,  1317,  1319,  1322,  1325,  1328,  1332,
    1336,  1338,  1340,  1342,  1344,  1348,  1350,  1354,  1356,  1358,
    1360,  1361,  1363,  1366,  1368,  1370,  1372,  1374,  1376,  1378,
    1380,  1382,  1383,  1385,  1387,  1389,  1393,  1399,  1401,  1405,
    1411,  1416,  1420,  1424,  1428,  1433,  1437,  1441,  1445,  1448,
    1451,  1453,  1455,  1459,  1463,  1465,  1467,  1468,  1470,  1473,
    1478,  1482,  1486,  1493,  1496,  1500,  1503,  1507,  1514,  1516,
    1518,  1520,  1522,  1524,  1531,  1535,  1540,  1547,  1551,  1555,
    1559,  1563,  1567,  1571,  1575,  1579,  1583,  1587,  1591,  1595,
    1598,  1601,  1604,  1607,  1611,  1615,  1619,  1623,  1627,  1631,
    1635,  1639,  1643,  1647,  1651,  1655,  1659,  1663,  1667,  1671,
    1675,  1678,  1681,  1684,  1687,  1691,  1695,  1699,  1703,  1707,
    1711,  1715,  1719,  1723,  1727,  1731,  1737,  1742,  1746,  1748,
    1751,  1754,  1757,  1760,  1763,  1766,  1769,  1772,  1775,  1777,
    1779,  1781,  1785,  1788,  1790,  1796,  1797,  1798,  1811,  1812,
    1826,  1827,  1832,  1833,  1841,  1842,  1848,  1849,  1853,  1854,
    1861,  1864,  1867,  1872,  1874,  1876,  1882,  1886,  1892,  1896,
    1899,  1900,  1903,  1904,  1909,  1914,  1918,  1923,  1928,  1933,
    1938,  1940,  1942,  1944,  1946,  1950,  1954,  1959,  1961,  1964,
    1969,  1972,  1979,  1980,  1982,  1987,  1988,  1991,  1992,  1994,
    1996,  2000,  2002,  2006,  2008,  2010,  2014,  2018,  2020,  2022,
    2024,  2026,  2028,  2030,  2032,  2034,  2036,  2038,  2040,  2042,
    2044,  2046,  2048,  2050,  2052,  2054,  2056,  2058,  2060,  2062,
    2064,  2066,  2068,  2070,  2072,  2074,  2076,  2078,  2080,  2082,
    2084,  2086,  2088,  2090,  2092,  2094,  2096,  2098,  2100,  2102,
    2104,  2106,  2108,  2110,  2112,  2114,  2116,  2118,  2120,  2122,
    2124,  2126,  2128,  2130,  2132,  2134,  2136,  2138,  2140,  2142,
    2144,  2146,  2148,  2150,  2152,  2154,  2156,  2158,  2160,  2162,
    2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,  2180,  2185,
    2187,  2189,  2191,  2193,  2195,  2197,  2201,  2203,  2207,  2209,
    2211,  2215,  2217,  2219,  2221,  2224,  2226,  2227,  2228,  2230,
    2232,  2236,  2237,  2239,  2241,  2243,  2245,  2247,  2249,  2251,
    2253,  2255,  2257,  2259,  2261,  2263,  2267,  2270,  2272,  2274,
    2279,  2283,  2288,  2290,  2292,  2296,  2300,  2304,  2308,  2312,
    2316,  2320,  2324,  2328,  2332,  2336,  2340,  2344,  2348,  2352,
    2356,  2360,  2364,  2367,  2370,  2373,  2376,  2380,  2384,  2388,
    2392,  2396,  2400,  2404,  2408,  2412,  2418,  2423,  2427,  2431,
    2435,  2437,  2439,  2441,  2443,  2447,  2451,  2455,  2458,  2459,
    2461,  2462,  2464,  2465,  2471,  2475,  2479,  2481,  2483,  2485,
    2487,  2491,  2494,  2496,  2498,  2500,  2502,  2504,  2508,  2510,
    2512,  2514,  2517,  2520,  2525,  2529,  2534,  2537,  2538,  2544,
    2548,  2552,  2554,  2558,  2560,  2563,  2564,  2570,  2574,  2577,
    2578,  2582,  2583,  2588,  2591,  2592,  2596,  2600,  2602,  2603,
    2605,  2607,  2609,  2611,  2615,  2617,  2619,  2621,  2625,  2627,
    2629,  2633,  2637,  2640,  2645,  2648,  2653,  2659,  2665,  2671,
    2677,  2679,  2681,  2683,  2685,  2687,  2689,  2693,  2697,  2702,
    2707,  2711,  2713,  2715,  2717,  2719,  2723,  2725,  2730,  2734,
    2738,  2740,  2742,  2744,  2746,  2748,  2752,  2756,  2761,  2766,
    2770,  2772,  2774,  2782,  2792,  2800,  2807,  2816,  2818,  2823,
    2828,  2830,  2832,  2837,  2840,  2842,  2843,  2845,  2847,  2849,
    2853,  2857,  2861,  2862,  2864,  2866,  2870,  2874,  2877,  2881,
    2888,  2889,  2891,  2896,  2899,  2900,  2906,  2910,  2914,  2916,
    2923,  2928,  2933,  2936,  2939,  2940,  2946,  2950,  2954,  2956,
    2959,  2960,  2966,  2970,  2974,  2976,  2979,  2982,  2984,  2987,
    2989,  2994,  2998,  3002,  3009,  3013,  3015,  3017,  3019,  3024,
    3029,  3034,  3039,  3044,  3049,  3052,  3055,  3060,  3063,  3066,
    3068,  3072,  3076,  3080,  3081,  3084,  3090,  3097,  3104,  3112,
    3114,  3117,  3119,  3122,  3124,  3129,  3131,  3136,  3140,  3141,
    3143,  3147,  3150,  3154,  3156,  3158,  3159,  3160,  3163,  3166,
    3169,  3174,  3177,  3183,  3187,  3189,  3191,  3192,  3196,  3201,
    3207,  3211,  3213,  3216,  3217,  3222,  3224,  3228,  3231,  3234,
    3237,  3239,  3241,  3243,  3245,  3249,  3254,  3261,  3263,  3272,
    3279,  3281
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     199,     0,    -1,    -1,   200,   201,    -1,   201,   202,    -1,
      -1,   222,    -1,   239,    -1,   246,    -1,   243,    -1,   253,
      -1,   445,    -1,   127,   188,   189,   190,    -1,   154,   215,
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
     157,   215,    -1,   216,   450,    -1,   216,   450,    -1,   219,
       9,   446,    14,   385,    -1,   110,   446,    14,   385,    -1,
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
     146,   330,   190,    -1,   124,   188,   442,   189,   190,    -1,
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
      37,    -1,    -1,   109,    -1,    -1,   238,   237,   449,   240,
     188,   281,   189,   454,   315,    -1,    -1,   319,   238,   237,
     449,   241,   188,   281,   189,   454,   315,    -1,    -1,   406,
     318,   238,   237,   449,   242,   188,   281,   189,   454,   315,
      -1,    -1,   164,   205,   244,    31,   464,   444,   191,   288,
     192,    -1,    -1,   406,   164,   205,   245,    31,   464,   444,
     191,   288,   192,    -1,    -1,   259,   256,   247,   260,   261,
     191,   291,   192,    -1,    -1,   406,   259,   256,   248,   260,
     261,   191,   291,   192,    -1,    -1,   129,   257,   249,   262,
     191,   291,   192,    -1,    -1,   406,   129,   257,   250,   262,
     191,   291,   192,    -1,    -1,   128,   252,   383,   260,   261,
     191,   291,   192,    -1,    -1,   166,   258,   254,   261,   191,
     291,   192,    -1,    -1,   406,   166,   258,   255,   261,   191,
     291,   192,    -1,   449,    -1,   158,    -1,   449,    -1,   449,
      -1,   128,    -1,   121,   128,    -1,   121,   120,   128,    -1,
     120,   121,   128,    -1,   120,   128,    -1,   130,   376,    -1,
      -1,   131,   263,    -1,    -1,   130,   263,    -1,    -1,   376,
      -1,   263,     9,   376,    -1,   376,    -1,   264,     9,   376,
      -1,   134,   266,    -1,    -1,   418,    -1,    37,   418,    -1,
     135,   188,   431,   189,    -1,   222,    -1,    31,   220,    95,
     190,    -1,   222,    -1,    31,   220,    97,   190,    -1,   222,
      -1,    31,   220,    93,   190,    -1,   222,    -1,    31,   220,
      99,   190,    -1,   205,    14,   385,    -1,   271,     9,   205,
      14,   385,    -1,   191,   273,   192,    -1,   191,   190,   273,
     192,    -1,    31,   273,   103,   190,    -1,    31,   190,   273,
     103,   190,    -1,   273,   104,   340,   274,   220,    -1,   273,
     105,   274,   220,    -1,    -1,    31,    -1,   190,    -1,   275,
      74,   329,   222,    -1,    -1,   276,    74,   329,    31,   220,
      -1,    -1,    75,   222,    -1,    -1,    75,    31,   220,    -1,
      -1,   280,     9,   407,   321,   465,   167,    82,    -1,   280,
       9,   407,   321,   465,    37,   167,    82,    -1,   280,     9,
     407,   321,   465,   167,    -1,   280,   390,    -1,   407,   321,
     465,   167,    82,    -1,   407,   321,   465,    37,   167,    82,
      -1,   407,   321,   465,   167,    -1,    -1,   407,   321,   465,
      82,    -1,   407,   321,   465,    37,    82,    -1,   407,   321,
     465,    37,    82,    14,   340,    -1,   407,   321,   465,    82,
      14,   340,    -1,   280,     9,   407,   321,   465,    82,    -1,
     280,     9,   407,   321,   465,    37,    82,    -1,   280,     9,
     407,   321,   465,    37,    82,    14,   340,    -1,   280,     9,
     407,   321,   465,    82,    14,   340,    -1,   282,     9,   407,
     465,   167,    82,    -1,   282,     9,   407,   465,    37,   167,
      82,    -1,   282,     9,   407,   465,   167,    -1,   282,   390,
      -1,   407,   465,   167,    82,    -1,   407,   465,    37,   167,
      82,    -1,   407,   465,   167,    -1,    -1,   407,   465,    82,
      -1,   407,   465,    37,    82,    -1,   407,   465,    37,    82,
      14,   340,    -1,   407,   465,    82,    14,   340,    -1,   282,
       9,   407,   465,    82,    -1,   282,     9,   407,   465,    37,
      82,    -1,   282,     9,   407,   465,    37,    82,    14,   340,
      -1,   282,     9,   407,   465,    82,    14,   340,    -1,   284,
     390,    -1,    -1,   340,    -1,    37,   418,    -1,   167,   340,
      -1,   284,     9,   340,    -1,   284,     9,   167,   340,    -1,
     284,     9,    37,   418,    -1,   285,     9,   286,    -1,   286,
      -1,    82,    -1,   193,   418,    -1,   193,   191,   340,   192,
      -1,   287,     9,    82,    -1,   287,     9,    82,    14,   385,
      -1,    82,    -1,    82,    14,   385,    -1,   288,   289,    -1,
      -1,   290,   190,    -1,   447,    14,   385,    -1,   291,   292,
      -1,    -1,    -1,   317,   293,   323,   190,    -1,    -1,   319,
     464,   294,   323,   190,    -1,   324,   190,    -1,   325,   190,
      -1,   326,   190,    -1,    -1,   318,   238,   237,   448,   188,
     295,   279,   189,   454,   316,    -1,    -1,   406,   318,   238,
     237,   449,   188,   296,   279,   189,   454,   316,    -1,   160,
     301,   190,    -1,   161,   309,   190,    -1,   163,   311,   190,
      -1,     4,   130,   376,   190,    -1,     4,   131,   376,   190,
      -1,   115,   264,   190,    -1,   115,   264,   191,   297,   192,
      -1,   297,   298,    -1,   297,   299,    -1,    -1,   218,   153,
     205,   168,   264,   190,    -1,   300,   100,   318,   205,   190,
      -1,   300,   100,   319,   190,    -1,   218,   153,   205,    -1,
     205,    -1,   302,    -1,   301,     9,   302,    -1,   303,   373,
     307,   308,    -1,   158,    -1,    30,   304,    -1,   304,    -1,
     136,    -1,   136,   174,   464,   175,    -1,   136,   174,   464,
       9,   464,   175,    -1,   376,    -1,   123,    -1,   164,   191,
     306,   192,    -1,   137,    -1,   384,    -1,   305,     9,   384,
      -1,   305,   389,    -1,    14,   385,    -1,    -1,    58,   165,
      -1,    -1,   310,    -1,   309,     9,   310,    -1,   162,    -1,
     312,    -1,   205,    -1,   126,    -1,   188,   313,   189,    -1,
     188,   313,   189,    52,    -1,   188,   313,   189,    30,    -1,
     188,   313,   189,    49,    -1,   312,    -1,   314,    -1,   314,
      52,    -1,   314,    30,    -1,   314,    49,    -1,   313,     9,
     313,    -1,   313,    35,   313,    -1,   205,    -1,   158,    -1,
     162,    -1,   190,    -1,   191,   220,   192,    -1,   190,    -1,
     191,   220,   192,    -1,   319,    -1,   123,    -1,   319,    -1,
      -1,   320,    -1,   319,   320,    -1,   117,    -1,   118,    -1,
     119,    -1,   122,    -1,   121,    -1,   120,    -1,   184,    -1,
     322,    -1,    -1,   117,    -1,   118,    -1,   119,    -1,   323,
       9,    82,    -1,   323,     9,    82,    14,   385,    -1,    82,
      -1,    82,    14,   385,    -1,   324,     9,   447,    14,   385,
      -1,   110,   447,    14,   385,    -1,   325,     9,   447,    -1,
     121,   110,   447,    -1,   121,   327,   444,    -1,   327,   444,
      14,   464,    -1,   110,   179,   449,    -1,   188,   328,   189,
      -1,    71,   380,   383,    -1,    71,   251,    -1,    70,   340,
      -1,   365,    -1,   360,    -1,   188,   340,   189,    -1,   330,
       9,   340,    -1,   340,    -1,   330,    -1,    -1,    27,    -1,
      27,   340,    -1,    27,   340,   134,   340,    -1,   188,   332,
     189,    -1,   418,    14,   332,    -1,   135,   188,   431,   189,
      14,   332,    -1,    29,   340,    -1,   418,    14,   335,    -1,
      28,   340,    -1,   418,    14,   337,    -1,   135,   188,   431,
     189,    14,   337,    -1,   341,    -1,   418,    -1,   328,    -1,
     422,    -1,   421,    -1,   135,   188,   431,   189,    14,   340,
      -1,   418,    14,   340,    -1,   418,    14,    37,   418,    -1,
     418,    14,    37,    71,   380,   383,    -1,   418,    26,   340,
      -1,   418,    25,   340,    -1,   418,    24,   340,    -1,   418,
      23,   340,    -1,   418,    22,   340,    -1,   418,    21,   340,
      -1,   418,    20,   340,    -1,   418,    19,   340,    -1,   418,
      18,   340,    -1,   418,    17,   340,    -1,   418,    16,   340,
      -1,   418,    15,   340,    -1,   418,    67,    -1,    67,   418,
      -1,   418,    66,    -1,    66,   418,    -1,   340,    33,   340,
      -1,   340,    34,   340,    -1,   340,    10,   340,    -1,   340,
      12,   340,    -1,   340,    11,   340,    -1,   340,    35,   340,
      -1,   340,    37,   340,    -1,   340,    36,   340,    -1,   340,
      51,   340,    -1,   340,    49,   340,    -1,   340,    50,   340,
      -1,   340,    52,   340,    -1,   340,    53,   340,    -1,   340,
      68,   340,    -1,   340,    54,   340,    -1,   340,    48,   340,
      -1,   340,    47,   340,    -1,    49,   340,    -1,    50,   340,
      -1,    55,   340,    -1,    57,   340,    -1,   340,    39,   340,
      -1,   340,    38,   340,    -1,   340,    41,   340,    -1,   340,
      40,   340,    -1,   340,    42,   340,    -1,   340,    46,   340,
      -1,   340,    43,   340,    -1,   340,    45,   340,    -1,   340,
      44,   340,    -1,   340,    56,   380,    -1,   188,   341,   189,
      -1,   340,    30,   340,    31,   340,    -1,   340,    30,    31,
     340,    -1,   340,    32,   340,    -1,   441,    -1,    65,   340,
      -1,    64,   340,    -1,    63,   340,    -1,    62,   340,    -1,
      61,   340,    -1,    60,   340,    -1,    59,   340,    -1,    72,
     381,    -1,    58,   340,    -1,   387,    -1,   359,    -1,   358,
      -1,   194,   382,   194,    -1,    13,   340,    -1,   362,    -1,
     115,   188,   364,   390,   189,    -1,    -1,    -1,   238,   237,
     188,   344,   281,   189,   454,   342,   454,   191,   220,   192,
      -1,    -1,   319,   238,   237,   188,   345,   281,   189,   454,
     342,   454,   191,   220,   192,    -1,    -1,   184,    82,   347,
     352,    -1,    -1,   184,   185,   348,   281,   186,   454,   352,
      -1,    -1,   184,   191,   349,   220,   192,    -1,    -1,    82,
     350,   352,    -1,    -1,   185,   351,   281,   186,   454,   352,
      -1,     8,   340,    -1,     8,   337,    -1,     8,   191,   220,
     192,    -1,    89,    -1,   443,    -1,   354,     9,   353,   134,
     340,    -1,   353,   134,   340,    -1,   355,     9,   353,   134,
     385,    -1,   353,   134,   385,    -1,   354,   389,    -1,    -1,
     355,   389,    -1,    -1,   178,   188,   356,   189,    -1,   136,
     188,   432,   189,    -1,    69,   432,   195,    -1,   376,   191,
     434,   192,    -1,   376,   191,   436,   192,    -1,   362,    69,
     428,   195,    -1,   363,    69,   428,   195,    -1,   359,    -1,
     443,    -1,   421,    -1,    89,    -1,   188,   341,   189,    -1,
     364,     9,    82,    -1,   364,     9,    37,    82,    -1,    82,
      -1,    37,    82,    -1,   172,   158,   366,   173,    -1,   368,
      53,    -1,   368,   173,   369,   172,    53,   367,    -1,    -1,
     158,    -1,   368,   370,    14,   371,    -1,    -1,   369,   372,
      -1,    -1,   158,    -1,   159,    -1,   191,   340,   192,    -1,
     159,    -1,   191,   340,   192,    -1,   365,    -1,   374,    -1,
     373,    31,   374,    -1,   373,    50,   374,    -1,   205,    -1,
      72,    -1,   109,    -1,   110,    -1,   111,    -1,    27,    -1,
      29,    -1,    28,    -1,   112,    -1,   113,    -1,   171,    -1,
     114,    -1,    73,    -1,    74,    -1,    76,    -1,    75,    -1,
      92,    -1,    93,    -1,    91,    -1,    94,    -1,    95,    -1,
      96,    -1,    97,    -1,    98,    -1,    99,    -1,    56,    -1,
     100,    -1,   102,    -1,   103,    -1,   104,    -1,   105,    -1,
     106,    -1,   108,    -1,   107,    -1,    90,    -1,    13,    -1,
     128,    -1,   129,    -1,   130,    -1,   131,    -1,    71,    -1,
      70,    -1,   123,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   154,    -1,   115,    -1,   116,    -1,
     125,    -1,   126,    -1,   127,    -1,   122,    -1,   121,    -1,
     120,    -1,   119,    -1,   118,    -1,   117,    -1,   184,    -1,
     124,    -1,   135,    -1,   136,    -1,    10,    -1,    12,    -1,
      11,    -1,   138,    -1,   140,    -1,   139,    -1,   141,    -1,
     142,    -1,   156,    -1,   155,    -1,   183,    -1,   166,    -1,
     169,    -1,   168,    -1,   179,    -1,   181,    -1,   178,    -1,
     217,   188,   283,   189,    -1,   218,    -1,   158,    -1,   376,
      -1,   384,    -1,   122,    -1,   426,    -1,   188,   341,   189,
      -1,   377,    -1,   378,   153,   427,    -1,   377,    -1,   424,
      -1,   379,   153,   427,    -1,   376,    -1,   122,    -1,   429,
      -1,   188,   189,    -1,   329,    -1,    -1,    -1,    88,    -1,
     438,    -1,   188,   283,   189,    -1,    -1,    77,    -1,    78,
      -1,    79,    -1,    89,    -1,   141,    -1,   142,    -1,   156,
      -1,   138,    -1,   169,    -1,   139,    -1,   140,    -1,   155,
      -1,   183,    -1,   149,    88,   150,    -1,   149,   150,    -1,
     384,    -1,   216,    -1,   136,   188,   388,   189,    -1,    69,
     388,   195,    -1,   178,   188,   357,   189,    -1,   386,    -1,
     361,    -1,   188,   385,   189,    -1,   385,    33,   385,    -1,
     385,    34,   385,    -1,   385,    10,   385,    -1,   385,    12,
     385,    -1,   385,    11,   385,    -1,   385,    35,   385,    -1,
     385,    37,   385,    -1,   385,    36,   385,    -1,   385,    51,
     385,    -1,   385,    49,   385,    -1,   385,    50,   385,    -1,
     385,    52,   385,    -1,   385,    53,   385,    -1,   385,    54,
     385,    -1,   385,    48,   385,    -1,   385,    47,   385,    -1,
     385,    68,   385,    -1,    55,   385,    -1,    57,   385,    -1,
      49,   385,    -1,    50,   385,    -1,   385,    39,   385,    -1,
     385,    38,   385,    -1,   385,    41,   385,    -1,   385,    40,
     385,    -1,   385,    42,   385,    -1,   385,    46,   385,    -1,
     385,    43,   385,    -1,   385,    45,   385,    -1,   385,    44,
     385,    -1,   385,    30,   385,    31,   385,    -1,   385,    30,
      31,   385,    -1,   218,   153,   206,    -1,   158,   153,   206,
      -1,   218,   153,   128,    -1,   216,    -1,    81,    -1,   443,
      -1,   384,    -1,   196,   438,   196,    -1,   197,   438,   197,
      -1,   149,   438,   150,    -1,   391,   389,    -1,    -1,     9,
      -1,    -1,     9,    -1,    -1,   391,     9,   385,   134,   385,
      -1,   391,     9,   385,    -1,   385,   134,   385,    -1,   385,
      -1,    77,    -1,    78,    -1,    79,    -1,   149,    88,   150,
      -1,   149,   150,    -1,    77,    -1,    78,    -1,    79,    -1,
     205,    -1,    89,    -1,    89,    51,   394,    -1,   392,    -1,
     394,    -1,   205,    -1,    49,   393,    -1,    50,   393,    -1,
     136,   188,   396,   189,    -1,    69,   396,   195,    -1,   178,
     188,   399,   189,    -1,   397,   389,    -1,    -1,   397,     9,
     395,   134,   395,    -1,   397,     9,   395,    -1,   395,   134,
     395,    -1,   395,    -1,   398,     9,   395,    -1,   395,    -1,
     400,   389,    -1,    -1,   400,     9,   353,   134,   395,    -1,
     353,   134,   395,    -1,   398,   389,    -1,    -1,   188,   401,
     189,    -1,    -1,   403,     9,   205,   402,    -1,   205,   402,
      -1,    -1,   405,   403,   389,    -1,    48,   404,    47,    -1,
     406,    -1,    -1,   132,    -1,   133,    -1,   205,    -1,   158,
      -1,   191,   340,   192,    -1,   409,    -1,   427,    -1,   205,
      -1,   191,   340,   192,    -1,   411,    -1,   427,    -1,    69,
     428,   195,    -1,   191,   340,   192,    -1,   419,   413,    -1,
     188,   328,   189,   413,    -1,   430,   413,    -1,   188,   328,
     189,   413,    -1,   188,   328,   189,   408,   410,    -1,   188,
     341,   189,   408,   410,    -1,   188,   328,   189,   408,   409,
      -1,   188,   341,   189,   408,   409,    -1,   425,    -1,   375,
      -1,   423,    -1,   424,    -1,   414,    -1,   416,    -1,   418,
     408,   410,    -1,   379,   153,   427,    -1,   420,   188,   283,
     189,    -1,   421,   188,   283,   189,    -1,   188,   418,   189,
      -1,   375,    -1,   423,    -1,   424,    -1,   414,    -1,   418,
     408,   410,    -1,   417,    -1,   420,   188,   283,   189,    -1,
     188,   418,   189,    -1,   379,   153,   427,    -1,   425,    -1,
     414,    -1,   375,    -1,   359,    -1,   384,    -1,   188,   418,
     189,    -1,   188,   341,   189,    -1,   421,   188,   283,   189,
      -1,   420,   188,   283,   189,    -1,   188,   422,   189,    -1,
     343,    -1,   346,    -1,   418,   408,   412,   450,   188,   283,
     189,    -1,   188,   328,   189,   408,   412,   450,   188,   283,
     189,    -1,   379,   153,   207,   450,   188,   283,   189,    -1,
     379,   153,   427,   188,   283,   189,    -1,   379,   153,   191,
     340,   192,   188,   283,   189,    -1,   426,    -1,   426,    69,
     428,   195,    -1,   426,   191,   340,   192,    -1,   427,    -1,
      82,    -1,   193,   191,   340,   192,    -1,   193,   427,    -1,
     340,    -1,    -1,   425,    -1,   415,    -1,   416,    -1,   429,
     408,   410,    -1,   378,   153,   425,    -1,   188,   418,   189,
      -1,    -1,   415,    -1,   417,    -1,   429,   408,   409,    -1,
     188,   418,   189,    -1,   431,     9,    -1,   431,     9,   418,
      -1,   431,     9,   135,   188,   431,   189,    -1,    -1,   418,
      -1,   135,   188,   431,   189,    -1,   433,   389,    -1,    -1,
     433,     9,   340,   134,   340,    -1,   433,     9,   340,    -1,
     340,   134,   340,    -1,   340,    -1,   433,     9,   340,   134,
      37,   418,    -1,   433,     9,    37,   418,    -1,   340,   134,
      37,   418,    -1,    37,   418,    -1,   435,   389,    -1,    -1,
     435,     9,   340,   134,   340,    -1,   435,     9,   340,    -1,
     340,   134,   340,    -1,   340,    -1,   437,   389,    -1,    -1,
     437,     9,   385,   134,   385,    -1,   437,     9,   385,    -1,
     385,   134,   385,    -1,   385,    -1,   438,   439,    -1,   438,
      88,    -1,   439,    -1,    88,   439,    -1,    82,    -1,    82,
      69,   440,   195,    -1,    82,   408,   205,    -1,   151,   340,
     192,    -1,   151,    81,    69,   340,   195,   192,    -1,   152,
     418,   192,    -1,   205,    -1,    83,    -1,    82,    -1,   125,
     188,   330,   189,    -1,   126,   188,   418,   189,    -1,   126,
     188,   341,   189,    -1,   126,   188,   422,   189,    -1,   126,
     188,   421,   189,    -1,   126,   188,   328,   189,    -1,     7,
     340,    -1,     6,   340,    -1,     5,   188,   340,   189,    -1,
       4,   340,    -1,     3,   340,    -1,   418,    -1,   442,     9,
     418,    -1,   379,   153,   206,    -1,   379,   153,   128,    -1,
      -1,   100,   464,    -1,   179,   449,    14,   464,   190,    -1,
     406,   179,   449,    14,   464,   190,    -1,   181,   449,   444,
      14,   464,   190,    -1,   406,   181,   449,   444,    14,   464,
     190,    -1,   207,    -1,   464,   207,    -1,   206,    -1,   464,
     206,    -1,   207,    -1,   207,   174,   456,   175,    -1,   205,
      -1,   205,   174,   456,   175,    -1,   174,   452,   175,    -1,
      -1,   464,    -1,   451,     9,   464,    -1,   451,   389,    -1,
     451,     9,   167,    -1,   452,    -1,   167,    -1,    -1,    -1,
      31,   464,    -1,   100,   464,    -1,   101,   464,    -1,   456,
       9,   457,   205,    -1,   457,   205,    -1,   456,     9,   457,
     205,   455,    -1,   457,   205,   455,    -1,    49,    -1,    50,
      -1,    -1,    89,   134,   464,    -1,    30,    89,   134,   464,
      -1,   218,   153,   205,   134,   464,    -1,   459,     9,   458,
      -1,   458,    -1,   459,   389,    -1,    -1,   178,   188,   460,
     189,    -1,   218,    -1,   205,   153,   463,    -1,   205,   450,
      -1,    30,   464,    -1,    58,   464,    -1,   218,    -1,   136,
      -1,   137,    -1,   461,    -1,   462,   153,   463,    -1,   136,
     174,   464,   175,    -1,   136,   174,   464,     9,   464,   175,
      -1,   158,    -1,   188,   109,   188,   453,   189,    31,   464,
     189,    -1,   188,   464,     9,   451,   389,   189,    -1,   464,
      -1,    -1
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
    2664,  2668,  2669,  2670,  2674,  2679,  2684,  2685,  2689,  2694,
    2699,  2700,  2704,  2705,  2710,  2712,  2717,  2728,  2742,  2754,
    2769,  2770,  2771,  2772,  2773,  2774,  2775,  2785,  2794,  2796,
    2798,  2802,  2803,  2804,  2805,  2806,  2822,  2823,  2825,  2827,
    2834,  2835,  2836,  2837,  2838,  2839,  2840,  2841,  2843,  2848,
    2852,  2853,  2857,  2860,  2867,  2871,  2880,  2887,  2895,  2897,
    2898,  2902,  2903,  2905,  2910,  2911,  2922,  2923,  2924,  2925,
    2936,  2939,  2942,  2943,  2944,  2945,  2956,  2960,  2961,  2962,
    2964,  2965,  2966,  2970,  2972,  2975,  2977,  2978,  2979,  2980,
    2983,  2985,  2986,  2990,  2992,  2995,  2997,  2998,  2999,  3003,
    3005,  3008,  3011,  3013,  3015,  3019,  3020,  3022,  3023,  3029,
    3030,  3032,  3042,  3044,  3046,  3049,  3050,  3051,  3055,  3056,
    3057,  3058,  3059,  3060,  3061,  3062,  3063,  3064,  3065,  3069,
    3070,  3074,  3076,  3084,  3086,  3090,  3094,  3099,  3103,  3111,
    3112,  3116,  3117,  3123,  3124,  3133,  3134,  3142,  3145,  3149,
    3152,  3157,  3162,  3164,  3165,  3166,  3170,  3171,  3175,  3176,
    3179,  3182,  3184,  3188,  3194,  3195,  3196,  3200,  3204,  3214,
    3222,  3224,  3228,  3230,  3235,  3241,  3244,  3249,  3257,  3260,
    3263,  3264,  3267,  3270,  3271,  3276,  3279,  3283,  3287,  3293,
    3303,  3304
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
  "variable_no_calls", "dimmable_variable_no_calls", "assignment_list",
  "array_pair_list", "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_constname_with_type", "hh_name_with_typevar",
  "hh_name_no_semireserved_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_func_type_list",
  "opt_return_type", "hh_constraint", "hh_typevar_list",
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
     418,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   421,
     422,   422,   423,   423,   424,   424,   424,   425,   426,   426,
     426,   427,   427,   427,   428,   428,   429,   429,   429,   429,
     429,   429,   430,   430,   430,   430,   430,   431,   431,   431,
     431,   431,   431,   432,   432,   433,   433,   433,   433,   433,
     433,   433,   433,   434,   434,   435,   435,   435,   435,   436,
     436,   437,   437,   437,   437,   438,   438,   438,   438,   439,
     439,   439,   439,   439,   439,   440,   440,   440,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   442,
     442,   443,   443,   444,   444,   445,   445,   445,   445,   446,
     446,   447,   447,   448,   448,   449,   449,   450,   450,   451,
     451,   452,   453,   453,   453,   453,   454,   454,   455,   455,
     456,   456,   456,   456,   457,   457,   457,   458,   458,   458,
     459,   459,   460,   460,   461,   462,   463,   463,   464,   464,
     464,   464,   464,   464,   464,   464,   464,   464,   464,   464,
     465,   465
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
       4,     6,     1,     1,     4,     1,     1,     3,     2,     2,
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
       3,     1,     1,     1,     1,     3,     1,     4,     3,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     4,     4,
       1,     1,     4,     2,     1,     0,     1,     1,     1,     3,
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
       0,   427,     0,     0,   794,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   884,
       0,   872,   676,     0,   682,   683,   684,    25,   741,   861,
     151,   152,   685,     0,   132,     0,     0,     0,     0,    26,
       0,     0,     0,     0,   186,     0,     0,     0,     0,     0,
       0,   393,   394,   395,   398,   397,   396,     0,     0,     0,
       0,   215,     0,     0,     0,   689,   691,   692,   686,   687,
       0,     0,     0,   693,   688,     0,   660,    27,    28,    29,
      31,    30,     0,   690,     0,     0,     0,     0,   694,   399,
     528,     0,   150,   122,     0,   677,     0,     0,     4,   112,
     114,   740,     0,   659,     0,     6,   185,     7,     9,     8,
      10,     0,     0,   391,   440,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   438,   850,   851,   510,   509,   421,
     513,     0,   420,   821,   661,   668,     0,   743,   508,   390,
     824,   825,   836,   439,     0,     0,   442,   441,   822,   823,
     820,   857,   860,   498,   742,    11,   398,   397,   396,     0,
       0,    31,     0,   112,   185,     0,   928,   439,   927,     0,
     925,   924,   512,     0,   428,   435,   433,     0,     0,   480,
     481,   482,   483,   507,   505,   504,   503,   502,   501,   500,
     499,   861,   685,   663,     0,     0,   948,   843,   661,     0,
     662,   462,     0,   460,     0,   888,     0,   750,   419,   672,
     205,     0,   948,   418,   671,   666,     0,   681,   662,   867,
     868,   874,   866,   673,     0,     0,   675,   506,     0,     0,
       0,     0,   424,     0,   130,   426,     0,     0,   136,   138,
       0,     0,   140,     0,    72,    71,    66,    65,    57,    58,
      49,    69,    80,    81,     0,    52,     0,    64,    56,    62,
      83,    75,    74,    47,    70,    90,    91,    48,    86,    45,
      87,    46,    88,    44,    92,    79,    84,    89,    76,    77,
      51,    78,    82,    43,    73,    59,    93,    67,    60,    50,
      42,    41,    40,    39,    38,    37,    61,    94,    96,    54,
      35,    36,    63,   981,   982,    55,   987,    34,    53,    85,
       0,     0,   112,    95,   939,   980,     0,   983,     0,     0,
     142,     0,     0,     0,   176,     0,     0,     0,     0,     0,
       0,   752,     0,   100,   102,   304,     0,     0,   303,     0,
     219,     0,   216,   309,     0,     0,     0,     0,     0,   945,
     201,   213,   880,   884,     0,   909,     0,   696,     0,     0,
       0,   907,     0,    16,     0,   116,   193,   207,   214,   565,
     540,     0,   933,   520,   522,   524,   798,   427,   440,     0,
       0,   438,   439,   441,     0,     0,   863,   678,     0,   679,
       0,     0,     0,   175,     0,     0,   118,   295,     0,    24,
     184,     0,   212,   197,   211,   396,   399,   185,   392,   165,
     166,   167,   168,   169,   171,   172,   174,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   872,     0,   164,   865,   865,   894,
       0,     0,     0,     0,     0,     0,     0,     0,   389,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   461,   459,   799,   800,     0,   865,     0,   812,
     295,   295,   865,     0,   880,     0,   185,     0,     0,   144,
       0,   796,   791,   750,     0,   440,   438,     0,   892,     0,
     545,   749,   883,   681,   440,   438,   439,   118,     0,   295,
     417,     0,   814,   674,     0,   122,   255,     0,   527,     0,
     147,     0,     0,   425,     0,     0,     0,     0,     0,   139,
     163,   141,   981,   982,   978,   979,     0,   973,     0,     0,
       0,     0,    68,    33,    55,    32,   940,   170,   173,   143,
     122,     0,   160,   162,     0,     0,     0,     0,   103,     0,
     751,   101,    18,     0,    97,     0,   305,     0,   145,   218,
     217,     0,     0,   146,   929,     0,     0,   440,   438,   439,
     442,   441,     0,   966,   225,     0,   881,     0,     0,   148,
       0,     0,   695,   908,   741,     0,     0,   906,   746,   905,
     115,     5,    13,    14,     0,   223,     0,     0,   533,     0,
       0,   750,     0,     0,   669,   664,   534,     0,     0,     0,
       0,   798,   122,     0,   752,   797,   991,   416,   430,   494,
     830,   849,   127,   121,   123,   124,   125,   126,   390,     0,
     511,   744,   745,   113,   750,     0,   949,     0,     0,     0,
     752,   296,     0,   516,   187,   221,     0,   465,   467,   466,
       0,     0,   497,   463,   464,   468,   470,   469,   485,   484,
     487,   486,   488,   490,   492,   491,   489,   479,   478,   472,
     473,   471,   474,   475,   477,   493,   476,   864,     0,     0,
     898,     0,   750,   932,     0,   931,   948,   827,   203,   195,
     209,     0,   933,   199,   185,     0,   431,   434,   436,   444,
     458,   457,   456,   455,   454,   453,   452,   451,   450,   449,
     448,   447,   802,     0,   801,   804,   826,   808,   948,   805,
       0,     0,     0,     0,     0,     0,     0,     0,   926,   429,
     789,   793,   749,   795,     0,   665,     0,   887,     0,   886,
     221,     0,   665,   871,   870,   857,   860,     0,     0,   801,
     804,   869,   805,   422,   257,   259,   122,   531,   530,   423,
       0,   122,   239,   131,   426,     0,     0,     0,     0,     0,
     251,   251,   137,     0,     0,     0,     0,   971,   750,     0,
     955,     0,     0,     0,     0,     0,   748,     0,   660,     0,
       0,   698,   659,   703,     0,   697,   120,   702,   948,   984,
       0,     0,     0,     0,    19,     0,    20,     0,    98,     0,
       0,     0,   109,   752,     0,   107,   102,    99,   104,     0,
     302,   310,   307,     0,     0,   918,   923,   920,   919,   922,
     921,    12,   964,   965,     0,     0,     0,     0,   880,   877,
       0,   544,   917,   916,   915,     0,   911,     0,   912,   914,
       0,     5,     0,     0,     0,   559,   560,   568,   567,     0,
     438,     0,   749,   539,   543,     0,     0,   934,     0,   521,
       0,     0,   956,   798,   281,   990,     0,     0,   813,     0,
     862,   749,   951,   947,   297,   298,   658,   751,   294,     0,
     798,     0,     0,   223,   518,   189,   496,     0,   548,   549,
       0,   546,   749,   893,     0,     0,   295,   225,     0,   223,
       0,     0,   221,     0,   872,   445,     0,     0,   810,   811,
     828,   829,   858,   859,     0,     0,     0,   777,   757,   758,
     759,   766,     0,     0,     0,   770,   768,   769,   783,   750,
       0,   791,   891,   890,     0,   223,     0,   815,   680,     0,
     261,     0,     0,   128,     0,     0,     0,     0,     0,     0,
       0,   231,   232,   243,     0,   122,   241,   157,   251,     0,
     251,     0,     0,   985,     0,     0,     0,   749,   972,   974,
     954,   750,   953,     0,   750,   724,   725,   722,   723,   756,
       0,   750,   748,     0,   542,     0,     0,   900,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   977,   177,     0,   180,
     161,     0,     0,   105,   110,   111,   103,   751,   108,     0,
     306,     0,   930,   149,   966,   946,   961,   224,   226,   316,
       0,     0,   878,     0,   910,     0,    17,     0,   933,   222,
     316,     0,     0,   665,   536,     0,   670,   935,     0,   956,
     525,     0,     0,   991,     0,   286,   284,   804,   816,   948,
     804,   817,   950,     0,     0,   299,   119,     0,   798,   220,
       0,   798,     0,   495,   897,   896,     0,   295,     0,     0,
       0,     0,     0,     0,   223,   191,   681,   803,   295,     0,
     762,   763,   764,   765,   771,   772,   781,     0,   750,     0,
     777,     0,   761,   785,   749,   788,   790,   792,     0,   885,
       0,   803,     0,     0,     0,     0,   258,   532,   133,     0,
     426,   231,   233,   880,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   245,     0,     0,   967,     0,   970,   749,
       0,     0,     0,   700,   749,   747,     0,   738,     0,   750,
       0,   704,   739,   737,   904,     0,   750,   707,   709,   708,
       0,     0,   705,   706,   710,   712,   711,   727,   726,   729,
     728,   730,   732,   734,   733,   731,   720,   719,   714,   715,
     713,   716,   717,   718,   721,   976,     0,   122,     0,     0,
     106,    21,   308,     0,     0,     0,   963,     0,   390,   882,
     880,   432,   437,   443,     0,    15,     0,   390,   571,     0,
       0,   573,   566,   569,     0,   564,     0,   937,     0,   957,
     529,     0,   287,     0,     0,   282,     0,   301,   300,   956,
       0,   316,     0,   798,     0,   295,     0,   855,   316,   933,
     316,   936,     0,     0,     0,   446,     0,     0,   774,   749,
     776,   767,     0,   760,     0,     0,   750,   782,   889,   316,
       0,   122,     0,   254,   240,     0,     0,     0,   230,   153,
     244,     0,     0,   247,     0,   252,   253,   122,   246,   986,
     968,     0,   952,     0,   989,   755,   754,   699,     0,   749,
     541,   701,     0,   547,   749,   899,   736,     0,     0,     0,
      22,    23,   960,   958,   959,   227,     0,     0,     0,   397,
     388,     0,     0,     0,   202,   315,   317,     0,   387,     0,
       0,     0,   933,   390,     0,   913,   312,   208,   562,     0,
       0,   535,   523,     0,   290,   280,     0,   283,   289,   295,
     515,   956,   390,   956,     0,   895,     0,   854,   390,     0,
     390,   938,   316,   798,   852,   780,   779,   773,     0,   775,
     749,   784,   390,   122,   260,   129,   134,   155,   234,     0,
     242,   248,   122,   250,   969,     0,     0,   538,     0,   903,
     902,   735,   122,   181,   962,     0,     0,     0,   941,     0,
       0,     0,   228,     0,   933,     0,   353,   349,   355,   660,
      31,     0,   343,     0,   348,   352,   365,     0,   363,   368,
       0,   367,     0,   366,     0,   185,   319,     0,   321,     0,
     322,   323,     0,     0,   879,     0,   563,   561,   572,   570,
     291,     0,     0,   278,   288,     0,     0,   956,     0,   198,
     515,   956,   856,   204,   312,   210,   390,     0,     0,   787,
       0,   206,   256,     0,     0,   122,   237,   154,   249,   988,
     753,     0,     0,     0,     0,     0,   415,     0,   942,     0,
     333,   337,   412,   413,   347,     0,     0,     0,   328,   624,
     623,   620,   622,   621,   641,   643,   642,   612,   582,   584,
     583,   602,   618,   617,   578,   589,   590,   592,   591,   611,
     595,   593,   594,   596,   597,   598,   599,   600,   601,   603,
     604,   605,   606,   607,   608,   610,   609,   579,   580,   581,
     585,   586,   588,   626,   627,   636,   635,   634,   633,   632,
     631,   619,   638,   628,   629,   630,   613,   614,   615,   616,
     639,   640,   644,   646,   645,   647,   648,   625,   650,   649,
     652,   654,   653,   587,   657,   655,   656,   651,   637,   577,
     360,   574,     0,   329,   381,   382,   380,   373,     0,   374,
     330,   407,     0,     0,     0,     0,   411,     0,   185,   194,
     311,     0,     0,     0,   279,   293,   853,     0,     0,   383,
     122,   188,   956,     0,     0,   200,   956,   778,     0,   122,
     235,   135,   156,     0,   537,   901,   179,   331,   332,   410,
     229,     0,     0,   750,     0,   356,   344,     0,     0,     0,
     362,   364,     0,     0,   369,   376,   377,   375,     0,     0,
     318,   943,     0,     0,     0,   414,     0,   313,     0,   292,
       0,   557,   752,   122,     0,     0,   190,   196,     0,   786,
       0,     0,   158,   334,   112,     0,   335,   336,     0,     0,
     350,   749,   358,   354,   359,   575,   576,     0,   345,   378,
     379,   371,   372,   370,   408,   405,   966,   324,   320,   409,
       0,   314,   558,   751,     0,     0,   384,   122,   192,     0,
     238,     0,   183,     0,   390,     0,   357,   361,     0,     0,
     798,   326,     0,   555,   514,   517,     0,   236,     0,     0,
     159,   341,     0,   389,   351,   406,   944,     0,   752,   401,
     798,   556,   519,     0,   182,     0,     0,   340,   956,   798,
     265,   402,   403,   404,   991,   400,     0,     0,     0,   339,
       0,   401,     0,   956,     0,   338,   385,   122,   325,   991,
       0,   270,   268,     0,   122,     0,     0,   271,     0,     0,
     266,   327,     0,   386,     0,   274,   264,     0,   267,   273,
     178,   275,     0,     0,   262,   272,     0,   263,   277,   276
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   108,   871,   611,   173,  1428,   706,
     340,   341,   342,   343,   833,   834,   835,   110,   111,   112,
     113,   114,   394,   643,   644,   532,   243,  1493,   538,  1409,
    1494,  1732,   822,   335,   560,  1692,  1050,  1227,  1750,   411,
     174,   645,   911,  1112,  1284,   118,   614,   928,   646,   665,
     932,   594,   927,   223,   513,   647,   615,   929,   413,   360,
     377,   121,   913,   874,   857,  1067,  1431,  1165,   981,  1641,
    1497,   783,   987,   537,   792,   989,  1317,   775,   970,   973,
    1154,  1757,  1758,   633,   634,   659,   660,   347,   348,   354,
    1465,  1620,  1621,  1238,  1355,  1454,  1614,  1740,  1760,  1651,
    1696,  1697,  1698,  1441,  1442,  1443,  1444,  1653,  1654,  1660,
    1708,  1447,  1448,  1452,  1607,  1608,  1609,  1631,  1788,  1356,
    1357,   175,   123,  1774,  1775,  1612,  1359,  1360,  1361,  1362,
     124,   236,   533,   534,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,  1477,   135,   910,  1111,   136,   630,
     631,   632,   240,   386,   528,   620,   621,  1189,   622,  1190,
     137,   138,   139,   813,   140,   141,  1682,   142,   616,  1467,
     617,  1081,   879,  1255,  1252,  1600,  1601,   143,   144,   145,
     226,   146,   227,   237,   398,   520,   147,  1009,   817,   148,
    1010,   902,   571,  1011,   956,  1134,   957,  1136,  1137,  1138,
     959,  1295,  1296,   960,   751,   503,   187,   188,   648,   636,
     486,  1097,  1098,   737,   738,   898,   150,   229,   151,   152,
     177,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     698,   233,   234,   597,   216,   217,   701,   702,  1195,  1196,
     370,   371,   865,   163,   585,   164,   629,   165,   326,  1622,
    1672,   361,   406,   654,   655,  1003,  1092,  1236,   854,   855,
     797,   798,   799,   327,   328,   819,  1430,   896
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1472
static const yytype_int16 yypact[] =
{
   -1472,   197, -1472, -1472,  5419, 13024, 13024,     4, 13024, 13024,
   13024, 10879, 13024, 13024, -1472, 13024, 13024, 13024, 13024, 13024,
   13024, 13024, 13024, 13024, 13024, 13024, 13024, 15802, 15802, 11074,
   13024,  4315,    17,   172, -1472, -1472, -1472, -1472, -1472,   361,
   -1472, -1472,   318, 13024, -1472,   172,   220,   223,   268, -1472,
     172, 11269,  1014, 11464, -1472, 13974,  4746,   293, 13024,  1371,
      79, -1472, -1472, -1472,    56,    55,    78,   332,   337,   340,
     345, -1472,  1014,   371,   392, -1472, -1472, -1472, -1472, -1472,
   13024,   499,   607, -1472, -1472,  1014, -1472, -1472, -1472, -1472,
    1014, -1472,  1014, -1472,   391,   396,  1014,  1014, -1472,   328,
   -1472, 11659, -1472, -1472,    43,   516,   552,   552, -1472,   508,
     412,    -1,   411, -1472,   104, -1472,   565, -1472, -1472, -1472,
   -1472,  1275,  1264, -1472, -1472,   423,   425,   439,   446,   448,
     452,   454,   489, 11253, -1472, -1472, -1472, -1472,    97, -1472,
     553,   594, -1472,   155,   500, -1472,   545,   227, -1472,  2236,
     168, -1472, -1472,  2588,    76,   514,   150, -1472,   156,   186,
     517,   192, -1472, -1472,   649, -1472, -1472, -1472,   567,   547,
     570, -1472, 13024, -1472,   565,  1264, 16480,  3143, 16480, 13024,
   16480, 16480, 13412,   561, 15198, 13412, 16480,   695,  1014,   689,
     689,   459,   689,   689,   689,   689,   689,   689,   689,   689,
     689, -1472, -1472, -1472,   301, 13024,   588, -1472, -1472,   612,
     581,   273,   587,   273, 15802, 15408,   578,   771, -1472,   567,
   -1472, 13024,   588, -1472,   644, -1472,   646,   616, -1472,   163,
   -1472, -1472, -1472,   273,    76, 11854, -1472, -1472, 13024,  8734,
     797,   105, 16480,  9709, -1472, 13024, 13024,  1014, -1472, -1472,
   11838,   618, -1472, 13008, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472,  3537, -1472,  3537, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472,    92,    96,   570, -1472, -1472, -1472, -1472,
     627,  2142,    98, -1472, -1472,   660,   803, -1472,   666, 14742,
   -1472,   630,   632, 13203, -1472,    44, 14585,   979,   979,  1014,
     634,   817,   643, -1472,    71, -1472, 15426,   109, -1472,   707,
   -1472,   709, -1472,   824,   110, 15802, 13024, 13024,   654,   670,
   -1472, -1472, 15519, 11074,   112,   288,   444, -1472, 13219, 15802,
     530, -1472,  1014, -1472,   480,   412, -1472, -1472, -1472, -1472,
    4574,   832,   748, -1472, -1472, -1472,   143, 13024,   661,   662,
   16480,   665,  2140,   668,  5614, 13024, -1472,   479,   664,   625,
     479,   456,   465, -1472,  1014,  3537,   667, 10099, 13974, -1472,
   -1472,   595, -1472, -1472, -1472, -1472, -1472,   565, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, 13024, 13024, 13024,
   12049, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024,
   13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024,
   13024, 13024, 13024, 13024, 16200, 13024, -1472, 13024, 13024, 13024,
   13414,  1014,  1014,  1014,  1014,  1014,  1275,   750,  1420,  9904,
   13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024, 13024,
   13024, 13024, -1472, -1472, -1472, -1472,  1405, 13024, 13024, -1472,
   10099, 10099, 13024, 13024, 15519,   672,   565, 12244, 14632, -1472,
   13024, -1472,   673,   854,   715,   677,   678, 13554,   273, 12439,
   -1472, 12634, -1472,   616,   679,   680,  2325, -1472,   306, 10099,
   -1472,  1496, -1472, -1472, 14679, -1472, -1472, 10294, -1472, 13024,
   -1472,   786,  8929,   877,   698, 16362,   879,   129,   106, -1472,
   -1472, -1472,   717, -1472, -1472, -1472,  3537,  1142,   701,   885,
   15333,  1014, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472,   716, -1472, -1472,   706,   734,   712,   736,   255,  1614,
    1600, -1472, -1472,  1014,  1014, 13024,   273,    79, -1472, -1472,
   -1472, 15333,   845, -1472,   273,   132,   133,   743,   759,  2530,
     184,   760,   747,   364,   820,   765,   273,   134,   767, -1472,
     861,  1014, -1472, -1472,   889,  2853,   419, -1472, -1472, -1472,
     412, -1472, -1472, -1472,   929,   830,   790,   323,   811, 13024,
     831,   960,   783,   822, -1472,   175, -1472,  3537,  3537,   959,
     797,   143, -1472,   792,   971, -1472,  3537,    90, -1472,   432,
     173, -1472, -1472, -1472, -1472, -1472, -1472, -1472,  2265,  3371,
   -1472, -1472, -1472, -1472,   973,   809, -1472, 15802, 13024,   800,
     981, 16480,   977, -1472, -1472,   864,   621, 11449,  3831, 13412,
   13024, 16433, 14421, 10079, 11053, 12222, 12611, 12025, 13548, 13548,
   13548, 13548,  2685,  2685,  2685,  2685,  2685,  1504,  1504,   663,
     663,   663,   459,   459,   459, -1472,   689, 16480,   804,   806,
   15964,   813,   986,    -9, 13024,   219,   588,    82, -1472, -1472,
   -1472,   984,   748, -1472,   565, 15616, -1472, -1472, -1472, 13412,
   13412, 13412, 13412, 13412, 13412, 13412, 13412, 13412, 13412, 13412,
   13412, 13412, -1472, 13024,   388, -1472,   176, -1472,   588,   398,
     808,  3419,   821,   826,   814,  3504,   137,   818, -1472, 16480,
    1867, -1472,  1014, -1472,    90,   458, 15802, 16480, 15802, 16011,
     864,    90,   273,   180, -1472,   175,   858,   827, 13024, -1472,
     188, -1472, -1472, -1472,  8539,   584, -1472, -1472, 16480, 16480,
     172, -1472, -1472, -1472, 13024,   913, 15216, 15333,  1014,  9124,
     833,   837, -1472,    85,   928,   897,   867, -1472,  1025,   852,
    2915,  3537, 15333, 15333, 15333, 15333, 15333,   856,   894,   860,
   15333,   435,   896, -1472,   851, -1472, 16572, -1472,    19, -1472,
    5809,   875,   862,  1600, -1472,  1600, -1472,  1014,  1014,  1600,
    1600,  1014, -1472,  1042,   865, -1472,   262, -1472, -1472,  3758,
   -1472, 16572,  1046, 15802,   866, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472,    94,  1014,   875,   870, 15519, 15709,
    1052, -1472, -1472, -1472, -1472,   876, -1472, 13024, -1472, -1472,
    4997, -1472,  3537,   875,   881, -1472, -1472, -1472, -1472,  1056,
     884, 13024,  4574, -1472, -1472, 13414,   898, -1472,  3537, -1472,
     893,  6004,  1060,    61, -1472, -1472,   144,  1405, -1472,  1496,
   -1472,  3537, -1472, -1472,   273, 16480, -1472, 10489, -1472, 15333,
      54,   905,   875,   830, -1472, -1472, 14421, 13024, -1472, -1472,
   13024, -1472, 13024, -1472,  3929,   907, 10099,   820,  1065,   830,
    3537,  1087,   864,  1014, 16200,   273,  3976,   915, -1472, -1472,
     177,   916, -1472, -1472,  1095,   997,   997,  1867, -1472, -1472,
   -1472,  1059,   925,   424,   926, -1472, -1472, -1472, -1472,  1108,
     927,   673,   273,   273, 12829,   830,  1496, -1472, -1472,  4063,
     598,   172,  9709, -1472,  6199,   931,  6394,   932, 15216, 15802,
     930,   985,   273, 16572,  1112, -1472, -1472, -1472, -1472,   281,
   -1472,    64,  3537, -1472,  1000,  3537,  1014,  1142, -1472, -1472,
   -1472,  1126, -1472,   949,   973,   641,   641,  1077,  1077,  4228,
     951,  1139, 15333, 15022,  4574,  4159, 14882, 15333, 15333, 15333,
   15333, 15123, 15333, 15333, 15333, 15333, 15333, 15333, 15333, 15333,
   15333, 15333, 15333, 15333, 15333, 15333, 15333, 15333, 15333, 15333,
   15333, 15333, 15333, 15333, 15333,  1014, -1472, -1472,  1067, -1472,
   -1472,   958,   961, -1472, -1472, -1472,   359,  1614, -1472,   969,
   -1472, 15333,   273, -1472,   364, -1472,   585,  1154, -1472, -1472,
     138,   963,   273, 10684, -1472,  2639, -1472,  5224,   748,  1154,
   -1472,   473,    27, -1472, 16480,  1030,   978, -1472,   991,  1060,
   -1472,  3537,   797,  3537,    81,  1153,  1094,   207, -1472,   588,
     208, -1472, -1472, 15802, 13024, 16480, 16572,   980,    54, -1472,
     993,    54,   994, 14421, 16480, 16070,   999, 10099,   996,   998,
    3537,  1001,  1006,  3537,   830, -1472,   616,   418, 10099, 13024,
   -1472, -1472, -1472, -1472, -1472, -1472,  1054,  1003,  1182,  1110,
    1867,  1051, -1472,  4574,  1867, -1472, -1472, -1472, 15802, 16480,
    1013, -1472,   172,  1174,  1130,  9709, -1472, -1472, -1472,  1017,
   13024,   985,   273, 15519, 15216,  1020, 15333,  6589,   635,  1027,
   13024,    69,   313, -1472,  1043,  3537, -1472,  1086, -1472,  3071,
    1190,  1047, 15333, -1472, 15333, -1472,  1048, -1472,  1101,  1230,
    1053, -1472, -1472, -1472, 16117,  1055,  1237, 16616, 16659,  4927,
   15333, 16527, 10469,  3247, 12417, 12806, 13688, 14876, 14876, 14876,
   14876,  4099,  4099,  4099,  4099,  4099,  1572,  1572,   641,   641,
     641,  1077,  1077,  1077,  1077, -1472,  1061, -1472,  1062,  1068,
   -1472, -1472, 16572,  1014,  3537,  3537, -1472,   875,    91, -1472,
   15519, -1472, -1472, 13412,  1070, -1472,  1066,   762, -1472,   321,
   13024, -1472, -1472, -1472, 13024, -1472, 13024, -1472,   797, -1472,
   -1472,   145,  1246,  1181, 13024, -1472,  1076,   273, 16480,  1060,
    1078, -1472,  1079,    54, 13024, 10099,  1082, -1472, -1472,   748,
   -1472, -1472,  1084,  1085,  1090, -1472,  1092,  1867, -1472,  1867,
   -1472, -1472,  1097, -1472,  1141,  1098,  1270, -1472,   273, -1472,
    1251, -1472,  1099, -1472, -1472,  1103,  1104,   139, -1472, -1472,
   16572,  1105,  1111, -1472, 10863, -1472, -1472, -1472, -1472, -1472,
   -1472,  3537, -1472,  3537, -1472, 16572, 16172, -1472, 15333,  4574,
   -1472, -1472, 15333, -1472, 15333, -1472, 14743, 15333,  1117,  6784,
   -1472, -1472,   585, -1472, -1472, -1472,   580, 14114,   875,  1184,
   -1472,   589,  1122,   710, -1472, -1472, -1472,   750,  2692,   114,
     115,  1119,   748,  1420,   141, -1472, -1472, -1472,  1152,  4490,
    4587, 16480, -1472,   249,  1302,  1236, 13024, -1472, 16480, 10099,
    1204,  1060,  1010,  1060,  1137, 16480,  1138, -1472,  1093,  1144,
    1286, -1472, -1472,    54, -1472, -1472,  1194, -1472,  1867, -1472,
    4574, -1472,  1589, -1472,  8539, -1472, -1472, -1472, -1472,  9319,
   -1472, -1472, -1472,  8539, -1472,  1143, 15333, 16572,  1199, 16572,
   16219, 14743, -1472, -1472, -1472,   875,   875,  1014, -1472,  1323,
   15022,    88, -1472, 14114,   748,  2365, -1472,  1164, -1472,   119,
    1148,   120, -1472, 14423, -1472, -1472, -1472,   123, -1472, -1472,
    1265, -1472,  1150, -1472,  1259,   565, -1472, 14254, -1472, 14254,
   -1472, -1472,  1329,   750, -1472, 13694, -1472, -1472, -1472, -1472,
    1332,  1267, 13024, -1472, 16480,  1161,  1165,  1060,   533, -1472,
    1204,  1060, -1472, -1472, -1472, -1472,  1877,  1167,  1867, -1472,
    1220, -1472,  8539,  9514,  9319, -1472, -1472, -1472,  8539, -1472,
   16572, 15333, 15333,  6979,  1171,  1175, -1472, 15333, -1472,   875,
   -1472, -1472, -1472, -1472, -1472,  3537,  2063,   589, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472, -1472,
     171, -1472,  1122, -1472, -1472, -1472, -1472, -1472,   118,   146,
   -1472,  1343,   124, 14742,  1259,  1353, -1472,  3537,   565, -1472,
   -1472,  1178,  1355, 13024, -1472, 16480, -1472,   117,  1179, -1472,
   -1472, -1472,  1060,   533, 13834, -1472,  1060, -1472,  1867, -1472,
   -1472, -1472, -1472,  7174, 16572, 16572, -1472, -1472, -1472, 16572,
   -1472,  1160,   126,  1363,  1183, -1472, -1472, 15333, 14423, 14423,
    1319, -1472,  1265,  1265,   351, -1472, -1472, -1472, 15333,  1297,
   -1472,  1213,  1200,   125, 15333, -1472,  1014, -1472, 15333, 16480,
    1307, -1472,  1385, -1472,  7369,  1206, -1472, -1472,   533, -1472,
    7564,  1205,  1285, -1472,  1299,  1247, -1472, -1472,  1310,  3537,
   -1472,  2063, -1472, -1472, 16572, -1472, -1472,  1248, -1472,  1367,
   -1472, -1472, -1472, -1472, 16572,  1398,   364, -1472, -1472, 16572,
    1226, 16572, -1472,   121,  1229,  7759, -1472, -1472, -1472,  1225,
   -1472,  1232,  1253,  1014,  1420,  1262, -1472, -1472, 15333,   131,
      74, -1472,  1340, -1472, -1472, -1472,  7954, -1472,   875,   862,
   -1472,  1274,  1014,  1107, -1472, 16572, -1472,  1255,  1436,   713,
      74, -1472, -1472,  1368, -1472,   875,  1266, -1472,  1060,   130,
   -1472, -1472, -1472, -1472,  3537, -1472,  1271,  1273,   127, -1472,
     539,   713,   157,  1060,  1263, -1472, -1472, -1472, -1472,  3537,
     373,  1445,  1384,   539, -1472,  8149,   183,  1455,  1389, 13024,
   -1472, -1472,  8344, -1472,   381,  1460,  1400, 13024, -1472, 16480,
   -1472,  1469,  1402, 13024, -1472, 16480, 13024, -1472, 16480, 16480
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1472, -1472, -1472,  -537, -1472, -1472, -1472,   445,    49,   -33,
     395, -1472,  -250,  -497, -1472, -1472,   429,    -2,  1603, -1472,
    2722, -1472,  -483, -1472,    25, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472,  -261, -1472, -1472,  -143,
     116,    29, -1472, -1472, -1472, -1472, -1472, -1472,    30, -1472,
   -1472, -1472, -1472, -1472, -1472,    31, -1472, -1472,  1023,  1031,
    1028,   -90,  -661,  -821,   566,   623,  -268,   338,  -880, -1472,
       8, -1472, -1472, -1472, -1472,  -698,   190, -1472, -1472, -1472,
   -1472,  -262, -1472,  -590, -1472,  -428, -1472, -1472,   923, -1472,
      21, -1472, -1472,  -991, -1472, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472,    -7, -1472,    73, -1472, -1472, -1472,
   -1472, -1472,   -91, -1472,   159,  -874, -1472, -1471,  -280, -1472,
    -140,   113,  -120,  -267, -1472,   -98, -1472, -1472, -1472,   174,
     -17,    16,    35,  -702,   -65, -1472, -1472,    34, -1472,     9,
   -1472, -1472,    -5,   -41,    41, -1472, -1472, -1472, -1472, -1472,
   -1472, -1472, -1472, -1472,  -561,  -818, -1472, -1472, -1472, -1472,
   -1472,  1611, -1472, -1472, -1472, -1472, -1472,   443, -1472, -1472,
   -1472, -1472, -1472, -1472, -1472, -1472,  -866, -1472,  2162,     6,
   -1472,   886,  -384, -1472, -1472,  -466,  3281,  3542, -1472, -1472,
     513,  -159,  -604, -1472, -1472,   600,   394,  -664,   387, -1472,
   -1472, -1472, -1472, -1472,   569, -1472, -1472, -1472,    75,  -822,
    -107,  -410,  -405, -1472,   647,  -104, -1472, -1472,    23,    37,
     573, -1472, -1472,  1336,   -16, -1472,  -337,    36,  -348,   103,
     169, -1472, -1472,  -449,  1185, -1472, -1472, -1472, -1472, -1472,
     735,   684, -1472, -1472, -1472,  -336,  -686, -1472,  1151,  -854,
   -1472,   -69,  -183,     1,   773, -1472,  -740,   225,  -154,   507,
     577, -1472, -1472, -1472, -1472,   532,  1243, -1053
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -976
static const yytype_int16 yytable[] =
{
     176,   178,   418,   180,   181,   182,   184,   185,   186,   467,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   324,   378,   215,   218,   931,   381,   382,   115,
     894,   495,   625,   117,   119,   120,   389,   225,   242,   517,
    1261,   890,   774,   624,   626,   746,   250,   760,   253,   239,
     489,   333,   414,   336,   230,   418,   908,   344,   512,   466,
     391,   244,   742,   743,  1085,   332,   248,   232,   231,   889,
     695,  1093,   832,   837,   870,   242,   735,   820,   241,   149,
     374,   736,   977,   375,   388,   393,   958,   565,   567,  1247,
     331,   767,  1110,   991,   992,  1346,   390,  1509,  1161,   965,
    1315,   -68,    14,  1064,   323,   -33,   -68,   -32,  1121,    14,
     -33,   770,   -32,   408,   529,   364,   771,   122,   577,   582,
     116,   529,    14,  1457,  1459,   201,   521,  1662,  -346,  1517,
     522,   391,  1602,  1669,  1669,  1699,  1509,   790,   788,    14,
    1064,   843,   529,   859,  1150,   487,   859,   859,   859,   891,
     859,  -839,  -948,  1663,  1680,   388,   393,   561,  1742,   487,
     353,   345,  1686,  1262,   506,   -96,  -550,   390,  1170,  1171,
     765,   573,  1045,   405,   498,   351,  1665,   349,    14,   -96,
     515,  1094,  1373,   352,   350,  1657,  1253,  -948,   505,   393,
    -948,    14,   179,   405,  1790,  1666,  1188,     3,  1667,  1681,
     390,  1347,  1658,  1743,   514,   235,  1348,   396,    61,    62,
      63,   166,  1349,   415,  1350,   562,   390,  1728,  1254,  -552,
    1804,  1659,   484,   485,  -831,  -832,  1095,  1374,   574,  1058,
     524,  -663,  -873,   524,   395,  -670,   104,  -834,   417,  1791,
     242,   535,  -838,  -285,   492,  -835,  -837,  -751,  1263,  -876,
    -751,  1351,  1352,  -552,  1353,  -833,  1173,  -875,   601,  1316,
     993,   492,   468,  -269,   526,  1805,   546,   488,   531,  1065,
     926,  1124,   346,  -839,   666,   416,  -818,  -819,  1510,  1511,
    1382,   488,   -68,  1354,  1308,  -843,   -33,  1388,   -32,  1390,
    1168,   496,  1172,   974,   409,   530,   556,   791,   976,   578,
     583,  1700,   599,  1283,  1458,  1460,  1756,  1664,  1402,  -346,
    1518,  1096,  1375,  1603,  1670,  1718,   588,  1785,   789,  -751,
    1107,   844,   845,   860,  1792,  1294,   944,  1239,  1408,  -285,
    1464,  1470,  1054,  1055,  1077,   344,   344,   568,   491,  -669,
     587,   591,   664,  -842,   753,  -664,  -831,  -832,   418,  1258,
    1806,   242,   390,   747,  -873,   827,  -841,   600,   215,  -834,
     238,  -845,   573,   605,  -838,  -848,   493,  -835,  -837,  -526,
     610,  -876,   491,   849,  1368,   324,   876,  -833,   323,  -875,
    -662,  1711,   184,   493,  1169,  1170,  1171,  -553,   201,   504,
     649,   586,  1246,   -95,   378,   711,   712,   414,  -818,  -819,
    1712,  1486,   661,  1713,   716,   484,   485,   -95,   245,  1070,
     383,   246,   828,   852,   853,  -844,  1471,  1170,  1171,   404,
     484,   485,   667,   668,   669,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   109,
     696,   367,   697,   697,   700,  1797,   247,   323,  1305,   827,
     225,   635,   883,  1811,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,   730,   731,   230,   718,   379,
    1297,   877,   697,   741,   334,   661,   661,   697,   745,  1100,
     232,   231,   719,  1429,  1101,   749,   878,   251,  1118,   104,
     322,  -554,  1167,   717,   757,  1318,   759,   122,   467,   705,
     116,  1418,  1141,   384,   661,   454,   404,   359,  1270,   385,
     355,  1272,   778,   925,   779,   356,   365,   455,   357,  1380,
     897,  1260,   899,   358,   625,   376,   777,   359,   365,   403,
    1798,   359,   359,   923,   607,   624,   626,   365,  1812,   379,
    1126,   484,   485,   607,   764,   937,   323,   782,   466,   362,
     832,   365,  -806,   707,   484,   485,   359,   836,   836,   404,
     839,   933,  -809,  1051,  1142,  1052,  -806,   153,   880,  1512,
     363,   365,  1490,   714,   380,  -665,  -809,   366,  -948,   739,
     484,   485,  -807,  1389,   602,   368,   369,   915,   365,   407,
     211,   213,   410,  1615,   397,  1616,  -807,   368,   369,   405,
     707,   869,   365,   419,   390,   420,   368,   369,   607,  1435,
    -846,   766,   457,  1395,   772,  1396,  -948,   699,   517,   421,
     368,   369,  1248,   502,   365,  1046,   422,   404,   423,   998,
     400,  1478,   424,  1480,   425,  1249,  -846,   897,   899,   367,
     368,   369,   651,   905,   966,   899,   740,   967,   971,   972,
    1285,   744,   652,   458,  1250,   916,   625,   368,   369,    37,
     612,   613,  1152,  1153,   392,    37,  1462,   624,   626,   426,
     608,   368,   369,  1384,   109,  1234,  1235,    37,   109,  1276,
      49,   459,   536,  1041,  1042,  1043,    49,  1372,   460,   924,
    1286,    37,   490,   368,   369,  -840,   635,   365,    49,  1044,
    1425,  1426,  1436,   607,  1307,   451,   452,   453,  -551,   454,
    -663,  1782,    49,  1629,  1630,  1437,  1438,   372,   936,  1786,
    1787,   455,   564,   566,  1489,   494,  1796,  1628,  1312,  1170,
    1171,  1633,   501,   170,  1339,   392,    85,  1439,  1513,    87,
      88,   499,    89,  1440,    91,    87,    88,   455,    89,   171,
      91,   468,   405,   969,   372,   507,  1346,    87,    88,  -844,
      89,   171,    91,   510,   555,   491,   368,   369,   392,   242,
     511,    87,    88,   663,    89,   171,    91,   508,  1709,  1710,
      37,  1364,  1705,  1706,   516,   625,   975,  -661,   373,   518,
    1145,  1001,  1004,  1487,   519,   527,   624,   626,   540,   914,
      14,    49,   153,  -975,   986,   547,   153,   550,  1404,   551,
     557,   836,   558,   836,  1637,   569,   570,   836,   836,  1056,
    1771,  1772,  1773,   572,  1413,   579,  1449,   580,   581,   109,
     399,   401,   402,   592,   593,  1181,   627,  1386,   628,   653,
     637,   638,  1185,   322,   639,  -117,   359,   641,   650,    54,
     663,   750,  1075,   752,  1125,   602,   754,   755,   761,   762,
      87,    88,  1347,    89,   171,    91,  1084,  1348,   780,    61,
      62,    63,   166,  1349,   415,  1350,   529,   122,   784,   800,
     116,   546,  1685,   787,   801,   115,  1688,   823,  1450,   117,
     119,   120,  1105,   825,   821,   555,   359,   709,   359,   359,
     359,   359,  1113,   209,   209,  1114,  1266,  1115,  1759,   576,
    1492,   661,  1351,  1352,   824,  1353,   826,   842,   584,  1498,
     589,   734,   846,   122,   705,   596,   116,   851,  1759,  1503,
     225,    37,   606,   862,   863,   149,   416,  1781,   847,   850,
     856,  1475,   555,   858,  1367,    37,   861,   230,   867,  1149,
     872,   873,    49,   875,  -685,   881,   769,   153,   635,   882,
     232,   231,   884,   888,  1689,   885,    49,   109,   892,  1290,
     893,   625,   901,   122,   903,   635,   116,  1155,  1086,   906,
     907,   909,   624,   626,   912,   922,   818,  1156,   930,   918,
     739,   919,   772,   938,   122,   921,   914,   116,  1241,   942,
     940,  -667,  1643,   978,  1346,   941,   968,   994,   838,   653,
     996,    87,    88,   988,    89,   171,    91,   990,  1780,   170,
    1330,   995,    85,    86,   997,    87,    88,  1335,    89,   171,
      91,   999,  1017,  1793,  1012,   864,   866,  1013,  1014,  1016,
     603,  1057,   625,  1049,   609,   836,  1063,  1059,    14,    37,
    1061,  1069,  1187,   624,   626,  1193,  1073,   596,  1243,   772,
    1082,  1074,  1080,  1083,  1130,  1131,  1132,    37,  1724,  1089,
      49,   603,  1242,   609,   603,   609,   609,   122,  1087,   122,
     116,  1091,   116,  1108,    37,  1117,  1120,  1346,    49,  1268,
     209,  1123,   115,  1128,  -847,   153,   117,   119,   120,  1129,
    1139,   359,   661,  1140,  1143,    49,  1146,  1144,  1163,  1164,
    1347,  1158,  1160,   661,  1243,  1348,  1166,    61,    62,    63,
     166,  1349,   415,  1350,  1175,  1179,   339,  1401,  1180,    87,
      88,    14,    89,   171,    91,  1044,  1183,  1684,  1184,  1226,
    1228,  1240,   149,  1229,  1770,   242,  1690,    87,    88,  1231,
      89,   171,    91,  1237,  1256,  1314,   926,  1264,  1300,  1269,
    1351,  1352,   794,  1353,    87,    88,  1265,    89,   171,    91,
    1303,  1257,  1273,   635,  1271,  1277,   635,  1275,  1287,  1278,
     122,  1289,  1280,   116,   416,   955,  1281,   961,  1288,   951,
    1725,  1293,  1479,  1347,  1299,  1301,  1302,  1304,  1348,  1309,
      61,    62,    63,   166,  1349,   415,  1350,  1313,  1319,   109,
    1321,  1323,    37,  1463,    61,    62,    63,   166,   167,   415,
     904,   795,   209,   984,   109,  1328,  1324,  1327,   418,  1329,
      37,   209,  1331,    49,  1746,  1369,  1334,  1333,   209,  1370,
    1338,  1371,  1340,  1351,  1352,   209,  1353,  1366,  1341,  1378,
    1376,    49,  1365,  1377,  1379,   109,   623,  1381,  1383,  1385,
     661,  1387,  1053,   653,  1391,  1398,  1392,   416,  1393,  1400,
     122,  1394,  1403,   116,  1446,  1483,  1397,  1399,   935,  1405,
    1346,   416,  1406,  1407,  1433,  1410,   170,  1767,   329,    85,
    1066,  1411,    87,    88,  1795,    89,   171,    91,  1422,  1461,
    1466,  1802,  1613,  1363,   170,   109,  1472,    85,  1473,  1476,
      87,    88,  1363,    89,   171,    91,  1481,  1482,  1488,   962,
     555,   963,  1499,  1501,    14,  1484,   109,  1507,  1515,  1516,
    1610,  1611,   734,  1617,   769,    37,  1623,   153,   635,  1624,
    1626,  1358,  1693,  1627,  1638,    37,  1636,  1668,  1506,   982,
    1358,  1647,   153,   212,   212,  1648,    49,  1674,  1677,  1678,
    1683,  1474,  1701,    54,   661,  1703,    49,  1707,   359,  1715,
     209,    61,    62,    63,   166,   167,   415,  1716,  1717,  1722,
    1133,  1133,   955,   153,  1723,  1730,  1347,  1727,  1731,  -342,
    1733,  1348,  1663,    61,    62,    63,   166,  1349,   415,  1350,
    1734,   769,  1738,  1737,  1741,  1747,  1062,   109,  1744,   109,
    1748,   109,  1761,  1604,  1749,    87,    88,  1605,    89,   171,
      91,   596,  1072,   412,  1496,    87,    88,  1754,    89,   171,
      91,  1177,  1765,   153,  1768,  1769,  1351,  1352,   416,  1353,
    1777,    37,   122,  1450,  1794,   116,  1779,  1363,   555,  1799,
    1783,   555,  1784,  1363,   153,  1363,  1800,  1625,   635,  1807,
     416,  1808,    49,  1455,  1813,  1676,   468,  1363,  1485,  1508,
     337,   338,  1814,  1816,  1817,    37,  1230,   201,  1764,   713,
     818,   710,   708,  1119,  1702,  1358,  1079,  1778,  1776,  1306,
     840,  1358,  1642,  1358,  1412,  1634,    49,   544,  1514,   545,
    1656,  1661,  1453,  1801,  1789,  1358,  1673,   122,  1640,  1496,
     116,  1632,   109,  1434,  1251,  1186,   122,  1292,   339,   116,
    1147,    87,    88,  1291,    89,   171,    91,    61,    62,    63,
     166,   167,   415,   209,  1099,   153,  1135,   153,   598,   153,
     212,   982,  1162,   448,   449,   450,   451,   452,   453,   662,
     454,  1363,  1739,   732,   549,    87,    88,  1424,    89,   171,
      91,  1233,   455,  1002,  1178,     0,    37,  1225,   201,  1618,
    1671,     0,     0,     0,     0,   955,     0,     0,     0,   955,
       0,     0,     0,  1346,  1752,     0,   733,    49,   104,  1358,
     109,   209,     0,     0,   416,   122,     0,  1720,   116,     0,
       0,   122,   109,     0,   116,     0,   122,     0,  1679,   116,
       0,  1038,  1039,  1040,  1041,  1042,  1043,     0,     0,     0,
     206,   206,     0,   418,   222,     0,     0,    14,   207,   207,
    1044,     0,   209,     0,   209,     0,     0,     0,   656,     0,
     153,   329,     0,     0,   732,     0,    87,    88,   222,    89,
     171,    91,   323,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,     0,     0,  1267,     0,  1342,     0,
      37,     0,   212,     0,     0,     0,     0,   768,     0,   104,
       0,   212,     0,   590,    37,     0,     0,     0,   212,  1347,
       0,    49,     0,     0,  1348,   212,    61,    62,    63,   166,
    1349,   415,  1350,     0,     0,    49,     0,     0,     0,     0,
       0,  1298,     0,   829,   830,     0,     0,     0,   153,   209,
       0,     0,   955,     0,   955,     0,   596,   982,     0,     0,
     153,     0,     0,     0,   209,   209,     0,     0,     0,  1351,
    1352,     0,  1353,     0,     0,     0,   122,   831,     0,   116,
      87,    88,     0,    89,   171,    91,     0,     0,   623,     0,
       0,   831,     0,   416,    87,    88,     0,    89,   171,    91,
       0,  1491,     0,     0,   109,     0,     0,     0,     0,   793,
       0,     0,   322,     0,  1809,     0,     0,   122,  1451,     0,
     116,     0,  1815,   122,     0,     0,   116,     0,  1818,     0,
       0,  1819,     0,   596,     0,   635,     0,   206,     0,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,     0,
     212,     0,     0,     0,     0,   635,     0,     0,   122,     0,
       0,   116,     0,   955,   635,     0,     0,  1753,     0,   109,
       0,     0,     0,     0,   109,     0,     0,     0,   109,   122,
       0,     0,   116,     0,   209,   209,     0,   222,     0,   222,
     886,   887,   359,     0,     0,   555,     0,     0,   322,   895,
       0,  1346,     0,     0,     0,     0,     0,     0,  1599,     0,
       0,     0,     0,     0,     0,  1606,     0,     0,     0,     0,
     623,     0,   322,     0,   322,     0,     0,     0,   122,     0,
     322,   116,   153,     0,     0,   122,   945,   946,   116,     0,
       0,     0,     0,     0,   222,    14,     0,     0,     0,     0,
       0,     0,     0,   955,     0,     0,   947,   109,   109,   109,
       0,     0,     0,   109,   948,   949,   950,    37,   109,   206,
       0,     0,     0,     0,     0,     0,   951,   207,   206,     0,
       0,     0,     0,     0,     0,   206,   207,     0,    49,     0,
       0,     0,   206,   207,     0,     0,     0,   153,     0,     0,
     207,     0,   153,   222,     0,     0,   153,  1347,     0,   209,
       0,     0,  1348,   212,    61,    62,    63,   166,  1349,   415,
    1350,     0,     0,   952,     0,     0,     0,     0,   222,     0,
       0,   222,     0,     0,     0,     0,   953,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,    88,   623,
      89,   171,    91,     0,   209,     0,     0,  1351,  1352,     0,
    1353,     0,     0,   656,   656,   954,     0,     0,     0,   209,
     209,   212,     0,     0,     0,     0,     0,   222,   555,     0,
       0,   416,     0,     0,     0,   153,   153,   153,     0,  1635,
       0,   153,     0,     0,     0,     0,   153,     0,     0,   322,
       0,     0,     0,   955,     0,     0,     0,     0,   109,     0,
       0,     0,   212,     0,   212,     0,  1694,   206,     0,     0,
       0,     0,     0,  1599,  1599,   207,     0,  1606,  1606,     0,
       0,     0,     0,     0,     0,  1078,     0,     0,     0,     0,
       0,   359,   212,     0,     0,     0,   209,     0,     0,   109,
       0,  1088,     0,     0,     0,   109,     0,     0,     0,     0,
      34,    35,    36,     0,  1102,     0,     0,     0,     0,   222,
     222,     0,   202,   811,   497,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,     0,     0,
     109,     0,   264,  1122,     0,     0,     0,     0,  1751,   212,
       0,     0,     0,     0,   811,     0,     0,     0,     0,   208,
     208,   109,     0,   224,   212,   212,     0,  1766,     0,     0,
     266,    75,    76,    77,    78,    79,   482,   483,     0,     0,
       0,     0,   204,     0,     0,   623,   153,     0,    83,    84,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
     222,   222,    93,     0,     0,  1174,     0,     0,  1176,   222,
     109,     0,     0,    49,     0,     0,    98,   109,     0,     0,
       0,   548,     0,     0,     0,     0,     0,   153,     0,     0,
     206,     0,     0,   153,     0,     0,     0,     0,   207,     0,
       0,     0,   484,   485,     0,     0,     0,     0,   542,   543,
       0,     0,     0,     0,     0,     0,   623,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   170,     0,   153,    85,
     316,     0,    87,    88,     0,    89,   171,    91,     0,     0,
       0,     0,     0,     0,   212,   212,     0,     0,   206,   153,
     320,     0,     0,     0,     0,     0,   207,     0,     0,   640,
     321,     0,     0,     0,  1259,     0,   895,     0,     0,   497,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,     0,    61,    62,    63,    64,    65,   415,   206,
       0,   206,     0,  1279,    71,   461,  1282,   207,   153,   207,
       0,     0,     0,     0,     0,   153,   208,     0,     0,     0,
       0,     0,    61,    62,    63,    64,    65,   415,     0,   206,
     811,   482,   483,    71,   461,     0,     0,   207,     0,     0,
     462,     0,   463,   222,   222,   811,   811,   811,   811,   811,
       0,     0,     0,   811,     0,   464,     0,   465,  1320,     0,
     416,     0,  1102,     0,   222,     0,     0,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,    37,   206,     0,     0,   416,
       0,     0,     0,     0,   207,     0,     0,   484,   485,   222,
       0,   206,   206,     0,     0,     0,    49,     0,     0,   207,
     207,     0,     0,     0,     0,   222,   222,  1343,  1344,     0,
       0,     0,     0,     0,   212,   222,     0,     0,  1436,     0,
       0,   222,     0,     0,     0,     0,     0,     0,     0,   212,
     212,  1437,  1438,     0,   222,     0,     0,     0,   208,     0,
       0,     0,   811,     0,   763,   222,     0,   208,     0,   170,
       0,     0,    85,    86,   208,    87,    88,     0,    89,  1440,
      91,   208,     0,   222,     0,     0,     0,   222,     0,     0,
       0,     0,   208,     0,   497,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,     0,     0,
       0,     0,     0,     0,  1414,     0,  1415,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,   206,   206,     0,     0,     0,     0,     0,     0,   207,
     207,     0,     0,     0,     0,   222,   482,   483,   222,     0,
     222,  1456,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   481,   811,   224,   222,     0,     0,
     811,   811,   811,   811,   811,   811,   811,   811,   811,   811,
     811,   811,   811,   811,   811,   811,   811,   811,   811,   811,
     811,   811,   811,   811,   811,   811,   811,   811,     0,   427,
     428,   429,     0,     0,   482,   483,   208,     0,     0,     0,
       0,     0,   484,   485,   811,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   222,   454,   222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   206,   455,     0,     0,
       0,     0,   814,     0,   207,     0,     0,     0,     0,   848,
     484,   485,   264,   222,     0,     0,   222,  -976,  -976,  -976,
    -976,  -976,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,     0,   814,     0,     0,   222,     0,     0,     0,
     266,   206,     0,   455,     0,     0,     0,     0,  1652,   207,
       0,     0,     0,     0,     0,     0,   206,   206,     0,   811,
       0,     0,    37,     0,   207,   207,     0,   325,   222,     0,
       0,     0,   222,     0,     0,   811,     0,   811,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,  -389,     0,   811,     0,     0,     0,     0,     0,    61,
      62,    63,   166,   167,   415,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,   542,   543,
       0,     0,     0,     0,  1244,     0,     0,   222,   222,     0,
     222,     0,     0,   206,     0,     0,   170,     0,     0,    85,
     316,   207,    87,    88,     0,    89,   171,    91,     0,     0,
    1675,     0,     0,   427,   428,   429,     0,     0,     0,     0,
     320,     0,     0,     0,     0,     0,   416,   208,     0,     0,
     321,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
     208,   455,     0,     0,   222,     0,   222,     0,     0,     0,
       0,   811,   222,     0,     0,   811,     0,   811,     0,     0,
     811,     0,  1735,     0,     0,   264,     0,     0,   208,   814,
     222,   222,     0,     0,   222,     0,     0,     0,     0,     0,
       0,   222,     0,     0,   814,   814,   814,   814,   814,     0,
       0,     0,   814,   266,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1048,     0,     0,   325,     0,   325,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,   222,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,   895,  1068,   811,
     208,   208,     0,     0,     0,     0,     0,     0,   222,   222,
       0,     0,   895,     0,     0,  1068,   222,     0,   222,     0,
       0,     0,     0,   325,   208,   868,     0,     0,     0,     0,
       0,   542,   543,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   222,     0,     0,     0,     0,     0,   222,   170,
       0,   814,    85,   316,  1109,    87,    88,     0,    89,   171,
      91,     0,  1000,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,   224,     0,     0,     0,
       0,   264,     0,   321,   811,   811,     0,     0,     0,     0,
     811,     0,   222,     0,     0,     0,     0,     0,   222,     0,
     222,     0,     0,     0,     0,     0,     0,   325,     0,   266,
     325,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     208,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,   497,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
       0,     0,    49,     0,   814,     0,   208,     0,     0,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   542,   543,   482,
     483,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,   814,     0,   170,     0,     0,    85,   316,
       0,    87,    88,     0,    89,   171,    91,   222,  1322,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
       0,     0,     0,     0,   222,     0,     0,     0,     0,   321,
     811,     0,     0,     0,     0,   208,     0,     0,   325,   796,
       0,   811,   812,     0,     0,   484,   485,   811,     0,     0,
       0,   811,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,   222,   812,     0,   208,     0,     0,   210,   210,
     208,     0,   228,     0,     0,  1044,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,   208,     0,   814,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   811,     0,     0,   814,     0,   814,     0,     0,   325,
     325,   222,     0,     0,     0,     0,     0,     0,   325,     0,
       0,     0,   814,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,  1345,
       0,   430,   208,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   455,     0,     0,
     814,   208,     0,     0,   814,   210,   814,     0,     0,   814,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   812,
    1432,     0,     0,  1445,   427,   428,   429,     0,     0,     0,
       0,     0,   325,   325,   812,   812,   812,   812,   812,     0,
       0,     0,   812,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,     0,   208,   900,     0,     0,     0,   264,     0,     0,
       0,     0,   455,     0,     0,     0,     0,     0,   814,     0,
       0,     0,     0,     0,     0,     0,     0,  1504,  1505,     0,
       0,     0,     0,     0,   325,   266,     0,  1445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     325,   939,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,   325,     0,     0,     0,   210,     0,     0,
       0,   812,     0,     0,     0,     0,   210,     0,    49,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
     210,     0,   325,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,   814,   814,     0,     0,     0,     0,   814,
       0,  1650,     0,   542,   543,     0,     0,     0,     0,  1445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   170,     0,     0,    85,   316,   943,    87,    88,     0,
      89,   171,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   325,   320,     0,   325,     0,   796,
       0,     0,     0,     0,     0,   321,     0,     0,     0,     0,
       0,     0,     0,     0,   812,   228,     0,     0,     0,   812,
     812,   812,   812,   812,   812,   812,   812,   812,   812,   812,
     812,   812,   812,   812,   812,   812,   812,   812,   812,   812,
     812,   812,   812,   812,   812,   812,   812,     0,   427,   428,
     429,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,   812,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   325,   454,   325,     0,     0,     0,   814,
       0,     0,     0,     0,     0,     0,   455,     0,     0,     0,
     814,   815,     0,     0,     0,     0,   814,     0,     0,     0,
     814,     0,   325,   429,     0,   325,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   430,   815,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,   812,     0,
       0,     0,     0,     0,     0,     0,     0,   325,     0,   455,
     814,   325,     0,     0,   812,     0,   812,     0,     0,     0,
    1763,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   812,     0,     0,     0,     0,  1432,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   210,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
    1060,     0,     0,     0,     0,     0,   325,   325,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,   210,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,     0,     0,     0,   210,     0,   210,
       0,     0,     0,   325,   455,   325,     0,     0,     0,     0,
     812,     0,     0,     0,   812,     0,   812,     0,     0,   812,
       0,     0,     0,     0,     0,     0,     0,   210,   815,   325,
       0,     0,     0,   427,   428,   429,     0,     0,     0,     0,
     325,     0,     0,   815,   815,   815,   815,   815,     0,     0,
       0,   815,   816,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,  1116,     0,   841,   210,     0,     0,     0,     0,     0,
       0,   455,     0,     0,     0,     0,     0,     0,   812,   210,
     210,  -976,  -976,  -976,  -976,  -976,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,     0,   325,     0,     0,     0,     0,
       0,     0,     0,   228,     0,     0,     0,  1044,  1127,  1018,
    1019,  1020,     0,     0,     0,     0,     0,     0,     0,   325,
       0,   325,     0,     0,     0,     0,     0,   325,     0,  1021,
     815,     0,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,     0,   228,     0,     0,     0,     0,
       0,     0,     0,   812,   812,     0,     0,  1044,     0,   812,
       0,     0,     0,     0,     0,     0,     0,   325,  1018,  1019,
    1020,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1151,     0,     0,  1021,   210,
     210,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   815,     0,   228,  1044,     0,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,     0,     0,     0,   983,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   325,
       0,     0,   815,     0,  1005,  1006,  1007,  1008,  1191,     0,
       0,     0,  1015,     0,     0,     0,   325,     0,     0,     0,
       0,     0,  1182,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1695,     0,     0,     0,     0,     0,   812,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
     812,     0,    34,    35,    36,    37,   812,   201,     0,     0,
     812,     0,     0,     0,   202,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,   325,     0,     0,   228,     0,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,   220,   210,   210,     0,   815,     0,     0,
       0,  1106,     0,    75,    76,    77,    78,    79,     0,     0,
     812,     0,     0,   815,   204,   815,     0,     0,     0,   170,
      83,    84,    85,    86,     0,    87,    88,     0,    89,   171,
      91,   815,     0,     0,    93,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   325,     0,    98,     0,
     427,   428,   429,   221,     0,     0,     0,     0,   104,     0,
       0,   325,     0,     0,     0,     0,     0,     0,     0,     0,
     430,   210,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,  1194,
    1197,  1198,  1199,  1201,  1202,  1203,  1204,  1205,  1206,  1207,
    1208,  1209,  1210,  1211,  1212,  1213,  1214,  1215,  1216,  1217,
    1218,  1219,  1220,  1221,  1222,  1223,  1224,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,     0,  1232,     0,     0,     0,     0,     0,   815,
     228,     0,     0,   815,     0,   815,     0,   430,   815,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,   455,   201,     0,     0,     0,
       0,     0,     0,   618,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,   228,  1468,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   203,   815,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1310,     0,
       0,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,   204,  1325,     0,  1326,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,  1336,    93,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,    98,     0,    10,
       0,     0,   619,     0,     0,     0,     0,   104,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,  1469,
       0,     0,   815,   815,     0,     0,     0,     0,   815,     0,
       0,     0,     0,     0,     0,    15,    16,  1655,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,     0,
       0,     0,     0,     0,     0,    42,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,    61,    62,    63,   166,   167,   168,     0,
    1417,    68,    69,     0,  1419,     0,  1420,     0,     0,  1421,
       0,   169,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,     0,
     170,    83,    84,    85,    86,     0,    87,    88,     0,    89,
     171,    91,     0,     0,     0,    93,     0,     0,    94,     0,
       0,     0,     0,     0,    95,     0,     0,     0,     0,    98,
      99,   100,     0,     0,   172,     0,   330,     0,   815,   104,
     105,     0,   106,   107,     0,     0,     0,     0,     0,   815,
       0,     0,     0,     0,     0,   815,     0,  1021,  1500,   815,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1736,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1044,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,   815,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1644,  1645,    14,    15,    16,     0,  1649,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
       0,    40,    41,     0,     0,     0,    42,    43,    44,    45,
       0,    46,     0,    47,     0,    48,     0,     0,    49,    50,
       0,     0,     0,    51,    52,    53,    54,    55,    56,    57,
       0,    58,    59,    60,    61,    62,    63,    64,    65,    66,
       0,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,    73,    74,     0,    75,    76,    77,    78,    79,
       0,     0,     0,    80,     0,     0,    81,     0,     0,     0,
       0,    82,    83,    84,    85,    86,     0,    87,    88,     0,
      89,    90,    91,    92,     0,     0,    93,     0,     0,    94,
       0,     0,     0,     0,     0,    95,    96,     0,    97,     0,
      98,    99,   100,     0,     0,   101,     0,   102,   103,  1076,
     104,   105,     0,   106,   107,     0,     0,     0,     0,  1704,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1714,     0,     0,     0,     0,     0,  1719,     0,     0,     0,
    1721,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
    1755,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,    55,    56,    57,     0,    58,    59,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,    82,    83,
      84,    85,    86,     0,    87,    88,     0,    89,    90,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,    96,     0,    97,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,  1245,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,    55,
      56,    57,     0,    58,    59,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,    82,    83,    84,    85,    86,     0,    87,
      88,     0,    89,    90,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,    96,     0,
      97,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,   642,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1047,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,  1090,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1157,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,  1159,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,  1311,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,  1423,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1646,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,  1691,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1726,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,  1729,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1745,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,  1762,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,  1803,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,     0,    71,    72,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
      92,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,  1810,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,    64,
      65,    66,     0,    67,    68,    69,     0,    71,    72,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    92,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,   525,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,   166,   167,    66,     0,    67,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
     781,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,   166,
     167,    66,     0,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,   985,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,   166,   167,    66,     0,    67,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
    1495,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,   166,
     167,    66,     0,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,  1639,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,    40,    41,
       0,     0,     0,    42,    43,    44,    45,     0,    46,     0,
      47,     0,    48,     0,     0,    49,    50,     0,     0,     0,
      51,    52,    53,    54,     0,    56,    57,     0,    58,     0,
      60,    61,    62,    63,   166,   167,    66,     0,    67,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,   102,   103,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,    40,    41,     0,     0,     0,    42,    43,
      44,    45,     0,    46,     0,    47,     0,    48,     0,     0,
      49,    50,     0,     0,     0,    51,    52,    53,    54,     0,
      56,    57,     0,    58,     0,    60,    61,    62,    63,   166,
     167,    66,     0,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,    80,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,     0,   102,
     103,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,    12,    13,     0,     0,     0,     0,     0,     0,
       0,   715,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,     0,     0,     0,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,   657,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   455,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,   658,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,     0,     0,   776,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,     0,     0,  1103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1044,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,  1104,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   101,   427,   428,   429,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   430,  1315,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   455,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,   183,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,  1316,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,   427,   428,   429,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   455,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,   456,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,   249,
     428,   429,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,   455,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,     0,   252,     0,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   101,   427,   428,
     429,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   455,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,   539,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,   523,     0,     0,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     670,   454,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   455,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,   715,     0,     0,     0,     0,     0,     0,     0,     0,
     455,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,     0,     0,     0,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,     0,     0,     0,     0,   756,     0,     0,     0,
       0,     0,     0,     0,     0,  1044,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,     0,
       0,   758,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,     0,     0,     0,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,     0,     0,     0,     0,     0,  1148,     0,     0,     0,
       0,     0,     0,     0,  1044,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,     0,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,   427,   428,
     429,     0,   104,   105,     0,   106,   107,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   455,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,   166,   167,   168,     0,     0,    68,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   169,
      74,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,    94,     0,   541,     0,
       0,     0,    95,     0,     0,     0,     0,    98,    99,   100,
       0,     0,   172,   427,   428,   429,     0,   104,   105,     0,
     106,   107,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   455,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
     604,    39,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,   166,
     167,   168,     0,     0,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   169,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       0,    94,     0,   559,     0,     0,     0,    95,     0,     0,
       0,     0,    98,    99,   100,     0,     0,   172,     0,     0,
       0,     0,   104,   105,     0,   106,   107,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   430,   263,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
     265,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     455,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,   201,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,   703,   309,   310,   311,     0,     0,     0,   312,
     552,   553,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   554,     0,
       0,     0,     0,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,     0,   319,  -976,  -976,  -976,  -976,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,   704,     0,   104,     0,     0,
     265,     0,     0,     0,     0,     0,   455,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,   201,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,   308,   309,   310,   311,     0,     0,     0,   312,
     552,   553,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   554,     0,
       0,     0,     0,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,   264,   319,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,     0,     0,   704,     0,   104,     0,     0,
     265,     0,   266,     0,     0,     0,  1044,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,     0,   309,   310,   311,     0,     0,     0,   312,
     313,   314,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   315,     0,
       0,    85,   316,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,   264,   319,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,  1619,     0,     0,     0,
     265,     0,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,     0,   309,   310,   311,     0,     0,     0,   312,
     313,   314,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   315,     0,
       0,    85,   316,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,   264,   319,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,  1687,     0,     0,     0,
     265,     0,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,   308,   309,   310,   311,     0,     0,     0,   312,
     313,   314,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   315,     0,
       0,    85,   316,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,   264,   319,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,     0,     0,     0,     0,
     265,     0,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,     0,   309,   310,   311,     0,     0,     0,   312,
     313,   314,     0,     0,     0,     0,     0,   254,   255,     0,
     256,   257,     0,     0,   258,   259,   260,   261,   315,     0,
       0,    85,   316,     0,    87,    88,     0,    89,   171,    91,
     317,   262,   318,   263,   264,   319,     0,     0,     0,     0,
       0,     0,   320,  1427,     0,     0,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,     0,     0,     0,     0,
     265,     0,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,    49,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,     0,
       0,     0,     0,   309,   310,   311,     0,     0,     0,   312,
     313,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,     0,
       0,    85,   316,     0,    87,    88,     0,    89,   171,    91,
     317,     0,   318,     0,     0,   319,  1519,  1520,  1521,  1522,
    1523,     0,   320,  1524,  1525,  1526,  1527,     0,     0,     0,
       0,     0,   321,     0,     0,     0,     0,     0,     0,     0,
    1528,  1529,  1530,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,  1531,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,     0,  1532,  1533,  1534,  1535,  1536,  1537,  1538,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1539,  1540,  1541,  1542,  1543,  1544,  1545,
    1546,  1547,  1548,  1549,    49,  1550,  1551,  1552,  1553,  1554,
    1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,
    1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,  1573,  1574,
    1575,  1576,  1577,  1578,  1579,     0,     0,     0,  1580,  1581,
       0,  1582,  1583,  1584,  1585,  1586,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1587,  1588,  1589,
       0,     0,     0,    87,    88,     0,    89,   171,    91,  1590,
       0,  1591,  1592,     0,  1593,   427,   428,   429,     0,     0,
       0,  1594,  1595,     0,  1596,     0,  1597,  1598,     0,     0,
       0,     0,     0,     0,     0,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,   455,     0,     0,     0,     0,     0,     0,
       0,     0,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,   255,   455,   256,   257,
       0,     0,   258,   259,   260,   261,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
       0,   263,     0,     0,     0,   563,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1044,   267,   268,   269,   270,   271,   272,   273,     0,
       0,   748,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,    49,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,     0,   773,     0,
     308,   309,   310,   311,     0,     0,     0,   312,   552,   553,
       0,     0,     0,     0,     0,   254,   255,     0,   256,   257,
       0,     0,   258,   259,   260,   261,   554,     0,     0,     0,
       0,     0,    87,    88,     0,    89,   171,    91,   317,   262,
     318,   263,     0,   319,  -976,  -976,  -976,  -976,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,     0,     0,     0,     0,     0,     0,     0,   265,     0,
       0,     0,     0,     0,  1044,     0,     0,     0,     0,     0,
       0,     0,   267,   268,   269,   270,   271,   272,   273,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,    49,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,     0,     0,     0,
    1192,   309,   310,   311,     0,     0,     0,   312,   552,   553,
       0,     0,     0,     0,     0,   254,   255,     0,   256,   257,
       0,     0,   258,   259,   260,   261,   554,     0,     0,     0,
       0,     0,    87,    88,     0,    89,   171,    91,   317,   262,
     318,   263,     0,   319,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   267,   268,   269,   270,   271,   272,   273,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,    49,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,     0,     0,     0,
       0,   309,   310,   311,  1200,     0,     0,   312,   552,   553,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   802,   803,     0,     0,   554,     0,   804,     0,
     805,     0,    87,    88,     0,    89,   171,    91,   317,     0,
     318,     0,   806,   319,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,     0,     0,     0,     0,   427,   428,
     429,     0,   202,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   979,   454,     0,     0,     0,     0,   807,
       0,    75,    76,    77,    78,    79,   455,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,   170,    83,    84,
      85,   808,     0,    87,    88,    29,    89,   171,    91,     0,
       0,     0,    93,    34,    35,    36,    37,     0,   201,     0,
       0,   809,     0,     0,     0,   202,    98,     0,     0,     0,
       0,   810,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   500,     0,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   980,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
     170,    83,    84,    85,    86,     0,    87,    88,     0,    89,
     171,    91,   802,   803,     0,    93,     0,     0,   804,     0,
     805,     0,     0,     0,     0,     0,     0,     0,     0,    98,
       0,     0,   806,     0,   205,     0,     0,     0,     0,   104,
      34,    35,    36,    37,     0,     0,     0,     0,   427,   428,
     429,     0,   202,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,   807,
       0,    75,    76,    77,    78,    79,   455,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,   170,    83,    84,
      85,   808,     0,    87,    88,    29,    89,   171,    91,     0,
       0,     0,    93,    34,    35,    36,    37,     0,   201,     0,
       0,   809,     0,     0,     0,   202,    98,     0,     0,     0,
       0,   810,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,     0,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
     170,    83,    84,    85,    86,     0,    87,    88,    29,    89,
     171,    91,     0,     0,     0,    93,    34,    35,    36,    37,
       0,   201,     0,     0,     0,     0,     0,     0,   202,    98,
       0,     0,     0,     0,   205,     0,     0,   575,     0,   104,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   595,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,    29,     0,   934,    93,     0,
       0,     0,     0,    34,    35,    36,    37,     0,   201,     0,
       0,     0,    98,     0,     0,   202,     0,   205,     0,     0,
       0,     0,   104,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
     170,    83,    84,    85,    86,     0,    87,    88,    29,    89,
     171,    91,     0,     0,     0,    93,    34,    35,    36,    37,
       0,   201,     0,     0,     0,     0,     0,     0,   202,    98,
       0,     0,     0,     0,   205,     0,     0,     0,     0,   104,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1071,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,    29,    89,   171,    91,     0,     0,     0,    93,    34,
      35,    36,    37,     0,   201,     0,     0,     0,     0,     0,
       0,   202,    98,     0,     0,     0,     0,   205,     0,     0,
       0,     0,   104,    49,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
      75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
       0,   204,     0,     0,     0,     0,   170,    83,    84,    85,
      86,     0,    87,    88,     0,    89,   171,    91,     0,     0,
       0,    93,     0,     0,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     205,     0,     0,     0,   430,   104,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   455,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   920,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,  1018,  1019,  1020,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,     0,     0,     0,   964,     0,  1021,     0,     0,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1018,  1019,  1020,  1044,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1021,     0,  1274,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,     0,     0,  1018,
    1019,  1020,     0,     0,     0,     0,     0,     0,     0,     0,
    1044,     0,     0,     0,     0,     0,     0,     0,     0,  1021,
       0,  1332,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,     0,     0,     0,    34,    35,    36,
      37,     0,   201,     0,     0,     0,     0,  1044,     0,   202,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,  1416,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,   204,
       0,     0,     0,  1502,   170,    83,    84,    85,    86,     0,
      87,    88,     0,    89,   171,    91,     0,     0,     0,    93,
       0,     0,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   221,     0,
     785,     0,   430,   104,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   427,   428,   429,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   786,   430,   917,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,  1018,  1019,  1020,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,     0,     0,     0,     0,     0,  1021,  1337,     0,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1018,  1019,  1020,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1044,     0,     0,     0,     0,
       0,     0,  1021,     0,     0,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1019,  1020,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1044,     0,     0,     0,     0,     0,  1021,     0,     0,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1020,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1044,     0,     0,     0,     0,  1021,
       0,     0,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1044
};

static const yytype_int16 yycheck[] =
{
       5,     6,   122,     8,     9,    10,    11,    12,    13,   149,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    55,    92,    29,    30,   712,    96,    97,     4,
     634,   174,   380,     4,     4,     4,   101,    31,    43,   222,
    1093,   631,   525,   380,   380,   494,    51,   513,    53,    33,
     154,    56,   121,    58,    31,   175,   660,    59,   217,   149,
     101,    45,   490,   491,   882,    56,    50,    31,    31,   630,
     454,   893,   569,   570,   611,    80,   486,   560,    43,     4,
      82,   486,   784,    85,   101,   101,   750,   337,   338,  1080,
      56,   519,   913,   791,     9,     4,   101,     9,   978,   760,
      31,     9,    48,     9,    55,     9,    14,     9,   929,    48,
      14,   521,    14,     9,     9,    80,   521,     4,     9,     9,
       4,     9,    48,     9,     9,    82,   233,     9,     9,     9,
     234,   172,     9,     9,     9,     9,     9,    31,     9,    48,
       9,     9,     9,     9,   965,    69,     9,     9,     9,   632,
       9,    69,   153,    35,    37,   172,   172,   113,    37,    69,
      82,    82,  1633,    82,   205,   174,    69,   172,   104,   105,
     518,   100,   153,   174,   179,   120,    30,   121,    48,   188,
     221,    37,    37,   128,   128,    14,   159,   188,   205,   205,
     191,    48,   188,   174,    37,    49,  1014,     0,    52,    82,
     205,   110,    31,    82,   221,   188,   115,   104,   117,   118,
     119,   120,   121,   122,   123,   171,   221,  1688,   191,    69,
      37,    50,   132,   133,    69,    69,    82,    82,   157,   833,
     235,   153,    69,   238,   191,   153,   193,    69,   122,    82,
     245,   246,    69,   189,    69,    69,    69,   186,   167,    69,
     189,   160,   161,    69,   163,    69,   192,    69,   365,   190,
     175,    69,   149,   189,   239,    82,   174,   191,   243,   175,
     188,   932,   193,   191,   417,   184,    69,    69,   190,   191,
    1271,   191,   190,   192,  1164,   188,   190,  1278,   190,  1280,
     988,   175,   990,   776,   190,   190,   329,   191,   781,   190,
     190,   175,   190,  1124,   190,   190,   175,   189,  1299,   190,
     190,   167,   167,   190,   190,   190,   357,   190,   189,   189,
     910,   189,   189,   189,   167,  1143,   189,   189,   189,   186,
     189,    82,   829,   830,   871,   337,   338,   339,   188,   153,
     357,   357,   411,   188,   503,   153,   191,   191,   468,  1089,
     167,   356,   357,   496,   191,   100,   188,    69,   363,   191,
     188,   188,   100,   368,   191,   188,   191,   191,   191,     8,
     372,   191,   188,   189,    53,   408,    53,   191,   329,   191,
     153,    30,   387,   191,   103,   104,   105,    69,    82,    88,
     395,   356,  1078,   174,   463,   464,   465,   466,   191,   191,
      49,  1392,   407,    52,   469,   132,   133,   188,   188,   858,
      82,   188,   157,    49,    50,   188,   167,   104,   105,   157,
     132,   133,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     4,
     455,   150,   457,   458,   459,    82,   188,   408,  1160,   100,
     454,   386,   621,    82,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   454,   469,   158,
    1144,   158,   487,   488,   191,   490,   491,   492,   493,   899,
     454,   454,   497,  1347,   899,   500,   173,    52,   926,   193,
      55,    69,   985,   469,   509,   192,   511,   394,   648,   460,
     394,  1329,    88,   185,   519,    56,   157,    72,  1108,   191,
     188,  1111,   527,   706,   529,   188,    82,    68,   188,  1269,
     637,  1092,   639,   188,   882,    90,   527,    92,    82,    31,
     167,    96,    97,   702,    88,   882,   882,    82,   167,   158,
     934,   132,   133,    88,   518,   738,   507,   532,   648,   188,
    1057,    82,   174,   460,   132,   133,   121,   569,   570,   157,
     575,   714,   174,   823,   150,   825,   188,     4,   619,  1433,
     188,    82,  1400,   467,   188,   153,   188,    88,   153,   486,
     132,   133,   174,  1279,   150,   151,   152,   666,    82,   188,
      27,    28,    37,  1457,    88,  1459,   188,   151,   152,   174,
     507,   192,    82,   190,   619,   190,   151,   152,    88,    30,
     188,   518,    69,  1287,   521,  1289,   191,   458,   811,   190,
     151,   152,   159,   188,    82,   818,   190,   157,   190,   798,
      88,  1381,   190,  1383,   190,   172,   188,   754,   755,   150,
     151,   152,   196,   658,   761,   762,   487,   761,    74,    75,
    1126,   492,   197,    69,   191,   670,  1014,   151,   152,    80,
     190,   191,    74,    75,   101,    80,  1362,  1014,  1014,   190,
     150,   151,   152,  1273,   239,   100,   101,    80,   243,  1117,
     101,   191,   247,    52,    53,    54,   101,  1258,   153,   704,
    1128,    80,   188,   151,   152,   188,   631,    82,   101,    68,
     130,   131,   123,    88,  1163,    52,    53,    54,    69,    56,
     153,  1774,   101,   190,   191,   136,   137,   157,   733,   190,
     191,    68,   337,   338,  1398,   188,  1789,  1477,   103,   104,
     105,  1481,    47,   154,  1227,   172,   157,   158,  1434,   160,
     161,   190,   163,   164,   165,   160,   161,    68,   163,   164,
     165,   648,   174,   768,   157,   153,     4,   160,   161,   188,
     163,   164,   165,   195,   329,   188,   151,   152,   205,   784,
       9,   160,   161,   188,   163,   164,   165,   214,  1662,  1663,
      80,  1240,  1658,  1659,   221,  1143,   780,   153,   191,   153,
     959,   800,   801,  1393,   188,     8,  1143,  1143,   190,   188,
      48,   101,   239,   153,   789,   188,   243,    14,  1301,   153,
     190,   823,   190,   825,  1488,   191,     9,   829,   830,   831,
     117,   118,   119,   190,  1317,   128,   126,   128,    14,   394,
     105,   106,   107,   189,   174,  1004,    14,  1275,   100,   404,
     189,   189,  1011,   408,   189,   188,   411,   189,   194,   109,
     188,   188,   867,     9,   933,   150,   189,   189,   189,   189,
     160,   161,   110,   163,   164,   165,   881,   115,    92,   117,
     118,   119,   120,   121,   122,   123,     9,   774,   190,   188,
     774,   174,  1632,    14,     9,   870,  1636,   191,   188,   870,
     870,   870,   907,   191,   188,   460,   461,   462,   463,   464,
     465,   466,   917,    27,    28,   920,  1099,   922,  1740,   346,
    1403,   926,   160,   161,   190,   163,   190,    82,   355,  1412,
     357,   486,   189,   820,   885,   362,   820,   190,  1760,  1422,
     934,    80,   369,    82,    83,   870,   184,  1769,   189,   189,
     130,  1379,   507,   188,   192,    80,   189,   934,    69,   964,
      31,   131,   101,   173,   153,   134,   521,   394,   893,     9,
     934,   934,   189,    14,  1638,   153,   101,   532,   186,  1138,
       9,  1329,     9,   870,   175,   910,   870,   971,   885,   189,
       9,    14,  1329,  1329,   130,     9,   551,   972,    14,   195,
     897,   195,   899,   195,   891,   192,   188,   891,  1073,   195,
     189,   153,  1495,   100,     4,   189,   189,    89,   573,   574,
     153,   160,   161,   190,   163,   164,   165,   190,  1768,   154,
    1189,   134,   157,   158,     9,   160,   161,  1196,   163,   164,
     165,   189,   191,  1783,   188,   600,   601,   153,   188,   153,
     366,     9,  1400,   191,   370,  1057,   190,   192,    48,    80,
      14,   191,  1013,  1400,  1400,  1016,    14,   494,  1073,   966,
      14,   195,   191,   189,    77,    78,    79,    80,  1682,   186,
     101,   397,  1073,   399,   400,   401,   402,   974,   190,   976,
     974,    31,   976,   188,    80,   188,    31,     4,   101,  1104,
     214,    14,  1077,   188,   188,   532,  1077,  1077,  1077,    14,
      51,   666,  1117,   188,   188,   101,   189,     9,   188,   134,
     110,   190,   190,  1128,  1129,   115,    14,   117,   118,   119,
     120,   121,   122,   123,   134,     9,   157,  1296,   189,   160,
     161,    48,   163,   164,   165,    68,   195,  1630,     9,    82,
     192,   188,  1077,   192,  1758,  1160,  1639,   160,   161,   190,
     163,   164,   165,     9,   134,  1170,   188,    14,  1152,   189,
     160,   161,    30,   163,   160,   161,    82,   163,   164,   165,
    1155,   190,   188,  1108,   191,   189,  1111,   188,   134,   191,
    1077,     9,   191,  1077,   184,   750,   190,   752,   195,    89,
    1683,   150,   192,   110,   191,    31,    76,   190,   115,   189,
     117,   118,   119,   120,   121,   122,   123,   190,   175,   774,
     134,    31,    80,  1363,   117,   118,   119,   120,   121,   122,
     657,    89,   346,   788,   789,   134,   189,   189,  1358,     9,
      80,   355,   189,   101,  1727,  1250,     9,   192,   362,  1254,
     189,  1256,   190,   160,   161,   369,   163,   191,   190,  1264,
      14,   101,   192,    82,   188,   820,   380,   189,   189,  1274,
    1275,   189,   827,   828,   190,   134,   191,   184,   188,     9,
    1167,   189,    31,  1167,   162,   192,   189,   189,   715,   190,
       4,   184,   189,   189,   110,   190,   154,   190,    55,   157,
     855,   190,   160,   161,  1787,   163,   164,   165,   191,   190,
     158,  1794,  1455,  1238,   154,   870,    14,   157,    82,   115,
     160,   161,  1247,   163,   164,   165,   189,   189,   134,   756,
     885,   758,   189,   134,    48,   191,   891,    14,   174,   191,
     190,    82,   897,    14,   899,    80,    14,   774,  1273,    82,
     189,  1238,   192,   188,   134,    80,   189,    14,  1427,   786,
    1247,   190,   789,    27,    28,   190,   101,    14,   190,    14,
     191,  1376,     9,   109,  1379,   192,   101,    58,   933,    82,
     494,   117,   118,   119,   120,   121,   122,   174,   188,    82,
     945,   946,   947,   820,     9,   190,   110,   191,   113,   100,
     153,   115,    35,   117,   118,   119,   120,   121,   122,   123,
     100,   966,    14,   165,   188,   190,   843,   972,   189,   974,
     188,   976,    82,   158,   171,   160,   161,   162,   163,   164,
     165,   858,   859,   158,  1409,   160,   161,   175,   163,   164,
     165,   996,   168,   870,   189,     9,   160,   161,   184,   163,
      82,    80,  1339,   188,   191,  1339,   190,  1382,  1013,    14,
     189,  1016,   189,  1388,   891,  1390,    82,  1472,  1393,    14,
     184,    82,   101,  1357,    14,  1618,  1363,  1402,   192,  1430,
     109,   110,    82,    14,    82,    80,  1057,    82,  1749,   466,
    1045,   463,   461,   927,  1653,  1382,   873,  1765,  1760,  1161,
     577,  1388,  1494,  1390,  1314,  1484,   101,   264,  1435,   266,
    1517,  1602,  1353,  1793,  1781,  1402,  1614,  1404,  1493,  1494,
    1404,  1480,  1077,  1349,  1081,  1012,  1413,  1140,   157,  1413,
     961,   160,   161,  1139,   163,   164,   165,   117,   118,   119,
     120,   121,   122,   657,   897,   972,   946,   974,   363,   976,
     214,   978,   979,    49,    50,    51,    52,    53,    54,   408,
      56,  1486,  1716,   158,   321,   160,   161,  1342,   163,   164,
     165,  1064,    68,   800,   997,    -1,    80,  1045,    82,  1463,
    1613,    -1,    -1,    -1,    -1,  1140,    -1,    -1,    -1,  1144,
      -1,    -1,    -1,     4,  1734,    -1,   191,   101,   193,  1486,
    1155,   715,    -1,    -1,   184,  1492,    -1,  1676,  1492,    -1,
      -1,  1498,  1167,    -1,  1498,    -1,  1503,    -1,  1623,  1503,
      -1,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      27,    28,    -1,  1753,    31,    -1,    -1,    48,    27,    28,
      68,    -1,   756,    -1,   758,    -1,    -1,    -1,   405,    -1,
    1077,   408,    -1,    -1,   158,    -1,   160,   161,    55,   163,
     164,   165,  1613,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   786,    -1,    -1,    -1,  1103,    -1,  1233,    -1,
      80,    -1,   346,    -1,    -1,    -1,    -1,   191,    -1,   193,
      -1,   355,    -1,   357,    80,    -1,    -1,    -1,   362,   110,
      -1,   101,    -1,    -1,   115,   369,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,  1148,    -1,   109,   110,    -1,    -1,    -1,  1155,   843,
      -1,    -1,  1287,    -1,  1289,    -1,  1163,  1164,    -1,    -1,
    1167,    -1,    -1,    -1,   858,   859,    -1,    -1,    -1,   160,
     161,    -1,   163,    -1,    -1,    -1,  1643,   157,    -1,  1643,
     160,   161,    -1,   163,   164,   165,    -1,    -1,   882,    -1,
      -1,   157,    -1,   184,   160,   161,    -1,   163,   164,   165,
      -1,   192,    -1,    -1,  1339,    -1,    -1,    -1,    -1,   546,
      -1,    -1,  1347,    -1,  1799,    -1,    -1,  1684,  1353,    -1,
    1684,    -1,  1807,  1690,    -1,    -1,  1690,    -1,  1813,    -1,
      -1,  1816,    -1,  1240,    -1,  1740,    -1,   214,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   214,    -1,    -1,    -1,    -1,
     494,    -1,    -1,    -1,    -1,  1760,    -1,    -1,  1725,    -1,
      -1,  1725,    -1,  1398,  1769,    -1,    -1,  1734,    -1,  1404,
      -1,    -1,    -1,    -1,  1409,    -1,    -1,    -1,  1413,  1746,
      -1,    -1,  1746,    -1,   978,   979,    -1,   264,    -1,   266,
     627,   628,  1427,    -1,    -1,  1430,    -1,    -1,  1433,   636,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,  1443,    -1,
      -1,    -1,    -1,    -1,    -1,  1450,    -1,    -1,    -1,    -1,
    1014,    -1,  1457,    -1,  1459,    -1,    -1,    -1,  1795,    -1,
    1465,  1795,  1339,    -1,    -1,  1802,    49,    50,  1802,    -1,
      -1,    -1,    -1,    -1,   321,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1488,    -1,    -1,    69,  1492,  1493,  1494,
      -1,    -1,    -1,  1498,    77,    78,    79,    80,  1503,   346,
      -1,    -1,    -1,    -1,    -1,    -1,    89,   346,   355,    -1,
      -1,    -1,    -1,    -1,    -1,   362,   355,    -1,   101,    -1,
      -1,    -1,   369,   362,    -1,    -1,    -1,  1404,    -1,    -1,
     369,    -1,  1409,   380,    -1,    -1,  1413,   110,    -1,  1103,
      -1,    -1,   115,   657,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,   136,    -1,    -1,    -1,    -1,   405,    -1,
      -1,   408,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   160,   161,  1143,
     163,   164,   165,    -1,  1148,    -1,    -1,   160,   161,    -1,
     163,    -1,    -1,   800,   801,   178,    -1,    -1,    -1,  1163,
    1164,   715,    -1,    -1,    -1,    -1,    -1,   454,  1613,    -1,
      -1,   184,    -1,    -1,    -1,  1492,  1493,  1494,    -1,   192,
      -1,  1498,    -1,    -1,    -1,    -1,  1503,    -1,    -1,  1634,
      -1,    -1,    -1,  1638,    -1,    -1,    -1,    -1,  1643,    -1,
      -1,    -1,   756,    -1,   758,    -1,  1651,   494,    -1,    -1,
      -1,    -1,    -1,  1658,  1659,   494,    -1,  1662,  1663,    -1,
      -1,    -1,    -1,    -1,    -1,   872,    -1,    -1,    -1,    -1,
      -1,  1676,   786,    -1,    -1,    -1,  1240,    -1,    -1,  1684,
      -1,   888,    -1,    -1,    -1,  1690,    -1,    -1,    -1,    -1,
      77,    78,    79,    -1,   901,    -1,    -1,    -1,    -1,   546,
     547,    -1,    89,   550,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
    1725,    -1,    30,   930,    -1,    -1,    -1,    -1,  1733,   843,
      -1,    -1,    -1,    -1,   581,    -1,    -1,    -1,    -1,    27,
      28,  1746,    -1,    31,   858,   859,    -1,  1752,    -1,    -1,
      58,   138,   139,   140,   141,   142,    66,    67,    -1,    -1,
      -1,    -1,   149,    -1,    -1,  1329,  1643,    -1,   155,   156,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     627,   628,   169,    -1,    -1,   992,    -1,    -1,   995,   636,
    1795,    -1,    -1,   101,    -1,    -1,   183,  1802,    -1,    -1,
      -1,   109,    -1,    -1,    -1,    -1,    -1,  1684,    -1,    -1,
     657,    -1,    -1,  1690,    -1,    -1,    -1,    -1,   657,    -1,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,  1400,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,  1725,   157,
     158,    -1,   160,   161,    -1,   163,   164,   165,    -1,    -1,
      -1,    -1,    -1,    -1,   978,   979,    -1,    -1,   715,  1746,
     178,    -1,    -1,    -1,    -1,    -1,   715,    -1,    -1,   189,
     188,    -1,    -1,    -1,  1091,    -1,  1093,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,   117,   118,   119,   120,   121,   122,   756,
      -1,   758,    -1,  1120,   128,   129,  1123,   756,  1795,   758,
      -1,    -1,    -1,    -1,    -1,  1802,   214,    -1,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,    -1,   786,
     787,    66,    67,   128,   129,    -1,    -1,   786,    -1,    -1,
     164,    -1,   166,   800,   801,   802,   803,   804,   805,   806,
      -1,    -1,    -1,   810,    -1,   179,    -1,   181,  1175,    -1,
     184,    -1,  1179,    -1,   821,    -1,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1103,
      -1,    -1,    -1,    -1,    -1,    80,   843,    -1,    -1,   184,
      -1,    -1,    -1,    -1,   843,    -1,    -1,   132,   133,   856,
      -1,   858,   859,    -1,    -1,    -1,   101,    -1,    -1,   858,
     859,    -1,    -1,    -1,    -1,   872,   873,  1234,  1235,    -1,
      -1,    -1,    -1,    -1,  1148,   882,    -1,    -1,   123,    -1,
      -1,   888,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1163,
    1164,   136,   137,    -1,   901,    -1,    -1,    -1,   346,    -1,
      -1,    -1,   909,    -1,   189,   912,    -1,   355,    -1,   154,
      -1,    -1,   157,   158,   362,   160,   161,    -1,   163,   164,
     165,   369,    -1,   930,    -1,    -1,    -1,   934,    -1,    -1,
      -1,    -1,   380,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1321,    -1,  1323,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1240,    -1,    -1,    -1,
      -1,   978,   979,    -1,    -1,    -1,    -1,    -1,    -1,   978,
     979,    -1,    -1,    -1,    -1,   992,    66,    67,   995,    -1,
     997,  1358,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1012,   454,  1014,    -1,    -1,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,    -1,    10,
      11,    12,    -1,    -1,    66,    67,   494,    -1,    -1,    -1,
      -1,    -1,   132,   133,  1061,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,  1091,    56,  1093,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1103,    68,    -1,    -1,
      -1,    -1,   550,    -1,  1103,    -1,    -1,    -1,    -1,   189,
     132,   133,    30,  1120,    -1,    -1,  1123,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,   581,    -1,    -1,  1143,    -1,    -1,    -1,
      58,  1148,    -1,    68,    -1,    -1,    -1,    -1,  1515,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,  1163,  1164,    -1,  1166,
      -1,    -1,    80,    -1,  1163,  1164,    -1,    55,  1175,    -1,
      -1,    -1,  1179,    -1,    -1,  1182,    -1,  1184,    -1,    -1,
      -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   109,    -1,  1200,    -1,    -1,    -1,    -1,    -1,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,   657,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
      -1,    -1,    -1,    -1,   195,    -1,    -1,  1234,  1235,    -1,
    1237,    -1,    -1,  1240,    -1,    -1,   154,    -1,    -1,   157,
     158,  1240,   160,   161,    -1,   163,   164,   165,    -1,    -1,
    1617,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
     178,    -1,    -1,    -1,    -1,    -1,   184,   715,    -1,    -1,
     188,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   756,    -1,
     758,    68,    -1,    -1,  1321,    -1,  1323,    -1,    -1,    -1,
      -1,  1328,  1329,    -1,    -1,  1332,    -1,  1334,    -1,    -1,
    1337,    -1,  1699,    -1,    -1,    30,    -1,    -1,   786,   787,
    1347,  1348,    -1,    -1,  1351,    -1,    -1,    -1,    -1,    -1,
      -1,  1358,    -1,    -1,   802,   803,   804,   805,   806,    -1,
      -1,    -1,   810,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   821,    -1,    -1,   264,    -1,   266,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1400,    -1,   843,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,  1774,   856,  1416,
     858,   859,    -1,    -1,    -1,    -1,    -1,    -1,  1425,  1426,
      -1,    -1,  1789,    -1,    -1,   873,  1433,    -1,  1435,    -1,
      -1,    -1,    -1,   321,   882,   192,    -1,    -1,    -1,    -1,
      -1,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1457,    -1,  1459,    -1,    -1,    -1,    -1,    -1,  1465,   154,
      -1,   909,   157,   158,   912,   160,   161,    -1,   163,   164,
     165,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   178,    -1,    -1,   934,    -1,    -1,    -1,
      -1,    30,    -1,   188,  1501,  1502,    -1,    -1,    -1,    -1,
    1507,    -1,  1509,    -1,    -1,    -1,    -1,    -1,  1515,    -1,
    1517,    -1,    -1,    -1,    -1,    -1,    -1,   405,    -1,    58,
     408,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     978,   979,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   101,    -1,  1012,    -1,  1014,    -1,    -1,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,   136,   137,    66,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1617,    -1,    -1,  1061,    -1,   154,    -1,    -1,   157,   158,
      -1,   160,   161,    -1,   163,   164,   165,  1634,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,
      -1,    -1,    -1,    -1,  1651,    -1,    -1,    -1,    -1,   188,
    1657,    -1,    -1,    -1,    -1,  1103,    -1,    -1,   546,   547,
      -1,  1668,   550,    -1,    -1,   132,   133,  1674,    -1,    -1,
      -1,  1678,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,  1699,   581,    -1,  1143,    -1,    -1,    27,    28,
    1148,    -1,    31,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1163,  1164,    -1,  1166,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1738,    -1,    -1,  1182,    -1,  1184,    -1,    -1,   627,
     628,  1748,    -1,    -1,    -1,    -1,    -1,    -1,   636,    -1,
      -1,    -1,  1200,    -1,    -1,    -1,    -1,    -1,  1765,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1774,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1789,    -1,    -1,    -1,    -1,    -1,    -1,  1237,
      -1,    30,  1240,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
    1328,  1329,    -1,    -1,  1332,   214,  1334,    -1,    -1,  1337,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   787,
    1348,    -1,    -1,  1351,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   800,   801,   802,   803,   804,   805,   806,    -1,
      -1,    -1,   810,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    -1,  1400,   192,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,  1416,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1425,  1426,    -1,
      -1,    -1,    -1,    -1,   872,    58,    -1,  1435,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     888,   192,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,   901,    -1,    -1,    -1,   346,    -1,    -1,
      -1,   909,    -1,    -1,    -1,    -1,   355,    -1,   101,    -1,
      -1,    -1,    -1,   362,    -1,    -1,    -1,    -1,    -1,    -1,
     369,    -1,   930,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   380,    -1,  1501,  1502,    -1,    -1,    -1,    -1,  1507,
      -1,  1509,    -1,   136,   137,    -1,    -1,    -1,    -1,  1517,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,   157,   158,   192,   160,   161,    -1,
     163,   164,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   992,   178,    -1,   995,    -1,   997,
      -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1012,   454,    -1,    -1,    -1,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,   494,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1061,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,  1091,    56,  1093,    -1,    -1,    -1,  1657,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
    1668,   550,    -1,    -1,    -1,    -1,  1674,    -1,    -1,    -1,
    1678,    -1,  1120,    12,    -1,  1123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,   581,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,  1166,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1175,    -1,    68,
    1738,  1179,    -1,    -1,  1182,    -1,  1184,    -1,    -1,    -1,
    1748,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1200,    -1,    -1,    -1,    -1,  1765,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   657,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    -1,    -1,  1234,  1235,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   715,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    -1,    -1,    -1,   756,    -1,   758,
      -1,    -1,    -1,  1321,    68,  1323,    -1,    -1,    -1,    -1,
    1328,    -1,    -1,    -1,  1332,    -1,  1334,    -1,    -1,  1337,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   786,   787,  1347,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
    1358,    -1,    -1,   802,   803,   804,   805,   806,    -1,    -1,
      -1,   810,   550,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,   192,    -1,   581,   843,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,  1416,   858,
     859,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,  1433,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   882,    -1,    -1,    -1,    68,   192,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1457,
      -1,  1459,    -1,    -1,    -1,    -1,    -1,  1465,    -1,    30,
     909,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,   934,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1501,  1502,    -1,    -1,    68,    -1,  1507,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1515,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    30,   978,
     979,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1012,    -1,  1014,    68,    -1,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,    -1,    -1,    -1,   787,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1617,
      -1,    -1,  1061,    -1,   802,   803,   804,   805,   189,    -1,
      -1,    -1,   810,    -1,    -1,    -1,  1634,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1651,    -1,    -1,    -1,    -1,    -1,  1657,
      -1,    -1,    -1,    -1,  1103,    -1,    -1,    -1,    -1,    -1,
    1668,    -1,    77,    78,    79,    80,  1674,    82,    -1,    -1,
    1678,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      -1,  1699,    -1,    -1,  1143,    -1,    -1,    -1,    -1,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,   128,  1163,  1164,    -1,  1166,    -1,    -1,
      -1,   909,    -1,   138,   139,   140,   141,   142,    -1,    -1,
    1738,    -1,    -1,  1182,   149,  1184,    -1,    -1,    -1,   154,
     155,   156,   157,   158,    -1,   160,   161,    -1,   163,   164,
     165,  1200,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1774,    -1,   183,    -1,
      10,    11,    12,   188,    -1,    -1,    -1,    -1,   193,    -1,
      -1,  1789,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,  1240,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,  1061,    -1,    -1,    -1,    -1,    -1,  1328,
    1329,    -1,    -1,  1332,    -1,  1334,    -1,    30,  1337,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    68,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,  1400,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   122,  1416,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1166,    -1,
      -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,  1182,    -1,  1184,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,  1200,   169,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,   183,    -1,    13,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,   192,
      -1,    -1,  1501,  1502,    -1,    -1,    -1,    -1,  1507,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,  1516,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   117,   118,   119,   120,   121,   122,    -1,
    1328,   125,   126,    -1,  1332,    -1,  1334,    -1,    -1,  1337,
      -1,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,   172,    -1,
      -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,   188,    -1,   190,    -1,  1657,   193,
     194,    -1,   196,   197,    -1,    -1,    -1,    -1,    -1,  1668,
      -1,    -1,    -1,    -1,    -1,  1674,    -1,    30,  1416,  1678,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,  1701,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1738,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1501,  1502,    48,    49,    50,    -1,  1507,
      -1,    -1,    55,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    77,    78,    79,    80,    81,    82,
      -1,    84,    85,    -1,    -1,    -1,    89,    90,    91,    92,
      -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,   102,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,   135,   136,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,    -1,
      -1,   154,   155,   156,   157,   158,    -1,   160,   161,    -1,
     163,   164,   165,   166,    -1,    -1,   169,    -1,    -1,   172,
      -1,    -1,    -1,    -1,    -1,   178,   179,    -1,   181,    -1,
     183,   184,   185,    -1,    -1,   188,    -1,   190,   191,   192,
     193,   194,    -1,   196,   197,    -1,    -1,    -1,    -1,  1657,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1668,    -1,    -1,    -1,    -1,    -1,  1674,    -1,    -1,    -1,
    1678,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
    1738,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,   179,    -1,   181,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,   179,    -1,
     181,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    99,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    97,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    95,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   166,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
     106,   107,   108,   109,    -1,   111,   112,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,   106,   107,   108,   109,    -1,
     111,   112,    -1,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,   167,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,    -1,   191,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,   167,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    10,    11,    12,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    68,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,   106,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    10,    11,    12,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    68,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,
      11,    12,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    68,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,   190,    -1,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    10,    11,
      12,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    68,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   190,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,   189,    -1,    -1,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      31,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    10,    11,
      12,    -1,   193,   194,    -1,   196,   197,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    68,    -1,    -1,    55,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,   172,    -1,   190,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,    10,    11,    12,    -1,   193,   194,    -1,
     196,   197,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      -1,    68,    -1,    -1,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
      -1,   172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   194,    -1,   196,   197,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    30,    29,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,    -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    -1,   171,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,   191,    -1,   193,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,    -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    30,   171,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,   191,    -1,   193,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,   192,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,   192,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,   128,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,    -1,
      -1,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    27,   168,    29,    30,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   178,   179,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,   131,    -1,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
     166,    -1,   168,    -1,    -1,   171,     3,     4,     5,     6,
       7,    -1,   178,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,   156,
      -1,    -1,    -1,   160,   161,    -1,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,    10,    11,    12,    -1,    -1,
      -1,   178,   179,    -1,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    68,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    -1,    -1,    -1,   190,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    70,    71,    72,    73,    74,    75,    76,    -1,
      -1,   189,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   189,    -1,
     128,   129,   130,   131,    -1,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,    -1,
      -1,    -1,   160,   161,    -1,   163,   164,   165,   166,    27,
     168,    29,    -1,   171,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
     128,   129,   130,   131,    -1,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,    -1,
      -1,    -1,   160,   161,    -1,   163,   164,   165,   166,    27,
     168,    29,    -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,   131,    31,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,   154,    -1,    55,    -1,
      57,    -1,   160,   161,    -1,   163,   164,   165,   166,    -1,
     168,    -1,    69,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    37,    56,    -1,    -1,    -1,    -1,   136,
      -1,   138,   139,   140,   141,   142,    68,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    69,   163,   164,   165,    -1,
      -1,    -1,   169,    77,    78,    79,    80,    -1,    82,    -1,
      -1,   178,    -1,    -1,    -1,    89,   183,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,   165,    49,    50,    -1,   169,    -1,    -1,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    69,    -1,   188,    -1,    -1,    -1,    -1,   193,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,   136,
      -1,   138,   139,   140,   141,   142,    68,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    69,   163,   164,   165,    -1,
      -1,    -1,   169,    77,    78,    79,    80,    -1,    82,    -1,
      -1,   178,    -1,    -1,    -1,    89,   183,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    69,   163,
     164,   165,    -1,    -1,    -1,   169,    77,    78,    79,    80,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,   183,
      -1,    -1,    -1,    -1,   188,    -1,    -1,   191,    -1,   193,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    69,    -1,    71,   169,    -1,
      -1,    -1,    -1,    77,    78,    79,    80,    -1,    82,    -1,
      -1,    -1,   183,    -1,    -1,    89,    -1,   188,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    69,   163,
     164,   165,    -1,    -1,    -1,   169,    77,    78,    79,    80,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,   183,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,   193,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    69,   163,   164,   165,    -1,    -1,    -1,   169,    77,
      78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    89,   183,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,   101,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,   157,
     158,    -1,   160,   161,    -1,   163,   164,   165,    -1,    -1,
      -1,   169,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,    -1,    -1,    -1,    30,   193,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,   134,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,   134,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,   134,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    77,    78,    79,
      80,    -1,    82,    -1,    -1,    -1,    -1,    68,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,   134,   154,   155,   156,   157,   158,    -1,
     160,   161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,    -1,
      28,    -1,    30,   193,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     425,   426,   427,   441,   443,   445,   120,   121,   122,   135,
     154,   164,   188,   205,   238,   319,   340,   418,   340,   188,
     340,   340,   340,   106,   340,   340,   340,   404,   405,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,    82,    89,   122,   149,   188,   216,   359,   376,   379,
     384,   418,   421,   418,    37,   340,   432,   433,   340,   122,
     128,   188,   216,   251,   376,   377,   378,   380,   384,   415,
     416,   417,   425,   429,   430,   188,   329,   381,   188,   329,
     350,   330,   340,   224,   329,   188,   188,   188,   329,   190,
     340,   205,   190,   340,     3,     4,     6,     7,    10,    11,
      12,    13,    27,    29,    30,    56,    58,    70,    71,    72,
      73,    74,    75,    76,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   128,   129,
     130,   131,   135,   136,   137,   154,   158,   166,   168,   171,
     178,   188,   205,   206,   207,   218,   446,   461,   462,   464,
     190,   335,   337,   340,   191,   231,   340,   109,   110,   157,
     208,   209,   210,   211,   215,    82,   193,   285,   286,   121,
     128,   120,   128,    82,   287,   188,   188,   188,   188,   205,
     257,   449,   188,   188,   330,    82,    88,   150,   151,   152,
     438,   439,   157,   191,   215,   215,   205,   258,   449,   158,
     188,   449,   449,    82,   185,   191,   351,    27,   328,   332,
     340,   341,   418,   422,   220,   191,   427,    88,   382,   438,
      88,   438,   438,    31,   157,   174,   450,   188,     9,   190,
      37,   237,   158,   256,   449,   122,   184,   238,   320,   190,
     190,   190,   190,   190,   190,   190,   190,    10,    11,    12,
      30,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    56,    68,   190,    69,    69,   191,
     153,   129,   164,   166,   179,   181,   259,   318,   319,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    66,    67,   132,   133,   408,    69,   191,   413,
     188,   188,    69,   191,   188,   237,   238,    14,   340,   190,
     134,    47,   205,   403,    88,   328,   341,   153,   418,   134,
     195,     9,   389,   252,   328,   341,   418,   450,   153,   188,
     383,   408,   413,   189,   340,    31,   222,     8,   352,     9,
     190,   222,   223,   330,   331,   340,   205,   271,   226,   190,
     190,   190,   136,   137,   464,   464,   174,   188,   109,   464,
      14,   153,   136,   137,   154,   205,   207,   190,   190,   190,
     232,   113,   171,   190,   208,   210,   208,   210,   215,   191,
       9,   390,   190,   100,   157,   191,   418,     9,   190,   128,
     128,    14,     9,   190,   418,   442,   330,   328,   341,   418,
     421,   422,   189,   174,   249,   135,   418,   431,   432,   190,
      69,   408,   150,   439,    81,   340,   418,    88,   150,   439,
     215,   204,   190,   191,   244,   254,   366,   368,    89,   188,
     353,   354,   356,   379,   424,   426,   443,    14,   100,   444,
     347,   348,   349,   281,   282,   406,   407,   189,   189,   189,
     189,   189,   192,   221,   222,   239,   246,   253,   406,   340,
     194,   196,   197,   205,   451,   452,   464,    37,   167,   283,
     284,   340,   446,   188,   449,   247,   237,   340,   340,   340,
      31,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   380,   340,   340,   428,   428,
     340,   434,   435,   128,   191,   206,   207,   427,   257,   205,
     258,   449,   449,   256,   238,    37,   332,   335,   337,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   158,   191,   205,   409,   410,   411,   412,   427,
     428,   340,   283,   283,   428,   340,   431,   237,   189,   340,
     188,   402,     9,   389,   189,   189,    37,   340,    37,   340,
     383,   189,   189,   189,   425,   426,   427,   283,   191,   205,
     409,   410,   427,   189,   220,   275,   191,   337,   340,   340,
      92,    31,   222,   269,   190,    28,   100,    14,     9,   189,
      31,   191,   272,   464,    30,    89,   218,   458,   459,   460,
     188,     9,    49,    50,    55,    57,    69,   136,   158,   178,
     188,   216,   218,   361,   376,   384,   385,   386,   205,   463,
     220,   188,   230,   191,   190,   191,   190,   100,   157,   109,
     110,   157,   211,   212,   213,   214,   215,   211,   205,   340,
     286,   385,    82,     9,   189,   189,   189,   189,   189,   189,
     189,   190,    49,    50,   456,   457,   130,   262,   188,     9,
     189,   189,    82,    83,   205,   440,   205,    69,   192,   192,
     201,   203,    31,   131,   261,   173,    53,   158,   173,   370,
     341,   134,     9,   389,   189,   153,   464,   464,    14,   352,
     281,   220,   186,     9,   390,   464,   465,   408,   413,   408,
     192,     9,   389,   175,   418,   340,   189,     9,   390,    14,
     344,   240,   130,   260,   188,   449,   340,    31,   195,   195,
     134,   192,     9,   389,   340,   450,   188,   250,   245,   255,
      14,   444,   248,   237,    71,   418,   340,   450,   195,   192,
     189,   189,   195,   192,   189,    49,    50,    69,    77,    78,
      79,    89,   136,   149,   178,   205,   392,   394,   395,   398,
     401,   205,   418,   418,   134,   260,   408,   413,   189,   340,
     276,    74,    75,   277,   220,   329,   220,   331,   100,    37,
     135,   266,   418,   385,   205,    31,   222,   270,   190,   273,
     190,   273,     9,   175,    89,   134,   153,     9,   389,   189,
     167,   451,   452,   453,   451,   385,   385,   385,   385,   385,
     388,   391,   188,   153,   188,   385,   153,   191,    10,    11,
      12,    30,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    68,   153,   450,   192,   376,   191,
     234,   210,   210,   205,   211,   211,   215,     9,   390,   192,
     192,    14,   418,   190,     9,   175,   205,   263,   376,   191,
     431,   135,   418,    14,   195,   340,   192,   201,   464,   263,
     191,   369,    14,   189,   340,   353,   427,   190,   464,   186,
     192,    31,   454,   407,    37,    82,   167,   409,   410,   412,
     409,   410,   464,    37,   167,   340,   385,   281,   188,   376,
     261,   345,   241,   340,   340,   340,   192,   188,   283,   262,
      31,   261,   464,    14,   260,   449,   380,   192,   188,    14,
      77,    78,    79,   205,   393,   393,   395,   396,   397,    51,
     188,    88,   150,   188,     9,   389,   189,   402,    37,   340,
     261,   192,    74,    75,   278,   329,   222,   192,   190,    93,
     190,   266,   418,   188,   134,   265,    14,   220,   273,   103,
     104,   105,   273,   192,   464,   134,   464,   205,   458,     9,
     189,   389,   134,   195,     9,   389,   388,   206,   353,   355,
     357,   189,   128,   206,   385,   436,   437,   385,   385,   385,
      31,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   463,    82,   235,   192,   192,
     214,   190,   385,   457,   100,   101,   455,     9,   291,   189,
     188,   332,   337,   340,   195,   192,   444,   291,   159,   172,
     191,   365,   372,   159,   191,   371,   134,   190,   454,   464,
     352,   465,    82,   167,    14,    82,   450,   418,   340,   189,
     281,   191,   281,   188,   134,   188,   283,   189,   191,   464,
     191,   190,   464,   261,   242,   383,   283,   134,   195,     9,
     389,   394,   396,   150,   353,   399,   400,   395,   418,   191,
     329,    31,    76,   222,   190,   331,   265,   431,   266,   189,
     385,    99,   103,   190,   340,    31,   190,   274,   192,   175,
     464,   134,   167,    31,   189,   385,   385,   189,   134,     9,
     389,   189,   134,   192,     9,   389,   385,    31,   189,   220,
     190,   190,   205,   464,   464,   376,     4,   110,   115,   121,
     123,   160,   161,   163,   192,   292,   317,   318,   319,   324,
     325,   326,   327,   406,   431,   192,   191,   192,    53,   340,
     340,   340,   352,    37,    82,   167,    14,    82,   340,   188,
     454,   189,   291,   189,   281,   340,   283,   189,   291,   444,
     291,   190,   191,   188,   189,   395,   395,   189,   134,   189,
       9,   389,   291,    31,   220,   190,   189,   189,   189,   227,
     190,   190,   274,   220,   464,   464,   134,   385,   353,   385,
     385,   385,   191,   192,   455,   130,   131,   179,   206,   447,
     464,   264,   376,   110,   327,    30,   123,   136,   137,   158,
     164,   301,   302,   303,   304,   376,   162,   309,   310,   126,
     188,   205,   311,   312,   293,   238,   464,     9,   190,     9,
     190,   190,   444,   318,   189,   288,   158,   367,   192,   192,
      82,   167,    14,    82,   340,   283,   115,   342,   454,   192,
     454,   189,   189,   192,   191,   192,   291,   281,   134,   395,
     353,   192,   220,   225,   228,    31,   222,   268,   220,   189,
     385,   134,   134,   220,   376,   376,   449,    14,   206,     9,
     190,   191,   447,   444,   304,   174,   191,     9,   190,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      29,    56,    70,    71,    72,    73,    74,    75,    76,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     135,   136,   138,   139,   140,   141,   142,   154,   155,   156,
     166,   168,   169,   171,   178,   179,   181,   183,   184,   205,
     373,   374,     9,   190,   158,   162,   205,   312,   313,   314,
     190,    82,   323,   237,   294,   447,   447,    14,   238,   192,
     289,   290,   447,    14,    82,   340,   189,   188,   454,   190,
     191,   315,   342,   454,   288,   192,   189,   395,   134,    31,
     222,   267,   268,   220,   385,   385,   192,   190,   190,   385,
     376,   297,   464,   305,   306,   384,   302,    14,    31,    50,
     307,   310,     9,    35,   189,    30,    49,    52,    14,     9,
     190,   207,   448,   323,    14,   464,   237,   190,    14,   340,
      37,    82,   364,   191,   220,   454,   315,   192,   454,   395,
     220,    97,   233,   192,   205,   218,   298,   299,   300,     9,
     175,     9,   389,   192,   385,   374,   374,    58,   308,   313,
     313,    30,    49,    52,   385,    82,   174,   188,   190,   385,
     449,   385,    82,     9,   390,   220,   192,   191,   315,    95,
     190,   113,   229,   153,   100,   464,   384,   165,    14,   456,
     295,   188,    37,    82,   189,   192,   220,   190,   188,   171,
     236,   205,   318,   319,   175,   385,   175,   279,   280,   407,
     296,    82,   192,   376,   234,   168,   205,   190,   189,     9,
     390,   117,   118,   119,   321,   322,   279,    82,   264,   190,
     454,   407,   465,   189,   189,   190,   190,   191,   316,   321,
      37,    82,   167,   454,   191,   220,   465,    82,   167,    14,
      82,   316,   220,   192,    37,    82,   167,    14,    82,   340,
     192,    82,   167,    14,    82,   340,    14,    82,   340,   340
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
#line 2679 "hphp.y"
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
#line 2694 "hphp.y"
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
#line 2829 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2905 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval).reset();;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

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

  case 870:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

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

  case 876:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval).reset();;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 911:

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

  case 912:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    {;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    {;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    {;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3273 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3280 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3289 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3295 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13785 "hphp.7.tab.cpp"
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
#line 3307 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

