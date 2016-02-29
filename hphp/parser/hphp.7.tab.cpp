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
#line 877 "hphp.7.tab.cpp"

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
#define YYLAST   17865

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  201
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  279
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1021
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1883

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
    2810,  2814,  2816,  2821,  2825,  2829,  2831,  2833,  2835,  2837,
    2839,  2843,  2847,  2852,  2857,  2861,  2863,  2865,  2873,  2883,
    2891,  2898,  2907,  2909,  2914,  2919,  2921,  2923,  2925,  2930,
    2933,  2935,  2936,  2938,  2940,  2942,  2946,  2950,  2954,  2955,
    2957,  2959,  2963,  2967,  2970,  2974,  2981,  2982,  2984,  2989,
    2992,  2993,  2999,  3003,  3007,  3009,  3016,  3021,  3026,  3029,
    3032,  3033,  3039,  3043,  3047,  3049,  3052,  3053,  3059,  3063,
    3067,  3069,  3072,  3075,  3077,  3080,  3082,  3087,  3091,  3095,
    3102,  3106,  3108,  3110,  3112,  3117,  3122,  3127,  3132,  3137,
    3142,  3145,  3148,  3153,  3156,  3159,  3161,  3165,  3169,  3173,
    3174,  3177,  3183,  3190,  3197,  3205,  3207,  3210,  3212,  3215,
    3217,  3222,  3224,  3229,  3233,  3234,  3236,  3240,  3243,  3247,
    3249,  3251,  3252,  3253,  3256,  3259,  3262,  3265,  3270,  3273,
    3279,  3283,  3285,  3287,  3288,  3292,  3297,  3303,  3307,  3309,
    3312,  3313,  3318,  3320,  3324,  3327,  3332,  3338,  3341,  3344,
    3346,  3348,  3350,  3352,  3354,  3358,  3361,  3364,  3366,  3375,
    3382,  3384
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     202,     0,    -1,    -1,   203,   204,    -1,   204,   205,    -1,
      -1,   225,    -1,   242,    -1,   249,    -1,   246,    -1,   256,
      -1,   457,    -1,   129,   191,   192,   193,    -1,   157,   218,
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
     218,    -1,   160,   218,    -1,   219,   462,    -1,   219,   462,
      -1,   222,     9,   458,    14,   397,    -1,   112,   458,    14,
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
     193,    -1,   149,   333,   193,    -1,   126,   191,   454,   192,
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
     461,   243,   191,   284,   192,   466,   318,    -1,    -1,   322,
     241,   240,   461,   244,   191,   284,   192,   466,   318,    -1,
      -1,   418,   321,   241,   240,   461,   245,   191,   284,   192,
     466,   318,    -1,    -1,   167,   208,   247,    32,   478,   456,
     194,   291,   195,    -1,    -1,   418,   167,   208,   248,    32,
     478,   456,   194,   291,   195,    -1,    -1,   262,   259,   250,
     263,   264,   194,   294,   195,    -1,    -1,   418,   262,   259,
     251,   263,   264,   194,   294,   195,    -1,    -1,   131,   260,
     252,   265,   194,   294,   195,    -1,    -1,   418,   131,   260,
     253,   265,   194,   294,   195,    -1,    -1,   130,   255,   395,
     263,   264,   194,   294,   195,    -1,    -1,   169,   261,   257,
     264,   194,   294,   195,    -1,    -1,   418,   169,   261,   258,
     264,   194,   294,   195,    -1,   461,    -1,   161,    -1,   461,
      -1,   461,    -1,   130,    -1,   123,   130,    -1,   123,   122,
     130,    -1,   122,   123,   130,    -1,   122,   130,    -1,   132,
     388,    -1,    -1,   133,   266,    -1,    -1,   132,   266,    -1,
      -1,   388,    -1,   266,     9,   388,    -1,   388,    -1,   267,
       9,   388,    -1,   136,   269,    -1,    -1,   430,    -1,    38,
     430,    -1,   137,   191,   443,   192,    -1,   225,    -1,    32,
     223,    97,   193,    -1,   225,    -1,    32,   223,    99,   193,
      -1,   225,    -1,    32,   223,    95,   193,    -1,   225,    -1,
      32,   223,   101,   193,    -1,   208,    14,   397,    -1,   274,
       9,   208,    14,   397,    -1,   194,   276,   195,    -1,   194,
     193,   276,   195,    -1,    32,   276,   105,   193,    -1,    32,
     193,   276,   105,   193,    -1,   276,   106,   343,   277,   223,
      -1,   276,   107,   277,   223,    -1,    -1,    32,    -1,   193,
      -1,   278,    75,   332,   225,    -1,    -1,   279,    75,   332,
      32,   223,    -1,    -1,    76,   225,    -1,    -1,    76,    32,
     223,    -1,    -1,   283,     9,   419,   324,   479,   170,    83,
      -1,   283,     9,   419,   324,   479,    38,   170,    83,    -1,
     283,     9,   419,   324,   479,   170,    -1,   283,   402,    -1,
     419,   324,   479,   170,    83,    -1,   419,   324,   479,    38,
     170,    83,    -1,   419,   324,   479,   170,    -1,    -1,   419,
     324,   479,    83,    -1,   419,   324,   479,    38,    83,    -1,
     419,   324,   479,    38,    83,    14,   343,    -1,   419,   324,
     479,    83,    14,   343,    -1,   283,     9,   419,   324,   479,
      83,    -1,   283,     9,   419,   324,   479,    38,    83,    -1,
     283,     9,   419,   324,   479,    38,    83,    14,   343,    -1,
     283,     9,   419,   324,   479,    83,    14,   343,    -1,   285,
       9,   419,   479,   170,    83,    -1,   285,     9,   419,   479,
      38,   170,    83,    -1,   285,     9,   419,   479,   170,    -1,
     285,   402,    -1,   419,   479,   170,    83,    -1,   419,   479,
      38,   170,    83,    -1,   419,   479,   170,    -1,    -1,   419,
     479,    83,    -1,   419,   479,    38,    83,    -1,   419,   479,
      38,    83,    14,   343,    -1,   419,   479,    83,    14,   343,
      -1,   285,     9,   419,   479,    83,    -1,   285,     9,   419,
     479,    38,    83,    -1,   285,     9,   419,   479,    38,    83,
      14,   343,    -1,   285,     9,   419,   479,    83,    14,   343,
      -1,   287,   402,    -1,    -1,   343,    -1,    38,   430,    -1,
     170,   343,    -1,   287,     9,   343,    -1,   287,     9,   170,
     343,    -1,   287,     9,    38,   430,    -1,   288,     9,   289,
      -1,   289,    -1,    83,    -1,   196,   430,    -1,   196,   194,
     343,   195,    -1,   290,     9,    83,    -1,   290,     9,    83,
      14,   397,    -1,    83,    -1,    83,    14,   397,    -1,   291,
     292,    -1,    -1,   293,   193,    -1,   459,    14,   397,    -1,
     294,   295,    -1,    -1,    -1,   320,   296,   326,   193,    -1,
      -1,   322,   478,   297,   326,   193,    -1,   327,   193,    -1,
     328,   193,    -1,   329,   193,    -1,    -1,   321,   241,   240,
     460,   191,   298,   282,   192,   466,   319,    -1,    -1,   418,
     321,   241,   240,   461,   191,   299,   282,   192,   466,   319,
      -1,   163,   304,   193,    -1,   164,   312,   193,    -1,   166,
     314,   193,    -1,     4,   132,   388,   193,    -1,     4,   133,
     388,   193,    -1,   117,   267,   193,    -1,   117,   267,   194,
     300,   195,    -1,   300,   301,    -1,   300,   302,    -1,    -1,
     221,   156,   208,   171,   267,   193,    -1,   303,   102,   321,
     208,   193,    -1,   303,   102,   322,   193,    -1,   221,   156,
     208,    -1,   208,    -1,   305,    -1,   304,     9,   305,    -1,
     306,   385,   310,   311,    -1,   161,    -1,    31,   307,    -1,
     307,    -1,   138,    -1,   138,   177,   478,   401,   178,    -1,
     138,   177,   478,     9,   478,   178,    -1,   388,    -1,   125,
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
     459,    14,   397,    -1,   112,   459,    14,   397,    -1,   328,
       9,   459,    -1,   123,   112,   459,    -1,   123,   330,   456,
      -1,   330,   456,    14,   478,    -1,   112,   182,   461,    -1,
     191,   331,   192,    -1,    72,   392,   395,    -1,    72,   254,
      -1,    71,   343,    -1,   377,    -1,   372,    -1,   191,   343,
     192,    -1,   333,     9,   343,    -1,   343,    -1,   333,    -1,
      -1,    27,    -1,    27,   343,    -1,    27,   343,   136,   343,
      -1,   191,   335,   192,    -1,   430,    14,   335,    -1,   137,
     191,   443,   192,    14,   335,    -1,    29,   343,    -1,   430,
      14,   338,    -1,    28,   343,    -1,   430,    14,   340,    -1,
     137,   191,   443,   192,    14,   340,    -1,   344,    -1,   430,
      -1,   331,    -1,   434,    -1,   433,    -1,   137,   191,   443,
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
      33,   343,    -1,   453,    -1,    66,   343,    -1,    65,   343,
      -1,    64,   343,    -1,    63,   343,    -1,    62,   343,    -1,
      61,   343,    -1,    60,   343,    -1,    73,   393,    -1,    59,
     343,    -1,   399,    -1,   362,    -1,   369,    -1,   361,    -1,
     197,   394,   197,    -1,    13,   343,    -1,   374,    -1,   117,
     191,   376,   402,   192,    -1,    -1,    -1,   241,   240,   191,
     347,   284,   192,   466,   345,   466,   194,   223,   195,    -1,
      -1,   322,   241,   240,   191,   348,   284,   192,   466,   345,
     466,   194,   223,   195,    -1,    -1,   187,    83,   350,   355,
      -1,    -1,   187,   188,   351,   284,   189,   466,   355,    -1,
      -1,   187,   194,   352,   223,   195,    -1,    -1,    83,   353,
     355,    -1,    -1,   188,   354,   284,   189,   466,   355,    -1,
       8,   343,    -1,     8,   340,    -1,     8,   194,   223,   195,
      -1,    91,    -1,   455,    -1,   357,     9,   356,   136,   343,
      -1,   356,   136,   343,    -1,   358,     9,   356,   136,   397,
      -1,   356,   136,   397,    -1,   357,   401,    -1,    -1,   358,
     401,    -1,    -1,   181,   191,   359,   192,    -1,   138,   191,
     444,   192,    -1,    70,   444,   198,    -1,   364,   401,    -1,
      -1,   364,     9,   343,   136,   343,    -1,   343,   136,   343,
      -1,   364,     9,   343,   136,    38,   430,    -1,   343,   136,
      38,   430,    -1,   366,   401,    -1,    -1,   366,     9,   397,
     136,   397,    -1,   397,   136,   397,    -1,   368,   401,    -1,
      -1,   368,     9,   407,   136,   407,    -1,   407,   136,   407,
      -1,   139,    70,   363,   198,    -1,   139,    70,   365,   198,
      -1,   139,    70,   367,   198,    -1,   388,   194,   446,   195,
      -1,   388,   194,   448,   195,    -1,   374,    70,   440,   198,
      -1,   375,    70,   440,   198,    -1,   362,    -1,   369,    -1,
     455,    -1,   433,    -1,    91,    -1,   191,   344,   192,    -1,
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
     390,   156,   439,    -1,   389,    -1,   436,    -1,   391,   156,
     439,    -1,   388,    -1,   124,    -1,   441,    -1,   191,   192,
      -1,   332,    -1,    -1,    -1,    90,    -1,   450,    -1,   191,
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
     221,   156,   130,    -1,   219,    -1,    82,    -1,   455,    -1,
     396,    -1,   199,   450,   199,    -1,   200,   450,   200,    -1,
     152,   450,   153,    -1,   403,   401,    -1,    -1,     9,    -1,
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
     161,    -1,   194,   343,   195,    -1,   421,    -1,   439,    -1,
     208,    -1,   194,   343,   195,    -1,   423,    -1,   439,    -1,
      70,   440,   198,    -1,   194,   343,   195,    -1,   431,   425,
      -1,   191,   331,   192,   425,    -1,   442,   425,    -1,   191,
     331,   192,   425,    -1,   191,   331,   192,   420,   422,    -1,
     191,   344,   192,   420,   422,    -1,   191,   331,   192,   420,
     421,    -1,   191,   344,   192,   420,   421,    -1,   437,    -1,
     387,    -1,   435,    -1,   436,    -1,   426,    -1,   428,    -1,
     430,   420,   422,    -1,   391,   156,   439,    -1,   432,   191,
     286,   192,    -1,   433,   191,   286,   192,    -1,   191,   430,
     192,    -1,   387,    -1,   435,    -1,   436,    -1,   426,    -1,
     430,   420,   422,    -1,   429,    -1,   432,   191,   286,   192,
      -1,   191,   430,   192,    -1,   391,   156,   439,    -1,   437,
      -1,   426,    -1,   387,    -1,   362,    -1,   396,    -1,   191,
     430,   192,    -1,   191,   344,   192,    -1,   433,   191,   286,
     192,    -1,   432,   191,   286,   192,    -1,   191,   434,   192,
      -1,   346,    -1,   349,    -1,   430,   420,   424,   462,   191,
     286,   192,    -1,   191,   331,   192,   420,   424,   462,   191,
     286,   192,    -1,   391,   156,   210,   462,   191,   286,   192,
      -1,   391,   156,   439,   191,   286,   192,    -1,   391,   156,
     194,   343,   195,   191,   286,   192,    -1,   438,    -1,   438,
      70,   440,   198,    -1,   438,   194,   343,   195,    -1,   439,
      -1,    83,    -1,    84,    -1,   196,   194,   343,   195,    -1,
     196,   439,    -1,   343,    -1,    -1,   437,    -1,   427,    -1,
     428,    -1,   441,   420,   422,    -1,   390,   156,   437,    -1,
     191,   430,   192,    -1,    -1,   427,    -1,   429,    -1,   441,
     420,   421,    -1,   191,   430,   192,    -1,   443,     9,    -1,
     443,     9,   430,    -1,   443,     9,   137,   191,   443,   192,
      -1,    -1,   430,    -1,   137,   191,   443,   192,    -1,   445,
     401,    -1,    -1,   445,     9,   343,   136,   343,    -1,   445,
       9,   343,    -1,   343,   136,   343,    -1,   343,    -1,   445,
       9,   343,   136,    38,   430,    -1,   445,     9,    38,   430,
      -1,   343,   136,    38,   430,    -1,    38,   430,    -1,   447,
     401,    -1,    -1,   447,     9,   343,   136,   343,    -1,   447,
       9,   343,    -1,   343,   136,   343,    -1,   343,    -1,   449,
     401,    -1,    -1,   449,     9,   397,   136,   397,    -1,   449,
       9,   397,    -1,   397,   136,   397,    -1,   397,    -1,   450,
     451,    -1,   450,    90,    -1,   451,    -1,    90,   451,    -1,
      83,    -1,    83,    70,   452,   198,    -1,    83,   420,   208,
      -1,   154,   343,   195,    -1,   154,    82,    70,   343,   198,
     195,    -1,   155,   430,   195,    -1,   208,    -1,    85,    -1,
      83,    -1,   127,   191,   333,   192,    -1,   128,   191,   430,
     192,    -1,   128,   191,   344,   192,    -1,   128,   191,   434,
     192,    -1,   128,   191,   433,   192,    -1,   128,   191,   331,
     192,    -1,     7,   343,    -1,     6,   343,    -1,     5,   191,
     343,   192,    -1,     4,   343,    -1,     3,   343,    -1,   430,
      -1,   454,     9,   430,    -1,   391,   156,   209,    -1,   391,
     156,   130,    -1,    -1,   102,   478,    -1,   182,   461,    14,
     478,   193,    -1,   418,   182,   461,    14,   478,   193,    -1,
     184,   461,   456,    14,   478,   193,    -1,   418,   184,   461,
     456,    14,   478,   193,    -1,   210,    -1,   478,   210,    -1,
     209,    -1,   478,   209,    -1,   210,    -1,   210,   177,   468,
     178,    -1,   208,    -1,   208,   177,   468,   178,    -1,   177,
     464,   178,    -1,    -1,   478,    -1,   463,     9,   478,    -1,
     463,   401,    -1,   463,     9,   170,    -1,   464,    -1,   170,
      -1,    -1,    -1,    32,   478,    -1,   102,   478,    -1,   103,
     478,    -1,   469,   401,    -1,   469,     9,   470,   208,    -1,
     470,   208,    -1,   469,     9,   470,   208,   467,    -1,   470,
     208,   467,    -1,    50,    -1,    51,    -1,    -1,    91,   136,
     478,    -1,    31,    91,   136,   478,    -1,   221,   156,   208,
     136,   478,    -1,   472,     9,   471,    -1,   471,    -1,   472,
     401,    -1,    -1,   181,   191,   473,   192,    -1,   221,    -1,
     208,   156,   476,    -1,   208,   462,    -1,   177,   478,   401,
     178,    -1,   177,   478,     9,   478,   178,    -1,    31,   478,
      -1,    59,   478,    -1,   221,    -1,   138,    -1,   139,    -1,
     140,    -1,   474,    -1,   475,   156,   476,    -1,   138,   477,
      -1,   139,   477,    -1,   161,    -1,   191,   111,   191,   465,
     192,    32,   478,   192,    -1,   191,   478,     9,   463,   401,
     192,    -1,   478,    -1,    -1
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
    2748,  2753,  2754,  2758,  2763,  2768,  2769,  2773,  2774,  2779,
    2781,  2786,  2797,  2811,  2823,  2838,  2839,  2840,  2841,  2842,
    2843,  2844,  2854,  2863,  2865,  2867,  2871,  2872,  2873,  2874,
    2875,  2891,  2892,  2894,  2896,  2903,  2904,  2905,  2906,  2907,
    2908,  2909,  2910,  2912,  2917,  2921,  2922,  2926,  2929,  2936,
    2940,  2949,  2956,  2964,  2966,  2967,  2971,  2972,  2973,  2975,
    2980,  2981,  2992,  2993,  2994,  2995,  3006,  3009,  3012,  3013,
    3014,  3015,  3026,  3030,  3031,  3032,  3034,  3035,  3036,  3040,
    3042,  3045,  3047,  3048,  3049,  3050,  3053,  3055,  3056,  3060,
    3062,  3065,  3067,  3068,  3069,  3073,  3075,  3078,  3081,  3083,
    3085,  3089,  3090,  3092,  3093,  3099,  3100,  3102,  3112,  3114,
    3116,  3119,  3120,  3121,  3125,  3126,  3127,  3128,  3129,  3130,
    3131,  3132,  3133,  3134,  3135,  3139,  3140,  3144,  3146,  3154,
    3156,  3160,  3164,  3169,  3173,  3181,  3182,  3186,  3187,  3193,
    3194,  3203,  3204,  3212,  3215,  3219,  3222,  3227,  3232,  3234,
    3235,  3236,  3240,  3241,  3245,  3246,  3249,  3254,  3257,  3259,
    3263,  3269,  3270,  3271,  3275,  3279,  3289,  3297,  3299,  3303,
    3305,  3310,  3316,  3319,  3324,  3329,  3331,  3338,  3341,  3344,
    3345,  3348,  3351,  3354,  3355,  3360,  3362,  3364,  3368,  3374,
    3384,  3385
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
     431,   431,   431,   431,   431,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   433,   434,   434,   435,   435,   436,
     436,   436,   437,   438,   438,   438,   439,   439,   439,   439,
     440,   440,   441,   441,   441,   441,   441,   441,   442,   442,
     442,   442,   442,   443,   443,   443,   443,   443,   443,   444,
     444,   445,   445,   445,   445,   445,   445,   445,   445,   446,
     446,   447,   447,   447,   447,   448,   448,   449,   449,   449,
     449,   450,   450,   450,   450,   451,   451,   451,   451,   451,
     451,   452,   452,   452,   453,   453,   453,   453,   453,   453,
     453,   453,   453,   453,   453,   454,   454,   455,   455,   456,
     456,   457,   457,   457,   457,   458,   458,   459,   459,   460,
     460,   461,   461,   462,   462,   463,   463,   464,   465,   465,
     465,   465,   466,   466,   467,   467,   468,   469,   469,   469,
     469,   470,   470,   470,   471,   471,   471,   472,   472,   473,
     473,   474,   475,   476,   476,   477,   477,   478,   478,   478,
     478,   478,   478,   478,   478,   478,   478,   478,   478,   478,
     479,   479
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
       1,     1,     1,     1,     3,     2,     2,     1,     8,     6,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   428,     0,     0,   819,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   910,
       0,   898,   699,     0,   705,   706,   707,    25,   765,   886,
     887,   152,   153,   708,     0,   133,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   187,     0,     0,     0,     0,
       0,     0,   394,   395,   396,   399,   398,   397,     0,     0,
       0,     0,   216,     0,     0,     0,     0,   712,   714,   715,
     709,   710,     0,     0,     0,   716,   711,     0,   683,    27,
      28,    29,    31,    30,     0,   713,     0,     0,     0,     0,
     717,   400,   531,     0,   151,   123,     0,   700,     0,     0,
       4,   113,   115,   764,     0,   682,     0,     6,   186,     7,
       9,     8,    10,     0,     0,   392,   441,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   439,   875,   876,   513,
     511,   512,   422,   516,     0,   421,   846,   684,   691,     0,
     767,   510,   391,   849,   850,   861,   440,     0,     0,   443,
     442,   847,   848,   845,   882,   885,   500,   766,    11,   399,
     398,   397,     0,     0,    31,     0,   113,   186,     0,   954,
     440,   953,     0,   951,   950,   515,     0,   429,   436,   434,
       0,     0,   482,   483,   484,   485,   509,   507,   506,   505,
     504,   503,   502,   501,   886,   708,   686,     0,     0,     0,
     974,   868,   684,     0,   685,   463,     0,   461,     0,   914,
       0,   774,   420,   695,   206,     0,   974,   419,   694,   689,
       0,   704,   685,   893,   894,   900,   892,   696,     0,     0,
     698,   508,     0,     0,     0,     0,   425,     0,   131,   427,
       0,     0,   137,   139,     0,     0,   141,     0,    72,    71,
      66,    65,    57,    58,    49,    69,    80,    81,     0,    52,
       0,    64,    56,    62,    83,    75,    74,    47,    70,    90,
      91,    48,    86,    45,    87,    46,    88,    44,    92,    79,
      84,    89,    76,    77,    51,    78,    82,    43,    73,    59,
      93,    67,    60,    50,    42,    41,    40,    39,    38,    37,
      61,    94,    97,    54,    35,    36,    63,  1010,  1011,  1012,
      55,  1017,    34,    53,    85,     0,     0,   113,    96,   965,
    1009,     0,  1013,     0,     0,   143,     0,     0,     0,   177,
       0,     0,     0,     0,     0,     0,   776,     0,   101,   103,
     305,     0,     0,   304,     0,   220,     0,   217,   310,     0,
       0,     0,     0,     0,   971,   202,   214,   906,   910,   550,
     296,     0,   935,     0,   719,     0,     0,     0,   933,     0,
      16,     0,   117,   194,   208,   215,   586,   543,     0,   959,
     523,   525,   527,   823,   428,   441,     0,     0,   439,   440,
     442,     0,     0,   889,   701,     0,   702,     0,     0,     0,
     176,     0,     0,   119,   296,     0,    24,   185,     0,   213,
     198,   212,   397,   400,   186,   393,   166,   167,   168,   169,
     170,   172,   173,   175,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   898,     0,   165,   891,   891,   920,     0,     0,
       0,     0,     0,     0,     0,     0,   390,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     462,   460,   824,   825,     0,   891,     0,   837,   296,   296,
     891,     0,   906,     0,   186,     0,     0,   145,     0,   821,
     816,   774,     0,   441,   439,     0,   918,     0,   548,   773,
     909,   704,   441,   439,   440,   119,     0,   296,   418,     0,
     839,   697,     0,   123,   256,     0,   530,     0,   148,     0,
       0,   426,     0,     0,     0,     0,     0,   140,   164,   142,
    1010,  1011,  1012,  1007,  1008,     0,  1015,  1016,  1000,     0,
       0,     0,     0,    68,    95,    33,    55,    32,   966,   171,
     174,   144,   123,     0,   161,   163,     0,     0,     0,     0,
     104,     0,   775,   102,    18,     0,    98,     0,   306,     0,
     146,   219,   218,     0,     0,   147,   955,     0,     0,   441,
     439,   440,   443,   442,     0,   993,   226,     0,   907,     0,
       0,     0,     0,   774,     0,     0,     0,   776,   297,   149,
       0,     0,   718,   934,   765,     0,     0,   932,   770,   931,
     116,     5,    13,    14,     0,   224,     0,     0,   536,     0,
       0,   774,     0,     0,   692,   687,   537,     0,     0,     0,
       0,   823,   123,     0,   776,   822,  1021,   417,   431,   496,
     855,   874,   128,   122,   124,   125,   126,   127,   391,     0,
     514,   768,   769,   114,   774,     0,   975,     0,     0,   519,
     188,   222,     0,   466,   468,   467,   479,     0,     0,   499,
     464,   465,   469,   471,   470,   487,   486,   489,   488,   490,
     492,   494,   493,   491,   481,   480,   473,   474,   472,   475,
     476,   478,   495,   477,   890,     0,     0,   924,     0,   774,
     958,     0,   957,   974,   852,   204,   196,   210,     0,   959,
     200,   186,     0,   432,   435,   437,   445,   459,   458,   457,
     456,   455,   454,   453,   452,   451,   450,   449,   448,   827,
       0,   826,   829,   851,   833,   974,   830,     0,     0,     0,
       0,     0,     0,     0,     0,   952,   430,   814,   818,   773,
     820,     0,   688,     0,   913,     0,   912,   222,     0,   688,
     897,   896,   882,   885,     0,     0,   826,   829,   895,   830,
     423,   258,   260,   123,   534,   533,   424,     0,   123,   240,
     132,   427,     0,     0,     0,     0,     0,   252,   252,   138,
     774,     0,     0,     0,   998,   774,     0,   981,     0,     0,
       0,     0,     0,   772,     0,     0,   683,     0,     0,   721,
     682,   725,   727,     0,   720,   121,   726,   974,  1014,     0,
       0,     0,     0,    19,     0,    20,     0,    99,     0,     0,
       0,   110,   776,     0,   108,   103,   100,   105,     0,   303,
     311,   308,     0,     0,   944,   949,   946,   945,   948,   947,
      12,   991,   992,     0,   774,     0,     0,     0,   906,   903,
       0,   547,     0,   563,   773,   549,   298,   299,   681,   775,
     295,   943,   942,   941,     0,   937,     0,   938,   940,     0,
       5,     0,     0,     0,   580,   581,   589,   588,     0,   439,
       0,   773,   542,   546,     0,     0,   960,     0,   524,     0,
       0,   982,   823,   282,  1020,     0,     0,   838,     0,   888,
     773,   977,   973,   680,     0,   823,     0,     0,   224,   521,
     190,   498,     0,   568,   569,     0,   566,   773,   919,     0,
       0,   296,   226,     0,   224,     0,     0,   222,     0,   898,
     446,     0,     0,   835,   836,   853,   854,   883,   884,     0,
       0,     0,   802,   781,   782,   783,   790,     0,     0,     0,
       0,   794,   800,   792,   793,   808,   774,     0,   816,   917,
     916,     0,   224,     0,   840,   703,     0,   262,     0,     0,
     129,     0,     0,     0,     0,     0,     0,     0,   232,   233,
     244,     0,   123,   242,   158,   252,     0,   252,     0,   773,
       0,     0,     0,     0,   773,   999,  1001,   980,   774,   979,
       0,   774,   748,   749,   746,   747,   780,     0,   774,   772,
     556,     0,   545,     0,     0,   926,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1004,   178,     0,   181,   162,     0,
       0,   106,   111,   112,   104,   775,   109,     0,   307,     0,
     956,   150,   972,   993,   986,   988,   225,   227,   317,     0,
       0,   904,     0,     0,   552,     0,     0,     0,   300,   936,
       0,    17,     0,   959,   223,   317,     0,     0,   688,   539,
       0,   693,   961,     0,   982,   528,     0,     0,  1021,     0,
     287,   285,   829,   841,   974,   829,   842,   976,   120,     0,
     823,   221,     0,   823,     0,   497,   923,   922,     0,   296,
       0,     0,     0,     0,     0,     0,   224,   192,   704,   828,
     296,     0,   786,   787,   788,   789,   795,   796,   806,     0,
     774,     0,   802,   560,     0,   785,   810,   773,   813,   815,
     817,     0,   911,     0,   828,     0,     0,     0,     0,   259,
     535,   134,     0,   427,   232,   234,   906,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   246,     0,  1005,     0,
     994,     0,   997,   773,     0,     0,     0,   723,   773,   771,
       0,     0,   774,     0,   762,     0,   774,     0,   728,   763,
     761,   930,     0,   774,   731,   733,   732,     0,     0,   729,
     730,   734,   736,   735,   751,   750,   753,   752,   754,   756,
     758,   757,   755,   744,   743,   738,   739,   737,   740,   741,
     742,   745,  1003,     0,   123,     0,     0,   107,    21,   309,
       0,     0,     0,   990,     0,   391,   908,   906,   433,   438,
     444,   554,     0,   302,   301,     0,    15,     0,   391,   592,
       0,     0,   594,   587,   590,     0,   585,     0,   963,     0,
     983,   532,     0,   288,     0,     0,   283,     0,   982,     0,
     317,     0,   823,     0,   296,     0,   880,   317,   959,   317,
     962,     0,     0,     0,   447,     0,     0,   798,   773,   801,
     791,     0,     0,   774,     0,   784,     0,     0,   774,   807,
     915,   317,     0,   123,     0,   255,   241,     0,     0,     0,
     231,   154,   245,     0,     0,   248,     0,   253,   254,   123,
     247,  1006,   995,     0,   978,     0,  1019,   779,   778,   722,
     564,   773,   555,     0,     0,   773,   544,   724,     0,   567,
     773,   925,   760,     0,     0,     0,    22,    23,   987,   984,
     985,   228,     0,     0,     0,   398,   389,     0,     0,     0,
     203,   316,   318,     0,   388,     0,     0,     0,   959,   391,
       0,     0,   551,   939,   313,   209,   583,     0,     0,   538,
     526,     0,   291,   281,     0,   284,   290,   296,   518,   982,
     391,   982,     0,   921,     0,   879,   391,     0,   391,   964,
     317,   823,   877,   805,   804,   797,   565,   773,   559,     0,
       0,   799,   773,   809,   391,   123,   261,   130,   135,   156,
     235,     0,   243,   249,   123,   251,   996,     0,     0,     0,
     558,   541,     0,   929,   928,   759,   123,   182,   989,     0,
       0,     0,   967,     0,     0,     0,   229,     0,   959,     0,
     354,   350,   356,   683,    31,     0,   344,     0,   349,   353,
     366,     0,   364,   369,     0,   368,     0,   367,     0,   186,
     320,     0,   322,     0,   323,   324,     0,     0,   905,   553,
       0,   584,   582,   593,   591,   292,     0,     0,   279,   289,
       0,     0,   982,     0,   199,   518,   982,   881,   205,   313,
     211,   391,     0,     0,     0,   562,   812,     0,   207,   257,
       0,     0,   123,   238,   155,   250,  1018,   777,     0,     0,
       0,     0,     0,     0,   416,     0,   968,     0,   334,   338,
     413,   414,   348,     0,     0,     0,   329,   645,   644,   641,
     643,   642,   662,   664,   663,   633,   603,   605,   604,   623,
     639,   638,   599,   610,   611,   613,   612,   632,   616,   614,
     615,   617,   618,   619,   620,   621,   622,   624,   625,   626,
     627,   628,   629,   631,   630,   600,   601,   602,   606,   607,
     609,   647,   648,   657,   656,   655,   654,   653,   652,   640,
     659,   649,   650,   651,   634,   635,   636,   637,   660,   661,
     679,   665,   667,   666,   668,   669,   646,   671,   670,   673,
     675,   674,   608,   678,   676,   677,   672,   658,   598,   361,
     595,     0,   330,   382,   383,   381,   374,     0,   375,   331,
     408,     0,     0,     0,     0,   412,     0,   186,   195,   312,
       0,     0,     0,   280,   294,   878,     0,     0,   384,   123,
     189,   982,     0,     0,   201,   982,   803,     0,     0,   123,
     236,   136,   157,     0,   557,   540,   927,   180,   332,   333,
     411,   230,     0,   774,   774,     0,   357,   345,     0,     0,
       0,   363,   365,     0,     0,   370,   377,   378,   376,     0,
       0,   319,   969,     0,     0,     0,   415,     0,   314,     0,
     293,     0,   578,   776,   123,     0,     0,   191,   197,     0,
     561,   811,     0,     0,   159,   335,   113,     0,   336,   337,
       0,   773,     0,   773,   359,   355,   360,   596,   597,     0,
     346,   379,   380,   372,   373,   371,   409,   406,   993,   325,
     321,   410,     0,   315,   579,   775,     0,     0,   385,   123,
     193,     0,   239,     0,   184,     0,   391,     0,   351,   358,
     362,     0,     0,   823,   327,     0,   576,   517,   520,     0,
     237,     0,     0,   160,   342,     0,   390,   352,   407,   970,
       0,   776,   402,   823,   577,   522,     0,   183,     0,     0,
     341,   982,   823,   266,   403,   404,   405,  1021,   401,     0,
       0,     0,   340,     0,   402,     0,   982,     0,   339,   386,
     123,   326,  1021,     0,   271,   269,     0,   123,     0,     0,
     272,     0,     0,   267,   328,     0,   387,     0,   275,   265,
       0,   268,   274,   179,   276,     0,     0,   263,   273,     0,
     264,   278,   277
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   110,   900,   631,   176,  1482,   723,
     345,   346,   347,   348,   852,   853,   854,   112,   113,   114,
     115,   116,   401,   663,   664,   540,   247,  1550,   546,  1461,
    1551,  1794,   841,   340,   572,  1754,  1078,  1264,  1813,   418,
     177,   665,   936,  1144,  1323,   120,   634,   953,   666,   681,
     957,   606,   952,   227,   521,   667,   635,   954,   420,   365,
     384,   123,   938,   903,   877,  1096,  1485,  1198,  1008,  1701,
    1554,   800,  1014,   545,   809,  1016,  1359,   792,   997,  1000,
    1187,  1820,  1821,   653,   654,   616,   617,   352,   353,   359,
    1520,  1679,  1680,  1275,  1401,  1508,  1673,  1803,  1823,  1712,
    1758,  1759,  1760,  1495,  1496,  1497,  1498,  1714,  1715,  1721,
    1770,  1501,  1502,  1506,  1666,  1667,  1668,  1690,  1851,  1402,
    1403,   178,   125,  1837,  1838,  1671,  1405,  1406,  1407,  1408,
     126,   240,   541,   542,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,  1532,   137,   935,  1143,   138,   650,
     651,   652,   244,   393,   536,   640,   641,  1226,   642,  1227,
     139,   140,   612,   613,  1221,  1222,  1332,  1333,   141,   831,
     982,   142,   832,   143,   144,  1743,   145,   636,  1522,   637,
    1116,   908,  1296,  1293,  1659,  1660,   146,   147,   148,   230,
     149,   231,   241,   405,   528,   150,  1036,   836,   151,  1037,
     931,   583,  1038,   983,  1166,   984,  1168,  1169,  1170,   986,
    1337,  1338,   987,   768,   511,   190,   191,   668,   656,   494,
    1132,  1133,   754,   755,   927,   153,   233,   154,   155,   180,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   715,
     237,   238,   609,   220,   221,   718,   719,  1232,  1233,   377,
     378,   894,   166,   597,   167,   649,   168,   331,  1681,  1733,
     366,   413,   674,   675,  1030,  1127,  1273,   873,   874,   875,
     814,   815,   816,   332,   333,   838,   556,  1484,   925
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1544
static const yytype_int16 yypact[] =
{
   -1544,   182, -1544, -1544,  5527, 13645, 13645,   189, 13645, 13645,
   13645, 11071, 13645, 13645, -1544, 13645, 13645, 13645, 13645, 13645,
   13645, 13645, 13645, 13645, 13645, 13645, 13645, 16520, 16520, 11269,
   13645, 17181,   227,   229, -1544, -1544, -1544, -1544, -1544,   172,
   -1544, -1544, -1544,   145, 13645, -1544,   229,   237,   278,   332,
   -1544,   229, 11467,   950, 11665, -1544, 14676, 10081,    -2, 13645,
    1337,    26, -1544, -1544, -1544,   260,   506,    74,   342,   368,
     392,   401, -1544,   950,   417,   420,    64, -1544, -1544, -1544,
   -1544, -1544, 13645,   673,   542, -1544, -1544,   950, -1544, -1544,
   -1544, -1544,   950, -1544,   950, -1544,   262,   449,   950,   950,
   -1544,   436, -1544, 11863, -1544, -1544,   317,   543,   629,   629,
   -1544,   167,    68,   481,   453, -1544,    97, -1544,   194, -1544,
   -1544, -1544, -1544,  1616,   851, -1544, -1544,   454,   470,   474,
     480,   484,   527,   541,   546, 11451, -1544, -1544, -1544, -1544,
     157,   616, -1544,   671,   691, -1544,    75,   571, -1544,   612,
       3, -1544,  2587,   180, -1544, -1544,  2580,   123,   579,   306,
   -1544,   166,   108,   588,   314, -1544, -1544,   711, -1544, -1544,
   -1544,   644,   620,   682, -1544, 13645, -1544,   194,   851, 17658,
    2716, 17658, 13645, 17658, 17658, 14042,   638, 15983, 14042, 17658,
     800,   950,   781,   781,   545,   781,   781,   781,   781,   781,
     781,   781,   781,   781, -1544, -1544, -1544,   663,    76, 13645,
     683, -1544, -1544,   706,   678,   383,   685,   383, 16520, 16102,
     675,   865, -1544,   644, -1544, 13645,   683, -1544,   719, -1544,
     722,   690, -1544,   179, -1544, -1544, -1544,   383,   123, 12061,
   -1544, -1544, 13645,  8893,   872,   106, 17658,  9883, -1544, 13645,
   13645,   950, -1544, -1544, 12045,   692, -1544, 13629, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,  4383, -1544,
    4383, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544,    81,   104,   103,
     682, -1544, -1544, -1544, -1544,   695,  2321,   105, -1544, -1544,
     727,   873, -1544,   732, 15518, -1544,   697,   700, 13827, -1544,
       1, 15358,  1721,  1721,   950,   702,   885,   707, -1544,    58,
   -1544, 16124,   112, -1544,   767, -1544,   771, -1544,   890,   113,
   16520, 13645, 13645,   714,   730, -1544, -1544, 16223, 11269, 13645,
   10279,   114,   534,   464, -1544, 13843, 16520,   684, -1544,   950,
   -1544,   209,    68, -1544, -1544, -1544, -1544, 17277,   895,   809,
   -1544, -1544, -1544,    52, 13645,   721,   725, 17658,   728,  2153,
     729,  5725, 13645, -1544,    71,   717,   681,    71,   516,   511,
   -1544,   950,  4383,   734, 10279, 14676, -1544, -1544,   704, -1544,
   -1544, -1544, -1544, -1544,   194, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, 13645, 13645, 13645, 13645, 12259, 13645,
   13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645,
   13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645,
   13645, 13645, 17373, 13645, -1544, 13645, 13645, 13645, 14041,   950,
     950,   950,   950,   950,  1616,   808,   605,  4722, 13645, 13645,
   13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645, 13645,
   -1544, -1544, -1544, -1544,  1192, 13645, 13645, -1544, 10279, 10279,
   13645, 13645, 16223,   735,   194, 12457, 15406, -1544, 13645, -1544,
     740,   919,   780,   743,   751, 14184,   383, 12655, -1544, 12853,
   -1544,   690,   752,   756,  2212, -1544,    90, 10279, -1544,  1299,
   -1544, -1544, 15454, -1544, -1544, 10477, -1544, 13645, -1544,   856,
    9091,   942,   759, 17537,   940,    87,    65, -1544, -1544, -1544,
     783,   783, -1544, -1544, -1544,  4383, -1544, -1544,   493,   770,
     954, 16025,   950, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544,   774, -1544, -1544,   782,   775,   784,   786,
     435,  2165,  1834, -1544, -1544,   950,   950, 13645,   383,    26,
   -1544, -1544, -1544, 16025,   897, -1544,   383,   139,   147,   789,
     790,  2291,   187,   792,   795,   515,   853,   799,   383,   149,
     801, 16684,   793,   983, 16520, 13645,   804,   988, 17658, -1544,
    1477,   950, -1544, -1544,   930,  2948,   232, -1544, -1544, -1544,
      68, -1544, -1544, -1544,   969,   874,   830,   211,   854, 13645,
     878,  1000,   824,   862, -1544,   200, -1544,  4383,  4383,  1005,
     872,    52, -1544,   831,  1012, -1544,  4383,    62, -1544,   354,
     197, -1544, -1544, -1544, -1544, -1544, -1544, -1544,  1923,  3392,
   -1544, -1544, -1544, -1544,  1013,   845, -1544,   835,  1015, -1544,
   -1544,   908,  1017, 11650, 17796, 14042, 14503, 13645, 17610, 14675,
   11248,  4593,  4552,  3225, 12235, 13026, 13026, 13026, 13026,  3940,
    3940,  3940,  3940,  3940,  1557,  1557,   693,   693,   693,   545,
     545,   545, -1544,   781, 17658,   848,   849, 16732,   857,  1039,
      -1, 13645,   369,   683,   168, -1544, -1544, -1544,  1035,   809,
   -1544,   194, 16322, -1544, -1544, -1544, 14042, 14042, 14042, 14042,
   14042, 14042, 14042, 14042, 14042, 14042, 14042, 14042, 14042, -1544,
   13645,   378, -1544,   202, -1544,   683,   421,   859,  3555,   863,
     868,   864,  3783,   152,   870, -1544, 17658,  3157, -1544,   950,
   -1544,    62,   587, 16520, 17658, 16520, 16788,   908,    62,   383,
     225, -1544,   200,   898,   871, 13645, -1544,   298, -1544, -1544,
   -1544,  8695,   110, -1544, -1544, 17658, 17658,   229, -1544, -1544,
   -1544, 13645,   962,  3573, 16025,   950,  9289,   875,   876, -1544,
    1061,   980,   938,   920, -1544,  1066,   886,  1204,  4383, 16025,
   16025, 16025, 16025, 16025,   889,  1007,   926,   892, 16025,   561,
     928, -1544, -1544,   893, -1544, 17752, -1544,    42, -1544,  5923,
    1369,   896,  1834, -1544,  1834, -1544,   950,   950,  1834,  1834,
     950, -1544,  1077,   894, -1544,   440, -1544, -1544,  3897, -1544,
   17752,  1079, 16520,   903, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544,   921,  1091,   950,  1369,   907, 16223, 16421,
    1089, -1544, 13051, -1544, 13645, -1544,   383, 17658, -1544, 10675,
   -1544, -1544, -1544, -1544,   906, -1544, 13645, -1544, -1544,  5060,
   -1544,  4383,  1369,   912, -1544, -1544, -1544, -1544,  1094,   923,
   13645, 17277, -1544, -1544, 14041,   929, -1544,  4383, -1544,   932,
    6121,  1092,    51, -1544, -1544,   131,  1192, -1544,  1299, -1544,
    4383, -1544, -1544, -1544, 16025,    55,   936,  1369,   874, -1544,
   -1544, 14675, 13645, -1544, -1544, 13645, -1544, 13645, -1544,  4124,
     939, 10279,   853,  1100,   874,  4383,  1109,   908,   950, 17373,
     383,  4315,   946, -1544, -1544,   201,   947, -1544, -1544,  1119,
    1408,  1408,  3157, -1544, -1544, -1544,  1090,   956,  1080,    82,
     960, -1544, -1544, -1544, -1544, -1544,  1144,   965,   740,   383,
     383, 13249,   874,  1299, -1544, -1544,  4934,   625,   229,  9883,
   -1544,  6319,   961,  6517,   966,  3573, 16520,   967,  1024,   383,
   17752,  1159, -1544, -1544, -1544, -1544,   456, -1544,   414,  4383,
     987,  1040,  4383,   950,   493, -1544, -1544, -1544,  1168, -1544,
     994,  1013,   761,   761,  1110,  1110, 16940,   989,  1180, 16025,
   16025, 15804, 17277,  2507, 15661, 16025, 16025, 16025, 16025, 15906,
   16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025,
   16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025, 16025,
   16025, 16025, 16025,   950, -1544, -1544,  1111, -1544, -1544,   998,
    1001, -1544, -1544, -1544,   447,  2165, -1544,  1002, -1544, 16025,
     383, -1544, -1544,    85, -1544,   578,  1188, -1544, -1544,   153,
    1009,   383, 10873, 16520, 17658, 16836, 16520, 13645, 17658, -1544,
    2778, -1544,  5329,   809,  1188, -1544,   223,   199, -1544, 17658,
    1073,  1020, -1544,  1021,  1092, -1544,  4383,   872,  4383,    47,
    1201,  1134,   305, -1544,   683,   312, -1544, -1544, 17752,  1026,
      55, -1544,  1022,    55,  1031, 14675, 17658, 16892,  1033, 10279,
    1034,  1037,  4383,  1038,  1036,  4383,   874, -1544,   690,   455,
   10279, 13645, -1544, -1544, -1544, -1544, -1544, -1544,  1097,  1029,
    1225,  1148,  3157,  3157,  1093, -1544, 17277,  3157, -1544, -1544,
   -1544, 16520, 17658,  1048, -1544,   229,  1212,  1170,  9883, -1544,
   -1544, -1544,  1052, 13645,  1024,   383, 16223,  3573,  1056, 16025,
    6715,   646,  1058, 13645,    67,   434, -1544,  1071, -1544,  4383,
   -1544,  1123, -1544,  4345,  1228,  1070, 16025, -1544, 16025, -1544,
    1075,  1067,  1259, 16995, -1544,  1135,  1260,  1078, -1544, -1544,
   -1544, 17043,  1083,  1263, 10259, 10655,  2876, 16025, 17706, 12436,
   12633, 12830,  3476, 13224, 13422, 13422, 13422, 13422,  2209,  2209,
    2209,  2209,  2209,  1151,  1151,   761,   761,   761,  1110,  1110,
    1110,  1110, -1544,  1087, -1544,  1088,  1095, -1544, -1544, 17752,
     950,  4383,  4383, -1544,  1369,    88, -1544, 16223, -1544, -1544,
   14042,   383, 13447,   383, 17658,  1096, -1544,  1102,  1635, -1544,
     412, 13645, -1544, -1544, -1544, 13645, -1544, 13645, -1544,   872,
   -1544, -1544,   133,  1270,  1206, 13645, -1544,  1103,  1092,  1105,
   -1544,  1112,    55, 13645, 10279,  1113, -1544, -1544,   809, -1544,
   -1544,  1108,  1116,  1115, -1544,  1121,  3157, -1544,  3157, -1544,
   -1544,  1122,  1104,  1294,  1181, -1544,  1182,  1130,  1315, -1544,
     383, -1544,  1295, -1544,  1136, -1544, -1544,  1138,  1139,   154,
   -1544, -1544, 17752,  1140,  1141, -1544, 11055, -1544, -1544, -1544,
   -1544, -1544, -1544,  4383, -1544,  4383, -1544, 17752, 17098, -1544,
   -1544, 16025, -1544, 16025, 16025, 17277, -1544, -1544, 16025, -1544,
   16025, -1544,  2825, 16025,  1142,  6913, -1544, -1544,   578, -1544,
   -1544, -1544,   598, 14848,  1369,  1229, -1544,  1927,  1172,  1266,
   -1544, -1544, -1544,   808,  3028,   116,   117,  1147,   809,   605,
     155, 16520, 17658, -1544, -1544, -1544,  1184,  4990,  5259, 17658,
   -1544,    69,  1332,  1265, 13645, -1544, 17658, 10279,  1232,  1092,
    1829,  1092,  1158, 17658,  1160, -1544,  1854,  1163,  1938, -1544,
   -1544,    55, -1544, -1544,  1226, -1544, -1544,  3157, -1544,  3157,
    3157, -1544, 17277, -1544,  2023, -1544,  8695, -1544, -1544, -1544,
   -1544,  9487, -1544, -1544, -1544,  8695, -1544,  1174, 16025, 17146,
   17752, 17752,  1237, 17752, 17201,  2825, -1544, -1544, -1544,  1369,
    1369,   950, -1544,  1361, 15804,    94, -1544, 14848,   809,  2369,
   -1544,  1200, -1544,   118,  1187,   122, -1544, 15193, -1544, -1544,
   -1544,   132, -1544, -1544,   656, -1544,  1185, -1544,  1301,   194,
   -1544, 15021, -1544, 15021, -1544, -1544,  1376,   808, -1544,   383,
   14332, -1544, -1544, -1544, -1544,  1377,  1316, 13645, -1544, 17658,
    1209,  1207,  1092,   582, -1544,  1232,  1092, -1544, -1544, -1544,
   -1544,  2079,  1211,  3157,  1268, -1544, -1544,  1269, -1544,  8695,
    9685,  9487, -1544, -1544, -1544,  8695, -1544, 17752, 16025, 16025,
   16025,  7111,  1213,  1214, -1544, 16025, -1544,  1369, -1544, -1544,
   -1544, -1544, -1544,  4383,  1648,  1927, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,   170,
   -1544,  1172, -1544, -1544, -1544, -1544, -1544,   129,   540, -1544,
    1395,   134, 15518,  1301,  1397, -1544,  4383,   194, -1544, -1544,
    1219,  1400, 13645, -1544, 17658, -1544,   150,  1222, -1544, -1544,
   -1544,  1092,   582, 14504, -1544,  1092, -1544,  3157,  3157, -1544,
   -1544, -1544, -1544,  7309, 17752, 17752, 17752, -1544, -1544, -1544,
   17752, -1544,  1004,  1412,  1414,  1230, -1544, -1544, 16025, 15193,
   15193,  1358, -1544,   656,   656,   654, -1544, -1544, -1544, 16025,
    1341, -1544,  1251,  1240,   135, 16025, -1544,   950, -1544, 16025,
   17658,  1352, -1544,  1427, -1544,  7507,  1244, -1544, -1544,   582,
   -1544, -1544,  7705,  1246,  1326, -1544,  1342,  1289, -1544, -1544,
    1344,  4383,  1273,  1648, -1544, -1544, 17752, -1544, -1544,  1284,
   -1544,  1418, -1544, -1544, -1544, -1544, 17752,  1441,   515, -1544,
   -1544, 17752,  1267, 17752, -1544,   339,  1264,  7903, -1544, -1544,
   -1544,  1276, -1544,  1279,  1297,   950,   605,  1296, -1544, -1544,
   -1544, 16025,  1298,    56, -1544,  1392, -1544, -1544, -1544,  8101,
   -1544,  1369,   896, -1544,  1308,   950,  1133, -1544, 17752, -1544,
    1290,  1472,   689,    56, -1544, -1544,  1401, -1544,  1369,  1303,
   -1544,  1092,    61, -1544, -1544, -1544, -1544,  4383, -1544,  1300,
    1302,   137, -1544,   593,   689,   163,  1092,  1305, -1544, -1544,
   -1544, -1544,  4383,   251,  1488,  1423,   593, -1544,  8299,   185,
    1493,  1430, 13645, -1544, -1544,  8497, -1544,   255,  1500,  1432,
   13645, -1544, 17658, -1544,  1502,  1434, 13645, -1544, 17658, 13645,
   -1544, 17658, 17658
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1544, -1544, -1544,  -547, -1544, -1544, -1544,   220,   -11,   -34,
     510, -1544,  -263,  -513, -1544, -1544,   406,     7,  1569, -1544,
    2827, -1544,  -461, -1544,    37, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544,  -294, -1544, -1544,  -146,
     138,    19, -1544, -1544, -1544, -1544, -1544, -1544,    31, -1544,
   -1544, -1544, -1544, -1544, -1544,    33, -1544, -1544,  1045,  1051,
    1050,   -90,  -649,  -852,   572,   623,  -301,   334,  -912, -1544,
     -13, -1544, -1544, -1544, -1544,  -723,   183, -1544, -1544, -1544,
   -1544,  -281, -1544,  -609, -1544,  -348, -1544, -1544,   955, -1544,
       6, -1544, -1544, -1028, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544,   -25, -1544,    63, -1544, -1544, -1544,
   -1544, -1544,  -105, -1544,   158,  -934, -1544, -1543,  -297, -1544,
    -143,   125,  -122,  -279, -1544,  -104, -1544, -1544, -1544,   173,
     -22,    -3,    38,  -725,   -67, -1544, -1544,    21, -1544,     8,
   -1544, -1544,    -5,   -42,    50, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544,  -612,  -847, -1544, -1544, -1544, -1544,
   -1544,   705, -1544, -1544, -1544, -1544, -1544, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544, -1544,   457, -1544, -1544, -1544,
   -1544, -1544, -1544, -1544, -1544,  -861, -1544,  2295,    39, -1544,
    1661,  -402, -1544, -1544,  -462,  3197,  3635, -1544, -1544,   538,
     -97,  -591, -1544, -1544,   607,   411,  -586,   419, -1544, -1544,
   -1544, -1544, -1544,   601, -1544, -1544, -1544,    23,  -876,  -130,
    -441,  -421, -1544,   660,   -99, -1544, -1544,    40,    43,   548,
   -1544, -1544,   437,   -20, -1544,  -347,    24,  -358,    41,   120,
   -1544, -1544,  -452,  1224, -1544, -1544, -1544, -1544, -1544,   737,
     635, -1544, -1544, -1544,  -343,  -654, -1544,  1178,  -835, -1544,
     -66,  -175,    48,   778, -1544,  -893,   210,  -179, -1544,   508,
     580, -1544, -1544, -1544, -1544,   529,  -229,   357, -1094
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1003
static const yytype_int16 yytable[] =
{
     179,   181,   425,   183,   184,   185,   187,   188,   189,   475,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   329,   119,   219,   222,   890,   152,   385,   645,
     243,   503,   388,   389,  1302,   121,   396,   122,   918,   246,
     644,   117,   919,   248,   646,   328,  1128,   254,   252,   257,
     763,   525,   338,   752,   341,   236,   425,   421,   497,   777,
     712,   398,   474,   923,  1120,   337,   677,   349,   851,   856,
     229,   234,   791,   753,   235,   956,  1004,   246,   336,   577,
     579,   395,   245,   400,   899,  1018,  1142,  1288,   787,   557,
     -68,   381,  1392,  1194,   382,   -68,   805,   807,   397,  1357,
      14,    14,  1153,  1567,    14,    14,   415,   529,   788,   350,
      14,   839,   -33,   -95,   -32,   537,   573,   -33,   -95,   -32,
     371,   589,   594,   537,   520,  1511,  1513,  -347,   992,   124,
    1303,  1575,   495,   398,   369,   871,   872,    14,  1723,   530,
    1183,  1661,   118,  1730,  1730,  -856,  1567,   403,   862,  1747,
     759,   760,  1525,   395,   372,   400,   537,   358,   879,  -685,
     585,   879,   879,   879,   879,  1724,   512,   514,   782,  1129,
     397,  1421,  1174,   204,    40,   574,   -97,   506,  -858,   784,
    -529,   985,     3,   523,  1718,   998,   999,   513,  1741,   400,
     -97,   920,   339,   495,  -869,  1225,   492,   493,  1073,   410,
    1393,  1853,  1719,   522,   397,  1394,  1790,    62,    63,    64,
     169,  1395,   422,  1396,  1130,  -574,  1422,  1304,   586,   412,
     397,  1720,   351,  1867,   111,   375,   376,  -570,   411,   374,
    -686,  1299,   417,  1742,   532,  1175,  -857,   532,  -864,  1526,
    -775,  -286,   621,  -775,   246,   543,  1854,  -286,  -270,  -899,
    -859,  1397,  1398,  -775,  1399,   370,   496,  -573,   555,   808,
    1358,  1086,   424,  -773,  -692,   905,  -867,  -863,  1868,  -856,
     500,  -862,  -860,   255,   -68,   423,   327,   476,   682,   806,
     534,   555,  1430,  1400,   539,  1350,   106,  1568,  1569,  1436,
     416,  1438,  1201,   364,  1205,  -902,   -33,   -95,   -32,   538,
     568,  1131,  -858,  1423,  1322,   590,   595,   619,  1156,  1512,
    1514,  -347,   383,  1454,   364,  1576,   504,   496,   364,   364,
     600,  1725,   557,   328,  -693,  1662,  1139,  1731,  1780,  1336,
    1848,   863,  1001,  1855,  1860,  1082,  1083,  1003,  1874,   864,
     599,   880,   603,   364,   969,  1276,  1460,  1518,  -868,   349,
     349,   580,   680,  1112,   425,  1869,   246,   397,   764,   951,
    -857,  1294,  -864,   219,   611,   618,   492,   493,  -901,   411,
     625,  -866,   906,  -899,  -859,  -843,  -573,  1805,   499,   868,
     182,   329,  -844,   354,   500,  1289,   630,   907,  -870,   187,
     355,  -863,  -873,  1295,   501,  -862,  -860,   669,  1290,   598,
     204,    40,   632,   633,   328,   385,   728,   729,   421,   618,
     733,   510,  1541,   334,   770,  1428,   655,  1291,   239,  -902,
     242,  1861,  1806,   386,  -575,  1875,  1099,   898,   249,   683,
     684,   685,   686,   688,   689,   690,   691,   692,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   722,   713,  1287,
     714,   714,   717,   111,   216,   216,  1416,   111,  1347,   250,
    -687,   544,   736,   737,   738,   739,   740,   741,   742,   743,
     744,   745,   746,   747,   748,   735,   236,  1135,   492,   493,
     714,   758,  -901,   618,   618,   714,   762,   499,   734,  -843,
     736,   229,   234,   766,   328,   235,  -844,  1136,   501,   724,
    -688,   402,   774,   106,   776,  1301,   885,   492,   493,   390,
    1203,  1204,   618,   251,   811,   475,   124,   926,  1472,   928,
     795,  1309,   796,   360,  1311,   756,  1533,   846,  1535,   118,
    1203,  1204,   585,   794,   912,  -871,   -96,   372,   950,   846,
     781,  1200,   156,   645,   567,  -831,   724,  1158,  1483,   361,
     -96,  1202,  1203,  1204,   644,   871,   872,   783,   646,  -831,
     789,  1726,   851,   386,    37,   215,   217,   799,   474,  1079,
     962,  1080,   858,   362,   812,   958,   716,  1334,   855,   855,
    1727,  1339,   363,  1728,   372,   847,    50,   909,  -834,   372,
     411,   627,   462,  1150,   620,  1547,   627,   411,   367,  1206,
     887,   368,  -834,   731,   463,   757,   940,   622,   375,   376,
     761,   111,   948,    37,   391,   553,   372,   554,   356,  1360,
     392,   673,  -832,   404,   397,   327,   357,  -974,   364,  1687,
     387,   926,   928,  1692,   414,    50,  -832,   426,   993,   928,
     173,   399,  1570,    87,   525,   216,    89,    90,   412,    91,
     174,    93,  1074,   427,  1437,   375,   376,   428,   492,   493,
     375,   376,  -974,   429,   655,  -974,  1674,   430,  1675,   994,
    1271,  1272,   941,   560,   645,  1773,  -571,  1420,   567,   364,
     726,   364,   364,   364,   364,   644,  1324,   375,   376,   646,
    1185,  1186,   379,  1432,  1774,    89,    90,  1775,    91,   174,
      93,   672,   372,  1020,   751,   671,   949,  -974,  1025,   407,
     431,   492,   493,   399,    62,    63,    64,   169,   170,   422,
    1479,  1480,   211,   211,   432,   567,   380,    37,   412,   433,
    1443,   465,  1444,  1845,  1349,   961,   459,   460,   461,   786,
     462,  1354,  1203,  1204,  1516,  -974,   372,   399,  1859,    50,
     111,   466,   463,   373,   372,   467,   516,   372,   468,   676,
     498,   627,   334,   524,   627,  1688,  1689,  1094,  -871,  -865,
     996,  -572,   837,   375,   376,    37,  1849,  1850,   216,  1771,
    1772,   156,   423,   476,  1002,   156,   246,   216,  1746,   602,
    -686,  1315,  1749,  1385,   216,   857,   673,    50,  1834,  1835,
    1836,   502,  1325,   216,  1069,  1070,  1071,  1663,   645,    89,
      90,  1664,    91,   174,    93,  1410,   374,   375,   376,   644,
    1072,   507,  1542,   646,  1571,   375,   376,   628,   375,   376,
     893,   895,   379,  1013,   406,   408,   409,  1504,   509,   855,
     463,   855,   576,   578,   370,   855,   855,  1084,  1767,  1768,
     412,  1544,   515,  1545,  1546,  1028,  1031,    89,    90,  -869,
      91,   174,    93,   518,   519,  -684,   499,  1104,   526,  1105,
     535,   527,  1456, -1002,  1108,   548,   558,   561,   562,  1178,
     569,  1110,  1157,   570,   582,   679,   581,   591,  1465,   588,
     584,   592,   364,   722,   593,  1119,   604,   605,   596,   647,
     601,   648,   810,   657,   670,   608,   124,   658,   119,    55,
     659,   661,   152,   211,   626,  -118,   679,  1822,   769,   118,
     121,   767,   122,   622,  1215,   771,   117,  1145,  1843,   216,
    1146,  1219,  1147,   772,   778,   655,   618,  1822,   779,   156,
     797,   537,   801,  1856,   804,  1121,  1844,  1696,   655,  1307,
     555,   817,    55,   818,   124,   840,  1434,   756,   843,   789,
      62,    63,    64,   169,   170,   422,   842,   118,   844,   845,
     861,   865,   866,   236,   869,   876,  1182,   981,   870,   988,
     878,   883,   884,   881,  1549,  1188,   888,   889,   229,   234,
     896,   901,   235,  1555,   915,   916,   904,   902,   623,   911,
    -708,   111,   629,   924,   910,  1561,   913,   645,   914,   917,
     921,   922,   930,   932,   124,  1011,   111,   933,   644,   934,
    1224,    37,   646,  1230,   789,  1278,  1189,   118,   423,   623,
     937,   629,   623,   629,   629,   124,   943,   944,   947,   955,
     608,   216,   946,    50,  -690,   965,   211,   963,   118,   111,
     966,   939,   967,   995,  1005,   211,  1081,   673,  1015,  1017,
    1019,  1021,   211,  1329,  1022,  1024,  1023,  1040,  1026,  1530,
    1039,   211,  1041,  1042,  1044,    37,  1085,  1045,   156,  1087,
    1077,  1703,   855,  1089,   645,  1095,  1091,  1280,    37,  1092,
    1093,  1098,  1284,  1102,  1109,   644,  1115,    50,  1117,   646,
    1279,  1750,  1751,    89,    90,  1118,    91,   174,    93,   111,
      50,  1124,  1122,  1155,  1126,  1372,   124,  1140,   124,  1376,
    1149,   119,  1152,  1161,   567,   152,  1381,  1160,  -872,   118,
     111,   118,  1171,   121,   618,   122,   751,  1172,   786,   117,
    1173,  1176,  1786,  1177,  1191,   618,  1280,  1179,  1196,  1193,
    1197,   173,   886,   655,    87,  1208,   655,    89,    90,   216,
      91,   174,    93,  1199,   676,   676,  1209,  1213,   364,  1072,
      89,    90,  1342,    91,   174,    93,  1214,  1217,   246,  1218,
    1165,  1165,   981,  1265,  1263,  1268,  1266,  1274,  1356,  1755,
    1277,  1066,  1067,  1068,  1069,  1070,  1071,   211,   939,  1297,
     216,   951,   216,   786,  1298,  1305,  1310,  1306,  1308,   111,
    1072,   111,  1312,   111,  1314,  1345,  1316,  1327,  1745,  1320,
    1833,  1317,  1319,  1326,  1328,   268,  1448,   124,  1752,   976,
     216,  1453,  1341,  1211,  1343,  1346,  1335,  1344,  1351,  1361,
     118,  1355,    62,    63,    64,   169,   170,   422,  1113,  1363,
    1365,   567,  1366,   270,   567,  1370,  1517,  1369,  1371,  1375,
    1377,  1374,  1380,    37,  1123,   204,    40,  1412,  1379,  1384,
     960,  1386,   425,  1787,  1424,    37,  1417,  1137,  1387,  1425,
    1418,  1413,  1419,   837,  1427,    50,  1414,  1429,  1409,   216,
    1426,  1439,  1446,  1447,  1431,  1435,  1441,    50,  1433,   618,
    1440,  1409,  1154,  1442,  1445,   216,   216,  1449,  1450,   211,
     423,   989,  1451,   990,  1452,   124,  1830,  1455,  1809,  1457,
    1458,  1459,   111,  1462,  1463,   655,  1476,  1500,   118,   156,
    1515,  1487,   550,   551,   552,  1521,  1527,    37,  1528,  1531,
    1536,  1009,  1537,   749,   156,    89,    90,  1539,    91,   174,
      93,   173,  1543,  1672,    87,   321,  1556,    89,    90,    50,
      91,   174,    93,  1559,  1027,  1565,  1207,  1573,  1669,  1210,
      37,  1574,   204,    40,  1670,   325,   750,   156,   106,  1858,
    1676,  1682,   981,   981,  1503,   326,  1865,   981,  1686,  1683,
    1404,  1685,    50,  1695,  1697,  1698,  1708,  1709,   111,  1729,
    1090,  1735,  1738,  1404,  1739,  1564,  1744,  1769,    37,  1529,
     111,  1761,   618,  1763,  1777,  1765,   608,  1101,  1778,    89,
      90,  1779,    91,   174,    93,  1784,  1785,   211,  1789,  1792,
      50,  1793,   216,   216,  -343,  1795,  1796,   156,   342,   343,
      37,  1798,  1800,  1409,  1724,  1801,  1807,  1504,  1804,  1409,
     749,  1409,    89,    90,   655,    91,   174,    93,   156,  1810,
    1811,  1812,    50,  1566,  1817,  1824,  1819,  1409,   211,  1828,
     211,  1832,  1831,  1300,  1840,   924,  1162,  1163,  1164,    37,
    1388,  1267,  1846,   785,  1847,   106,  1842,   344,  1553,  1857,
      89,    90,  1862,    91,   174,    93,  1863,  1870,   211,  1318,
     124,    50,  1321,  1871,  1876,  1877,  1879,  1880,  1827,   730,
     725,   727,  1684,   118,  1151,  1114,   173,  1841,  1348,    87,
      88,  1737,    89,    90,   476,    91,   174,    93,  1702,  1464,
     216,  1509,  1839,   216,   859,  1693,   981,   156,   981,   156,
    1717,   156,  1572,  1009,  1195,  1404,  1722,  1507,    37,  1864,
     891,  1404,   892,  1404,  1409,  1852,  1362,   211,  1488,  1734,
    1137,    89,    90,  1292,    91,   174,    93,  1220,  1167,  1404,
      50,   124,  1330,   211,   211,  1691,  1134,  1700,  1553,  1180,
     124,  1331,   610,   678,   118,  1029,   210,   210,  1478,  1802,
     226,  1270,  1262,   118,  1212,   111,     0,   456,   457,   458,
     459,   460,   461,   327,   462,     0,  1762,  1764,   216,  1505,
       0,     0,     0,     0,     0,   226,   463,     0,  1389,  1390,
       0,     0,     0,   216,   216,     0,     0,     0,  1732,  1392,
      89,    90,     0,    91,   174,    93,     0,     0,     0,     0,
       0,  1281,     0,  1815,  1283,  1677,     0,     0,     0,     0,
     156,   328,     0,     0,     0,     0,  1404,   981,     0,   981,
     981,  1782,     0,     0,   124,     0,   111,  1740,     0,     0,
     124,   111,     0,     0,    14,   111,   124,   118,   213,   213,
       0,     0,     0,   118,   425,     0,     0,    37,     0,   118,
       0,   364,     0,     0,   567,     0,     0,   327,     0,     0,
     211,   211,     0,     0,   216,     0,     0,  1658,     0,    50,
    1466,     0,  1467,     0,  1665,     0,    34,    35,    36,  1340,
       0,   327,     0,   327,     0,     0,   156,     0,     0,   205,
     327,     0,     0,     0,   608,  1009,     0,  1393,   156,     0,
       0,     0,  1394,     0,    62,    63,    64,   169,  1395,   422,
    1396,  1510,     0,   981,     0,     0,     0,     0,     0,   111,
     111,   111,     0,     0,     0,   111,     0,   419,     0,    89,
      90,   111,    91,   174,    93,     0,     0,   210,     0,    77,
      78,    79,    80,    81,     0,     0,     0,     0,  1397,  1398,
     208,  1399,    37,     0,     0,     0,    85,    86,   211,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
      95,     0,   423,     0,    50,   608,   655,     0,   124,     0,
    1415,     0,     0,  1392,   100,     0,     0,   226,     0,   226,
       0,   118,     0,     0,     0,     0,   655,     0,   216,     0,
       0,     0,     0,     0,     0,   655,     0,  1872,  1392,     0,
       0,     0,     0,     0,     0,  1878,     0,     0,     0,     0,
     124,  1881,     0,     0,  1882,     0,     0,   124,    14,   213,
       0,   344,     0,   118,    89,    90,   211,    91,   174,    93,
     118,     0,   567,     0,     0,   226,     0,     0,     0,     0,
       0,   211,   211,    14,     0,     0,     0,     0,     0,     0,
       0,     0,   124,   327,     0,    37,     0,   981,   981,     0,
     210,  1816,     0,   111,     0,   118,     0,     0,     0,   210,
    1713,     0,  1756,   156,   124,     0,   210,    50,     0,  1658,
    1658,  1393,  1392,  1665,  1665,   210,  1394,   118,    62,    63,
      64,   169,  1395,   422,  1396,     0,   226,   364,  1489,  1519,
       0,     0,     0,     0,     0,   111,  1393,     0,     0,     0,
       0,  1394,   111,    62,    63,    64,   169,  1395,   422,  1396,
       0,   226,   211,   124,   226,     0,     0,    14,     0,     0,
     124,     0,  1397,  1398,   850,  1399,   118,    89,    90,     0,
      91,   174,    93,   118,   156,     0,     0,   111,    37,   156,
       0,     0,   213,   156,     0,  1814,   423,  1397,  1398,     0,
    1399,   213,     0,     0,  1534,     0,     0,  1392,   213,   111,
      50,   226,     0,  1736,     0,  1829,     0,   213,     0,     0,
       0,   423,    62,    63,    64,    65,    66,   422,   643,  1538,
    1393,     0,  1490,    72,   469,  1394,     0,    62,    63,    64,
     169,  1395,   422,  1396,     0,  1491,     0,  1492,     0,     0,
       0,   210,    14,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,  1392,   173,   111,     0,    87,  1493,     0,
      89,    90,   471,    91,  1494,    93,     0,   156,   156,   156,
       0,  1397,  1398,   156,  1399,     0,     0,     0,     0,   156,
     423,     0,     0,     0,     0,     0,   211,     0,  1797,     0,
       0,     0,     0,     0,   226,   423,     0,   226,    14,     0,
     829,     0,     0,  1540,     0,  1393,     0,     0,     0,     0,
    1394,     0,    62,    63,    64,   169,  1395,   422,  1396,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   829,   213,     0,     0,     0,   505,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
       0,     0,     0,   210,     0,     0,  1397,  1398,     0,  1399,
       0,  1393,     0,     0,   924,     0,  1394,     0,    62,    63,
      64,   169,  1395,   422,  1396,     0,     0,     0,     0,   924,
     423,     0,     0,     0,     0,     0,   226,   226,  1548,     0,
     490,   491,     0,     0,     0,   226,   505,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,     0,
       0,     0,  1397,  1398,     0,  1399,    37,     0,     0,     0,
       0,   156, -1003, -1003, -1003, -1003, -1003,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,     0,   423,     0,    50,     0,
       0,     0,     0,     0,  1694,   213,   848,   849,  1072,   490,
     491,     0,     0,     0,     0,     0,     0,   492,   493,     0,
       0,     0,     0,   156,     0,     0,     0,     0,     0,     0,
     156,   210,     0,     0,     0,   505,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,     0,     0,
       0,     0,   212,   212,     0,   850,   228,     0,    89,    90,
       0,    91,   174,    93,     0,   156,     0,     0,     0,     0,
       0,     0,   210,     0,   210,   660,   492,   493,     0,     0,
       0,     0,   268,     0,     0,     0,     0,   156,   490,   491,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   210,   829,     0,     0,     0,     0,     0,     0,
     270,     0,     0,     0,     0,     0,   226,   226,   829,   829,
     829,   829,   829,   213,     0,     0,     0,   829,     0,     0,
       0,     0,    37,     0,   780,     0,   156,     0,     0,   226,
       0,     0,     0,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   492,   493,     0,     0,     0,
       0,   210,   559,     0,   213,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,   226,     0,   210,   210,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,   550,
     551,   552,     0,     0,   213,     0,     0,     0,     0,     0,
     226,   226,    50,     0,     0,     0,     0,     0,   173,     0,
     226,    87,   321,   867,    89,    90,   226,    91,   174,    93,
       0,     0,     0,     0,  1490,     0,     0,     0,     0,   226,
       0,     0,   325,   829,     0,     0,   226,  1491,     0,  1492,
       0,     0,   326,   212,     0,     0,     0,  1046,  1047,  1048,
       0,     0,     0,   213,   226,     0,   173,     0,   226,    87,
      88,     0,    89,    90,     0,    91,  1494,    93,  1049,   213,
     213,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   643,     0,   210,   210,  1072,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,     0,
       0,   226,     0,   226,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,     0,   829,   829,
       0,   226,     0,     0,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,     0,     0,     0,     0,   212,   490,   491,     0,
       0,     0,     0,     0,     0,   212,     0,     0,   829,     0,
       0,     0,   212,     0,     0,     0,   213,   213,     0,     0,
       0,   212,   210,     0,     0,   210,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   226,     0,   226,     0,  1228,
       0,     0,     0,   643,     0,     0,    62,    63,    64,    65,
      66,   422,     0,     0,   492,   493,     0,    72,   469,     0,
       0,   226,     0,     0,   226,     0,     0,     0,     0,     0,
     505,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,     0,     0,   226,     0,     0,     0,     0,
     210,     0,     0,     0,   470,     0,   471,   228,     0,     0,
       0,     0,     0,     0,   213,   210,   210,   213,   829,   472,
       0,   473,     0,     0,   423,     0,     0,     0,   226,     0,
       0,     0,   226,   490,   491,   829,     0,   829,   434,   435,
     436,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,   829,     0,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,   643,     0,     0,
     226,   226,   213,   226,     0,     0,   210,   463,     0,     0,
     492,   493,     0,     0,     0,     0,   833,   213,   213,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,     0,     0,   330,     0,     0,     0,     0,   833,     0,
       0,     0,     0,     0,  1072,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1049,     0,   212,
    1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,   226,     0,   226,     0,     0,     0,   213,     0,
     829,     0,   829,   829,   226,  1072,     0,   829,     0,   829,
       0,     0,   829,     0,     0,     0,     0,     0,   434,   435,
     436,     0,   226,   226,     0,     0,   226,     0,     0,     0,
       0,     0,     0,   226,     0,     0,  1285,     0,   437,   438,
     210,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,   226,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,   643,   829,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   226,   226,
       0,     0,     0,     0,     0,     0,   226,     0,   226,   268,
       0,     0,     0,     0,     0,     0,     0,     0,   212,     0,
     212,     0,   213,     0,     0,     0,     0,     0,     0,     0,
     226,     0,   226,     0,     0,     0,     0,   270,     0,   226,
       0,     0,     0,     0,     0,   330,     0,   330,   212,   833,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,   643,   833,   833,   833,   833,   833,     0,
       0,     0,     0,   833,     0,     0,     0,   829,   829,   829,
       0,    50,     0,     0,   829,  1076,   226,     0,     0,  -390,
       0,     0,   226,   897,   226,     0,     0,    62,    63,    64,
     169,   170,   422,   330,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,   550,   551,   552,     0,
       0,  1097,     0,   212,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   173,     0,     0,    87,   321,
       0,    89,    90,     0,    91,   174,    93,  1097,     0,     0,
       0,     0,     0,     0,     0,     0,   212,   970,   971,   325,
       0,     0,     0,     0,     0,   423,     0,     0,     0,   326,
       0,     0,     0,     0,   214,   214,     0,   972,   232,   833,
       0,     0,  1141,     0,     0,   973,   974,   975,    37,   330,
       0,     0,   330,     0,     0,   226,     0,     0,   976,     0,
       0,     0,     0,     0,   228,     0,     0,     0,     0,     0,
      50,     0,   226,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   226,   462,     0,     0,     0,     0,   829,     0,     0,
       0,     0,     0,     0,   463,   977,   978,     0,   829,     0,
     212,   212,     0,     0,   829,     0,     0,     0,   829,   979,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,     0,
     226,     0,     0,     0,   833,   833,     0,   212,   980,     0,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,     0,     0,
     829,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     226,     0,   330,     0,   833,   813,     0,     0,   830,     0,
       0,     0,     0,     0,     0,     0,     0,   226,   212,     0,
       0,   212,   434,   435,   436,     0,   226,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     830,   226,   437,   438,     0,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   212,     0,     0,   330,   330,   212,     0,     0,     0,
       0,     0,     0,   330,     0,     0,     0,     0,     0,     0,
       0,   212,   212,     0,   833,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   833,     0,   833,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,   833,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1072,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,   214,   434,   435,   436,     0,  1391,
       0,     0,   212,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   232,   437,   438,   929,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,  1006,   462,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   463,     0,     0,     0,     0,     0,
       0,   830,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,   330,   330,   830,   830,   830,   830,
     830,    34,    35,    36,    37,   830,   204,    40,     0,   232,
       0,     0,     0,     0,   205,     0,   833,     0,   833,   833,
     212,     0,     0,   833,     0,   833,    50,     0,   833,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1486,
       0,     0,  1499,     0,     0,     0,     0,   206,     0,   214,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
    1007,    75,   207,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,   208,     0,     0,   330,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,   330,    95,     0,   212,     0,     0,
     964,     0,     0,     0,     0,     0,     0,   330,   834,   100,
       0,   830,     0,   833,   209,     0,     0,     0,     0,   106,
       0,     0,     0,     0,  1562,  1563,     0,     0,     0,     0,
       0,     0,   330,     0,  1499,     0,     0,     0,     0,     0,
     834,     0,     0,   434,   435,   436,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,   437,   438,     0,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,     0,     0,     0,     0,     0,   330,     0,     0,   330,
       0,   813,   463,   833,   833,   833,     0,     0,     0,     0,
     833,     0,  1711,     0,     0,     0,   830,   830,     0,     0,
    1499,     0,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
       0,     0,     0,     0,     0,     0,     0,   434,   435,   436,
       0,     0,     0,     0,     0,     0,   830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   437,   438,   214,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   330,   462,   330,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463,     0,     0,     0,
     214,     0,   214,     0,     0,     0,     0,     0,   968,   330,
       0,     0,   330, -1003, -1003, -1003, -1003, -1003,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,     0,     0,
     214,   834,     0,     0,     0,     0,     0,     0,     0,   463,
       0,     0,     0,   833,     0,     0,   834,   834,   834,   834,
     834,     0,     0,     0,   833,   834,   830,     0,     0,     0,
     833,     0,     0,     0,   833,     0,   330,     0,     0,     0,
     330,     0,     0,   830,     0,   830,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   830,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   214,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1088,     0,     0,     0,   833,     0,   330,   330,
       0,     0,     0,     0,     0,     0,  1826,     0,   232,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1486,     0,     0,     0,     0,     0,     0,
       0,   834,     0,     0,   434,   435,   436,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   437,   438,   232,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,     0,     0,
     330,     0,   330,   463,     0,     0,   835,     0,   830,     0,
     830,   830,   214,   214,     0,   830,     0,   830,     0,     0,
     830,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     330,     0,     0,     0,     0,     0,     0,     0,   860,     0,
       0,   330,     0,     0,     0,     0,   834,   834,     0,   232,
       0,     0,   834,   834,   834,   834,   834,   834,   834,   834,
     834,   834,   834,   834,   834,   834,   834,   834,   834,   834,
     834,   834,   834,   834,   834,   834,   834,   834,   834,   834,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   834,     0,     0,     0,
       0,     0,     0,     0,     0,   830,     0,     0,     0,     0,
     214,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   330,     0,     0,     0,     0,  1148,
       0,     0,     0,     0,     0,   434,   435,   436,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   330,     0,
     330,     0,     0,     0,     0,   437,   438,   330,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,     0,   462,   232,     0,     0,   268,     0,   214,     0,
       0,     0,     0,     0,   463,   830,   830,   830,     0,     0,
       0,     0,   830,   214,   214,     0,   834,     0,     0,     0,
     330,     0,     0,     0,   270,     0,     0,     0,     0,     0,
       0,     0,     0,   834,   268,   834,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,   834,     0,     0,     0,     0,  1010,
       0,     0,   270,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,  1032,  1033,  1034,  1035,     0,     0,
       0,     0,     0,  1043,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,   550,   551,   552,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   173,   330,     0,    87,   321,     0,    89,    90,
    1159,    91,   174,    93,     0,  1364,     0,     0,     0,     0,
     330,   550,   551,   552,     0,     0,   325,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   326,     0,     0,  1757,
     173,     0,     0,    87,   321,   830,    89,    90,     0,    91,
     174,    93,     0,     0,     0,     0,   830,     0,     0,     0,
       0,     0,   830,     0,   325,     0,   830,     0,   834,  1138,
     834,   834,   232,     0,   326,   834,     0,   834,     0,     0,
     834,     0,     0,     0,     0,     0,     0,     0,   330,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   214,   462,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,     0,   830,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   232,
     462,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   463,     0,   330,   834,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1223,     0,     0,     0,   330,
    1231,  1234,  1235,  1236,  1238,  1239,  1240,  1241,  1242,  1243,
    1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,  1252,  1253,
    1254,  1255,  1256,  1257,  1258,  1259,  1260,  1261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1269,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   394,
      12,    13,     0,     0,     0,   834,   834,   834,     0,     0,
     732,     0,   834,     0,     0,     0,     0,     0,     0,     0,
       0,  1716,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,  1352,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   169,   170,   171,     0,     0,    69,
      70,  1367,     0,  1368,     0,     0,     0,     0,     0,   172,
      75,    76,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,  1382,     0,    83,     0,     0,     0,     0,   173,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   174,
      93,     0,     0,     0,    95,     0,     0,    96,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   100,   101,
     102,     0,     0,   103,     0,   834,     0,     0,   106,   107,
       0,   108,   109,     0,     0,     0,   834,     0,     0,     0,
       0,     0,   834,     0,     0,     0,   834,     0,     0,     0,
       0,     0,     0,     0,   434,   435,   436,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1799,     0,     0,     0,   437,   438,     0,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,   834,     0,
     434,   435,   436,   463,     0,     0,  1469,     0,  1470,  1471,
       0,     0,     0,  1473,     0,  1474,     0,     0,  1475,     0,
     437,   438,     0,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   463,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1557,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,  1184,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,  1523,    68,    69,    70,    71,
      72,    73,     0,  1704,  1705,  1706,     0,    74,    75,    76,
    1710,    77,    78,    79,    80,    81,     0,     0,     0,    82,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,     0,    89,    90,     0,    91,    92,    93,    94,
       0,     0,    95,     0,     0,    96,     0,     0,     0,     0,
       0,    97,    98,     0,    99,     0,   100,   101,   102,     0,
       0,   103,     0,   104,   105,  1111,   106,   107,     0,   108,
     109,     0,     0,     0,     0,     0,     0,     0,     0,   434,
     435,   436,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   463,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1766,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,  1776,     0,     0,     0,     0,     0,
    1781,     0,     0,     0,  1783,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,  1818,    52,    53,    54,
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,  1524,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,    82,     0,
       0,    83,     0,     0,     0,     0,    84,    85,    86,    87,
      88,     0,    89,    90,     0,    91,    92,    93,    94,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,    98,     0,    99,     0,   100,   101,   102,     0,     0,
     103,     0,   104,   105,  1286,   106,   107,     0,   108,   109,
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
       0,     0,     0,     0,    74,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,    82,     0,     0,    83,
       0,     0,     0,     0,    84,    85,    86,    87,    88,     0,
      89,    90,     0,    91,    92,    93,    94,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,    98,
       0,    99,     0,   100,   101,   102,     0,     0,   103,     0,
     104,   105,     0,   106,   107,     0,   108,   109,     5,     6,
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
       0,     0,    74,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,    82,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,    94,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   103,     0,   104,   105,
     662,   106,   107,     0,   108,   109,     5,     6,     7,     8,
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
      74,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,    82,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,    94,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,     0,   104,   105,  1075,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
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
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
      82,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
      94,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   103,     0,   104,   105,  1125,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
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
      73,     0,     0,     0,     0,     0,    74,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,    82,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,    94,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     103,     0,   104,   105,  1190,   106,   107,     0,   108,   109,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,  1192,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,    82,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,    94,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   103,     0,
     104,   105,     0,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,  1353,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,    82,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,    94,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   103,     0,   104,   105,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
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
      74,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,    82,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,    94,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,     0,   104,   105,  1477,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
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
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
      82,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
      94,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   103,     0,   104,   105,  1707,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,  1753,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,    82,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,    94,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     103,     0,   104,   105,     0,   106,   107,     0,   108,   109,
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
       0,     0,     0,     0,    74,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,    82,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,    94,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   103,     0,
     104,   105,  1788,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,  1791,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,    82,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,    94,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   103,     0,   104,   105,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
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
      74,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,    82,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,    94,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,     0,   104,   105,  1808,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
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
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
      82,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
      94,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   103,     0,   104,   105,  1825,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
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
      73,     0,     0,     0,     0,     0,    74,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,    82,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,    94,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     103,     0,   104,   105,  1866,   106,   107,     0,   108,   109,
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
       0,     0,     0,     0,    74,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,    82,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,    94,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   103,     0,
     104,   105,  1873,   106,   107,     0,   108,   109,     5,     6,
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
       0,     0,    74,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,    82,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,    94,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   103,     0,   104,   105,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   533,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   169,   170,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,    82,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,     0,   104,   105,     0,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,   798,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,   169,   170,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
      82,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   103,     0,   104,   105,     0,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1012,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,   169,   170,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,    82,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,     0,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     103,     0,   104,   105,     0,   106,   107,     0,   108,   109,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1552,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,   169,
     170,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,    82,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   103,     0,
     104,   105,     0,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1699,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,   169,   170,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,    82,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,     0,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   103,     0,   104,   105,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,   169,   170,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,    82,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,     0,   104,   105,     0,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   169,   170,   171,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   172,    75,
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
    1047,  1048,   175,     0,   335,     0,     0,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
    1049,     0,    10,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,     0,     0,   614,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   169,   170,   171,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   172,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,     0,   615,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     175,     0,     0,     0,     0,   106,   107,     0,   108,   109,
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
       0,     0,     0,     0,     0,     0,    62,    63,    64,   169,
     170,   171,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   172,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,  1048,   175,     0,
       0,   793,     0,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,  1049,     0,    10,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,     0,     0,  1106,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1072,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   169,   170,   171,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   172,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,     0,  1107,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   175,     0,     0,     0,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     394,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   169,   170,   171,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     172,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,   434,   435,   436,     0,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   437,   438,  1357,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,     0,   462,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   463,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   186,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   169,   170,   171,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   172,    75,
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,     0,    96,     0,  1358,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   175,     0,     0,     0,     0,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,     0,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   169,   170,   171,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   172,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,     0,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     175,   434,   435,   436,     0,   106,   107,     0,   108,   109,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   437,   438,     0,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,     0,   462,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     463,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   169,
     170,   171,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   172,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,     0,    96,     0,   464,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   175,     0,
     253,   435,   436,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
     437,   438,     0,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,   463,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   169,   170,   171,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   172,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,     0,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   175,     0,   256,     0,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     394,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   169,   170,   171,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     172,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   103,   434,   435,   436,     0,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   437,   438,     0,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,     0,   462,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   463,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   169,   170,   171,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   172,    75,
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,     0,    96,     0,   547,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   175,   531,     0,     0,     0,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   687,   462,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   463,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   169,   170,   171,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   172,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,     0,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     175,     0,     0,     0,     0,   106,   107,     0,   108,   109,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,     0,     0,     0,   732,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1072,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   169,
     170,   171,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   172,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   175,     0,
       0,     0,     0,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,
       0,     0,     0,   773,     0,     0,     0,     0,     0,     0,
       0,     0,  1072,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   169,   170,   171,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   172,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,     0,     0,     0,    95,     0,     0,
      96,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   175,     0,     0,     0,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,     0,     0,     0,     0,
       0,   775,     0,     0,     0,     0,     0,     0,     0,  1072,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   169,   170,   171,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     172,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   175,     0,     0,     0,     0,   106,
     107,     0,   108,   109,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10, -1003, -1003, -1003, -1003,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,     0,     0,     0,     0,     0,  1103,
       0,     0,     0,     0,     0,   463,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   169,   170,   171,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   172,    75,
      76,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,     0,    96,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   100,   101,   102,
       0,     0,   175,     0,     0,     0,     0,   106,   107,     0,
     108,   109,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,  1181,     0,     0,
       0,     0,     0,  1072,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,   169,   170,   171,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   172,    75,    76,     0,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,   173,    85,    86,    87,
      88,     0,    89,    90,     0,    91,   174,    93,     0,     0,
       0,    95,     0,     0,    96,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   100,   101,   102,     0,     0,
     175,     0,     0,     0,     0,   106,   107,     0,   108,   109,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10, -1003, -1003, -1003, -1003,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,     0,
       0,     0,     0,     0,     0,  1411,     0,     0,     0,     0,
       0,  1072,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,   169,
     170,   171,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   172,    75,    76,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,     0,    96,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   100,   101,   102,     0,     0,   175,   434,
     435,   436,     0,   106,   107,     0,   108,   109,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   437,
     438,     0,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,     0,   462,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   463,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   169,   170,   171,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   172,    75,    76,     0,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,   173,    85,    86,    87,    88,     0,    89,    90,
       0,    91,   174,    93,     0,     0,     0,    95,     0,     0,
      96,     0,   549,     0,     0,     0,    97,     0,     0,     0,
       0,   100,   101,   102,     0,     0,   175,   434,   435,   436,
       0,   106,   107,     0,   108,   109,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   437,   438,     0,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,     0,   462,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   463,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,   624,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,   169,   170,   171,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     172,    75,    76,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,    83,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,     0,    96,     0,
     571,     0,     0,     0,    97,     0,     0,     0,     0,   100,
     101,   102,     0,     0,   175,     0,     0,     0,     0,   106,
     107,     0,   108,   109,   258,   259,     0,   260,   261,     0,
       0,   262,   263,   264,   265,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   266,     0,
     267,     0,   437,   438,     0,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   269,   462,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   463,   271,   272,   273,   274,   275,   276,   277,     0,
       0,     0,    37,     0,   204,    40,     0,     0,     0,     0,
       0,     0,     0,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,    50,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,     0,     0,
       0,   720,   313,   314,   315,     0,     0,     0,   316,   563,
     564,   565,     0,     0,     0,     0,     0,   258,   259,     0,
     260,   261,     0,     0,   262,   263,   264,   265,   566,     0,
       0,     0,     0,     0,    89,    90,     0,    91,   174,    93,
     322,   266,   323,   267,     0,   324,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   721,     0,   106,     0,     0,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   271,   272,   273,   274,   275,
     276,   277,     0,     0,     0,    37,     0,   204,    40,     0,
       0,     0,     0,     0,     0,     0,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,    50,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,   312,   313,   314,   315,     0,     0,
       0,   316,   563,   564,   565,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   258,   259,     0,   260,   261,
       0,   566,   262,   263,   264,   265,     0,    89,    90,     0,
      91,   174,    93,   322,     0,   323,     0,     0,   324,   266,
       0,   267,     0,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   721,     0,
     106,     0,     0,     0,     0,     0,     0,     0,     0,   269,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   271,   272,   273,   274,   275,   276,   277,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,    50,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,     0,
       0,     0,     0,   313,   314,   315,     0,     0,     0,   316,
     317,   318,   319,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
       0,     0,    87,   321,     0,    89,    90,     0,    91,   174,
      93,   322,     0,   323,     0,     0,   324,   258,   259,     0,
     260,   261,     0,   325,   262,   263,   264,   265,     0,     0,
       0,     0,     0,   326,     0,     0,     0,  1678,     0,     0,
       0,   266,     0,   267,   438,   268,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,   269,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,   463,     0,     0,   271,   272,   273,   274,   275,
     276,   277,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,    50,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,     0,   313,   314,   315,     0,     0,
       0,   316,   317,   318,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,    87,   321,     0,    89,    90,     0,
      91,   174,    93,   322,     0,   323,     0,     0,   324,   258,
     259,     0,   260,   261,     0,   325,   262,   263,   264,   265,
       0,     0,     0,     0,     0,   326,     0,     0,     0,  1748,
       0,     0,     0,   266,     0,   267,     0,   268,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,     0,   462,   269,     0,   270,     0,     0,     0,     0,
       0,     0,     0,     0,   463,     0,     0,   271,   272,   273,
     274,   275,   276,   277,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,    50,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,     0,     0,     0,   312,   313,   314,   315,
       0,     0,     0,   316,   317,   318,   319,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,    87,   321,     0,    89,
      90,     0,    91,   174,    93,   322,     0,   323,     0,     0,
     324,   258,   259,     0,   260,   261,     0,   325,   262,   263,
     264,   265,     0,     0,     0,     0,     0,   326,     0,     0,
       0,     0,     0,     0,     0,   266,     0,   267,     0,   268,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,   270,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   271,
     272,   273,   274,   275,   276,   277,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,    50,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,     0,     0,     0,     0,   313,
     314,   315,     0,     0,     0,   316,   317,   318,   319,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,    87,   321,
       0,    89,    90,     0,    91,   174,    93,   322,     0,   323,
       0,     0,   324,     0,   258,   259,     0,   260,   261,   325,
    1481,   262,   263,   264,   265,     0,     0,     0,     0,   326,
       0,     0,     0,     0,     0,     0,     0,     0,   266,     0,
     267,     0,   268,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,     0,
     270,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   271,   272,   273,   274,   275,   276,   277,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,    50,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,     0,     0,
       0,     0,   313,   314,   315,     0,     0,     0,   316,   317,
     318,   319,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   320,     0,
       0,    87,   321,     0,    89,    90,     0,    91,   174,    93,
     322,     0,   323,     0,     0,   324,  1577,  1578,  1579,  1580,
    1581,     0,   325,  1582,  1583,  1584,  1585,     0,     0,     0,
       0,     0,   326,     0,     0,     0,     0,     0,     0,     0,
    1586,  1587,  1588,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1589,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1590,  1591,  1592,  1593,  1594,  1595,
    1596,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1597,  1598,  1599,  1600,  1601,
    1602,  1603,  1604,  1605,  1606,  1607,    50,  1608,  1609,  1610,
    1611,  1612,  1613,  1614,  1615,  1616,  1617,  1618,  1619,  1620,
    1621,  1622,  1623,  1624,  1625,  1626,  1627,  1628,  1629,  1630,
    1631,  1632,  1633,  1634,  1635,  1636,  1637,     0,     0,     0,
    1638,  1639,  1640,     0,  1641,  1642,  1643,  1644,  1645,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1646,  1647,  1648,     0,     0,     0,    89,    90,     0,    91,
     174,    93,  1649,     0,  1650,  1651,     0,  1652,   434,   435,
     436,     0,     0,     0,  1653,  1654,     0,  1655,     0,  1656,
    1657,     0,     0,     0,     0,     0,     0,     0,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,   434,   435,   436,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,     0,     0,     0,     0,     0,   437,   438,     0,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,   434,   435,   436,     0,     0,     0,
       0,     0,     0,     0,     0,   463,     0,     0,     0,     0,
       0,     0,     0,     0,   437,   438,     0,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   258,   259,   463,   260,   261,     0,     0,   262,   263,
     264,   265,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   266,     0,   267,     0,     0,
       0,   575,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   271,
     272,   273,   274,   275,   276,   277,     0,     0,   765,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,    50,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,     0,   790,     0,   312,   313,
     314,   315,     0,     0,     0,   316,   563,   564,   565,     0,
       0,     0,     0,     0,   258,   259,     0,   260,   261,     0,
       0,   262,   263,   264,   265,   566,     0,     0,     0,     0,
       0,    89,    90,     0,    91,   174,    93,   322,   266,   323,
     267,     0,   324,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   271,   272,   273,   274,   275,   276,   277,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,    50,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,     0,     0,
       0,  1229,   313,   314,   315,     0,     0,     0,   316,   563,
     564,   565,     0,     0,     0,     0,     0,   258,   259,     0,
     260,   261,     0,     0,   262,   263,   264,   265,   566,     0,
       0,     0,     0,     0,    89,    90,     0,    91,   174,    93,
     322,   266,   323,   267,     0,   324,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   271,   272,   273,   274,   275,
     276,   277,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,    50,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,     0,   313,   314,   315,  1237,     0,
       0,   316,   563,   564,   565,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   819,   820,     0,     0,
       0,   566,   821,     0,   822,     0,     0,    89,    90,     0,
      91,   174,    93,   322,     0,   323,   823,     0,   324,     0,
       0,     0,     0,     0,    34,    35,    36,    37,     0,     0,
       0,     0,     0,   434,   435,   436,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,   437,   438,     0,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,     0,
     462,     0,     0,     0,   824,   825,     0,    77,    78,    79,
      80,    81,   463,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,   173,    85,    86,    87,   826,     0,    89,
      90,     0,    91,   174,    93,   819,   820,     0,    95,     0,
       0,   821,     0,   822,     0,     0,     0,   827,     0,     0,
       0,     0,   100,     0,     0,   823,     0,   828,     0,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,     0,
       0,     0,   434,   435,   436,     0,   205,     0,     0,   508,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,   437,   438,     0,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,     0,     0,   824,   825,     0,    77,    78,    79,    80,
      81,   463,     0,     0,     0,     0,     0,   208,     0,     0,
       0,     0,   173,    85,    86,    87,   826,     0,    89,    90,
       0,    91,   174,    93,    29,     0,     0,    95,     0,     0,
       0,     0,    34,    35,    36,    37,   827,   204,    40,     0,
       0,   100,     0,     0,     0,   205,   828,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   517,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   207,     0,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,   173,    85,    86,    87,    88,     0,    89,    90,     0,
      91,   174,    93,    29,     0,     0,    95,     0,     0,     0,
       0,    34,    35,    36,    37,     0,   204,    40,     0,     0,
     100,     0,     0,     0,   205,   209,     0,     0,   587,     0,
     106,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     607,    75,   207,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,    29,     0,   959,    95,     0,     0,     0,     0,
      34,    35,    36,    37,     0,   204,    40,     0,     0,   100,
       0,     0,     0,   205,   209,     0,     0,     0,     0,   106,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   207,     0,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,   208,     0,     0,     0,     0,   173,
      85,    86,    87,    88,     0,    89,    90,     0,    91,   174,
      93,    29,     0,     0,    95,     0,     0,     0,     0,    34,
      35,    36,    37,     0,   204,    40,     0,     0,   100,     0,
       0,     0,   205,   209,     0,     0,     0,     0,   106,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1100,    75,
     207,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,     0,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
      29,     0,     0,    95,     0,     0,     0,     0,    34,    35,
      36,    37,     0,   204,    40,     0,     0,   100,     0,     0,
       0,   205,   209,     0,     0,     0,     0,   106,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   207,
       0,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,     0,   173,    85,    86,
      87,    88,     0,    89,    90,     0,    91,   174,    93,     0,
       0,     0,    95,     0,   434,   435,   436,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   100,     0,     0,     0,
       0,   209,     0,     0,   437,   438,   106,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
       0,   462,   434,   435,   436,     0,     0,     0,     0,     0,
       0,     0,     0,   463,     0,     0,     0,     0,     0,     0,
       0,     0,   437,   438,     0,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
       0,     0,     0,     0,     0,     0,     0,     0,   434,   435,
     436,   463,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   437,   438,
     882,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,   434,   435,   436,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,     0,     0,     0,     0,     0,   437,   438,   945,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,     0,     0,     0,     0,     0,     0,
       0,     0,   434,   435,   436,   463,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   437,   438,   991,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,     0,   462,
    1046,  1047,  1048,     0,     0,     0,     0,     0,     0,     0,
       0,   463,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1049,  1282,     0,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1046,  1047,  1048,     0,  1072,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1049,     0,  1313,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,     0,     0,  1046,  1047,  1048,     0,     0,     0,     0,
       0,     0,     0,     0,  1072,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1049,     0,  1216,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1046,  1047,
    1048,     0,  1072,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1049,
       0,  1373,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,     0,     0,  1046,  1047,  1048,     0,
       0,     0,     0,     0,     0,     0,     0,  1072,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1049,     0,  1378,
    1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1046,  1047,  1048,     0,  1072,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1049,     0,  1468,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,     0,    34,
      35,    36,    37,     0,   204,    40,     0,     0,     0,     0,
    1072,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1558,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,  1560,   173,    85,
      86,    87,    88,     0,    89,    90,     0,    91,   174,    93,
       0,     0,     0,    95,     0,    34,    35,    36,    37,     0,
     204,    40,     0,     0,     0,     0,     0,   100,   638,     0,
       0,     0,   225,     0,     0,     0,     0,   106,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,   173,    85,    86,    87,    88,     0,
      89,    90,     0,    91,   174,    93,     0,     0,     0,    95,
       0,    34,    35,    36,    37,     0,   204,    40,     0,     0,
       0,     0,     0,   100,   205,     0,     0,     0,   639,     0,
       0,     0,     0,   106,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
     173,    85,    86,    87,    88,     0,    89,    90,     0,    91,
     174,    93,     0,     0,     0,    95,     0,   434,   435,   436,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,     0,     0,   225,   802,     0,   437,   438,   106,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,     0,   462,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   463,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     434,   435,   436,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   803,
     437,   438,   942,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,     0,   462,   434,   435,
     436,     0,     0,     0,     0,     0,     0,     0,     0,   463,
       0,     0,     0,     0,     0,     0,     0,     0,   437,   438,
       0,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,     0,   462,  1046,  1047,  1048,     0,
       0,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1049,  1383,     0,
    1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1046,  1047,  1048,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1072,     0,     0,     0,     0,
       0,     0,     0,  1049,     0,     0,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,   436,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1072,     0,     0,     0,     0,   437,   438,     0,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,     0,   462,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   463
};

static const yytype_int16 yycheck[] =
{
       5,     6,   124,     8,     9,    10,    11,    12,    13,   152,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    56,     4,    29,    30,   617,     4,    94,   387,
      33,   177,    98,    99,  1128,     4,   103,     4,   650,    44,
     387,     4,   651,    46,   387,    56,   922,    52,    51,    54,
     502,   226,    57,   494,    59,    31,   178,   123,   157,   521,
     462,   103,   152,   654,   911,    57,   414,    60,   581,   582,
      31,    31,   533,   494,    31,   729,   801,    82,    57,   342,
     343,   103,    44,   103,   631,   808,   938,  1115,   529,   318,
       9,    84,     4,  1005,    87,    14,     9,    32,   103,    32,
      49,    49,   954,     9,    49,    49,     9,   237,   529,    83,
      49,   572,     9,     9,     9,     9,   115,    14,    14,    14,
      82,     9,     9,     9,   221,     9,     9,     9,   777,     4,
      83,     9,    70,   175,    70,    50,    51,    49,     9,   238,
     992,     9,     4,     9,     9,    70,     9,   106,     9,  1692,
     498,   499,    83,   175,    83,   175,     9,    83,     9,   156,
     102,     9,     9,     9,     9,    36,    90,   209,   526,    38,
     175,    38,    90,    83,    84,   174,   177,   182,    70,   527,
       8,   767,     0,   225,    14,    75,    76,   209,    38,   209,
     191,   652,   194,    70,   191,  1042,   134,   135,   156,    32,
     112,    38,    32,   225,   209,   117,  1749,   119,   120,   121,
     122,   123,   124,   125,    83,    70,    83,   170,   160,   177,
     225,    51,   196,    38,     4,   154,   155,    70,   160,   153,
     156,  1124,    38,    83,   239,   153,    70,   242,    70,   170,
     189,   189,   372,   192,   249,   250,    83,   192,   192,    70,
      70,   163,   164,   192,   166,   191,   194,    70,   177,   194,
     193,   852,   124,   178,   156,    54,   191,    70,    83,   194,
      70,    70,    70,    53,   193,   187,    56,   152,   424,   192,
     243,   177,  1310,   195,   247,  1197,   196,   193,   194,  1317,
     193,  1319,  1015,    73,  1017,    70,   193,   193,   193,   193,
     334,   170,   194,   170,  1156,   193,   193,   193,   957,   193,
     193,   193,    92,  1341,    94,   193,   178,   194,    98,    99,
     362,   192,   551,   334,   156,   193,   935,   193,   193,  1176,
     193,   192,   793,   170,    83,   848,   849,   798,    83,   192,
     362,   192,   362,   123,   192,   192,   192,   192,   191,   342,
     343,   344,   418,   900,   476,   170,   361,   362,   504,   191,
     194,   162,   194,   368,   369,   370,   134,   135,    70,   160,
     375,   191,   161,   194,   194,    70,    70,    38,   191,   192,
     191,   415,    70,   123,    70,   162,   379,   176,   191,   394,
     130,   194,   191,   194,   194,   194,   194,   402,   175,   361,
      83,    84,   193,   194,   415,   471,   472,   473,   474,   414,
     477,   191,  1440,    56,   511,  1308,   393,   194,   191,   194,
     191,   170,    83,   161,    70,   170,   878,   195,   191,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   468,   463,  1113,
     465,   466,   467,   243,    27,    28,    54,   247,  1193,   191,
     156,   251,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   477,   462,   928,   134,   135,
     495,   496,   194,   498,   499,   500,   501,   191,   477,   194,
     505,   462,   462,   508,   515,   462,   194,   928,   194,   468,
     156,   194,   517,   196,   519,  1127,   613,   134,   135,    83,
     106,   107,   527,   191,    31,   668,   401,   657,  1375,   659,
     535,  1140,   537,   191,  1143,   494,  1429,   102,  1431,   401,
     106,   107,   102,   535,   641,   191,   177,    83,   723,   102,
     526,  1012,     4,   911,   334,   177,   515,   959,  1393,   191,
     191,   105,   106,   107,   911,    50,    51,   526,   911,   191,
     529,    31,  1085,   161,    81,    27,    28,   540,   668,   842,
     755,   844,   587,   191,    91,   731,   466,  1173,   581,   582,
      50,  1177,   191,    53,    83,   160,   103,   639,   177,    83,
     160,    90,    57,   951,    70,  1452,    90,   160,   191,   195,
     615,   191,   191,   475,    69,   495,   682,   153,   154,   155,
     500,   401,   719,    81,   188,   268,    83,   270,   122,   195,
     194,   411,   177,    90,   639,   415,   130,   156,   418,  1532,
     191,   771,   772,  1536,   191,   103,   191,   193,   778,   779,
     157,   103,  1487,   160,   829,   218,   163,   164,   177,   166,
     167,   168,   837,   193,  1318,   154,   155,   193,   134,   135,
     154,   155,   191,   193,   651,   194,  1511,   193,  1513,   778,
     102,   103,   687,   326,  1042,    31,    70,  1299,   468,   469,
     470,   471,   472,   473,   474,  1042,  1158,   154,   155,  1042,
      75,    76,   160,  1312,    50,   163,   164,    53,   166,   167,
     168,   200,    83,   810,   494,   199,   721,   156,   815,    90,
     193,   134,   135,   175,   119,   120,   121,   122,   123,   124,
     132,   133,    27,    28,   193,   515,   194,    81,   177,   193,
    1326,    70,  1328,  1837,  1196,   750,    53,    54,    55,   529,
      57,   105,   106,   107,  1408,   194,    83,   209,  1852,   103,
     540,    70,    69,    90,    83,   194,   218,    83,   156,   412,
     191,    90,   415,   225,    90,   193,   194,   874,   191,   191,
     785,    70,   562,   154,   155,    81,   193,   194,   351,  1723,
    1724,   243,   187,   668,   797,   247,   801,   360,  1691,   362,
     156,  1149,  1695,  1264,   367,   585,   586,   103,   119,   120,
     121,   191,  1160,   376,    53,    54,    55,   161,  1176,   163,
     164,   165,   166,   167,   168,  1277,   153,   154,   155,  1176,
      69,   193,  1441,  1176,  1488,   154,   155,   153,   154,   155,
     620,   621,   160,   806,   107,   108,   109,   191,    48,   842,
      69,   844,   342,   343,   191,   848,   849,   850,  1719,  1720,
     177,  1447,   156,  1449,  1450,   817,   818,   163,   164,   191,
     166,   167,   168,   198,     9,   156,   191,   882,   156,   884,
       8,   191,  1343,   156,   889,   193,   191,    14,   156,   986,
     193,   896,   958,   193,     9,   191,   194,   130,  1359,   351,
     193,   130,   682,   914,    14,   910,   192,   177,   360,    14,
     362,   102,   555,   192,   197,   367,   791,   192,   899,   111,
     192,   192,   899,   218,   376,   191,   191,  1803,     9,   791,
     899,   191,   899,   153,  1031,   192,   899,   942,  1831,   502,
     945,  1038,   947,   192,   192,   922,   951,  1823,   192,   401,
      94,     9,   193,  1846,    14,   914,  1832,  1543,   935,  1134,
     177,   191,   111,     9,   839,   191,  1314,   926,   193,   928,
     119,   120,   121,   122,   123,   124,   194,   839,   194,   193,
      83,   192,   192,   959,   192,   132,   991,   767,   193,   769,
     191,   198,     9,   192,  1455,   998,   192,     9,   959,   959,
      70,    32,   959,  1464,   647,   648,   176,   133,   373,     9,
     156,   791,   377,   656,   136,  1476,   192,  1375,   156,    14,
     189,     9,     9,   178,   899,   805,   806,   192,  1375,    14,
    1041,    81,  1375,  1044,   993,  1102,   999,   899,   187,   404,
     132,   406,   407,   408,   409,   920,   198,   198,     9,    14,
     502,   614,   195,   103,   156,   192,   351,   198,   920,   839,
     192,   191,   198,   192,   102,   360,   846,   847,   193,   193,
       9,    91,   367,  1170,   136,     9,   156,    70,   192,  1427,
     191,   376,   156,   191,   156,    81,     9,   194,   540,   195,
     194,  1552,  1085,    14,  1452,   875,   193,  1102,    81,   178,
       9,   194,  1107,    14,   198,  1452,   194,   103,    14,  1452,
    1102,  1697,  1698,   163,   164,   192,   166,   167,   168,   899,
     103,   189,   193,    14,    32,  1222,  1001,   191,  1003,  1226,
     191,  1112,    32,    14,   914,  1112,  1233,   191,   191,  1001,
     920,  1003,    52,  1112,  1149,  1112,   926,   191,   928,  1112,
      70,   191,  1743,     9,   193,  1160,  1161,   192,   191,   193,
     136,   157,   614,  1140,   160,   178,  1143,   163,   164,   732,
     166,   167,   168,    14,   817,   818,   136,     9,   958,    69,
     163,   164,  1185,   166,   167,   168,   192,   198,  1193,     9,
     970,   971,   972,   195,    83,   193,   195,     9,  1203,   195,
     191,    50,    51,    52,    53,    54,    55,   502,   191,   136,
     773,   191,   775,   993,   193,    14,   194,    83,   192,   999,
      69,  1001,   191,  1003,   191,  1188,   192,   198,  1689,   193,
    1821,   194,   194,   136,     9,    31,  1333,  1112,  1699,    91,
     803,  1338,   194,  1023,    32,   193,   153,    77,   192,   178,
    1112,   193,   119,   120,   121,   122,   123,   124,   901,   136,
      32,  1041,   192,    59,  1044,   198,  1409,   192,     9,     9,
     192,   136,     9,    81,   917,    83,    84,  1282,   195,   192,
     732,   193,  1404,  1744,    14,    81,  1291,   930,   193,    83,
    1295,   195,  1297,  1073,   191,   103,   194,   192,  1275,   862,
    1305,   193,   198,     9,   192,   192,   191,   103,  1313,  1314,
     194,  1288,   955,   192,   192,   878,   879,   136,   136,   614,
     187,   773,   192,   775,     9,  1200,   193,    32,  1789,   193,
     192,   192,  1112,   193,   193,  1312,   194,   165,  1200,   791,
     193,   112,   138,   139,   140,   161,    14,    81,    83,   117,
     192,   803,   192,   161,   806,   163,   164,   194,   166,   167,
     168,   157,   136,  1509,   160,   161,   192,   163,   164,   103,
     166,   167,   168,   136,   170,    14,  1019,   177,   193,  1022,
      81,   194,    83,    84,    83,   181,   194,   839,   196,  1850,
      14,    14,  1172,  1173,   128,   191,  1857,  1177,   191,    83,
    1275,   192,   103,   192,   136,   136,   193,   193,  1188,    14,
     862,    14,   193,  1288,    14,  1481,   194,    59,    81,  1424,
    1200,     9,  1427,     9,    83,   195,   878,   879,   177,   163,
     164,   191,   166,   167,   168,    83,     9,   732,   194,   193,
     103,   115,  1005,  1006,   102,   156,   102,   899,   111,   112,
      81,   178,   168,  1430,    36,    14,   192,   191,   191,  1436,
     161,  1438,   163,   164,  1441,   166,   167,   168,   920,   193,
     191,   174,   103,  1484,   178,    83,   178,  1454,   773,   171,
     775,     9,   192,  1126,    83,  1128,    78,    79,    80,    81,
    1270,  1085,   192,   194,   192,   196,   193,   160,  1461,   194,
     163,   164,    14,   166,   167,   168,    83,    14,   803,  1152,
    1385,   103,  1155,    83,    14,    83,    14,    83,  1812,   474,
     469,   471,  1527,  1385,   952,   902,   157,  1828,  1194,   160,
     161,  1677,   163,   164,  1409,   166,   167,   168,  1551,  1356,
    1103,  1403,  1823,  1106,   589,  1539,  1326,   999,  1328,  1001,
    1575,  1003,  1489,  1005,  1006,  1430,  1661,  1399,    81,  1856,
      83,  1436,    85,  1438,  1541,  1844,  1209,   862,  1395,  1673,
    1213,   163,   164,  1116,   166,   167,   168,  1039,   971,  1454,
     103,  1456,  1171,   878,   879,  1535,   926,  1550,  1551,   988,
    1465,  1172,   368,   415,  1456,   817,    27,    28,  1388,  1778,
      31,  1093,  1073,  1465,  1024,  1385,    -1,    50,    51,    52,
      53,    54,    55,  1393,    57,    -1,  1713,  1714,  1181,  1399,
      -1,    -1,    -1,    -1,    -1,    56,    69,    -1,  1271,  1272,
      -1,    -1,    -1,  1196,  1197,    -1,    -1,    -1,  1672,     4,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,    -1,
      -1,  1103,    -1,  1796,  1106,  1517,    -1,    -1,    -1,    -1,
    1112,  1672,    -1,    -1,    -1,    -1,  1541,  1447,    -1,  1449,
    1450,  1737,    -1,    -1,  1549,    -1,  1456,  1682,    -1,    -1,
    1555,  1461,    -1,    -1,    49,  1465,  1561,  1549,    27,    28,
      -1,    -1,    -1,  1555,  1816,    -1,    -1,    81,    -1,  1561,
      -1,  1481,    -1,    -1,  1484,    -1,    -1,  1487,    -1,    -1,
    1005,  1006,    -1,    -1,  1277,    -1,    -1,  1497,    -1,   103,
    1363,    -1,  1365,    -1,  1504,    -1,    78,    79,    80,  1181,
      -1,  1511,    -1,  1513,    -1,    -1,  1188,    -1,    -1,    91,
    1520,    -1,    -1,    -1,  1196,  1197,    -1,   112,  1200,    -1,
      -1,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,  1404,    -1,  1543,    -1,    -1,    -1,    -1,    -1,  1549,
    1550,  1551,    -1,    -1,    -1,  1555,    -1,   161,    -1,   163,
     164,  1561,   166,   167,   168,    -1,    -1,   218,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,   163,   164,
     152,   166,    81,    -1,    -1,    -1,   158,   159,  1103,    -1,
      -1,  1106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     172,    -1,   187,    -1,   103,  1277,  1803,    -1,  1703,    -1,
     195,    -1,    -1,     4,   186,    -1,    -1,   268,    -1,   270,
      -1,  1703,    -1,    -1,    -1,    -1,  1823,    -1,  1411,    -1,
      -1,    -1,    -1,    -1,    -1,  1832,    -1,  1862,     4,    -1,
      -1,    -1,    -1,    -1,    -1,  1870,    -1,    -1,    -1,    -1,
    1745,  1876,    -1,    -1,  1879,    -1,    -1,  1752,    49,   218,
      -1,   160,    -1,  1745,   163,   164,  1181,   166,   167,   168,
    1752,    -1,  1672,    -1,    -1,   326,    -1,    -1,    -1,    -1,
      -1,  1196,  1197,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1787,  1693,    -1,    81,    -1,  1697,  1698,    -1,
     351,  1796,    -1,  1703,    -1,  1787,    -1,    -1,    -1,   360,
    1573,    -1,  1712,  1385,  1809,    -1,   367,   103,    -1,  1719,
    1720,   112,     4,  1723,  1724,   376,   117,  1809,   119,   120,
     121,   122,   123,   124,   125,    -1,   387,  1737,    31,  1411,
      -1,    -1,    -1,    -1,    -1,  1745,   112,    -1,    -1,    -1,
      -1,   117,  1752,   119,   120,   121,   122,   123,   124,   125,
      -1,   412,  1277,  1858,   415,    -1,    -1,    49,    -1,    -1,
    1865,    -1,   163,   164,   160,   166,  1858,   163,   164,    -1,
     166,   167,   168,  1865,  1456,    -1,    -1,  1787,    81,  1461,
      -1,    -1,   351,  1465,    -1,  1795,   187,   163,   164,    -1,
     166,   360,    -1,    -1,   195,    -1,    -1,     4,   367,  1809,
     103,   462,    -1,  1676,    -1,  1815,    -1,   376,    -1,    -1,
      -1,   187,   119,   120,   121,   122,   123,   124,   387,   195,
     112,    -1,   125,   130,   131,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   138,    -1,   140,    -1,    -1,
      -1,   502,    49,    -1,    -1,    -1,    -1,    -1,  1858,    -1,
      -1,    -1,    -1,     4,   157,  1865,    -1,   160,   161,    -1,
     163,   164,   169,   166,   167,   168,    -1,  1549,  1550,  1551,
      -1,   163,   164,  1555,   166,    -1,    -1,    -1,    -1,  1561,
     187,    -1,    -1,    -1,    -1,    -1,  1411,    -1,  1761,    -1,
      -1,    -1,    -1,    -1,   555,   187,    -1,   558,    49,    -1,
     561,    -1,    -1,   195,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   593,   502,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,   614,    -1,    -1,   163,   164,    -1,   166,
      -1,   112,    -1,    -1,  1837,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,  1852,
     187,    -1,    -1,    -1,    -1,    -1,   647,   648,   195,    -1,
      67,    68,    -1,    -1,    -1,   656,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,   163,   164,    -1,   166,    81,    -1,    -1,    -1,
      -1,  1703,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,   187,    -1,   103,    -1,
      -1,    -1,    -1,    -1,   195,   614,   111,   112,    69,    67,
      68,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,  1745,    -1,    -1,    -1,    -1,    -1,    -1,
    1752,   732,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    27,    28,    -1,   160,    31,    -1,   163,   164,
      -1,   166,   167,   168,    -1,  1787,    -1,    -1,    -1,    -1,
      -1,    -1,   773,    -1,   775,   192,   134,   135,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,  1809,    67,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   803,   804,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,    -1,   817,   818,   819,   820,
     821,   822,   823,   732,    -1,    -1,    -1,   828,    -1,    -1,
      -1,    -1,    81,    -1,   192,    -1,  1858,    -1,    -1,   840,
      -1,    -1,    -1,  1865,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   134,   135,    -1,    -1,    -1,
      -1,   862,   111,    -1,   773,    -1,   775,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   876,    -1,   878,   879,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,    -1,    -1,   803,    -1,    -1,    -1,    -1,    -1,
     901,   902,   103,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     911,   160,   161,   192,   163,   164,   917,   166,   167,   168,
      -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,   930,
      -1,    -1,   181,   934,    -1,    -1,   937,   138,    -1,   140,
      -1,    -1,   191,   218,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,   862,   955,    -1,   157,    -1,   959,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    31,   878,
     879,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   911,    -1,  1005,  1006,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1019,    -1,
      -1,  1022,    -1,  1024,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,  1039,  1040,
      -1,  1042,    -1,    -1,  1045,  1046,  1047,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,    -1,    -1,    -1,    -1,   351,    67,    68,    -1,
      -1,    -1,    -1,    -1,    -1,   360,    -1,    -1,  1089,    -1,
      -1,    -1,   367,    -1,    -1,    -1,  1005,  1006,    -1,    -1,
      -1,   376,  1103,    -1,    -1,  1106,    -1,    -1,    -1,    -1,
      -1,    -1,   387,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1126,    -1,  1128,    -1,   192,
      -1,    -1,    -1,  1042,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   134,   135,    -1,   130,   131,    -1,
      -1,  1152,    -1,    -1,  1155,    -1,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,  1176,    -1,    -1,    -1,    -1,
    1181,    -1,    -1,    -1,   167,    -1,   169,   462,    -1,    -1,
      -1,    -1,    -1,    -1,  1103,  1196,  1197,  1106,  1199,   182,
      -1,   184,    -1,    -1,   187,    -1,    -1,    -1,  1209,    -1,
      -1,    -1,  1213,    67,    68,  1216,    -1,  1218,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,   502,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1237,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,  1176,    -1,    -1,
    1271,  1272,  1181,  1274,    -1,    -1,  1277,    69,    -1,    -1,
     134,   135,    -1,    -1,    -1,    -1,   561,  1196,  1197,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    56,    -1,    -1,    -1,    -1,   593,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   614,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1363,    -1,  1365,    -1,    -1,    -1,  1277,    -1,
    1371,    -1,  1373,  1374,  1375,    69,    -1,  1378,    -1,  1380,
      -1,    -1,  1383,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,  1393,  1394,    -1,    -1,  1397,    -1,    -1,    -1,
      -1,    -1,    -1,  1404,    -1,    -1,   198,    -1,    30,    31,
    1411,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,  1452,    -1,    -1,    -1,    -1,    -1,   732,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1375,  1468,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1479,  1480,
      -1,    -1,    -1,    -1,    -1,    -1,  1487,    -1,  1489,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   773,    -1,
     775,    -1,  1411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1511,    -1,  1513,    -1,    -1,    -1,    -1,    59,    -1,  1520,
      -1,    -1,    -1,    -1,    -1,   268,    -1,   270,   803,   804,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,  1452,   819,   820,   821,   822,   823,    -1,
      -1,    -1,    -1,   828,    -1,    -1,    -1,  1558,  1559,  1560,
      -1,   103,    -1,    -1,  1565,   840,  1567,    -1,    -1,   111,
      -1,    -1,  1573,   195,  1575,    -1,    -1,   119,   120,   121,
     122,   123,   124,   326,    -1,    -1,    -1,   862,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,    -1,
      -1,   876,    -1,   878,   879,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   902,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   911,    50,    51,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    27,    28,    -1,    70,    31,   934,
      -1,    -1,   937,    -1,    -1,    78,    79,    80,    81,   412,
      -1,    -1,   415,    -1,    -1,  1676,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,   959,    -1,    -1,    -1,    -1,    -1,
     103,    -1,  1693,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,  1712,    57,    -1,    -1,    -1,    -1,  1718,    -1,    -1,
      -1,    -1,    -1,    -1,    69,   138,   139,    -1,  1729,    -1,
    1005,  1006,    -1,    -1,  1735,    -1,    -1,    -1,  1739,   152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,    -1,
    1761,    -1,    -1,    -1,  1039,  1040,    -1,  1042,   181,    -1,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,    -1,    -1,
    1801,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1811,    -1,   555,    -1,  1089,   558,    -1,    -1,   561,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1828,  1103,    -1,
      -1,  1106,    10,    11,    12,    -1,  1837,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   218,    -1,    -1,    -1,    -1,
     593,  1852,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1176,    -1,    -1,   647,   648,  1181,    -1,    -1,    -1,
      -1,    -1,    -1,   656,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1196,  1197,    -1,  1199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1216,    -1,  1218,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,   351,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   360,    -1,    -1,
      -1,    -1,    -1,    -1,   367,    10,    11,    12,    -1,  1274,
      -1,    -1,  1277,   376,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   387,    30,    31,   195,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    38,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,   804,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,   817,   818,   819,   820,   821,   822,
     823,    78,    79,    80,    81,   828,    83,    84,    -1,   462,
      -1,    -1,    -1,    -1,    91,    -1,  1371,    -1,  1373,  1374,
    1375,    -1,    -1,  1378,    -1,  1380,   103,    -1,  1383,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1394,
      -1,    -1,  1397,    -1,    -1,    -1,    -1,   124,    -1,   502,
      -1,    -1,    -1,    -1,    -1,    -1,  1411,    -1,    -1,    -1,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   901,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,   917,   172,    -1,  1452,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,   930,   561,   186,
      -1,   934,    -1,  1468,   191,    -1,    -1,    -1,    -1,   196,
      -1,    -1,    -1,    -1,  1479,  1480,    -1,    -1,    -1,    -1,
      -1,    -1,   955,    -1,  1489,    -1,    -1,    -1,    -1,    -1,
     593,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   614,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,  1019,    -1,    -1,  1022,
      -1,  1024,    69,  1558,  1559,  1560,    -1,    -1,    -1,    -1,
    1565,    -1,  1567,    -1,    -1,    -1,  1039,  1040,    -1,    -1,
    1575,    -1,  1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,  1089,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   732,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1126,    57,  1128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
     773,    -1,   775,    -1,    -1,    -1,    -1,    -1,   195,  1152,
      -1,    -1,  1155,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
     803,   804,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,  1718,    -1,    -1,   819,   820,   821,   822,
     823,    -1,    -1,    -1,  1729,   828,  1199,    -1,    -1,    -1,
    1735,    -1,    -1,    -1,  1739,    -1,  1209,    -1,    -1,    -1,
    1213,    -1,    -1,  1216,    -1,  1218,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   862,
      -1,    -1,    -1,    -1,  1237,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   878,   879,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,  1801,    -1,  1271,  1272,
      -1,    -1,    -1,    -1,    -1,    -1,  1811,    -1,   911,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1828,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   934,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   959,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1363,    -1,  1365,    69,    -1,    -1,   561,    -1,  1371,    -1,
    1373,  1374,  1005,  1006,    -1,  1378,    -1,  1380,    -1,    -1,
    1383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   593,    -1,
      -1,  1404,    -1,    -1,    -1,    -1,  1039,  1040,    -1,  1042,
      -1,    -1,  1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1089,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1468,    -1,    -1,    -1,    -1,
    1103,    -1,    -1,  1106,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1487,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1511,    -1,
    1513,    -1,    -1,    -1,    -1,    30,    31,  1520,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,  1176,    -1,    -1,    31,    -1,  1181,    -1,
      -1,    -1,    -1,    -1,    69,  1558,  1559,  1560,    -1,    -1,
      -1,    -1,  1565,  1196,  1197,    -1,  1199,    -1,    -1,    -1,
    1573,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1216,    31,  1218,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1237,    -1,    -1,    -1,    -1,   804,
      -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,   819,   820,   821,   822,    -1,    -1,
      -1,    -1,    -1,   828,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1277,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,  1676,    -1,   160,   161,    -1,   163,   164,
     195,   166,   167,   168,    -1,   170,    -1,    -1,    -1,    -1,
    1693,   138,   139,   140,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,  1712,
     157,    -1,    -1,   160,   161,  1718,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,    -1,  1729,    -1,    -1,    -1,
      -1,    -1,  1735,    -1,   181,    -1,  1739,    -1,  1371,   934,
    1373,  1374,  1375,    -1,   191,  1378,    -1,  1380,    -1,    -1,
    1383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1761,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1411,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,  1801,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1452,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,  1837,  1468,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1040,    -1,    -1,    -1,  1852,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1089,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,  1558,  1559,  1560,    -1,    -1,
      38,    -1,  1565,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1574,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,  1199,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,  1216,    -1,  1218,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,  1237,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,
     188,    -1,    -1,   191,    -1,  1718,    -1,    -1,   196,   197,
      -1,   199,   200,    -1,    -1,    -1,  1729,    -1,    -1,    -1,
      -1,    -1,  1735,    -1,    -1,    -1,  1739,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1763,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,  1801,    -1,
      10,    11,    12,    69,    -1,    -1,  1371,    -1,  1373,  1374,
      -1,    -1,    -1,  1378,    -1,  1380,    -1,    -1,  1383,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1468,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   195,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   195,   126,   127,   128,   129,
     130,   131,    -1,  1558,  1559,  1560,    -1,   137,   138,   139,
    1565,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,   169,
      -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,    -1,   186,   187,   188,    -1,
      -1,   191,    -1,   193,   194,   195,   196,   197,    -1,   199,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1718,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,  1729,    -1,    -1,    -1,    -1,    -1,
    1735,    -1,    -1,    -1,  1739,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,  1801,   108,   109,   110,
     111,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   195,   126,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,   169,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,   193,   194,   195,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,   169,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,   194,    -1,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,   194,
     195,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,   193,   194,   195,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
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
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
     169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,   193,   194,   195,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,   169,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,   193,   194,   195,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,   169,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,   194,    -1,   196,   197,    -1,   199,   200,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,   193,   194,   195,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
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
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
     169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,   193,   194,   195,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    99,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,   169,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,   169,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,   194,   195,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,   169,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,   193,   194,   195,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
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
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
     169,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,   193,   194,   195,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,   169,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,   193,   194,   195,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,   169,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,   194,   195,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,   169,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,   193,   194,    -1,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
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
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,   193,   194,    -1,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,   193,   194,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,   194,    -1,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,   194,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,   193,   194,    -1,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      11,    12,   191,    -1,   193,    -1,    -1,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      31,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
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
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    -1,   170,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    12,   191,    -1,
      -1,   194,    -1,   196,   197,    -1,   199,   200,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    -1,   170,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,    -1,    -1,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    10,    11,    12,    -1,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   193,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,    -1,    -1,    -1,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    10,    11,    12,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,   193,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
     193,    11,    12,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,   193,    -1,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    10,    11,    12,    -1,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
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
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   193,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,   192,    -1,    -1,    -1,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    32,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    -1,
      -1,    -1,    -1,   196,   197,    -1,   199,   200,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    -1,    -1,    -1,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,
     197,    -1,   199,   200,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
      -1,    -1,   191,    -1,    -1,    -1,    -1,   196,   197,    -1,
     199,   200,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,   164,    -1,   166,   167,   168,    -1,    -1,
      -1,   172,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,   187,   188,    -1,    -1,
     191,    -1,    -1,    -1,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,   187,   188,    -1,    -1,   191,    10,
      11,    12,    -1,   196,   197,    -1,   199,   200,     3,     4,
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
      -1,    -1,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    -1,    -1,    -1,   172,    -1,    -1,
     175,    -1,   193,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,   187,   188,    -1,    -1,   191,    10,    11,    12,
      -1,   196,   197,    -1,   199,   200,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    -1,   175,    -1,
     193,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,   186,
     187,   188,    -1,    -1,   191,    -1,    -1,    -1,    -1,   196,
     197,    -1,   199,   200,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   157,    -1,
      -1,    -1,    -1,    -1,   163,   164,    -1,   166,   167,   168,
     169,    27,   171,    29,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,    -1,   196,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   157,    10,    11,    12,    13,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,    -1,    -1,   174,    27,
      -1,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,    -1,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,   169,    -1,   171,    -1,    -1,   174,     3,     4,    -1,
       6,     7,    -1,   181,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   191,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    27,    -1,    29,    31,    31,    33,    34,    35,    36,
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
      -1,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,    -1,    -1,   174,     3,
       4,    -1,     6,     7,    -1,   181,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,   195,
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
      -1,    -1,    -1,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,   169,    -1,   171,    -1,    -1,
     174,     3,     4,    -1,     6,     7,    -1,   181,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,
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
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,
      -1,   163,   164,    -1,   166,   167,   168,   169,    -1,   171,
      -1,    -1,   174,    -1,     3,     4,    -1,     6,     7,   181,
     182,    10,    11,    12,    13,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
      -1,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
     169,    -1,   171,    -1,    -1,   174,     3,     4,     5,     6,
       7,    -1,   181,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,   164,    -1,   166,
     167,   168,   169,    -1,   171,   172,    -1,   174,    10,    11,
      12,    -1,    -1,    -1,   181,   182,    -1,   184,    -1,   186,
     187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
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
      -1,     3,     4,    69,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
      -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,   192,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   192,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   157,    -1,    -1,    -1,    -1,
      -1,   163,   164,    -1,   166,   167,   168,   169,    27,   171,
      29,    -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   157,    -1,
      -1,    -1,    -1,    -1,   163,   164,    -1,   166,   167,   168,
     169,    27,   171,    29,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    32,    -1,
      -1,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,   157,    56,    -1,    58,    -1,    -1,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,    70,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,   138,   139,    -1,   141,   142,   143,
     144,   145,    69,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
     164,    -1,   166,   167,   168,    50,    51,    -1,   172,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    70,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    91,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,   138,   139,    -1,   141,   142,   143,   144,
     145,    69,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   164,
      -1,   166,   167,   168,    70,    -1,    -1,   172,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   181,    83,    84,    -1,
      -1,   186,    -1,    -1,    -1,    91,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,   164,    -1,
     166,   167,   168,    70,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     186,    -1,    -1,    -1,    91,   191,    -1,    -1,   194,    -1,
     196,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    70,    -1,    72,   172,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   186,
      -1,    -1,    -1,    91,   191,    -1,    -1,    -1,    -1,   196,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,    -1,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   164,    -1,   166,   167,
     168,    70,    -1,    -1,   172,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,   186,    -1,
      -1,    -1,    91,   191,    -1,    -1,    -1,    -1,   196,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      70,    -1,    -1,   172,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   186,    -1,    -1,
      -1,    91,   191,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,   164,    -1,   166,   167,   168,    -1,
      -1,    -1,   172,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,    -1,    -1,    30,    31,   196,    33,    34,    35,
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
      -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   136,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,   136,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
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
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      69,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,   136,   157,   158,
     159,   160,   161,    -1,   163,   164,    -1,   166,   167,   168,
      -1,    -1,    -1,   172,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,   186,    91,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,   196,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,   164,    -1,   166,   167,   168,    -1,    -1,    -1,   172,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   186,    91,    -1,    -1,    -1,   191,    -1,
      -1,    -1,    -1,   196,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,   164,    -1,   166,
     167,   168,    -1,    -1,    -1,   172,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,    28,    -1,    30,    31,   196,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69
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
     434,   435,   436,   437,   438,   439,   453,   455,   457,   122,
     123,   124,   137,   157,   167,   191,   208,   241,   322,   343,
     430,   343,   191,   343,   343,   343,   108,   343,   343,   343,
     416,   417,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,    83,    91,   124,   139,   152,   191,
     219,   362,   388,   391,   396,   430,   433,   430,    38,   343,
     444,   445,   343,   124,   130,   191,   219,   254,   388,   389,
     390,   392,   396,   427,   428,   429,   437,   441,   442,   191,
     332,   393,   191,   332,   353,   333,   343,   227,   332,   191,
     191,   191,   332,   193,   343,   208,   193,   343,     3,     4,
       6,     7,    10,    11,    12,    13,    27,    29,    31,    57,
      59,    71,    72,    73,    74,    75,    76,    77,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   130,   131,   132,   133,   137,   138,   139,   140,
     157,   161,   169,   171,   174,   181,   191,   208,   209,   210,
     221,   458,   474,   475,   478,   193,   338,   340,   343,   194,
     234,   343,   111,   112,   160,   211,   212,   213,   214,   218,
      83,   196,   288,   289,   123,   130,   122,   130,    83,   290,
     191,   191,   191,   191,   208,   260,   461,   191,   191,    70,
     191,   333,    83,    90,   153,   154,   155,   450,   451,   160,
     194,   218,   218,   208,   261,   461,   161,   191,   461,   461,
      83,   188,   194,   354,    27,   331,   335,   343,   344,   430,
     434,   223,   194,   439,    90,   394,   450,    90,   450,   450,
      32,   160,   177,   462,   191,     9,   193,    38,   240,   161,
     259,   461,   124,   187,   241,   323,   193,   193,   193,   193,
     193,   193,   193,   193,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   193,    70,    70,   194,   156,   131,
     167,   169,   182,   184,   262,   321,   322,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      67,    68,   134,   135,   420,    70,   194,   425,   191,   191,
      70,   194,   191,   240,   241,    14,   343,   193,   136,    48,
     208,   415,    90,   331,   344,   156,   430,   136,   198,     9,
     401,   255,   331,   344,   430,   462,   156,   191,   395,   420,
     425,   192,   343,    32,   225,     8,   355,     9,   193,   225,
     226,   333,   334,   343,   208,   274,   229,   193,   193,   193,
     138,   139,   140,   478,   478,   177,   477,   477,   191,   111,
     478,    14,   156,   138,   139,   140,   157,   208,   210,   193,
     193,   193,   235,   115,   174,   193,   211,   213,   211,   213,
     218,   194,     9,   402,   193,   102,   160,   194,   430,     9,
     193,   130,   130,    14,     9,   193,   430,   454,   333,   331,
     344,   430,   433,   434,   192,   177,   252,   137,   430,   443,
     444,   343,   363,   364,    38,   170,   286,   287,   343,   193,
      70,   420,   153,   451,    82,   343,   430,    90,   153,   451,
     218,   207,   193,   194,   247,   257,   378,   380,    91,   191,
     356,   357,   359,   391,   436,   438,   455,    14,   102,   456,
     350,   351,   352,   284,   285,   418,   419,   192,   192,   192,
     192,   192,   195,   224,   225,   242,   249,   256,   418,   343,
     197,   199,   200,   208,   463,   464,   478,   286,   458,   191,
     461,   250,   240,   343,   343,   343,   343,    32,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   392,   343,   343,   440,   440,   343,   446,   447,
     130,   194,   209,   210,   439,   260,   208,   261,   461,   461,
     259,   241,    38,   335,   338,   340,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   161,
     194,   208,   421,   422,   423,   424,   439,   440,   343,   286,
     286,   440,   343,   443,   240,   192,   343,   191,   414,     9,
     401,   192,   192,    38,   343,    38,   343,   395,   192,   192,
     192,   437,   438,   439,   286,   194,   208,   421,   422,   439,
     192,   223,   278,   194,   340,   343,   343,    94,    32,   225,
     272,   193,    28,   102,    14,     9,   192,    32,   194,   275,
     478,    31,    91,   221,   471,   472,   473,   191,     9,    50,
      51,    56,    58,    70,   138,   139,   161,   181,   191,   219,
     221,   370,   373,   388,   396,   397,   398,   208,   476,   223,
     191,   233,   194,   193,   194,   193,   102,   160,   111,   112,
     160,   214,   215,   216,   217,   218,   214,   208,   343,   289,
     397,    83,     9,   192,   192,   192,   192,   192,   192,   192,
     193,    50,    51,   468,   469,   470,   132,   265,   191,     9,
     192,   192,   136,   198,     9,   401,   430,   343,   192,     9,
     402,    83,    85,   208,   452,   208,    70,   195,   195,   204,
     206,    32,   133,   264,   176,    54,   161,   176,   382,   344,
     136,     9,   401,   192,   156,   478,   478,    14,   355,   284,
     223,   189,     9,   402,   478,   479,   420,   425,   420,   195,
       9,   401,   178,   192,    14,   347,   243,   132,   263,   191,
     461,   343,    32,   198,   198,   136,   195,     9,   401,   343,
     462,   191,   253,   248,   258,    14,   456,   251,   240,    72,
     430,   343,   462,   198,   195,   192,   192,   198,   195,   192,
      50,    51,    70,    78,    79,    80,    91,   138,   139,   152,
     181,   208,   371,   404,   406,   407,   410,   413,   208,   430,
     430,   136,   263,   420,   425,   192,   343,   279,    75,    76,
     280,   223,   332,   223,   334,   102,    38,   137,   269,   430,
     397,   208,    32,   225,   273,   193,   276,   193,   276,     9,
     401,    91,   136,   156,     9,   401,   192,   170,   463,   464,
     465,   463,   397,   397,   397,   397,   397,   400,   403,   191,
      70,   156,   191,   397,   156,   194,    10,    11,    12,    31,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    69,   156,   462,   195,   388,   194,   237,   213,
     213,   208,   214,   214,   218,     9,   402,   195,   195,    14,
     430,   193,   178,     9,   401,   208,   266,   388,   194,   443,
     137,   430,    14,    38,   343,   343,    38,   170,   343,   198,
     343,   195,   204,   478,   266,   194,   381,    14,   192,   343,
     356,   439,   193,   478,   189,   195,    32,   466,   419,    38,
      83,   170,   421,   422,   424,   421,   422,   478,   397,   284,
     191,   388,   264,   348,   244,   343,   343,   343,   195,   191,
     286,   265,    32,   264,   478,    14,   263,   461,   392,   195,
     191,    14,    78,    79,    80,   208,   405,   405,   407,   408,
     409,    52,   191,    70,    90,   153,   191,     9,   401,   192,
     414,    38,   343,   264,   195,    75,    76,   281,   332,   225,
     195,   193,    95,   193,   269,   430,   191,   136,   268,    14,
     223,   276,   105,   106,   107,   276,   195,   478,   178,   136,
     478,   208,   471,     9,   192,   401,   136,   198,     9,   401,
     400,   365,   366,   397,   209,   356,   358,   360,   192,   130,
     209,   397,   448,   449,   397,   397,   397,    32,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   476,    83,   238,   195,   195,   217,   193,   397,
     470,   102,   103,   467,     9,   294,   192,   191,   335,   340,
     343,   430,   136,   430,   343,   198,   195,   456,   294,   162,
     175,   194,   377,   384,   162,   194,   383,   136,   193,   466,
     478,   355,   479,    83,   170,    14,    83,   462,   192,   284,
     194,   284,   191,   136,   191,   286,   192,   194,   478,   194,
     193,   478,   264,   245,   395,   286,   136,   198,     9,   401,
     406,   408,   367,   368,   407,   153,   356,   411,   412,   407,
     430,   194,   332,    32,    77,   225,   193,   334,   268,   443,
     269,   192,   397,   101,   105,   193,   343,    32,   193,   277,
     195,   178,   478,   136,   170,    32,   192,   397,   397,   192,
     198,     9,   401,   136,   136,     9,   401,   192,   136,   195,
       9,   401,   397,    32,   192,   223,   193,   193,   208,   478,
     478,   388,     4,   112,   117,   123,   125,   163,   164,   166,
     195,   295,   320,   321,   322,   327,   328,   329,   330,   418,
     443,    38,   343,   195,   194,   195,    54,   343,   343,   343,
     355,    38,    83,   170,    14,    83,   343,   191,   466,   192,
     294,   192,   284,   343,   286,   192,   294,   456,   294,   193,
     194,   191,   192,   407,   407,   192,   198,     9,   401,   136,
     136,   192,     9,   401,   294,    32,   223,   193,   192,   192,
     192,   230,   193,   193,   277,   223,   478,   478,   136,   397,
     397,   397,   356,   397,   397,   397,   194,   195,   467,   132,
     133,   182,   209,   459,   478,   267,   388,   112,   330,    31,
     125,   138,   140,   161,   167,   304,   305,   306,   307,   388,
     165,   312,   313,   128,   191,   208,   314,   315,   296,   241,
     478,     9,   193,     9,   193,   193,   456,   321,   192,   430,
     291,   161,   379,   195,   195,    83,   170,    14,    83,   343,
     286,   117,   345,   466,   195,   466,   192,   192,   195,   194,
     195,   294,   284,   136,   407,   407,   407,   356,   195,   223,
     228,   231,    32,   225,   271,   223,   192,   397,   136,   136,
     136,   223,   388,   388,   461,    14,   209,     9,   193,   194,
     459,   456,   307,   177,   194,     9,   193,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   137,   138,
     139,   141,   142,   143,   144,   145,   157,   158,   159,   169,
     171,   172,   174,   181,   182,   184,   186,   187,   208,   385,
     386,     9,   193,   161,   165,   208,   315,   316,   317,   193,
      83,   326,   240,   297,   459,   459,    14,   241,   195,   292,
     293,   459,    14,    83,   343,   192,   191,   466,   193,   194,
     318,   345,   466,   291,   195,   192,   407,   136,   136,    32,
     225,   270,   271,   223,   397,   397,   397,   195,   193,   193,
     397,   388,   300,   478,   308,   309,   396,   305,    14,    32,
      51,   310,   313,     9,    36,   192,    31,    50,    53,    14,
       9,   193,   210,   460,   326,    14,   478,   240,   193,    14,
     343,    38,    83,   376,   194,   223,   466,   318,   195,   466,
     407,   407,   223,    99,   236,   195,   208,   221,   301,   302,
     303,     9,   401,     9,   401,   195,   397,   386,   386,    59,
     311,   316,   316,    31,    50,    53,   397,    83,   177,   191,
     193,   397,   461,   397,    83,     9,   402,   223,   195,   194,
     318,    97,   193,   115,   232,   156,   102,   478,   178,   396,
     168,    14,   468,   298,   191,    38,    83,   192,   195,   223,
     193,   191,   174,   239,   208,   321,   322,   178,   397,   178,
     282,   283,   419,   299,    83,   195,   388,   237,   171,   208,
     193,   192,     9,   402,   119,   120,   121,   324,   325,   282,
      83,   267,   193,   466,   419,   479,   192,   192,   193,   193,
     194,   319,   324,    38,    83,   170,   466,   194,   223,   479,
      83,   170,    14,    83,   319,   223,   195,    38,    83,   170,
      14,    83,   343,   195,    83,   170,    14,    83,   343,    14,
      83,   343,   343
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
#line 2748 "hphp.y"
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
#line 2763 "hphp.y"
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
#line 2898 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2975 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { (yyval).reset();;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

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

  case 896:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

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

  case 902:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { (yyval).reset();;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 937:

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

  case 938:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    {;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3270 "hphp.y"
    {;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    {;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 995:

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

  case 996:

/* Line 1455 of yacc.c  */
#line 3293 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3321 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3325 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3330 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3332 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3341 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3360 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3362 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3364 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14272 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

