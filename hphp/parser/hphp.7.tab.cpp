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
#line 878 "hphp.7.tab.cpp"

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
#define YYLAST   18177

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  202
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  287
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1043
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1920

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
    2866,  2870,  2872,  2877,  2881,  2885,  2887,  2889,  2891,  2893,
    2895,  2899,  2903,  2908,  2913,  2917,  2919,  2921,  2929,  2939,
    2947,  2954,  2963,  2965,  2970,  2975,  2977,  2979,  2981,  2986,
    2989,  2991,  2992,  2994,  2996,  2998,  3002,  3006,  3010,  3011,
    3013,  3015,  3019,  3023,  3026,  3030,  3037,  3038,  3040,  3045,
    3048,  3049,  3055,  3059,  3063,  3065,  3072,  3077,  3082,  3085,
    3088,  3089,  3095,  3099,  3103,  3105,  3108,  3109,  3115,  3119,
    3123,  3125,  3128,  3131,  3133,  3136,  3138,  3143,  3147,  3151,
    3158,  3162,  3164,  3166,  3168,  3173,  3178,  3183,  3188,  3193,
    3198,  3201,  3204,  3209,  3212,  3215,  3217,  3221,  3225,  3229,
    3230,  3233,  3239,  3246,  3253,  3261,  3263,  3266,  3268,  3271,
    3273,  3278,  3280,  3285,  3289,  3290,  3292,  3296,  3299,  3303,
    3305,  3307,  3308,  3309,  3312,  3315,  3318,  3321,  3326,  3329,
    3335,  3339,  3341,  3343,  3344,  3348,  3353,  3359,  3363,  3365,
    3368,  3369,  3374,  3376,  3380,  3383,  3388,  3394,  3397,  3400,
    3402,  3404,  3406,  3408,  3410,  3412,  3416,  3419,  3422,  3425,
    3427,  3436,  3443,  3445
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     203,     0,    -1,    -1,   204,   205,    -1,   205,   206,    -1,
      -1,   226,    -1,   243,    -1,   250,    -1,   247,    -1,   257,
      -1,   466,    -1,   129,   192,   193,   194,    -1,   158,   219,
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
     158,   161,   219,    -1,   161,   219,    -1,   220,   471,    -1,
     220,   471,    -1,   223,     9,   467,    14,   404,    -1,   112,
     467,    14,   404,    -1,   224,   225,    -1,    -1,   226,    -1,
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
     463,   193,   194,    -1,   194,    -1,    86,    -1,    87,    -1,
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
     242,   241,   470,   244,   192,   285,   193,   475,   319,    -1,
      -1,   323,   242,   241,   470,   245,   192,   285,   193,   475,
     319,    -1,    -1,   427,   322,   242,   241,   470,   246,   192,
     285,   193,   475,   319,    -1,    -1,   168,   209,   248,    32,
     487,   465,   195,   292,   196,    -1,    -1,   427,   168,   209,
     249,    32,   487,   465,   195,   292,   196,    -1,    -1,   263,
     260,   251,   264,   265,   195,   295,   196,    -1,    -1,   427,
     263,   260,   252,   264,   265,   195,   295,   196,    -1,    -1,
     131,   261,   253,   266,   195,   295,   196,    -1,    -1,   427,
     131,   261,   254,   266,   195,   295,   196,    -1,    -1,   130,
     256,   402,   264,   265,   195,   295,   196,    -1,    -1,   170,
     262,   258,   265,   195,   295,   196,    -1,    -1,   427,   170,
     262,   259,   265,   195,   295,   196,    -1,   470,    -1,   162,
      -1,   470,    -1,   470,    -1,   130,    -1,   123,   130,    -1,
     123,   122,   130,    -1,   122,   123,   130,    -1,   122,   130,
      -1,   132,   395,    -1,    -1,   133,   267,    -1,    -1,   132,
     267,    -1,    -1,   395,    -1,   267,     9,   395,    -1,   395,
      -1,   268,     9,   395,    -1,   136,   270,    -1,    -1,   439,
      -1,    38,   439,    -1,   137,   192,   452,   193,    -1,   226,
      -1,    32,   224,    97,   194,    -1,   226,    -1,    32,   224,
      99,   194,    -1,   226,    -1,    32,   224,    95,   194,    -1,
     226,    -1,    32,   224,   101,   194,    -1,   209,    14,   404,
      -1,   275,     9,   209,    14,   404,    -1,   195,   277,   196,
      -1,   195,   194,   277,   196,    -1,    32,   277,   105,   194,
      -1,    32,   194,   277,   105,   194,    -1,   277,   106,   344,
     278,   224,    -1,   277,   107,   278,   224,    -1,    -1,    32,
      -1,   194,    -1,   279,    75,   333,   226,    -1,    -1,   280,
      75,   333,    32,   224,    -1,    -1,    76,   226,    -1,    -1,
      76,    32,   224,    -1,    -1,   284,     9,   428,   325,   488,
     171,    83,    -1,   284,     9,   428,   325,   488,    38,   171,
      83,    -1,   284,     9,   428,   325,   488,   171,    -1,   284,
     410,    -1,   428,   325,   488,   171,    83,    -1,   428,   325,
     488,    38,   171,    83,    -1,   428,   325,   488,   171,    -1,
      -1,   428,   325,   488,    83,    -1,   428,   325,   488,    38,
      83,    -1,   428,   325,   488,    38,    83,    14,   344,    -1,
     428,   325,   488,    83,    14,   344,    -1,   284,     9,   428,
     325,   488,    83,    -1,   284,     9,   428,   325,   488,    38,
      83,    -1,   284,     9,   428,   325,   488,    38,    83,    14,
     344,    -1,   284,     9,   428,   325,   488,    83,    14,   344,
      -1,   286,     9,   428,   488,   171,    83,    -1,   286,     9,
     428,   488,    38,   171,    83,    -1,   286,     9,   428,   488,
     171,    -1,   286,   410,    -1,   428,   488,   171,    83,    -1,
     428,   488,    38,   171,    83,    -1,   428,   488,   171,    -1,
      -1,   428,   488,    83,    -1,   428,   488,    38,    83,    -1,
     428,   488,    38,    83,    14,   344,    -1,   428,   488,    83,
      14,   344,    -1,   286,     9,   428,   488,    83,    -1,   286,
       9,   428,   488,    38,    83,    -1,   286,     9,   428,   488,
      38,    83,    14,   344,    -1,   286,     9,   428,   488,    83,
      14,   344,    -1,   288,   410,    -1,    -1,   344,    -1,    38,
     439,    -1,   171,   344,    -1,   288,     9,   344,    -1,   288,
       9,   171,   344,    -1,   288,     9,    38,   439,    -1,   289,
       9,   290,    -1,   290,    -1,    83,    -1,   197,   439,    -1,
     197,   195,   344,   196,    -1,   291,     9,    83,    -1,   291,
       9,    83,    14,   404,    -1,    83,    -1,    83,    14,   404,
      -1,   292,   293,    -1,    -1,   294,   194,    -1,   468,    14,
     404,    -1,   295,   296,    -1,    -1,    -1,   321,   297,   327,
     194,    -1,    -1,   323,   487,   298,   327,   194,    -1,   328,
     194,    -1,   329,   194,    -1,   330,   194,    -1,    -1,   322,
     242,   241,   469,   192,   299,   283,   193,   475,   320,    -1,
      -1,   427,   322,   242,   241,   470,   192,   300,   283,   193,
     475,   320,    -1,   164,   305,   194,    -1,   165,   313,   194,
      -1,   167,   315,   194,    -1,     4,   132,   395,   194,    -1,
       4,   133,   395,   194,    -1,   117,   268,   194,    -1,   117,
     268,   195,   301,   196,    -1,   301,   302,    -1,   301,   303,
      -1,    -1,   222,   157,   209,   172,   268,   194,    -1,   304,
     102,   322,   209,   194,    -1,   304,   102,   323,   194,    -1,
     222,   157,   209,    -1,   209,    -1,   306,    -1,   305,     9,
     306,    -1,   307,   392,   311,   312,    -1,   162,    -1,    31,
     308,    -1,   308,    -1,   138,    -1,   138,   178,   487,   409,
     179,    -1,   138,   178,   487,     9,   487,   179,    -1,   395,
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
     328,     9,   468,    14,   404,    -1,   112,   468,    14,   404,
      -1,   329,     9,   468,    -1,   123,   112,   468,    -1,   123,
     331,   465,    -1,   331,   465,    14,   487,    -1,   112,   183,
     470,    -1,   192,   332,   193,    -1,    72,   399,   402,    -1,
      72,   255,    -1,    71,   344,    -1,   384,    -1,   379,    -1,
     192,   344,   193,    -1,   334,     9,   344,    -1,   344,    -1,
     334,    -1,    -1,    27,    -1,    27,   344,    -1,    27,   344,
     136,   344,    -1,   192,   336,   193,    -1,   439,    14,   336,
      -1,   137,   192,   452,   193,    14,   336,    -1,    29,   344,
      -1,   439,    14,   339,    -1,    28,   344,    -1,   439,    14,
     341,    -1,   137,   192,   452,   193,    14,   341,    -1,   345,
      -1,   439,    -1,   332,    -1,   443,    -1,   442,    -1,   137,
     192,   452,   193,    14,   344,    -1,   439,    14,   344,    -1,
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
      -1,   344,    33,   344,    -1,   462,    -1,    66,   344,    -1,
      65,   344,    -1,    64,   344,    -1,    63,   344,    -1,    62,
     344,    -1,    61,   344,    -1,    60,   344,    -1,    73,   400,
      -1,    59,   344,    -1,   407,    -1,   363,    -1,   370,    -1,
     373,    -1,   362,    -1,   198,   401,   198,    -1,    13,   344,
      -1,   381,    -1,   117,   192,   383,   410,   193,    -1,    -1,
      -1,   242,   241,   192,   348,   285,   193,   475,   346,   475,
     195,   224,   196,    -1,    -1,   323,   242,   241,   192,   349,
     285,   193,   475,   346,   475,   195,   224,   196,    -1,    -1,
     188,    83,   351,   356,    -1,    -1,   188,   189,   352,   285,
     190,   475,   356,    -1,    -1,   188,   195,   353,   224,   196,
      -1,    -1,    83,   354,   356,    -1,    -1,   189,   355,   285,
     190,   475,   356,    -1,     8,   344,    -1,     8,   341,    -1,
       8,   195,   224,   196,    -1,    91,    -1,   464,    -1,   358,
       9,   357,   136,   344,    -1,   357,   136,   344,    -1,   359,
       9,   357,   136,   404,    -1,   357,   136,   404,    -1,   358,
     409,    -1,    -1,   359,   409,    -1,    -1,   182,   192,   360,
     193,    -1,   138,   192,   453,   193,    -1,    70,   453,   199,
      -1,   365,   409,    -1,    -1,   365,     9,   344,   136,   344,
      -1,   344,   136,   344,    -1,   365,     9,   344,   136,    38,
     439,    -1,   344,   136,    38,   439,    -1,   367,   409,    -1,
      -1,   367,     9,   404,   136,   404,    -1,   404,   136,   404,
      -1,   369,   409,    -1,    -1,   369,     9,   415,   136,   415,
      -1,   415,   136,   415,    -1,   139,    70,   364,   199,    -1,
     139,    70,   366,   199,    -1,   139,    70,   368,   199,    -1,
     140,    70,   376,   199,    -1,   140,    70,   377,   199,    -1,
     140,    70,   378,   199,    -1,   334,   409,    -1,    -1,   405,
     409,    -1,    -1,   416,   409,    -1,    -1,   395,   195,   455,
     196,    -1,   395,   195,   457,   196,    -1,   381,    70,   449,
     199,    -1,   382,    70,   449,   199,    -1,   363,    -1,   370,
      -1,   373,    -1,   464,    -1,   442,    -1,    91,    -1,   192,
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
     397,   157,   448,    -1,   396,    -1,   445,    -1,   398,   157,
     448,    -1,   395,    -1,   124,    -1,   450,    -1,   192,   193,
      -1,   333,    -1,    -1,    -1,    90,    -1,   459,    -1,   192,
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
     130,    -1,   220,    -1,    82,    -1,   464,    -1,   403,    -1,
     200,   459,   200,    -1,   201,   459,   201,    -1,   153,   459,
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
     196,    -1,   430,    -1,   448,    -1,   209,    -1,   195,   344,
     196,    -1,   432,    -1,   448,    -1,    70,   449,   199,    -1,
     195,   344,   196,    -1,   440,   434,    -1,   192,   332,   193,
     434,    -1,   451,   434,    -1,   192,   332,   193,   434,    -1,
     192,   332,   193,   429,   431,    -1,   192,   345,   193,   429,
     431,    -1,   192,   332,   193,   429,   430,    -1,   192,   345,
     193,   429,   430,    -1,   446,    -1,   394,    -1,   444,    -1,
     445,    -1,   435,    -1,   437,    -1,   439,   429,   431,    -1,
     398,   157,   448,    -1,   441,   192,   287,   193,    -1,   442,
     192,   287,   193,    -1,   192,   439,   193,    -1,   394,    -1,
     444,    -1,   445,    -1,   435,    -1,   439,   429,   431,    -1,
     438,    -1,   441,   192,   287,   193,    -1,   192,   439,   193,
      -1,   398,   157,   448,    -1,   446,    -1,   435,    -1,   394,
      -1,   363,    -1,   403,    -1,   192,   439,   193,    -1,   192,
     345,   193,    -1,   442,   192,   287,   193,    -1,   441,   192,
     287,   193,    -1,   192,   443,   193,    -1,   347,    -1,   350,
      -1,   439,   429,   433,   471,   192,   287,   193,    -1,   192,
     332,   193,   429,   433,   471,   192,   287,   193,    -1,   398,
     157,   211,   471,   192,   287,   193,    -1,   398,   157,   448,
     192,   287,   193,    -1,   398,   157,   195,   344,   196,   192,
     287,   193,    -1,   447,    -1,   447,    70,   449,   199,    -1,
     447,   195,   344,   196,    -1,   448,    -1,    83,    -1,    84,
      -1,   197,   195,   344,   196,    -1,   197,   448,    -1,   344,
      -1,    -1,   446,    -1,   436,    -1,   437,    -1,   450,   429,
     431,    -1,   397,   157,   446,    -1,   192,   439,   193,    -1,
      -1,   436,    -1,   438,    -1,   450,   429,   430,    -1,   192,
     439,   193,    -1,   452,     9,    -1,   452,     9,   439,    -1,
     452,     9,   137,   192,   452,   193,    -1,    -1,   439,    -1,
     137,   192,   452,   193,    -1,   454,   409,    -1,    -1,   454,
       9,   344,   136,   344,    -1,   454,     9,   344,    -1,   344,
     136,   344,    -1,   344,    -1,   454,     9,   344,   136,    38,
     439,    -1,   454,     9,    38,   439,    -1,   344,   136,    38,
     439,    -1,    38,   439,    -1,   456,   409,    -1,    -1,   456,
       9,   344,   136,   344,    -1,   456,     9,   344,    -1,   344,
     136,   344,    -1,   344,    -1,   458,   409,    -1,    -1,   458,
       9,   404,   136,   404,    -1,   458,     9,   404,    -1,   404,
     136,   404,    -1,   404,    -1,   459,   460,    -1,   459,    90,
      -1,   460,    -1,    90,   460,    -1,    83,    -1,    83,    70,
     461,   199,    -1,    83,   429,   209,    -1,   155,   344,   196,
      -1,   155,    82,    70,   344,   199,   196,    -1,   156,   439,
     196,    -1,   209,    -1,    85,    -1,    83,    -1,   127,   192,
     334,   193,    -1,   128,   192,   439,   193,    -1,   128,   192,
     345,   193,    -1,   128,   192,   443,   193,    -1,   128,   192,
     442,   193,    -1,   128,   192,   332,   193,    -1,     7,   344,
      -1,     6,   344,    -1,     5,   192,   344,   193,    -1,     4,
     344,    -1,     3,   344,    -1,   439,    -1,   463,     9,   439,
      -1,   398,   157,   210,    -1,   398,   157,   130,    -1,    -1,
     102,   487,    -1,   183,   470,    14,   487,   194,    -1,   427,
     183,   470,    14,   487,   194,    -1,   185,   470,   465,    14,
     487,   194,    -1,   427,   185,   470,   465,    14,   487,   194,
      -1,   211,    -1,   487,   211,    -1,   210,    -1,   487,   210,
      -1,   211,    -1,   211,   178,   477,   179,    -1,   209,    -1,
     209,   178,   477,   179,    -1,   178,   473,   179,    -1,    -1,
     487,    -1,   472,     9,   487,    -1,   472,   409,    -1,   472,
       9,   171,    -1,   473,    -1,   171,    -1,    -1,    -1,    32,
     487,    -1,   102,   487,    -1,   103,   487,    -1,   478,   409,
      -1,   478,     9,   479,   209,    -1,   479,   209,    -1,   478,
       9,   479,   209,   476,    -1,   479,   209,   476,    -1,    50,
      -1,    51,    -1,    -1,    91,   136,   487,    -1,    31,    91,
     136,   487,    -1,   222,   157,   209,   136,   487,    -1,   481,
       9,   480,    -1,   480,    -1,   481,   409,    -1,    -1,   182,
     192,   482,   193,    -1,   222,    -1,   209,   157,   485,    -1,
     209,   471,    -1,   178,   487,   409,   179,    -1,   178,   487,
       9,   487,   179,    -1,    31,   487,    -1,    59,   487,    -1,
     222,    -1,   138,    -1,   139,    -1,   140,    -1,   141,    -1,
     483,    -1,   484,   157,   485,    -1,   138,   486,    -1,   139,
     486,    -1,   140,   486,    -1,   162,    -1,   192,   111,   192,
     474,   193,    32,   487,   193,    -1,   192,   487,     9,   472,
     409,   193,    -1,   487,    -1,    -1
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
    2800,  2805,  2806,  2810,  2815,  2820,  2821,  2825,  2826,  2831,
    2833,  2838,  2849,  2863,  2875,  2890,  2891,  2892,  2893,  2894,
    2895,  2896,  2906,  2915,  2917,  2919,  2923,  2924,  2925,  2926,
    2927,  2943,  2944,  2946,  2948,  2955,  2956,  2957,  2958,  2959,
    2960,  2961,  2962,  2964,  2969,  2973,  2974,  2978,  2981,  2988,
    2992,  3001,  3008,  3016,  3018,  3019,  3023,  3024,  3025,  3027,
    3032,  3033,  3044,  3045,  3046,  3047,  3058,  3061,  3064,  3065,
    3066,  3067,  3078,  3082,  3083,  3084,  3086,  3087,  3088,  3092,
    3094,  3097,  3099,  3100,  3101,  3102,  3105,  3107,  3108,  3112,
    3114,  3117,  3119,  3120,  3121,  3125,  3127,  3130,  3133,  3135,
    3137,  3141,  3142,  3144,  3145,  3151,  3152,  3154,  3164,  3166,
    3168,  3171,  3172,  3173,  3177,  3178,  3179,  3180,  3181,  3182,
    3183,  3184,  3185,  3186,  3187,  3191,  3192,  3196,  3198,  3206,
    3208,  3212,  3216,  3221,  3225,  3233,  3234,  3238,  3239,  3245,
    3246,  3255,  3256,  3264,  3267,  3271,  3274,  3279,  3284,  3286,
    3287,  3288,  3292,  3293,  3297,  3298,  3301,  3306,  3309,  3311,
    3315,  3321,  3322,  3323,  3327,  3331,  3341,  3349,  3351,  3355,
    3357,  3362,  3368,  3371,  3376,  3381,  3383,  3390,  3393,  3396,
    3397,  3400,  3403,  3406,  3409,  3410,  3415,  3417,  3419,  3421,
    3425,  3431,  3441,  3442
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
     440,   440,   440,   440,   440,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   442,   443,   443,   444,   444,   445,
     445,   445,   446,   447,   447,   447,   448,   448,   448,   448,
     449,   449,   450,   450,   450,   450,   450,   450,   451,   451,
     451,   451,   451,   452,   452,   452,   452,   452,   452,   453,
     453,   454,   454,   454,   454,   454,   454,   454,   454,   455,
     455,   456,   456,   456,   456,   457,   457,   458,   458,   458,
     458,   459,   459,   459,   459,   460,   460,   460,   460,   460,
     460,   461,   461,   461,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   462,   462,   463,   463,   464,   464,   465,
     465,   466,   466,   466,   466,   467,   467,   468,   468,   469,
     469,   470,   470,   471,   471,   472,   472,   473,   474,   474,
     474,   474,   475,   475,   476,   476,   477,   478,   478,   478,
     478,   479,   479,   479,   480,   480,   480,   481,   481,   482,
     482,   483,   484,   485,   485,   486,   486,   487,   487,   487,
     487,   487,   487,   487,   487,   487,   487,   487,   487,   487,
     487,   487,   488,   488
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
       3,     1,     4,     3,     3,     1,     1,     1,     1,     1,
       3,     3,     4,     4,     3,     1,     1,     7,     9,     7,
       6,     8,     1,     4,     4,     1,     1,     1,     4,     2,
       1,     0,     1,     1,     1,     3,     3,     3,     0,     1,
       1,     3,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     3,     0,
       2,     5,     6,     6,     7,     1,     2,     1,     2,     1,
       4,     1,     4,     3,     0,     1,     3,     2,     3,     1,
       1,     0,     0,     2,     2,     2,     2,     4,     2,     5,
       3,     1,     1,     0,     3,     4,     5,     3,     1,     2,
       0,     4,     1,     3,     2,     4,     5,     2,     2,     1,
       1,     1,     1,     1,     1,     3,     2,     2,     2,     1,
       8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   429,     0,     0,   839,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   930,
       0,   918,   713,     0,   719,   720,   721,    25,   782,   906,
     907,   153,   154,   722,     0,   134,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   188,     0,     0,     0,     0,
       0,     0,   395,   396,   397,   400,   399,   398,     0,     0,
       0,     0,   217,     0,     0,     0,     0,     0,   726,   728,
     729,   723,   724,     0,     0,     0,   730,   725,     0,   697,
      27,    28,    29,    31,    30,     0,   727,     0,     0,     0,
       0,   731,   401,   533,     0,   152,   124,     0,   714,     0,
       0,     4,   114,   116,   781,     0,   696,     0,     6,   187,
       7,     9,     8,    10,     0,     0,   393,   442,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   440,   895,   896,
     515,   512,   513,   514,   423,   518,     0,   422,   866,   698,
     705,     0,   784,   511,   392,   869,   870,   881,   441,     0,
       0,   444,   443,   867,   868,   865,   902,   905,   501,   783,
      11,   400,   399,   398,     0,     0,    31,     0,   114,   187,
       0,   974,   441,   973,     0,   971,   970,   517,     0,   430,
     437,   435,     0,     0,   483,   484,   485,   486,   510,   508,
     507,   506,   505,   504,   503,   502,   906,   722,   700,     0,
       0,     0,     0,   994,   888,   698,     0,   699,   464,     0,
     462,     0,   934,     0,   791,   421,   709,   207,     0,   994,
     420,   708,   703,     0,   718,   699,   913,   914,   920,   912,
     710,     0,     0,   712,   509,     0,     0,     0,     0,   426,
       0,   132,   428,     0,     0,   138,   140,     0,     0,   142,
       0,    72,    71,    66,    65,    57,    58,    49,    69,    80,
      81,     0,    52,     0,    64,    56,    62,    83,    75,    74,
      47,    70,    90,    91,    48,    86,    45,    87,    46,    88,
      44,    92,    79,    84,    89,    76,    77,    51,    78,    82,
      43,    73,    59,    93,    67,    60,    50,    42,    41,    40,
      39,    38,    37,    61,    94,    98,    54,    35,    36,    63,
    1030,  1031,  1032,  1033,    55,  1039,    34,    53,    85,     0,
       0,   114,    97,   985,  1029,     0,  1034,     0,     0,   144,
       0,     0,     0,   178,     0,     0,     0,     0,     0,     0,
     793,     0,   102,   104,   306,     0,     0,   305,     0,   221,
       0,   218,   311,     0,     0,     0,     0,     0,   991,   203,
     215,   926,   930,   552,   297,   572,   297,     0,   955,     0,
     733,     0,     0,     0,   953,     0,    16,     0,   118,   195,
     209,   216,   598,   545,     0,   979,   525,   527,   529,   843,
     429,   442,     0,     0,   440,   441,   443,     0,     0,   909,
     715,     0,   716,     0,     0,     0,   177,     0,     0,   120,
     297,     0,    24,   186,     0,   214,   199,   213,   398,   401,
     187,   394,   167,   168,   169,   170,   171,   173,   174,   176,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   918,     0,
     166,   911,   911,   940,     0,     0,     0,     0,     0,     0,
       0,     0,   391,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463,   461,   844,   845,
       0,   911,     0,   857,   297,   297,   911,     0,   926,     0,
     187,     0,     0,   146,     0,   841,   836,   791,     0,   442,
     440,     0,   938,     0,   550,   790,   929,   718,   442,   440,
     441,   120,     0,   297,   419,     0,   859,   711,     0,   124,
     257,     0,   532,     0,   149,     0,     0,   427,     0,     0,
       0,     0,     0,   141,   165,   143,  1030,  1031,  1032,  1033,
    1027,  1028,     0,  1036,  1037,  1038,  1020,     0,     0,     0,
       0,    68,    95,    96,    33,    55,    32,   986,   172,   175,
     145,   124,     0,   162,   164,     0,     0,     0,     0,   105,
       0,   792,   103,    18,     0,    99,     0,   307,     0,   147,
     220,   219,     0,     0,   148,   975,     0,     0,   442,   440,
     441,   444,   443,     0,  1013,   227,     0,   927,     0,     0,
       0,     0,   791,     0,     0,     0,   793,   298,   791,     0,
       0,   150,     0,     0,   732,   954,   782,     0,     0,   952,
     787,   951,   117,     5,    13,    14,     0,   225,     0,     0,
     538,     0,     0,   791,     0,     0,   706,   701,   539,     0,
       0,     0,     0,   843,   124,     0,   793,   842,  1043,   418,
     432,   497,   875,   894,   129,   123,   125,   126,   127,   128,
     392,     0,   516,   785,   786,   115,   791,     0,   995,     0,
       0,   521,   189,   223,     0,   467,   469,   468,   480,     0,
       0,   500,   465,   466,   470,   472,   471,   488,   487,   490,
     489,   491,   493,   495,   494,   492,   482,   481,   474,   475,
     473,   476,   477,   479,   496,   478,   910,     0,     0,   944,
       0,   791,   978,     0,   977,   994,   872,   205,   197,   211,
       0,   979,   201,   187,     0,   433,   436,   438,   446,   460,
     459,   458,   457,   456,   455,   454,   453,   452,   451,   450,
     449,   847,     0,   846,   849,   871,   853,   994,   850,     0,
       0,     0,     0,     0,     0,     0,     0,   972,   431,   834,
     838,   790,   840,     0,   702,     0,   933,     0,   932,   223,
       0,   702,   917,   916,   902,   905,     0,     0,   846,   849,
     915,   850,   424,   259,   261,   124,   536,   535,   425,     0,
     124,   241,   133,   428,     0,     0,     0,     0,     0,   253,
     253,   139,   791,     0,     0,     0,  1018,   791,     0,  1001,
       0,     0,     0,     0,     0,   789,     0,     0,     0,   697,
       0,     0,   735,   696,   739,   740,   742,     0,   734,   122,
     741,   994,  1035,     0,     0,     0,     0,    19,     0,    20,
       0,   100,     0,     0,     0,   111,   793,     0,   109,   104,
     101,   106,     0,   304,   312,   309,     0,     0,   964,   969,
     966,   965,   968,   967,    12,  1011,  1012,     0,   791,     0,
       0,     0,   926,   923,     0,   549,     0,   565,   790,   551,
     299,   300,   694,   792,   296,   790,   571,   568,   695,   963,
     962,   961,     0,   957,     0,   958,   960,     0,     5,     0,
       0,     0,   592,   593,   601,   600,     0,   440,     0,   790,
     544,   548,     0,     0,   980,     0,   526,     0,     0,  1002,
     843,   283,  1042,     0,     0,   858,     0,   908,   790,   997,
     993,   693,     0,   843,     0,     0,   225,   523,   191,   499,
       0,   579,   580,     0,   577,   790,   939,     0,     0,   297,
     227,     0,   225,     0,     0,   223,     0,   918,   447,     0,
       0,   855,   856,   873,   874,   903,   904,     0,     0,     0,
     822,   798,   799,   800,   807,     0,     0,     0,     0,     0,
     811,   817,   818,   809,   810,   828,   791,     0,   836,   937,
     936,     0,   225,     0,   860,   717,     0,   263,     0,     0,
     130,     0,     0,     0,     0,     0,     0,     0,   233,   234,
     245,     0,   124,   243,   159,   253,     0,   253,     0,   790,
       0,     0,     0,     0,   790,  1019,  1021,  1000,   791,   999,
       0,   791,   763,   764,   761,   762,   797,     0,   791,   789,
     558,   574,     0,   547,     0,     0,   946,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1024,   179,     0,   182,   163,
       0,     0,   107,   112,   113,   105,   792,   110,     0,   308,
       0,   976,   151,   992,  1013,  1006,  1008,   226,   228,   318,
       0,     0,   924,     0,     0,   554,     0,     0,     0,   301,
     956,     0,    17,     0,   979,   224,   318,     0,     0,   702,
     541,     0,   707,   981,     0,  1002,   530,     0,     0,  1043,
       0,   288,   286,   849,   861,   994,   849,   862,   996,   121,
       0,   843,   222,     0,   843,     0,   498,   943,   942,     0,
     297,     0,     0,     0,     0,     0,     0,   225,   193,   718,
     848,   297,     0,   803,   804,   805,   806,   812,   813,   826,
       0,   791,     0,   822,   562,   576,     0,   802,   830,   790,
     833,   835,   837,     0,   931,     0,   848,     0,     0,     0,
       0,   260,   537,   135,     0,   428,   233,   235,   926,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   247,     0,
    1025,     0,  1014,     0,  1017,   790,     0,     0,     0,   737,
     790,   788,     0,     0,   791,     0,     0,   777,   791,   779,
       0,   791,     0,   743,   780,   778,   950,     0,   791,   746,
     748,   747,     0,     0,   744,   745,   749,   751,   750,   766,
     765,   768,   767,   769,   771,   773,   772,   770,   759,   758,
     753,   754,   752,   755,   756,   757,   760,  1023,     0,   124,
       0,     0,   108,    21,   310,     0,     0,     0,  1010,     0,
     392,   928,   926,   434,   439,   445,   556,     0,   303,   302,
       0,    15,     0,   392,   604,     0,     0,   606,   599,   602,
       0,   597,     0,   983,     0,  1003,   534,     0,   289,     0,
       0,   284,     0,  1002,     0,   318,     0,   843,     0,   297,
       0,   900,   318,   979,   318,   982,     0,     0,     0,   448,
       0,     0,   815,   790,   821,   808,     0,     0,   791,     0,
       0,   820,   791,   801,     0,     0,   791,   827,   935,   318,
       0,   124,     0,   256,   242,     0,     0,     0,   232,   155,
     246,     0,     0,   249,     0,   254,   255,   124,   248,  1026,
    1015,     0,   998,     0,  1041,   796,   795,   736,   566,   790,
     557,     0,   569,   790,   573,     0,   790,   546,   738,     0,
     578,   790,   945,   775,     0,     0,     0,    22,    23,  1007,
    1004,  1005,   229,     0,     0,     0,   399,   390,     0,     0,
       0,   204,   317,   319,     0,   389,     0,     0,     0,   979,
     392,     0,     0,   553,   959,   314,   210,   595,     0,     0,
     540,   528,     0,   292,   282,     0,   285,   291,   297,   520,
    1002,   392,  1002,     0,   941,     0,   899,   392,     0,   392,
     984,   318,   843,   897,   825,   824,   814,   567,   790,   561,
       0,   570,   790,   575,     0,   816,   790,   829,   392,   124,
     262,   131,   136,   157,   236,     0,   244,   250,   124,   252,
    1016,     0,     0,     0,   560,   776,   543,     0,   949,   948,
     774,   124,   183,  1009,     0,     0,     0,   987,     0,     0,
       0,   230,     0,   979,     0,   355,   351,   357,   697,    31,
       0,   345,     0,   350,   354,   367,     0,   365,   370,     0,
     369,     0,   368,     0,   187,   321,     0,   323,     0,   324,
     325,     0,     0,   925,   555,     0,   596,   594,   605,   603,
     293,     0,     0,   280,   290,     0,     0,  1002,     0,   200,
     520,  1002,   901,   206,   314,   212,   392,     0,     0,     0,
     564,   819,   832,     0,   208,   258,     0,     0,   124,   239,
     156,   251,  1040,   794,     0,     0,     0,     0,     0,     0,
     417,     0,   988,     0,   335,   339,   414,   415,   349,     0,
       0,     0,   330,   657,   656,   653,   655,   654,   674,   676,
     675,   645,   615,   617,   616,   635,   651,   650,   611,   622,
     623,   625,   624,   644,   628,   626,   627,   629,   630,   631,
     632,   633,   634,   636,   637,   638,   639,   640,   641,   643,
     642,   612,   613,   614,   618,   619,   621,   659,   660,   669,
     668,   667,   666,   665,   664,   652,   671,   661,   662,   663,
     646,   647,   648,   649,   672,   673,   691,   692,   677,   679,
     678,   680,   681,   658,   683,   682,   685,   687,   686,   620,
     690,   688,   689,   684,   670,   610,   362,   607,     0,   331,
     383,   384,   382,   375,     0,   376,   332,   409,     0,     0,
       0,     0,   413,     0,   187,   196,   313,     0,     0,     0,
     281,   295,   898,     0,     0,   385,   124,   190,  1002,     0,
       0,   202,  1002,   823,     0,     0,   124,   237,   137,   158,
       0,   559,   542,   947,   181,   333,   334,   412,   231,     0,
     791,   791,     0,   358,   346,     0,     0,     0,   364,   366,
       0,     0,   371,   378,   379,   377,     0,     0,   320,   989,
       0,     0,     0,   416,     0,   315,     0,   294,     0,   590,
     793,   124,     0,     0,   192,   198,     0,   563,   831,     0,
       0,   160,   336,   114,     0,   337,   338,     0,   790,     0,
     790,   360,   356,   361,   608,   609,     0,   347,   380,   381,
     373,   374,   372,   410,   407,  1013,   326,   322,   411,     0,
     316,   591,   792,     0,     0,   386,   124,   194,     0,   240,
       0,   185,     0,   392,     0,   352,   359,   363,     0,     0,
     843,   328,     0,   588,   519,   522,     0,   238,     0,     0,
     161,   343,     0,   391,   353,   408,   990,     0,   793,   403,
     843,   589,   524,     0,   184,     0,     0,   342,  1002,   843,
     267,   404,   405,   406,  1043,   402,     0,     0,     0,   341,
       0,   403,     0,  1002,     0,   340,   387,   124,   327,  1043,
       0,   272,   270,     0,   124,     0,     0,   273,     0,     0,
     268,   329,     0,   388,     0,   276,   266,     0,   269,   275,
     180,   277,     0,     0,   264,   274,     0,   265,   279,   278
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   111,   918,   643,   178,  1517,   735,
     349,   350,   351,   352,   866,   867,   868,   113,   114,   115,
     116,   117,   407,   675,   676,   546,   250,  1586,   552,  1495,
    1587,  1831,   855,   344,   581,  1791,  1099,  1289,  1850,   424,
     179,   677,   954,  1165,  1348,   121,   646,   971,   678,   693,
     975,   615,   970,   230,   527,   679,   647,   972,   426,   369,
     390,   124,   956,   921,   891,  1117,  1520,  1220,  1028,  1738,
    1590,   812,  1034,   551,   821,  1036,  1387,   804,  1017,  1020,
    1209,  1857,  1858,   665,   666,   625,   626,   356,   357,   363,
    1555,  1716,  1717,  1300,  1432,  1543,  1710,  1840,  1860,  1749,
    1795,  1796,  1797,  1530,  1531,  1532,  1533,  1751,  1752,  1758,
    1807,  1536,  1537,  1541,  1703,  1704,  1705,  1727,  1888,  1433,
    1434,   180,   126,  1874,  1875,  1708,  1436,  1437,  1438,  1439,
     127,   243,   547,   548,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,  1567,   138,   953,  1164,   139,   662,
     663,   664,   247,   399,   542,   652,   653,  1251,   654,  1252,
     140,   141,   621,   622,  1243,  1244,  1357,  1358,   142,   844,
    1001,   143,   845,  1002,   629,  1246,  1360,   144,   846,   145,
     146,  1780,   147,   648,  1557,   649,  1137,   926,  1321,  1318,
    1696,  1697,   148,   149,   150,   233,   151,   234,   244,   411,
     534,   152,  1056,  1248,   850,   153,  1057,   949,   592,  1058,
    1003,  1187,  1004,  1189,  1362,  1190,  1191,  1006,  1365,  1366,
    1007,   780,   517,   192,   193,   680,   668,   500,  1153,  1154,
     766,   767,   945,   155,   236,   156,   157,   182,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   727,   240,   241,
     618,   223,   224,   730,   731,  1257,  1258,   383,   384,   912,
     168,   606,   169,   661,   170,   335,  1718,  1770,   370,   419,
     686,   687,  1050,  1148,  1298,   887,   888,   889,   826,   827,
     828,   336,   337,   852,   563,  1519,   943
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1574
static const yytype_int16 yypact[] =
{
   -1574,   177, -1574, -1574,  5617, 13776, 13776,   -10, 13776, 13776,
   13776, 11189, 13776, 13776, -1574, 13776, 13776, 13776, 13776, 13776,
   13776, 13776, 13776, 13776, 13776, 13776, 13776, 16932, 16932, 11388,
   13776, 17490,    12,    54, -1574, -1574, -1574, -1574, -1574,   227,
   -1574, -1574, -1574,   332, 13776, -1574,    54,   213,   232,   242,
   -1574,    54, 11587,  1090, 11786, -1574, 14813, 10194,   275, 13776,
     814,    67, -1574, -1574, -1574,   423,    59,   314,   282,   411,
     420,   435, -1574,  1090,   442,   463,    85,    98, -1574, -1574,
   -1574, -1574, -1574, 13776,   461,   920, -1574, -1574,  1090, -1574,
   -1574, -1574, -1574,  1090, -1574,  1090, -1574,   432,   468,  1090,
    1090, -1574,   236, -1574, 11985, -1574, -1574,   298,   603,   634,
     634, -1574,   653,   502,   445,   495, -1574,    84, -1574,   657,
   -1574, -1574, -1574, -1574,  1382,  1078, -1574, -1574,   513,   518,
     531,   533,   538,   545,   557,   562, 13760, -1574, -1574, -1574,
   -1574,   152,   694,   695, -1574,   700,   701, -1574,   150,   580,
   -1574,   619,     9, -1574,  1605,   154, -1574, -1574,  2415,    81,
     596,   181, -1574,    92,    31,   604,    71, -1574, -1574,   723,
   -1574, -1574, -1574,   638,   610,   644, -1574, 13776, -1574,   657,
    1078, 17970,  3015, 17970, 13776, 17970, 17970, 14175,   618, 16280,
   14175, 17970,   776,  1090,   746,   746,   462,   746,   746,   746,
     746,   746,   746,   746,   746,   746, -1574, -1574, -1574,   633,
     635,    88, 13776,   651, -1574, -1574,   673,   643,   274,   650,
     274, 16932, 16510,   645,   841, -1574,   638, -1574, 13776,   651,
   -1574,   697, -1574,   705,   669, -1574,   155, -1574, -1574, -1574,
     274,    81, 12184, -1574, -1574, 13776,  9000,   855,    94, 17970,
    9995, -1574, 13776, 13776,  1090, -1574, -1574, 13959,   670, -1574,
   15499, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574,  3037, -1574,  3037, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
      95,   106,   107,    97,   644, -1574, -1574, -1574, -1574,   675,
    2997,   104, -1574, -1574,   708,   854, -1574,   712, 15811, -1574,
     677,   678, 15547, -1574,   243, 15595,  1405,  1405,  1090,   679,
     864,   681, -1574,    58, -1574, 16532,   101, -1574,   749, -1574,
     756, -1574,   876,   105, 16932, 13776, 13776,   699,   716, -1574,
   -1574, 16632, 11388, 13776, 10393, 13776, 10393,   108,   292,   535,
   -1574, 13975, 16932,   515, -1574,  1090, -1574,   402,   502, -1574,
   -1574, -1574, -1574, 17587,   882,   795, -1574, -1574, -1574,    93,
   13776,   706,   709, 17970,   714,  2494,   718,  5816, 13776, -1574,
      61,   703,   704,    61,    74,   434, -1574,  1090,  3037,   713,
   10393, 14813, -1574, -1574,   649, -1574, -1574, -1574, -1574, -1574,
     657, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   13776, 13776, 13776, 13776, 12383, 13776, 13776, 13776, 13776, 13776,
   13776, 13776, 13776, 13776, 13776, 13776, 13776, 13776, 13776, 13776,
   13776, 13776, 13776, 13776, 13776, 13776, 13776, 13776, 17684, 13776,
   -1574, 13776, 13776, 13776, 14174,  1090,  1090,  1090,  1090,  1090,
    1382,   801,  1317,  4873, 13776, 13776, 13776, 13776, 13776, 13776,
   13776, 13776, 13776, 13776, 13776, 13776, -1574, -1574, -1574, -1574,
     869, 13776, 13776, -1574, 10393, 10393, 13776, 13776, 16632,   721,
     657, 12582, 15661, -1574, 13776, -1574,   722,   906,   765,   727,
     728, 14318,   274, 12781, -1574, 12980, -1574,   669,   731,   734,
    2674, -1574,   268, 10393, -1574,  1187, -1574, -1574, 15709, -1574,
   -1574, 10592, -1574, 13776, -1574,   834,  9199,   921,   735, 17849,
     917,   127,    62, -1574, -1574, -1574,   763,   763,   763, -1574,
   -1574, -1574,  3037, -1574, -1574, -1574,   642,   757,   939, 16432,
    1090, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574,   759, -1574, -1574,   766,   760,   770,   768,   419,
    1258,  1416, -1574, -1574,  1090,  1090, 13776,   274,    67, -1574,
   -1574, -1574, 16432,   888, -1574,   274,   129,   134,   784,   792,
    2872,   148,   799,   794,   327,   861,   804,   274,   136,   806,
   17097,   798,   991, 16932, 13776,   809,   999, 17970,  1002,   813,
     821, -1574,   616,  1090, -1574, -1574,   945,  3757,     0, -1574,
   -1574, -1574,   502, -1574, -1574, -1574,   984,   885,   844,    83,
     865, 13776,   889,  1015,   835,   870, -1574,   162, -1574,  3037,
    3037,  1016,   855,    93, -1574,   842,  1031, -1574,  3037,    57,
   -1574,   473,   161, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
    1944,  4089, -1574, -1574, -1574, -1574,  1032,   863, -1574,   850,
    1033, -1574, -1574,   916,  1040, 11771, 18108, 14175, 14639, 13776,
   17922, 14812,  3792, 12759,  4411,  4602,  4683, 12359, 12359, 12359,
   12359,  3194,  3194,  3194,  3194,  3194,  1575,  1575,   693,   693,
     693,   462,   462,   462, -1574,   746, 17970,   857,   858, 17145,
     853,  1050,    -7, 13776,   215,   651,    70, -1574, -1574, -1574,
    1048,   795, -1574,   657, 16732, -1574, -1574, -1574, 14175, 14175,
   14175, 14175, 14175, 14175, 14175, 14175, 14175, 14175, 14175, 14175,
   14175, -1574, 13776,   222, -1574,   164, -1574,   651,   514,   866,
    4224,   874,   875,   872,  4352,   137,   871, -1574, 17970,  2767,
   -1574,  1090, -1574,    57,   430, 16932, 17970, 16932, 17201,   916,
      57,   274,   168, -1574,   162,   912,   880, 13776, -1574,   179,
   -1574, -1574, -1574,  8801,   472, -1574, -1574, 17970, 17970,    54,
   -1574, -1574, -1574, 13776,   972, 16312, 16432,  1090,  9398,   881,
     883, -1574,  1069,   988,   944,   925, -1574,  1074,   893,  1981,
    3037, 16432, 16432, 16432, 16432, 16432,   898,  1029,  1034,   946,
     909, 16432,   235,   950, -1574, -1574, -1574,   907, -1574, 18064,
   -1574,    19, -1574,  6015,  1054,   913,  1416, -1574,  1416, -1574,
    1090,  1090,  1416,  1416,  1090, -1574,  1101,   924, -1574,   438,
   -1574, -1574,  4555, -1574, 18064,  1098, 16932,   928, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574,   951,  1120,  1090,
    1054,   937, 16632, 16832,  1119, -1574, 13179, -1574, 13776, -1574,
     274, 17970, -1574, 10791, -1574, 13776, -1574, -1574, -1574, -1574,
   -1574, -1574,   935, -1574, 13776, -1574, -1574,  5104, -1574,  3037,
    1054,   941, -1574, -1574, -1574, -1574,  1126,   948, 13776, 17587,
   -1574, -1574, 14174,   952, -1574,  3037, -1574,   954,  6214,  1110,
      51, -1574, -1574,   123,   869, -1574,  1187, -1574,  3037, -1574,
   -1574, -1574, 16432,    99,   955,  1054,   885, -1574, -1574, 14812,
   13776, -1574, -1574, 13776, -1574, 13776, -1574,  4806,   956, 10393,
     861,  1117,   885,  3037,  1136,   916,  1090, 17684,   274,  5297,
     962, -1574, -1574,   180,   963, -1574, -1574,  1137,   688,   688,
    2767, -1574, -1574, -1574,  1104,   967,  1094,  1096,   322,   978,
   -1574, -1574, -1574, -1574, -1574, -1574,  1164,   981,   722,   274,
     274, 13378,   885,  1187, -1574, -1574,  5347,   509,    54,  9995,
   -1574,  6413,   987,  6612,   989, 16312, 16932,   983,  1042,   274,
   18064,  1172, -1574, -1574, -1574, -1574,   450, -1574,   279,  3037,
    1008,  1052,  3037,  1090,   642, -1574, -1574, -1574,  1182, -1574,
    1001,  1032,   767,   767,  1123,  1123,  3685,   996,  1194, 16432,
   16432, 16432, 16099, 17587,  2569, 15955, 16432, 16432, 16432, 16432,
   16202, 16432, 16432, 16432, 16432, 16432, 16432, 16432, 16432, 16432,
   16432, 16432, 16432, 16432, 16432, 16432, 16432, 16432, 16432, 16432,
   16432, 16432, 16432, 16432,  1090, -1574, -1574,  1128, -1574, -1574,
    1017,  1018, -1574, -1574, -1574,   440,  1258, -1574,  1023, -1574,
   16432,   274, -1574, -1574,   149, -1574,   485,  1215, -1574, -1574,
     143,  1037,   274, 10990, 16932, 17970, 17249, 16932, 13776, 17970,
   -1574,  3586, -1574,  5418,   795,  1215, -1574,   448,    17, -1574,
   17970,  1089,  1039, -1574,  1045,  1110, -1574,  3037,   855,  3037,
      90,  1212,  1150,   184, -1574,   651,   188, -1574, -1574, 18064,
    1043,    99, -1574,  1046,    99,  1053, 14812, 17970, 17305,  1055,
   10393,  1049,  1066,  3037,  1068,  1056,  3037,   885, -1574,   669,
     526, 10393, 13776, -1574, -1574, -1574, -1574, -1574, -1574,  1108,
    1065,  1239,  1158,  2767,  2767,  2767,  1111, -1574, 17587,  2767,
   -1574, -1574, -1574, 16932, 17970,  1072, -1574,    54,  1237,  1195,
    9995, -1574, -1574, -1574,  1085, 13776,  1042,   274, 16632, 16312,
    1087, 16432,  6811,   470,  1088, 13776,    65,   326, -1574,  1105,
   -1574,  3037, -1574,  1149, -1574,  3191,  1255,  1095, 16432, -1574,
   16432, -1574,  1099,  1097,  1280, 15757,  1102, 18064,  1284, -1574,
    1161,  1285,  1107, -1574, -1574, -1574, 17353,  1122,  1289, 10373,
   10771, 11368, 16432, 18018,  4280, 12958, 13156,  4638, 13353, 13552,
   13552, 13552, 13552,  2417,  2417,  2417,  2417,  2417,  1620,  1620,
     767,   767,   767,  1123,  1123,  1123,  1123, -1574,  1129, -1574,
    1127,  1130, -1574, -1574, 18064,  1090,  3037,  3037, -1574,  1054,
    1371, -1574, 16632, -1574, -1574, 14175,   274, 13577,   274, 17970,
    1131, -1574,  1109,  1660, -1574,    77, 13776, -1574, -1574, -1574,
   13776, -1574, 13776, -1574,   855, -1574, -1574,   132,  1302,  1243,
   13776, -1574,  1139,  1110,  1135, -1574,  1142,    99, 13776, 10393,
    1143, -1574, -1574,   795, -1574, -1574,  1138,  1134,  1146, -1574,
    1151,  2767, -1574,  2767, -1574, -1574,  1152,  1144,  1333,  1210,
    1154, -1574,  1348, -1574,  1222,  1166,  1351, -1574,   274, -1574,
    1331, -1574,  1170, -1574, -1574,  1173,  1178,   144, -1574, -1574,
   18064,  1171,  1180, -1574, 12168, -1574, -1574, -1574, -1574, -1574,
   -1574,  3037, -1574,  3037, -1574, 18064, 17408, -1574, -1574, 16432,
   -1574, 16432, -1574, 16432, -1574, 16432, 17587, -1574, -1574, 16432,
   -1574, 16432, -1574, 12562, 16432,  1177,  7010, -1574, -1574,   485,
   -1574, -1574, -1574,   477, 14986,  1054,  1267, -1574,   573,  1214,
     464, -1574, -1574, -1574,   801,  3289,   113,   114,  1189,   795,
    1317,   145, 16932, 17970, -1574, -1574, -1574,  1219, 11173, 11571,
   17970, -1574,   321,  1373,  1305, 13776, -1574, 17970, 10393,  1272,
    1110,  1970,  1110,  1197, 17970,  1199, -1574,  2106,  1198,  2203,
   -1574, -1574,    99, -1574, -1574,  1259, -1574, -1574,  2767, -1574,
    2767, -1574,  2767, -1574,  2767, -1574, 17587, -1574,  2261, -1574,
    8801, -1574, -1574, -1574, -1574,  9597, -1574, -1574, -1574,  8801,
   -1574,  1201, 16432, 17456, 18064, 18064, 18064,  1260, 18064, 17511,
   12562, -1574, -1574, -1574,  1054,  1054,  1090, -1574,  1384, 16099,
      86, -1574, 14986,   795,  1757, -1574,  1221, -1574,   115,  1205,
     116, -1574, 15333, -1574, -1574, -1574,   117, -1574, -1574,  1349,
   -1574,  1207, -1574,  1319,   657, -1574, 15160, -1574, 15160, -1574,
   -1574,  1390,   801, -1574,   274, 14467, -1574, -1574, -1574, -1574,
    1392,  1324, 13776, -1574, 17970,  1216,  1220,  1110,   456, -1574,
    1272,  1110, -1574, -1574, -1574, -1574,  2291,  1218,  2767,  1278,
   -1574, -1574, -1574,  1279, -1574,  8801,  9796,  9597, -1574, -1574,
   -1574,  8801, -1574, 18064, 16432, 16432, 16432,  7209,  1234,  1238,
   -1574, 16432, -1574,  1054, -1574, -1574, -1574, -1574, -1574,  3037,
    2679,   573, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574,   170, -1574,  1214, -1574,
   -1574, -1574, -1574, -1574,   103,   622, -1574,  1394,   119, 15811,
    1319,  1402, -1574,  3037,   657, -1574, -1574,  1240,  1404, 13776,
   -1574, 17970, -1574,   125,  1236, -1574, -1574, -1574,  1110,   456,
   14640, -1574,  1110, -1574,  2767,  2767, -1574, -1574, -1574, -1574,
    7408, 18064, 18064, 18064, -1574, -1574, -1574, 18064, -1574,   720,
    1424,  1433,  1247, -1574, -1574, 16432, 15333, 15333,  1385, -1574,
    1349,  1349,   631, -1574, -1574, -1574, 16432,  1363, -1574,  1273,
    1262,   120, 16432, -1574,  1090, -1574, 16432, 17970,  1376, -1574,
    1447, -1574,  7607,  1265, -1574, -1574,   456, -1574, -1574,  7806,
    1268,  1350, -1574,  1362,  1309, -1574, -1574,  1366,  3037,  1290,
    2679, -1574, -1574, 18064, -1574, -1574,  1301, -1574,  1435, -1574,
   -1574, -1574, -1574, 18064,  1459,   327, -1574, -1574, 18064,  1283,
   18064, -1574,   131,  1286,  8005, -1574, -1574, -1574,  1282, -1574,
    1295,  1323,  1090,  1317,  1298, -1574, -1574, -1574, 16432,  1320,
     100, -1574,  1417, -1574, -1574, -1574,  8204, -1574,  1054,   913,
   -1574,  1329,  1090,   974, -1574, 18064, -1574,  1310,  1495,   601,
     100, -1574, -1574,  1426, -1574,  1054,  1313, -1574,  1110,   146,
   -1574, -1574, -1574, -1574,  3037, -1574,  1330,  1332,   121, -1574,
     549,   601,   160,  1110,  1315, -1574, -1574, -1574, -1574,  3037,
     333,  1498,  1443,   549, -1574,  8403,   172,  1513,  1445, 13776,
   -1574, -1574,  8602, -1574,   334,  1515,  1448, 13776, -1574, 17970,
   -1574,  1519,  1471, 13776, -1574, 17970, 13776, -1574, 17970, 17970
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1574, -1574, -1574,  -564, -1574, -1574, -1574,   459,    52,   -33,
     407, -1574,  -263,  -528, -1574, -1574,   449,   -30,  1706, -1574,
    2945, -1574,   135, -1574,    25, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574,  -293, -1574, -1574,  -177,
     207,    30, -1574, -1574, -1574, -1574, -1574, -1574,    39, -1574,
   -1574, -1574, -1574, -1574, -1574,    46, -1574, -1574,  1082,  1083,
    1091,   -94,  -693,  -865,   594,   655,  -289,   362,  -933, -1574,
      -1, -1574, -1574, -1574, -1574,  -731,   203, -1574, -1574, -1574,
   -1574,  -271, -1574,  -627, -1574,  -330, -1574, -1574,   992, -1574,
      20, -1574, -1574, -1072, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574,   -20, -1574,    69, -1574, -1574, -1574,
   -1574, -1574,  -102, -1574,   167, -1116, -1574, -1573,  -294, -1574,
    -145,   252,   -98,  -281, -1574,  -107, -1574, -1574, -1574,   176,
     -19,   -11,    36,  -738,   -73, -1574, -1574,    15, -1574,     8,
   -1574, -1574,    -5,   -45,    37, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574,  -609,  -873, -1574, -1574, -1574, -1574,
   -1574,   959, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1574,   471, -1574, -1574, -1574, -1574, -1574, -1574, -1574,
   -1574, -1077, -1574,  2449,    26, -1574,  1224,  -407, -1574, -1574,
    -479,  3687,  1905, -1574, -1574, -1574,   550,    55,  -598, -1574,
   -1574,   621,   421,  -574, -1574,   422, -1574, -1574, -1574, -1574,
   -1574,   606, -1574, -1574, -1574,    73,  -871,  -142,  -433,  -430,
   -1574,   672,  -108, -1574, -1574,    40,    43,   745, -1574, -1574,
     159,   -18, -1574,  -361,    42,  -367,   126,   -78, -1574, -1574,
    -463,  1246, -1574, -1574, -1574, -1574, -1574,   738,   895, -1574,
   -1574, -1574,  -360,  -665, -1574,  1213, -1157, -1574,   -58,  -185,
     -69,   790, -1574, -1064,   201,  -193, -1574,   517,   591, -1574,
   -1574, -1574, -1574,   542,  -234,   308, -1111
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1023
static const yytype_int16 yytable[] =
{
     181,   183,   509,   185,   186,   187,   189,   190,   191,   481,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   246,   333,   222,   225,   657,   431,   904,   118,
     353,   402,   656,   658,   120,   251,   937,   391,  1327,   249,
     255,   394,   395,   122,   531,   775,   630,   257,   789,   260,
     123,   503,   342,   936,   345,   387,  1141,   232,   388,   404,
     480,   724,   865,   870,  1313,   341,   427,   764,   941,  1149,
     765,   237,   340,   239,   238,  1024,   974,   154,   249,   917,
     248,  1324,   431,   586,   588,   401,   406,   564,   565,  1038,
     689,  1163,  1216,   421,   819,  1603,  1012,  1385,   535,   403,
      14,  -878,   799,   543,   -68,   800,   -33,  1174,   332,   -68,
     598,   -33,  1760,   -32,   603,   -95,   -96,   543,   -32,   377,
     -95,   -96,  1546,  1548,  -348,  1611,  1698,   501,  1767,  1767,
    1603,  1447,   404,   536,   498,   499,   817,   923,   876,  1761,
    -884,   506,    14,   543,   378,   893,   893,  1205,    14,    14,
     354,   501,   893,   893,   893,   373,  1784,   378,   401,   406,
     594,  1150,  -877,  1778,   639,   794,  -699,   520,   375,  1842,
    1452,   -98,   403,  1328,   771,   772,  1094,     3,   518,   512,
    1319,   360,   184,   529,  1755,   -98,   219,   219,  -706,   361,
    1250,   498,   499,   519,   406,    14,   916,   418,  1890,   885,
     886,  -889,  1756,   796,   242,  1005,  1151,   403,  1779,   528,
    1904,   119,  1320,  1827,  1843,  1453,   381,   382,  -585,   595,
    -876,  1757,  -581,   403,  -879,  -919,  -878,  -707,  -701,   381,
     382,  -883,   506,   409,  -880,  -531,   633,   538,  -922,   392,
     538,  -792,   380,  1891,  -792,   924,   245,   249,   549,  -921,
    -882,  -585,   502,   694,  -863,  1905,   125,   820,  -864,  1386,
     925,  1329,   969,  1461,   355,  -884,   507,  1518,  1107,  1459,
    1467,   540,  1469,   562,   683,   545,   502,   374,   422,   526,
    1604,  1605,  1177,  -287,   562,   562,  1378,  -877,   544,   -68,
     376,   -33,  -287,  -271,  1152,   599,  1762,  1488,   -32,   604,
     -95,   -96,   631,  1454,  1223,   577,  1227,  1547,  1549,  -348,
    1612,  1699,  1347,  1768,  1817,  1885,   353,   353,   589,   396,
     818,   609,   877,   564,   565,  1364,  1160,   878,  -790,   894,
     987,  1892,   430,   776,  1103,  1104,  1301,  1494,  1553,  -792,
     505,   882,  -887,  1906,  -888,  -876,  -886,   608,   612,  -879,
    -919,   206,    40,  -890,  1133,   642,  -883,   507,   582,  -880,
     249,   403,   632,  -922,   338,  1606,   692,   222,   620,   627,
     249,   627,  -893,   505,  -921,  -882,   637,   885,   886,  -863,
     219,   206,    40,  -864,   431,  1225,  1226,   510,   333,  1711,
     332,  1712,  -994,   -97,   728,   189,  1568,   362,  1570,  1576,
    -851,   607,  -586,   681,  1560,   252,   482,   -97,   498,   499,
     745,   628,  1196,   418,  -851,   627,  1897,  1911,   583,   391,
     740,   741,   427,   769,   253,   397,   498,   499,   773,  1120,
    -994,   398,  1225,  1226,   254,   695,   696,   697,   698,   700,
     701,   702,   703,   704,   705,   706,   707,   708,   709,   710,
     711,   712,   713,   714,   715,   716,   717,   718,   719,   720,
     721,   722,   723,   112,   725,   107,   726,   726,   729,  1312,
     343,  -700,   667,   332,   364,  1228,  1197,  1375,   748,   749,
     750,   751,   752,   753,   754,   755,   756,   757,   758,   759,
     760,   747,  1561,   408,   232,   107,   726,   770,   746,   627,
     627,   726,   774,  1724,  1898,  1912,   748,  1729,   237,   778,
     239,   238,   258,  1156,   219,   331,  1157,   378,   786,   468,
     788,   860,  1388,   219,   639,   611,   734,   944,   627,   946,
     219,   469,   368,  1507,  1334,   481,   807,  1336,   808,  1326,
     594,   219,   860,  -587,   378,    37,   358,  1018,  1019,   806,
     968,   379,   389,   359,   368,  1224,  1225,  1226,   368,   368,
     869,   869,   657,   417,   498,   499,   976,    50,   656,   658,
    1179,   811,   782,   332,   793,  1382,  1225,  1226,   865,   560,
     861,   561,   980,   368,  1207,  1208,   480,  1296,  1297,   381,
     382,   872,  1538,  1100,   392,  1101,   644,   645,   378,   417,
     736,   417,  -994,   365,  1524,   639,   927,   498,   499,  1514,
    1515,  1314,   366,  1583,   119,   380,   381,   382,   378,   901,
    1359,  1361,  -891,   418,  1315,  1367,   768,   367,    90,    91,
    -702,    92,   176,    94,   371,   684,   958,  -994,   568,  1171,
    -994,   944,   946,  1316,  1808,  1809,   403,   736,  1013,   946,
    1725,  1726,   516,  1763,    37,   372,  1539,   531,   795,   125,
     393,   801,  1810,   417,  1783,  -891,  1095,   219,  1786,   640,
     381,   382,  1764,   823,   803,  1765,    50,   899,  1468,  1804,
    1805,  1811,  1014,   906,  1812,   416,   378,   420,   743,   634,
     381,   382,  -854,   410,   959,   423,   657,    37,  1525,   909,
    1349,   910,   656,   658,  -852,   112,  -854,   432,   930,   112,
    1463,  1526,   433,   550,  1527,  1451,   853,   378,  -852,    50,
    1871,  1872,  1873,    37,   413,   434,   688,   435,   967,   338,
      37,   175,   436,   824,    88,  1528,   667,    90,    91,   437,
      92,  1529,    94,  1886,  1887,    50,   465,   466,   467,   158,
     468,   438,    50,   585,   587,  1377,   439,   979,   381,   382,
    1048,  1051,   469,  1882,  -582,  -583,  1183,  1184,  1185,    37,
     471,   472,   218,   220,  1551,   473,   474,  1474,  1896,  1475,
      90,    91,   219,    92,   176,    94,   966,   378,   504,   381,
     382,    50,  1016,  -584,   639,  -700,  -885,   576,  1022,   938,
     175,    37,   508,    88,  1880,   385,    90,    91,   249,    92,
     176,    94,   513,    90,    91,   469,    92,   176,    94,  1893,
    1090,  1091,  1092,    50,   515,   374,   869,   376,   869,   418,
     521,   657,   869,   869,  1105,  -889,  1093,   656,   658,  1441,
    1340,   691,   505,  1033,   524,  1577,   412,   414,   415,   405,
     525,  1350,    90,    91,  -698,    92,   176,    94,  1607,   381,
     382,   533,   532,   541,   554, -1022,   112,   566,   569,   570,
     822,   578,   579,   591,   590,   593,   685,  1040,   175,   600,
     331,    88,  1045,   368,    90,    91,   601,    92,   176,    94,
     602,  1125,   613,  1126,   614,    37,   659,   660,  1129,   669,
     808,   682,   670,   219,  1579,  -119,  1580,   671,  1581,  1131,
    1582,   673,    55,   691,   779,   781,  1792,    50,  1178,   634,
     783,   784,   405,  1140,   790,   346,   347,   791,   809,   813,
     543,   816,   482,   576,   368,   738,   368,   368,   368,   368,
    1021,   562,   118,  1115,   219,  1023,   219,   120,   830,   829,
      37,   854,   206,    40,   857,  1166,   122,   405,  1167,   763,
    1168,   856,   859,   123,   627,   858,   522,   933,   934,  1859,
    1332,   875,    50,   530,   219,   348,   942,   879,    90,    91,
     576,    92,   176,    94,   734,   880,   214,   214,   884,  1859,
     154,   158,   883,   890,   798,   158,   892,   897,  1881,   895,
     898,    37,   902,   232,  1733,   112,  1204,  1210,   903,  1465,
     119,   905,   907,   667,   908,   914,   919,   237,   920,   239,
     238,   922,  -722,    50,   929,   928,   667,   932,   931,   851,
     935,   761,   939,    90,    91,   219,    92,   176,    94,   657,
     940,   948,   950,   951,  1211,   656,   658,   952,   955,   964,
    1303,   219,   219,   871,   685,   125,   961,   962,  1142,   965,
     119,  1200,   973,   957,   762,   981,   107,   983,   984,  -704,
     768,   985,   801,  1015,  1025,  1035,   869,  1037,  1039,  1041,
    1042,   385,  1043,  1044,    90,    91,  1046,    92,   176,    94,
    1059,   911,   913,    62,    63,    64,   171,   172,   428,  1060,
     597,  1063,  1066,  1062,  1061,   125,  1237,  1065,  1098,   605,
    1106,   610,  1110,  1241,  1249,   386,   617,  1255,  1305,   657,
    1108,    37,  1112,  1309,   119,   656,   658,   638,  1565,  1114,
    1113,  1304,  1119,  1123,  1130,    37,  1136,   688,   688,   801,
    1138,  1139,  1147,    50,  1145,   119,  1143,  1161,  1170,  1173,
    1176,  1182,   158,   368,  1181,  -892,  1192,    50,   118,  1193,
    1787,  1788,   429,   120,  1194,   627,  1195,  1222,  1867,   125,
    1198,    37,   122,  1199,  1201,  1218,   627,  1305,  1219,   123,
     214,  1213,  1823,  1215,   219,   219,  1221,  1230,  1231,    55,
     125,  1235,  1093,    50,  1236,  1239,  1370,    62,    63,    64,
     171,   172,   428,  1240,    90,    91,   154,    92,   176,    94,
     249,  1288,   175,  1290,  1291,    88,    89,  1293,    90,    91,
    1384,    92,   176,    94,  1299,  1322,  1330,  1134,   119,  1302,
     119,   969,   957,  1331,   667,  1373,  1333,   667,  1000,  1323,
    1008,  1335,  1341,  1144,  1351,  1337,  1354,  1339,  1353,   994,
    1345,   216,   216,   617,    90,    91,  1158,    92,   176,    94,
    1870,  1342,   112,  1344,  1352,  1363,   429,  1369,    37,  1371,
     206,    40,  1372,   125,   635,   125,  1031,   112,   641,  1374,
    1379,  1175,  1383,   219,  1389,  1391,   219,  1393,  1394,  1399,
      50,   158,  1397,  1403,  1406,  1552,  1398,  1405,  1411,  1400,
    1408,  1402,  1443,  1404,  1445,   635,  1407,   641,   635,   641,
     641,  1448,   112,  1412,   214,  1449,  1455,  1450,  1410,  1102,
     685,  1417,  1415,   214,  1418,  1457,  1456,  1444,  1460,  1471,
     214,  1458,  1470,  1464,   627,  1462,  1466,   431,  1472,    37,
     119,   214,  1478,  1477,  1473,  1476,  1480,  1229,  1116,   761,
    1232,    90,    91,  1481,    92,   176,    94,  1482,  1484,  1485,
    1486,    50,   219,  1489,  1491,  1496,  1492,  1709,   900,   862,
     863,  1493,  1511,  1440,  1497,  1423,   112,   219,   219,  1522,
    1535,  1556,   797,  1550,   107,   125,  1440,  1562,  1563,  1566,
    1571,   576,  1572,  1574,  1592,  1578,  1595,   112,  1601,  1609,
    1610,  1706,  1707,   763,  1713,   798,  1719,  1720,  1766,  1722,
     667,  1732,  1723,  1479,  1734,  1735,  1772,  1483,  1776,   864,
      14,  1487,    90,    91,  1416,    92,   176,    94,  1745,   119,
      37,  1781,  1746,  1798,  1775,   368,    62,    63,    64,   171,
     172,   428,  1800,  1802,  1806,   216,  1814,  1186,  1186,  1000,
    1564,  1815,    50,   627,  1816,  1325,  1822,   942,  1600,  1821,
    1826,   219,  1829,    37,  -344,  1830,  1832,   214,  1833,  1835,
    1837,  1761,   798,  1838,   125,  1841,  1847,  1854,   112,  1844,
     112,  1343,   112,  1424,  1346,    50,    37,  1848,  1425,   978,
      62,    63,    64,   171,  1426,   428,  1427,    37,  1849,  1856,
    1861,  1865,  1233,  1868,  1869,   429,  1490,  1879,    50,  1877,
    1894,  1700,  1899,    90,    91,  1701,    92,   176,    94,    50,
    1589,   576,  1499,  1883,   576,  1884,  1900,  1907,  1908,  1913,
    1009,  1914,  1010,  1916,  1440,  1428,  1429,  1774,  1430,  1390,
    1440,  1539,  1440,  1158,   425,   667,    90,    91,   158,    92,
     176,    94,  1435,   851,  1917,  1292,  1864,  1721,   737,   429,
    1029,  1440,   742,   158,  1172,  1435,   348,  1431,   739,    90,
      91,  1602,    92,   176,    94,  1135,  1878,   864,  1376,   216,
      90,    91,   214,    92,   176,    94,  1739,  1498,   216,  1876,
     873,  1754,   112,  1608,  1730,   216,  1759,  1542,   158,  1901,
    1889,   219,  1523,  1771,  1420,  1421,   216,  1728,  1317,  1242,
    1188,  1737,  1589,  1355,  1202,  1356,  1155,   655,   619,  1049,
    1513,  1111,  1839,   119,  1585,   462,   463,   464,   465,   466,
     467,  1295,   468,  1591,   690,  1234,  1287,   617,  1122,     0,
       0,  1544,     0,     0,   469,     0,  1597,     0,     0,  1440,
       0,     0,  1000,  1000,  1000,     0,     0,     0,  1000,     0,
       0,     0,   158,     0,  1423,     0,     0,     0,   125,   112,
    1087,  1088,  1089,  1090,  1091,  1092,  1769,     0,     0,     0,
       0,   112,     0,   158,     0,     0,     0,     0,  1852,  1093,
       0,     0,   482,     0,     0,     0,     0,   119,     0,  1500,
       0,  1501,     0,   214,     0,     0,   119,     0,     0,    14,
       0,     0,     0,  1435,  1777,     0,  1819,     0,     0,  1435,
       0,  1435,     0,  1740,    62,    63,    64,    65,    66,   428,
       0,     0,   216,   213,   213,    72,   475,   229,     0,     0,
    1435,     0,   125,  1545,   214,     0,   214,     0,     0,     0,
       0,   125,     0,     0,  1419,   431,     0,     0,     0,  1714,
       0,   332,   229,     0,   158,     0,   158,     0,   158,     0,
    1029,  1217,  1424,   476,   214,   477,     0,  1425,     0,    62,
      63,    64,   171,  1426,   428,  1427,     0,     0,   478,     0,
     479,     0,   119,   429,     0,     0,     0,     0,   119,     0,
       0,     0,     0,     0,   119,  1799,  1801,     0,     0,     0,
    1000,     0,  1000,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1428,  1429,     0,  1430,  1435,     0,
       0,     0,     0,     0,     0,   214,     0,   125,    37,     0,
       0,     0,     0,   125,     0,     0,     0,   216,   429,   125,
       0,   214,   214,     0,     0,     0,  1446,     0,     0,     0,
      50,  1782,     0,     0,     0,     0,     0,     0,     0,  1306,
       0,  1789,  1308,     0,     0,   112,     0,     0,   158,     0,
       0,     0,  1525,   331,     0,     0,     0,     0,     0,  1540,
       0,     0,     0,     0,  1909,  1526,     0,     0,  1527,     0,
       0,     0,  1915,     0,     0,     0,     0,     0,  1918,     0,
       0,  1919,     0,   667,     0,   175,  1824,  1750,    88,    89,
       0,    90,    91,     0,    92,  1529,    94,   213,     0,     0,
       0,     0,     0,   667,     0,     0,     0,  1000,     0,  1000,
       0,  1000,   667,  1000,     0,     0,     0,   119,  1368,   112,
       0,     0,     0,     0,   112,   158,     0,     0,   112,     0,
       0,  1846,     0,   617,  1029,     0,     0,   158,   216,     0,
       0,     0,     0,     0,  1423,   368,     0,   229,   576,   229,
       0,   331,     0,     0,   214,   214,     0,     0,     0,   119,
       0,  1695,   125,     0,     0,     0,   119,     0,  1702,     0,
       0,     0,     0,     0,     0,   331,     0,   331,     0,   216,
       0,   216,   271,     0,   331,     0,     0,     0,     0,    14,
       0,  1773,  1895,     0,     0,     0,     0,     0,     0,  1902,
       0,   119,     0,     0,   125,     0,   229,  1000,     0,   216,
     273,   125,     0,     0,   112,   112,   112,   617,     0,     0,
     112,     0,     0,   119,     0,     0,   112,     0,     0,     0,
       0,   213,    37,    62,    63,    64,    65,    66,   428,     0,
     213,     0,     0,     0,    72,   475,   125,   213,     0,     0,
       0,     0,  1424,   214,    50,  1853,   214,  1425,   213,    62,
      63,    64,   171,  1426,   428,  1427,     0,     0,   125,   229,
     216,     0,   119,     0,     0,     0,  1834,     0,     0,   119,
    1423,     0,     0,     0,   477,     0,   216,   216,     0,   556,
     557,   558,   559,     0,   229,     0,     0,   229,     0,     0,
       0,     0,   429,     0,  1428,  1429,     0,  1430,     0,   175,
       0,     0,    88,   325,     0,    90,    91,   125,    92,   176,
      94,     0,  1047,   655,   125,    14,     0,     0,   429,     0,
       0,   158,   214,   329,     0,     0,  1569,     0,   576,     0,
       0,     0,     0,   330,   229,     0,     0,   214,   214,     0,
       0,     0,   942,     0,     0,     0,     0,  1554,     0,   331,
       0,     0,     0,  1000,  1000,     0,     0,   942,     0,   112,
       0,     0,     0,     0,     0,     0,     0,  1423,  1793,     0,
       0,     0,     0,     0,   213,  1695,  1695,     0,  1424,  1702,
    1702,     0,     0,  1425,     0,    62,    63,    64,   171,  1426,
     428,  1427,     0,   368,     0,   158,     0,     0,     0,     0,
     158,   112,     0,     0,   158,     0,     0,     0,   112,   216,
     216,     0,    14,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,  1423,     0,     0,   229,     0,
    1428,  1429,   229,  1430,     0,   842,     0,     0,     0,     0,
       0,     0,     0,   112,     0,     0,     0,   655,     0,     0,
       0,  1851,     0,     0,   429,  1423,     0,     0,     0,     0,
       0,     0,  1573,     0,     0,   112,     0,     0,   842,     0,
      14,  1866,     0,     0,     0,  1424,     0,     0,     0,     0,
    1425,     0,    62,    63,    64,   171,  1426,   428,  1427,   213,
     158,   158,   158,     0,     0,     0,   158,     0,     0,     0,
      14,     0,   158,     0,     0,     0,     0,     0,   216,     0,
       0,   216,     0,     0,   112,     0,     0,     0,     0,     0,
       0,   112,     0,     0,     0,   229,   229,  1428,  1429,     0,
    1430,     0,     0,  1424,   229,     0,     0,     0,  1425,     0,
      62,    63,    64,   171,  1426,   428,  1427,     0,     0,     0,
       0,   429,     0,     0,     0,     0,     0,     0,     0,  1575,
       0,   214,     0,  1424,     0,     0,     0,     0,  1425,     0,
      62,    63,    64,   171,  1426,   428,  1427,     0,     0,     0,
       0,     0,   655,     0,     0,  1428,  1429,   216,  1430,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   216,   216,     0,     0,     0,     0,     0,   429,
     213,     0,     0,     0,     0,  1428,  1429,  1584,  1430,     0,
   -1023, -1023, -1023, -1023, -1023,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,     0,   849,     0,   215,   215,     0,   429,
     231,     0,   496,   497,     0,   158,  1093,  1731,     0,     0,
       0,   213,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   874,   511,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   213,   842,     0,     0,     0,   216,   158,     0,     0,
       0,     0,     0,     0,   158,   229,   229,   842,   842,   842,
     842,   842,     0,     0,     0,     0,     0,   842,     0,   498,
     499,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     229,   496,   497,     0,     0,     0,     0,     0,     0,   158,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1067,
    1068,  1069,   213,     0,     0,     0,     0,     0,     0,     0,
       0,   158,     0,     0,     0,     0,   229,     0,   213,   213,
    1070,     0,     0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,  1091,  1092,   229,   229,     0,   498,   499,
     655,     0,     0,     0,     0,   229,     0,     0,  1093,     0,
     158,   229,     0,     0,     0,     0,     0,   158,     0,     0,
       0,     0,     0,     0,   229,     0,     0,     0,   842,     0,
       0,   229,     0,     0,     0,     0,   216,     0,     0,     0,
     215,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,     0,     0,   229,     0,     0,     0,   672,   511,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     655,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1030,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,   213,     0,     0,     0,  1052,  1053,  1054,  1055,
       0,   496,   497,     0,     0,   229,  1064,     0,   229,     0,
     229,     0,     0,     0,     0,     0,     0,    34,    35,    36,
       0,     0,  1253,     0,     0,   842,   842,   842,     0,   229,
     207,     0,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
       0,     0,     0,     0,   215,     0,     0,     0,   498,   499,
       0,     0,     0,   215,     0,     0,   842,   988,   989,     0,
     215,    78,    79,    80,    81,    82,     0,     0,     0,     0,
     213,   215,   211,   213,     0,     0,     0,   990,    86,    87,
       0,     0,   215,     0,     0,   991,   992,   993,    37,     0,
       0,     0,    96,   229,     0,   229,     0,  1159,   994,     0,
       0,     0,     0,     0,     0,     0,   101,   792,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,     0,   229,     0,     0,     0,   511,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,     0,
       0,     0,     0,     0,   229,   995,   996,   997,     0,   213,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
     998,     0,     0,     0,   213,   213,     0,   842,     0,     0,
       0,    90,    91,     0,    92,   176,    94,   229,     0,   496,
     497,   229,     0,     0,   842,     0,   842,     0,     0,   999,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,     0,     0,  1245,  1247,     0,   842,     0,
       0,  1256,  1259,  1260,  1261,  1263,  1264,  1265,  1266,  1267,
    1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,  1276,  1277,
    1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,     0,
       0,   334,   229,   229,     0,   229,   498,   499,   213,     0,
       0,     0,     0,     0,     0,  1294,     0,     0,   847,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   271,   511,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   847,     0,     0,     0,     0,   273,     0,     0,     0,
       0,     0,     0,     0,     0,   881,     0,     0,   271,     0,
       0,     0,   215,     0,     0,     0,     0,     0,    37,     0,
       0,     0,   496,   497,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   273,   229,     0,   229,
      50,     0,     0,     0,     0,   842,     0,   842,   567,   842,
       0,   842,   229,     0,     0,   842,     0,   842,    37,     0,
     842,     0,     0,     0,     0,     0,  1380,     0,     0,     0,
     229,   229,     0,     0,   229,   556,   557,   558,   559,     0,
      50,   229,     0,  1395,     0,  1396,     0,     0,   213,   498,
     499,     0,     0,     0,     0,   175,     0,     0,    88,   325,
       0,    90,    91,     0,    92,   176,    94,  1413,     0,     0,
       0,     0,     0,     0,     0,   556,   557,   558,   559,   329,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   330,
       0,     0,   229,   215,     0,   175,     0,     0,    88,   325,
       0,    90,    91,     0,    92,   176,    94,     0,   842,     0,
       0,     0,     0,     0,     0,     0,   334,     0,   334,   329,
     229,   229,   271,     0,     0,     0,     0,     0,   229,   330,
     229,     0,     0,     0,   215,     0,   215, -1023, -1023, -1023,
   -1023, -1023,   460,   461,   462,   463,   464,   465,   466,   467,
     273,   468,   229,     0,   229,     0,     0,     0,     0,     0,
       0,   229,     0,   469,   215,   847,     0,     0,     0,     0,
       0,     0,    37,     0,     0,   334,     0,     0,     0,     0,
     847,   847,   847,   847,   847,     0,     0,     0,     0,     0,
     847,     0,     0,     0,    50,     0,     0,     0,     0,     0,
     842,   842,   842,  1097,  1503,     0,  1504,   842,  1505,   229,
    1506,     0,     0,     0,  1508,   229,  1509,   229,     0,  1510,
     271,     0,     0,     0,     0,   215,     0,     0,     0,   556,
     557,   558,   559,     0,     0,     0,     0,     0,     0,  1118,
       0,   215,   215,     0,     0,     0,     0,     0,   273,   175,
       0,     0,    88,   325,     0,    90,    91,     0,    92,   176,
      94,     0,  1392,   334,     0,     0,   334,     0,     0,  1118,
      37,     0,     0,   329,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   330,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
    -391,   847,     0,     0,  1162,     0,     0,  1593,    62,    63,
      64,   171,   172,   428,     0,     0,     0,     0,     0,   229,
       0,     0,     0,     0,     0,     0,   231,   556,   557,   558,
     559,     0,     0,     0,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   175,     0,     0,
      88,   325,     0,    90,    91,   229,    92,   176,    94,     0,
       0,   842,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   329,   842,     0,   215,   215,     0,   429,   842,     0,
       0,   330,   842,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1741,
    1742,  1743,     0,     0,   229,     0,  1747,   334,   847,   847,
     847,   825,   215,     0,   843,   847,   847,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,   847,   847,   847,   847,   847,   847,   847,
     847,   847,   847,     0,   842,     0,     0,   843,     0,     0,
       0,     0,     0,     0,   229,     0,     0,     0,     0,   847,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,   215,     0,     0,   215,     0,     0,     0,
     229,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   229,   440,   441,   442,     0,
       0,     0,     0,     0,   334,   334,     0,     0,     0,     0,
       0,     0,     0,   334,     0,     0,   443,   444,     0,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,     0,   468,     0,     0,     0,   215,     0,     0,
       0,     0,   215,     0,     0,   469,     0,     0,     0,     0,
    1803,     0,     0,     0,     0,     0,     0,   215,   215,     0,
     847,  1813,     0,     0,     0,     0,     0,  1818,     0,     0,
       0,  1820,     0,     0,     0,     0,     0,   847,     0,   847,
       0,     0,     0,     0,     0,  1067,  1068,  1069,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   847,     0,     0,   217,   217,  1070,     0,   235,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,     0,     0,  1855,     0,     0,     0,     0,  1422,     0,
       0,   215,     0,     0,  1093,     0,     0,     0,     0,     0,
       0,   843,     0,     0,     0,     0,     0,   440,   441,   442,
       0,     0,     0,     0,   334,   334,   843,   843,   843,   843,
     843,     0,     0,     0,     0,  1310,   843,   443,   444,     0,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,     0,     0,     0,     0,     0,
       0,  1238,     0,     0,     0,     0,   469,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   847,   468,
     847,     0,   847,     0,   847,   215,     0,     0,   847,     0,
     847,   469,     0,   847,   334,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1521,     0,     0,  1534,     0,     0,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,   334,     0,     0,     0,   843,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   334,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   847,     0,   915,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1598,  1599,     0,     0,     0,     0,     0,
       0,     0,     0,  1534,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,     0,   334,     0,   825,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   843,   843,   843,     0,     0,     0,
       0,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,     0,
       0,     0,   217,   847,   847,   847,     0,     0,     0,     0,
     847,   217,  1748,     0,     0,   843,     0,     0,   217,     0,
    1534,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     235,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   334,     0,   334,     0,     0,     0,     0,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   334,   443,
     444,   334,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,     0,     0,
       0,     0,     0,     0,     0,   235,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,   843,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   334,     0,     0,     0,
     334,     0,     0,   843,     0,   843,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,     0,     0,     0,     0,
       0,     0,     0,     0,   847,     0,     0,   843,     0,     0,
       0,     0,     0,     0,     0,   847,     0,     0,     0,     0,
       0,   847,     0,     0,     0,   847,     0,     0,     0,     0,
       0,     0,     0,     0,   440,   441,   442,     0,     0,     0,
       0,   334,   334,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   443,   444,   848,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
       0,   468,     0,     0,     0,   947,     0,   847,     0,   848,
       0,     0,     0,   469,     0,     0,     0,  1863,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,  1521,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,   334,     0,   334,     0,
       0,     0,     0,     0,   843,     0,   843,     0,   843,  1093,
     843,     0,     0,     0,   843,     0,   843,     0,     0,   843,
       0,     0,   440,   441,   442,     0,     0,     0,     0,   334,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     334,     0,   443,   444,     0,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,     0,   468,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     982,   469,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   843,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   334,   468,     0,
       0,     0,   217,     0,   217,     0,     0,     0,     0,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,     0,   334,     0,     0,     0,     0,     0,     0,
     334,     0,   217,   848,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   848,   848,
     848,   848,   848,     0,     0,     0,     0,     0,   848,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   843,
     843,   843,     0,     0,     0,     0,   843,     0,   986,     0,
       0,     0,     0,     0,   334,     0,     0,     0,     0,     0,
       0,     0,     0,   217,     0,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
     217,     0,     0,     0,     0,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,   235,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   848,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   334,   468,
       0,     0,     0,     0,   235,     0,     0,     0,     0,     0,
       0,   469,     0,     0,     0,   334,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1794,     0,     0,     0,     0,     0,
     843,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,   843,   217,   217,     0,     0,     0,   843,     0,     0,
       0,   843,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,     0,
     468,     0,     0,   334,     0,     0,   848,   848,   848,     0,
     235,  1109,   469,   848,   848,   848,   848,   848,   848,   848,
     848,   848,   848,   848,   848,   848,   848,   848,   848,   848,
     848,   848,   848,   848,   848,   848,   848,   848,   848,   848,
     848,     0,     0,   843,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   848,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,   217,     0,   440,   441,   442,   334,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,   443,   444,     0,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,     0,   468,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   469,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   235,    10,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     400,    12,    13,     0,     0,   217,   217,     0,   848,     0,
       0,   744,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,   848,     0,   848,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,   848,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,   217,
       0,     0,    62,    63,    64,   171,   172,   173,     0,     0,
      69,    70,  1169,     0,     0,     0,     0,     0,     0,     0,
     174,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,   175,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   176,    94,     0,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   104,     0,     0,     0,     0,
     107,   108,     0,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   848,     0,   848,     0,
     848,     0,   848,   235,     0,     0,   848,     0,   848,     0,
       0,   848,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,   235,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,   848,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,    83,     0,     0,    84,     0,     0,
       0,     0,    85,    86,    87,    88,    89,     0,    90,    91,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,   848,   848,   848,     0,     0,    98,    99,   848,   100,
       0,   101,   102,   103,     0,     0,   104,  1753,   105,   106,
    1132,   107,   108,     0,   109,   110,     0,   440,   441,   442,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,     0,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,     0,     0,   440,   441,   442,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,     0,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   848,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,   848,     0,     0,     0,     0,     0,   848,
       0,     0,     0,   848,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,  1836,    29,    30,
      31,    32,    33,  1180,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,   848,    52,    53,    54,    55,
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,  1206,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,    83,     0,
       0,    84,     0,     0,     0,     0,    85,    86,    87,    88,
      89,     0,    90,    91,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,    99,     0,   100,     0,   101,   102,   103,     0,     0,
     104,     0,   105,   106,  1311,   107,   108,     0,   109,   110,
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
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,    83,     0,     0,
      84,     0,     0,     0,     0,    85,    86,    87,    88,    89,
       0,    90,    91,     0,    92,    93,    94,    95,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
      99,     0,   100,     0,   101,   102,   103,     0,     0,   104,
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
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    84,
       0,     0,     0,     0,   175,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   176,    94,    95,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   104,     0,
     105,   106,   674,   107,   108,     0,   109,   110,     5,     6,
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
       0,     0,     0,   175,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   176,    94,    95,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,     0,   105,
     106,  1096,   107,   108,     0,   109,   110,     5,     6,     7,
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
       0,     0,   175,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   176,    94,    95,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   104,     0,   105,   106,
    1146,   107,   108,     0,   109,   110,     5,     6,     7,     8,
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
      74,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    84,     0,     0,     0,
       0,   175,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   176,    94,    95,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   104,     0,   105,   106,  1212,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,  1214,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,    83,     0,     0,    84,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    95,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
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
       0,    49,  1381,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,    83,     0,     0,    84,     0,     0,     0,     0,   175,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   176,
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
      83,     0,     0,    84,     0,     0,     0,     0,   175,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   176,    94,
      95,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,   105,   106,  1512,   107,   108,     0,
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
       0,     0,    84,     0,     0,     0,     0,   175,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   176,    94,    95,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   104,     0,   105,   106,  1744,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,  1790,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,    83,     0,
       0,    84,     0,     0,     0,     0,   175,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   176,    94,    95,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     104,     0,   105,   106,     0,   107,   108,     0,   109,   110,
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
      84,     0,     0,     0,     0,   175,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   176,    94,    95,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   104,
       0,   105,   106,  1825,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  1828,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    84,
       0,     0,     0,     0,   175,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   176,    94,    95,     0,     0,    96,
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
       0,     0,     0,   175,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   176,    94,    95,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,     0,   105,
     106,  1845,   107,   108,     0,   109,   110,     5,     6,     7,
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
       0,     0,   175,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   176,    94,    95,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   104,     0,   105,   106,
    1862,   107,   108,     0,   109,   110,     5,     6,     7,     8,
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
      74,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    84,     0,     0,     0,
       0,   175,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   176,    94,    95,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   104,     0,   105,   106,  1903,
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
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    95,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   104,     0,   105,   106,  1910,   107,
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
       0,    83,     0,     0,    84,     0,     0,     0,     0,   175,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   176,
      94,    95,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   104,     0,   105,   106,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   539,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   171,   172,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
      83,     0,     0,    84,     0,     0,     0,     0,   175,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   176,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   104,     0,   105,   106,     0,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   810,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   171,   172,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,    83,
       0,     0,    84,     0,     0,     0,     0,   175,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   176,    94,     0,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   104,     0,   105,   106,     0,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1032,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     171,   172,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,    83,     0,
       0,    84,     0,     0,     0,     0,   175,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   176,    94,     0,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     104,     0,   105,   106,     0,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1588,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   171,
     172,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,    83,     0,     0,
      84,     0,     0,     0,     0,   175,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   176,    94,     0,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   104,
       0,   105,   106,     0,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,  1736,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,   171,   172,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    84,
       0,     0,     0,     0,   175,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   176,    94,     0,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   104,     0,
     105,   106,     0,   107,   108,     0,   109,   110,     5,     6,
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
       0,    59,     0,    61,    62,    63,    64,   171,   172,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,    83,     0,     0,    84,     0,
       0,     0,     0,   175,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   176,    94,     0,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,     0,   105,
     106,     0,   107,   108,     0,   109,   110,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   171,   172,   173,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   174,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,   175,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   176,    94,     0,     0,     0,    96,     0,     0,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,  1068,  1069,   177,     0,   339,     0,
       0,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,  1070,     0,    10,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,   623,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1093,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   171,   172,   173,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     174,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,   175,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   176,    94,     0,   624,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   177,     0,     0,     0,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   171,   172,   173,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   174,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,     0,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,  1069,   177,     0,     0,   805,     0,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,  1070,     0,    10,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,  1127,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1093,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   171,   172,   173,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   174,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,   175,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   176,
      94,     0,  1128,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   177,     0,     0,     0,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   400,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   171,   172,   173,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   174,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,   175,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   176,    94,
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
       0,     0,    50,     0,     0,     0,     0,   188,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   171,   172,   173,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   174,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,   175,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   176,    94,     0,
       0,     0,    96,     0,     0,    97,     0,     0,     0,  1558,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   177,     0,     0,     0,     0,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,  1070,
       0,    10,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     171,   172,   173,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   174,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,   175,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   176,    94,     0,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     177,   440,   441,   442,     0,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   443,   444,     0,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,     0,   468,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     469,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   171,
     172,   173,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   174,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,   175,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   176,    94,     0,     0,     0,
      96,     0,     0,    97,     0,     0,     0,  1559,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   177,
       0,   256,   441,   442,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   443,   444,     0,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,     0,   468,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
     469,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   171,   172,
     173,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   174,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,   175,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   176,    94,     0,     0,     0,    96,
       0,     0,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   177,     0,
     259,     0,     0,   107,   108,     0,   109,   110,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   400,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   171,   172,   173,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   174,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,   175,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   176,    94,     0,     0,     0,    96,     0,
       0,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   104,   440,   441,
     442,     0,   107,   108,     0,   109,   110,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   443,   444,
    1385,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   469,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   171,   172,   173,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   174,    75,    76,    77,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,   175,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   176,    94,     0,     0,     0,    96,     0,     0,
      97,     0,  1386,     0,     0,     0,    98,     0,     0,     0,
       0,   101,   102,   103,     0,     0,   177,   537,     0,     0,
       0,   107,   108,     0,   109,   110,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0, -1023, -1023,
   -1023, -1023,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   699,   468,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   171,   172,   173,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     174,    75,    76,    77,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,   175,    86,    87,    88,    89,     0,    90,    91,     0,
      92,   176,    94,     0,     0,     0,    96,     0,     0,    97,
       0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
     101,   102,   103,     0,     0,   177,     0,     0,     0,     0,
     107,   108,     0,   109,   110,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
     744,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1093,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   171,   172,   173,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   174,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,     0,     0,     0,    96,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,     0,   101,
     102,   103,     0,     0,   177,     0,     0,     0,     0,   107,
     108,     0,   109,   110,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,     0,   785,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   171,   172,   173,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   174,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,   175,
      86,    87,    88,    89,     0,    90,    91,     0,    92,   176,
      94,     0,     0,     0,    96,     0,     0,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,     0,   101,   102,
     103,     0,     0,   177,     0,     0,     0,     0,   107,   108,
       0,   109,   110,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,     0,     0,   787,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   171,   172,   173,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   174,    75,    76,
      77,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,   175,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   176,    94,
       0,     0,     0,    96,     0,     0,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,     0,   101,   102,   103,
       0,     0,   177,     0,     0,     0,     0,   107,   108,     0,
     109,   110,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,     0,     0,     0,  1124,     0,     0,
       0,     0,     0,     0,     0,  1093,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   171,   172,   173,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   174,    75,    76,    77,
       0,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,   175,    86,    87,
      88,    89,     0,    90,    91,     0,    92,   176,    94,     0,
       0,     0,    96,     0,     0,    97,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,   101,   102,   103,     0,
       0,   177,     0,     0,     0,     0,   107,   108,     0,   109,
     110,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,     0,     0,     0,     0,     0,  1203,     0,     0,     0,
       0,     0,  1093,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     171,   172,   173,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   174,    75,    76,    77,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,   175,    86,    87,    88,
      89,     0,    90,    91,     0,    92,   176,    94,     0,     0,
       0,    96,     0,     0,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,     0,   101,   102,   103,     0,     0,
     177,     0,     0,     0,     0,   107,   108,     0,   109,   110,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10, -1023, -1023, -1023, -1023,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,     0,
       0,     0,     0,     0,     0,  1442,     0,     0,     0,     0,
       0,  1093,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   171,
     172,   173,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   174,    75,    76,    77,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,   175,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   176,    94,     0,     0,     0,
      96,     0,     0,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,     0,   101,   102,   103,     0,     0,   177,
     440,   441,   442,     0,   107,   108,     0,   109,   110,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     443,   444,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,     0,   468,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   469,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   171,   172,
     173,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   174,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,   175,    86,    87,    88,    89,     0,
      90,    91,     0,    92,   176,    94,     0,     0,     0,    96,
       0,     0,    97,     0,   470,     0,     0,     0,    98,     0,
       0,     0,     0,   101,   102,   103,     0,     0,   177,   440,
     441,   442,     0,   107,   108,     0,   109,   110,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   443,
     444,     0,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   469,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   636,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   171,   172,   173,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   174,    75,    76,    77,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,   175,    86,    87,    88,    89,     0,    90,
      91,     0,    92,   176,    94,     0,     0,     0,    96,     0,
       0,    97,     0,   553,     0,     0,     0,    98,     0,     0,
       0,     0,   101,   102,   103,     0,     0,   177,     0,     0,
       0,     0,   107,   108,     0,   109,   110,   261,   262,     0,
     263,   264,     0,     0,   265,   266,   267,   268,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,   270,     0,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   272,   468,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   469,   274,   275,   276,   277,   278,
     279,   280,     0,     0,     0,    37,     0,   206,    40,     0,
       0,     0,     0,     0,     0,     0,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    50,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,     0,     0,     0,   732,   316,   317,   318,     0,     0,
       0,   319,   571,   572,   573,   574,     0,     0,     0,     0,
       0,   261,   262,     0,   263,   264,     0,     0,   265,   266,
     267,   268,   575,     0,     0,     0,     0,     0,    90,    91,
       0,    92,   176,    94,   326,   269,   327,   270,     0,   328,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   733,
       0,   107,     0,     0,     0,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   274,
     275,   276,   277,   278,   279,   280,     0,     0,     0,    37,
       0,   206,    40,     0,     0,     0,     0,     0,     0,     0,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,    50,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,     0,     0,     0,   315,   316,
     317,   318,     0,     0,     0,   319,   571,   572,   573,   574,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     261,   262,     0,   263,   264,     0,   575,   265,   266,   267,
     268,     0,    90,    91,     0,    92,   176,    94,   326,     0,
     327,     0,     0,   328,   269,     0,   270,     0,   271,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   733,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,   272,     0,   273,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   274,   275,
     276,   277,   278,   279,   280,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
      50,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,     0,     0,     0,     0,   316,   317,
     318,     0,     0,     0,   319,   320,   321,   322,   323,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   324,     0,     0,    88,   325,
       0,    90,    91,     0,    92,   176,    94,   326,     0,   327,
       0,     0,   328,   261,   262,     0,   263,   264,     0,   329,
     265,   266,   267,   268,     0,     0,     0,     0,     0,   330,
       0,     0,     0,  1715,     0,     0,     0,   269,     0,   270,
     444,   271,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,   272,     0,   273,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,   274,   275,   276,   277,   278,   279,   280,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    50,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,     0,     0,     0,
       0,   316,   317,   318,     0,     0,     0,   319,   320,   321,
     322,   323,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   324,     0,
       0,    88,   325,     0,    90,    91,     0,    92,   176,    94,
     326,     0,   327,     0,     0,   328,   261,   262,     0,   263,
     264,     0,   329,   265,   266,   267,   268,     0,     0,     0,
       0,     0,   330,     0,     0,     0,  1785,     0,     0,     0,
     269,     0,   270,     0,   271,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,     0,   468,
     272,     0,   273,     0,     0,     0,     0,     0,     0,     0,
       0,   469,     0,     0,   274,   275,   276,   277,   278,   279,
     280,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,    50,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
       0,     0,     0,   315,   316,   317,   318,     0,     0,     0,
     319,   320,   321,   322,   323,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   324,     0,     0,    88,   325,     0,    90,    91,     0,
      92,   176,    94,   326,     0,   327,     0,     0,   328,   261,
     262,     0,   263,   264,     0,   329,   265,   266,   267,   268,
       0,     0,     0,     0,     0,   330,     0,     0,     0,     0,
       0,     0,     0,   269,     0,   270,     0,   271,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   272,     0,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   274,   275,   276,
     277,   278,   279,   280,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,    50,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,     0,     0,     0,     0,   316,   317,   318,
       0,     0,     0,   319,   320,   321,   322,   323,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   324,     0,     0,    88,   325,     0,
      90,    91,     0,    92,   176,    94,   326,     0,   327,     0,
       0,   328,     0,   261,   262,     0,   263,   264,   329,  1516,
     265,   266,   267,   268,     0,     0,     0,     0,   330,     0,
       0,     0,     0,     0,     0,     0,     0,   269,     0,   270,
       0,   271,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   272,     0,   273,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   274,   275,   276,   277,   278,   279,   280,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    50,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,     0,     0,     0,
       0,   316,   317,   318,     0,     0,     0,   319,   320,   321,
     322,   323,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   324,     0,
       0,    88,   325,     0,    90,    91,     0,    92,   176,    94,
     326,     0,   327,     0,     0,   328,  1613,  1614,  1615,  1616,
    1617,     0,   329,  1618,  1619,  1620,  1621,     0,     0,     0,
       0,     0,   330,     0,     0,     0,     0,     0,     0,     0,
    1622,  1623,  1624,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1625,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1626,  1627,  1628,  1629,  1630,  1631,
    1632,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1633,  1634,  1635,  1636,  1637,
    1638,  1639,  1640,  1641,  1642,  1643,    50,  1644,  1645,  1646,
    1647,  1648,  1649,  1650,  1651,  1652,  1653,  1654,  1655,  1656,
    1657,  1658,  1659,  1660,  1661,  1662,  1663,  1664,  1665,  1666,
    1667,  1668,  1669,  1670,  1671,  1672,  1673,     0,     0,     0,
    1674,  1675,  1676,  1677,     0,  1678,  1679,  1680,  1681,  1682,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1683,  1684,  1685,     0,     0,     0,    90,    91,     0,
      92,   176,    94,  1686,     0,  1687,  1688,     0,  1689,   440,
     441,   442,     0,     0,     0,  1690,  1691,     0,  1692,     0,
    1693,  1694,     0,     0,     0,     0,     0,     0,     0,   443,
     444,     0,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,   440,   441,   442,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   444,     0,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,   440,   441,   442,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   555,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,     0,   468,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     444,   580,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,  1067,  1068,  1069,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,   584,
       0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,  1092,     0,   261,   262,     0,   263,   264,     0,
       0,   265,   266,   267,   268,     0,  1093,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,     0,
     270,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   777,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   272,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,     0,
       0,     0,    37,  1401,     0,     0,     0,     0,     0,     0,
       0,     0,   802,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,    50,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,     0,
       0,   315,   316,   317,   318,     0,     0,     0,   319,   571,
     572,   573,   574,     0,     0,     0,     0,     0,   261,   262,
       0,   263,   264,     0,     0,   265,   266,   267,   268,   575,
       0,     0,     0,     0,     0,    90,    91,     0,    92,   176,
      94,   326,   269,   327,   270,     0,   328,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   272,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   274,   275,   276,   277,
     278,   279,   280,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,    50,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,     0,     0,     0,  1254,   316,   317,   318,     0,
       0,     0,   319,   571,   572,   573,   574,     0,     0,     0,
       0,     0,   261,   262,     0,   263,   264,     0,     0,   265,
     266,   267,   268,   575,     0,     0,     0,     0,     0,    90,
      91,     0,    92,   176,    94,   326,   269,   327,   270,     0,
     328,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   272,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     274,   275,   276,   277,   278,   279,   280,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,    50,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,     0,     0,     0,     0,
     316,   317,   318,     0,  1262,     0,   319,   571,   572,   573,
     574,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   831,   832,     0,     0,     0,   575,   833,     0,
     834,     0,     0,    90,    91,     0,    92,   176,    94,   326,
       0,   327,   835,     0,   328,     0,     0,     0,     0,     0,
      34,    35,    36,    37,     0,     0,     0,     0,     0,     0,
     440,   441,   442,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     443,   444,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,     0,   468,     0,     0,
     836,   837,   838,     0,    78,    79,    80,    81,    82,   469,
    1026,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,   839,     0,    90,    91,     0,    92,
     176,    94,     0,     0,     0,    96,     0,     0,     0,     0,
       0,     0,    29,     0,   840,     0,     0,     0,     0,   101,
      34,    35,    36,    37,   841,   206,    40,     0,     0,     0,
       0,     0,     0,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,   514,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1027,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,   831,   832,     0,    96,     0,     0,   833,     0,
     834,     0,     0,     0,     0,     0,     0,     0,     0,   101,
       0,     0,   835,     0,   212,     0,     0,     0,     0,   107,
      34,    35,    36,    37,     0,     0,     0,     0,     0,     0,
     440,   441,   442,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     443,   444,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,     0,   468,     0,     0,
     836,   837,   838,     0,    78,    79,    80,    81,    82,   469,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,   839,     0,    90,    91,     0,    92,
     176,    94,    29,     0,     0,    96,     0,     0,     0,     0,
      34,    35,    36,    37,   840,   206,    40,     0,     0,   101,
       0,     0,     0,   207,   841,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   523,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    29,     0,     0,    96,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   206,    40,     0,     0,   101,
       0,     0,     0,   207,   212,     0,     0,   596,     0,   107,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   616,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    29,     0,   977,    96,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   206,    40,     0,     0,   101,
       0,     0,     0,   207,   212,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    29,     0,     0,    96,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   206,    40,     0,     0,   101,
       0,     0,     0,   207,   212,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1121,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,    29,     0,     0,    96,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   206,    40,     0,     0,   101,
       0,     0,     0,   207,   212,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   209,   210,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
     175,    86,    87,    88,    89,     0,    90,    91,     0,    92,
     176,    94,     0,     0,     0,    96,     0,   440,   441,   442,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   212,     0,     0,   443,   444,   107,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     0,   468,   440,   441,   442,     0,     0,
       0,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,     0,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,   440,   441,   442,   469,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,   444,   896,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,     0,   468,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     444,   963,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,     0,     0,
       0,     0,     0,     0,     0,   440,   441,   442,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   443,   444,  1011,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,     0,   468,  1067,  1068,  1069,     0,     0,     0,     0,
       0,     0,     0,     0,   469,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1070,  1307,     0,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1067,  1068,
    1069,     0,  1093,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
       0,  1338,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,     0,     0,  1067,  1068,  1069,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1070,     0,  1409,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1067,  1068,  1069,     0,  1093,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1070,     0,  1502,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1090,  1091,  1092,     0,    34,    35,
      36,    37,     0,   206,    40,     0,     0,     0,     0,     0,
    1093,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1594,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   226,     0,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,  1596,   175,    86,
      87,    88,    89,     0,    90,    91,     0,    92,   176,    94,
       0,     0,     0,    96,     0,    34,    35,    36,    37,     0,
     206,    40,     0,     0,     0,     0,     0,   101,   650,     0,
       0,     0,   228,     0,     0,     0,     0,   107,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,   175,    86,    87,    88,    89,
       0,    90,    91,     0,    92,   176,    94,     0,     0,     0,
      96,     0,    34,    35,    36,    37,     0,   206,    40,     0,
       0,     0,     0,     0,   101,   207,     0,     0,     0,   651,
       0,     0,     0,     0,   107,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,   175,    86,    87,    88,    89,     0,    90,    91,
       0,    92,   176,    94,     0,     0,     0,    96,     0,   440,
     441,   442,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   228,   814,     0,   443,
     444,   107,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,     0,   468,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   440,   441,   442,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   815,   443,   444,   960,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,     0,   468,
     440,   441,   442,     0,     0,     0,     0,     0,     0,     0,
       0,   469,     0,     0,     0,     0,     0,     0,     0,     0,
     443,   444,     0,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,     0,   468,  1067,  1068,
    1069,     0,     0,     0,     0,     0,     0,     0,     0,   469,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
    1414,     0,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1067,  1068,  1069,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,  1070,     0,     0,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1093,     0,     0,     0,     0,   443,   444,
       0,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,     0,   468,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   469
};

static const yytype_int16 yycheck[] =
{
       5,     6,   179,     8,     9,    10,    11,    12,    13,   154,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    33,    56,    29,    30,   393,   125,   626,     4,
      60,   104,   393,   393,     4,    46,   663,    95,  1149,    44,
      51,    99,   100,     4,   229,   508,   376,    52,   527,    54,
       4,   159,    57,   662,    59,    85,   929,    31,    88,   104,
     154,   468,   590,   591,  1136,    57,   124,   500,   666,   940,
     500,    31,    57,    31,    31,   813,   741,     4,    83,   643,
      44,  1145,   180,   346,   347,   104,   104,   321,   322,   820,
     420,   956,  1025,     9,    32,     9,   789,    32,   240,   104,
      49,    70,   535,     9,     9,   535,     9,   972,    56,    14,
       9,    14,     9,     9,     9,     9,     9,     9,    14,    83,
      14,    14,     9,     9,     9,     9,     9,    70,     9,     9,
       9,    54,   177,   241,   134,   135,     9,    54,     9,    36,
      70,    70,    49,     9,    83,     9,     9,  1012,    49,    49,
      83,    70,     9,     9,     9,    70,  1729,    83,   177,   177,
     102,    38,    70,    38,    90,   532,   157,   212,    70,    38,
      38,   178,   177,    83,   504,   505,   157,     0,    90,   184,
     163,   122,   192,   228,    14,   192,    27,    28,   157,   130,
    1063,   134,   135,   212,   212,    49,   196,   178,    38,    50,
      51,   192,    32,   533,   192,   779,    83,   212,    83,   228,
      38,     4,   195,  1786,    83,    83,   155,   156,    70,   161,
      70,    51,    70,   228,    70,    70,   195,   157,   157,   155,
     156,    70,    70,   107,    70,     8,   378,   242,    70,   162,
     245,   190,   154,    83,   193,   162,   192,   252,   253,    70,
      70,    70,   195,   430,    70,    83,     4,   195,    70,   194,
     177,   171,   192,  1335,   197,   195,   195,  1424,   866,  1333,
    1342,   246,  1344,   178,   200,   250,   195,   192,   194,   224,
     194,   195,   975,   190,   178,   178,  1219,   195,   194,   194,
     192,   194,   193,   193,   171,   194,   193,  1369,   194,   194,
     194,   194,   194,   171,  1035,   338,  1037,   194,   194,   194,
     194,   194,  1177,   194,   194,   194,   346,   347,   348,    83,
     193,   366,   193,   557,   558,  1198,   953,   193,   179,   193,
     193,   171,   125,   510,   862,   863,   193,   193,   193,   193,
     192,   193,   192,   171,   192,   195,   192,   366,   366,   195,
     195,    83,    84,   192,   918,   385,   195,   195,   115,   195,
     365,   366,    70,   195,    56,  1522,   424,   372,   373,   374,
     375,   376,   192,   192,   195,   195,   381,    50,    51,   195,
     221,    83,    84,   195,   482,   106,   107,   180,   421,  1546,
     338,  1548,   157,   178,   472,   400,  1460,    83,  1462,  1471,
     178,   365,    70,   408,    83,   192,   154,   192,   134,   135,
     483,   375,    90,   178,   192,   420,    83,    83,   175,   477,
     478,   479,   480,   501,   192,   189,   134,   135,   506,   892,
     195,   195,   106,   107,   192,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,     4,   469,   197,   471,   472,   473,  1134,
     195,   157,   399,   421,   192,   196,   154,  1215,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   483,   171,   195,   468,   197,   501,   502,   483,   504,
     505,   506,   507,  1567,   171,   171,   511,  1571,   468,   514,
     468,   468,    53,   946,   355,    56,   946,    83,   523,    57,
     525,   102,   196,   364,    90,   366,   474,   669,   533,   671,
     371,    69,    73,  1406,  1161,   680,   541,  1164,   543,  1148,
     102,   382,   102,    70,    83,    81,   123,    75,    76,   541,
     735,    90,    93,   130,    95,   105,   106,   107,    99,   100,
     590,   591,   929,   161,   134,   135,   743,   103,   929,   929,
     977,   546,   517,   521,   532,   105,   106,   107,  1106,   271,
     161,   273,   767,   124,    75,    76,   680,   102,   103,   155,
     156,   596,   128,   856,   162,   858,   194,   195,    83,   161,
     474,   161,   157,   192,    31,    90,   651,   134,   135,   132,
     133,   163,   192,  1486,   407,   154,   155,   156,    83,   624,
    1194,  1195,   192,   178,   176,  1199,   500,   192,   164,   165,
     157,   167,   168,   169,   192,   201,   694,   192,   330,   969,
     195,   783,   784,   195,  1760,  1761,   651,   521,   790,   791,
     194,   195,   193,    31,    81,   192,   192,   842,   532,   407,
     192,   535,    31,   161,  1728,   192,   851,   508,  1732,   154,
     155,   156,    50,    31,   539,    53,   103,   622,  1343,  1756,
    1757,    50,   790,   628,    53,    32,    83,   192,   481,   154,
     155,   156,   178,    90,   699,    38,  1063,    81,   125,    83,
    1179,    85,  1063,  1063,   178,   246,   192,   194,   653,   250,
    1337,   138,   194,   254,   141,  1324,   581,    83,   192,   103,
     119,   120,   121,    81,    90,   194,   418,   194,   733,   421,
      81,   158,   194,    91,   161,   162,   663,   164,   165,   194,
     167,   168,   169,   194,   195,   103,    53,    54,    55,     4,
      57,   194,   103,   346,   347,  1218,   194,   762,   155,   156,
     829,   830,    69,  1874,    70,    70,    78,    79,    80,    81,
      70,    70,    27,    28,  1439,   195,   157,  1351,  1889,  1353,
     164,   165,   623,   167,   168,   169,   731,    83,   192,   155,
     156,   103,   797,    70,    90,   157,   192,   338,   809,   664,
     158,    81,   192,   161,  1868,   161,   164,   165,   813,   167,
     168,   169,   194,   164,   165,    69,   167,   168,   169,  1883,
      53,    54,    55,   103,    48,   192,   856,   192,   858,   178,
     157,  1198,   862,   863,   864,   192,    69,  1198,  1198,  1302,
    1170,   192,   192,   818,   199,  1472,   108,   109,   110,   104,
       9,  1181,   164,   165,   157,   167,   168,   169,  1523,   155,
     156,   192,   157,     8,   194,   157,   407,   192,    14,   157,
     562,   194,   194,     9,   195,   194,   417,   822,   158,   130,
     421,   161,   827,   424,   164,   165,   130,   167,   168,   169,
      14,   896,   193,   898,   178,    81,    14,   102,   903,   193,
     905,   198,   193,   744,  1478,   192,  1480,   193,  1482,   914,
    1484,   193,   111,   192,   192,     9,   196,   103,   976,   154,
     193,   193,   177,   928,   193,   111,   112,   193,    94,   194,
       9,    14,   680,   474,   475,   476,   477,   478,   479,   480,
     805,   178,   917,   888,   785,   810,   787,   917,     9,   192,
      81,   192,    83,    84,   194,   960,   917,   212,   963,   500,
     965,   195,   194,   917,   969,   195,   221,   659,   660,  1840,
    1155,    83,   103,   228,   815,   161,   668,   193,   164,   165,
     521,   167,   168,   169,   932,   193,    27,    28,   194,  1860,
     917,   246,   193,   132,   535,   250,   192,   199,  1869,   193,
       9,    81,   193,   977,  1578,   546,  1011,  1018,     9,  1339,
     803,     9,   199,   940,   193,    70,    32,   977,   133,   977,
     977,   177,   157,   103,     9,   136,   953,   157,   193,   570,
      14,   162,   190,   164,   165,   876,   167,   168,   169,  1406,
       9,     9,   179,   193,  1019,  1406,  1406,    14,   132,   196,
    1123,   892,   893,   594,   595,   803,   199,   199,   932,     9,
     853,  1006,    14,   192,   195,   199,   197,   193,   193,   157,
     944,   199,   946,   193,   102,   194,  1106,   194,     9,    91,
     136,   161,   157,     9,   164,   165,   193,   167,   168,   169,
     192,   632,   633,   119,   120,   121,   122,   123,   124,    70,
     355,   192,   195,   157,    70,   853,  1051,   157,   195,   364,
       9,   366,    14,  1058,  1062,   195,   371,  1065,  1123,  1486,
     196,    81,   194,  1128,   917,  1486,  1486,   382,  1458,     9,
     179,  1123,   195,    14,   199,    81,   195,   829,   830,  1013,
      14,   193,    32,   103,   190,   938,   194,   192,   192,    32,
      14,    14,   407,   694,   192,   192,    52,   103,  1133,   192,
    1734,  1735,   188,  1133,    70,  1170,    70,  1032,   194,   917,
     192,    81,  1133,     9,   193,   192,  1181,  1182,   136,  1133,
     221,   194,  1780,   194,  1025,  1026,    14,   179,   136,   111,
     938,     9,    69,   103,   193,   199,  1207,   119,   120,   121,
     122,   123,   124,     9,   164,   165,  1133,   167,   168,   169,
    1215,    83,   158,   196,   196,   161,   162,   194,   164,   165,
    1225,   167,   168,   169,     9,   136,    14,   919,  1021,   192,
    1023,   192,   192,    83,  1161,  1210,   193,  1164,   779,   194,
     781,   195,   193,   935,   136,   192,  1191,   192,     9,    91,
     194,    27,    28,   508,   164,   165,   948,   167,   168,   169,
    1858,   195,   803,   195,   199,   154,   188,   195,    81,    32,
      83,    84,    77,  1021,   379,  1023,   817,   818,   383,   194,
     193,   973,   194,  1124,   179,   136,  1127,    32,   193,     9,
     103,   546,   193,     9,     9,  1440,   199,   136,     9,  1244,
     193,   199,  1307,  1248,   195,   410,  1251,   412,   413,   414,
     415,  1316,   853,  1258,   355,  1320,    14,  1322,   196,   860,
     861,   194,   193,   364,   194,  1330,    83,   196,   193,   195,
     371,   192,   194,  1338,  1339,   193,   193,  1435,   192,    81,
    1133,   382,     9,   199,   193,   193,   136,  1039,   889,   162,
    1042,   164,   165,   199,   167,   168,   169,     9,   136,   193,
       9,   103,  1203,    32,   194,   194,   193,  1544,   623,   111,
     112,   193,   195,  1300,   194,     4,   917,  1218,  1219,   112,
     166,   162,   195,   194,   197,  1133,  1313,    14,    83,   117,
     193,   932,   193,   195,   193,   136,   136,   938,    14,   178,
     195,   194,    83,   944,    14,   946,    14,    83,    14,   193,
    1337,   193,   192,  1358,   136,   136,    14,  1362,    14,   161,
      49,  1366,   164,   165,  1289,   167,   168,   169,   194,  1222,
      81,   195,   194,     9,   194,   976,   119,   120,   121,   122,
     123,   124,     9,   196,    59,   221,    83,   988,   989,   990,
    1455,   178,   103,  1458,   192,  1147,     9,  1149,  1516,    83,
     195,  1302,   194,    81,   102,   115,   157,   508,   102,   179,
     169,    36,  1013,    14,  1222,   192,   194,   179,  1019,   193,
    1021,  1173,  1023,   112,  1176,   103,    81,   192,   117,   744,
     119,   120,   121,   122,   123,   124,   125,    81,   175,   179,
      83,   172,  1043,   193,     9,   188,  1371,   194,   103,    83,
     195,   162,    14,   164,   165,   166,   167,   168,   169,   103,
    1495,  1062,  1387,   193,  1065,   193,    83,    14,    83,    14,
     785,    83,   787,    14,  1461,   164,   165,  1714,   167,  1231,
    1467,   192,  1469,  1235,   162,  1472,   164,   165,   803,   167,
     168,   169,  1300,  1094,    83,  1106,  1849,  1562,   475,   188,
     815,  1488,   480,   818,   970,  1313,   161,   196,   477,   164,
     165,  1519,   167,   168,   169,   920,  1865,   161,  1216,   355,
     164,   165,   623,   167,   168,   169,  1587,  1384,   364,  1860,
     598,  1611,  1133,  1524,  1574,   371,  1698,  1430,   853,  1893,
    1881,  1442,  1426,  1710,  1296,  1297,   382,  1570,  1137,  1059,
     989,  1586,  1587,  1192,  1008,  1193,   944,   393,   372,   829,
    1419,   876,  1815,  1416,  1489,    50,    51,    52,    53,    54,
      55,  1114,    57,  1498,   421,  1044,  1094,   892,   893,    -1,
      -1,  1434,    -1,    -1,    69,    -1,  1511,    -1,    -1,  1576,
      -1,    -1,  1193,  1194,  1195,    -1,    -1,    -1,  1199,    -1,
      -1,    -1,   917,    -1,     4,    -1,    -1,    -1,  1416,  1210,
      50,    51,    52,    53,    54,    55,  1709,    -1,    -1,    -1,
      -1,  1222,    -1,   938,    -1,    -1,    -1,    -1,  1833,    69,
      -1,    -1,  1440,    -1,    -1,    -1,    -1,  1490,    -1,  1391,
      -1,  1393,    -1,   744,    -1,    -1,  1499,    -1,    -1,    49,
      -1,    -1,    -1,  1461,  1719,    -1,  1774,    -1,    -1,  1467,
      -1,  1469,    -1,  1588,   119,   120,   121,   122,   123,   124,
      -1,    -1,   508,    27,    28,   130,   131,    31,    -1,    -1,
    1488,    -1,  1490,  1435,   785,    -1,   787,    -1,    -1,    -1,
      -1,  1499,    -1,    -1,  1295,  1853,    -1,    -1,    -1,  1552,
      -1,  1709,    56,    -1,  1019,    -1,  1021,    -1,  1023,    -1,
    1025,  1026,   112,   168,   815,   170,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,   183,    -1,
     185,    -1,  1585,   188,    -1,    -1,    -1,    -1,  1591,    -1,
      -1,    -1,    -1,    -1,  1597,  1750,  1751,    -1,    -1,    -1,
    1351,    -1,  1353,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   165,    -1,   167,  1576,    -1,
      -1,    -1,    -1,    -1,    -1,   876,    -1,  1585,    81,    -1,
      -1,    -1,    -1,  1591,    -1,    -1,    -1,   623,   188,  1597,
      -1,   892,   893,    -1,    -1,    -1,   196,    -1,    -1,    -1,
     103,  1726,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1124,
      -1,  1736,  1127,    -1,    -1,  1416,    -1,    -1,  1133,    -1,
      -1,    -1,   125,  1424,    -1,    -1,    -1,    -1,    -1,  1430,
      -1,    -1,    -1,    -1,  1899,   138,    -1,    -1,   141,    -1,
      -1,    -1,  1907,    -1,    -1,    -1,    -1,    -1,  1913,    -1,
      -1,  1916,    -1,  1840,    -1,   158,  1781,  1609,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,   221,    -1,    -1,
      -1,    -1,    -1,  1860,    -1,    -1,    -1,  1478,    -1,  1480,
      -1,  1482,  1869,  1484,    -1,    -1,    -1,  1740,  1203,  1490,
      -1,    -1,    -1,    -1,  1495,  1210,    -1,    -1,  1499,    -1,
      -1,  1826,    -1,  1218,  1219,    -1,    -1,  1222,   744,    -1,
      -1,    -1,    -1,    -1,     4,  1516,    -1,   271,  1519,   273,
      -1,  1522,    -1,    -1,  1025,  1026,    -1,    -1,    -1,  1782,
      -1,  1532,  1740,    -1,    -1,    -1,  1789,    -1,  1539,    -1,
      -1,    -1,    -1,    -1,    -1,  1546,    -1,  1548,    -1,   785,
      -1,   787,    31,    -1,  1555,    -1,    -1,    -1,    -1,    49,
      -1,  1713,  1887,    -1,    -1,    -1,    -1,    -1,    -1,  1894,
      -1,  1824,    -1,    -1,  1782,    -1,   330,  1578,    -1,   815,
      59,  1789,    -1,    -1,  1585,  1586,  1587,  1302,    -1,    -1,
    1591,    -1,    -1,  1846,    -1,    -1,  1597,    -1,    -1,    -1,
      -1,   355,    81,   119,   120,   121,   122,   123,   124,    -1,
     364,    -1,    -1,    -1,   130,   131,  1824,   371,    -1,    -1,
      -1,    -1,   112,  1124,   103,  1833,  1127,   117,   382,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,  1846,   393,
     876,    -1,  1895,    -1,    -1,    -1,  1798,    -1,    -1,  1902,
       4,    -1,    -1,    -1,   170,    -1,   892,   893,    -1,   138,
     139,   140,   141,    -1,   418,    -1,    -1,   421,    -1,    -1,
      -1,    -1,   188,    -1,   164,   165,    -1,   167,    -1,   158,
      -1,    -1,   161,   162,    -1,   164,   165,  1895,   167,   168,
     169,    -1,   171,   929,  1902,    49,    -1,    -1,   188,    -1,
      -1,  1416,  1203,   182,    -1,    -1,   196,    -1,  1709,    -1,
      -1,    -1,    -1,   192,   468,    -1,    -1,  1218,  1219,    -1,
      -1,    -1,  1874,    -1,    -1,    -1,    -1,  1442,    -1,  1730,
      -1,    -1,    -1,  1734,  1735,    -1,    -1,  1889,    -1,  1740,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,  1749,    -1,
      -1,    -1,    -1,    -1,   508,  1756,  1757,    -1,   112,  1760,
    1761,    -1,    -1,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,  1774,    -1,  1490,    -1,    -1,    -1,    -1,
    1495,  1782,    -1,    -1,  1499,    -1,    -1,    -1,  1789,  1025,
    1026,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1302,    -1,    -1,    -1,     4,    -1,    -1,   562,    -1,
     164,   165,   566,   167,    -1,   569,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1824,    -1,    -1,    -1,  1063,    -1,    -1,
      -1,  1832,    -1,    -1,   188,     4,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    -1,    -1,  1846,    -1,    -1,   602,    -1,
      49,  1852,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,   623,
    1585,  1586,  1587,    -1,    -1,    -1,  1591,    -1,    -1,    -1,
      49,    -1,  1597,    -1,    -1,    -1,    -1,    -1,  1124,    -1,
      -1,  1127,    -1,    -1,  1895,    -1,    -1,    -1,    -1,    -1,
      -1,  1902,    -1,    -1,    -1,   659,   660,   164,   165,    -1,
     167,    -1,    -1,   112,   668,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
      -1,  1442,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,  1198,    -1,    -1,   164,   165,  1203,   167,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,  1218,  1219,    -1,    -1,    -1,    -1,    -1,   188,
     744,    -1,    -1,    -1,    -1,   164,   165,   196,   167,    -1,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,   569,    -1,    27,    28,    -1,   188,
      31,    -1,    67,    68,    -1,  1740,    69,   196,    -1,    -1,
      -1,   785,    -1,   787,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   602,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   815,   816,    -1,    -1,    -1,  1302,  1782,    -1,    -1,
      -1,    -1,    -1,    -1,  1789,   829,   830,   831,   832,   833,
     834,   835,    -1,    -1,    -1,    -1,    -1,   841,    -1,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     854,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,  1824,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,   876,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1846,    -1,    -1,    -1,    -1,   890,    -1,   892,   893,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   919,   920,    -1,   134,   135,
    1406,    -1,    -1,    -1,    -1,   929,    -1,    -1,    69,    -1,
    1895,   935,    -1,    -1,    -1,    -1,    -1,  1902,    -1,    -1,
      -1,    -1,    -1,    -1,   948,    -1,    -1,    -1,   952,    -1,
      -1,   955,    -1,    -1,    -1,    -1,  1442,    -1,    -1,    -1,
     221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   973,
      -1,    -1,    -1,   977,    -1,    -1,    -1,   193,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1486,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   816,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1025,  1026,    -1,    -1,    -1,   831,   832,   833,   834,
      -1,    67,    68,    -1,    -1,  1039,   841,    -1,  1042,    -1,
    1044,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      -1,    -1,   193,    -1,    -1,  1059,  1060,  1061,    -1,  1063,
      91,    -1,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,
      -1,    -1,    -1,    -1,   355,    -1,    -1,    -1,   134,   135,
      -1,    -1,    -1,   364,    -1,    -1,  1110,    50,    51,    -1,
     371,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
    1124,   382,   153,  1127,    -1,    -1,    -1,    70,   159,   160,
      -1,    -1,   393,    -1,    -1,    78,    79,    80,    81,    -1,
      -1,    -1,   173,  1147,    -1,  1149,    -1,   952,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,   193,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1173,
      -1,    -1,  1176,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,  1198,   138,   139,   140,    -1,  1203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   468,    -1,    -1,
     153,    -1,    -1,    -1,  1218,  1219,    -1,  1221,    -1,    -1,
      -1,   164,   165,    -1,   167,   168,   169,  1231,    -1,    67,
      68,  1235,    -1,    -1,  1238,    -1,  1240,    -1,    -1,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   508,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1060,  1061,    -1,  1262,    -1,
      -1,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,    -1,
      -1,    56,  1296,  1297,    -1,  1299,   134,   135,  1302,    -1,
      -1,    -1,    -1,    -1,    -1,  1110,    -1,    -1,   569,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   602,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    31,    -1,
      -1,    -1,   623,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,  1391,    -1,  1393,
     103,    -1,    -1,    -1,    -1,  1399,    -1,  1401,   111,  1403,
      -1,  1405,  1406,    -1,    -1,  1409,    -1,  1411,    81,    -1,
    1414,    -1,    -1,    -1,    -1,    -1,  1221,    -1,    -1,    -1,
    1424,  1425,    -1,    -1,  1428,   138,   139,   140,   141,    -1,
     103,  1435,    -1,  1238,    -1,  1240,    -1,    -1,  1442,   134,
     135,    -1,    -1,    -1,    -1,   158,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,  1262,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      -1,    -1,  1486,   744,    -1,   158,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,    -1,  1502,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   271,    -1,   273,   182,
    1514,  1515,    31,    -1,    -1,    -1,    -1,    -1,  1522,   192,
    1524,    -1,    -1,    -1,   785,    -1,   787,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      59,    57,  1546,    -1,  1548,    -1,    -1,    -1,    -1,    -1,
      -1,  1555,    -1,    69,   815,   816,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,   330,    -1,    -1,    -1,    -1,
     831,   832,   833,   834,   835,    -1,    -1,    -1,    -1,    -1,
     841,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
    1594,  1595,  1596,   854,  1399,    -1,  1401,  1601,  1403,  1603,
    1405,    -1,    -1,    -1,  1409,  1609,  1411,  1611,    -1,  1414,
      31,    -1,    -1,    -1,    -1,   876,    -1,    -1,    -1,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   890,
      -1,   892,   893,    -1,    -1,    -1,    -1,    -1,    59,   158,
      -1,    -1,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,   171,   418,    -1,    -1,   421,    -1,    -1,   920,
      81,    -1,    -1,   182,    -1,    -1,    -1,    -1,   929,    -1,
      -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,   952,    -1,    -1,   955,    -1,    -1,  1502,   119,   120,
     121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,  1713,
      -1,    -1,    -1,    -1,    -1,    -1,   977,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,  1730,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
     161,   162,    -1,   164,   165,  1749,   167,   168,   169,    -1,
      -1,  1755,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,  1766,    -1,  1025,  1026,    -1,   188,  1772,    -1,
      -1,   192,  1776,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1594,
    1595,  1596,    -1,    -1,  1798,    -1,  1601,   562,  1059,  1060,
    1061,   566,  1063,    -1,   569,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,  1093,    -1,  1838,    -1,    -1,   602,    -1,    -1,
      -1,    -1,    -1,    -1,  1848,    -1,    -1,    -1,    -1,  1110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1865,    -1,  1124,    -1,    -1,  1127,    -1,    -1,    -1,
    1874,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1889,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,   659,   660,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   668,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,  1198,    -1,    -1,
      -1,    -1,  1203,    -1,    -1,    69,    -1,    -1,    -1,    -1,
    1755,    -1,    -1,    -1,    -1,    -1,    -1,  1218,  1219,    -1,
    1221,  1766,    -1,    -1,    -1,    -1,    -1,  1772,    -1,    -1,
      -1,  1776,    -1,    -1,    -1,    -1,    -1,  1238,    -1,  1240,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1262,    -1,    -1,    27,    28,    31,    -1,    31,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,  1838,    -1,    -1,    -1,    -1,  1299,    -1,
      -1,  1302,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,   816,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,   829,   830,   831,   832,   833,   834,
     835,    -1,    -1,    -1,    -1,   199,   841,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,    69,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1399,    57,
    1401,    -1,  1403,    -1,  1405,  1406,    -1,    -1,  1409,    -1,
    1411,    69,    -1,  1414,   919,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1425,    -1,    -1,  1428,    -1,    -1,
     935,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1442,    -1,   948,    -1,    -1,    -1,   952,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   221,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   973,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1486,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1502,    -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1514,  1515,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1524,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1039,    -1,    -1,  1042,    -1,  1044,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1059,  1060,  1061,    -1,    -1,    -1,
      -1,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,  1093,    -1,
      -1,    -1,   355,  1594,  1595,  1596,    -1,    -1,    -1,    -1,
    1601,   364,  1603,    -1,    -1,  1110,    -1,    -1,   371,    -1,
    1611,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   382,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1147,    -1,  1149,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1173,    30,
      31,  1176,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   468,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1221,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1231,    -1,    -1,    -1,
    1235,    -1,    -1,  1238,    -1,  1240,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   508,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1755,    -1,    -1,  1262,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1766,    -1,    -1,    -1,    -1,
      -1,  1772,    -1,    -1,    -1,  1776,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1296,  1297,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   569,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,   196,    -1,  1838,    -1,   602,
      -1,    -1,    -1,    69,    -1,    -1,    -1,  1848,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     623,    -1,    -1,    -1,  1865,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1391,    -1,  1393,    -1,
      -1,    -1,    -1,    -1,  1399,    -1,  1401,    -1,  1403,    69,
    1405,    -1,    -1,    -1,  1409,    -1,  1411,    -1,    -1,  1414,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1424,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1435,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     196,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1502,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1522,    57,    -1,
      -1,    -1,   785,    -1,   787,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1546,    -1,  1548,    -1,    -1,    -1,    -1,    -1,    -1,
    1555,    -1,   815,   816,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   831,   832,
     833,   834,   835,    -1,    -1,    -1,    -1,    -1,   841,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1594,
    1595,  1596,    -1,    -1,    -1,    -1,  1601,    -1,   196,    -1,
      -1,    -1,    -1,    -1,  1609,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   876,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   892,
     893,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,   929,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   952,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1713,    57,
      -1,    -1,    -1,    -1,   977,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,  1730,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1749,    -1,    -1,    -1,    -1,    -1,
    1755,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,  1766,  1025,  1026,    -1,    -1,    -1,  1772,    -1,    -1,
      -1,  1776,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,  1798,    -1,    -1,  1059,  1060,  1061,    -1,
    1063,   196,    69,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,    -1,    -1,  1838,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1124,    -1,    -1,  1127,    -1,    10,    11,    12,  1874,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1889,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,  1198,    13,    -1,    -1,    -1,
    1203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,  1218,  1219,    -1,  1221,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,  1238,    -1,  1240,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,  1262,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,  1302,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,    -1,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1399,    -1,  1401,    -1,
    1403,    -1,  1405,  1406,    -1,    -1,  1409,    -1,  1411,    -1,
      -1,  1414,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1442,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,  1486,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,  1502,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,
     176,  1594,  1595,  1596,    -1,    -1,   182,   183,  1601,   185,
      -1,   187,   188,   189,    -1,    -1,   192,  1610,   194,   195,
     196,   197,   198,    -1,   200,   201,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1755,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,  1766,    -1,    -1,    -1,    -1,    -1,  1772,
      -1,    -1,    -1,  1776,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,  1800,    70,    71,
      72,    73,    74,   196,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,  1838,   108,   109,   110,   111,
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   196,   126,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,    -1,   187,   188,   189,    -1,    -1,
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
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,   170,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
     183,    -1,   185,    -1,   187,   188,   189,    -1,    -1,   192,
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
     194,   195,   196,   197,   198,    -1,   200,   201,     3,     4,
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
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
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
      -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,   108,
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
      92,    93,    94,    -1,    96,    -1,    98,    99,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,   194,   195,    -1,   197,   198,    -1,   200,   201,
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
      94,    -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,
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
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,    -1,
      -1,   158,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
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
     189,    -1,    -1,   192,    -1,   194,   195,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,   194,   195,    -1,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,   150,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      -1,   192,    -1,   194,   195,    -1,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
     162,    -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    -1,   194,   195,    -1,   197,   198,    -1,   200,   201,
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
      -1,    -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,
     173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,   194,   195,    -1,   197,   198,    -1,   200,   201,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,
     194,   195,    -1,   197,   198,    -1,   200,   201,     3,     4,
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
      -1,    -1,   137,   138,   139,   140,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,
      -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,   164,
     165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,   194,
     195,    -1,   197,   198,    -1,   200,   201,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   187,   188,   189,    11,    12,   192,    -1,   194,    -1,
      -1,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    31,    -1,    13,    34,    35,    36,
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
     167,   168,   169,    -1,   171,    -1,   173,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,   188,   189,    -1,    -1,   192,    -1,    -1,    -1,    -1,
     197,   198,    -1,   200,   201,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
     188,   189,    -1,    12,   192,    -1,    -1,   195,    -1,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    31,    -1,    13,    34,    35,    36,    37,    38,
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
     139,   140,    -1,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,
     159,   160,   161,   162,    -1,   164,   165,    -1,   167,   168,
     169,    -1,   171,    -1,   173,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,    -1,   103,    -1,    -1,    -1,    -1,   108,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
      -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,   162,    -1,   164,   165,    -1,   167,   168,   169,    -1,
      -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,   196,
      -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
      -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,   200,
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
     162,    -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,   173,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,
     192,    10,    11,    12,    -1,   197,   198,    -1,   200,   201,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      69,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
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
     173,    -1,    -1,   176,    -1,    -1,    -1,   196,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,
      -1,   194,    11,    12,   197,   198,    -1,   200,   201,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      69,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
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
     194,    -1,    -1,   197,   198,    -1,   200,   201,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
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
      -1,   137,   138,   139,   140,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,    -1,
     176,    -1,   194,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,    -1,    -1,   192,   193,    -1,    -1,
      -1,   197,   198,    -1,   200,   201,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    32,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
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
      -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
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
     188,   189,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,
     198,    -1,   200,   201,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
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
     189,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,
      -1,   200,   201,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    38,    -1,
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
     140,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,
     200,   201,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    50,
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
      -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,   200,
     201,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    50,    51,
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
      13,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
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
      10,    11,    12,    -1,   197,   198,    -1,   200,   201,     3,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,   173,
      -1,    -1,   176,    -1,   194,    -1,    -1,    -1,   182,    -1,
      -1,    -1,    -1,   187,   188,   189,    -1,    -1,   192,    10,
      11,    12,    -1,   197,   198,    -1,   200,   201,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,
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
      -1,   176,    -1,   194,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   187,   188,   189,    -1,    -1,   192,    -1,    -1,
      -1,    -1,   197,   198,    -1,   200,   201,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   158,    -1,    -1,    -1,    -1,    -1,   164,   165,
      -1,   167,   168,   169,   170,    27,   172,    29,    -1,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,   197,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   158,    10,    11,    12,
      13,    -1,   164,   165,    -1,   167,   168,   169,   170,    -1,
     172,    -1,    -1,   175,    27,    -1,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,   170,    -1,   172,
      -1,    -1,   175,     3,     4,    -1,     6,     7,    -1,   182,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   192,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    27,    -1,    29,
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
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
     170,    -1,   172,    -1,    -1,   175,     3,     4,    -1,     6,
       7,    -1,   182,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,   196,    -1,    -1,    -1,
      27,    -1,    29,    -1,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,   161,   162,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,   172,    -1,    -1,   175,     3,
       4,    -1,     6,     7,    -1,   182,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,   167,   168,   169,   170,    -1,   172,    -1,
      -1,   175,    -1,     3,     4,    -1,     6,     7,   182,   183,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
     170,    -1,   172,    -1,    -1,   175,     3,     4,     5,     6,
       7,    -1,   182,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,    -1,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,   159,   160,    -1,    -1,    -1,   164,   165,    -1,
     167,   168,   169,   170,    -1,   172,   173,    -1,   175,    10,
      11,    12,    -1,    -1,    -1,   182,   183,    -1,   185,    -1,
     187,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
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
      53,    54,    55,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   158,
      -1,    -1,    -1,    -1,    -1,   164,   165,    -1,   167,   168,
     169,   170,    27,   172,    29,    -1,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   158,    -1,    -1,    -1,    -1,    -1,   164,
     165,    -1,   167,   168,   169,   170,    27,   172,    29,    -1,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,    -1,    32,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,   158,    56,    -1,
      58,    -1,    -1,   164,   165,    -1,   167,   168,   169,   170,
      -1,   172,    70,    -1,   175,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    69,
      38,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,   182,    -1,    -1,    -1,    -1,   187,
      78,    79,    80,    81,   192,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    50,    51,    -1,   173,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    70,    -1,   192,    -1,    -1,    -1,    -1,   197,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    69,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    70,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,   182,    83,    84,    -1,    -1,   187,
      -1,    -1,    -1,    91,   192,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    70,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   187,
      -1,    -1,    -1,    91,   192,    -1,    -1,   195,    -1,   197,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    70,    -1,    72,   173,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   187,
      -1,    -1,    -1,    91,   192,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    70,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   187,
      -1,    -1,    -1,    91,   192,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    70,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   187,
      -1,    -1,    -1,    91,   192,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,    -1,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
     158,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,   173,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,    -1,    -1,    30,    31,   197,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   136,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   136,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
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
      49,    50,    51,    52,    53,    54,    55,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      69,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,   136,   158,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,   173,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,   187,    91,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   197,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,   168,   169,    -1,    -1,    -1,
     173,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,   187,    91,    -1,    -1,    -1,   192,
      -1,    -1,    -1,    -1,   197,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,   173,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,    28,    -1,    30,
      31,   197,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
     441,   442,   443,   444,   445,   446,   447,   448,   462,   464,
     466,   122,   123,   124,   137,   158,   168,   192,   209,   242,
     323,   344,   439,   344,   192,   344,   344,   344,   108,   344,
     344,   344,   425,   426,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,    83,    91,   124,   139,
     140,   153,   192,   220,   363,   395,   398,   403,   439,   442,
     439,    38,   344,   453,   454,   344,   124,   130,   192,   220,
     255,   395,   396,   397,   399,   403,   436,   437,   438,   446,
     450,   451,   192,   333,   400,   192,   333,   354,   334,   344,
     228,   333,   192,   192,   192,   333,   194,   344,   209,   194,
     344,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    31,    57,    59,    71,    72,    73,    74,    75,    76,
      77,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   130,   131,   132,   133,   137,
     138,   139,   140,   141,   158,   162,   170,   172,   175,   182,
     192,   209,   210,   211,   222,   467,   483,   484,   487,   194,
     339,   341,   344,   195,   235,   344,   111,   112,   161,   212,
     213,   214,   215,   219,    83,   197,   289,   290,   123,   130,
     122,   130,    83,   291,   192,   192,   192,   192,   209,   261,
     470,   192,   192,    70,   192,    70,   192,   334,    83,    90,
     154,   155,   156,   459,   460,   161,   195,   219,   219,   209,
     262,   470,   162,   192,   470,   470,    83,   189,   195,   355,
      27,   332,   336,   344,   345,   439,   443,   224,   195,   448,
      90,   401,   459,    90,   459,   459,    32,   161,   178,   471,
     192,     9,   194,    38,   241,   162,   260,   470,   124,   188,
     242,   324,   194,   194,   194,   194,   194,   194,   194,   194,
      10,    11,    12,    30,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    69,
     194,    70,    70,   195,   157,   131,   168,   170,   183,   185,
     263,   322,   323,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    67,    68,   134,   135,
     429,    70,   195,   434,   192,   192,    70,   195,   192,   241,
     242,    14,   344,   194,   136,    48,   209,   424,    90,   332,
     345,   157,   439,   136,   199,     9,   409,   256,   332,   345,
     439,   471,   157,   192,   402,   429,   434,   193,   344,    32,
     226,     8,   356,     9,   194,   226,   227,   334,   335,   344,
     209,   275,   230,   194,   194,   194,   138,   139,   140,   141,
     487,   487,   178,   486,   486,   486,   192,   111,   487,    14,
     157,   138,   139,   140,   141,   158,   209,   211,   194,   194,
     194,   236,   115,   175,   194,   212,   214,   212,   214,   219,
     195,     9,   410,   194,   102,   161,   195,   439,     9,   194,
     130,   130,    14,     9,   194,   439,   463,   334,   332,   345,
     439,   442,   443,   193,   178,   253,   137,   439,   452,   453,
     344,   364,   365,    38,   171,   287,   288,   344,   334,   376,
     287,   194,    70,   429,   154,   460,    82,   344,   439,    90,
     154,   460,   219,   208,   194,   195,   248,   258,   385,   387,
      91,   192,   357,   358,   360,   398,   445,   447,   464,    14,
     102,   465,   351,   352,   353,   285,   286,   427,   428,   193,
     193,   193,   193,   193,   196,   225,   226,   243,   250,   257,
     427,   344,   198,   200,   201,   209,   472,   473,   487,   287,
     467,   192,   470,   251,   241,   344,   344,   344,   344,    32,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   399,   344,   344,   449,   449,   344,
     455,   456,   130,   195,   210,   211,   448,   261,   209,   262,
     470,   470,   260,   242,    38,   336,   339,   341,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   162,   195,   209,   430,   431,   432,   433,   448,   449,
     344,   287,   287,   449,   344,   452,   241,   193,   344,   192,
     423,     9,   409,   193,   193,    38,   344,    38,   344,   402,
     193,   193,   193,   446,   447,   448,   287,   195,   209,   430,
     431,   448,   193,   224,   279,   195,   341,   344,   344,    94,
      32,   226,   273,   194,    28,   102,    14,     9,   193,    32,
     195,   276,   487,    31,    91,   222,   480,   481,   482,   192,
       9,    50,    51,    56,    58,    70,   138,   139,   140,   162,
     182,   192,   220,   222,   371,   374,   380,   395,   403,   404,
     406,   209,   485,   224,   192,   234,   195,   194,   195,   194,
     102,   161,   111,   112,   161,   215,   216,   217,   218,   219,
     215,   209,   344,   290,   404,    83,     9,   193,   193,   193,
     193,   193,   193,   193,   194,    50,    51,   477,   478,   479,
     132,   266,   192,     9,   193,   193,   136,   199,     9,   409,
     439,   344,   193,     9,   410,     9,   409,   199,   193,    83,
      85,   209,   461,   209,    70,   196,   196,   205,   207,    32,
     133,   265,   177,    54,   162,   177,   389,   345,   136,     9,
     409,   193,   157,   487,   487,    14,   356,   285,   224,   190,
       9,   410,   487,   488,   429,   434,   429,   196,     9,   409,
     179,   193,    14,   348,   244,   132,   264,   192,   470,   344,
      32,   199,   199,   136,   196,     9,   409,   344,   471,   192,
     254,   249,   259,    14,   465,   252,   241,    72,   439,   344,
     471,   199,   196,   193,   193,   199,   196,   193,    50,    51,
      70,    78,    79,    80,    91,   138,   139,   140,   153,   182,
     209,   372,   375,   412,   414,   415,   419,   422,   209,   439,
     439,   136,   264,   429,   434,   193,   344,   280,    75,    76,
     281,   224,   333,   224,   335,   102,    38,   137,   270,   439,
     404,   209,    32,   226,   274,   194,   277,   194,   277,     9,
     409,    91,   136,   157,     9,   409,   193,   171,   472,   473,
     474,   472,   404,   404,   404,   404,   404,   408,   411,   192,
      70,    70,   157,   192,   404,   157,   195,    10,    11,    12,
      31,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    69,   157,   471,   196,   395,   195,   238,
     214,   214,   209,   215,   215,   219,     9,   410,   196,   196,
      14,   439,   194,   179,     9,   409,   209,   267,   395,   195,
     452,   137,   439,    14,    38,   344,   344,    38,   171,   344,
     199,   344,   196,   205,   487,   267,   195,   388,    14,   193,
     344,   357,   448,   194,   487,   190,   196,    32,   475,   428,
      38,    83,   171,   430,   431,   433,   430,   431,   487,   404,
     285,   192,   395,   265,   349,   245,   344,   344,   344,   196,
     192,   287,   266,    32,   265,   487,    14,   264,   470,   399,
     196,   192,    14,    78,    79,    80,   209,   413,   413,   415,
     417,   418,    52,   192,    70,    70,    90,   154,   192,     9,
     409,   193,   423,    38,   344,   265,   196,    75,    76,   282,
     333,   226,   196,   194,    95,   194,   270,   439,   192,   136,
     269,    14,   224,   277,   105,   106,   107,   277,   196,   487,
     179,   136,   487,   209,   480,     9,   193,   409,   136,   199,
       9,   409,   408,   366,   367,   404,   377,   404,   405,   210,
     357,   359,   361,   193,   130,   210,   404,   457,   458,   404,
     404,   404,    32,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   485,    83,   239,
     196,   196,   218,   194,   404,   479,   102,   103,   476,     9,
     295,   193,   192,   336,   341,   344,   439,   136,   439,   344,
     199,   196,   465,   295,   163,   176,   195,   384,   391,   163,
     195,   390,   136,   194,   475,   487,   356,   488,    83,   171,
      14,    83,   471,   193,   285,   195,   285,   192,   136,   192,
     287,   193,   195,   487,   195,   194,   487,   265,   246,   402,
     287,   136,   199,     9,   409,   414,   417,   368,   369,   415,
     378,   415,   416,   154,   357,   420,   421,   415,   439,   195,
     333,    32,    77,   226,   194,   335,   269,   452,   270,   193,
     404,   101,   105,   194,   344,    32,   194,   278,   196,   179,
     487,   136,   171,    32,   193,   404,   404,   193,   199,     9,
     409,   136,   199,     9,   409,   136,     9,   409,   193,   136,
     196,     9,   409,   404,    32,   193,   224,   194,   194,   209,
     487,   487,   395,     4,   112,   117,   123,   125,   164,   165,
     167,   196,   296,   321,   322,   323,   328,   329,   330,   331,
     427,   452,    38,   344,   196,   195,   196,    54,   344,   344,
     344,   356,    38,    83,   171,    14,    83,   344,   192,   475,
     193,   295,   193,   285,   344,   287,   193,   295,   465,   295,
     194,   195,   192,   193,   415,   415,   193,   199,     9,   409,
     136,   199,     9,   409,   136,   193,     9,   409,   295,    32,
     224,   194,   193,   193,   193,   231,   194,   194,   278,   224,
     487,   487,   136,   404,   404,   404,   404,   357,   404,   404,
     404,   195,   196,   476,   132,   133,   183,   210,   468,   487,
     268,   395,   112,   331,    31,   125,   138,   141,   162,   168,
     305,   306,   307,   308,   395,   166,   313,   314,   128,   192,
     209,   315,   316,   297,   242,   487,     9,   194,     9,   194,
     194,   465,   322,   193,   439,   292,   162,   386,   196,   196,
      83,   171,    14,    83,   344,   287,   117,   346,   475,   196,
     475,   193,   193,   196,   195,   196,   295,   285,   136,   415,
     415,   415,   415,   357,   196,   224,   229,   232,    32,   226,
     272,   224,   193,   404,   136,   136,   136,   224,   395,   395,
     470,    14,   210,     9,   194,   195,   468,   465,   308,   178,
     195,     9,   194,     3,     4,     5,     6,     7,    10,    11,
      12,    13,    27,    28,    29,    57,    71,    72,    73,    74,
      75,    76,    77,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   137,   138,   139,   140,   142,   143,
     144,   145,   146,   158,   159,   160,   170,   172,   173,   175,
     182,   183,   185,   187,   188,   209,   392,   393,     9,   194,
     162,   166,   209,   316,   317,   318,   194,    83,   327,   241,
     298,   468,   468,    14,   242,   196,   293,   294,   468,    14,
      83,   344,   193,   192,   475,   194,   195,   319,   346,   475,
     292,   196,   193,   415,   136,   136,    32,   226,   271,   272,
     224,   404,   404,   404,   196,   194,   194,   404,   395,   301,
     487,   309,   310,   403,   306,    14,    32,    51,   311,   314,
       9,    36,   193,    31,    50,    53,    14,     9,   194,   211,
     469,   327,    14,   487,   241,   194,    14,   344,    38,    83,
     383,   195,   224,   475,   319,   196,   475,   415,   415,   224,
      99,   237,   196,   209,   222,   302,   303,   304,     9,   409,
       9,   409,   196,   404,   393,   393,    59,   312,   317,   317,
      31,    50,    53,   404,    83,   178,   192,   194,   404,   470,
     404,    83,     9,   410,   224,   196,   195,   319,    97,   194,
     115,   233,   157,   102,   487,   179,   403,   169,    14,   477,
     299,   192,    38,    83,   193,   196,   224,   194,   192,   175,
     240,   209,   322,   323,   179,   404,   179,   283,   284,   428,
     300,    83,   196,   395,   238,   172,   209,   194,   193,     9,
     410,   119,   120,   121,   325,   326,   283,    83,   268,   194,
     475,   428,   488,   193,   193,   194,   194,   195,   320,   325,
      38,    83,   171,   475,   195,   224,   488,    83,   171,    14,
      83,   320,   224,   196,    38,    83,   171,    14,    83,   344,
     196,    83,   171,    14,    83,   344,    14,    83,   344,   344
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
#line 2800 "hphp.y"
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
#line 2815 "hphp.y"
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
#line 2950 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 3027 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval).reset();;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 915:

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

  case 916:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

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

  case 922:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval).reset();;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 957:

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

  case 958:

/* Line 1455 of yacc.c  */
#line 3165 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3274 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3287 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3314 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    {;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    {;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3323 "hphp.y"
    {;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1015:

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

  case 1016:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3350 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3356 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3373 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3390 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3393 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3397 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3400 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3403 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3406 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3412 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3415 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3419 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3421 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3427 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3433 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3442 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14513 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

