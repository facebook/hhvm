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
#define YYLAST   16644

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  198
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  269
/* YYNRULES -- Number of rules.  */
#define YYNRULES  992
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1824

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
    2739,  2741,  2743,  2745,  2747,  2749,  2753,  2757,  2762,  2767,
    2771,  2773,  2775,  2783,  2793,  2801,  2808,  2817,  2819,  2824,
    2829,  2831,  2833,  2838,  2841,  2843,  2844,  2846,  2848,  2850,
    2854,  2858,  2862,  2863,  2865,  2867,  2871,  2875,  2878,  2882,
    2889,  2890,  2892,  2897,  2900,  2901,  2907,  2911,  2915,  2917,
    2924,  2929,  2934,  2937,  2940,  2941,  2947,  2951,  2955,  2957,
    2960,  2961,  2967,  2971,  2975,  2977,  2980,  2983,  2985,  2988,
    2990,  2995,  2999,  3003,  3010,  3014,  3016,  3018,  3020,  3025,
    3030,  3035,  3040,  3045,  3050,  3053,  3056,  3061,  3064,  3067,
    3069,  3073,  3077,  3081,  3082,  3085,  3091,  3098,  3105,  3113,
    3115,  3118,  3120,  3123,  3125,  3130,  3132,  3137,  3141,  3142,
    3144,  3148,  3151,  3155,  3157,  3159,  3160,  3161,  3164,  3167,
    3170,  3173,  3178,  3181,  3187,  3191,  3193,  3195,  3196,  3200,
    3205,  3211,  3215,  3217,  3220,  3221,  3226,  3228,  3232,  3235,
    3238,  3241,  3243,  3245,  3247,  3249,  3253,  3259,  3266,  3268,
    3277,  3284,  3286
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
      -1,    -1,   164,   205,   244,    31,   465,   444,   191,   288,
     192,    -1,    -1,   406,   164,   205,   245,    31,   465,   444,
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
      -1,   280,     9,   407,   321,   466,   167,    82,    -1,   280,
       9,   407,   321,   466,    37,   167,    82,    -1,   280,     9,
     407,   321,   466,   167,    -1,   280,   390,    -1,   407,   321,
     466,   167,    82,    -1,   407,   321,   466,    37,   167,    82,
      -1,   407,   321,   466,   167,    -1,    -1,   407,   321,   466,
      82,    -1,   407,   321,   466,    37,    82,    -1,   407,   321,
     466,    37,    82,    14,   340,    -1,   407,   321,   466,    82,
      14,   340,    -1,   280,     9,   407,   321,   466,    82,    -1,
     280,     9,   407,   321,   466,    37,    82,    -1,   280,     9,
     407,   321,   466,    37,    82,    14,   340,    -1,   280,     9,
     407,   321,   466,    82,    14,   340,    -1,   282,     9,   407,
     466,   167,    82,    -1,   282,     9,   407,   466,    37,   167,
      82,    -1,   282,     9,   407,   466,   167,    -1,   282,   390,
      -1,   407,   466,   167,    82,    -1,   407,   466,    37,   167,
      82,    -1,   407,   466,   167,    -1,    -1,   407,   466,    82,
      -1,   407,   466,    37,    82,    -1,   407,   466,    37,    82,
      14,   340,    -1,   407,   466,    82,    14,   340,    -1,   282,
       9,   407,   466,    82,    -1,   282,     9,   407,   466,    37,
      82,    -1,   282,     9,   407,   466,    37,    82,    14,   340,
      -1,   282,     9,   407,   466,    82,    14,   340,    -1,   284,
     390,    -1,    -1,   340,    -1,    37,   418,    -1,   167,   340,
      -1,   284,     9,   340,    -1,   284,     9,   167,   340,    -1,
     284,     9,    37,   418,    -1,   285,     9,   286,    -1,   286,
      -1,    82,    -1,   193,   418,    -1,   193,   191,   340,   192,
      -1,   287,     9,    82,    -1,   287,     9,    82,    14,   385,
      -1,    82,    -1,    82,    14,   385,    -1,   288,   289,    -1,
      -1,   290,   190,    -1,   447,    14,   385,    -1,   291,   292,
      -1,    -1,    -1,   317,   293,   323,   190,    -1,    -1,   319,
     465,   294,   323,   190,    -1,   324,   190,    -1,   325,   190,
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
     136,    -1,   136,   174,   465,   389,   175,    -1,   136,   174,
     465,     9,   465,   175,    -1,   376,    -1,   123,    -1,   164,
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
      82,    -1,    82,    14,   385,    -1,   324,     9,   447,    14,
     385,    -1,   110,   447,    14,   385,    -1,   325,     9,   447,
      -1,   121,   110,   447,    -1,   121,   327,   444,    -1,   327,
     444,    14,   465,    -1,   110,   179,   449,    -1,   188,   328,
     189,    -1,    71,   380,   383,    -1,    71,   251,    -1,    70,
     340,    -1,   365,    -1,   360,    -1,   188,   340,   189,    -1,
     330,     9,   340,    -1,   340,    -1,   330,    -1,    -1,    27,
      -1,    27,   340,    -1,    27,   340,   134,   340,    -1,   188,
     332,   189,    -1,   418,    14,   332,    -1,   135,   188,   431,
     189,    14,   332,    -1,    29,   340,    -1,   418,    14,   335,
      -1,    28,   340,    -1,   418,    14,   337,    -1,   135,   188,
     431,   189,    14,   337,    -1,   341,    -1,   418,    -1,   328,
      -1,   422,    -1,   421,    -1,   135,   188,   431,   189,    14,
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
      31,   340,    -1,   340,    32,   340,    -1,   441,    -1,    65,
     340,    -1,    64,   340,    -1,    63,   340,    -1,    62,   340,
      -1,    61,   340,    -1,    60,   340,    -1,    59,   340,    -1,
      72,   381,    -1,    58,   340,    -1,   387,    -1,   359,    -1,
     358,    -1,   194,   382,   194,    -1,    13,   340,    -1,   362,
      -1,   115,   188,   364,   390,   189,    -1,    -1,    -1,   238,
     237,   188,   344,   281,   189,   454,   342,   454,   191,   220,
     192,    -1,    -1,   319,   238,   237,   188,   345,   281,   189,
     454,   342,   454,   191,   220,   192,    -1,    -1,   184,    82,
     347,   352,    -1,    -1,   184,   185,   348,   281,   186,   454,
     352,    -1,    -1,   184,   191,   349,   220,   192,    -1,    -1,
      82,   350,   352,    -1,    -1,   185,   351,   281,   186,   454,
     352,    -1,     8,   340,    -1,     8,   337,    -1,     8,   191,
     220,   192,    -1,    89,    -1,   443,    -1,   354,     9,   353,
     134,   340,    -1,   353,   134,   340,    -1,   355,     9,   353,
     134,   385,    -1,   353,   134,   385,    -1,   354,   389,    -1,
      -1,   355,   389,    -1,    -1,   178,   188,   356,   189,    -1,
     136,   188,   432,   189,    -1,    69,   432,   195,    -1,   376,
     191,   434,   192,    -1,   376,   191,   436,   192,    -1,   362,
      69,   428,   195,    -1,   363,    69,   428,   195,    -1,   359,
      -1,   443,    -1,   421,    -1,    89,    -1,   188,   341,   189,
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
     189,    -1,   377,    -1,   378,   153,   427,    -1,   377,    -1,
     424,    -1,   379,   153,   427,    -1,   376,    -1,   122,    -1,
     429,    -1,   188,   189,    -1,   329,    -1,    -1,    -1,    88,
      -1,   438,    -1,   188,   283,   189,    -1,    -1,    77,    -1,
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
     443,    -1,   384,    -1,   196,   438,   196,    -1,   197,   438,
     197,    -1,   149,   438,   150,    -1,   391,   389,    -1,    -1,
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
     158,    -1,   191,   340,   192,    -1,   409,    -1,   427,    -1,
     205,    -1,   191,   340,   192,    -1,   411,    -1,   427,    -1,
      69,   428,   195,    -1,   191,   340,   192,    -1,   419,   413,
      -1,   188,   328,   189,   413,    -1,   430,   413,    -1,   188,
     328,   189,   413,    -1,   188,   328,   189,   408,   410,    -1,
     188,   341,   189,   408,   410,    -1,   188,   328,   189,   408,
     409,    -1,   188,   341,   189,   408,   409,    -1,   425,    -1,
     375,    -1,   423,    -1,   424,    -1,   414,    -1,   416,    -1,
     418,   408,   410,    -1,   379,   153,   427,    -1,   420,   188,
     283,   189,    -1,   421,   188,   283,   189,    -1,   188,   418,
     189,    -1,   375,    -1,   423,    -1,   424,    -1,   414,    -1,
     418,   408,   410,    -1,   417,    -1,   420,   188,   283,   189,
      -1,   188,   418,   189,    -1,   379,   153,   427,    -1,   425,
      -1,   414,    -1,   375,    -1,   359,    -1,   384,    -1,   188,
     418,   189,    -1,   188,   341,   189,    -1,   421,   188,   283,
     189,    -1,   420,   188,   283,   189,    -1,   188,   422,   189,
      -1,   343,    -1,   346,    -1,   418,   408,   412,   450,   188,
     283,   189,    -1,   188,   328,   189,   408,   412,   450,   188,
     283,   189,    -1,   379,   153,   207,   450,   188,   283,   189,
      -1,   379,   153,   427,   188,   283,   189,    -1,   379,   153,
     191,   340,   192,   188,   283,   189,    -1,   426,    -1,   426,
      69,   428,   195,    -1,   426,   191,   340,   192,    -1,   427,
      -1,    82,    -1,   193,   191,   340,   192,    -1,   193,   427,
      -1,   340,    -1,    -1,   425,    -1,   415,    -1,   416,    -1,
     429,   408,   410,    -1,   378,   153,   425,    -1,   188,   418,
     189,    -1,    -1,   415,    -1,   417,    -1,   429,   408,   409,
      -1,   188,   418,   189,    -1,   431,     9,    -1,   431,     9,
     418,    -1,   431,     9,   135,   188,   431,   189,    -1,    -1,
     418,    -1,   135,   188,   431,   189,    -1,   433,   389,    -1,
      -1,   433,     9,   340,   134,   340,    -1,   433,     9,   340,
      -1,   340,   134,   340,    -1,   340,    -1,   433,     9,   340,
     134,    37,   418,    -1,   433,     9,    37,   418,    -1,   340,
     134,    37,   418,    -1,    37,   418,    -1,   435,   389,    -1,
      -1,   435,     9,   340,   134,   340,    -1,   435,     9,   340,
      -1,   340,   134,   340,    -1,   340,    -1,   437,   389,    -1,
      -1,   437,     9,   385,   134,   385,    -1,   437,     9,   385,
      -1,   385,   134,   385,    -1,   385,    -1,   438,   439,    -1,
     438,    88,    -1,   439,    -1,    88,   439,    -1,    82,    -1,
      82,    69,   440,   195,    -1,    82,   408,   205,    -1,   151,
     340,   192,    -1,   151,    81,    69,   340,   195,   192,    -1,
     152,   418,   192,    -1,   205,    -1,    83,    -1,    82,    -1,
     125,   188,   330,   189,    -1,   126,   188,   418,   189,    -1,
     126,   188,   341,   189,    -1,   126,   188,   422,   189,    -1,
     126,   188,   421,   189,    -1,   126,   188,   328,   189,    -1,
       7,   340,    -1,     6,   340,    -1,     5,   188,   340,   189,
      -1,     4,   340,    -1,     3,   340,    -1,   418,    -1,   442,
       9,   418,    -1,   379,   153,   206,    -1,   379,   153,   128,
      -1,    -1,   100,   465,    -1,   179,   449,    14,   465,   190,
      -1,   406,   179,   449,    14,   465,   190,    -1,   181,   449,
     444,    14,   465,   190,    -1,   406,   181,   449,   444,    14,
     465,   190,    -1,   207,    -1,   465,   207,    -1,   206,    -1,
     465,   206,    -1,   207,    -1,   207,   174,   456,   175,    -1,
     205,    -1,   205,   174,   456,   175,    -1,   174,   452,   175,
      -1,    -1,   465,    -1,   451,     9,   465,    -1,   451,   389,
      -1,   451,     9,   167,    -1,   452,    -1,   167,    -1,    -1,
      -1,    31,   465,    -1,   100,   465,    -1,   101,   465,    -1,
     457,   389,    -1,   457,     9,   458,   205,    -1,   458,   205,
      -1,   457,     9,   458,   205,   455,    -1,   458,   205,   455,
      -1,    49,    -1,    50,    -1,    -1,    89,   134,   465,    -1,
      30,    89,   134,   465,    -1,   218,   153,   205,   134,   465,
      -1,   460,     9,   459,    -1,   459,    -1,   460,   389,    -1,
      -1,   178,   188,   461,   189,    -1,   218,    -1,   205,   153,
     464,    -1,   205,   450,    -1,    30,   465,    -1,    58,   465,
      -1,   218,    -1,   136,    -1,   137,    -1,   462,    -1,   463,
     153,   464,    -1,   136,   174,   465,   389,   175,    -1,   136,
     174,   465,     9,   465,   175,    -1,   158,    -1,   188,   109,
     188,   453,   189,    31,   465,   189,    -1,   188,   465,     9,
     451,   389,   189,    -1,   465,    -1,    -1
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
    3179,  3184,  3187,  3189,  3193,  3199,  3200,  3201,  3205,  3209,
    3219,  3227,  3229,  3233,  3235,  3240,  3246,  3249,  3254,  3262,
    3265,  3268,  3269,  3272,  3275,  3276,  3281,  3284,  3288,  3292,
    3298,  3308,  3309
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
     456,   457,   457,   457,   457,   458,   458,   458,   459,   459,
     459,   460,   460,   461,   461,   462,   463,   464,   464,   465,
     465,   465,   465,   465,   465,   465,   465,   465,   465,   465,
     465,   466,   466
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
       2,     4,     2,     5,     3,     1,     1,     0,     3,     4,
       5,     3,     1,     2,     0,     4,     1,     3,     2,     2,
       2,     1,     1,     1,     1,     3,     5,     6,     1,     8,
       6,     1,     0
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
      35,    36,    63,   982,   983,    55,   988,    34,    53,    85,
       0,     0,   112,    95,   939,   981,     0,   984,     0,     0,
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
     163,   141,   982,   983,   979,   980,     0,   974,     0,     0,
       0,     0,    68,    33,    55,    32,   940,   170,   173,   143,
     122,     0,   160,   162,     0,     0,     0,     0,   103,     0,
     751,   101,    18,     0,    97,     0,   305,     0,   145,   218,
     217,     0,     0,   146,   929,     0,     0,   440,   438,   439,
     442,   441,     0,   967,   225,     0,   881,     0,     0,   148,
       0,     0,   695,   908,   741,     0,     0,   906,   746,   905,
     115,     5,    13,    14,     0,   223,     0,     0,   533,     0,
       0,   750,     0,     0,   669,   664,   534,     0,     0,     0,
       0,   798,   122,     0,   752,   797,   992,   416,   430,   494,
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
     251,   251,   137,   750,     0,     0,     0,   972,   750,     0,
     955,     0,     0,     0,     0,     0,   748,     0,   660,     0,
       0,   698,   659,   703,     0,   697,   120,   702,   948,   985,
       0,     0,     0,     0,    19,     0,    20,     0,    98,     0,
       0,     0,   109,   752,     0,   107,   102,    99,   104,     0,
     302,   310,   307,     0,     0,   918,   923,   920,   919,   922,
     921,    12,   965,   966,     0,   750,     0,     0,     0,   880,
     877,     0,   544,   917,   916,   915,     0,   911,     0,   912,
     914,     0,     5,     0,     0,     0,   559,   560,   568,   567,
       0,   438,     0,   749,   539,   543,     0,     0,   934,     0,
     521,     0,     0,   956,   798,   281,   991,     0,     0,   813,
       0,   862,   749,   951,   947,   297,   298,   658,   751,   294,
       0,   798,     0,     0,   223,   518,   189,   496,     0,   548,
     549,     0,   546,   749,   893,     0,     0,   295,   225,     0,
     223,     0,     0,   221,     0,   872,   445,     0,     0,   810,
     811,   828,   829,   858,   859,     0,     0,     0,   777,   757,
     758,   759,   766,     0,     0,     0,   770,   768,   769,   783,
     750,     0,   791,   891,   890,     0,   223,     0,   815,   680,
       0,   261,     0,     0,   128,     0,     0,     0,     0,     0,
       0,     0,   231,   232,   243,     0,   122,   241,   157,   251,
       0,   251,     0,   749,     0,     0,     0,     0,   749,   973,
     975,   954,   750,   953,     0,   750,   724,   725,   722,   723,
     756,     0,   750,   748,     0,   542,     0,     0,   900,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   978,   177,     0,
     180,   161,     0,     0,   105,   110,   111,   103,   751,   108,
       0,   306,     0,   930,   149,   946,   967,   960,   962,   224,
     226,   316,     0,     0,   878,     0,   910,     0,    17,     0,
     933,   222,   316,     0,     0,   665,   536,     0,   670,   935,
       0,   956,   525,     0,     0,   992,     0,   286,   284,   804,
     816,   948,   804,   817,   950,     0,     0,   299,   119,     0,
     798,   220,     0,   798,     0,   495,   897,   896,     0,   295,
       0,     0,     0,     0,     0,     0,   223,   191,   681,   803,
     295,     0,   762,   763,   764,   765,   771,   772,   781,     0,
     750,     0,   777,     0,   761,   785,   749,   788,   790,   792,
       0,   885,     0,   803,     0,     0,     0,     0,   258,   532,
     133,     0,   426,   231,   233,   880,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   245,     0,   986,     0,   968,
       0,   971,   749,     0,     0,     0,   700,   749,   747,     0,
     738,     0,   750,     0,   704,   739,   737,   904,     0,   750,
     707,   709,   708,     0,     0,   705,   706,   710,   712,   711,
     727,   726,   729,   728,   730,   732,   734,   733,   731,   720,
     719,   714,   715,   713,   716,   717,   718,   721,   977,     0,
     122,     0,     0,   106,    21,   308,     0,     0,     0,   964,
       0,   390,   882,   880,   432,   437,   443,     0,    15,     0,
     390,   571,     0,     0,   573,   566,   569,     0,   564,     0,
     937,     0,   957,   529,     0,   287,     0,     0,   282,     0,
     301,   300,   956,     0,   316,     0,   798,     0,   295,     0,
     855,   316,   933,   316,   936,     0,     0,     0,   446,     0,
       0,   774,   749,   776,   767,     0,   760,     0,     0,   750,
     782,   889,   316,     0,   122,     0,   254,   240,     0,     0,
       0,   230,   153,   244,     0,     0,   247,     0,   252,   253,
     122,   246,   987,   969,     0,   952,     0,   990,   755,   754,
     699,     0,   749,   541,   701,     0,   547,   749,   899,   736,
       0,     0,     0,    22,    23,   961,   958,   959,   227,     0,
       0,     0,   397,   388,     0,     0,     0,   202,   315,   317,
       0,   387,     0,     0,     0,   933,   390,     0,   913,   312,
     208,   562,     0,     0,   535,   523,     0,   290,   280,     0,
     283,   289,   295,   515,   956,   390,   956,     0,   895,     0,
     854,   390,     0,   390,   938,   316,   798,   852,   780,   779,
     773,     0,   775,   749,   784,   390,   122,   260,   129,   134,
     155,   234,     0,   242,   248,   122,   250,   970,     0,     0,
     538,     0,   903,   902,   735,   122,   181,   963,     0,     0,
       0,   941,     0,     0,     0,   228,     0,   933,     0,   353,
     349,   355,   660,    31,     0,   343,     0,   348,   352,   365,
       0,   363,   368,     0,   367,     0,   366,     0,   185,   319,
       0,   321,     0,   322,   323,     0,     0,   879,     0,   563,
     561,   572,   570,   291,     0,     0,   278,   288,     0,     0,
     956,     0,   198,   515,   956,   856,   204,   312,   210,   390,
       0,     0,   787,     0,   206,   256,     0,     0,   122,   237,
     154,   249,   989,   753,     0,     0,     0,     0,     0,   415,
       0,   942,     0,   333,   337,   412,   413,   347,     0,     0,
       0,   328,   624,   623,   620,   622,   621,   641,   643,   642,
     612,   582,   584,   583,   602,   618,   617,   578,   589,   590,
     592,   591,   611,   595,   593,   594,   596,   597,   598,   599,
     600,   601,   603,   604,   605,   606,   607,   608,   610,   609,
     579,   580,   581,   585,   586,   588,   626,   627,   636,   635,
     634,   633,   632,   631,   619,   638,   628,   629,   630,   613,
     614,   615,   616,   639,   640,   644,   646,   645,   647,   648,
     625,   650,   649,   652,   654,   653,   587,   657,   655,   656,
     651,   637,   577,   360,   574,     0,   329,   381,   382,   380,
     373,     0,   374,   330,   407,     0,     0,     0,     0,   411,
       0,   185,   194,   311,     0,     0,     0,   279,   293,   853,
       0,     0,   383,   122,   188,   956,     0,     0,   200,   956,
     778,     0,   122,   235,   135,   156,     0,   537,   901,   179,
     331,   332,   410,   229,     0,   750,   750,     0,   356,   344,
       0,     0,     0,   362,   364,     0,     0,   369,   376,   377,
     375,     0,     0,   318,   943,     0,     0,     0,   414,     0,
     313,     0,   292,     0,   557,   752,   122,     0,     0,   190,
     196,     0,   786,     0,     0,   158,   334,   112,     0,   335,
     336,     0,   749,     0,   749,   358,   354,   359,   575,   576,
       0,   345,   378,   379,   371,   372,   370,   408,   405,   967,
     324,   320,   409,     0,   314,   558,   751,     0,     0,   384,
     122,   192,     0,   238,     0,   183,     0,   390,     0,   350,
     357,   361,     0,     0,   798,   326,     0,   555,   514,   517,
       0,   236,     0,     0,   159,   341,     0,   389,   351,   406,
     944,     0,   752,   401,   798,   556,   519,     0,   182,     0,
       0,   340,   956,   798,   265,   402,   403,   404,   992,   400,
       0,     0,     0,   339,     0,   401,     0,   956,     0,   338,
     385,   122,   325,   992,     0,   270,   268,     0,   122,     0,
       0,   271,     0,     0,   266,   327,     0,   386,     0,   274,
     264,     0,   267,   273,   178,   275,     0,     0,   262,   272,
       0,   263,   277,   276
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   108,   872,   611,   173,  1431,   706,
     340,   341,   342,   343,   833,   834,   835,   110,   111,   112,
     113,   114,   394,   643,   644,   532,   243,  1496,   538,  1412,
    1497,  1735,   822,   335,   560,  1695,  1051,  1230,  1754,   411,
     174,   645,   912,  1114,  1287,   118,   614,   929,   646,   665,
     933,   594,   928,   223,   513,   647,   615,   930,   413,   360,
     377,   121,   914,   875,   858,  1069,  1434,  1167,   982,  1644,
    1500,   783,   988,   537,   792,   990,  1320,   775,   971,   974,
    1156,  1761,  1762,   633,   634,   659,   660,   347,   348,   354,
    1468,  1623,  1624,  1241,  1358,  1457,  1617,  1744,  1764,  1654,
    1699,  1700,  1701,  1444,  1445,  1446,  1447,  1656,  1657,  1663,
    1711,  1450,  1451,  1455,  1610,  1611,  1612,  1634,  1792,  1359,
    1360,   175,   123,  1778,  1779,  1615,  1362,  1363,  1364,  1365,
     124,   236,   533,   534,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,  1480,   135,   911,  1113,   136,   630,
     631,   632,   240,   386,   528,   620,   621,  1192,   622,  1193,
     137,   138,   139,   813,   140,   141,  1685,   142,   616,  1470,
     617,  1083,   880,  1258,  1255,  1603,  1604,   143,   144,   145,
     226,   146,   227,   237,   398,   520,   147,  1010,   817,   148,
    1011,   903,   571,  1012,   957,  1136,   958,  1138,  1139,  1140,
     960,  1298,  1299,   961,   751,   503,   187,   188,   648,   636,
     486,  1099,  1100,   737,   738,   899,   150,   229,   151,   152,
     177,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     698,   233,   234,   597,   216,   217,   701,   702,  1198,  1199,
     370,   371,   866,   163,   585,   164,   629,   165,   326,  1625,
    1675,   361,   406,   654,   655,  1004,  1094,  1239,   854,   855,
     856,   797,   798,   799,   327,   328,   819,  1433,   897
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1471
static const yytype_int16 yypact[] =
{
   -1471,   183, -1471, -1471,  5573, 13178, 13178,     0, 13178, 13178,
   13178, 11033, 13178, 13178, -1471, 13178, 13178, 13178, 13178, 13178,
   13178, 13178, 13178, 13178, 13178, 13178, 13178, 15676, 15676, 11228,
   13178,  5009,     7,    48, -1471, -1471, -1471, -1471, -1471,   200,
   -1471, -1471,   158, 13178, -1471,    48,   198,   203,   218, -1471,
      48, 11423,  1573, 11618, -1471, 14128, 10058,   219, 13178,   914,
      52, -1471, -1471, -1471,   245,   470,    64,   224,   341,   362,
     372, -1471,  1573,   407,   413, -1471, -1471, -1471, -1471, -1471,
   13178,    72,  1016, -1471, -1471,  1573, -1471, -1471, -1471, -1471,
    1573, -1471,  1573, -1471,   152,   417,  1573,  1573, -1471,   421,
   -1471, 11813, -1471, -1471,    66,   554,   635,   635, -1471,   357,
     259,    15,   442, -1471,    86, -1471,   602, -1471, -1471, -1471,
   -1471,  1475,  1148, -1471, -1471,   447,   451,   457,   462,   464,
     511,   532,   541,  5028, -1471, -1471, -1471, -1471,   157, -1471,
     607,   625, -1471,   130,   544, -1471,   603,    19, -1471,  2478,
     156, -1471, -1471,  2426,   149,   573,   340, -1471,   159,   202,
     577,   228, -1471, -1471,   704, -1471, -1471, -1471,   650,   616,
     649, -1471, 13178, -1471,   602,  1148, 16354,  3309, 16354, 13178,
   16354, 16354, 13566,   617, 15375, 13566, 16354,   763,  1573,   748,
     748,   129,   748,   748,   748,   748,   748,   748,   748,   748,
     748, -1471, -1471, -1471,    96, 13178,   645, -1471, -1471,   669,
     641,   530,   642,   530, 15676, 15838,   630,   824, -1471,   650,
   -1471, 13178,   645, -1471,   683, -1471,   684,   652, -1471,   170,
   -1471, -1471, -1471,   530,   149, 12008, -1471, -1471, 13178,  8888,
     830,    98, 16354,  9863, -1471, 13178, 13178,  1573, -1471, -1471,
   11017,   654, -1471, 11407, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471,  3118, -1471,  3118, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471,    88,    90,   649, -1471, -1471, -1471, -1471,
     658,  2055,    92, -1471, -1471,   695,   835, -1471,   703, 14802,
   -1471,   671,   678, 11992, -1471,   384, 13162,   773,   773,  1573,
     682,   866,   686, -1471,    53, -1471,  3949,    99, -1471,   751,
   -1471,   753, -1471,   869,   108, 15676, 13178, 13178,   697,   710,
   -1471, -1471, 15393, 11228,   111,    87,   662, -1471, 13373, 15676,
     600, -1471,  1573, -1471,   382,   259, -1471, -1471, -1471, -1471,
    3591,   874,   789, -1471, -1471, -1471,    93, 13178,   702,   709,
   16354,   717,  1277,   718,  5768, 13178, -1471,   482,   698,   680,
     482,   504,   448, -1471,  1573,  3118,   711, 10253, 14128, -1471,
   -1471,  1229, -1471, -1471, -1471, -1471, -1471,   602, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, 13178, 13178, 13178,
   12203, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178,
   13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178,
   13178, 13178, 13178, 13178, 16074, 13178, -1471, 13178, 13178, 13178,
   13568,  1573,  1573,  1573,  1573,  1573,  1475,   799,  1204,  4831,
   13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178, 13178,
   13178, 13178, -1471, -1471, -1471, -1471,   694, 13178, 13178, -1471,
   10253, 10253, 13178, 13178, 15393,   723,   602, 12398, 13357, -1471,
   13178, -1471,   726,   908,   769,   732,   734, 13708,   530, 12593,
   -1471, 12788, -1471,   652,   736,   737,  2240, -1471,    70, 10253,
   -1471,  1176, -1471, -1471, 14739, -1471, -1471, 10448, -1471, 13178,
   -1471,   836,  9083,   920,   745, 16236,   928,    94,   113, -1471,
   -1471, -1471,   770, -1471, -1471, -1471,  3118,   507,   765,   946,
   15300,  1573, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471,   768, -1471, -1471,   767,   772,   774,   776,   260,  1731,
    1965, -1471, -1471,  1573,  1573, 13178,   530,    52, -1471, -1471,
   -1471, 15300,   881, -1471,   530,   126,   128,   778,   779,  2657,
      23,   780,   785,   631,   847,   791,   530,   131,   792, -1471,
    1393,  1573, -1471, -1471,   911,  2774,    37, -1471, -1471, -1471,
     259, -1471, -1471, -1471,   951,   854,   813,   367,   834, 13178,
     856,   982,   809,   848, -1471,   179, -1471,  3118,  3118,   986,
     830,    93, -1471,   817,   999, -1471,  3118,   174, -1471,   557,
     212, -1471, -1471, -1471, -1471, -1471, -1471, -1471,  1988,  2850,
   -1471, -1471, -1471, -1471,  1002,   837, -1471, 15676, 13178,   831,
    1018, 16354,  1011, -1471, -1471,   898,  1251, 11603, 16533, 13566,
   13178, 16307, 14800, 10233, 11207, 12571,  4668, 12179, 12959, 12959,
   12959, 12959,  2660,  2660,  2660,  2660,  2660,  2335,  2335,   500,
     500,   500,   129,   129,   129, -1471,   748, 16354,   838,   840,
   15885,   850,  1023,    -9, 13178,   443,   645,   166, -1471, -1471,
   -1471,  1030,   789, -1471,   602, 15490, -1471, -1471, -1471, 13566,
   13566, 13566, 13566, 13566, 13566, 13566, 13566, 13566, 13566, 13566,
   13566, 13566, -1471, 13178,   461, -1471,   293, -1471,   645,   472,
     851,  2944,   858,   859,   857,  3035,   136,   863, -1471, 16354,
    2199, -1471,  1573, -1471,   174,   456, 15676, 16354, 15676, 15944,
     898,   174,   530,   305, -1471,   179,   900,   865, 13178, -1471,
     314, -1471, -1471, -1471,  8693,   609, -1471, -1471, 16354, 16354,
      48, -1471, -1471, -1471, 13178,   958, 15183, 15300,  1573,  9278,
     871,   872, -1471,  1056,   984,   942,   927, -1471,  1075,   896,
    1910,  3118, 15300, 15300, 15300, 15300, 15300,   910,   947,   915,
   15300,    24,   957, -1471,   922, -1471, 16446, -1471,    20, -1471,
    5963,  1747,   924,  1965, -1471,  1965, -1471,  1573,  1573,  1965,
    1965,  1573, -1471,  1102,   929, -1471,   355, -1471, -1471,  3423,
   -1471, 16446,  1105, 15676,   926, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471,   945,  1115,  1573,  1747,   936, 15393,
   15583,  1114, -1471, -1471, -1471, -1471,   935, -1471, 13178, -1471,
   -1471,  5177, -1471,  3118,  1747,   940, -1471, -1471, -1471, -1471,
    1118,   948, 13178,  3591, -1471, -1471, 13568,   949, -1471,  3118,
   -1471,   950,  6158,  1107,   114, -1471, -1471,    77,   694, -1471,
    1176, -1471,  3118, -1471, -1471,   530, 16354, -1471, 10643, -1471,
   15300,    45,   952,  1747,   854, -1471, -1471, 14800, 13178, -1471,
   -1471, 13178, -1471, 13178, -1471,  3599,   954, 10253,   847,  1113,
     854,  3118,  1131,   898,  1573, 16074,   530,  3807,   960, -1471,
   -1471,   269,   961, -1471, -1471,  1136,  2260,  2260,  2199, -1471,
   -1471, -1471,  1100,   965,   271,   966, -1471, -1471, -1471, -1471,
    1146,   967,   726,   530,   530, 12983,   854,  1176, -1471, -1471,
    3933,   634,    48,  9863, -1471,  6353,   968,  6548,   969, 15183,
   15676,   973,  1028,   530, 16446,  1149, -1471, -1471, -1471, -1471,
     637, -1471,   267,  3118,   989,  1031,  3118,  1573,   507, -1471,
   -1471, -1471,  1157, -1471,   980,  1002,   644,   644,  1103,  1103,
    4336,   983,  1166, 15300, 15082,  3591,  2279, 14942, 15300, 15300,
   15300, 15300,  4516, 15300, 15300, 15300, 15300, 15300, 15300, 15300,
   15300, 15300, 15300, 15300, 15300, 15300, 15300, 15300, 15300, 15300,
   15300, 15300, 15300, 15300, 15300, 15300,  1573, -1471, -1471,  1101,
   -1471, -1471,   996,   997, -1471, -1471, -1471,   361,  1731, -1471,
    1003, -1471, 15300,   530, -1471, -1471,    63, -1471,   104,  1185,
   -1471, -1471,   140,  1007,   530, 10838, -1471,  2500, -1471,  5378,
     789,  1185, -1471,   422,     2, -1471, 16354,  1062,  1009, -1471,
    1012,  1107, -1471,  3118,   830,  3118,    65,  1187,  1117,   330,
   -1471,   645,   336, -1471, -1471, 15676, 13178, 16354, 16446,  1017,
      45, -1471,  1014,    45,  1022, 14800, 16354, 15991,  1024, 10253,
    1025,  1020,  3118,  1026,  1029,  3118,   854, -1471,   652,   485,
   10253, 13178, -1471, -1471, -1471, -1471, -1471, -1471,  1079,  1021,
    1213,  1134,  2199,  1083, -1471,  3591,  2199, -1471, -1471, -1471,
   15676, 16354,  1038, -1471,    48,  1203,  1159,  9863, -1471, -1471,
   -1471,  1047, 13178,  1028,   530, 15393, 15183,  1050, 15300,  6743,
     643,  1055, 13178,    74,   310, -1471,  1065, -1471,  3118, -1471,
    1112, -1471,  3074,  1216,  1060, 15300, -1471, 15300, -1471,  1061,
   -1471,  1119,  1242,  1066, -1471, -1471, -1471,  4406,  1069,  1254,
   16490, 16576, 14578, 15300, 16401, 10623, 12377, 12766,  3468,  3553,
   13702, 13702, 13702, 13702,  2805,  2805,  2805,  2805,  2805,  1769,
    1769,   644,   644,   644,  1103,  1103,  1103,  1103, -1471,  1082,
   -1471,  1074,  1089, -1471, -1471, 16446,  1573,  3118,  3118, -1471,
    1747,   132, -1471, 15393, -1471, -1471, 13566,  1088, -1471,  1090,
    1401, -1471,   102, 13178, -1471, -1471, -1471, 13178, -1471, 13178,
   -1471,   830, -1471, -1471,   120,  1269,  1202, 13178, -1471,  1098,
     530, 16354,  1107,  1116, -1471,  1121,    45, 13178, 10253,  1122,
   -1471, -1471,   789, -1471, -1471,  1097,  1099,  1120, -1471,  1123,
    2199, -1471,  2199, -1471, -1471,  1127, -1471,  1173,  1128,  1309,
   -1471,   530, -1471,  1296, -1471,  1143, -1471, -1471,  1158,  1161,
     141, -1471, -1471, 16446,  1145,  1163, -1471,  4774, -1471, -1471,
   -1471, -1471, -1471, -1471,  3118, -1471,  3118, -1471, 16446, 16046,
   -1471, 15300,  3591, -1471, -1471, 15300, -1471, 15300, -1471, 14941,
   15300,  1147,  6938, -1471, -1471,   104, -1471, -1471, -1471,   589,
   14268,  1747,  1238, -1471,  1067,  1189,   606, -1471, -1471, -1471,
     799,   663,   112,   115,  1164,   789,  1204,   142, -1471, -1471,
   -1471,  1200,  4142,  4240, 16354, -1471,   322,  1345,  1278, 13178,
   -1471, 16354, 10253,  1247,  1107,  1608,  1107,  1174, 16354,  1175,
   -1471,  1625,  1179,  1641, -1471, -1471,    45, -1471, -1471,  1231,
   -1471,  2199, -1471,  3591, -1471,  1902, -1471,  8693, -1471, -1471,
   -1471, -1471,  9473, -1471, -1471, -1471,  8693, -1471,  1182, 15300,
   16446,  1239, 16446, 16093, 14941, -1471, -1471, -1471,  1747,  1747,
    1573, -1471,  1361, 15082,    79, -1471, 14268,   789,  3403, -1471,
    1206, -1471,   118,  1191,   119, -1471, 14577, -1471, -1471, -1471,
     121, -1471, -1471,   930, -1471,  1186, -1471,  1301,   602, -1471,
   14408, -1471, 14408, -1471, -1471,  1371,   799, -1471, 13848, -1471,
   -1471, -1471, -1471,  1373,  1313, 13178, -1471, 16354,  1207,  1210,
    1107,   536, -1471,  1247,  1107, -1471, -1471, -1471, -1471,  1919,
    1211,  2199, -1471,  1257, -1471,  8693,  9668,  9473, -1471, -1471,
   -1471,  8693, -1471, 16446, 15300, 15300,  7133,  1209,  1212, -1471,
   15300, -1471,  1747, -1471, -1471, -1471, -1471, -1471,  3118,  1541,
    1067, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471,   543, -1471,  1189, -1471, -1471, -1471, -1471,
   -1471,    76,   359, -1471,  1387,   122, 14802,  1301,  1390, -1471,
    3118,   602, -1471, -1471,  1217,  1392, 13178, -1471, 16354, -1471,
     127,  1227, -1471, -1471, -1471,  1107,   536, 13988, -1471,  1107,
   -1471,  2199, -1471, -1471, -1471, -1471,  7328, 16446, 16446, -1471,
   -1471, -1471, 16446, -1471,  1380,  1410,  1412,  1230, -1471, -1471,
   15300, 14577, 14577,  1366, -1471,   930,   930,   572, -1471, -1471,
   -1471, 15300,  1343, -1471,  1252,  1243,   123, 15300, -1471,  1573,
   -1471, 15300, 16354,  1348, -1471,  1418, -1471,  7523,  1244, -1471,
   -1471,   536, -1471,  7718,  1246,  1324, -1471,  1340,  1289, -1471,
   -1471,  1350,  3118,  1268,  1541, -1471, -1471, 16446, -1471, -1471,
    1280, -1471,  1413, -1471, -1471, -1471, -1471, 16446,  1437,   631,
   -1471, -1471, 16446,  1264, 16446, -1471,   319,  1266,  7913, -1471,
   -1471, -1471,  1267, -1471,  1270,  1285,  1573,  1204,  1290, -1471,
   -1471, -1471, 15300,  1292,    67, -1471,  1382, -1471, -1471, -1471,
    8108, -1471,  1747,   924, -1471,  1300,  1573,  1746, -1471, 16446,
   -1471,  1282,  1460,   636,    67, -1471, -1471,  1397, -1471,  1747,
    1294, -1471,  1107,   110, -1471, -1471, -1471, -1471,  3118, -1471,
    1283,  1291,   124, -1471,   547,   636,   155,  1107,  1298, -1471,
   -1471, -1471, -1471,  3118,   449,  1468,  1405,   547, -1471,  8303,
     176,  1477,  1411, 13178, -1471, -1471,  8498, -1471,   452,  1478,
    1415, 13178, -1471, 16354, -1471,  1481,  1420, 13178, -1471, 16354,
   13178, -1471, 16354, 16354
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1471, -1471, -1471,  -531, -1471, -1471, -1471,   486,    55,   -33,
     440, -1471,  -256,  -493, -1471, -1471,   454,    41,  1596, -1471,
    2897, -1471,  -442, -1471,    29, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471,  -246, -1471, -1471,  -143,
     210,    25, -1471, -1471, -1471, -1471, -1471, -1471,    30, -1471,
   -1471, -1471, -1471, -1471, -1471,    33, -1471, -1471,  1043,  1068,
    1054,   -89,  -666,  -841,   599,   656,  -241,   370,  -889, -1471,
      38, -1471, -1471, -1471, -1471,  -705,   221, -1471, -1471, -1471,
   -1471,  -228, -1471,  -562, -1471,  -428, -1471, -1471,   972, -1471,
      73, -1471, -1471, -1008, -1471, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471,    22, -1471,   133, -1471, -1471, -1471,
   -1471, -1471,   -59, -1471,   194,  -873, -1471, -1470,  -231, -1471,
    -140,   197,  -120,  -234, -1471,   -50, -1471, -1471, -1471,   216,
     -30,    16,    36,  -706,   -71, -1471, -1471,     9, -1471,   -20,
   -1471, -1471,    -5,   -34,    91, -1471, -1471, -1471, -1471, -1471,
   -1471, -1471, -1471, -1471,  -574,  -819, -1471, -1471, -1471, -1471,
   -1471,   349, -1471, -1471, -1471, -1471, -1471,   490, -1471, -1471,
   -1471, -1471, -1471, -1471, -1471, -1471,  -864, -1471,  2387,     4,
   -1471,   263,  -386, -1471, -1471,  -459,  3458,  3719, -1471, -1471,
     556,  -170,  -602, -1471, -1471,   628,   436,  -652,   437, -1471,
   -1471, -1471, -1471, -1471,   619, -1471, -1471, -1471,   105,  -849,
    -104,  -402,  -399, -1471,   689,   -95, -1471, -1471,    26,    39,
     688, -1471, -1471,   154,   -29, -1471,  -337,     8,  -340,    59,
    -123, -1471, -1471,  -452,  1215, -1471, -1471, -1471, -1471, -1471,
     653,   639, -1471, -1471, -1471,  -336,  -662, -1471,  1178,  -833,
   -1471,   -69,  -196,     1,   784, -1471,  -905,   244,  -128, -1471,
     533,   598, -1471, -1471, -1471, -1471,   552,   877, -1054
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -977
static const yytype_int16 yytable[] =
{
     176,   178,   418,   180,   181,   182,   184,   185,   186,   467,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   324,   378,   215,   218,   517,   381,   382,   117,
     389,   495,   895,   115,   119,   225,   332,   120,   242,   232,
     625,  1264,   746,   624,   626,  1095,   250,   512,   253,   239,
     932,   333,   414,   336,   760,   418,   890,   230,   909,   489,
     466,   244,   742,   743,  1087,   331,   248,   391,   695,   891,
     231,   388,   393,  1112,  1250,   242,   832,   837,   978,   241,
     871,   565,   567,   774,   735,  1665,   992,   736,  1512,  1123,
    1163,   767,  -552,    14,   966,   408,   390,   -68,   959,   -33,
     344,   -32,   -68,   788,   -33,  1318,   -32,   529,   577,   149,
     323,  1666,   852,   853,  1096,    14,   364,   582,   820,   770,
     529,  1460,   771,   374,  1462,  1152,   375,  -346,  1520,   521,
    1605,  1672,  1672,  1512,   345,   843,  1349,   529,   391,   522,
     860,    14,   388,   393,   790,   860,   353,  1265,   201,   860,
     860,   860,   201,   573,   365,  1371,   600,  1376,    14,  1097,
     366,  1256,    14,   396,  1683,   -96,  1689,   390,  -948,   484,
     485,   506,  -662,  1046,   498,   505,   393,  -948,   765,   -96,
      14,   212,   212,     3,   504,   454,  1261,   515,   179,   405,
     892,   514,  1794,  1257,   405,   235,  1191,   455,   405,  -831,
     390,   122,  1377,  -948,  1237,  1238,  -948,  -844,  -526,  1684,
     574,   491,   849,  1808,   116,  -948,   390,  -663,   487,   484,
     485,  1731,   367,   368,   369,  -834,  -550,  -553,  -832,   870,
     524,  1059,  1266,   524,  -285,  -839,   238,  1795,  -749,  -873,
     242,   535,  1350,   487,  1098,   346,   367,  1351,   492,    61,
      62,    63,   166,  1352,   415,  1353,  -269,   395,  1809,   104,
     379,   601,   546,   104,  1319,  1667,  1385,  1126,   526,  1513,
    1514,  -833,   531,  1391,   666,  1393,   409,  1311,   -68,  -285,
     -33,  -838,   -32,   789,  1170,  1286,  1174,  1378,   530,   578,
     209,   209,  1354,  1355,  1405,  1356,   556,   492,   583,  -751,
    -751,   599,  1461,  -751,   791,  1463,   484,   485,  -346,  1521,
     379,  1606,  1673,  1721,  1789,   844,   416,   845,  -842,  -670,
     861,  -831,  1796,   588,  1357,   945,  1297,   587,   591,  1242,
    1411,  1467,   417,   753,   975,   699,  1055,  1056,  -837,   977,
     488,  1079,   664,  1810,  -841,  -843,   468,  -834,   418,  1109,
    -832,   242,   390,   747,   927,  -669,  1746,  -839,   215,  1143,
     827,  -873,  -835,   605,   740,   488,   349,  1383,   212,   744,
     493,  1172,  1173,   350,  -876,   324,   207,   207,   344,   344,
     568,  -664,   184,  -875,   323,   496,   245,  1489,   403,  1668,
     649,   246,   586,  -833,   378,   711,   712,   414,   716,  -818,
    -845,  1747,   661,  -838,  1473,  -819,   247,  1072,  1669,  -552,
     334,  1670,   355,   610,  1172,  1173,   404,   828,  1249,   493,
     877,  1144,   667,   668,   669,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   718,
     696,   884,   697,   697,   700,   573,  1308,  -848,   225,  1175,
    -837,   827,   232,   323,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,   730,   731,   209,   717,  1481,
     230,  1483,   697,   741,  -835,   661,   661,   697,   745,  1474,
     109,   635,   719,   231,  1300,   749,  -876,   561,  1102,  1120,
     212,  1103,  1321,   383,   757,  -875,   759,   777,   467,   212,
     926,   590,   404,  1421,   661,   705,   212,  1432,   404,   707,
    1263,  -818,   778,   212,   779,   878,   764,  -819,   491,   356,
     365,  1801,   924,   898,  1815,   900,   607,   794,   251,   404,
     879,   322,   938,   625,  1169,   739,   624,   626,  1273,  1128,
     357,  1275,   451,   452,   453,   562,   454,  1660,   359,   466,
     358,   782,   323,   207,   365,   832,   707,  1052,   455,  1053,
     839,   934,   612,   613,  1661,  1631,   376,   766,   359,  1636,
     772,  1251,   359,   359,  1493,   881,   365,    37,   484,   485,
     351,   122,   607,  1662,  1252,   362,   795,   916,   352,   368,
     369,   363,  1714,  1515,   116,   380,   384,   359,    49,   209,
     836,   836,   385,  1253,   390,   517,  1802,   -95,   209,  1816,
    1392,  1715,  1047,   994,  1716,   209,  -554,  1618,   999,  1619,
     407,   -95,   209,   368,   369,  -806,   365,   419,  1398,   410,
    1399,   420,   397,   623,  -846,   652,  -809,   421,   212,  -806,
     898,   900,   422,   906,   423,   368,   369,   967,   900,  -807,
    -809,   170,   484,   485,    85,   917,   968,    87,    88,  1288,
      89,   171,    91,  -807,   502,   625,   457,   714,   624,   626,
     852,   853,   365,   972,   973,  1067,    37,  1375,   607,   484,
     485,  1279,   153,   264,   458,   207,  1042,  1043,  1044,   925,
     651,   424,  1289,  1465,   207,   368,   369,    49,  1154,  1155,
    -665,   207,  1045,  1310,  1387,   211,   213,   365,   207,  1428,
    1429,   266,   425,   400,  1786,   109,  1632,  1633,   937,   109,
    1688,   426,  1452,   536,  1691,   459,   635,  1790,  1791,  1800,
    1171,  1172,  1173,    37,   365,  -846,  1315,  1172,  1173,  1492,
     608,   368,   369,  1775,  1776,  1777,   460,   209,   399,   401,
     402,   490,   365,   970,    49,  -840,    87,    88,   607,    89,
     171,    91,  -389,  -551,    37,  1516,   201,   564,   566,   242,
      61,    62,    63,   166,   167,   415,   368,   369,  1342,   392,
    1147,  1367,  1712,  1713,  1453,    49,   976,  1708,  1709,   542,
     543,  1002,  1005,  -663,   494,   625,   372,   499,   624,   626,
     501,   212,   602,   368,   369,   555,   455,   170,   987,   405,
      85,   316,   507,    87,    88,   510,    89,   171,    91,  -844,
     491,   368,   369,   511,  1490,  1184,  -661,   518,   527,  1640,
     519,   320,  1188,   207,   540,   468,   547,   416,  -976,   550,
    1389,   321,   732,    37,    87,    88,   551,    89,   171,    91,
     392,   557,  1407,  1077,   836,  1127,   836,  1784,   558,   212,
     836,   836,  1057,   569,    49,   570,   572,  1086,  1416,   579,
     109,   580,  1797,   581,   593,   733,   592,   104,   627,   628,
     653,   637,   650,   392,   322,  1763,   117,   359,   638,  -117,
     115,   119,   508,  1107,   120,  1269,   639,   641,    54,   516,
     212,   663,   212,  1115,   750,  1763,  1116,   752,  1117,   602,
     209,   754,   661,   755,  1785,   761,   762,   153,   780,   529,
     339,   153,   329,    87,    88,   784,    89,   171,    91,   225,
     212,   705,   787,   232,   546,  1088,   555,   359,   709,   359,
     359,   359,   359,   800,  1478,   801,   821,   739,   823,   772,
    1151,   230,   824,   842,  1495,   825,   826,   846,   847,   850,
    1293,   122,   734,  1501,   231,   851,   149,   857,   209,   859,
     868,   862,   873,  1506,   116,   874,   876,  -685,  1157,  1692,
     882,   883,   625,   555,    37,   624,   626,   212,   885,   635,
     889,   886,  1158,   893,  1244,   603,   207,   769,   894,   609,
      37,   902,   904,   212,   212,    49,   635,   122,   109,   209,
     907,   209,  1333,   337,   338,   910,   772,   908,   913,  1338,
     116,    49,   923,   919,   576,   920,   603,   818,   609,   603,
     609,   609,   922,   584,   931,   589,   939,   941,   942,   209,
     596,   915,   943,  -667,   969,  1245,  1646,   606,   979,   838,
     653,   989,   991,   625,   207,   993,   624,   626,   122,  1190,
    1246,   339,  1196,   995,    87,    88,   996,    89,   171,    91,
     997,   116,   153,  1727,   998,  1000,   865,   867,  1607,   122,
      87,    88,  1608,    89,   171,    91,    37,  1438,  1013,   836,
    1014,  1271,   116,  1015,   117,   207,   209,   207,   115,   119,
    1017,  1058,   120,  1018,   661,  1050,  1064,    49,  1453,  1062,
    1065,  1060,   209,   209,  1066,   661,  1246,  1071,  1075,  1404,
    1076,  1082,  1084,   212,   212,   207,  1091,  1085,  1093,  1089,
    1110,   544,  1119,   545,  1122,  1125,   623,    37,  1130,  -847,
    1131,  1141,   359,  1142,  1145,  1146,  1148,   242,  1160,  1162,
    1774,  1165,  1166,  1168,  1177,  1178,  1182,  1317,    49,  1183,
    1303,  1045,   122,   372,   122,  1187,    87,    88,  1186,    89,
     171,    91,   596,  1229,   149,   116,  1306,   116,  1231,  1232,
    1439,  1687,   207,  1234,  1240,  1243,  1259,   927,   549,  1268,
    1693,  1267,  1260,  1440,  1441,  1274,  1272,   373,   207,   207,
    1276,  1281,  1278,  1290,  1280,   635,  1291,  1283,   635,  1284,
     153,   170,  1292,   952,    85,  1442,  1466,    87,    88,  1302,
      89,  1443,    91,  1296,  1304,  1305,   956,  1307,   962,  1312,
    1322,   418,   209,   209,  1728,  1316,  1324,  1326,  1372,  1327,
    1330,  1332,  1373,  1331,  1374,  1334,    37,    54,   201,   212,
     109,  1336,  1381,  1337,  1343,    61,    62,    63,   166,   167,
     415,  1341,  1388,   661,   985,   109,   122,    49,   623,  1344,
    1368,  1369,   656,  1379,  1380,   329,  1382,  1394,  1750,   116,
    1395,   497,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   212,  1384,   109,  1401,  1396,    37,
    1386,  1390,  1397,  1054,   653,  1616,  1400,  1402,  1403,   212,
     212,    61,    62,    63,   166,   167,   415,  1406,   207,   207,
      49,    37,   416,  1408,   732,  1413,    87,    88,  1425,    89,
     171,    91,  1068,   482,   483,   905,  1366,  1409,  1436,  1799,
    1410,  1449,    49,  1414,  1464,  1366,  1806,   109,  1469,  1475,
    1476,  1509,  1479,  1484,  1485,  1491,   122,   768,   209,   104,
    1487,  1502,   555,  1504,  1477,  1510,  1613,   661,   109,   116,
    1518,   635,  1519,  1614,   734,  1620,   769,  1626,   416,    87,
      88,  1641,    89,   171,    91,  1627,  1629,   212,  1630,  1650,
    1639,  1671,  1651,   936,  1677,  1349,  1681,  1680,   623,   484,
     485,    87,    88,   209,    89,   171,    91,   663,  1686,  1702,
     359,  1704,  1706,   793,  1710,  1718,  1719,  1726,   209,   209,
    1725,  1720,  1135,  1135,   956,  1730,  1733,  1734,  1361,   915,
    -342,  1499,  1736,  1739,   963,  1741,   964,  1361,  1666,    14,
    1737,  1742,  1745,   769,   207,  1748,  1753,  1751,  1752,   109,
      37,   109,   153,   109,  1765,  1758,   640,  1760,  1769,  1773,
    1628,  1772,  1787,    37,   983,   863,   864,   153,  1679,  1781,
    1788,    49,  1803,  1180,  1783,  1703,  1705,  1804,  1511,  1798,
    1366,  1811,  1817,  1812,    49,  1820,  1366,  1818,  1366,   207,
     555,   635,  1821,   555,   887,   888,   209,  1768,   153,   713,
    1366,  1350,  1233,   896,   207,   207,  1351,   710,    61,    62,
      63,   166,  1352,   415,  1353,  1643,  1499,  1121,  1782,   708,
    1081,  1063,   818,  1309,   170,  1645,  1780,    85,  1415,   122,
      87,    88,  1659,    89,   171,    91,  1664,   596,  1074,   840,
    1456,  1793,   116,    87,    88,    37,    89,   171,    91,   153,
    1637,  1354,  1355,   468,  1356,   109,  1805,  1676,  1437,  1189,
    1458,  1517,  1696,  1254,  1635,  1137,    49,  1294,   598,  1295,
     153,  1149,  1361,  1674,  1003,   416,   662,  1101,  1361,  1427,
    1361,  1743,   207,  1370,  1366,   623,  1181,  1756,  1228,  1236,
       0,     0,  1361,     0,   122,     0,     0,     0,     0,     0,
    1723,     0,  1349,   122,     0,     0,     0,   116,    34,    35,
      36,  1682,     0,   206,   206,     0,   116,   222,   956,  1349,
     202,     0,   956,   412,     0,    87,    88,   418,    89,   171,
      91,     0,     0,   109,     0,  1349,     0,     0,     0,     0,
       0,   222,     0,    37,     0,   109,    14,     0,     0,     0,
       0,   153,     0,   153,     0,   153,   623,   983,  1164,     0,
       0,   323,     0,    14,    49,     0,  1621,   656,   656,    75,
      76,    77,    78,    79,     0,     0,  1361,     0,     0,    14,
     204,     0,   122,     0,     0,     0,    83,    84,   122,     0,
       0,     0,     0,   122,     0,   116,     0,     0,     0,     0,
      93,   116,     0,     0,     0,     0,   116,     0,  1350,     0,
       0,     0,  1345,  1351,    98,    61,    62,    63,   166,  1352,
     415,  1353,     0,    87,    88,  1350,    89,   171,    91,     0,
    1351,     0,    61,    62,    63,   166,  1352,   415,  1353,     0,
    1080,  1350,     0,     0,     0,     0,  1351,     0,    61,    62,
      63,   166,  1352,   415,  1353,     0,  1090,   153,  1354,  1355,
       0,  1356,     0,     0,     0,     0,   956,     0,   956,  1104,
       0,     0,     0,     0,     0,  1354,  1355,     0,  1356,     0,
       0,     0,   416,  1270,     0,     0,     0,     0,  1813,     0,
    1482,  1354,  1355,     0,  1356,     0,  1819,     0,  1124,   416,
     206,    37,  1822,     0,     0,  1823,     0,  1486,  1039,  1040,
    1041,  1042,  1043,  1044,     0,   416,     0,    37,   109,     0,
       0,     0,    49,  1488,     0,     0,   322,  1045,  1301,     0,
     829,   830,  1454,   122,     0,   153,     0,     0,    49,   635,
       0,     0,     0,   596,   983,     0,   116,   153,     0,     0,
     222,     0,   222,    61,    62,    63,   166,   167,   415,   635,
    1176,     0,     0,  1179,     0,     0,     0,     0,   635,     0,
       0,     0,     0,     0,   122,     0,     0,   956,   831,     0,
     122,    87,    88,   109,    89,   171,    91,   116,   109,     0,
       0,   170,   109,   116,    85,    86,  1349,    87,    88,     0,
      89,   171,    91,     0,     0,     0,   359,   222,     0,   555,
       0,     0,   322,  1349,     0,   122,     0,     0,     0,     0,
     416,   596,  1602,     0,  1757,     0,  1771,     0,   116,  1609,
     264,     0,   206,     0,     0,     0,   322,   122,   322,     0,
      14,   206,     0,     0,   322,     0,     0,     0,   206,     0,
     116,     0,     0,     0,     0,   206,     0,    14,   266,     0,
    1262,     0,   896,     0,     0,     0,   222,   956,     0,     0,
       0,   109,   109,   109,     0,     0,     0,   109,     0,     0,
      37,     0,   109,     0,     0,     0,   122,     0,     0,  1282,
       0,   222,  1285,   122,   222,     0,     0,     0,     0,   116,
       0,    49,  1350,     0,     0,     0,   116,  1351,     0,    61,
      62,    63,   166,  1352,   415,  1353,     0,     0,     0,  1350,
     153,     0,     0,     0,  1351,     0,    61,    62,    63,   166,
    1352,   415,  1353,     0,     0,    37,   542,   543,     0,     0,
     222,     0,     0,     0,     0,  1323,     0,     0,     0,  1104,
       0,     0,  1354,  1355,   170,  1356,    49,    85,   316,     0,
      87,    88,     0,    89,   171,    91,     0,  1001,     0,  1354,
    1355,     0,  1356,     0,     0,   264,   416,     0,   320,     0,
     206,     0,     0,     0,  1494,   153,     0,     0,   321,     0,
     153,     0,   555,   416,   153,    61,    62,    63,    64,    65,
     415,  1638,     0,   266,  1346,  1347,    71,   461,     0,     0,
       0,     0,   831,   322,     0,    87,    88,   956,    89,   171,
      91,     0,   109,     0,     0,    37,     0,     0,     0,     0,
    1697,     0,   222,   222,     0,     0,   811,  1602,  1602,     0,
       0,  1609,  1609,     0,   463,     0,    49,     0,     0,     0,
       0,     0,     0,     0,   548,   359,     0,     0,     0,     0,
       0,     0,   416,   109,     0,     0,     0,   811,     0,   109,
       0,     0,     0,   153,   153,   153,     0,     0,     0,   153,
       0,   542,   543,     0,   153,     0,     0,     0,     0,     0,
       0,  1417,     0,  1418,     0,     0,     0,     0,     0,   170,
       0,     0,    85,   316,   109,    87,    88,     0,    89,   171,
      91,     0,  1755,   222,   222,     0,     0,     0,     0,     0,
       0,     0,   222,   320,     0,     0,   109,     0,  1459,     0,
       0,     0,  1770,   321,     0,     0,     0,     0,   946,   947,
       0,     0,     0,   206,   497,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,   948,     0,
       0,     0,     0,     0,     0,     0,   949,   950,   951,    37,
       0,     0,     0,     0,     0,   109,     0,     0,   952,  1019,
    1020,  1021,   109,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,   482,   483,     0,  1022,
       0,   206,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,   153,   953,     0,  1132,  1133,  1134,
      37,     0,     0,     0,     0,     0,     0,  1045,   954,     0,
       0,     0,   206,     0,   206,     0,     0,     0,     0,    87,
      88,    49,    89,   171,    91,     0,     0,     0,     0,     0,
       0,     0,   484,   485,     0,   153,     0,   955,     0,     0,
       0,   153,   206,   811,   448,   449,   450,   451,   452,   453,
       0,   454,     0,     0,     0,  1655,   222,   222,   811,   811,
     811,   811,   811,   455,     0,     0,   811,     0,     0,     0,
       0,     0,     0,     0,   208,   208,   153,   222,   224,     0,
      87,    88,     0,    89,   171,    91,     0,     0,     0,   763,
       0,     0,     0,     0,     0,     0,     0,     0,   153,   206,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   222,     0,   206,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1194,   222,
     222,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,     0,   222,     0,   153,     0,     0,
       0,     0,   482,   483,   153,     0,     0,  1678,   222,     0,
       0,     0,     0,     0,     0,     0,   811,     0,     0,   222,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,     0,
     430,   222,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,   484,   485,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,     0,     0,     0,   206,   206,     0,     0,  1738,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,   222,     0,   222,    61,    62,    63,    64,    65,
     415,   208,     0,     0,     0,     0,    71,   461,     0,   811,
       0,   222,     0,     0,   811,   811,   811,   811,   811,   811,
     811,   811,   811,   811,   811,   811,   811,   811,   811,   811,
     811,   811,   811,   811,   811,   811,   811,   811,   811,   811,
     811,   811,   462,     0,   463,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   896,     0,   464,   811,   465,
       0,     0,   416,     0,     0,     0,     0,     0,     0,     0,
     896,   497,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,     0,     0,     0,     0,     0,   222,
       0,   222,     0,     0,     0,  1247,     0,     0,     0,     0,
       0,   206,  -977,  -977,  -977,  -977,  -977,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,   222,     0,
       0,   222,     0,   482,   483,     0,     0,     0,   455,     0,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,   222,   208,     0,     0,     0,   206,     0,     0,   208,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,   206,   206,     0,   811,     0,     0,   208,     0,     0,
       0,     0,     0,     0,   222,     0,     0,     0,   222,     0,
       0,   811,     0,   811,   427,   428,   429,     0,     0,   484,
     485,     0,     0,     0,     0,     0,     0,     0,     0,   811,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,     0,     0,   222,   222,     0,   222,     0,     0,   206,
       0,   224,   455,     0,     0,     0,   848,  -977,  -977,  -977,
    -977,  -977,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1045,     0,     0,     0,     0,     0,     0,
     430,   208,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
     222,     0,   222,     0,     0,     0,     0,   811,   222,     0,
       0,   811,     0,   811,     0,     0,   811,   814,     0,     0,
       0,     0,     0,     0,     0,     0,   222,   222,     0,     0,
     222,     0,   325,     0,   427,   428,   429,   222,     0,     0,
       0,     0,     0,     0,     0,     0,   869,     0,   814,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   222,
     454,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,   811,     0,     0,     0,     0,
       0,     0,     0,     0,   222,   222,     0,     0,     0,     0,
       0,     0,   222,     0,   222,     0,     0,     0,     0,     0,
       0,     0,   901,     0,   208,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   222,     0,   222,     0,
       0,     0,     0,     0,   222,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,     0,     0,     0,     0,     0,     0,     0,     0,
     811,   811,   208,   455,   264,     0,   811,     0,   222,     0,
       0,     0,     0,     0,   222,     0,   222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   266,     0,     0,     0,   940,     0,     0,     0,
       0,     0,     0,   208,     0,   208,     0,     0,   264,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,   325,     0,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   208,   814,    49,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   814,
     814,   814,   814,   814,     0,     0,     0,   814,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1049,     0,
     542,   543,     0,     0,     0,     0,   222,     0,   325,    49,
       0,     0,     0,     0,     0,     0,     0,   944,   170,     0,
     208,    85,   316,   222,    87,    88,     0,    89,   171,    91,
       0,  1325,     0,     0,  1070,     0,   208,   208,     0,     0,
     222,     0,   320,     0,   542,   543,   811,     0,     0,     0,
       0,  1070,   321,     0,     0,     0,     0,   811,     0,     0,
     208,     0,   170,   811,     0,    85,   316,   811,    87,    88,
       0,    89,   171,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   320,   814,   222,     0,
    1111,     0,   325,     0,     0,   325,   321,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,   497,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,     0,     0,   811,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   222,   208,   208,     0,     0,
       0,     0,     0,     0,   222,   482,   483,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     814,     0,   208,     0,     0,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   427,   428,   429,     0,     0,     0,     0,
       0,   484,   485,   325,   796,     0,     0,   812,     0,   814,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   812,   454,
       0,     0,     0,    37,     0,   210,   210,     0,     0,   228,
       0,   455,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,     0,   325,   325,  1439,     0,     0,     0,
       0,     0,   208,   325,     0,     0,  1045,   208,     0,  1440,
    1441,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   208,   208,     0,   814,     0,   170,     0,     0,
      85,    86,     0,    87,    88,     0,    89,  1443,    91,     0,
       0,     0,   814,     0,   814,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     814,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,     0,   427,
     428,   429,     0,     0,     0,  1061,     0,     0,     0,     0,
       0,  1045,     0,     0,     0,     0,     0,  1348,     0,   430,
     208,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   455,    34,    35,
      36,    37,   210,   201,     0,     0,     0,     0,     0,     0,
     618,     0,     0,     0,   812,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,   325,   325,   812,
     812,   812,   812,   812,     0,     0,     0,   812,     0,     0,
       0,     0,     0,   203,     0,     0,     0,     0,   814,   208,
       0,     0,   814,     0,   814,     0,     0,   814,     0,    75,
      76,    77,    78,    79,     0,     0,     0,     0,  1435,     0,
     204,  1448,     0,     0,     0,   170,    83,    84,    85,    86,
       0,    87,    88,     0,    89,   171,    91,     0,     0,     0,
      93,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     325,     0,     0,     0,    98,     0,     0,     0,     0,   619,
       0,     0,     0,     0,   104,     0,   325,     0,     0,     0,
     208,  1118,     0,     0,     0,     0,     0,     0,     0,   325,
       0,     0,     0,     0,   210,     0,   814,   812,     0,     0,
       0,     0,     0,   210,     0,  1507,  1508,   427,   428,   429,
     210,     0,     0,     0,     0,  1448,     0,   210,   325,     0,
       0,     0,     0,     0,     0,     0,     0,   430,   228,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     325,   814,   814,   325,     0,   796,     0,   814,     0,  1653,
       0,     0,     0,     0,     0,     0,     0,  1448,     0,     0,
     812,     0,   228,     0,     0,   812,   812,   812,   812,   812,
     812,   812,   812,   812,   812,   812,   812,   812,   812,   812,
     812,   812,   812,   812,   812,   812,   812,   812,   812,   812,
     812,   812,   812,   427,   428,   429,     0,     0,     0,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,   812,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
     325,     0,   325,     0,     0,     0,     0,     0,     0,  1129,
       0,   455,     0,     0,     0,     0,     0,     0,   815,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    29,   325,
       0,     0,   325,     0,     0,     0,    34,    35,    36,    37,
       0,   201,     0,     0,     0,     0,     0,     0,   202,   815,
       0,     0,     0,     0,     0,     0,     0,   814,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,   814,     0,
       0,     0,     0,     0,   814,   812,     0,     0,   814,     0,
       0,   203,     0,     0,     0,   325,     0,     0,     0,   325,
       0,     0,   812,     0,   812,    74,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,   204,     0,
     812,     0,     0,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,   210,     0,     0,    93,     0,
       0,     0,     0,     0,     0,  1153,     0,     0,     0,   814,
       0,     0,    98,     0,   325,   325,     0,   205,     0,  1767,
     575,     0,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   427,   428,   429,     0,  1435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   430,   210,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,     0,   454,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     455,     0,     0,     0,   210,     0,   210,     0,     0,     0,
       0,   325,     0,   325,     0,     0,     0,     0,   812,     0,
       0,     0,   812,     0,   812,     0,     0,   812,     0,     0,
       0,     0,     0,     0,   210,   815,     0,   325,     0,     0,
     427,   428,   429,     0,     0,     0,     0,     0,   325,     0,
     815,   815,   815,   815,   815,     0,     0,     0,   815,   816,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,     0,     0,
     841,   210,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,     0,     0,     0,     0,   812,   210,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   325,  1471,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,  1019,  1020,  1021,     0,
       0,     0,     0,     0,     0,     0,     0,   325,     0,   325,
       0,     0,     0,     0,     0,   325,  1022,     0,   815,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,     0,     0,   228,     0,     0,     0,     0,     0,     0,
       0,   812,   812,     0,  1045,     0,     0,   812,     0,     0,
       0,     0,     0,     0,     0,   325,  1019,  1020,  1021,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1472,     0,     0,     0,  1022,   210,   210,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1185,   815,     0,   228,  1045,     0,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,     0,     0,   984,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   325,     0,     0,
     815,  1006,  1007,  1008,  1009,     0,     0,     0,     0,  1016,
       0,     0,     0,     0,   325,     0,     0,     0,     0,     0,
    1335,     0,     0,     0,     0,     0,     0,  1203,     0,     0,
       0,  1698,     0,     0,     0,     0,     0,   812,     0,     0,
       0,     0,     0,   210,     0,   802,   803,     0,   812,     0,
       0,   804,     0,   805,   812,     0,     0,     0,   812,     0,
       0,     0,     0,     0,     0,   806,     0,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,   325,
       0,     0,     0,   228,     0,   202,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,   210,   210,     0,   815,     0,     0,  1108,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   812,
       0,     0,     0,   815,     0,   815,     0,     0,     0,     0,
       0,     0,   807,     0,    75,    76,    77,    78,    79,     0,
       0,   815,     0,     0,     0,   204,     0,     0,     0,     0,
     170,    83,    84,    85,   808,   325,    87,    88,     0,    89,
     171,    91,     0,     0,     0,    93,     0,     0,     0,     0,
     325,     0,     0,     0,   809,     0,     0,     0,     0,    98,
       0,   210,     0,     0,   810,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   455,  1197,  1200,  1201,
    1202,  1204,  1205,  1206,  1207,  1208,  1209,  1210,  1211,  1212,
    1213,  1214,  1215,  1216,  1217,  1218,  1219,  1220,  1221,  1222,
    1223,  1224,  1225,  1226,  1227,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1235,     0,     0,   427,   428,   429,     0,     0,   815,
     228,     0,     0,   815,     0,   815,     0,     0,   815,     0,
       0,     0,     0,     0,   430,  1318,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,   455,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   387,    12,
      13,   228,     0,     0,     0,     0,     0,     0,   715,     0,
       0,     0,     0,     0,     0,     0,     0,   815,     0,     0,
      15,    16,     0,     0,     0,     0,    17,  1313,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,  1328,     0,  1329,     0,    34,    35,
      36,    37,    38,    39,     0,     0,     0,     0,     0,     0,
      42,     0,  1339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      54,     0,     0,     0,     0,     0,     0,     0,    61,    62,
      63,   166,   167,   168,     0,     0,    68,    69,     0,     0,
       0,     0,   815,   815,  1319,     0,   169,    74,   815,    75,
      76,    77,    78,    79,     0,     0,     0,  1658,     0,     0,
      81,     0,     0,     0,     0,   170,    83,    84,    85,    86,
       0,    87,    88,     0,    89,   171,    91,     0,     0,     0,
      93,     0,     0,    94,     0,     0,     0,     0,     0,    95,
       0,     0,     0,     0,    98,    99,   100,     0,     0,   101,
       0,     0,     0,     0,   104,   105,     0,   106,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1420,     0,     0,     0,  1422,     0,  1423,     0,   430,  1424,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,     0,    34,    35,    36,    37,
       0,   201,     0,     0,     0,     0,   455,     0,   202,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,   815,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   815,
       0,   219,     0,     0,     0,   815,     0,   220,  1503,   815,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,   204,     0,
       0,     0,  1740,   170,    83,    84,    85,    86,     0,    87,
      88,     0,    89,   171,    91,     0,     0,     0,    93,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,    98,     0,     0,     0,     0,   221,     0,     0,
     815,     0,   104,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,     0,
       0,     0,     0,  1647,  1648,    14,    15,    16,     0,  1652,
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
      98,    99,   100,     0,     0,   101,     0,   102,   103,  1078,
     104,   105,     0,   106,   107,     0,     0,     0,     0,  1707,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
    1717,    10,     0,     0,     0,     0,  1722,     0,     0,     0,
    1724,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,  1759,    40,    41,     0,     0,     0,    42,    43,    44,
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
    1248,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
     642,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1048,   104,   105,     0,   106,
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
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1092,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1159,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,  1161,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
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
       0,    48,  1314,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,    64,    65,    66,     0,    67,    68,    69,
       0,    71,    72,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
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
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1426,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1649,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,  1694,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1729,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,  1732,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,    64,    65,
      66,     0,    67,    68,    69,     0,    71,    72,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1749,   104,   105,     0,   106,
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
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1766,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,  1807,   104,   105,     0,   106,
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
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,    92,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
    1814,   104,   105,     0,   106,   107,     5,     6,     7,     8,
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
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    92,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,   525,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   166,   167,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,   781,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,   166,   167,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,   986,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   166,   167,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   101,     0,   102,   103,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,  1498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,    40,    41,     0,
       0,     0,    42,    43,    44,    45,     0,    46,     0,    47,
       0,    48,     0,     0,    49,    50,     0,     0,     0,    51,
      52,    53,    54,     0,    56,    57,     0,    58,     0,    60,
      61,    62,    63,   166,   167,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,  1642,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,    40,    41,     0,     0,     0,    42,    43,    44,
      45,     0,    46,     0,    47,     0,    48,     0,     0,    49,
      50,     0,     0,     0,    51,    52,    53,    54,     0,    56,
      57,     0,    58,     0,    60,    61,    62,    63,   166,   167,
      66,     0,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,    80,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
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
      61,    62,    63,   166,   167,    66,     0,    67,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,    80,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   101,     0,   102,   103,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,     0,   330,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
     657,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
     658,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,     0,     0,   776,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,     0,     0,
    1105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1045,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
    1106,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
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
       0,     0,     0,     0,    49,     0,     0,     0,     0,   183,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,   539,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   455,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,   427,   428,   429,
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
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,   541,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,   249,   428,   429,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,     0,   454,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,   455,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,     0,   252,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     387,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
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
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,   559,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,   523,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   670,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   455,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,     0,     0,     0,   715,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1045,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,     0,
     756,     0,     0,     0,     0,     0,     0,     0,     0,   455,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,     0,     0,     0,     0,   758,     0,     0,     0,     0,
       0,     0,     0,     0,  1045,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,     0,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,     0,     0,     0,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  -977,  -977,  -977,
    -977,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
    1150,     0,     0,     0,     0,     0,     0,   455,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,     0,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,   427,   428,   429,     0,   104,   105,     0,   106,
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
       0,     0,     0,     0,     0,    61,    62,    63,   166,   167,
     168,     0,     0,    68,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   169,    74,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,   170,    83,    84,    85,    86,     0,    87,    88,
       0,    89,   171,    91,     0,     0,     0,    93,     0,     0,
      94,     0,   563,     0,     0,     0,    95,     0,     0,     0,
       0,    98,    99,   100,     0,     0,   172,   427,   428,   429,
       0,   104,   105,     0,   106,   107,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   455,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   604,    39,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,   166,   167,   168,     0,     0,    68,    69,
       0,     0,     0,     0,     0,     0,     0,     0,   169,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,     0,
       0,     0,    93,     0,     0,    94,   748,     0,     0,     0,
       0,    95,     0,     0,     0,     0,    98,    99,   100,     0,
       0,   172,     0,     0,     0,     0,   104,   105,     0,   106,
     107,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   262,   430,   263,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,   265,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
     201,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,   703,   309,   310,   311,
       0,     0,     0,   312,   552,   553,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   554,     0,     0,     0,     0,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,     0,   319,
    -977,  -977,  -977,  -977,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,     0,     0,   704,
       0,   104,     0,     0,   265,     0,     0,     0,     0,     0,
    1045,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
     201,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,   308,   309,   310,   311,
       0,     0,     0,   312,   552,   553,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   554,     0,     0,     0,     0,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,   264,   319,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   704,
       0,   104,     0,     0,   265,     0,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,     0,   309,   310,   311,
       0,     0,     0,   312,   313,   314,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   315,     0,     0,    85,   316,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,   264,   319,
       0,     0,     0,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,     0,
    1622,     0,     0,     0,   265,     0,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,     0,   309,   310,   311,
       0,     0,     0,   312,   313,   314,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   315,     0,     0,    85,   316,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,   264,   319,
       0,     0,     0,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,     0,
    1690,     0,     0,     0,   265,     0,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,   308,   309,   310,   311,
       0,     0,     0,   312,   313,   314,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   315,     0,     0,    85,   316,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,   264,   319,
       0,     0,     0,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,     0,
       0,     0,     0,     0,   265,     0,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,     0,   309,   310,   311,
       0,     0,     0,   312,   313,   314,     0,     0,     0,     0,
       0,   254,   255,     0,   256,   257,     0,     0,   258,   259,
     260,   261,   315,     0,     0,    85,   316,     0,    87,    88,
       0,    89,   171,    91,   317,   262,   318,   263,   264,   319,
       0,     0,     0,     0,     0,     0,   320,  1430,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,     0,
       0,     0,     0,     0,   265,     0,   266,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,    49,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,     0,     0,     0,     0,   309,   310,   311,
       0,     0,     0,   312,   313,   314,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   315,     0,     0,    85,   316,     0,    87,    88,
       0,    89,   171,    91,   317,     0,   318,     0,     0,   319,
    1522,  1523,  1524,  1525,  1526,     0,   320,  1527,  1528,  1529,
    1530,     0,     0,     0,     0,     0,   321,     0,     0,     0,
       0,     0,     0,     0,  1531,  1532,  1533,     0,  1022,     0,
       0,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1534,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1045,  1535,  1536,  1537,
    1538,  1539,  1540,  1541,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1542,  1543,  1544,
    1545,  1546,  1547,  1548,  1549,  1550,  1551,  1552,    49,  1553,
    1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,
    1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,  1573,
    1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,  1582,     0,
       0,     0,  1583,  1584,     0,  1585,  1586,  1587,  1588,  1589,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1590,  1591,  1592,     0,     0,     0,    87,    88,     0,
      89,   171,    91,  1593,     0,  1594,  1595,     0,  1596,   427,
     428,   429,     0,     0,     0,  1597,  1598,     0,  1599,     0,
    1600,  1601,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,     0,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,   255,   455,   256,   257,
       0,     0,   258,   259,   260,   261,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
       0,   263,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,     0,   454,     0,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,     0,
       0,     0,   267,   268,   269,   270,   271,   272,   273,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,    49,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,     0,   773,     0,
     308,   309,   310,   311,     0,     0,     0,   312,   552,   553,
       0,     0,     0,     0,     0,   254,   255,     0,   256,   257,
       0,     0,   258,   259,   260,   261,   554,     0,     0,     0,
       0,     0,    87,    88,     0,    89,   171,    91,   317,   262,
     318,   263,     0,   319,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,     0,     0,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1045,
       0,     0,   267,   268,   269,   270,   271,   272,   273,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,    49,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,     0,     0,     0,
    1195,   309,   310,   311,     0,     0,     0,   312,   552,   553,
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
       0,   309,   310,   311,     0,     0,     0,   312,   552,   553,
     980,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   554,     0,     0,     0,
       0,     0,    87,    88,     0,    89,   171,    91,   317,     0,
     318,     0,    29,   319,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   201,     0,     0,     0,     0,
       0,     0,   202,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   981,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,   802,
     803,     0,    93,     0,     0,   804,     0,   805,     0,     0,
       0,     0,     0,     0,     0,     0,    98,     0,     0,   806,
       0,   205,     0,     0,     0,     0,   104,    34,    35,    36,
      37,     0,     0,     0,     0,   427,   428,   429,     0,   202,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,     0,     0,     0,     0,   807,     0,    75,    76,
      77,    78,    79,   455,     0,     0,     0,     0,     0,   204,
       0,     0,     0,     0,   170,    83,    84,    85,   808,     0,
      87,    88,    29,    89,   171,    91,     0,     0,     0,    93,
      34,    35,    36,    37,     0,   201,     0,     0,   809,     0,
       0,     0,   202,    98,     0,     0,     0,     0,   810,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   500,
       0,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   595,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,     0,    89,   171,    91,    29,
       0,   935,    93,     0,     0,     0,     0,    34,    35,    36,
      37,     0,   201,     0,     0,     0,    98,     0,     0,   202,
       0,   205,     0,     0,     0,     0,   104,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,     0,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,   204,
       0,     0,     0,     0,   170,    83,    84,    85,    86,     0,
      87,    88,    29,    89,   171,    91,     0,     0,     0,    93,
      34,    35,    36,    37,     0,   201,     0,     0,     0,     0,
       0,     0,   202,    98,     0,     0,     0,     0,   205,     0,
       0,     0,     0,   104,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1073,    74,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,   170,    83,    84,
      85,    86,     0,    87,    88,    29,    89,   171,    91,     0,
       0,     0,    93,    34,    35,    36,    37,     0,   201,     0,
       0,     0,     0,     0,     0,   202,    98,     0,     0,     0,
       0,   205,     0,     0,     0,     0,   104,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
     170,    83,    84,    85,    86,     0,    87,    88,     0,    89,
     171,    91,     0,     0,     0,    93,     0,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   205,     0,     0,     0,   430,   104,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,     0,   454,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
       0,   454,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   455,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,     0,     0,   921,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,     0,   454,     0,     0,
       0,     0,     0,     0,     0,     0,  1019,  1020,  1021,   455,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1022,     0,   965,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,     0,     0,  1019,  1020,  1021,     0,     0,     0,     0,
       0,     0,     0,     0,  1045,     0,     0,     0,     0,     0,
       0,     0,     0,  1022,     0,  1277,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,     0,     0,
       0,    34,    35,    36,    37,     0,   201,     0,     0,     0,
       0,  1045,     0,   202,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
    1419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,   204,     0,     0,     0,  1505,   170,    83,
      84,    85,    86,     0,    87,    88,     0,    89,   171,    91,
       0,     0,     0,    93,     0,     0,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   221,     0,   785,     0,   430,   104,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,     0,   454,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   786,   430,   918,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,     0,   454,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,   455,     0,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,     0,
     454,  1019,  1020,  1021,     0,     0,     0,     0,     0,     0,
       0,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,  1022,  1340,     0,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1019,  1020,  1021,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1045,
       0,     0,     0,     0,     0,     0,  1022,     0,     0,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1020,  1021,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1045,     0,     0,     0,     0,     0,
    1022,     0,     0,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,   429,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1045,     0,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,  1021,   454,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,     0,     0,     0,     0,  1022,     0,     0,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1045
};

static const yytype_int16 yycheck[] =
{
       5,     6,   122,     8,     9,    10,    11,    12,    13,   149,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    55,    92,    29,    30,   222,    96,    97,     4,
     101,   174,   634,     4,     4,    31,    56,     4,    43,    31,
     380,  1095,   494,   380,   380,   894,    51,   217,    53,    33,
     712,    56,   121,    58,   513,   175,   630,    31,   660,   154,
     149,    45,   490,   491,   883,    56,    50,   101,   454,   631,
      31,   101,   101,   914,  1082,    80,   569,   570,   784,    43,
     611,   337,   338,   525,   486,     9,   791,   486,     9,   930,
     979,   519,    69,    48,   760,     9,   101,     9,   750,     9,
      59,     9,    14,     9,    14,    31,    14,     9,     9,     4,
      55,    35,    49,    50,    37,    48,    80,     9,   560,   521,
       9,     9,   521,    82,     9,   966,    85,     9,     9,   233,
       9,     9,     9,     9,    82,     9,     4,     9,   172,   234,
       9,    48,   172,   172,    31,     9,    82,    82,    82,     9,
       9,     9,    82,   100,    82,    53,    69,    37,    48,    82,
      88,   159,    48,   104,    37,   174,  1636,   172,   153,   132,
     133,   205,   153,   153,   179,   205,   205,   153,   518,   188,
      48,    27,    28,     0,    88,    56,  1091,   221,   188,   174,
     632,   221,    37,   191,   174,   188,  1015,    68,   174,    69,
     205,     4,    82,   188,   100,   101,   191,   188,     8,    82,
     157,   188,   189,    37,     4,   191,   221,   153,    69,   132,
     133,  1691,   150,   151,   152,    69,    69,    69,    69,   192,
     235,   833,   167,   238,   189,    69,   188,    82,   175,    69,
     245,   246,   110,    69,   167,   193,   150,   115,    69,   117,
     118,   119,   120,   121,   122,   123,   189,   191,    82,   193,
     158,   365,   174,   193,   190,   189,  1274,   933,   239,   190,
     191,    69,   243,  1281,   417,  1283,   190,  1166,   190,   186,
     190,    69,   190,   189,   989,  1126,   991,   167,   190,   190,
      27,    28,   160,   161,  1302,   163,   329,    69,   190,   189,
     186,   190,   190,   189,   191,   190,   132,   133,   190,   190,
     158,   190,   190,   190,   190,   189,   184,   189,   188,   153,
     189,   191,   167,   357,   192,   189,  1145,   357,   357,   189,
     189,   189,   122,   503,   776,   458,   829,   830,    69,   781,
     191,   872,   411,   167,   188,   188,   149,   191,   468,   911,
     191,   356,   357,   496,   188,   153,    37,   191,   363,    88,
     100,   191,    69,   368,   487,   191,   121,  1272,   214,   492,
     191,   104,   105,   128,    69,   408,    27,    28,   337,   338,
     339,   153,   387,    69,   329,   175,   188,  1395,    31,    30,
     395,   188,   356,   191,   463,   464,   465,   466,   469,    69,
     188,    82,   407,   191,    82,    69,   188,   859,    49,    69,
     191,    52,   188,   372,   104,   105,   157,   157,  1080,   191,
      53,   150,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   469,
     455,   621,   457,   458,   459,   100,  1162,   188,   454,   192,
     191,   100,   454,   408,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   214,   469,  1384,
     454,  1386,   487,   488,   191,   490,   491,   492,   493,   167,
       4,   386,   497,   454,  1146,   500,   191,   113,   900,   927,
     346,   900,   192,    82,   509,   191,   511,   527,   648,   355,
     706,   357,   157,  1332,   519,   460,   362,  1350,   157,   460,
    1094,   191,   527,   369,   529,   158,   518,   191,   188,   188,
      82,    82,   702,   637,    82,   639,    88,    30,    52,   157,
     173,    55,   738,   883,   986,   486,   883,   883,  1110,   935,
     188,  1113,    52,    53,    54,   171,    56,    14,    72,   648,
     188,   532,   507,   214,    82,  1058,   507,   823,    68,   825,
     575,   714,   190,   191,    31,  1480,    90,   518,    92,  1484,
     521,   159,    96,    97,  1403,   619,    82,    80,   132,   133,
     120,   394,    88,    50,   172,   188,    89,   666,   128,   151,
     152,   188,    30,  1436,   394,   188,   185,   121,   101,   346,
     569,   570,   191,   191,   619,   811,   167,   174,   355,   167,
    1282,    49,   818,   793,    52,   362,    69,  1460,   798,  1462,
     188,   188,   369,   151,   152,   174,    82,   190,  1290,    37,
    1292,   190,    88,   380,   188,   197,   174,   190,   494,   188,
     754,   755,   190,   658,   190,   151,   152,   761,   762,   174,
     188,   154,   132,   133,   157,   670,   761,   160,   161,  1128,
     163,   164,   165,   188,   188,  1015,    69,   467,  1015,  1015,
      49,    50,    82,    74,    75,   855,    80,  1261,    88,   132,
     133,  1119,     4,    30,    69,   346,    52,    53,    54,   704,
     196,   190,  1130,  1365,   355,   151,   152,   101,    74,    75,
     153,   362,    68,  1165,  1276,    27,    28,    82,   369,   130,
     131,    58,   190,    88,  1778,   239,   190,   191,   733,   243,
    1635,   190,   126,   247,  1639,   191,   631,   190,   191,  1793,
     103,   104,   105,    80,    82,   188,   103,   104,   105,  1401,
     150,   151,   152,   117,   118,   119,   153,   494,   105,   106,
     107,   188,    82,   768,   101,   188,   160,   161,    88,   163,
     164,   165,   109,    69,    80,  1437,    82,   337,   338,   784,
     117,   118,   119,   120,   121,   122,   151,   152,  1230,   101,
     960,  1243,  1665,  1666,   188,   101,   780,  1661,  1662,   136,
     137,   800,   801,   153,   188,  1145,   157,   190,  1145,  1145,
      47,   657,   150,   151,   152,   329,    68,   154,   789,   174,
     157,   158,   153,   160,   161,   195,   163,   164,   165,   188,
     188,   151,   152,     9,  1396,  1005,   153,   153,     8,  1491,
     188,   178,  1012,   494,   190,   648,   188,   184,   153,    14,
    1278,   188,   158,    80,   160,   161,   153,   163,   164,   165,
     172,   190,  1304,   868,   823,   934,   825,  1772,   190,   715,
     829,   830,   831,   191,   101,     9,   190,   882,  1320,   128,
     394,   128,  1787,    14,   174,   191,   189,   193,    14,   100,
     404,   189,   194,   205,   408,  1744,   871,   411,   189,   188,
     871,   871,   214,   908,   871,  1101,   189,   189,   109,   221,
     756,   188,   758,   918,   188,  1764,   921,     9,   923,   150,
     657,   189,   927,   189,  1773,   189,   189,   239,    92,     9,
     157,   243,    55,   160,   161,   190,   163,   164,   165,   935,
     786,   886,    14,   935,   174,   886,   460,   461,   462,   463,
     464,   465,   466,   188,  1382,     9,   188,   898,   191,   900,
     965,   935,   190,    82,  1406,   191,   190,   189,   189,   189,
    1140,   774,   486,  1415,   935,   190,   871,   130,   715,   188,
      69,   189,    31,  1425,   774,   131,   173,   153,   972,  1641,
     134,     9,  1332,   507,    80,  1332,  1332,   843,   189,   894,
      14,   153,   973,   186,  1075,   366,   657,   521,     9,   370,
      80,     9,   175,   859,   860,   101,   911,   820,   532,   756,
     189,   758,  1192,   109,   110,    14,   967,     9,   130,  1199,
     820,   101,     9,   195,   346,   195,   397,   551,   399,   400,
     401,   402,   192,   355,    14,   357,   195,   189,   189,   786,
     362,   188,   195,   153,   189,  1075,  1498,   369,   100,   573,
     574,   190,   190,  1403,   715,     9,  1403,  1403,   871,  1014,
    1075,   157,  1017,    89,   160,   161,   134,   163,   164,   165,
     153,   871,   394,  1685,     9,   189,   600,   601,   158,   892,
     160,   161,   162,   163,   164,   165,    80,    30,   188,  1058,
     153,  1106,   892,   188,  1079,   756,   843,   758,  1079,  1079,
     153,     9,  1079,   191,  1119,   191,   190,   101,   188,    14,
     175,   192,   859,   860,     9,  1130,  1131,   191,    14,  1299,
     195,   191,    14,   979,   980,   786,   186,   189,    31,   190,
     188,   264,   188,   266,    31,    14,   883,    80,   188,   188,
      14,    51,   666,   188,   188,     9,   189,  1162,   190,   190,
    1762,   188,   134,    14,   175,   134,     9,  1172,   101,   189,
    1154,    68,   975,   157,   977,     9,   160,   161,   195,   163,
     164,   165,   494,    82,  1079,   975,  1157,   977,   192,   192,
     123,  1633,   843,   190,     9,   188,   134,   188,   321,    82,
    1642,    14,   190,   136,   137,   191,   189,   191,   859,   860,
     188,   191,   188,   134,   189,  1110,   195,   191,  1113,   190,
     532,   154,     9,    89,   157,   158,  1366,   160,   161,   191,
     163,   164,   165,   150,    31,    76,   750,   190,   752,   189,
     175,  1361,   979,   980,  1686,   190,   134,    31,  1253,   189,
     189,     9,  1257,   134,  1259,   189,    80,   109,    82,  1105,
     774,   192,  1267,     9,   190,   117,   118,   119,   120,   121,
     122,   189,  1277,  1278,   788,   789,  1079,   101,  1015,   190,
     192,   191,   405,    14,    82,   408,   188,   190,  1730,  1079,
     191,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,  1150,   189,   820,   134,   188,    80,
     189,   189,   189,   827,   828,  1458,   189,   189,     9,  1165,
    1166,   117,   118,   119,   120,   121,   122,    31,   979,   980,
     101,    80,   184,   190,   158,   190,   160,   161,   191,   163,
     164,   165,   856,    66,    67,   657,  1241,   189,   110,  1791,
     189,   162,   101,   190,   190,  1250,  1798,   871,   158,    14,
      82,  1430,   115,   189,   189,   134,  1169,   191,  1105,   193,
     191,   189,   886,   134,  1379,    14,   190,  1382,   892,  1169,
     174,  1276,   191,    82,   898,    14,   900,    14,   184,   160,
     161,   134,   163,   164,   165,    82,   189,  1243,   188,   190,
     189,    14,   190,   715,    14,     4,    14,   190,  1145,   132,
     133,   160,   161,  1150,   163,   164,   165,   188,   191,     9,
     934,     9,   192,   546,    58,    82,   174,     9,  1165,  1166,
      82,   188,   946,   947,   948,   191,   190,   113,  1241,   188,
     100,  1412,   153,   175,   756,   165,   758,  1250,    35,    48,
     100,    14,   188,   967,  1105,   189,   171,   190,   188,   973,
      80,   975,   774,   977,    82,   175,   189,   175,   168,     9,
    1475,   189,   189,    80,   786,    82,    83,   789,  1621,    82,
     189,   101,    14,   997,   190,  1655,  1656,    82,  1433,   191,
    1385,    14,    14,    82,   101,    14,  1391,    82,  1393,  1150,
    1014,  1396,    82,  1017,   627,   628,  1243,  1753,   820,   466,
    1405,   110,  1058,   636,  1165,  1166,   115,   463,   117,   118,
     119,   120,   121,   122,   123,  1496,  1497,   928,  1769,   461,
     874,   843,  1046,  1163,   154,  1497,  1764,   157,  1317,  1342,
     160,   161,  1520,   163,   164,   165,  1605,   859,   860,   577,
    1356,  1785,  1342,   160,   161,    80,   163,   164,   165,   871,
    1487,   160,   161,  1366,   163,  1079,  1797,  1617,  1352,  1013,
    1360,  1438,   192,  1083,  1483,   947,   101,  1141,   363,  1142,
     892,   962,  1385,  1616,   800,   184,   408,   898,  1391,  1345,
    1393,  1719,  1243,   192,  1489,  1332,   998,  1737,  1046,  1066,
      -1,    -1,  1405,    -1,  1407,    -1,    -1,    -1,    -1,    -1,
    1679,    -1,     4,  1416,    -1,    -1,    -1,  1407,    77,    78,
      79,  1626,    -1,    27,    28,    -1,  1416,    31,  1142,     4,
      89,    -1,  1146,   158,    -1,   160,   161,  1757,   163,   164,
     165,    -1,    -1,  1157,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    80,    -1,  1169,    48,    -1,    -1,    -1,
      -1,   973,    -1,   975,    -1,   977,  1403,   979,   980,    -1,
      -1,  1616,    -1,    48,   101,    -1,  1466,   800,   801,   138,
     139,   140,   141,   142,    -1,    -1,  1489,    -1,    -1,    48,
     149,    -1,  1495,    -1,    -1,    -1,   155,   156,  1501,    -1,
      -1,    -1,    -1,  1506,    -1,  1495,    -1,    -1,    -1,    -1,
     169,  1501,    -1,    -1,    -1,    -1,  1506,    -1,   110,    -1,
      -1,    -1,  1236,   115,   183,   117,   118,   119,   120,   121,
     122,   123,    -1,   160,   161,   110,   163,   164,   165,    -1,
     115,    -1,   117,   118,   119,   120,   121,   122,   123,    -1,
     873,   110,    -1,    -1,    -1,    -1,   115,    -1,   117,   118,
     119,   120,   121,   122,   123,    -1,   889,  1079,   160,   161,
      -1,   163,    -1,    -1,    -1,    -1,  1290,    -1,  1292,   902,
      -1,    -1,    -1,    -1,    -1,   160,   161,    -1,   163,    -1,
      -1,    -1,   184,  1105,    -1,    -1,    -1,    -1,  1803,    -1,
     192,   160,   161,    -1,   163,    -1,  1811,    -1,   931,   184,
     214,    80,  1817,    -1,    -1,  1820,    -1,   192,    49,    50,
      51,    52,    53,    54,    -1,   184,    -1,    80,  1342,    -1,
      -1,    -1,   101,   192,    -1,    -1,  1350,    68,  1150,    -1,
     109,   110,  1356,  1646,    -1,  1157,    -1,    -1,   101,  1744,
      -1,    -1,    -1,  1165,  1166,    -1,  1646,  1169,    -1,    -1,
     264,    -1,   266,   117,   118,   119,   120,   121,   122,  1764,
     993,    -1,    -1,   996,    -1,    -1,    -1,    -1,  1773,    -1,
      -1,    -1,    -1,    -1,  1687,    -1,    -1,  1401,   157,    -1,
    1693,   160,   161,  1407,   163,   164,   165,  1687,  1412,    -1,
      -1,   154,  1416,  1693,   157,   158,     4,   160,   161,    -1,
     163,   164,   165,    -1,    -1,    -1,  1430,   321,    -1,  1433,
      -1,    -1,  1436,     4,    -1,  1728,    -1,    -1,    -1,    -1,
     184,  1243,  1446,    -1,  1737,    -1,   190,    -1,  1728,  1453,
      30,    -1,   346,    -1,    -1,    -1,  1460,  1750,  1462,    -1,
      48,   355,    -1,    -1,  1468,    -1,    -1,    -1,   362,    -1,
    1750,    -1,    -1,    -1,    -1,   369,    -1,    48,    58,    -1,
    1093,    -1,  1095,    -1,    -1,    -1,   380,  1491,    -1,    -1,
      -1,  1495,  1496,  1497,    -1,    -1,    -1,  1501,    -1,    -1,
      80,    -1,  1506,    -1,    -1,    -1,  1799,    -1,    -1,  1122,
      -1,   405,  1125,  1806,   408,    -1,    -1,    -1,    -1,  1799,
      -1,   101,   110,    -1,    -1,    -1,  1806,   115,    -1,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,   110,
    1342,    -1,    -1,    -1,   115,    -1,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    80,   136,   137,    -1,    -1,
     454,    -1,    -1,    -1,    -1,  1178,    -1,    -1,    -1,  1182,
      -1,    -1,   160,   161,   154,   163,   101,   157,   158,    -1,
     160,   161,    -1,   163,   164,   165,    -1,   167,    -1,   160,
     161,    -1,   163,    -1,    -1,    30,   184,    -1,   178,    -1,
     494,    -1,    -1,    -1,   192,  1407,    -1,    -1,   188,    -1,
    1412,    -1,  1616,   184,  1416,   117,   118,   119,   120,   121,
     122,   192,    -1,    58,  1237,  1238,   128,   129,    -1,    -1,
      -1,    -1,   157,  1637,    -1,   160,   161,  1641,   163,   164,
     165,    -1,  1646,    -1,    -1,    80,    -1,    -1,    -1,    -1,
    1654,    -1,   546,   547,    -1,    -1,   550,  1661,  1662,    -1,
      -1,  1665,  1666,    -1,   166,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   109,  1679,    -1,    -1,    -1,    -1,
      -1,    -1,   184,  1687,    -1,    -1,    -1,   581,    -1,  1693,
      -1,    -1,    -1,  1495,  1496,  1497,    -1,    -1,    -1,  1501,
      -1,   136,   137,    -1,  1506,    -1,    -1,    -1,    -1,    -1,
      -1,  1324,    -1,  1326,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,   157,   158,  1728,   160,   161,    -1,   163,   164,
     165,    -1,  1736,   627,   628,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   636,   178,    -1,    -1,  1750,    -1,  1361,    -1,
      -1,    -1,  1756,   188,    -1,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,   657,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,  1799,    -1,    -1,    89,    10,
      11,    12,  1806,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    30,
      -1,   715,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,  1646,   136,    -1,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    68,   149,    -1,
      -1,    -1,   756,    -1,   758,    -1,    -1,    -1,    -1,   160,
     161,   101,   163,   164,   165,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,  1687,    -1,   178,    -1,    -1,
      -1,  1693,   786,   787,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,  1518,   800,   801,   802,   803,
     804,   805,   806,    68,    -1,    -1,   810,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,  1728,   821,    31,    -1,
     160,   161,    -1,   163,   164,   165,    -1,    -1,    -1,   189,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1750,   843,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   857,    -1,   859,   860,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,   873,
     874,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   883,
      -1,    -1,    -1,    -1,    -1,   889,    -1,  1799,    -1,    -1,
      -1,    -1,    66,    67,  1806,    -1,    -1,  1620,   902,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   910,    -1,    -1,   913,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   931,    -1,    -1,
      30,   935,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,   979,   980,    -1,    -1,  1702,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   993,
      -1,    -1,   996,    -1,   998,   117,   118,   119,   120,   121,
     122,   214,    -1,    -1,    -1,    -1,   128,   129,    -1,  1013,
      -1,  1015,    -1,    -1,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,   164,    -1,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1778,    -1,   179,  1062,   181,
      -1,    -1,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1793,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1093,
      -1,  1095,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,
      -1,  1105,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,  1122,    -1,
      -1,  1125,    -1,    66,    67,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,   346,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1145,   355,    -1,    -1,    -1,  1150,    -1,    -1,   362,
      -1,    -1,    -1,    -1,    -1,    -1,   369,    -1,    -1,    -1,
      -1,  1165,  1166,    -1,  1168,    -1,    -1,   380,    -1,    -1,
      -1,    -1,    -1,    -1,  1178,    -1,    -1,    -1,  1182,    -1,
      -1,  1185,    -1,  1187,    10,    11,    12,    -1,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1203,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    -1,    -1,  1237,  1238,    -1,  1240,    -1,    -1,  1243,
      -1,   454,    68,    -1,    -1,    -1,   189,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      30,   494,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
    1324,    -1,  1326,    -1,    -1,    -1,    -1,  1331,  1332,    -1,
      -1,  1335,    -1,  1337,    -1,    -1,  1340,   550,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1350,  1351,    -1,    -1,
    1354,    -1,    55,    -1,    10,    11,    12,  1361,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   192,    -1,   581,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,  1403,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,  1419,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1428,  1429,    -1,    -1,    -1,    -1,
      -1,    -1,  1436,    -1,  1438,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   192,    -1,   657,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1460,    -1,  1462,    -1,
      -1,    -1,    -1,    -1,  1468,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1504,  1505,   715,    68,    30,    -1,  1510,    -1,  1512,    -1,
      -1,    -1,    -1,    -1,  1518,    -1,  1520,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    -1,    -1,    -1,   192,    -1,    -1,    -1,
      -1,    -1,    -1,   756,    -1,   758,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,   264,    -1,   266,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   786,   787,   101,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   802,
     803,   804,   805,   806,    -1,    -1,    -1,   810,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   821,    -1,
     136,   137,    -1,    -1,    -1,    -1,  1620,    -1,   321,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,   154,    -1,
     843,   157,   158,  1637,   160,   161,    -1,   163,   164,   165,
      -1,   167,    -1,    -1,   857,    -1,   859,   860,    -1,    -1,
    1654,    -1,   178,    -1,   136,   137,  1660,    -1,    -1,    -1,
      -1,   874,   188,    -1,    -1,    -1,    -1,  1671,    -1,    -1,
     883,    -1,   154,  1677,    -1,   157,   158,  1681,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   178,   910,  1702,    -1,
     913,    -1,   405,    -1,    -1,   408,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   935,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,  1742,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1752,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1769,   979,   980,    -1,    -1,
      -1,    -1,    -1,    -1,  1778,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1793,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1013,    -1,  1015,    -1,    -1,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,   132,   133,   546,   547,    -1,    -1,   550,    -1,  1062,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,   581,    56,
      -1,    -1,    -1,    80,    -1,    27,    28,    -1,    -1,    31,
      -1,    68,  1105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,   627,   628,   123,    -1,    -1,    -1,
      -1,    -1,  1145,   636,    -1,    -1,    68,  1150,    -1,   136,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1165,  1166,    -1,  1168,    -1,   154,    -1,    -1,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,  1185,    -1,  1187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1203,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    10,
      11,    12,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,  1240,    -1,    30,
    1243,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    77,    78,
      79,    80,   214,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    -1,    -1,    -1,   787,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   101,    -1,    -1,    -1,    -1,   800,   801,   802,
     803,   804,   805,   806,    -1,    -1,    -1,   810,    -1,    -1,
      -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,  1331,  1332,
      -1,    -1,  1335,    -1,  1337,    -1,    -1,  1340,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,  1351,    -1,
     149,  1354,    -1,    -1,    -1,   154,   155,   156,   157,   158,
      -1,   160,   161,    -1,   163,   164,   165,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     873,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,   188,
      -1,    -1,    -1,    -1,   193,    -1,   889,    -1,    -1,    -1,
    1403,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,
      -1,    -1,    -1,    -1,   346,    -1,  1419,   910,    -1,    -1,
      -1,    -1,    -1,   355,    -1,  1428,  1429,    10,    11,    12,
     362,    -1,    -1,    -1,    -1,  1438,    -1,   369,   931,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,   380,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     993,  1504,  1505,   996,    -1,   998,    -1,  1510,    -1,  1512,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1520,    -1,    -1,
    1013,    -1,   454,    -1,    -1,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,   494,    -1,    -1,    -1,    -1,    -1,    -1,  1062,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
    1093,    -1,  1095,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,   550,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,  1122,
      -1,    -1,  1125,    -1,    -1,    -1,    77,    78,    79,    80,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,   581,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1660,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1671,    -1,
      -1,    -1,    -1,    -1,  1677,  1168,    -1,    -1,  1681,    -1,
      -1,   122,    -1,    -1,    -1,  1178,    -1,    -1,    -1,  1182,
      -1,    -1,  1185,    -1,  1187,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
    1203,    -1,    -1,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,   657,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,  1742,
      -1,    -1,   183,    -1,  1237,  1238,    -1,   188,    -1,  1752,
     191,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,  1769,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,   715,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,   756,    -1,   758,    -1,    -1,    -1,
      -1,  1324,    -1,  1326,    -1,    -1,    -1,    -1,  1331,    -1,
      -1,    -1,  1335,    -1,  1337,    -1,    -1,  1340,    -1,    -1,
      -1,    -1,    -1,    -1,   786,   787,    -1,  1350,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1361,    -1,
     802,   803,   804,   805,   806,    -1,    -1,    -1,   810,   550,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
     581,   843,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1419,   859,   860,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1436,   192,    -1,    -1,    -1,    -1,    -1,
      -1,   883,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1460,    -1,  1462,
      -1,    -1,    -1,    -1,    -1,  1468,    30,    -1,   910,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,   935,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1504,  1505,    -1,    68,    -1,    -1,  1510,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1518,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    30,   979,   980,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,  1013,    -1,  1015,    68,    -1,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,    -1,    -1,   787,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1620,    -1,    -1,
    1062,   802,   803,   804,   805,    -1,    -1,    -1,    -1,   810,
      -1,    -1,    -1,    -1,  1637,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,  1654,    -1,    -1,    -1,    -1,    -1,  1660,    -1,    -1,
      -1,    -1,    -1,  1105,    -1,    49,    50,    -1,  1671,    -1,
      -1,    55,    -1,    57,  1677,    -1,    -1,    -1,  1681,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    78,    79,    80,    -1,    -1,  1702,
      -1,    -1,    -1,  1145,    -1,    89,    -1,    -1,  1150,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,  1165,  1166,    -1,  1168,    -1,    -1,   910,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1742,
      -1,    -1,    -1,  1185,    -1,  1187,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,  1203,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,  1778,   160,   161,    -1,   163,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
    1793,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,   183,
      -1,  1243,    -1,    -1,   188,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1062,    -1,    -1,    10,    11,    12,    -1,    -1,  1331,
    1332,    -1,    -1,  1335,    -1,  1337,    -1,    -1,  1340,    -1,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    68,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,  1403,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1419,    -1,    -1,
      49,    50,    -1,    -1,    -1,    -1,    55,  1168,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,  1185,    -1,  1187,    -1,    77,    78,
      79,    80,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    -1,  1203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,   118,
     119,   120,   121,   122,    -1,    -1,   125,   126,    -1,    -1,
      -1,    -1,  1504,  1505,   190,    -1,   135,   136,  1510,   138,
     139,   140,   141,   142,    -1,    -1,    -1,  1519,    -1,    -1,
     149,    -1,    -1,    -1,    -1,   154,   155,   156,   157,   158,
      -1,   160,   161,    -1,   163,   164,   165,    -1,    -1,    -1,
     169,    -1,    -1,   172,    -1,    -1,    -1,    -1,    -1,   178,
      -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,   188,
      -1,    -1,    -1,    -1,   193,   194,    -1,   196,   197,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1331,    -1,    -1,    -1,  1335,    -1,  1337,    -1,    30,  1340,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    77,    78,    79,    80,
      -1,    82,    -1,    -1,    -1,    -1,    68,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1660,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1671,
      -1,   122,    -1,    -1,    -1,  1677,    -1,   128,  1419,  1681,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,  1704,   154,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,   183,    -1,    -1,    -1,    -1,   188,    -1,    -1,
    1742,    -1,   193,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,    -1,  1504,  1505,    48,    49,    50,    -1,  1510,
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
     193,   194,    -1,   196,   197,    -1,    -1,    -1,    -1,  1660,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
    1671,    13,    -1,    -1,    -1,    -1,  1677,    -1,    -1,    -1,
    1681,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,  1742,    84,    85,    -1,    -1,    -1,    89,    90,    91,
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
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,
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
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
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
     167,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
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
      -1,   183,   184,   185,    -1,    -1,   188,    -1,    -1,   191,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
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
     167,    -1,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,   106,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    -1,
      -1,    -1,   169,    -1,    -1,   172,    -1,   190,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    49,    50,    -1,
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
      -1,   188,    -1,   190,    11,    12,   193,   194,    -1,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    68,    -1,    55,    -1,    57,    58,    59,    60,    61,
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
      -1,   183,   184,   185,    -1,    -1,   188,    -1,   190,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,   188,   189,    -1,    -1,
      -1,   193,   194,    -1,   196,   197,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    31,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
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
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    49,    50,    -1,
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
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
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
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    49,    50,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,    -1,    -1,    -1,   169,    -1,    -1,
     172,    -1,   190,    -1,    -1,    -1,   178,    -1,    -1,    -1,
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
      -1,    -1,   169,    -1,    -1,   172,   189,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   194,    -1,   196,
     197,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    30,    29,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,   128,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,    -1,    -1,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    -1,   171,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,   191,
      -1,   193,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,   128,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,    -1,    -1,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    30,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,   193,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    30,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    30,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,   128,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    30,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   154,    -1,    -1,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    27,   168,    29,    30,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   178,   179,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,   130,   131,
      -1,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,   157,   158,    -1,   160,   161,
      -1,   163,   164,   165,   166,    -1,   168,    -1,    -1,   171,
       3,     4,     5,     6,     7,    -1,   178,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    70,    71,    72,
      73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,   135,   136,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,   161,    -1,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,    10,
      11,    12,    -1,    -1,    -1,   178,   179,    -1,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    68,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   189,    -1,
     128,   129,   130,   131,    -1,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   154,    -1,    -1,    -1,
      -1,    -1,   160,   161,    -1,   163,   164,   165,   166,    27,
     168,    29,    -1,   171,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
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
      -1,   129,   130,   131,    -1,    -1,    -1,   135,   136,   137,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,    -1,   160,   161,    -1,   163,   164,   165,   166,    -1,
     168,    -1,    69,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    49,
      50,    -1,   169,    -1,    -1,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    69,
      -1,   188,    -1,    -1,    -1,    -1,   193,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,   136,    -1,   138,   139,
     140,   141,   142,    68,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,
     160,   161,    69,   163,   164,   165,    -1,    -1,    -1,   169,
      77,    78,    79,    80,    -1,    82,    -1,    -1,   178,    -1,
      -1,    -1,    89,   183,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    -1,   163,   164,   165,    69,
      -1,    71,   169,    -1,    -1,    -1,    -1,    77,    78,    79,
      80,    -1,    82,    -1,    -1,    -1,   183,    -1,    -1,    89,
      -1,   188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,    -1,   154,   155,   156,   157,   158,    -1,
     160,   161,    69,   163,   164,   165,    -1,    -1,    -1,   169,
      77,    78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,   183,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,   193,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,    -1,   160,   161,    69,   163,   164,   165,    -1,
      -1,    -1,   169,    77,    78,    79,    80,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    89,   183,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,   165,    -1,    -1,    -1,   169,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    30,   193,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,   134,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,   134,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    -1,
      -1,    77,    78,    79,    80,    -1,    82,    -1,    -1,    -1,
      -1,    68,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,   134,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,   165,
      -1,    -1,    -1,   169,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,   188,    -1,    28,    -1,    30,   193,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    12,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68
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
     178,   188,   205,   206,   207,   218,   446,   462,   463,   465,
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
     190,   190,   136,   137,   465,   465,   174,   188,   109,   465,
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
     194,   196,   197,   205,   451,   452,   465,    37,   167,   283,
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
      31,   191,   272,   465,    30,    89,   218,   459,   460,   461,
     188,     9,    49,    50,    55,    57,    69,   136,   158,   178,
     188,   216,   218,   361,   376,   384,   385,   386,   205,   464,
     220,   188,   230,   191,   190,   191,   190,   100,   157,   109,
     110,   157,   211,   212,   213,   214,   215,   211,   205,   340,
     286,   385,    82,     9,   189,   189,   189,   189,   189,   189,
     189,   190,    49,    50,   456,   457,   458,   130,   262,   188,
       9,   189,   189,    82,    83,   205,   440,   205,    69,   192,
     192,   201,   203,    31,   131,   261,   173,    53,   158,   173,
     370,   341,   134,     9,   389,   189,   153,   465,   465,    14,
     352,   281,   220,   186,     9,   390,   465,   466,   408,   413,
     408,   192,     9,   389,   175,   418,   340,   189,     9,   390,
      14,   344,   240,   130,   260,   188,   449,   340,    31,   195,
     195,   134,   192,     9,   389,   340,   450,   188,   250,   245,
     255,    14,   444,   248,   237,    71,   418,   340,   450,   195,
     192,   189,   189,   195,   192,   189,    49,    50,    69,    77,
      78,    79,    89,   136,   149,   178,   205,   392,   394,   395,
     398,   401,   205,   418,   418,   134,   260,   408,   413,   189,
     340,   276,    74,    75,   277,   220,   329,   220,   331,   100,
      37,   135,   266,   418,   385,   205,    31,   222,   270,   190,
     273,   190,   273,     9,   389,    89,   134,   153,     9,   389,
     189,   167,   451,   452,   453,   451,   385,   385,   385,   385,
     385,   388,   391,   188,   153,   188,   385,   153,   191,    10,
      11,    12,    30,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    68,   153,   450,   192,   376,
     191,   234,   210,   210,   205,   211,   211,   215,     9,   390,
     192,   192,    14,   418,   190,   175,     9,   389,   205,   263,
     376,   191,   431,   135,   418,    14,   195,   340,   192,   201,
     465,   263,   191,   369,    14,   189,   340,   353,   427,   190,
     465,   186,   192,    31,   454,   407,    37,    82,   167,   409,
     410,   412,   409,   410,   465,    37,   167,   340,   385,   281,
     188,   376,   261,   345,   241,   340,   340,   340,   192,   188,
     283,   262,    31,   261,   465,    14,   260,   449,   380,   192,
     188,    14,    77,    78,    79,   205,   393,   393,   395,   396,
     397,    51,   188,    88,   150,   188,     9,   389,   189,   402,
      37,   340,   261,   192,    74,    75,   278,   329,   222,   192,
     190,    93,   190,   266,   418,   188,   134,   265,    14,   220,
     273,   103,   104,   105,   273,   192,   465,   175,   134,   465,
     205,   459,     9,   189,   389,   134,   195,     9,   389,   388,
     206,   353,   355,   357,   189,   128,   206,   385,   436,   437,
     385,   385,   385,    31,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   385,   385,   385,   464,    82,
     235,   192,   192,   214,   190,   385,   458,   100,   101,   455,
       9,   291,   189,   188,   332,   337,   340,   195,   192,   444,
     291,   159,   172,   191,   365,   372,   159,   191,   371,   134,
     190,   454,   465,   352,   466,    82,   167,    14,    82,   450,
     418,   340,   189,   281,   191,   281,   188,   134,   188,   283,
     189,   191,   465,   191,   190,   465,   261,   242,   383,   283,
     134,   195,     9,   389,   394,   396,   150,   353,   399,   400,
     395,   418,   191,   329,    31,    76,   222,   190,   331,   265,
     431,   266,   189,   385,    99,   103,   190,   340,    31,   190,
     274,   192,   175,   465,   134,   167,    31,   189,   385,   385,
     189,   134,     9,   389,   189,   134,   192,     9,   389,   385,
      31,   189,   220,   190,   190,   205,   465,   465,   376,     4,
     110,   115,   121,   123,   160,   161,   163,   192,   292,   317,
     318,   319,   324,   325,   326,   327,   406,   431,   192,   191,
     192,    53,   340,   340,   340,   352,    37,    82,   167,    14,
      82,   340,   188,   454,   189,   291,   189,   281,   340,   283,
     189,   291,   444,   291,   190,   191,   188,   189,   395,   395,
     189,   134,   189,     9,   389,   291,    31,   220,   190,   189,
     189,   189,   227,   190,   190,   274,   220,   465,   465,   134,
     385,   353,   385,   385,   385,   191,   192,   455,   130,   131,
     179,   206,   447,   465,   264,   376,   110,   327,    30,   123,
     136,   137,   158,   164,   301,   302,   303,   304,   376,   162,
     309,   310,   126,   188,   205,   311,   312,   293,   238,   465,
       9,   190,     9,   190,   190,   444,   318,   189,   288,   158,
     367,   192,   192,    82,   167,    14,    82,   340,   283,   115,
     342,   454,   192,   454,   189,   189,   192,   191,   192,   291,
     281,   134,   395,   353,   192,   220,   225,   228,    31,   222,
     268,   220,   189,   385,   134,   134,   220,   376,   376,   449,
      14,   206,     9,   190,   191,   447,   444,   304,   174,   191,
       9,   190,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    29,    56,    70,    71,    72,    73,    74,
      75,    76,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,   136,   138,   139,   140,   141,   142,
     154,   155,   156,   166,   168,   169,   171,   178,   179,   181,
     183,   184,   205,   373,   374,     9,   190,   158,   162,   205,
     312,   313,   314,   190,    82,   323,   237,   294,   447,   447,
      14,   238,   192,   289,   290,   447,    14,    82,   340,   189,
     188,   454,   190,   191,   315,   342,   454,   288,   192,   189,
     395,   134,    31,   222,   267,   268,   220,   385,   385,   192,
     190,   190,   385,   376,   297,   465,   305,   306,   384,   302,
      14,    31,    50,   307,   310,     9,    35,   189,    30,    49,
      52,    14,     9,   190,   207,   448,   323,    14,   465,   237,
     190,    14,   340,    37,    82,   364,   191,   220,   454,   315,
     192,   454,   395,   220,    97,   233,   192,   205,   218,   298,
     299,   300,     9,   389,     9,   389,   192,   385,   374,   374,
      58,   308,   313,   313,    30,    49,    52,   385,    82,   174,
     188,   190,   385,   449,   385,    82,     9,   390,   220,   192,
     191,   315,    95,   190,   113,   229,   153,   100,   465,   175,
     384,   165,    14,   456,   295,   188,    37,    82,   189,   192,
     220,   190,   188,   171,   236,   205,   318,   319,   175,   385,
     175,   279,   280,   407,   296,    82,   192,   376,   234,   168,
     205,   190,   189,     9,   390,   117,   118,   119,   321,   322,
     279,    82,   264,   190,   454,   407,   466,   189,   189,   190,
     190,   191,   316,   321,    37,    82,   167,   454,   191,   220,
     466,    82,   167,    14,    82,   316,   220,   192,    37,    82,
     167,    14,    82,   340,   192,    82,   167,    14,    82,   340,
      14,    82,   340,   340
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
#line 3180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    {;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    {;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    {;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 969:

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

  case 970:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { (yyvsp[(1) - (5)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)])); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3294 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3300 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3309 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13779 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

