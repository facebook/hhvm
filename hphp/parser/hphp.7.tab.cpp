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
#line 876 "hphp.7.tab.cpp"

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
#define YYLAST   17141

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  200
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  269
/* YYNRULES -- Number of rules.  */
#define YYNRULES  994
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1827

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
    2739,  2743,  2745,  2747,  2749,  2751,  2753,  2757,  2761,  2766,
    2771,  2775,  2777,  2779,  2787,  2797,  2805,  2812,  2821,  2823,
    2828,  2833,  2835,  2837,  2839,  2844,  2847,  2849,  2850,  2852,
    2854,  2856,  2860,  2864,  2868,  2869,  2871,  2873,  2877,  2881,
    2884,  2888,  2895,  2896,  2898,  2903,  2906,  2907,  2913,  2917,
    2921,  2923,  2930,  2935,  2940,  2943,  2946,  2947,  2953,  2957,
    2961,  2963,  2966,  2967,  2973,  2977,  2981,  2983,  2986,  2989,
    2991,  2994,  2996,  3001,  3005,  3009,  3016,  3020,  3022,  3024,
    3026,  3031,  3036,  3041,  3046,  3051,  3056,  3059,  3062,  3067,
    3070,  3073,  3075,  3079,  3083,  3087,  3088,  3091,  3097,  3104,
    3111,  3119,  3121,  3124,  3126,  3129,  3131,  3136,  3138,  3143,
    3147,  3148,  3150,  3154,  3157,  3161,  3163,  3165,  3166,  3167,
    3170,  3173,  3176,  3179,  3184,  3187,  3193,  3197,  3199,  3201,
    3202,  3206,  3211,  3217,  3221,  3223,  3226,  3227,  3232,  3234,
    3238,  3241,  3244,  3247,  3249,  3251,  3253,  3255,  3259,  3265,
    3272,  3274,  3283,  3290,  3292
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     201,     0,    -1,    -1,   202,   203,    -1,   203,   204,    -1,
      -1,   224,    -1,   241,    -1,   248,    -1,   245,    -1,   255,
      -1,   447,    -1,   129,   190,   191,   192,    -1,   156,   217,
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
     159,   217,    -1,   218,   452,    -1,   218,   452,    -1,   221,
       9,   448,    14,   387,    -1,   112,   448,    14,   387,    -1,
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
     148,   332,   192,    -1,   126,   190,   444,   191,   192,    -1,
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
      38,    -1,    -1,   111,    -1,    -1,   240,   239,   451,   242,
     190,   283,   191,   456,   317,    -1,    -1,   321,   240,   239,
     451,   243,   190,   283,   191,   456,   317,    -1,    -1,   408,
     320,   240,   239,   451,   244,   190,   283,   191,   456,   317,
      -1,    -1,   166,   207,   246,    32,   467,   446,   193,   290,
     194,    -1,    -1,   408,   166,   207,   247,    32,   467,   446,
     193,   290,   194,    -1,    -1,   261,   258,   249,   262,   263,
     193,   293,   194,    -1,    -1,   408,   261,   258,   250,   262,
     263,   193,   293,   194,    -1,    -1,   131,   259,   251,   264,
     193,   293,   194,    -1,    -1,   408,   131,   259,   252,   264,
     193,   293,   194,    -1,    -1,   130,   254,   385,   262,   263,
     193,   293,   194,    -1,    -1,   168,   260,   256,   263,   193,
     293,   194,    -1,    -1,   408,   168,   260,   257,   263,   193,
     293,   194,    -1,   451,    -1,   160,    -1,   451,    -1,   451,
      -1,   130,    -1,   123,   130,    -1,   123,   122,   130,    -1,
     122,   123,   130,    -1,   122,   130,    -1,   132,   378,    -1,
      -1,   133,   265,    -1,    -1,   132,   265,    -1,    -1,   378,
      -1,   265,     9,   378,    -1,   378,    -1,   266,     9,   378,
      -1,   136,   268,    -1,    -1,   420,    -1,    38,   420,    -1,
     137,   190,   433,   191,    -1,   224,    -1,    32,   222,    97,
     192,    -1,   224,    -1,    32,   222,    99,   192,    -1,   224,
      -1,    32,   222,    95,   192,    -1,   224,    -1,    32,   222,
     101,   192,    -1,   207,    14,   387,    -1,   273,     9,   207,
      14,   387,    -1,   193,   275,   194,    -1,   193,   192,   275,
     194,    -1,    32,   275,   105,   192,    -1,    32,   192,   275,
     105,   192,    -1,   275,   106,   342,   276,   222,    -1,   275,
     107,   276,   222,    -1,    -1,    32,    -1,   192,    -1,   277,
      75,   331,   224,    -1,    -1,   278,    75,   331,    32,   222,
      -1,    -1,    76,   224,    -1,    -1,    76,    32,   222,    -1,
      -1,   282,     9,   409,   323,   468,   169,    83,    -1,   282,
       9,   409,   323,   468,    38,   169,    83,    -1,   282,     9,
     409,   323,   468,   169,    -1,   282,   392,    -1,   409,   323,
     468,   169,    83,    -1,   409,   323,   468,    38,   169,    83,
      -1,   409,   323,   468,   169,    -1,    -1,   409,   323,   468,
      83,    -1,   409,   323,   468,    38,    83,    -1,   409,   323,
     468,    38,    83,    14,   342,    -1,   409,   323,   468,    83,
      14,   342,    -1,   282,     9,   409,   323,   468,    83,    -1,
     282,     9,   409,   323,   468,    38,    83,    -1,   282,     9,
     409,   323,   468,    38,    83,    14,   342,    -1,   282,     9,
     409,   323,   468,    83,    14,   342,    -1,   284,     9,   409,
     468,   169,    83,    -1,   284,     9,   409,   468,    38,   169,
      83,    -1,   284,     9,   409,   468,   169,    -1,   284,   392,
      -1,   409,   468,   169,    83,    -1,   409,   468,    38,   169,
      83,    -1,   409,   468,   169,    -1,    -1,   409,   468,    83,
      -1,   409,   468,    38,    83,    -1,   409,   468,    38,    83,
      14,   342,    -1,   409,   468,    83,    14,   342,    -1,   284,
       9,   409,   468,    83,    -1,   284,     9,   409,   468,    38,
      83,    -1,   284,     9,   409,   468,    38,    83,    14,   342,
      -1,   284,     9,   409,   468,    83,    14,   342,    -1,   286,
     392,    -1,    -1,   342,    -1,    38,   420,    -1,   169,   342,
      -1,   286,     9,   342,    -1,   286,     9,   169,   342,    -1,
     286,     9,    38,   420,    -1,   287,     9,   288,    -1,   288,
      -1,    83,    -1,   195,   420,    -1,   195,   193,   342,   194,
      -1,   289,     9,    83,    -1,   289,     9,    83,    14,   387,
      -1,    83,    -1,    83,    14,   387,    -1,   290,   291,    -1,
      -1,   292,   192,    -1,   449,    14,   387,    -1,   293,   294,
      -1,    -1,    -1,   319,   295,   325,   192,    -1,    -1,   321,
     467,   296,   325,   192,    -1,   326,   192,    -1,   327,   192,
      -1,   328,   192,    -1,    -1,   320,   240,   239,   450,   190,
     297,   281,   191,   456,   318,    -1,    -1,   408,   320,   240,
     239,   451,   190,   298,   281,   191,   456,   318,    -1,   162,
     303,   192,    -1,   163,   311,   192,    -1,   165,   313,   192,
      -1,     4,   132,   378,   192,    -1,     4,   133,   378,   192,
      -1,   117,   266,   192,    -1,   117,   266,   193,   299,   194,
      -1,   299,   300,    -1,   299,   301,    -1,    -1,   220,   155,
     207,   170,   266,   192,    -1,   302,   102,   320,   207,   192,
      -1,   302,   102,   321,   192,    -1,   220,   155,   207,    -1,
     207,    -1,   304,    -1,   303,     9,   304,    -1,   305,   375,
     309,   310,    -1,   160,    -1,    31,   306,    -1,   306,    -1,
     138,    -1,   138,   176,   467,   391,   177,    -1,   138,   176,
     467,     9,   467,   177,    -1,   378,    -1,   125,    -1,   166,
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
      83,    -1,    83,    14,   387,    -1,   326,     9,   449,    14,
     387,    -1,   112,   449,    14,   387,    -1,   327,     9,   449,
      -1,   123,   112,   449,    -1,   123,   329,   446,    -1,   329,
     446,    14,   467,    -1,   112,   181,   451,    -1,   190,   330,
     191,    -1,    72,   382,   385,    -1,    72,   253,    -1,    71,
     342,    -1,   367,    -1,   362,    -1,   190,   342,   191,    -1,
     332,     9,   342,    -1,   342,    -1,   332,    -1,    -1,    27,
      -1,    27,   342,    -1,    27,   342,   136,   342,    -1,   190,
     334,   191,    -1,   420,    14,   334,    -1,   137,   190,   433,
     191,    14,   334,    -1,    29,   342,    -1,   420,    14,   337,
      -1,    28,   342,    -1,   420,    14,   339,    -1,   137,   190,
     433,   191,    14,   339,    -1,   343,    -1,   420,    -1,   330,
      -1,   424,    -1,   423,    -1,   137,   190,   433,   191,    14,
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
      -1,   443,    -1,    66,   342,    -1,    65,   342,    -1,    64,
     342,    -1,    63,   342,    -1,    62,   342,    -1,    61,   342,
      -1,    60,   342,    -1,    73,   383,    -1,    59,   342,    -1,
     389,    -1,   361,    -1,   360,    -1,   196,   384,   196,    -1,
      13,   342,    -1,   364,    -1,   117,   190,   366,   392,   191,
      -1,    -1,    -1,   240,   239,   190,   346,   283,   191,   456,
     344,   456,   193,   222,   194,    -1,    -1,   321,   240,   239,
     190,   347,   283,   191,   456,   344,   456,   193,   222,   194,
      -1,    -1,   186,    83,   349,   354,    -1,    -1,   186,   187,
     350,   283,   188,   456,   354,    -1,    -1,   186,   193,   351,
     222,   194,    -1,    -1,    83,   352,   354,    -1,    -1,   187,
     353,   283,   188,   456,   354,    -1,     8,   342,    -1,     8,
     339,    -1,     8,   193,   222,   194,    -1,    91,    -1,   445,
      -1,   356,     9,   355,   136,   342,    -1,   355,   136,   342,
      -1,   357,     9,   355,   136,   387,    -1,   355,   136,   387,
      -1,   356,   391,    -1,    -1,   357,   391,    -1,    -1,   180,
     190,   358,   191,    -1,   138,   190,   434,   191,    -1,    70,
     434,   197,    -1,   378,   193,   436,   194,    -1,   378,   193,
     438,   194,    -1,   364,    70,   430,   197,    -1,   365,    70,
     430,   197,    -1,   361,    -1,   445,    -1,   423,    -1,    91,
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
     429,    -1,   379,    -1,   426,    -1,   381,   155,   429,    -1,
     378,    -1,   124,    -1,   431,    -1,   190,   191,    -1,   331,
      -1,    -1,    -1,    90,    -1,   440,    -1,   190,   285,   191,
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
     218,    -1,    82,    -1,   445,    -1,   386,    -1,   198,   440,
     198,    -1,   199,   440,   199,    -1,   151,   440,   152,    -1,
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
     411,    -1,   429,    -1,   207,    -1,   193,   342,   194,    -1,
     413,    -1,   429,    -1,    70,   430,   197,    -1,   193,   342,
     194,    -1,   421,   415,    -1,   190,   330,   191,   415,    -1,
     432,   415,    -1,   190,   330,   191,   415,    -1,   190,   330,
     191,   410,   412,    -1,   190,   343,   191,   410,   412,    -1,
     190,   330,   191,   410,   411,    -1,   190,   343,   191,   410,
     411,    -1,   427,    -1,   377,    -1,   425,    -1,   426,    -1,
     416,    -1,   418,    -1,   420,   410,   412,    -1,   381,   155,
     429,    -1,   422,   190,   285,   191,    -1,   423,   190,   285,
     191,    -1,   190,   420,   191,    -1,   377,    -1,   425,    -1,
     426,    -1,   416,    -1,   420,   410,   412,    -1,   419,    -1,
     422,   190,   285,   191,    -1,   190,   420,   191,    -1,   381,
     155,   429,    -1,   427,    -1,   416,    -1,   377,    -1,   361,
      -1,   386,    -1,   190,   420,   191,    -1,   190,   343,   191,
      -1,   423,   190,   285,   191,    -1,   422,   190,   285,   191,
      -1,   190,   424,   191,    -1,   345,    -1,   348,    -1,   420,
     410,   414,   452,   190,   285,   191,    -1,   190,   330,   191,
     410,   414,   452,   190,   285,   191,    -1,   381,   155,   209,
     452,   190,   285,   191,    -1,   381,   155,   429,   190,   285,
     191,    -1,   381,   155,   193,   342,   194,   190,   285,   191,
      -1,   428,    -1,   428,    70,   430,   197,    -1,   428,   193,
     342,   194,    -1,   429,    -1,    83,    -1,    84,    -1,   195,
     193,   342,   194,    -1,   195,   429,    -1,   342,    -1,    -1,
     427,    -1,   417,    -1,   418,    -1,   431,   410,   412,    -1,
     380,   155,   427,    -1,   190,   420,   191,    -1,    -1,   417,
      -1,   419,    -1,   431,   410,   411,    -1,   190,   420,   191,
      -1,   433,     9,    -1,   433,     9,   420,    -1,   433,     9,
     137,   190,   433,   191,    -1,    -1,   420,    -1,   137,   190,
     433,   191,    -1,   435,   391,    -1,    -1,   435,     9,   342,
     136,   342,    -1,   435,     9,   342,    -1,   342,   136,   342,
      -1,   342,    -1,   435,     9,   342,   136,    38,   420,    -1,
     435,     9,    38,   420,    -1,   342,   136,    38,   420,    -1,
      38,   420,    -1,   437,   391,    -1,    -1,   437,     9,   342,
     136,   342,    -1,   437,     9,   342,    -1,   342,   136,   342,
      -1,   342,    -1,   439,   391,    -1,    -1,   439,     9,   387,
     136,   387,    -1,   439,     9,   387,    -1,   387,   136,   387,
      -1,   387,    -1,   440,   441,    -1,   440,    90,    -1,   441,
      -1,    90,   441,    -1,    83,    -1,    83,    70,   442,   197,
      -1,    83,   410,   207,    -1,   153,   342,   194,    -1,   153,
      82,    70,   342,   197,   194,    -1,   154,   420,   194,    -1,
     207,    -1,    85,    -1,    83,    -1,   127,   190,   332,   191,
      -1,   128,   190,   420,   191,    -1,   128,   190,   343,   191,
      -1,   128,   190,   424,   191,    -1,   128,   190,   423,   191,
      -1,   128,   190,   330,   191,    -1,     7,   342,    -1,     6,
     342,    -1,     5,   190,   342,   191,    -1,     4,   342,    -1,
       3,   342,    -1,   420,    -1,   444,     9,   420,    -1,   381,
     155,   208,    -1,   381,   155,   130,    -1,    -1,   102,   467,
      -1,   181,   451,    14,   467,   192,    -1,   408,   181,   451,
      14,   467,   192,    -1,   183,   451,   446,    14,   467,   192,
      -1,   408,   183,   451,   446,    14,   467,   192,    -1,   209,
      -1,   467,   209,    -1,   208,    -1,   467,   208,    -1,   209,
      -1,   209,   176,   458,   177,    -1,   207,    -1,   207,   176,
     458,   177,    -1,   176,   454,   177,    -1,    -1,   467,    -1,
     453,     9,   467,    -1,   453,   391,    -1,   453,     9,   169,
      -1,   454,    -1,   169,    -1,    -1,    -1,    32,   467,    -1,
     102,   467,    -1,   103,   467,    -1,   459,   391,    -1,   459,
       9,   460,   207,    -1,   460,   207,    -1,   459,     9,   460,
     207,   457,    -1,   460,   207,   457,    -1,    50,    -1,    51,
      -1,    -1,    91,   136,   467,    -1,    31,    91,   136,   467,
      -1,   220,   155,   207,   136,   467,    -1,   462,     9,   461,
      -1,   461,    -1,   462,   391,    -1,    -1,   180,   190,   463,
     191,    -1,   220,    -1,   207,   155,   466,    -1,   207,   452,
      -1,    31,   467,    -1,    59,   467,    -1,   220,    -1,   138,
      -1,   139,    -1,   464,    -1,   465,   155,   466,    -1,   138,
     176,   467,   391,   177,    -1,   138,   176,   467,     9,   467,
     177,    -1,   160,    -1,   190,   111,   190,   455,   191,    32,
     467,   191,    -1,   190,   467,     9,   453,   391,   191,    -1,
     467,    -1,    -1
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
    2666,  2667,  2671,  2672,  2673,  2677,  2682,  2687,  2688,  2692,
    2697,  2702,  2703,  2707,  2708,  2713,  2715,  2720,  2731,  2745,
    2757,  2772,  2773,  2774,  2775,  2776,  2777,  2778,  2788,  2797,
    2799,  2801,  2805,  2806,  2807,  2808,  2809,  2825,  2826,  2828,
    2830,  2837,  2838,  2839,  2840,  2841,  2842,  2843,  2844,  2846,
    2851,  2855,  2856,  2860,  2863,  2870,  2874,  2883,  2890,  2898,
    2900,  2901,  2905,  2906,  2907,  2909,  2914,  2915,  2926,  2927,
    2928,  2929,  2940,  2943,  2946,  2947,  2948,  2949,  2960,  2964,
    2965,  2966,  2968,  2969,  2970,  2974,  2976,  2979,  2981,  2982,
    2983,  2984,  2987,  2989,  2990,  2994,  2996,  2999,  3001,  3002,
    3003,  3007,  3009,  3012,  3015,  3017,  3019,  3023,  3024,  3026,
    3027,  3033,  3034,  3036,  3046,  3048,  3050,  3053,  3054,  3055,
    3059,  3060,  3061,  3062,  3063,  3064,  3065,  3066,  3067,  3068,
    3069,  3073,  3074,  3078,  3080,  3088,  3090,  3094,  3098,  3103,
    3107,  3115,  3116,  3120,  3121,  3127,  3128,  3137,  3138,  3146,
    3149,  3153,  3156,  3161,  3166,  3168,  3169,  3170,  3174,  3175,
    3179,  3180,  3183,  3188,  3191,  3193,  3197,  3203,  3204,  3205,
    3209,  3213,  3223,  3231,  3233,  3237,  3239,  3244,  3250,  3253,
    3258,  3266,  3269,  3272,  3273,  3276,  3279,  3280,  3285,  3288,
    3292,  3296,  3302,  3312,  3313
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
     421,   422,   422,   422,   422,   422,   422,   422,   422,   422,
     423,   424,   424,   425,   425,   426,   426,   426,   427,   428,
     428,   428,   429,   429,   429,   429,   430,   430,   431,   431,
     431,   431,   431,   431,   432,   432,   432,   432,   432,   433,
     433,   433,   433,   433,   433,   434,   434,   435,   435,   435,
     435,   435,   435,   435,   435,   436,   436,   437,   437,   437,
     437,   438,   438,   439,   439,   439,   439,   440,   440,   440,
     440,   441,   441,   441,   441,   441,   441,   442,   442,   442,
     443,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   444,   444,   445,   445,   446,   446,   447,   447,   447,
     447,   448,   448,   449,   449,   450,   450,   451,   451,   452,
     452,   453,   453,   454,   455,   455,   455,   455,   456,   456,
     457,   457,   458,   459,   459,   459,   459,   460,   460,   460,
     461,   461,   461,   462,   462,   463,   463,   464,   465,   466,
     466,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   468,   468
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
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     4,
       4,     1,     1,     1,     4,     2,     1,     0,     1,     1,
       1,     3,     3,     3,     0,     1,     1,     3,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     6,
       7,     1,     2,     1,     2,     1,     4,     1,     4,     3,
       0,     1,     3,     2,     3,     1,     1,     0,     0,     2,
       2,     2,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     3,     1,     2,     0,     4,     1,     3,
       2,     2,     2,     1,     1,     1,     1,     3,     5,     6,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   427,     0,     0,   795,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   886,
       0,   874,   677,     0,   683,   684,   685,    25,   742,   862,
     863,   151,   152,   686,     0,   132,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   186,     0,     0,     0,     0,
       0,     0,   393,   394,   395,   398,   397,   396,     0,     0,
       0,     0,   215,     0,     0,     0,   690,   692,   693,   687,
     688,     0,     0,     0,   694,   689,     0,   661,    27,    28,
      29,    31,    30,     0,   691,     0,     0,     0,     0,   695,
     399,   529,     0,   150,   122,     0,   678,     0,     0,     4,
     112,   114,   741,     0,   660,     0,     6,   185,     7,     9,
       8,    10,     0,     0,   391,   440,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   438,   851,   852,   511,   510,
     421,   514,     0,   420,   822,   662,   669,     0,   744,   509,
     390,   825,   826,   837,   439,     0,     0,   442,   441,   823,
     824,   821,   858,   861,   499,   743,    11,   398,   397,   396,
       0,     0,    31,     0,   112,   185,     0,   930,   439,   929,
       0,   927,   926,   513,     0,   428,   435,   433,     0,     0,
     481,   482,   483,   484,   508,   506,   505,   504,   503,   502,
     501,   500,   862,   686,   664,     0,     0,   950,   844,   662,
       0,   663,   462,     0,   460,     0,   890,     0,   751,   419,
     673,   205,     0,   950,   418,   672,   667,     0,   682,   663,
     869,   870,   876,   868,   674,     0,     0,   676,   507,     0,
       0,     0,     0,   424,     0,   130,   426,     0,     0,   136,
     138,     0,     0,   140,     0,    72,    71,    66,    65,    57,
      58,    49,    69,    80,    81,     0,    52,     0,    64,    56,
      62,    83,    75,    74,    47,    70,    90,    91,    48,    86,
      45,    87,    46,    88,    44,    92,    79,    84,    89,    76,
      77,    51,    78,    82,    43,    73,    59,    93,    67,    60,
      50,    42,    41,    40,    39,    38,    37,    61,    94,    96,
      54,    35,    36,    63,   984,   985,    55,   990,    34,    53,
      85,     0,     0,   112,    95,   941,   983,     0,   986,     0,
       0,   142,     0,     0,     0,   176,     0,     0,     0,     0,
       0,     0,   753,     0,   100,   102,   304,     0,     0,   303,
       0,   219,     0,   216,   309,     0,     0,     0,     0,     0,
     947,   201,   213,   882,   886,     0,   911,     0,   697,     0,
       0,     0,   909,     0,    16,     0,   116,   193,   207,   214,
     566,   541,     0,   935,   521,   523,   525,   799,   427,   440,
       0,     0,   438,   439,   441,     0,     0,   865,   679,     0,
     680,     0,     0,     0,   175,     0,     0,   118,   295,     0,
      24,   184,     0,   212,   197,   211,   396,   399,   185,   392,
     165,   166,   167,   168,   169,   171,   172,   174,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   874,     0,   164,   867,
     867,   896,     0,     0,     0,     0,     0,     0,     0,     0,
     389,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   461,   459,   800,   801,     0,   867,
       0,   813,   295,   295,   867,     0,   882,     0,   185,     0,
       0,   144,     0,   797,   792,   751,     0,   440,   438,     0,
     894,     0,   546,   750,   885,   682,   440,   438,   439,   118,
       0,   295,   417,     0,   815,   675,     0,   122,   255,     0,
     528,     0,   147,     0,     0,   425,     0,     0,     0,     0,
       0,   139,   163,   141,   984,   985,   981,   982,     0,   976,
       0,     0,     0,     0,    68,    33,    55,    32,   942,   170,
     173,   143,   122,     0,   160,   162,     0,     0,     0,     0,
     103,     0,   752,   101,    18,     0,    97,     0,   305,     0,
     145,   218,   217,     0,     0,   146,   931,     0,     0,   440,
     438,   439,   442,   441,     0,   969,   225,     0,   883,     0,
       0,   148,     0,     0,   696,   910,   742,     0,     0,   908,
     747,   907,   115,     5,    13,    14,     0,   223,     0,     0,
     534,     0,     0,   751,     0,     0,   670,   665,   535,     0,
       0,     0,     0,   799,   122,     0,   753,   798,   994,   416,
     430,   495,   831,   850,   127,   121,   123,   124,   125,   126,
     390,     0,   512,   745,   746,   113,   751,     0,   951,     0,
       0,     0,   753,   296,     0,   517,   187,   221,     0,   465,
     467,   466,   478,     0,     0,   498,   463,   464,   468,   470,
     469,   486,   485,   488,   487,   489,   491,   493,   492,   490,
     480,   479,   472,   473,   471,   474,   475,   477,   494,   476,
     866,     0,     0,   900,     0,   751,   934,     0,   933,   950,
     828,   203,   195,   209,     0,   935,   199,   185,     0,   431,
     434,   436,   444,   458,   457,   456,   455,   454,   453,   452,
     451,   450,   449,   448,   447,   803,     0,   802,   805,   827,
     809,   950,   806,     0,     0,     0,     0,     0,     0,     0,
       0,   928,   429,   790,   794,   750,   796,     0,   666,     0,
     889,     0,   888,   221,     0,   666,   873,   872,   858,   861,
       0,     0,   802,   805,   871,   806,   422,   257,   259,   122,
     532,   531,   423,     0,   122,   239,   131,   426,     0,     0,
       0,     0,     0,   251,   251,   137,   751,     0,     0,     0,
     974,   751,     0,   957,     0,     0,     0,     0,     0,   749,
       0,   661,     0,     0,   699,   660,   704,     0,   698,   120,
     703,   950,   987,     0,     0,     0,     0,    19,     0,    20,
       0,    98,     0,     0,     0,   109,   753,     0,   107,   102,
      99,   104,     0,   302,   310,   307,     0,     0,   920,   925,
     922,   921,   924,   923,    12,   967,   968,     0,   751,     0,
       0,     0,   882,   879,     0,   545,   919,   918,   917,     0,
     913,     0,   914,   916,     0,     5,     0,     0,     0,   560,
     561,   569,   568,     0,   438,     0,   750,   540,   544,     0,
       0,   936,     0,   522,     0,     0,   958,   799,   281,   993,
       0,     0,   814,     0,   864,   750,   953,   949,   297,   298,
     659,   752,   294,     0,   799,     0,     0,   223,   519,   189,
     497,     0,   549,   550,     0,   547,   750,   895,     0,     0,
     295,   225,     0,   223,     0,     0,   221,     0,   874,   445,
       0,     0,   811,   812,   829,   830,   859,   860,     0,     0,
       0,   778,   758,   759,   760,   767,     0,     0,     0,   771,
     769,   770,   784,   751,     0,   792,   893,   892,     0,   223,
       0,   816,   681,     0,   261,     0,     0,   128,     0,     0,
       0,     0,     0,     0,     0,   231,   232,   243,     0,   122,
     241,   157,   251,     0,   251,     0,   750,     0,     0,     0,
       0,   750,   975,   977,   956,   751,   955,     0,   751,   725,
     726,   723,   724,   757,     0,   751,   749,     0,   543,     0,
       0,   902,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     980,   177,     0,   180,   161,     0,     0,   105,   110,   111,
     103,   752,   108,     0,   306,     0,   932,   149,   948,   969,
     962,   964,   224,   226,   316,     0,     0,   880,     0,   912,
       0,    17,     0,   935,   222,   316,     0,     0,   666,   537,
       0,   671,   937,     0,   958,   526,     0,     0,   994,     0,
     286,   284,   805,   817,   950,   805,   818,   952,     0,     0,
     299,   119,     0,   799,   220,     0,   799,     0,   496,   899,
     898,     0,   295,     0,     0,     0,     0,     0,     0,   223,
     191,   682,   804,   295,     0,   763,   764,   765,   766,   772,
     773,   782,     0,   751,     0,   778,     0,   762,   786,   750,
     789,   791,   793,     0,   887,     0,   804,     0,     0,     0,
       0,   258,   533,   133,     0,   426,   231,   233,   882,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   245,     0,
     988,     0,   970,     0,   973,   750,     0,     0,     0,   701,
     750,   748,     0,   739,     0,   751,     0,   705,   740,   738,
     906,     0,   751,   708,   710,   709,     0,     0,   706,   707,
     711,   713,   712,   728,   727,   730,   729,   731,   733,   735,
     734,   732,   721,   720,   715,   716,   714,   717,   718,   719,
     722,   979,     0,   122,     0,     0,   106,    21,   308,     0,
       0,     0,   966,     0,   390,   884,   882,   432,   437,   443,
       0,    15,     0,   390,   572,     0,     0,   574,   567,   570,
       0,   565,     0,   939,     0,   959,   530,     0,   287,     0,
       0,   282,     0,   301,   300,   958,     0,   316,     0,   799,
       0,   295,     0,   856,   316,   935,   316,   938,     0,     0,
       0,   446,     0,     0,   775,   750,   777,   768,     0,   761,
       0,     0,   751,   783,   891,   316,     0,   122,     0,   254,
     240,     0,     0,     0,   230,   153,   244,     0,     0,   247,
       0,   252,   253,   122,   246,   989,   971,     0,   954,     0,
     992,   756,   755,   700,     0,   750,   542,   702,     0,   548,
     750,   901,   737,     0,     0,     0,    22,    23,   963,   960,
     961,   227,     0,     0,     0,   397,   388,     0,     0,     0,
     202,   315,   317,     0,   387,     0,     0,     0,   935,   390,
       0,   915,   312,   208,   563,     0,     0,   536,   524,     0,
     290,   280,     0,   283,   289,   295,   516,   958,   390,   958,
       0,   897,     0,   855,   390,     0,   390,   940,   316,   799,
     853,   781,   780,   774,     0,   776,   750,   785,   390,   122,
     260,   129,   134,   155,   234,     0,   242,   248,   122,   250,
     972,     0,     0,   539,     0,   905,   904,   736,   122,   181,
     965,     0,     0,     0,   943,     0,     0,     0,   228,     0,
     935,     0,   353,   349,   355,   661,    31,     0,   343,     0,
     348,   352,   365,     0,   363,   368,     0,   367,     0,   366,
       0,   185,   319,     0,   321,     0,   322,   323,     0,     0,
     881,     0,   564,   562,   573,   571,   291,     0,     0,   278,
     288,     0,     0,   958,     0,   198,   516,   958,   857,   204,
     312,   210,   390,     0,     0,   788,     0,   206,   256,     0,
       0,   122,   237,   154,   249,   991,   754,     0,     0,     0,
       0,     0,   415,     0,   944,     0,   333,   337,   412,   413,
     347,     0,     0,     0,   328,   625,   624,   621,   623,   622,
     642,   644,   643,   613,   583,   585,   584,   603,   619,   618,
     579,   590,   591,   593,   592,   612,   596,   594,   595,   597,
     598,   599,   600,   601,   602,   604,   605,   606,   607,   608,
     609,   611,   610,   580,   581,   582,   586,   587,   589,   627,
     628,   637,   636,   635,   634,   633,   632,   620,   639,   629,
     630,   631,   614,   615,   616,   617,   640,   641,   645,   647,
     646,   648,   649,   626,   651,   650,   653,   655,   654,   588,
     658,   656,   657,   652,   638,   578,   360,   575,     0,   329,
     381,   382,   380,   373,     0,   374,   330,   407,     0,     0,
       0,     0,   411,     0,   185,   194,   311,     0,     0,     0,
     279,   293,   854,     0,     0,   383,   122,   188,   958,     0,
       0,   200,   958,   779,     0,   122,   235,   135,   156,     0,
     538,   903,   179,   331,   332,   410,   229,     0,   751,   751,
       0,   356,   344,     0,     0,     0,   362,   364,     0,     0,
     369,   376,   377,   375,     0,     0,   318,   945,     0,     0,
       0,   414,     0,   313,     0,   292,     0,   558,   753,   122,
       0,     0,   190,   196,     0,   787,     0,     0,   158,   334,
     112,     0,   335,   336,     0,   750,     0,   750,   358,   354,
     359,   576,   577,     0,   345,   378,   379,   371,   372,   370,
     408,   405,   969,   324,   320,   409,     0,   314,   559,   752,
       0,     0,   384,   122,   192,     0,   238,     0,   183,     0,
     390,     0,   350,   357,   361,     0,     0,   799,   326,     0,
     556,   515,   518,     0,   236,     0,     0,   159,   341,     0,
     389,   351,   406,   946,     0,   753,   401,   799,   557,   520,
       0,   182,     0,     0,   340,   958,   799,   265,   402,   403,
     404,   994,   400,     0,     0,     0,   339,     0,   401,     0,
     958,     0,   338,   385,   122,   325,   994,     0,   270,   268,
       0,   122,     0,     0,   271,     0,     0,   266,   327,     0,
     386,     0,   274,   264,     0,   267,   273,   178,   275,     0,
       0,   262,   272,     0,   263,   277,   276
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   109,   875,   613,   174,  1434,   709,
     341,   342,   343,   344,   836,   837,   838,   111,   112,   113,
     114,   115,   395,   645,   646,   534,   244,  1499,   540,  1415,
    1500,  1738,   825,   336,   562,  1698,  1054,  1233,  1757,   412,
     175,   647,   915,  1117,  1290,   119,   616,   932,   648,   667,
     936,   596,   931,   224,   515,   649,   617,   933,   414,   361,
     378,   122,   917,   878,   861,  1072,  1437,  1170,   985,  1647,
    1503,   786,   991,   539,   795,   993,  1323,   778,   974,   977,
    1159,  1764,  1765,   635,   636,   661,   662,   348,   349,   355,
    1471,  1626,  1627,  1244,  1361,  1460,  1620,  1747,  1767,  1657,
    1702,  1703,  1704,  1447,  1448,  1449,  1450,  1659,  1660,  1666,
    1714,  1453,  1454,  1458,  1613,  1614,  1615,  1637,  1795,  1362,
    1363,   176,   124,  1781,  1782,  1618,  1365,  1366,  1367,  1368,
     125,   237,   535,   536,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,  1483,   136,   914,  1116,   137,   632,
     633,   634,   241,   387,   530,   622,   623,  1195,   624,  1196,
     138,   139,   140,   816,   141,   142,  1688,   143,   618,  1473,
     619,  1086,   883,  1261,  1258,  1606,  1607,   144,   145,   146,
     227,   147,   228,   238,   399,   522,   148,  1013,   820,   149,
    1014,   906,   573,  1015,   960,  1139,   961,  1141,  1142,  1143,
     963,  1301,  1302,   964,   754,   505,   188,   189,   650,   638,
     488,  1102,  1103,   740,   741,   902,   151,   230,   152,   153,
     178,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     701,   234,   235,   599,   217,   218,   704,   705,  1201,  1202,
     371,   372,   869,   164,   587,   165,   631,   166,   327,  1628,
    1678,   362,   407,   656,   657,  1007,  1097,  1242,   857,   858,
     859,   800,   801,   802,   328,   329,   822,  1436,   900
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1498
static const yytype_int16 yypact[] =
{
   -1498,   157, -1498, -1498,  5424, 13107, 13107,   -36, 13107, 13107,
   13107, 10940, 13107, 13107, -1498, 13107, 13107, 13107, 13107, 13107,
   13107, 13107, 13107, 13107, 13107, 13107, 13107, 16411, 16411, 11137,
   13107, 16506,   -14,    34, -1498, -1498, -1498, -1498, -1498,   184,
   -1498, -1498, -1498,   198, 13107, -1498,    34,    98,   153,   172,
   -1498,    34, 11334,   542, 11531, -1498, 13990,  9955,    39, 13107,
    1817,   147, -1498, -1498, -1498,    55,    51,    66,   196,   202,
     227,   275, -1498,   542,   301,   322, -1498, -1498, -1498, -1498,
   -1498, 13107,   679,   678, -1498, -1498,   542, -1498, -1498, -1498,
   -1498,   542, -1498,   542, -1498,   187,   333,   542,   542, -1498,
     317, -1498, 11728, -1498, -1498,   310,   530,   580,   580, -1498,
     379,   406,   386,   370, -1498,    85, -1498,   546, -1498, -1498,
   -1498, -1498,   608,  1053, -1498, -1498,   439,   441,   447,   452,
     466,   482,   484,   494, 14716, -1498, -1498, -1498, -1498,   138,
   -1498,   618,   628, -1498,   142,   510, -1498,   561,    10, -1498,
    2902,   144, -1498, -1498,  1626,   140,   533,   156, -1498,   155,
     170,   536,   212, -1498, -1498,   650, -1498, -1498, -1498,   575,
     556,   577, -1498, 13107, -1498,   546,  1053, 16980,  2215, 16980,
   13107, 16980, 16980,  3867,   560,  4745,  3867, 16980,   713,   542,
     703,   703,   532,   703,   703,   703,   703,   703,   703,   703,
     703,   703, -1498, -1498, -1498,    43, 13107,   572, -1498, -1498,
     621,   589,    53,   597,    53, 16411, 15553,   593,   813, -1498,
     575, -1498, 13107,   572, -1498,   669, -1498,   673,   639, -1498,
     163, -1498, -1498, -1498,    53,   140, 11925, -1498, -1498, 13107,
    8773,   827,    97, 16980,  9758, -1498, 13107, 13107,   542, -1498,
   -1498, 14764,   644, -1498, 14828, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498,  3648, -1498,  3648, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498,    91,    93,   577, -1498, -1498, -1498,
   -1498,   649,  2819,    95, -1498, -1498,   691,   842, -1498,   712,
   15091, -1498,   676,   677, 14876, -1498,    40, 14924,  1121,  1121,
     542,   682,   861,   685, -1498,   248, -1498, 16019,    99, -1498,
     751, -1498,   752, -1498,   871,   101, 16411, 13107, 13107,   695,
     715, -1498, -1498, 16117, 11137,   102,   112,   434, -1498, 13304,
   16411,   720, -1498,   542, -1498,   355,   406, -1498, -1498, -1498,
   -1498, 16601,   878,   792, -1498, -1498, -1498,    75, 13107,   704,
     705, 16980,   709,  2073,   714,  5621, 13107, -1498,   397,   708,
     744,   397,   453,   437, -1498,   542,  3648,   719, 10152, 13990,
   -1498, -1498,   686, -1498, -1498, -1498, -1498, -1498,   546, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, 13107, 13107,
   13107, 13107, 12122, 13107, 13107, 13107, 13107, 13107, 13107, 13107,
   13107, 13107, 13107, 13107, 13107, 13107, 13107, 13107, 13107, 13107,
   13107, 13107, 13107, 13107, 13107, 13107, 16696, 13107, -1498, 13107,
   13107, 13107,  4814,   542,   542,   542,   542,   542,   608,   800,
     896,  4574, 13107, 13107, 13107, 13107, 13107, 13107, 13107, 13107,
   13107, 13107, 13107, 13107, -1498, -1498, -1498, -1498,  1256, 13107,
   13107, -1498, 10152, 10152, 13107, 13107, 16117,   723,   546, 12319,
   14988, -1498, 13107, -1498,   730,   914,   772,   735,   739, 13501,
      53, 12516, -1498, 12713, -1498,   639,   740,   741,  2175, -1498,
      61, 10152, -1498,  1327, -1498, -1498, 15036, -1498, -1498, 10349,
   -1498, 13107, -1498,   839,  8970,   926,   746, 16859,   922,    81,
     128, -1498, -1498, -1498,   766, -1498, -1498, -1498,  3648,   591,
     761,   946, 15595,   542, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498,   767, -1498, -1498,   763,   773,   776,   781,
     253,  2019,  1297, -1498, -1498,   542,   542, 13107,    53,   147,
   -1498, -1498, -1498, 15595,   892, -1498,    53,   119,   120,   786,
     788,  2597,    71,   789,   794,   331,   849,   798,    53,   122,
     793, -1498,   893,   542, -1498, -1498,   919,  3304,    17, -1498,
   -1498, -1498,   406, -1498, -1498, -1498,   958,   858,   819,   220,
     853, 13107,   875,  1004,   831,   869, -1498,   175, -1498,  3648,
    3648,  1011,   827,    75, -1498,   840,  1020, -1498,  3648,    88,
   -1498,   522,   167, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
    1084,  3424, -1498, -1498, -1498, -1498,  1021,   854, -1498, 16411,
   13107,   841,  1026, 16980,  1023, -1498, -1498,   906,   981, 11516,
   17072,  3867,  3341, 13107, 16932, 13498, 12495,  3178, 13814, 12098,
    4043, 14326, 14326, 14326, 14326,  3616,  3616,  3616,  3616,  3616,
    1562,  1562,   895,   895,   895,   532,   532,   532, -1498,   703,
   16980,   851,   855, 15671,   856,  1044,   225, 13107,   345,   572,
      64, -1498, -1498, -1498,  1040,   792, -1498,   546, 16215, -1498,
   -1498, -1498,  3867,  3867,  3867,  3867,  3867,  3867,  3867,  3867,
    3867,  3867,  3867,  3867,  3867, -1498, 13107,   352, -1498,   179,
   -1498,   572,   419,   864,  3523,   873,   877,   872,  3830,   126,
     882, -1498, 16980,  3103, -1498,   542, -1498,    88,   424, 16411,
   16980, 16411, 15778,   906,    88,    53,   180, -1498,   175,   920,
     883, 13107, -1498,   181, -1498, -1498, -1498,  8576,   463, -1498,
   -1498, 16980, 16980,    34, -1498, -1498, -1498, 13107,   974,  4081,
   15595,   542,  9167,   886,   888, -1498,  1076,   996,   952,   934,
   -1498,  1081,   900,  2192,  3648, 15595, 15595, 15595, 15595, 15595,
     903,   939,   905, 15595,   223,   942, -1498,   907, -1498,  4290,
   -1498,   211, -1498,  5818,  1705,   909,  1297, -1498,  1297, -1498,
     542,   542,  1297,  1297,   542, -1498,  1089,   925, -1498,   294,
   -1498, -1498,  4425, -1498,  4290,  1100, 16411,   911, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498,   943,  1112,   542,
    1705,   930, 16117, 16313,  1110, -1498, -1498, -1498, -1498,   928,
   -1498, 13107, -1498, -1498,  5030, -1498,  3648,  1705,   933, -1498,
   -1498, -1498, -1498,  1113,   945, 13107, 16601, -1498, -1498,  4814,
     947, -1498,  3648, -1498,   949,  6015,  1117,   135, -1498, -1498,
     110,  1256, -1498,  1327, -1498,  3648, -1498, -1498,    53, 16980,
   -1498, 10546, -1498, 15595,    67,   960,  1705,   858, -1498, -1498,
   13498, 13107, -1498, -1498, 13107, -1498, 13107, -1498, 10924,   961,
   10152,   849,  1120,   858,  3648,  1139,   906,   542, 16696,    53,
   11318,   967, -1498, -1498,   168,   969, -1498, -1498,  1147,  1153,
    1153,  3103, -1498, -1498, -1498,  1111,   975,    84,   976, -1498,
   -1498, -1498, -1498,  1158,   977,   730,    53,    53, 12910,   858,
    1327, -1498, -1498, 11909,   529,    34,  9758, -1498,  6212,   986,
    6409,   988,  4081, 16411,   979,  1045,    53,  4290,  1169, -1498,
   -1498, -1498, -1498,   426, -1498,   375,  3648,  1010,  1052,  3648,
     542,   591, -1498, -1498, -1498,  1191, -1498,  1018,  1021,   501,
     501,  1132,  1132, 15881,  1013,  1203, 15595, 15375, 16601,  3966,
   15233, 15595, 15595, 15595, 15595, 15477, 15595, 15595, 15595, 15595,
   15595, 15595, 15595, 15595, 15595, 15595, 15595, 15595, 15595, 15595,
   15595, 15595, 15595, 15595, 15595, 15595, 15595, 15595, 15595,   542,
   -1498, -1498,  1130, -1498, -1498,  1022,  1024, -1498, -1498, -1498,
     381,  2019, -1498,  1025, -1498, 15595,    53, -1498, -1498,   154,
   -1498,   539,  1210, -1498, -1498,   127,  1030,    53, 10743, -1498,
    2446, -1498,  5227,   792,  1210, -1498,     3,    -2, -1498, 16980,
    1085,  1032, -1498,  1033,  1117, -1498,  3648,   827,  3648,    60,
    1212,  1144,   182, -1498,   572,   216, -1498, -1498, 16411, 13107,
   16980,  4290,  1046,    67, -1498,  1047,    67,  1055, 13498, 16980,
   15826,  1056, 10152,  1051,  1054,  3648,  1060,  1057,  3648,   858,
   -1498,   639,   456, 10152, 13107, -1498, -1498, -1498, -1498, -1498,
   -1498,  1118,  1061,  1239,  1170,  3103,  1108, -1498, 16601,  3103,
   -1498, -1498, -1498, 16411, 16980,  1069, -1498,    34,  1232,  1189,
    9758, -1498, -1498, -1498,  1077, 13107,  1045,    53, 16117,  4081,
    1080, 15595,  6606,   700,  1082, 13107,   105,   423, -1498,  1091,
   -1498,  3648, -1498,  1137, -1498,  2915,  1245,  1087, 15595, -1498,
   15595, -1498,  1088, -1498,  1149,  1281,  1102, -1498, -1498, -1498,
   15929,  1097,  1286, 10132, 10526, 11117, 15595, 17028, 12692, 12888,
   13985, 14155, 14495, 15082, 15082, 15082, 15082,  3096,  3096,  3096,
    3096,  3096,  1489,  1489,   501,   501,   501,  1132,  1132,  1132,
    1132, -1498,  1105, -1498,  1106,  1114, -1498, -1498,  4290,   542,
    3648,  3648, -1498,  1705,  1073, -1498, 16117, -1498, -1498,  3867,
    1115, -1498,  1119,  1180, -1498,   194, 13107, -1498, -1498, -1498,
   13107, -1498, 13107, -1498,   827, -1498, -1498,   145,  1296,  1230,
   13107, -1498,  1124,    53, 16980,  1117,  1133, -1498,  1134,    67,
   13107, 10152,  1136, -1498, -1498,   792, -1498, -1498,  1129,  1135,
    1141, -1498,  1138,  3103, -1498,  3103, -1498, -1498,  1142, -1498,
    1187,  1145,  1326, -1498,    53, -1498,  1309, -1498,  1152, -1498,
   -1498,  1155,  1156,   129, -1498, -1498,  4290,  1157,  1159, -1498,
   14668, -1498, -1498, -1498, -1498, -1498, -1498,  3648, -1498,  3648,
   -1498,  4290, 15984, -1498, 15595, 16601, -1498, -1498, 15595, -1498,
   15595, -1498, 12299, 15595,  1160,  6803, -1498, -1498,   539, -1498,
   -1498, -1498,   520, 14161,  1705,  1238, -1498,   562,  1192,   968,
   -1498, -1498, -1498,   800,  2680,   103,   104,  1163,   792,   896,
     131, -1498, -1498, -1498,  1197, 13091, 13288, 16980, -1498,   207,
    1344,  1278, 13107, -1498, 16980, 10152,  1248,  1117,  1583,  1117,
    1171, 16980,  1176, -1498,  1783,  1175,  1830, -1498, -1498,    67,
   -1498, -1498,  1233, -1498,  3103, -1498, 16601, -1498,  1921, -1498,
    8576, -1498, -1498, -1498, -1498,  9364, -1498, -1498, -1498,  8576,
   -1498,  1182, 15595,  4290,  1235,  4290, 16032, 12299, -1498, -1498,
   -1498,  1705,  1705,   542, -1498,  1362, 15375,    83, -1498, 14161,
     792,  2588, -1498,  1199, -1498,   106,  1186,   109, -1498, 14504,
   -1498, -1498, -1498,   113, -1498, -1498,   880, -1498,  1190, -1498,
    1298,   546, -1498, 14333, -1498, 14333, -1498, -1498,  1370,   800,
   -1498, 13648, -1498, -1498, -1498, -1498,  1371,  1304, 13107, -1498,
   16980,  1198,  1205,  1117,   474, -1498,  1248,  1117, -1498, -1498,
   -1498, -1498,  1940,  1202,  3103, -1498,  1260, -1498,  8576,  9561,
    9364, -1498, -1498, -1498,  8576, -1498,  4290, 15595, 15595,  7000,
    1206,  1211, -1498, 15595, -1498,  1705, -1498, -1498, -1498, -1498,
   -1498,  3648,  1361,   562, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498,   158, -1498,  1192, -1498,
   -1498, -1498, -1498, -1498,   111,   584, -1498,  1383,   114, 15091,
    1298,  1388, -1498,  3648,   546, -1498, -1498,  1213,  1390, 13107,
   -1498, 16980, -1498,   123,  1216, -1498, -1498, -1498,  1117,   474,
   13819, -1498,  1117, -1498,  3103, -1498, -1498, -1498, -1498,  7197,
    4290,  4290, -1498, -1498, -1498,  4290, -1498,   632,  1405,  1411,
    1234, -1498, -1498, 15595, 14504, 14504,  1365, -1498,   880,   880,
     710, -1498, -1498, -1498, 15595,  1343, -1498,  1255,  1242,   116,
   15595, -1498,   542, -1498, 15595, 16980,  1352, -1498,  1427, -1498,
    7394,  1249, -1498, -1498,   474, -1498,  7591,  1251,  1329, -1498,
    1345,  1293, -1498, -1498,  1348,  3648,  1276,  1361, -1498, -1498,
    4290, -1498, -1498,  1287, -1498,  1422, -1498, -1498, -1498, -1498,
    4290,  1451,   331, -1498, -1498,  4290,  1280,  4290, -1498,   133,
    1277,  7788, -1498, -1498, -1498,  1283, -1498,  1289,  1303,   542,
     896,  1305, -1498, -1498, -1498, 15595,  1306,    78, -1498,  1397,
   -1498, -1498, -1498,  7985, -1498,  1705,   909, -1498,  1314,   542,
     697, -1498,  4290, -1498,  1294,  1477,   528,    78, -1498, -1498,
    1408, -1498,  1705,  1307, -1498,  1117,   121, -1498, -1498, -1498,
   -1498,  3648, -1498,  1315,  1317,   117, -1498,   487,   528,   160,
    1117,  1295, -1498, -1498, -1498, -1498,  3648,   221,  1481,  1417,
     487, -1498,  8182,   161,  1495,  1430, 13107, -1498, -1498,  8379,
   -1498,   244,  1496,  1431, 13107, -1498, 16980, -1498,  1502,  1441,
   13107, -1498, 16980, 13107, -1498, 16980, 16980
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1498, -1498, -1498,  -539, -1498, -1498, -1498,   162,   -11,   -30,
     401, -1498,  -243,  -493, -1498, -1498,   465,    31,  1529, -1498,
    2610, -1498,  -395, -1498,    63, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498,  -229, -1498, -1498,  -153,
      27,    28, -1498, -1498, -1498, -1498, -1498, -1498,    33, -1498,
   -1498, -1498, -1498, -1498, -1498,    36, -1498, -1498,  1062,  1066,
    1068,   -77,  -670,  -830,   603,   658,  -235,   372,  -899, -1498,
      45, -1498, -1498, -1498, -1498,  -714,   228, -1498, -1498, -1498,
   -1498,  -217, -1498,  -598, -1498,  -423, -1498, -1498,   973, -1498,
      65, -1498, -1498, -1013, -1498, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498,    38, -1498,   118, -1498, -1498, -1498,
   -1498, -1498,   -44, -1498,   208,  -978, -1498, -1497,  -232, -1498,
    -141,    19,  -121,  -219, -1498,   -50, -1498, -1498, -1498,   218,
     -20,    -3,    20,  -706,   -74, -1498, -1498,    42, -1498,     8,
   -1498, -1498,    -5,   -43,    89, -1498, -1498, -1498, -1498, -1498,
   -1498, -1498, -1498, -1498,  -572,  -824, -1498, -1498, -1498, -1498,
   -1498,  1933, -1498, -1498, -1498, -1498, -1498,   485, -1498, -1498,
   -1498, -1498, -1498, -1498, -1498, -1498,  -880, -1498,  2090,     5,
   -1498,  1658,  -385, -1498, -1498,  -471,  3340,  3386, -1498, -1498,
     564,  -161,  -609, -1498, -1498,   633,   438,  -685,   433, -1498,
   -1498, -1498, -1498, -1498,   619, -1498, -1498, -1498,    73,  -868,
    -146,  -404,  -402, -1498,   687,   -89, -1498, -1498,     7,    30,
     571, -1498, -1498,  1364,   -17, -1498,  -325,     2,  -340,    57,
     289, -1498, -1498,  -454,  1222, -1498, -1498, -1498, -1498, -1498,
     754,   600, -1498, -1498, -1498,  -323,  -626, -1498,  1181,  -801,
   -1498,   -47,  -160,     9,   790, -1498,  -887,   243,  -130, -1498,
     525,   595, -1498, -1498, -1498, -1498,   548,   478, -1064
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -979
static const yytype_int16 yytable[] =
{
     177,   179,   419,   181,   182,   183,   185,   186,   187,   469,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   497,   123,   216,   219,   325,   898,   390,  1098,
     240,   117,   118,   233,  1267,   894,   226,   120,   231,   243,
     121,   627,   749,   245,   763,   324,   379,   251,   249,   254,
     382,   383,   334,   912,   337,   419,   626,   514,   628,   392,
     893,   232,  1090,   519,   242,   333,   491,   116,   962,   745,
     746,   698,  1253,   468,   874,   415,   243,   150,   835,   840,
     995,   981,   389,  1166,   738,   394,   739,  1115,   523,   935,
     791,   345,  1515,   969,   409,   567,   569,   391,   770,   332,
     -68,   365,   -33,  1126,   -32,   -68,   531,   -33,   579,   -32,
     584,   531,  1463,  1465,   375,  -346,    14,   376,  1523,   773,
    1668,   774,  1608,  1675,    14,  1675,  1515,    14,   846,   531,
     392,   863,   777,   506,  -840,   863,   863,  1321,   863,  1155,
     863,  -553,  1692,  1268,   202,    40,   524,  1669,  1099,   354,
     418,   486,   487,   389,   180,   563,   394,     3,   489,  1259,
     793,  1686,   397,   508,  1254,  -663,   110,   823,   391,   470,
      14,  1749,  1663,   352,  1146,   500,   236,  1255,   350,   517,
     768,   353,   602,  1379,    14,   351,   507,   486,   487,   394,
    1664,  1260,  -527,  1100,  1194,   368,  1256,  1734,  1797,  1811,
    -845,   391,   516,   498,   855,   856,  1687,  1264,  -551,  1665,
     489,   873,  -832,   564,  -835,   252,  1750,   391,   323,  -671,
     603,  -664,   486,   487,   239,  -833,  -553,  1062,  1380,  1269,
     346,   526,   335,  -875,   526,   360,  1147,  -839,  -838,   895,
    -834,   243,   537,  1798,  1812,   494,   486,   487,  1374,  -836,
    -878,  -877,  -819,   377,   930,   360,   105,  -840,  -285,   360,
     360,   493,   852,  -285,  1388,   668,  1129,   548,  -554,  -269,
    1314,  1394,   792,  1396,   880,  1516,  1517,   410,  1173,  1101,
    1177,   490,   494,   -68,   360,   -33,  -820,   -32,   246,   532,
    1476,   580,  1408,   585,   601,  1464,  1466,  1322,  -346,  1289,
     558,  1524,  1670,   528,  1804,  1609,  1676,   533,  1724,  1792,
     847,   848,  -752,   864,  1381,   590,  1112,   948,  1245,   324,
    1414,   794,  1470,  -752,  1300,  -670,  -752,  1818,  -844,  1799,
    1813,  -750,  -843,   490,  -842,  -832,  1082,  -835,   589,  1058,
    1059,   593,   347,   247,   756,   750,   493,   380,  -833,   419,
     575,   504,   243,   391,   380,   830,  -875,  -846,  -849,   216,
    -839,  -838,   248,  -834,   607,   666,  1049,  -665,   495,   345,
     345,   570,  -836,  -878,  -877,  -819,  1477,   588,  -950,   325,
     881,   855,   856,   185,   978,  1492,   356,   406,  1386,   980,
    1805,   651,   357,   202,    40,   882,   575,   719,   324,   406,
     384,   -96,   110,   663,   612,   495,   110,   576,  1075,  -820,
     538,   404,   831,  1819,   123,   -96,  -950,   358,   379,   714,
     715,   415,   117,   669,   670,   671,   672,   674,   675,   676,
     677,   678,   679,   680,   681,   682,   683,   684,   685,   686,
     687,   688,   689,   690,   691,   692,   693,   694,   695,   696,
     697,   708,   699,   405,   700,   700,   703,  1252,   233,  1311,
     637,   226,   887,   231,  1303,   359,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   721,
     366,  1175,  1176,   830,   700,   744,   232,   663,   663,   700,
     748,   363,   557,   901,   722,   903,   717,   752,   324,  1105,
    1484,  1106,  1486,   396,   385,   105,   760,  1123,   762,   469,
     386,  1424,   364,   720,   405,  1276,   663,   366,  1278,   710,
     366,   -95,   767,   381,   781,  1266,   782,   609,  -807,  1175,
    1176,  1174,  1175,  1176,   330,   -95,   366,   780,   975,   976,
     405,  -950,  -807,   609,   927,   742,   627,   614,   615,   929,
     369,   370,  1435,  1131,  1045,  1046,  1047,   110,   486,   487,
     408,   626,   406,   628,   937,   405,   710,   655,   835,  1178,
    1048,   323,   842,   468,   360,   154,  -950,   769,   884,  -950,
     775,   941,  1496,  1055,   411,  1056,   604,   369,   370,   456,
     369,   370,  -555,  1441,  1172,  -810,  1634,   785,   212,   214,
    1639,   457,   839,   839,  1157,  1158,   369,   370,  1401,  -810,
    1402,   901,   903,   366,  -847,  1671,   391,  1324,   970,   903,
     398,   919,   797,    37,   557,   360,   712,   360,   360,   360,
     360,   420,  -808,   421,  1672,   997,   654,  1673,  1518,   422,
    1002,  1240,  1241,    37,   423,    50,  -808,  1778,  1779,  1780,
     737,   653,  1431,  1432,   519,   909,   486,   487,   424,  1395,
    1291,  1050,  1621,   366,  1622,    50,  1635,  1636,   920,   470,
     401,   557,    37,   393,   425,   971,   426,  -666,   627,  1793,
    1794,  1390,   798,   369,   370,   772,   427,  1442,   459,    37,
    1715,  1716,  1378,   626,    50,   628,   110,  1070,   460,  1282,
    1443,  1444,   928,   461,    88,    89,   637,    90,   172,    92,
    1292,    50,  -847,    37,  1313,   821,   462,  1789,   171,  1495,
    -552,    86,  1445,   492,    88,    89,  -841,    90,  1446,    92,
    -664,   940,  1803,   369,   370,    50,   373,   841,   655,   566,
     568,  1717,  1468,   546,   393,   547,   496,   171,   406,   702,
      86,  1691,   501,    88,    89,  1694,    90,   172,    92,    37,
    1718,   503,   366,  1719,   868,   870,   973,    37,   413,   367,
      88,    89,   457,    90,   172,    92,   509,   393,   743,  -845,
     979,    50,   243,   747,  1711,  1712,   510,   493,   171,    50,
     512,    86,  1370,   518,    88,    89,   123,    90,   172,    92,
     551,  1493,  1150,   366,   117,  1318,  1175,  1176,   627,  1643,
     609,   154,  1005,  1008,  1519,   154,    62,    63,    64,   167,
     168,   416,   513,   626,  -662,   628,  1699,   366,   520,   521,
     360,   368,   369,   370,   609,   529,   542,   373,  1345,   549,
      88,    89,   123,    90,   172,    92,  -978,  1187,    88,    89,
     117,    90,   172,    92,  1191,   990,   552,   839,  1392,   839,
     400,   402,   403,   839,   839,  1060,  1080,   553,   559,   560,
     572,   374,   610,   369,   370,   571,   665,   574,   708,  1766,
    1089,   581,   582,   417,   658,   583,   594,   330,  1787,  1774,
    1130,   595,   629,   123,   630,   639,   640,   369,   370,  1766,
     641,   117,   118,  1800,   652,   643,  1110,   120,  1788,  -117,
     121,    55,  1410,   665,   123,   959,  1118,   965,   578,  1119,
     753,  1120,   117,   755,   604,   663,   757,   586,  1419,   591,
     758,   764,   765,   783,   598,   531,   790,   116,   787,   110,
     233,   608,   548,   226,  1272,   231,  1091,   150,   453,   454,
     455,   803,   456,   988,   110,   804,   826,   824,   742,  1695,
     775,    37,  1481,  1154,   457,   827,   154,   605,   232,   828,
     637,   611,  1160,   829,    37,   845,   866,   849,   867,   850,
     853,   860,  1296,    50,   865,   110,   854,   637,   862,   871,
     876,   877,  1057,   655,   879,   627,    50,   123,   605,   123,
     611,   605,   611,   611,  1247,   117,  1193,   117,  -686,  1199,
     626,   885,   628,   886,  1498,    62,    63,    64,   167,   168,
     416,  1071,   888,  1504,   889,   892,   796,   775,   896,   897,
     905,   907,   910,  1509,  1336,   911,   110,   913,   916,  1161,
    1610,  1341,    88,    89,  1611,    90,   172,    92,   922,    37,
     925,   557,   923,   926,   934,    88,    89,   110,    90,   172,
      92,   942,    37,   737,   944,   772,   627,   598,   945,   946,
    1456,    50,   918,  1249,   972,  -668,   982,  1352,   992,  1730,
     994,   626,   417,   628,    50,   996,  1248,   998,   999,  1000,
    1001,  1003,   839,  1016,  1017,  1018,  1455,  1020,  1061,   360,
    1021,   123,  1053,  1067,  1274,   154,  1649,   890,   891,   117,
     118,  1138,  1138,   959,  1065,   120,   899,   663,   121,  1063,
    1068,  1069,    14,  1074,  1078,  1079,  1085,  1087,   663,  1249,
      88,    89,   772,    90,   172,    92,  1088,  1094,   110,  1092,
     110,  1407,   110,    88,    89,   116,    90,   172,    92,  1096,
    1113,  1122,  1125,  1128,  1306,   150,  1777,  1133,  1456,  -848,
     243,  1134,  1183,  1144,    55,  1145,  1148,  1149,  1151,  1168,
    1320,   918,    62,    63,    64,   167,   168,   416,  1163,   557,
    1165,  1169,   557,  1171,  1352,  1353,   637,  1180,  1181,   637,
    1354,   123,    62,    63,    64,   167,  1355,   416,  1356,   117,
    1185,  1048,    37,    62,    63,    64,    65,    66,   416,  1186,
    1189,   821,  1190,  1232,    72,   463,  1234,  1237,  1235,  1243,
    1246,  1262,   930,  1309,    50,  1263,  1270,  1271,  1469,    14,
     908,  1135,  1136,  1137,    37,  1357,  1358,  1275,  1359,   417,
    1277,  1690,  1283,   419,   110,  1279,  1281,  1284,  1295,  1287,
    1696,  1375,   465,  1286,  1293,  1376,    50,  1377,  1294,   417,
    1299,   955,  1305,  1364,  1307,  1384,  1308,  1360,  1325,  1310,
     417,  1315,  1364,  1327,  1319,  1391,   663,  1329,  1330,  1333,
     340,   658,   658,    88,    89,  1334,    90,   172,    92,   939,
    1335,  1339,  1353,  1337,  1731,  1340,  1344,  1354,  1346,    62,
      63,    64,   167,  1355,   416,  1356,  1347,   959,  1619,  1371,
    1382,   959,  1372,  1383,  1385,    88,    89,  1369,    90,   172,
      92,  1397,   110,  1404,  1387,  1389,  1369,  1393,  1398,  1400,
     966,  1399,   967,  1403,   110,  1406,  1405,    37,  1753,   202,
      40,  1409,  1357,  1358,  1411,  1359,  1412,  1413,   154,  1416,
    1439,  1417,   637,  1428,  1083,  1467,  1452,  1472,  1478,    50,
     986,  1479,  1487,   154,   123,  1482,   417,  1488,  1490,  1494,
    1093,  1507,   117,  1505,  1373,  1521,  1513,  1480,    37,  1522,
     663,  1617,  1616,  1107,  1623,  1629,  1512,  1630,   470,  1632,
    1461,   213,   213,  1642,   154,  1633,  1644,  1674,  1653,  1802,
      50,  1348,  1680,  1654,  1684,  1683,  1809,  1364,    37,  1689,
     202,    40,  1127,  1364,  1705,  1364,   735,  1066,    88,    89,
    1707,    90,   172,    92,  1713,  1514,  1721,  1364,  1709,   123,
      50,  1722,  1723,   598,  1077,  1728,  1729,   117,   123,    34,
      35,    36,  1733,  1736,  1737,   154,   117,  -342,  1739,   736,
    1740,   105,   203,  1742,  1744,   959,   834,   959,  1669,    88,
      89,  1369,    90,   172,    92,  1745,   154,  1369,  1751,  1369,
    1748,  1682,   637,  1631,  1179,  1754,  1756,  1182,  1502,  1755,
    1768,  1369,  1761,  1763,  1772,  1775,  1776,   735,  1801,    88,
      89,  1784,    90,   172,    92,  1806,  1624,  1706,  1708,  1786,
    1807,    76,    77,    78,    79,    80,  1790,   110,  1791,  1814,
    1820,  1364,   205,  1815,  1821,   323,  1823,   123,    84,    85,
     771,  1457,   105,   123,  1824,   117,  1236,  1771,   123,   711,
     716,   117,    94,   713,  1124,  1084,   117,  1785,  1312,  1042,
    1043,  1044,  1045,  1046,  1047,  1648,    99,   154,  1418,   154,
    1783,   154,   843,   986,  1167,  1640,   207,   207,  1048,  1520,
     223,  1662,  1646,  1502,  1667,  1369,   959,  1459,  1808,  1796,
    1679,  1257,   110,  1440,  1265,  1638,   899,   110,  1298,   213,
    1192,   110,  1297,  1140,  1152,   223,   600,  1352,  1104,  1677,
     664,  1430,  1746,  1006,  1239,   360,  1184,  1231,   557,  1759,
       0,   323,     0,  1285,     0,     0,  1288,     0,   324,     0,
       0,  1605,   450,   451,   452,   453,   454,   455,  1612,   456,
       0,     0,     0,     0,  1685,   323,     0,   323,     0,     0,
       0,   457,    14,   323,     0,  1726,     0,     0,     0,   419,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   154,     0,     0,   959,     0,     0,  1326,
     110,   110,   110,  1107,     0,     0,   110,     0,   123,     0,
       0,   110,     0,     0,     0,     0,   117,     0,     0,  1273,
       0,     0,     0,     0,     0,   210,   210,     0,     0,     0,
       0,     0,     0,   484,   485,  1353,     0,     0,     0,     0,
    1354,     0,    62,    63,    64,   167,  1355,   416,  1356,   123,
       0,   213,     0,     0,     0,   123,     0,   117,  1349,  1350,
     213,     0,   592,   117,  1304,     0,     0,   213,     0,     0,
       0,   154,     0,     0,   213,     0,     0,     0,     0,   598,
     986,     0,     0,   154,   207,  1357,  1358,     0,  1359,     0,
     123,     0,     0,     0,     0,     0,     0,     0,   117,  1760,
     486,   487,     0,     0,     0,     0,     0,     0,     0,   417,
       0,     0,   123,     0,     0,     0,     0,  1485,     0,     0,
     117,   557,     0,     0,     0,     0,    37,  1352,     0,     0,
       0,     0,     0,     0,   223,     0,   223,     0,     0,     0,
       0,  1816,   323,     0,     0,  1420,   959,  1421,    50,  1822,
       0,   110,     0,     0,     0,  1825,     0,   598,  1826,  1700,
     637,   123,     0,     0,     0,     0,  1605,  1605,   123,   117,
    1612,  1612,    14,     0,  1352,     0,   117,     0,     0,     0,
     637,     0,  1462,     0,   360,     0,     0,     0,     0,   637,
       0,   223,   110,     0,     0,     0,     0,     0,   110,     0,
     213,   171,     0,     0,    86,    87,     0,    88,    89,     0,
      90,   172,    92,   210,     0,     0,   207,     0,     0,    14,
       0,     0,     0,     0,     0,   207,     0,     0,     0,     0,
       0,     0,   207,   110,     0,  1353,     0,     0,    37,   207,
    1354,  1758,    62,    63,    64,   167,  1355,   416,  1356,     0,
     223,     0,     0,     0,     0,   110,   154,     0,     0,     0,
      50,  1773,     0,     0,     0,  1352,     0,     0,   338,   339,
       0,     0,     0,     0,     0,   223,     0,     0,   223,     0,
       0,     0,  1353,     0,  1352,  1357,  1358,  1354,  1359,    62,
      63,    64,   167,  1355,   416,  1356,     0,     0,     0,     0,
     208,   208,     0,     0,   110,     0,     0,     0,     0,   417,
      14,   110,     0,     0,     0,     0,   340,  1489,     0,    88,
      89,   154,    90,   172,    92,   223,   154,     0,     0,    14,
     154,     0,  1357,  1358,     0,  1359,     0,     0,     0,  1658,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,   210,     0,   417,     0,     0,     0,
       0,   210,     0,   213,  1491,   207,     0,     0,   210,     0,
       0,     0,     0,  1353,     0,     0,     0,     0,  1354,   625,
      62,    63,    64,   167,  1355,   416,  1356,     0,     0,     0,
       0,     0,  1353,     0,     0,     0,     0,  1354,     0,    62,
      63,    64,   167,  1355,   416,  1356,     0,     0,     0,   154,
     154,   154,     0,     0,     0,   154,     0,   223,   223,     0,
     154,   814,   213,  1357,  1358,     0,  1359,   499,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
      37,  1681,  1357,  1358,     0,  1359,     0,   417,     0,     0,
       0,     0,   814,     0,     0,  1497,     0,   209,   209,     0,
       0,   225,    50,   213,     0,   213,   417,     0,     0,     0,
     832,   833,     0,     0,  1641,     0,     0,     0,     0,     0,
     484,   485,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,   213,   210,     0,     0,     0,   223,   223,
       0,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   834,     0,
       0,    88,    89,  1741,    90,   172,    92,     0,   207,   499,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,     0,     0,     0,     0,     0,   486,   487,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     154,     0,     0,   265,     0,     0,   213,   213,     0,   499,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,     0,     0,     0,   207,     0,     0,
       0,   267,     0,     0,     0,     0,     0,     0,     0,   899,
       0,   154,     0,     0,   642,     0,     0,   154,     0,     0,
       0,     0,     0,    37,   899,     0,     0,     0,     0,     0,
     208,     0,   484,   485,     0,     0,     0,     0,   207,   208,
     207,     0,     0,     0,     0,    50,   208,     0,     0,     0,
       0,     0,   154,   208,     0,   209,     0,     0,     0,   486,
     487,     0,     0,     0,     0,     0,     0,   210,   207,   814,
       0,     0,     0,     0,   154,     0,     0,     0,     0,     0,
     544,   545,   223,   223,   814,   814,   814,   814,   814,     0,
       0,     0,   814,     0,     0,     0,   213,   213,   171,   486,
     487,    86,   317,   223,    88,    89,     0,    90,   172,    92,
       0,  1004,     0,     0,     0,     0,   766,     0,     0,     0,
       0,     0,   321,   154,     0,   207,   210,     0,     0,     0,
     154,     0,   322,     0,     0,     0,     0,     0,     0,   223,
       0,   207,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,   223,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,   210,     0,   210,
       0,   223,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,   223,     0,     0,   209,     0,     0,
       0,     0,   814,     0,     0,   223,   209,   210,     0,     0,
       0,     0,     0,   209,     0,     0,   428,   429,   430,     0,
     209,     0,     0,   223,     0,     0,     0,   223,     0,     0,
       0,   209,   213,     0,     0,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,   210,     0,     0,     0,     0,     0,
       0,   207,   207,     0,     0,   457,     0,   213,     0,     0,
     210,   210,     0,     0,     0,   223,     0,     0,   223,     0,
     223,     0,   213,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   625,   814,   225,   223,     0,     0,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,   814,   814,
     814,   814,   814,   814,   814,   814,   814,   814,     0,     0,
       0,     0,     0,     0,     0,     0,   209,     0,     0,     0,
       0,     0,   208,     0,   814,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   499,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,     0,   223,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   207,     0,     0,
     210,   210,   817,  1250,     0,     0,     0,     0,     0,     0,
       0,   208,     0,     0,   223,     0,     0,   223,     0,     0,
       0,     0,     0,     0,   484,   485,   326,     0,     0,    37,
       0,     0,     0,   817,     0,     0,   625,   223,     0,     0,
       0,     0,   207,     0,     0,     0,     0,     0,     0,     0,
       0,    50,   208,     0,   208,     0,     0,   207,   207,     0,
     814,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     223,   265,     0,  1442,   223,     0,     0,   814,     0,   814,
       0,     0,   208,     0,     0,     0,  1443,  1444,     0,     0,
       0,   486,   487,     0,     0,   814,     0,     0,     0,   267,
       0,     0,     0,     0,   171,     0,     0,    86,    87,   209,
      88,    89,     0,    90,  1446,    92,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,   210,     0,     0,   223,
     223,     0,   223,     0,     0,   207,     0,     0,     0,   208,
       0,     0,     0,    50,     0,     0,     0,     0,   851,     0,
       0,  -389,     0,     0,     0,   208,   208,     0,     0,    62,
      63,    64,   167,   168,   416,     0,   625,     0,   209,     0,
       0,   210,     0,     0,     0,     0,     0,     0,   544,   545,
       0,     0,     0,     0,     0,     0,   210,   210,     0,     0,
       0,     0,     0,     0,     0,     0,   171,     0,     0,    86,
     317,     0,    88,    89,     0,    90,   172,    92,     0,   209,
     265,   209,     0,     0,     0,     0,   223,     0,   223,     0,
     321,     0,     0,   814,   223,     0,   417,   814,     0,   814,
     322,     0,   814,     0,     0,   326,     0,   326,   267,   209,
     817,     0,   223,   223,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   223,     0,   817,   817,   817,   817,   817,
      37,     0,     0,   817,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1052,   208,   208,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
     550,     0,   326,     0,     0,   223,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   265,     0,     0,     0,
    1073,   814,   209,   209,     0,     0,     0,   544,   545,     0,
     223,   223,     0,     0,     0,     0,     0,  1073,   223,     0,
     223,     0,     0,     0,   267,   171,   209,     0,    86,   317,
       0,    88,    89,     0,    90,   172,    92,     0,     0,     0,
       0,     0,   223,   625,   223,     0,    37,     0,     0,   321,
     223,     0,     0,   817,     0,     0,  1114,     0,     0,   322,
       0,     0,     0,     0,     0,     0,   326,     0,    50,   326,
       0,    62,    63,    64,    65,    66,   416,     0,   225,     0,
       0,     0,    72,   463,     0,     0,   814,   814,     0,     0,
       0,   208,   814,     0,   223,     0,     0,     0,     0,     0,
     223,     0,   223,   544,   545,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   625,     0,     0,     0,   464,     0,
     465,   171,   209,   209,    86,   317,     0,    88,    89,     0,
      90,   172,    92,   466,  1328,   467,   208,     0,   417,     0,
       0,     0,     0,     0,     0,   321,     0,     0,     0,     0,
       0,   208,   208,     0,     0,   322,   817,     0,   209,     0,
       0,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,   817,
     817,   817,   817,   817,   817,   817,   817,   817,   817,  -979,
    -979,  -979,  -979,  -979,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,   223,   949,   950,   817,     0,     0,   326,   799,
       0,     0,   815,     0,     0,  1048,     0,     0,     0,   223,
       0,     0,     0,   951,     0,     0,     0,     0,     0,   208,
       0,   952,   953,   954,    37,     0,   223,     0,     0,     0,
       0,     0,   814,   815,   955,     0,     0,     0,   209,     0,
       0,     0,     0,   814,     0,     0,    50,     0,     0,   814,
       0,     0,     0,   814,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   223,   456,     0,     0,   209,   326,
     326,   956,     0,   209,     0,     0,     0,   457,   326,     0,
       0,     0,     0,     0,   957,     0,     0,     0,   209,   209,
       0,   817,     0,     0,     0,    88,    89,     0,    90,   172,
      92,     0,     0,     0,   814,     0,     0,     0,   817,     0,
     817,     0,     0,   958,   223,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   817,     0,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
     223,     0,     0,     0,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,     0,  1351,   431,   432,   209,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,     0,   211,   211,     0,
       0,   229,   432,   457,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,     0,   456,     0,
     815,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     457,     0,     0,   326,   326,   815,   815,   815,   815,   815,
       0,     0,     0,   815,   817,   209,     0,     0,   817,     0,
     817,     0,     0,   817,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,  1438,     0,     0,  1451,     0,     0,
       0,     0,     0,     0,   431,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,   326,     0,     0,     0,
       0,     0,     0,   457,     0,     0,   209,     0,   872,     0,
       0,     0,   326,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   817,     0,     0,   326,     0,     0,     0,     0,
       0,  1510,  1511,   815,     0,     0,     0,     0,     0,     0,
       0,  1451,     0,   428,   429,   430,     0,     0,     0,     0,
       0,     0,     0,     0,   326,     0,     0,     0,     0,     0,
       0,     0,     0,   431,   432,   211,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,     0,
     456,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   457,     0,     0,     0,     0,   817,   817,     0,
       0,     0,     0,   817,     0,  1656,   326,     0,     0,   326,
       0,   799,     0,  1451,     0,     0,     0,     0,   904,     0,
       0,     0,     0,     0,     0,     0,   815,     0,     0,     0,
       0,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,   815,
     815,   815,   815,   815,   815,   815,   815,   815,   815,  -979,
    -979,  -979,  -979,  -979,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,   815,     0,     0,     0,   265,
       0,     0,     0,     0,     0,   457,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,   211,     0,     0,     0,
       0,     0,     0,   211,     0,     0,   326,   267,   326,     0,
     211,     0,     0,     0,     0,     0,     0,   943,     0,     0,
       0,   229,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,   326,     0,     0,   326,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,   817,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   817,     0,     0,     0,     0,     0,
     817,     0,     0,     0,   817,     0,     0,     0,     0,     0,
       0,   815,     0,     0,     0,     0,   544,   545,     0,     0,
       0,   326,     0,     0,     0,   326,   229,     0,   815,     0,
     815,     0,     0,     0,   171,     0,     0,    86,   317,     0,
      88,    89,     0,    90,   172,    92,   815,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,     0,     0,     0,   817,   211,     0,   322,     0,
     428,   429,   430,     0,     0,  1770,     0,     0,     0,     0,
     326,   326,     0,     0,     0,     0,     0,     0,     0,     0,
     431,   432,  1438,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,     0,     0,
       0,     0,   818,     0,     0,     0,     0,   431,   432,   457,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   818,   456,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   457,   326,   819,   326,
       0,     0,     0,     0,   815,     0,     0,     0,   815,     0,
     815,     0,     0,   815,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,     0,     0,     0,     0,     0,   844,
       0,     0,     0,     0,   326,     0,  1022,  1023,  1024,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,     0,   211,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,     0,     0,   947,     0,     0,     0,     0,     0,
       0,     0,   815,     0,     0,  1048,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   326,
       0,     0,     0,     0,     0,     0,     0,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,     0,   326,     0,     0,     0,     0,
       0,   326,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   211,
     456,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   457,     0,     0,     0,     0,   815,   815,   983,
       0,     0,     0,   815,     0,     0,     0,     0,     0,   211,
     818,   326,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   818,   818,   818,   818,   818,
       0,    29,     0,   818,     0,     0,     0,  1197,     0,    34,
      35,    36,    37,     0,   202,    40,     0,     0,     0,     0,
       0,     0,   203,     0,     0,     0,   987,     0,     0,     0,
       0,     0,     0,     0,    50,     0,   211,     0,     0,     0,
       0,  1009,  1010,  1011,  1012,     0,     0,     0,     0,  1019,
       0,     0,   211,   211,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   984,    75,
       0,    76,    77,    78,    79,    80,   229,     0,     0,     0,
       0,     0,   205,   326,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,     0,
     326,     0,    94,   818,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,  1701,     0,     0,
       0,   206,     0,   815,     0,     0,   105,     0,   229,     0,
       0,     0,     0,     0,   815,     0,     0,     0,     0,     0,
     815,     0,     0,     0,   815,     0,     0,     0,     0,  1111,
    1022,  1023,  1024,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   326,     0,     0,     0,     0,
       0,  1025,   211,   211,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,  1047,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   815,   818,     0,   229,  1048,
       0,   818,   818,   818,   818,   818,   818,   818,   818,   818,
     818,   818,   818,   818,   818,   818,   818,   818,   818,   818,
     818,   818,   818,   818,   818,   818,   818,   818,   818,     0,
       0,   326,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   818,   326,  1200,  1203,  1204,
    1205,  1207,  1208,  1209,  1210,  1211,  1212,  1213,  1214,  1215,
    1216,  1217,  1218,  1219,  1220,  1221,  1222,  1223,  1224,  1225,
    1226,  1227,  1228,  1229,  1230,   428,   429,   430,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   211,     0,
       0,  1238,     0,     0,     0,   431,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,     0,     0,     0,     0,     0,   229,     0,
       0,     0,     0,   211,   457,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   211,   211,
       0,   818,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   818,     0,
     818,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   818,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1331,     0,  1332,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   211,    10,     0,     0,
       0,     0,  1342,     0,     0,     0,     0,     0,     0,     0,
       0,   388,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,   718,     0,     0,     0,     0,     0,     0,  1064,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,   818,   229,     0,    50,   818,     0,
     818,     0,     0,   818,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   167,   168,   169,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   170,    75,     0,    76,    77,    78,    79,    80,     0,
    1423,     0,     0,     0,  1425,    82,  1426,     0,     0,  1427,
     171,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     172,    92,     0,     0,     0,    94,   229,     0,    95,     0,
       0,     0,     0,     0,    96,   428,   429,   430,     0,    99,
     100,   101,   818,     0,   102,     0,     0,     0,     0,   105,
     106,     0,   107,   108,     0,   431,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,     0,     0,     0,     0,     0,  1506,     0,
       0,     0,     0,     0,   457,     0,     0,   255,   256,     0,
     257,   258,     0,     0,   259,   260,   261,   262,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   263,     0,   264,     0,     0,     0,   818,   818,     0,
       0,     0,     0,   818,     0,     0,     0,     0,     0,     0,
       0,     0,  1661,     0,     0,     0,     0,     0,     0,     0,
       0,   266,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   502,     0,     0,     0,   268,   269,   270,   271,   272,
     273,   274,     0,  1650,  1651,    37,     0,   202,    40,  1655,
       0,     0,     0,     0,     0,     0,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,    50,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,     0,     0,     0,   706,   310,   311,   312,     0,     0,
       0,   313,   554,   555,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     556,     0,     0,     0,     0,     0,    88,    89,     0,    90,
     172,    92,   318,     0,   319,     0,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   818,     0,     0,     0,   707,     0,   105,
       0,     0,     0,     0,   818,     0,     0,     0,     0,     0,
     818,     0,     0,     0,   818,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,  1743,     0,  1710,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
    1720,     0,     0,     0,     0,     0,  1725,     0,     0,     0,
    1727,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,   818,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,  1762,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,     0,    88,    89,     0,    90,    91,    92,    93,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,    97,     0,    98,     0,    99,   100,   101,     0,     0,
     102,     0,   103,   104,  1081,   105,   106,     0,   107,   108,
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
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,     0,    88,
      89,     0,    90,    91,    92,    93,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,    97,     0,
      98,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,  1251,   105,   106,     0,   107,   108,     5,     6,     7,
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
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,    74,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,    81,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,     0,    88,    89,     0,    90,
      91,    92,    93,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,    97,     0,    98,     0,    99,
     100,   101,     0,     0,   102,     0,   103,   104,     0,   105,
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
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,    93,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   102,     0,   103,   104,   644,   105,   106,     0,   107,
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
       0,     0,     0,     0,   171,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   172,    92,    93,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,  1051,   105,   106,     0,   107,   108,     5,     6,
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
       0,   171,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   172,    92,    93,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   102,     0,   103,   104,  1095,
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
      81,     0,     0,    82,     0,     0,     0,     0,   171,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   172,    92,
      93,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,     0,   103,   104,  1162,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,  1164,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,    81,     0,     0,
      82,     0,     0,     0,     0,   171,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   172,    92,    93,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   102,
       0,   103,   104,     0,   105,   106,     0,   107,   108,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1317,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,    81,     0,     0,    82,     0,     0,
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,    93,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   102,     0,   103,   104,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
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
       0,    81,     0,     0,    82,     0,     0,     0,     0,   171,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   172,
      92,    93,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   102,     0,   103,   104,  1429,   105,   106,
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
       0,    82,     0,     0,     0,     0,   171,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   172,    92,    93,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     102,     0,   103,   104,  1652,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,  1697,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,   171,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   172,    92,    93,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,     0,   105,   106,     0,   107,   108,     5,     6,     7,
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
     171,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     172,    92,    93,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   102,     0,   103,   104,  1732,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,  1735,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,    93,
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
       0,     0,     0,     0,   171,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   172,    92,    93,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,  1752,   105,   106,     0,   107,   108,     5,     6,
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
       0,   171,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   172,    92,    93,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   102,     0,   103,   104,  1769,
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
      81,     0,     0,    82,     0,     0,     0,     0,   171,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   172,    92,
      93,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   102,     0,   103,   104,  1810,   105,   106,     0,
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
      82,     0,     0,     0,     0,   171,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   172,    92,    93,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   102,
       0,   103,   104,  1817,   105,   106,     0,   107,   108,     5,
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
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,    93,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   102,     0,   103,   104,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   167,   168,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,    81,     0,     0,    82,     0,     0,     0,     0,   171,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   172,
      92,     0,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   102,     0,   103,   104,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   784,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,   167,   168,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,     0,   171,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   172,    92,     0,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     102,     0,   103,   104,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   989,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   167,
     168,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,     0,   171,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   172,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   102,     0,   103,
     104,     0,   105,   106,     0,   107,   108,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1501,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,   167,   168,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,    81,     0,     0,    82,     0,     0,     0,     0,
     171,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     172,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,     0,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   102,     0,   103,   104,     0,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1645,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   167,   168,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,     0,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   102,     0,   103,   104,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
     167,   168,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,    81,     0,     0,    82,
       0,     0,     0,     0,   171,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   172,    92,     0,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,     0,
     103,   104,     0,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   167,   168,   169,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   170,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,   171,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   172,    92,     0,     0,     0,    94,     0,     0,    95,
       0,     0,     0,     0,     0,    96,     0,     0,     0,     0,
      99,   100,   101,  1023,  1024,   173,     0,   331,     0,     0,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
       0,     0,     0,  1025,     0,    10,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,     0,     0,
     659,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1048,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   167,   168,   169,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   170,
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,   171,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   172,    92,
       0,   660,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   173,     0,     0,     0,     0,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   167,   168,   169,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   170,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,   171,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   172,    92,     0,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,  1024,   173,
       0,     0,   779,     0,   105,   106,     0,   107,   108,     5,
       6,     7,     8,     9,     0,     0,     0,  1025,     0,    10,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,
    1046,  1047,     0,     0,  1108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1048,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   167,   168,
     169,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,     0,  1109,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   173,     0,     0,     0,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     388,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   167,   168,   169,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     170,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,   171,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   172,
      92,     0,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   102,   428,   429,   430,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   431,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   457,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   184,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   167,   168,   169,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   170,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,   171,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   172,    92,     0,     0,
       0,    94,     0,     0,    95,     0,     0,     0,  1121,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     173,     0,     0,     0,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,  1025,     0,
      10,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,
    1045,  1046,  1047,     0,     0,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1048,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   167,
     168,   169,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   170,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,   171,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   172,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,     0,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   173,   428,   429,
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
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   167,   168,   169,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   170,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
     171,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     172,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,  1132,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   173,     0,   250,   429,   430,   105,
     106,     0,   107,   108,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,   457,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   167,   168,   169,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   170,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,     0,
       0,     0,    94,     0,     0,    95,     0,     0,     0,     0,
       0,    96,     0,     0,     0,     0,    99,   100,   101,     0,
       0,   173,     0,   253,     0,     0,   105,   106,     0,   107,
     108,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   388,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
     167,   168,   169,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   170,    75,     0,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,     0,   171,    84,    85,    86,    87,     0,
      88,    89,     0,    90,   172,    92,     0,     0,     0,    94,
       0,     0,    95,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,    99,   100,   101,     0,     0,   102,   428,
     429,   430,     0,   105,   106,     0,   107,   108,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   431,
     432,     0,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,     0,   456,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   457,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   167,   168,   169,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   170,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,   171,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   172,    92,     0,     0,     0,    94,     0,     0,    95,
       0,     0,     0,  1156,     0,    96,     0,     0,     0,     0,
      99,   100,   101,     0,     0,   173,   525,     0,     0,     0,
     105,   106,     0,   107,   108,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   673,   456,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   167,   168,   169,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   170,
      75,     0,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,   171,    84,
      85,    86,    87,     0,    88,    89,     0,    90,   172,    92,
       0,     0,     0,    94,     0,     0,    95,     0,     0,     0,
       0,     0,    96,     0,     0,     0,     0,    99,   100,   101,
       0,     0,   173,     0,     0,     0,     0,   105,   106,     0,
     107,   108,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,     0,     0,   718,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1048,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   167,   168,   169,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   170,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,   171,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   172,    92,     0,     0,     0,
      94,     0,     0,    95,     0,     0,     0,     0,     0,    96,
       0,     0,     0,     0,    99,   100,   101,     0,     0,   173,
       0,     0,     0,     0,   105,   106,     0,   107,   108,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,     0,   456,     0,   759,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   457,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   167,   168,
     169,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   170,    75,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,     0,     0,     0,    94,     0,     0,
      95,     0,     0,     0,     0,     0,    96,     0,     0,     0,
       0,    99,   100,   101,     0,     0,   173,     0,     0,     0,
       0,   105,   106,     0,   107,   108,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,     0,     0,
       0,   761,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1048,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   167,   168,   169,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     170,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,   171,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   172,
      92,     0,     0,     0,    94,     0,     0,    95,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,    99,   100,
     101,     0,     0,   173,     0,     0,     0,     0,   105,   106,
       0,   107,   108,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,     0,     0,     0,     0,  1153,     0,
       0,     0,     0,     0,     0,     0,     0,  1048,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,    62,
      63,    64,   167,   168,   169,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   170,    75,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,   171,    84,    85,    86,
      87,     0,    88,    89,     0,    90,   172,    92,     0,     0,
       0,    94,     0,     0,    95,     0,     0,     0,     0,     0,
      96,     0,     0,     0,     0,    99,   100,   101,     0,     0,
     173,   428,   429,   430,     0,   105,   106,     0,   107,   108,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   431,   432,     0,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,     0,   456,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     457,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   167,
     168,   169,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   170,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,   171,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   172,    92,     0,     0,     0,    94,     0,
       0,    95,     0,     0,     0,  1474,     0,    96,     0,     0,
       0,     0,    99,   100,   101,     0,     0,   173,   428,   429,
     430,     0,   105,   106,     0,   107,   108,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   431,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   457,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,   606,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,    62,    63,    64,   167,   168,   169,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   170,    75,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
     171,    84,    85,    86,    87,     0,    88,    89,     0,    90,
     172,    92,     0,     0,     0,    94,     0,     0,    95,     0,
       0,     0,  1475,     0,    96,     0,     0,     0,     0,    99,
     100,   101,     0,     0,   173,     0,     0,     0,     0,   105,
     106,     0,   107,   108,   255,   256,     0,   257,   258,     0,
       0,   259,   260,   261,   262,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,     0,
     264,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,     0,     0,   266,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,   268,   269,   270,   271,   272,   273,   274,     0,
       0,     0,    37,     0,   202,    40,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    50,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,   309,   310,   311,   312,     0,     0,     0,   313,   554,
     555,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   255,   256,     0,   257,   258,     0,   556,   259,   260,
     261,   262,     0,    88,    89,     0,    90,   172,    92,   318,
       0,   319,     0,     0,   320,   263,     0,   264,     0,   265,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   707,     0,   105,     0,     0,     0,
       0,     0,     0,     0,     0,   266,     0,   267,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   268,
     269,   270,   271,   272,   273,   274,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,    50,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,     0,     0,     0,     0,   310,
     311,   312,     0,     0,     0,   313,   314,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   316,     0,     0,    86,   317,     0,
      88,    89,     0,    90,   172,    92,   318,     0,   319,     0,
       0,   320,   255,   256,     0,   257,   258,     0,   321,   259,
     260,   261,   262,     0,     0,     0,     0,     0,   322,     0,
       0,     0,  1625,     0,     0,     0,   263,     0,   264,     0,
     265,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,   266,     0,   267,     0,
       0,     0,     0,   457,     0,     0,     0,     0,     0,     0,
     268,   269,   270,   271,   272,   273,   274,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,    50,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,     0,     0,     0,     0,
     310,   311,   312,     0,     0,     0,   313,   314,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   316,     0,     0,    86,   317,
       0,    88,    89,     0,    90,   172,    92,   318,     0,   319,
       0,     0,   320,   255,   256,     0,   257,   258,     0,   321,
     259,   260,   261,   262,     0,     0,     0,     0,     0,   322,
       0,     0,     0,  1693,     0,     0,     0,   263,     0,   264,
       0,   265,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,
    1047,     0,     0,     0,     0,     0,     0,   266,     0,   267,
       0,     0,     0,     0,  1048,     0,     0,     0,     0,     0,
       0,   268,   269,   270,   271,   272,   273,   274,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,    50,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,     0,     0,     0,
     309,   310,   311,   312,     0,     0,     0,   313,   314,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   316,     0,     0,    86,
     317,     0,    88,    89,     0,    90,   172,    92,   318,     0,
     319,     0,     0,   320,   255,   256,     0,   257,   258,     0,
     321,   259,   260,   261,   262,     0,     0,     0,     0,     0,
     322,     0,     0,     0,     0,     0,     0,     0,   263,     0,
     264,     0,   265,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,
    1047,     0,     0,     0,     0,     0,     0,     0,   266,     0,
     267,     0,     0,     0,  1048,     0,     0,     0,     0,     0,
       0,     0,   268,   269,   270,   271,   272,   273,   274,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    50,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,     0,   310,   311,   312,     0,     0,     0,   313,   314,
     315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   316,     0,     0,
      86,   317,     0,    88,    89,     0,    90,   172,    92,   318,
       0,   319,     0,     0,   320,     0,   255,   256,     0,   257,
     258,   321,  1433,   259,   260,   261,   262,     0,     0,     0,
       0,   322,     0,     0,     0,     0,     0,     0,     0,     0,
     263,     0,   264,     0,   265,  -979,  -979,  -979,  -979,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,     0,     0,     0,     0,     0,
     266,     0,   267,     0,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,    50,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
       0,     0,     0,     0,   310,   311,   312,     0,     0,     0,
     313,   314,   315,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   316,
       0,     0,    86,   317,     0,    88,    89,     0,    90,   172,
      92,   318,     0,   319,     0,     0,   320,  1525,  1526,  1527,
    1528,  1529,     0,   321,  1530,  1531,  1532,  1533,     0,     0,
       0,     0,     0,   322,     0,     0,     0,     0,     0,     0,
       0,  1534,  1535,  1536,  1031,  1032,  1033,  1034,  1035,  1036,
    1037,  1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,
    1047,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1537,     0,     0,  1048,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1538,  1539,  1540,  1541,  1542,
    1543,  1544,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1545,  1546,  1547,  1548,
    1549,  1550,  1551,  1552,  1553,  1554,  1555,    50,  1556,  1557,
    1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,  1566,  1567,
    1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,  1576,  1577,
    1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,     0,     0,
       0,  1586,  1587,     0,  1588,  1589,  1590,  1591,  1592,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1593,  1594,  1595,     0,     0,     0,    88,    89,     0,    90,
     172,    92,  1596,     0,  1597,  1598,     0,  1599,   428,   429,
     430,     0,     0,     0,  1600,  1601,     0,  1602,     0,  1603,
    1604,     0,     0,     0,     0,     0,     0,     0,   431,   432,
    1321,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   428,   429,   430,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,     0,     0,     0,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,   431,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,   428,   429,
     430,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   431,   432,
    1322,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   428,   429,   430,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,     0,     0,     0,     0,   431,   432,   458,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,   428,   429,   430,     0,     0,     0,
       0,     0,     0,     0,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,   431,   432,   541,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
       0,   456,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     0,   428,   429,
     430,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   431,   432,
     543,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   428,   429,   430,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,     0,     0,     0,     0,   431,   432,   561,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,   255,   256,     0,   257,   258,     0,
       0,   259,   260,   261,   262,   457,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   565,     0,   263,     0,
     264,  -979,  -979,  -979,  -979,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   266,     0,
       0,  1048,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   268,   269,   270,   271,   272,   273,   274,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,   751,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,    50,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,     0,     0,
       0,   309,   310,   311,   312,     0,     0,   776,   313,   554,
     555,     0,     0,     0,     0,     0,   255,   256,     0,   257,
     258,     0,     0,   259,   260,   261,   262,   556,     0,     0,
       0,     0,     0,    88,    89,     0,    90,   172,    92,   318,
     263,   319,   264,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     266,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,    50,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
       0,     0,     0,  1198,   310,   311,   312,     0,     0,     0,
     313,   554,   555,     0,     0,     0,     0,     0,   255,   256,
       0,   257,   258,     0,     0,   259,   260,   261,   262,   556,
       0,     0,     0,     0,     0,    88,    89,     0,    90,   172,
      92,   318,   263,   319,   264,     0,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   268,   269,   270,   271,
     272,   273,   274,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,    50,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,     0,     0,     0,     0,   310,   311,   312,  1206,
       0,     0,   313,   554,   555,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   805,   806,     0,
       0,   556,     0,   807,     0,   808,     0,    88,    89,     0,
      90,   172,    92,   318,     0,   319,     0,   809,   320,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,     0,
       0,     0,     0,   428,   429,   430,     0,     0,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,   431,   432,     0,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,     0,
     456,     0,     0,     0,     0,   810,     0,    76,    77,    78,
      79,    80,   457,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,   171,    84,    85,    86,   811,     0,    88,
      89,     0,    90,   172,    92,   805,   806,     0,    94,     0,
       0,   807,     0,   808,     0,     0,     0,   812,     0,     0,
       0,     0,    99,     0,     0,   809,     0,   813,     0,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,     0,
       0,   428,   429,   430,     0,     0,   203,     0,     0,   511,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,   431,   432,     0,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,     0,   456,     0,
       0,     0,     0,   810,     0,    76,    77,    78,    79,    80,
     457,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,   171,    84,    85,    86,   811,     0,    88,    89,     0,
      90,   172,    92,     0,     0,     0,    94,     0,     0,     0,
       0,     0,     0,     0,     0,   812,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   813,     0,     0,   428,   429,
     430,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   924,   431,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,     0,   456,   428,   429,   430,     0,
       0,     0,     0,     0,     0,     0,     0,   457,     0,     0,
       0,     0,     0,     0,     0,     0,   431,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,     0,   456,     0,     0,     0,     0,     0,     0,
       0,  1022,  1023,  1024,     0,   457,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,     0,   968,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,  1047,     0,     0,  1022,
    1023,  1024,     0,     0,     0,     0,     0,     0,     0,     0,
    1048,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1025,     0,  1280,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,  1046,  1047,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1022,  1023,  1024,     0,  1048,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1025,     0,  1188,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
       0,     0,  1022,  1023,  1024,     0,     0,     0,     0,     0,
       0,     0,     0,  1048,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1025,     0,  1338,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,     0,    29,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,  1048,   202,    40,     0,     0,     0,     0,     0,     0,
     203,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1422,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,  1508,     0,
     205,     0,     0,     0,     0,   171,    84,    85,    86,    87,
       0,    88,    89,     0,    90,   172,    92,    29,     0,     0,
      94,     0,     0,     0,     0,    34,    35,    36,    37,     0,
     202,    40,     0,     0,    99,     0,     0,     0,   203,   206,
       0,     0,   577,     0,   105,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   597,    75,     0,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,   171,    84,    85,    86,    87,     0,    88,
      89,     0,    90,   172,    92,    29,     0,   938,    94,     0,
       0,     0,     0,    34,    35,    36,    37,     0,   202,    40,
       0,     0,    99,     0,     0,     0,   203,   206,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   204,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,   171,    84,    85,    86,    87,     0,    88,    89,     0,
      90,   172,    92,    29,     0,     0,    94,     0,     0,     0,
       0,    34,    35,    36,    37,     0,   202,    40,     0,     0,
      99,     0,     0,     0,   203,   206,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   204,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1076,    75,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,   205,     0,     0,     0,     0,   171,
      84,    85,    86,    87,     0,    88,    89,     0,    90,   172,
      92,    29,     0,     0,    94,     0,     0,     0,     0,    34,
      35,    36,    37,     0,   202,    40,     0,     0,    99,     0,
       0,     0,   203,   206,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,     0,
       0,     0,    94,     0,    34,    35,    36,    37,     0,   202,
      40,     0,     0,     0,     0,     0,    99,   203,     0,     0,
       0,   206,     0,     0,     0,     0,   105,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,     0,     0,     0,    94,     0,    34,
      35,    36,    37,     0,   202,    40,     0,     0,     0,     0,
       0,    99,   620,     0,     0,     0,   222,     0,     0,     0,
       0,   105,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,   171,    84,    85,
      86,    87,     0,    88,    89,     0,    90,   172,    92,     0,
       0,     0,    94,     0,    34,    35,    36,    37,     0,   202,
      40,     0,     0,     0,     0,     0,    99,   203,     0,     0,
       0,   621,     0,     0,     0,     0,   105,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,   171,    84,    85,    86,    87,     0,    88,    89,
       0,    90,   172,    92,     0,     0,     0,    94,     0,   428,
     429,   430,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,     0,     0,     0,     0,   222,   788,     0,   431,
     432,   105,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,     0,   456,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   457,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   428,   429,   430,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   789,   431,   432,   921,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
     428,   429,   430,     0,     0,     0,     0,     0,     0,     0,
       0,   457,     0,     0,     0,     0,     0,     0,     0,     0,
     431,   432,     0,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,     0,   456,  1022,  1023,
    1024,     0,     0,     0,     0,     0,     0,     0,     0,   457,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1025,
    1343,     0,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,   430,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1048,     0,     0,
       0,     0,   431,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,     0,   456,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   457
};

static const yytype_int16 yycheck[] =
{
       5,     6,   123,     8,     9,    10,    11,    12,    13,   150,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   175,     4,    29,    30,    56,   636,   102,   897,
      33,     4,     4,    31,  1098,   633,    31,     4,    31,    44,
       4,   381,   496,    46,   515,    56,    93,    52,    51,    54,
      97,    98,    57,   662,    59,   176,   381,   218,   381,   102,
     632,    31,   886,   223,    44,    57,   155,     4,   753,   492,
     493,   456,  1085,   150,   613,   122,    81,     4,   571,   572,
     794,   787,   102,   982,   488,   102,   488,   917,   234,   715,
       9,    60,     9,   763,     9,   338,   339,   102,   521,    57,
       9,    81,     9,   933,     9,    14,     9,    14,     9,    14,
       9,     9,     9,     9,    83,     9,    49,    86,     9,   523,
       9,   523,     9,     9,    49,     9,     9,    49,     9,     9,
     173,     9,   527,    90,    70,     9,     9,    32,     9,   969,
       9,    70,  1639,    83,    83,    84,   235,    36,    38,    83,
     123,   134,   135,   173,   190,   115,   173,     0,    70,   161,
      32,    38,   105,   206,   161,   155,     4,   562,   173,   150,
      49,    38,    14,   122,    90,   180,   190,   174,   123,   222,
     520,   130,    70,    38,    49,   130,   206,   134,   135,   206,
      32,   193,     8,    83,  1018,   152,   193,  1694,    38,    38,
     190,   206,   222,   176,    50,    51,    83,  1094,    70,    51,
      70,   194,    70,   173,    70,    53,    83,   222,    56,   155,
     366,   155,   134,   135,   190,    70,    70,   836,    83,   169,
      83,   236,   193,    70,   239,    73,   152,    70,    70,   634,
      70,   246,   247,    83,    83,    70,   134,   135,    54,    70,
      70,    70,    70,    91,   190,    93,   195,   193,   191,    97,
      98,   190,   191,   188,  1277,   418,   936,   176,    70,   191,
    1169,  1284,   191,  1286,    54,   192,   193,   192,   992,   169,
     994,   193,    70,   192,   122,   192,    70,   192,   190,   192,
      83,   192,  1305,   192,   192,   192,   192,   192,   192,  1129,
     330,   192,   191,   240,    83,   192,   192,   244,   192,   192,
     191,   191,   191,   191,   169,   358,   914,   191,   191,   330,
     191,   193,   191,   188,  1148,   155,   191,    83,   190,   169,
     169,   177,   190,   193,   190,   193,   875,   193,   358,   832,
     833,   358,   195,   190,   505,   498,   190,   160,   193,   470,
     102,   189,   357,   358,   160,   102,   193,   190,   190,   364,
     193,   193,   190,   193,   369,   412,   155,   155,   193,   338,
     339,   340,   193,   193,   193,   193,   169,   357,   155,   409,
     160,    50,    51,   388,   779,  1398,   190,   176,  1275,   784,
     169,   396,   190,    83,    84,   175,   102,   471,   409,   176,
      83,   176,   240,   408,   373,   193,   244,   159,   862,   193,
     248,    32,   159,   169,   395,   190,   193,   190,   465,   466,
     467,   468,   395,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   462,   457,   159,   459,   460,   461,  1083,   456,  1165,
     387,   456,   623,   456,  1149,   190,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   471,
      83,   106,   107,   102,   489,   490,   456,   492,   493,   494,
     495,   190,   330,   639,   499,   641,   469,   502,   509,   903,
    1387,   903,  1389,   193,   187,   195,   511,   930,   513,   650,
     193,  1335,   190,   471,   159,  1113,   521,    83,  1116,   462,
      83,   176,   520,   190,   529,  1097,   531,    90,   176,   106,
     107,   105,   106,   107,    56,   190,    83,   529,    75,    76,
     159,   155,   190,    90,   705,   488,   886,   192,   193,   709,
     153,   154,  1353,   938,    53,    54,    55,   395,   134,   135,
     190,   886,   176,   886,   717,   159,   509,   405,  1061,   194,
      69,   409,   577,   650,   412,     4,   190,   520,   621,   193,
     523,   741,  1406,   826,    38,   828,   152,   153,   154,    57,
     153,   154,    70,    31,   989,   176,  1483,   534,    27,    28,
    1487,    69,   571,   572,    75,    76,   153,   154,  1293,   190,
    1295,   757,   758,    83,   190,    31,   621,   194,   764,   765,
      90,   668,    31,    81,   462,   463,   464,   465,   466,   467,
     468,   192,   176,   192,    50,   796,   199,    53,  1439,   192,
     801,   102,   103,    81,   192,   103,   190,   119,   120,   121,
     488,   198,   132,   133,   814,   660,   134,   135,   192,  1285,
    1131,   821,  1463,    83,  1465,   103,   192,   193,   673,   650,
      90,   509,    81,   102,   192,   764,   192,   155,  1018,   192,
     193,  1279,    91,   153,   154,   523,   192,   125,    70,    81,
    1668,  1669,  1264,  1018,   103,  1018,   534,   858,    70,  1122,
     138,   139,   707,   193,   162,   163,   633,   165,   166,   167,
    1133,   103,   190,    81,  1168,   553,   155,  1781,   156,  1404,
      70,   159,   160,   190,   162,   163,   190,   165,   166,   167,
     155,   736,  1796,   153,   154,   103,   159,   575,   576,   338,
     339,    31,  1368,   265,   173,   267,   190,   156,   176,   460,
     159,  1638,   192,   162,   163,  1642,   165,   166,   167,    81,
      50,    48,    83,    53,   602,   603,   771,    81,   160,    90,
     162,   163,    69,   165,   166,   167,   155,   206,   489,   190,
     783,   103,   787,   494,  1664,  1665,   215,   190,   156,   103,
     197,   159,  1246,   222,   162,   163,   777,   165,   166,   167,
     322,  1399,   963,    83,   777,   105,   106,   107,  1148,  1494,
      90,   240,   803,   804,  1440,   244,   119,   120,   121,   122,
     123,   124,     9,  1148,   155,  1148,   194,    83,   155,   190,
     668,   152,   153,   154,    90,     8,   192,   159,  1233,   190,
     162,   163,   823,   165,   166,   167,   155,  1008,   162,   163,
     823,   165,   166,   167,  1015,   792,    14,   826,  1281,   828,
     106,   107,   108,   832,   833,   834,   871,   155,   192,   192,
       9,   193,   152,   153,   154,   193,   190,   192,   889,  1747,
     885,   130,   130,   186,   406,    14,   191,   409,  1775,   192,
     937,   176,    14,   874,   102,   191,   191,   153,   154,  1767,
     191,   874,   874,  1790,   196,   191,   911,   874,  1776,   190,
     874,   111,  1307,   190,   895,   753,   921,   755,   347,   924,
     190,   926,   895,     9,   152,   930,   191,   356,  1323,   358,
     191,   191,   191,    94,   363,     9,    14,   874,   192,   777,
     938,   370,   176,   938,  1104,   938,   889,   874,    53,    54,
      55,   190,    57,   791,   792,     9,   193,   190,   901,  1644,
     903,    81,  1385,   968,    69,   192,   395,   367,   938,   193,
     897,   371,   975,   192,    81,    83,    83,   191,    85,   191,
     191,   132,  1143,   103,   191,   823,   192,   914,   190,    70,
      32,   133,   830,   831,   175,  1335,   103,   978,   398,   980,
     400,   401,   402,   403,  1078,   978,  1017,   980,   155,  1020,
    1335,   136,  1335,     9,  1409,   119,   120,   121,   122,   123,
     124,   859,   191,  1418,   155,    14,   548,   970,   188,     9,
       9,   177,   191,  1428,  1195,     9,   874,    14,   132,   976,
     160,  1202,   162,   163,   164,   165,   166,   167,   197,    81,
     194,   889,   197,     9,    14,   162,   163,   895,   165,   166,
     167,   197,    81,   901,   191,   903,  1406,   496,   191,   197,
     190,   103,   190,  1078,   191,   155,   102,     4,   192,  1688,
     192,  1406,   186,  1406,   103,     9,  1078,    91,   136,   155,
       9,   191,  1061,   190,   155,   190,   128,   155,     9,   937,
     193,  1082,   193,   192,  1109,   534,  1501,   629,   630,  1082,
    1082,   949,   950,   951,    14,  1082,   638,  1122,  1082,   194,
     177,     9,    49,   193,    14,   197,   193,    14,  1133,  1134,
     162,   163,   970,   165,   166,   167,   191,   188,   976,   192,
     978,  1302,   980,   162,   163,  1082,   165,   166,   167,    32,
     190,   190,    32,    14,  1157,  1082,  1765,   190,   190,   190,
    1165,    14,  1000,    52,   111,   190,   190,     9,   191,   190,
    1175,   190,   119,   120,   121,   122,   123,   124,   192,  1017,
     192,   136,  1020,    14,     4,   112,  1113,   177,   136,  1116,
     117,  1172,   119,   120,   121,   122,   123,   124,   125,  1172,
       9,    69,    81,   119,   120,   121,   122,   123,   124,   191,
     197,  1049,     9,    83,   130,   131,   194,   192,   194,     9,
     190,   136,   190,  1160,   103,   192,    14,    83,  1369,    49,
     659,    78,    79,    80,    81,   162,   163,   191,   165,   186,
     193,  1636,   191,  1364,  1082,   190,   190,   193,     9,   192,
    1645,  1256,   168,   193,   136,  1260,   103,  1262,   197,   186,
     152,    91,   193,  1244,    32,  1270,    77,   194,   177,   192,
     186,   191,  1253,   136,   192,  1280,  1281,    32,   191,   191,
     159,   803,   804,   162,   163,   136,   165,   166,   167,   718,
       9,   194,   112,   191,  1689,     9,   191,   117,   192,   119,
     120,   121,   122,   123,   124,   125,   192,  1145,  1461,   194,
      14,  1149,   193,    83,   190,   162,   163,  1244,   165,   166,
     167,   192,  1160,   136,   191,   191,  1253,   191,   193,   191,
     759,   190,   761,   191,  1172,     9,   191,    81,  1733,    83,
      84,    32,   162,   163,   192,   165,   191,   191,   777,   192,
     112,   192,  1279,   193,   876,   192,   164,   160,    14,   103,
     789,    83,   191,   792,  1345,   117,   186,   191,   193,   136,
     892,   136,  1345,   191,   194,   176,    14,  1382,    81,   193,
    1385,    83,   192,   905,    14,    14,  1433,    83,  1369,   191,
    1363,    27,    28,   191,   823,   190,   136,    14,   192,  1794,
     103,  1239,    14,   192,    14,   192,  1801,  1388,    81,   193,
      83,    84,   934,  1394,     9,  1396,   160,   846,   162,   163,
       9,   165,   166,   167,    59,  1436,    83,  1408,   194,  1410,
     103,   176,   190,   862,   863,    83,     9,  1410,  1419,    78,
      79,    80,   193,   192,   115,   874,  1419,   102,   155,   193,
     102,   195,    91,   177,   167,  1293,   159,  1295,    36,   162,
     163,  1388,   165,   166,   167,    14,   895,  1394,   191,  1396,
     190,  1624,  1399,  1478,   996,   192,   173,   999,  1415,   190,
      83,  1408,   177,   177,   170,   191,     9,   160,   193,   162,
     163,    83,   165,   166,   167,    14,  1469,  1658,  1659,   192,
      83,   140,   141,   142,   143,   144,   191,  1345,   191,    14,
      14,  1492,   151,    83,    83,  1353,    14,  1498,   157,   158,
     193,  1359,   195,  1504,    83,  1498,  1061,  1756,  1509,   463,
     468,  1504,   171,   465,   931,   877,  1509,  1772,  1166,    50,
      51,    52,    53,    54,    55,  1500,   185,   976,  1320,   978,
    1767,   980,   579,   982,   983,  1490,    27,    28,    69,  1441,
      31,  1523,  1499,  1500,  1608,  1492,  1404,  1359,  1800,  1788,
    1620,  1086,  1410,  1355,  1096,  1486,  1098,  1415,  1145,   215,
    1016,  1419,  1144,   950,   965,    56,   364,     4,   901,  1619,
     409,  1348,  1722,   803,  1069,  1433,  1001,  1049,  1436,  1740,
      -1,  1439,    -1,  1125,    -1,    -1,  1128,    -1,  1619,    -1,
      -1,  1449,    50,    51,    52,    53,    54,    55,  1456,    57,
      -1,    -1,    -1,    -1,  1629,  1463,    -1,  1465,    -1,    -1,
      -1,    69,    49,  1471,    -1,  1682,    -1,    -1,    -1,  1760,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,  1082,    -1,    -1,  1494,    -1,    -1,  1181,
    1498,  1499,  1500,  1185,    -1,    -1,  1504,    -1,  1649,    -1,
      -1,  1509,    -1,    -1,    -1,    -1,  1649,    -1,    -1,  1108,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,  1690,
      -1,   347,    -1,    -1,    -1,  1696,    -1,  1690,  1240,  1241,
     356,    -1,   358,  1696,  1153,    -1,    -1,   363,    -1,    -1,
      -1,  1160,    -1,    -1,   370,    -1,    -1,    -1,    -1,  1168,
    1169,    -1,    -1,  1172,   215,   162,   163,    -1,   165,    -1,
    1731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1731,  1740,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,  1753,    -1,    -1,    -1,    -1,   194,    -1,    -1,
    1753,  1619,    -1,    -1,    -1,    -1,    81,     4,    -1,    -1,
      -1,    -1,    -1,    -1,   265,    -1,   267,    -1,    -1,    -1,
      -1,  1806,  1640,    -1,    -1,  1327,  1644,  1329,   103,  1814,
      -1,  1649,    -1,    -1,    -1,  1820,    -1,  1246,  1823,  1657,
    1747,  1802,    -1,    -1,    -1,    -1,  1664,  1665,  1809,  1802,
    1668,  1669,    49,    -1,     4,    -1,  1809,    -1,    -1,    -1,
    1767,    -1,  1364,    -1,  1682,    -1,    -1,    -1,    -1,  1776,
      -1,   322,  1690,    -1,    -1,    -1,    -1,    -1,  1696,    -1,
     496,   156,    -1,    -1,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,   215,    -1,    -1,   347,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,   356,    -1,    -1,    -1,    -1,
      -1,    -1,   363,  1731,    -1,   112,    -1,    -1,    81,   370,
     117,  1739,   119,   120,   121,   122,   123,   124,   125,    -1,
     381,    -1,    -1,    -1,    -1,  1753,  1345,    -1,    -1,    -1,
     103,  1759,    -1,    -1,    -1,     4,    -1,    -1,   111,   112,
      -1,    -1,    -1,    -1,    -1,   406,    -1,    -1,   409,    -1,
      -1,    -1,   112,    -1,     4,   162,   163,   117,   165,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,  1802,    -1,    -1,    -1,    -1,   186,
      49,  1809,    -1,    -1,    -1,    -1,   159,   194,    -1,   162,
     163,  1410,   165,   166,   167,   456,  1415,    -1,    -1,    49,
    1419,    -1,   162,   163,    -1,   165,    -1,    -1,    -1,  1521,
      -1,    -1,    -1,    -1,    -1,   347,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   356,    -1,   186,    -1,    -1,    -1,
      -1,   363,    -1,   659,   194,   496,    -1,    -1,   370,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,   117,   381,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,    -1,  1498,
    1499,  1500,    -1,    -1,    -1,  1504,    -1,   548,   549,    -1,
    1509,   552,   718,   162,   163,    -1,   165,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      81,  1623,   162,   163,    -1,   165,    -1,   186,    -1,    -1,
      -1,    -1,   583,    -1,    -1,   194,    -1,    27,    28,    -1,
      -1,    31,   103,   759,    -1,   761,   186,    -1,    -1,    -1,
     111,   112,    -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,
      67,    68,    -1,    -1,    -1,    -1,    -1,    -1,   215,    -1,
      -1,    -1,    -1,   789,   496,    -1,    -1,    -1,   629,   630,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   638,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,  1705,   165,   166,   167,    -1,   659,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     846,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1649,    -1,    -1,    31,    -1,    -1,   862,   863,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    67,    68,    -1,    -1,    -1,   718,    -1,    -1,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1781,
      -1,  1690,    -1,    -1,   191,    -1,    -1,  1696,    -1,    -1,
      -1,    -1,    -1,    81,  1796,    -1,    -1,    -1,    -1,    -1,
     347,    -1,    67,    68,    -1,    -1,    -1,    -1,   759,   356,
     761,    -1,    -1,    -1,    -1,   103,   363,    -1,    -1,    -1,
      -1,    -1,  1731,   370,    -1,   215,    -1,    -1,    -1,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,   659,   789,   790,
      -1,    -1,    -1,    -1,  1753,    -1,    -1,    -1,    -1,    -1,
     138,   139,   803,   804,   805,   806,   807,   808,   809,    -1,
      -1,    -1,   813,    -1,    -1,    -1,   982,   983,   156,   134,
     135,   159,   160,   824,   162,   163,    -1,   165,   166,   167,
      -1,   169,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      -1,    -1,   180,  1802,    -1,   846,   718,    -1,    -1,    -1,
    1809,    -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,   860,
      -1,   862,   863,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   876,   877,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   886,    -1,   759,    -1,   761,
      -1,   892,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   496,
      -1,    -1,    -1,    -1,   905,    -1,    -1,   347,    -1,    -1,
      -1,    -1,   913,    -1,    -1,   916,   356,   789,    -1,    -1,
      -1,    -1,    -1,   363,    -1,    -1,    10,    11,    12,    -1,
     370,    -1,    -1,   934,    -1,    -1,    -1,   938,    -1,    -1,
      -1,   381,  1108,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   846,    -1,    -1,    -1,    -1,    -1,
      -1,   982,   983,    -1,    -1,    69,    -1,  1153,    -1,    -1,
     862,   863,    -1,    -1,    -1,   996,    -1,    -1,   999,    -1,
    1001,    -1,  1168,  1169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   886,  1016,   456,  1018,    -1,    -1,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,
    1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   496,    -1,    -1,    -1,
      -1,    -1,   659,    -1,  1065,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1246,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,  1096,    -1,  1098,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1108,    -1,    -1,
     982,   983,   552,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   718,    -1,    -1,  1125,    -1,    -1,  1128,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    68,    56,    -1,    -1,    81,
      -1,    -1,    -1,   583,    -1,    -1,  1018,  1148,    -1,    -1,
      -1,    -1,  1153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   759,    -1,   761,    -1,    -1,  1168,  1169,    -1,
    1171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1181,    31,    -1,   125,  1185,    -1,    -1,  1188,    -1,  1190,
      -1,    -1,   789,    -1,    -1,    -1,   138,   139,    -1,    -1,
      -1,   134,   135,    -1,    -1,  1206,    -1,    -1,    -1,    59,
      -1,    -1,    -1,    -1,   156,    -1,    -1,   159,   160,   659,
     162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,  1108,    -1,    -1,  1240,
    1241,    -1,  1243,    -1,    -1,  1246,    -1,    -1,    -1,   846,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   191,    -1,
      -1,   111,    -1,    -1,    -1,   862,   863,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,  1148,    -1,   718,    -1,
      -1,  1153,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,  1168,  1169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,   759,
      31,   761,    -1,    -1,    -1,    -1,  1327,    -1,  1329,    -1,
     180,    -1,    -1,  1334,  1335,    -1,   186,  1338,    -1,  1340,
     190,    -1,  1343,    -1,    -1,   265,    -1,   267,    59,   789,
     790,    -1,  1353,  1354,    -1,    -1,  1357,    -1,    -1,    -1,
      -1,    -1,    -1,  1364,    -1,   805,   806,   807,   808,   809,
      81,    -1,    -1,   813,  1246,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   824,   982,   983,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   322,    -1,    -1,  1406,   846,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
     860,  1422,   862,   863,    -1,    -1,    -1,   138,   139,    -1,
    1431,  1432,    -1,    -1,    -1,    -1,    -1,   877,  1439,    -1,
    1441,    -1,    -1,    -1,    59,   156,   886,    -1,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,    -1,
      -1,    -1,  1463,  1335,  1465,    -1,    81,    -1,    -1,   180,
    1471,    -1,    -1,   913,    -1,    -1,   916,    -1,    -1,   190,
      -1,    -1,    -1,    -1,    -1,    -1,   406,    -1,   103,   409,
      -1,   119,   120,   121,   122,   123,   124,    -1,   938,    -1,
      -1,    -1,   130,   131,    -1,    -1,  1507,  1508,    -1,    -1,
      -1,  1108,  1513,    -1,  1515,    -1,    -1,    -1,    -1,    -1,
    1521,    -1,  1523,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1406,    -1,    -1,    -1,   166,    -1,
     168,   156,   982,   983,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,   181,   169,   183,  1153,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,  1168,  1169,    -1,    -1,   190,  1016,    -1,  1018,    -1,
      -1,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1623,    50,    51,  1065,    -1,    -1,   548,   549,
      -1,    -1,   552,    -1,    -1,    69,    -1,    -1,    -1,  1640,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,  1246,
      -1,    78,    79,    80,    81,    -1,  1657,    -1,    -1,    -1,
      -1,    -1,  1663,   583,    91,    -1,    -1,    -1,  1108,    -1,
      -1,    -1,    -1,  1674,    -1,    -1,   103,    -1,    -1,  1680,
      -1,    -1,    -1,  1684,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1705,    57,    -1,    -1,  1148,   629,
     630,   138,    -1,  1153,    -1,    -1,    -1,    69,   638,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,  1168,  1169,
      -1,  1171,    -1,    -1,    -1,   162,   163,    -1,   165,   166,
     167,    -1,    -1,    -1,  1745,    -1,    -1,    -1,  1188,    -1,
    1190,    -1,    -1,   180,  1755,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1206,    -1,    -1,    -1,
      -1,  1772,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1781,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1796,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1243,    30,    31,  1246,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    31,    31,    69,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
     790,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,   803,   804,   805,   806,   807,   808,   809,
      -1,    -1,    -1,   813,  1334,  1335,    -1,    -1,  1338,    -1,
    1340,    -1,    -1,  1343,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1354,    -1,    -1,  1357,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,   876,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,  1406,    -1,   194,    -1,
      -1,    -1,   892,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1422,    -1,    -1,   905,    -1,    -1,    -1,    -1,
      -1,  1431,  1432,   913,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1441,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   934,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   215,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,  1507,  1508,    -1,
      -1,    -1,    -1,  1513,    -1,  1515,   996,    -1,    -1,   999,
      -1,  1001,    -1,  1523,    -1,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1016,    -1,    -1,    -1,
      -1,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,  1065,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    69,    -1,   347,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   356,    -1,    -1,    -1,
      -1,    -1,    -1,   363,    -1,    -1,  1096,    59,  1098,    -1,
     370,    -1,    -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,
      -1,   381,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,  1125,    -1,    -1,  1128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,  1663,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1674,    -1,    -1,    -1,    -1,    -1,
    1680,    -1,    -1,    -1,  1684,    -1,    -1,    -1,    -1,    -1,
      -1,  1171,    -1,    -1,    -1,    -1,   138,   139,    -1,    -1,
      -1,  1181,    -1,    -1,    -1,  1185,   456,    -1,  1188,    -1,
    1190,    -1,    -1,    -1,   156,    -1,    -1,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,  1206,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,    -1,  1745,   496,    -1,   190,    -1,
      10,    11,    12,    -1,    -1,  1755,    -1,    -1,    -1,    -1,
    1240,  1241,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,  1772,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,   552,    -1,    -1,    -1,    -1,    30,    31,    69,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   583,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,  1327,   552,  1329,
      -1,    -1,    -1,    -1,  1334,    -1,    -1,    -1,  1338,    -1,
    1340,    -1,    -1,  1343,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1353,    -1,    -1,    -1,    -1,    -1,   583,
      -1,    -1,    -1,    -1,  1364,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   659,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1422,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1439,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   718,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1463,    -1,  1465,    -1,    -1,    -1,    -1,
      -1,  1471,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   759,
      57,   761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,  1507,  1508,    38,
      -1,    -1,    -1,  1513,    -1,    -1,    -1,    -1,    -1,   789,
     790,  1521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   805,   806,   807,   808,   809,
      -1,    70,    -1,   813,    -1,    -1,    -1,   191,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,   790,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,   846,    -1,    -1,    -1,
      -1,   805,   806,   807,   808,    -1,    -1,    -1,    -1,   813,
      -1,    -1,   862,   863,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,   886,    -1,    -1,    -1,
      -1,    -1,   151,  1623,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
    1640,    -1,   171,   913,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,  1657,    -1,    -1,
      -1,   190,    -1,  1663,    -1,    -1,   195,    -1,   938,    -1,
      -1,    -1,    -1,    -1,  1674,    -1,    -1,    -1,    -1,    -1,
    1680,    -1,    -1,    -1,  1684,    -1,    -1,    -1,    -1,   913,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1705,    -1,    -1,    -1,    -1,
      -1,    31,   982,   983,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1745,  1016,    -1,  1018,    69,
      -1,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,    -1,
      -1,  1781,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1065,  1796,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,  1048,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1108,    -1,
      -1,  1065,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,  1148,    -1,
      -1,    -1,    -1,  1153,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1168,  1169,
      -1,  1171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1188,    -1,
    1190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1206,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1171,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1188,    -1,  1190,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,  1246,    13,    -1,    -1,
      -1,    -1,  1206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,   194,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1334,  1335,    -1,   103,  1338,    -1,
    1340,    -1,    -1,  1343,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
    1334,    -1,    -1,    -1,  1338,   151,  1340,    -1,    -1,  1343,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,  1406,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,    10,    11,    12,    -1,   185,
     186,   187,  1422,    -1,   190,    -1,    -1,    -1,    -1,   195,
     196,    -1,   198,   199,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,  1422,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,  1507,  1508,    -1,
      -1,    -1,    -1,  1513,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1522,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,  1507,  1508,    81,    -1,    83,    84,  1513,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,    -1,   162,   163,    -1,   165,
     166,   167,   168,    -1,   170,    -1,    -1,   173,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1663,    -1,    -1,    -1,   193,    -1,   195,
      -1,    -1,    -1,    -1,  1674,    -1,    -1,    -1,    -1,    -1,
    1680,    -1,    -1,    -1,  1684,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,  1707,    -1,  1663,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
    1674,    -1,    -1,    -1,    -1,    -1,  1680,    -1,    -1,    -1,
    1684,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,  1745,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,  1745,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,   168,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,   192,   193,   194,   195,   196,    -1,   198,   199,
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
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,    -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,
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
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,   168,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,   192,   193,    -1,   195,
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
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,   148,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
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
      91,    92,    93,    94,    95,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,    -1,
     151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,   160,
      -1,   162,   163,    -1,   165,   166,   167,   168,    -1,    -1,
     171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,
      -1,   192,   193,    -1,   195,   196,    -1,   198,   199,     3,
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
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,   193,
      -1,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
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
     190,    -1,   192,   193,   194,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    99,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,   148,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,   168,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,   186,   187,    -1,    -1,   190,    -1,   192,
     193,    -1,   195,   196,    -1,   198,   199,     3,     4,     5,
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
      -1,    -1,    91,    92,    93,    94,    -1,    96,    97,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
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
      -1,   195,   196,    -1,   198,   199,     3,     4,     5,     6,
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
     137,   138,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,   148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,
     157,   158,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   190,    -1,   192,   193,    -1,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   148,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    11,    12,   190,    -1,   192,    -1,    -1,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    31,    -1,    13,    34,    35,    36,    37,
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
     138,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,
     158,   159,   160,    -1,   162,   163,    -1,   165,   166,   167,
      -1,   169,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
      -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,
     198,   199,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
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
      -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,     3,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     187,    -1,    -1,   190,    10,    11,    12,    -1,   195,   196,
      -1,   198,   199,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,    35,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,   194,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    -1,    -1,    -1,    -1,   195,   196,    -1,   198,   199,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    31,    -1,
      13,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,    -1,
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
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,   194,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,   192,    11,    12,   195,
     196,    -1,   198,   199,     3,     4,     5,     6,     7,    -1,
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
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,    -1,
      -1,    -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,
      -1,   190,    -1,   192,    -1,    -1,   195,   196,    -1,   198,
     199,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,    -1,
      -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,    10,
      11,    12,    -1,   195,   196,    -1,   198,   199,     3,     4,
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
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,
      -1,    -1,    -1,   194,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,    -1,    -1,   190,   191,    -1,    -1,    -1,
     195,   196,    -1,   198,   199,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    32,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,
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
      -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,   190,
      -1,    -1,    -1,    -1,   195,   196,    -1,   198,   199,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,
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
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,    -1,    -1,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,   185,   186,   187,    -1,    -1,
     190,    10,    11,    12,    -1,   195,   196,    -1,   198,   199,
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
      -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,   194,    -1,   180,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,    -1,
     156,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
     166,   167,    -1,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,   194,    -1,   180,    -1,    -1,    -1,    -1,   185,
     186,   187,    -1,    -1,   190,    -1,    -1,    -1,    -1,   195,
     196,    -1,   198,   199,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   156,    10,    11,
      12,    13,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,   170,    -1,    -1,   173,    27,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,   159,   160,    -1,
     162,   163,    -1,   165,   166,   167,   168,    -1,   170,    -1,
      -1,   173,     3,     4,    -1,     6,     7,    -1,   180,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,   194,    -1,    -1,    -1,    27,    -1,    29,    -1,
      31,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   173,     3,     4,    -1,     6,     7,    -1,   180,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    -1,   194,    -1,    -1,    -1,    27,    -1,    29,
      -1,    31,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,   159,
     160,    -1,   162,   163,    -1,   165,   166,   167,   168,    -1,
     170,    -1,    -1,   173,     3,     4,    -1,     6,     7,    -1,
     180,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    31,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      59,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
     159,   160,    -1,   162,   163,    -1,   165,   166,   167,   168,
      -1,   170,    -1,    -1,   173,    -1,     3,     4,    -1,     6,
       7,   180,   181,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    31,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    59,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,   159,   160,    -1,   162,   163,    -1,   165,   166,
     167,   168,    -1,   170,    -1,    -1,   173,     3,     4,     5,
       6,     7,    -1,   180,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,   158,    -1,    -1,    -1,   162,   163,    -1,   165,
     166,   167,   168,    -1,   170,   171,    -1,   173,    10,    11,
      12,    -1,    -1,    -1,   180,   181,    -1,   183,    -1,   185,
     186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     192,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   192,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   192,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     192,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   192,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   192,    -1,    27,    -1,
      29,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,   191,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   156,    -1,    -1,
      -1,    -1,    -1,   162,   163,    -1,   165,   166,   167,   168,
      27,   170,    29,    -1,   173,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   156,
      -1,    -1,    -1,    -1,    -1,   162,   163,    -1,   165,   166,
     167,   168,    27,   170,    29,    -1,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,   131,   132,   133,    32,
      -1,    -1,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,   156,    -1,    56,    -1,    58,    -1,   162,   163,    -1,
     165,   166,   167,   168,    -1,   170,    -1,    70,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,   138,    -1,   140,   141,   142,
     143,   144,    69,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,    -1,    -1,   156,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,   166,   167,    50,    51,    -1,   171,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,   180,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    70,    -1,   190,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    91,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,   138,    -1,   140,   141,   142,   143,   144,
      69,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,    -1,
      -1,   156,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,   166,   167,    -1,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
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
     124,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,
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
      -1,    -1,   171,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,   185,    91,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,   195,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
      -1,    -1,   156,   157,   158,   159,   160,    -1,   162,   163,
      -1,   165,   166,   167,    -1,    -1,    -1,   171,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,    28,    -1,    30,
      31,   195,    33,    34,    35,    36,    37,    38,    39,    40,
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
     426,   427,   428,   429,   443,   445,   447,   122,   123,   124,
     137,   156,   166,   190,   207,   240,   321,   342,   420,   342,
     190,   342,   342,   342,   108,   342,   342,   342,   406,   407,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,    83,    91,   124,   151,   190,   218,   361,   378,
     381,   386,   420,   423,   420,    38,   342,   434,   435,   342,
     124,   130,   190,   218,   253,   378,   379,   380,   382,   386,
     417,   418,   419,   427,   431,   432,   190,   331,   383,   190,
     331,   352,   332,   342,   226,   331,   190,   190,   190,   331,
     192,   342,   207,   192,   342,     3,     4,     6,     7,    10,
      11,    12,    13,    27,    29,    31,    57,    59,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   130,
     131,   132,   133,   137,   138,   139,   156,   160,   168,   170,
     173,   180,   190,   207,   208,   209,   220,   448,   464,   465,
     467,   192,   337,   339,   342,   193,   233,   342,   111,   112,
     159,   210,   211,   212,   213,   217,    83,   195,   287,   288,
     123,   130,   122,   130,    83,   289,   190,   190,   190,   190,
     207,   259,   451,   190,   190,   332,    83,    90,   152,   153,
     154,   440,   441,   159,   193,   217,   217,   207,   260,   451,
     160,   190,   451,   451,    83,   187,   193,   353,    27,   330,
     334,   342,   343,   420,   424,   222,   193,   429,    90,   384,
     440,    90,   440,   440,    32,   159,   176,   452,   190,     9,
     192,    38,   239,   160,   258,   451,   124,   186,   240,   322,
     192,   192,   192,   192,   192,   192,   192,   192,    10,    11,
      12,    30,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    69,   192,    70,
      70,   193,   155,   131,   166,   168,   181,   183,   261,   320,
     321,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    67,    68,   134,   135,   410,    70,
     193,   415,   190,   190,    70,   193,   190,   239,   240,    14,
     342,   192,   136,    48,   207,   405,    90,   330,   343,   155,
     420,   136,   197,     9,   391,   254,   330,   343,   420,   452,
     155,   190,   385,   410,   415,   191,   342,    32,   224,     8,
     354,     9,   192,   224,   225,   332,   333,   342,   207,   273,
     228,   192,   192,   192,   138,   139,   467,   467,   176,   190,
     111,   467,    14,   155,   138,   139,   156,   207,   209,   192,
     192,   192,   234,   115,   173,   192,   210,   212,   210,   212,
     217,   193,     9,   392,   192,   102,   159,   193,   420,     9,
     192,   130,   130,    14,     9,   192,   420,   444,   332,   330,
     343,   420,   423,   424,   191,   176,   251,   137,   420,   433,
     434,   192,    70,   410,   152,   441,    82,   342,   420,    90,
     152,   441,   217,   206,   192,   193,   246,   256,   368,   370,
      91,   190,   355,   356,   358,   381,   426,   428,   445,    14,
     102,   446,   349,   350,   351,   283,   284,   408,   409,   191,
     191,   191,   191,   191,   194,   223,   224,   241,   248,   255,
     408,   342,   196,   198,   199,   207,   453,   454,   467,    38,
     169,   285,   286,   342,   448,   190,   451,   249,   239,   342,
     342,   342,   342,    32,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   382,   342,
     342,   430,   430,   342,   436,   437,   130,   193,   208,   209,
     429,   259,   207,   260,   451,   451,   258,   240,    38,   334,
     337,   339,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   160,   193,   207,   411,   412,
     413,   414,   429,   430,   342,   285,   285,   430,   342,   433,
     239,   191,   342,   190,   404,     9,   391,   191,   191,    38,
     342,    38,   342,   385,   191,   191,   191,   427,   428,   429,
     285,   193,   207,   411,   412,   429,   191,   222,   277,   193,
     339,   342,   342,    94,    32,   224,   271,   192,    28,   102,
      14,     9,   191,    32,   193,   274,   467,    31,    91,   220,
     461,   462,   463,   190,     9,    50,    51,    56,    58,    70,
     138,   160,   180,   190,   218,   220,   363,   378,   386,   387,
     388,   207,   466,   222,   190,   232,   193,   192,   193,   192,
     102,   159,   111,   112,   159,   213,   214,   215,   216,   217,
     213,   207,   342,   288,   387,    83,     9,   191,   191,   191,
     191,   191,   191,   191,   192,    50,    51,   458,   459,   460,
     132,   264,   190,     9,   191,   191,    83,    85,   207,   442,
     207,    70,   194,   194,   203,   205,    32,   133,   263,   175,
      54,   160,   175,   372,   343,   136,     9,   391,   191,   155,
     467,   467,    14,   354,   283,   222,   188,     9,   392,   467,
     468,   410,   415,   410,   194,     9,   391,   177,   420,   342,
     191,     9,   392,    14,   346,   242,   132,   262,   190,   451,
     342,    32,   197,   197,   136,   194,     9,   391,   342,   452,
     190,   252,   247,   257,    14,   446,   250,   239,    72,   420,
     342,   452,   197,   194,   191,   191,   197,   194,   191,    50,
      51,    70,    78,    79,    80,    91,   138,   151,   180,   207,
     394,   396,   397,   400,   403,   207,   420,   420,   136,   262,
     410,   415,   191,   342,   278,    75,    76,   279,   222,   331,
     222,   333,   102,    38,   137,   268,   420,   387,   207,    32,
     224,   272,   192,   275,   192,   275,     9,   391,    91,   136,
     155,     9,   391,   191,   169,   453,   454,   455,   453,   387,
     387,   387,   387,   387,   390,   393,   190,   155,   190,   387,
     155,   193,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   155,
     452,   194,   378,   193,   236,   212,   212,   207,   213,   213,
     217,     9,   392,   194,   194,    14,   420,   192,   177,     9,
     391,   207,   265,   378,   193,   433,   137,   420,    14,   197,
     342,   194,   203,   467,   265,   193,   371,    14,   191,   342,
     355,   429,   192,   467,   188,   194,    32,   456,   409,    38,
      83,   169,   411,   412,   414,   411,   412,   467,    38,   169,
     342,   387,   283,   190,   378,   263,   347,   243,   342,   342,
     342,   194,   190,   285,   264,    32,   263,   467,    14,   262,
     451,   382,   194,   190,    14,    78,    79,    80,   207,   395,
     395,   397,   398,   399,    52,   190,    90,   152,   190,     9,
     391,   191,   404,    38,   342,   263,   194,    75,    76,   280,
     331,   224,   194,   192,    95,   192,   268,   420,   190,   136,
     267,    14,   222,   275,   105,   106,   107,   275,   194,   467,
     177,   136,   467,   207,   461,     9,   191,   391,   136,   197,
       9,   391,   390,   208,   355,   357,   359,   191,   130,   208,
     387,   438,   439,   387,   387,   387,    32,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   387,   387,   387,   387,   387,   387,   387,   387,   387,
     387,   466,    83,   237,   194,   194,   216,   192,   387,   460,
     102,   103,   457,     9,   293,   191,   190,   334,   339,   342,
     197,   194,   446,   293,   161,   174,   193,   367,   374,   161,
     193,   373,   136,   192,   456,   467,   354,   468,    83,   169,
      14,    83,   452,   420,   342,   191,   283,   193,   283,   190,
     136,   190,   285,   191,   193,   467,   193,   192,   467,   263,
     244,   385,   285,   136,   197,     9,   391,   396,   398,   152,
     355,   401,   402,   397,   420,   193,   331,    32,    77,   224,
     192,   333,   267,   433,   268,   191,   387,   101,   105,   192,
     342,    32,   192,   276,   194,   177,   467,   136,   169,    32,
     191,   387,   387,   191,   136,     9,   391,   191,   136,   194,
       9,   391,   387,    32,   191,   222,   192,   192,   207,   467,
     467,   378,     4,   112,   117,   123,   125,   162,   163,   165,
     194,   294,   319,   320,   321,   326,   327,   328,   329,   408,
     433,   194,   193,   194,    54,   342,   342,   342,   354,    38,
      83,   169,    14,    83,   342,   190,   456,   191,   293,   191,
     283,   342,   285,   191,   293,   446,   293,   192,   193,   190,
     191,   397,   397,   191,   136,   191,     9,   391,   293,    32,
     222,   192,   191,   191,   191,   229,   192,   192,   276,   222,
     467,   467,   136,   387,   355,   387,   387,   387,   193,   194,
     457,   132,   133,   181,   208,   449,   467,   266,   378,   112,
     329,    31,   125,   138,   139,   160,   166,   303,   304,   305,
     306,   378,   164,   311,   312,   128,   190,   207,   313,   314,
     295,   240,   467,     9,   192,     9,   192,   192,   446,   320,
     191,   290,   160,   369,   194,   194,    83,   169,    14,    83,
     342,   285,   117,   344,   456,   194,   456,   191,   191,   194,
     193,   194,   293,   283,   136,   397,   355,   194,   222,   227,
     230,    32,   224,   270,   222,   191,   387,   136,   136,   222,
     378,   378,   451,    14,   208,     9,   192,   193,   449,   446,
     306,   176,   193,     9,   192,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    29,    57,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   137,   138,   140,   141,
     142,   143,   144,   156,   157,   158,   168,   170,   171,   173,
     180,   181,   183,   185,   186,   207,   375,   376,     9,   192,
     160,   164,   207,   314,   315,   316,   192,    83,   325,   239,
     296,   449,   449,    14,   240,   194,   291,   292,   449,    14,
      83,   342,   191,   190,   456,   192,   193,   317,   344,   456,
     290,   194,   191,   397,   136,    32,   224,   269,   270,   222,
     387,   387,   194,   192,   192,   387,   378,   299,   467,   307,
     308,   386,   304,    14,    32,    51,   309,   312,     9,    36,
     191,    31,    50,    53,    14,     9,   192,   209,   450,   325,
      14,   467,   239,   192,    14,   342,    38,    83,   366,   193,
     222,   456,   317,   194,   456,   397,   222,    99,   235,   194,
     207,   220,   300,   301,   302,     9,   391,     9,   391,   194,
     387,   376,   376,    59,   310,   315,   315,    31,    50,    53,
     387,    83,   176,   190,   192,   387,   451,   387,    83,     9,
     392,   222,   194,   193,   317,    97,   192,   115,   231,   155,
     102,   467,   177,   386,   167,    14,   458,   297,   190,    38,
      83,   191,   194,   222,   192,   190,   173,   238,   207,   320,
     321,   177,   387,   177,   281,   282,   409,   298,    83,   194,
     378,   236,   170,   207,   192,   191,     9,   392,   119,   120,
     121,   323,   324,   281,    83,   266,   192,   456,   409,   468,
     191,   191,   192,   192,   193,   318,   323,    38,    83,   169,
     456,   193,   222,   468,    83,   169,    14,    83,   318,   222,
     194,    38,    83,   169,    14,    83,   342,   194,    83,   169,
      14,    83,   342,    14,    83,   342,   342
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
#line 2682 "hphp.y"
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
#line 2697 "hphp.y"
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
#line 2832 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2909 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval).reset();;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

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

  case 872:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

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

  case 878:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { (yyval).reset();;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 913:

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

  case 914:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    {;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    {;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    {;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 971:

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

  case 972:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3238 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3273 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { (yyvsp[(1) - (5)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)])); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3289 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3313 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13897 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

