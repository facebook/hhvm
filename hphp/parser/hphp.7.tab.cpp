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
#include <folly/String.h>

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
  folly::split(':', attributes.text(), classes, true);
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

static int yylex(YYSTYPE* token, HPHP::Location* loc, Parser* _p) {
  return _p->scan(token, loc);
}


/* Line 189 of yacc.c  */
#line 653 "hphp.7.tab.cpp"

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
     T_KEYSET = 377,
     T_CALLABLE = 378,
     T_CLASS_C = 379,
     T_METHOD_C = 380,
     T_FUNC_C = 381,
     T_LINE = 382,
     T_FILE = 383,
     T_COMMENT = 384,
     T_DOC_COMMENT = 385,
     T_OPEN_TAG = 386,
     T_OPEN_TAG_WITH_ECHO = 387,
     T_CLOSE_TAG = 388,
     T_WHITESPACE = 389,
     T_START_HEREDOC = 390,
     T_END_HEREDOC = 391,
     T_DOLLAR_OPEN_CURLY_BRACES = 392,
     T_CURLY_OPEN = 393,
     T_DOUBLE_COLON = 394,
     T_NAMESPACE = 395,
     T_NS_C = 396,
     T_DIR = 397,
     T_NS_SEPARATOR = 398,
     T_XHP_LABEL = 399,
     T_XHP_TEXT = 400,
     T_XHP_ATTRIBUTE = 401,
     T_XHP_CATEGORY = 402,
     T_XHP_CATEGORY_LABEL = 403,
     T_XHP_CHILDREN = 404,
     T_ENUM = 405,
     T_XHP_REQUIRED = 406,
     T_TRAIT = 407,
     T_ELLIPSIS = 408,
     T_INSTEADOF = 409,
     T_TRAIT_C = 410,
     T_HH_ERROR = 411,
     T_FINALLY = 412,
     T_XHP_TAG_LT = 413,
     T_XHP_TAG_GT = 414,
     T_TYPELIST_LT = 415,
     T_TYPELIST_GT = 416,
     T_UNRESOLVED_LT = 417,
     T_COLLECTION = 418,
     T_SHAPE = 419,
     T_TYPE = 420,
     T_UNRESOLVED_TYPE = 421,
     T_NEWTYPE = 422,
     T_UNRESOLVED_NEWTYPE = 423,
     T_COMPILER_HALT_OFFSET = 424,
     T_ASYNC = 425,
     T_LAMBDA_OP = 426,
     T_LAMBDA_CP = 427,
     T_UNRESOLVED_OP = 428
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
#line 881 "hphp.7.tab.cpp"

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
#define YYLAST   18047

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  203
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  291
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1047
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1924

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   428

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   201,     2,   198,    55,    38,   202,
     193,   194,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   195,
      43,    14,    44,    31,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   200,    37,     2,   199,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   196,    36,   197,    58,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192
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
     237,   240,   244,   248,   250,   253,   255,   258,   262,   267,
     271,   273,   276,   278,   281,   284,   286,   290,   292,   296,
     299,   302,   305,   311,   316,   319,   320,   322,   324,   326,
     328,   332,   338,   347,   348,   353,   354,   361,   362,   373,
     374,   379,   382,   386,   389,   393,   396,   400,   404,   408,
     412,   416,   420,   426,   428,   430,   432,   433,   443,   444,
     455,   461,   462,   476,   477,   483,   487,   491,   494,   497,
     500,   503,   506,   509,   513,   516,   519,   523,   526,   529,
     530,   535,   545,   546,   547,   552,   555,   556,   558,   559,
     561,   562,   572,   573,   584,   585,   597,   598,   608,   609,
     620,   621,   630,   631,   641,   642,   650,   651,   660,   661,
     670,   671,   679,   680,   689,   691,   693,   695,   697,   699,
     702,   706,   710,   713,   716,   717,   720,   721,   724,   725,
     727,   731,   733,   737,   740,   741,   743,   746,   751,   753,
     758,   760,   765,   767,   772,   774,   779,   783,   789,   793,
     798,   803,   809,   815,   820,   821,   823,   825,   830,   831,
     837,   838,   841,   842,   846,   847,   855,   864,   871,   874,
     880,   887,   892,   893,   898,   904,   912,   919,   926,   934,
     944,   953,   960,   968,   974,   977,   982,   988,   992,   993,
     997,  1002,  1009,  1015,  1021,  1028,  1037,  1045,  1048,  1049,
    1051,  1054,  1057,  1061,  1066,  1071,  1075,  1077,  1079,  1082,
    1087,  1091,  1097,  1099,  1103,  1106,  1107,  1110,  1114,  1117,
    1118,  1119,  1124,  1125,  1131,  1134,  1137,  1140,  1141,  1152,
    1153,  1165,  1169,  1173,  1177,  1182,  1187,  1191,  1197,  1200,
    1203,  1204,  1211,  1217,  1222,  1226,  1228,  1230,  1234,  1239,
    1241,  1244,  1246,  1248,  1254,  1261,  1263,  1265,  1270,  1272,
    1274,  1278,  1281,  1284,  1285,  1288,  1289,  1291,  1295,  1297,
    1299,  1301,  1303,  1307,  1312,  1317,  1322,  1324,  1326,  1329,
    1332,  1335,  1339,  1343,  1345,  1347,  1349,  1351,  1355,  1357,
    1361,  1363,  1365,  1367,  1368,  1370,  1373,  1375,  1377,  1379,
    1381,  1383,  1385,  1387,  1389,  1390,  1392,  1394,  1396,  1400,
    1406,  1408,  1412,  1418,  1423,  1427,  1431,  1435,  1440,  1444,
    1448,  1452,  1455,  1458,  1460,  1462,  1466,  1470,  1472,  1474,
    1475,  1477,  1480,  1485,  1489,  1493,  1500,  1503,  1507,  1510,
    1514,  1521,  1523,  1525,  1527,  1529,  1531,  1538,  1542,  1547,
    1554,  1558,  1562,  1566,  1570,  1574,  1578,  1582,  1586,  1590,
    1594,  1598,  1602,  1605,  1608,  1611,  1614,  1618,  1622,  1626,
    1630,  1634,  1638,  1642,  1646,  1650,  1654,  1658,  1662,  1666,
    1670,  1674,  1678,  1682,  1686,  1689,  1692,  1695,  1698,  1702,
    1706,  1710,  1714,  1718,  1722,  1726,  1730,  1734,  1738,  1742,
    1748,  1753,  1757,  1759,  1762,  1765,  1768,  1771,  1774,  1777,
    1780,  1783,  1786,  1788,  1790,  1792,  1794,  1796,  1798,  1802,
    1805,  1807,  1813,  1814,  1815,  1828,  1829,  1843,  1844,  1849,
    1850,  1858,  1859,  1865,  1866,  1870,  1871,  1878,  1881,  1884,
    1889,  1891,  1893,  1899,  1903,  1909,  1913,  1916,  1917,  1920,
    1921,  1926,  1931,  1935,  1938,  1939,  1945,  1949,  1956,  1961,
    1964,  1965,  1971,  1975,  1978,  1979,  1985,  1989,  1994,  1999,
    2004,  2009,  2014,  2019,  2024,  2029,  2034,  2037,  2038,  2041,
    2042,  2045,  2046,  2051,  2056,  2061,  2066,  2068,  2070,  2072,
    2074,  2076,  2078,  2080,  2084,  2086,  2090,  2095,  2097,  2100,
    2105,  2108,  2115,  2116,  2118,  2123,  2124,  2127,  2128,  2130,
    2132,  2136,  2138,  2142,  2144,  2146,  2150,  2154,  2156,  2158,
    2160,  2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,  2178,
    2180,  2182,  2184,  2186,  2188,  2190,  2192,  2194,  2196,  2198,
    2200,  2202,  2204,  2206,  2208,  2210,  2212,  2214,  2216,  2218,
    2220,  2222,  2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,
    2240,  2242,  2244,  2246,  2248,  2250,  2252,  2254,  2256,  2258,
    2260,  2262,  2264,  2266,  2268,  2270,  2272,  2274,  2276,  2278,
    2280,  2282,  2284,  2286,  2288,  2290,  2292,  2294,  2296,  2298,
    2300,  2302,  2304,  2306,  2308,  2310,  2312,  2314,  2316,  2321,
    2323,  2325,  2327,  2329,  2331,  2333,  2337,  2339,  2343,  2345,
    2347,  2351,  2353,  2355,  2357,  2360,  2362,  2363,  2364,  2366,
    2368,  2372,  2373,  2375,  2377,  2379,  2381,  2383,  2385,  2387,
    2389,  2391,  2393,  2395,  2397,  2399,  2403,  2406,  2408,  2410,
    2415,  2419,  2424,  2426,  2428,  2430,  2432,  2434,  2438,  2442,
    2446,  2450,  2454,  2458,  2462,  2466,  2470,  2474,  2478,  2482,
    2486,  2490,  2494,  2498,  2502,  2506,  2509,  2512,  2515,  2518,
    2522,  2526,  2530,  2534,  2538,  2542,  2546,  2550,  2554,  2560,
    2565,  2569,  2571,  2575,  2579,  2583,  2587,  2589,  2591,  2593,
    2595,  2599,  2603,  2607,  2610,  2611,  2613,  2614,  2616,  2617,
    2623,  2627,  2631,  2633,  2635,  2637,  2639,  2643,  2646,  2648,
    2650,  2652,  2654,  2656,  2660,  2662,  2664,  2666,  2669,  2672,
    2677,  2681,  2686,  2688,  2690,  2692,  2696,  2698,  2701,  2702,
    2708,  2712,  2716,  2718,  2722,  2724,  2727,  2728,  2734,  2738,
    2741,  2742,  2746,  2747,  2752,  2755,  2756,  2760,  2764,  2766,
    2767,  2769,  2771,  2773,  2775,  2779,  2781,  2783,  2785,  2789,
    2791,  2793,  2797,  2801,  2804,  2809,  2812,  2817,  2823,  2829,
    2835,  2841,  2843,  2845,  2847,  2849,  2851,  2853,  2857,  2861,
    2866,  2871,  2875,  2877,  2879,  2881,  2883,  2887,  2889,  2894,
    2898,  2902,  2904,  2906,  2908,  2910,  2912,  2916,  2920,  2925,
    2930,  2934,  2936,  2938,  2946,  2956,  2964,  2971,  2980,  2982,
    2987,  2992,  2994,  2996,  2998,  3003,  3006,  3008,  3009,  3011,
    3013,  3015,  3019,  3023,  3027,  3028,  3030,  3032,  3036,  3040,
    3043,  3047,  3054,  3055,  3057,  3062,  3065,  3066,  3072,  3076,
    3080,  3082,  3089,  3094,  3099,  3102,  3105,  3106,  3112,  3116,
    3120,  3122,  3125,  3126,  3132,  3136,  3140,  3142,  3145,  3148,
    3150,  3153,  3155,  3160,  3164,  3168,  3175,  3179,  3181,  3183,
    3185,  3190,  3195,  3200,  3205,  3210,  3215,  3218,  3221,  3226,
    3229,  3232,  3234,  3238,  3242,  3246,  3247,  3250,  3256,  3263,
    3270,  3278,  3280,  3283,  3285,  3288,  3290,  3295,  3297,  3302,
    3306,  3307,  3309,  3313,  3316,  3320,  3322,  3324,  3325,  3326,
    3329,  3332,  3335,  3338,  3340,  3343,  3348,  3351,  3357,  3361,
    3363,  3365,  3366,  3370,  3375,  3381,  3385,  3387,  3390,  3391,
    3396,  3398,  3402,  3405,  3410,  3416,  3419,  3422,  3424,  3426,
    3428,  3430,  3434,  3437,  3439,  3448,  3455,  3457
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     204,     0,    -1,    -1,   205,   206,    -1,   206,   207,    -1,
      -1,   227,    -1,   244,    -1,   251,    -1,   248,    -1,   258,
      -1,   470,    -1,   129,   193,   194,   195,    -1,   159,   220,
     195,    -1,    -1,   159,   220,   196,   208,   206,   197,    -1,
      -1,   159,   196,   209,   206,   197,    -1,   117,   215,   195,
      -1,   117,   111,   215,   195,    -1,   117,   112,   215,   195,
      -1,   117,   213,   196,   218,   197,   195,    -1,   117,   111,
     213,   196,   215,   197,   195,    -1,   117,   112,   213,   196,
     215,   197,   195,    -1,   224,   195,    -1,    81,    -1,   103,
      -1,   165,    -1,   166,    -1,   168,    -1,   170,    -1,   169,
      -1,   139,    -1,   140,    -1,   141,    -1,   210,    -1,   142,
      -1,   171,    -1,   132,    -1,   133,    -1,   124,    -1,   123,
      -1,   122,    -1,   121,    -1,   120,    -1,   119,    -1,   112,
      -1,   101,    -1,    97,    -1,    99,    -1,    77,    -1,    95,
      -1,    12,    -1,   118,    -1,   109,    -1,    57,    -1,   173,
      -1,   131,    -1,   159,    -1,    72,    -1,    10,    -1,    11,
      -1,   114,    -1,   117,    -1,   125,    -1,    73,    -1,   137,
      -1,    71,    -1,     7,    -1,     6,    -1,   116,    -1,   138,
      -1,    13,    -1,    92,    -1,     4,    -1,     3,    -1,   113,
      -1,    76,    -1,    75,    -1,   107,    -1,   108,    -1,   110,
      -1,   104,    -1,    27,    -1,    29,    -1,   111,    -1,    74,
      -1,   105,    -1,   176,    -1,    96,    -1,    98,    -1,   100,
      -1,   106,    -1,    93,    -1,    94,    -1,   102,    -1,   115,
      -1,   126,    -1,   211,    -1,   130,    -1,   220,   162,    -1,
     162,   220,   162,    -1,   214,     9,   216,    -1,   216,    -1,
     214,   414,    -1,   220,    -1,   162,   220,    -1,   220,   102,
     210,    -1,   162,   220,   102,   210,    -1,   217,     9,   219,
      -1,   219,    -1,   217,   414,    -1,   216,    -1,   111,   216,
      -1,   112,   216,    -1,   210,    -1,   220,   162,   210,    -1,
     220,    -1,   159,   162,   220,    -1,   162,   220,    -1,   221,
     475,    -1,   221,   475,    -1,   224,     9,   471,    14,   408,
      -1,   112,   471,    14,   408,    -1,   225,   226,    -1,    -1,
     227,    -1,   244,    -1,   251,    -1,   258,    -1,   196,   225,
     197,    -1,    74,   334,   227,   280,   282,    -1,    74,   334,
      32,   225,   281,   283,    77,   195,    -1,    -1,    94,   334,
     228,   274,    -1,    -1,    93,   229,   227,    94,   334,   195,
      -1,    -1,    96,   193,   336,   195,   336,   195,   336,   194,
     230,   272,    -1,    -1,   104,   334,   231,   277,    -1,   108,
     195,    -1,   108,   345,   195,    -1,   110,   195,    -1,   110,
     345,   195,    -1,   113,   195,    -1,   113,   345,   195,    -1,
      27,   108,   195,    -1,   118,   290,   195,    -1,   124,   292,
     195,    -1,    92,   335,   195,    -1,   151,   335,   195,    -1,
     126,   193,   467,   194,   195,    -1,   195,    -1,    86,    -1,
      87,    -1,    -1,    98,   193,   345,   102,   271,   270,   194,
     232,   273,    -1,    -1,    98,   193,   345,    28,   102,   271,
     270,   194,   233,   273,    -1,   100,   193,   276,   194,   275,
      -1,    -1,   114,   236,   115,   193,   399,    83,   194,   196,
     225,   197,   238,   234,   241,    -1,    -1,   114,   236,   176,
     235,   239,    -1,   116,   345,   195,    -1,   109,   210,   195,
      -1,   345,   195,    -1,   337,   195,    -1,   338,   195,    -1,
     339,   195,    -1,   340,   195,    -1,   341,   195,    -1,   113,
     340,   195,    -1,   342,   195,    -1,   343,   195,    -1,   113,
     342,   195,    -1,   344,   195,    -1,   210,    32,    -1,    -1,
     196,   237,   225,   197,    -1,   238,   115,   193,   399,    83,
     194,   196,   225,   197,    -1,    -1,    -1,   196,   240,   225,
     197,    -1,   176,   239,    -1,    -1,    38,    -1,    -1,   111,
      -1,    -1,   243,   242,   474,   245,   193,   286,   194,   479,
     320,    -1,    -1,   324,   243,   242,   474,   246,   193,   286,
     194,   479,   320,    -1,    -1,   431,   323,   243,   242,   474,
     247,   193,   286,   194,   479,   320,    -1,    -1,   169,   210,
     249,    32,   492,   469,   196,   293,   197,    -1,    -1,   431,
     169,   210,   250,    32,   492,   469,   196,   293,   197,    -1,
      -1,   264,   261,   252,   265,   266,   196,   296,   197,    -1,
      -1,   431,   264,   261,   253,   265,   266,   196,   296,   197,
      -1,    -1,   131,   262,   254,   267,   196,   296,   197,    -1,
      -1,   431,   131,   262,   255,   267,   196,   296,   197,    -1,
      -1,   130,   257,   406,   265,   266,   196,   296,   197,    -1,
      -1,   171,   263,   259,   266,   196,   296,   197,    -1,    -1,
     431,   171,   263,   260,   266,   196,   296,   197,    -1,   474,
      -1,   163,    -1,   474,    -1,   474,    -1,   130,    -1,   123,
     130,    -1,   123,   122,   130,    -1,   122,   123,   130,    -1,
     122,   130,    -1,   132,   399,    -1,    -1,   133,   268,    -1,
      -1,   132,   268,    -1,    -1,   399,    -1,   268,     9,   399,
      -1,   399,    -1,   269,     9,   399,    -1,   136,   271,    -1,
      -1,   443,    -1,    38,   443,    -1,   137,   193,   456,   194,
      -1,   227,    -1,    32,   225,    97,   195,    -1,   227,    -1,
      32,   225,    99,   195,    -1,   227,    -1,    32,   225,    95,
     195,    -1,   227,    -1,    32,   225,   101,   195,    -1,   210,
      14,   408,    -1,   276,     9,   210,    14,   408,    -1,   196,
     278,   197,    -1,   196,   195,   278,   197,    -1,    32,   278,
     105,   195,    -1,    32,   195,   278,   105,   195,    -1,   278,
     106,   345,   279,   225,    -1,   278,   107,   279,   225,    -1,
      -1,    32,    -1,   195,    -1,   280,    75,   334,   227,    -1,
      -1,   281,    75,   334,    32,   225,    -1,    -1,    76,   227,
      -1,    -1,    76,    32,   225,    -1,    -1,   285,     9,   432,
     326,   493,   172,    83,    -1,   285,     9,   432,   326,   493,
      38,   172,    83,    -1,   285,     9,   432,   326,   493,   172,
      -1,   285,   414,    -1,   432,   326,   493,   172,    83,    -1,
     432,   326,   493,    38,   172,    83,    -1,   432,   326,   493,
     172,    -1,    -1,   432,   326,   493,    83,    -1,   432,   326,
     493,    38,    83,    -1,   432,   326,   493,    38,    83,    14,
     345,    -1,   432,   326,   493,    83,    14,   345,    -1,   285,
       9,   432,   326,   493,    83,    -1,   285,     9,   432,   326,
     493,    38,    83,    -1,   285,     9,   432,   326,   493,    38,
      83,    14,   345,    -1,   285,     9,   432,   326,   493,    83,
      14,   345,    -1,   287,     9,   432,   493,   172,    83,    -1,
     287,     9,   432,   493,    38,   172,    83,    -1,   287,     9,
     432,   493,   172,    -1,   287,   414,    -1,   432,   493,   172,
      83,    -1,   432,   493,    38,   172,    83,    -1,   432,   493,
     172,    -1,    -1,   432,   493,    83,    -1,   432,   493,    38,
      83,    -1,   432,   493,    38,    83,    14,   345,    -1,   432,
     493,    83,    14,   345,    -1,   287,     9,   432,   493,    83,
      -1,   287,     9,   432,   493,    38,    83,    -1,   287,     9,
     432,   493,    38,    83,    14,   345,    -1,   287,     9,   432,
     493,    83,    14,   345,    -1,   289,   414,    -1,    -1,   345,
      -1,    38,   443,    -1,   172,   345,    -1,   289,     9,   345,
      -1,   289,     9,   172,   345,    -1,   289,     9,    38,   443,
      -1,   290,     9,   291,    -1,   291,    -1,    83,    -1,   198,
     443,    -1,   198,   196,   345,   197,    -1,   292,     9,    83,
      -1,   292,     9,    83,    14,   408,    -1,    83,    -1,    83,
      14,   408,    -1,   293,   294,    -1,    -1,   295,   195,    -1,
     472,    14,   408,    -1,   296,   297,    -1,    -1,    -1,   322,
     298,   328,   195,    -1,    -1,   324,   492,   299,   328,   195,
      -1,   329,   195,    -1,   330,   195,    -1,   331,   195,    -1,
      -1,   323,   243,   242,   473,   193,   300,   284,   194,   479,
     321,    -1,    -1,   431,   323,   243,   242,   474,   193,   301,
     284,   194,   479,   321,    -1,   165,   306,   195,    -1,   166,
     314,   195,    -1,   168,   316,   195,    -1,     4,   132,   399,
     195,    -1,     4,   133,   399,   195,    -1,   117,   269,   195,
      -1,   117,   269,   196,   302,   197,    -1,   302,   303,    -1,
     302,   304,    -1,    -1,   223,   158,   210,   173,   269,   195,
      -1,   305,   102,   323,   210,   195,    -1,   305,   102,   324,
     195,    -1,   223,   158,   210,    -1,   210,    -1,   307,    -1,
     306,     9,   307,    -1,   308,   396,   312,   313,    -1,   163,
      -1,    31,   309,    -1,   309,    -1,   138,    -1,   138,   179,
     492,   413,   180,    -1,   138,   179,   492,     9,   492,   180,
      -1,   399,    -1,   125,    -1,   169,   196,   311,   197,    -1,
     142,    -1,   407,    -1,   310,     9,   407,    -1,   310,   413,
      -1,    14,   408,    -1,    -1,    59,   170,    -1,    -1,   315,
      -1,   314,     9,   315,    -1,   167,    -1,   317,    -1,   210,
      -1,   128,    -1,   193,   318,   194,    -1,   193,   318,   194,
      53,    -1,   193,   318,   194,    31,    -1,   193,   318,   194,
      50,    -1,   317,    -1,   319,    -1,   319,    53,    -1,   319,
      31,    -1,   319,    50,    -1,   318,     9,   318,    -1,   318,
      36,   318,    -1,   210,    -1,   163,    -1,   167,    -1,   195,
      -1,   196,   225,   197,    -1,   195,    -1,   196,   225,   197,
      -1,   324,    -1,   125,    -1,   324,    -1,    -1,   325,    -1,
     324,   325,    -1,   119,    -1,   120,    -1,   121,    -1,   124,
      -1,   123,    -1,   122,    -1,   189,    -1,   327,    -1,    -1,
     119,    -1,   120,    -1,   121,    -1,   328,     9,    83,    -1,
     328,     9,    83,    14,   408,    -1,    83,    -1,    83,    14,
     408,    -1,   329,     9,   472,    14,   408,    -1,   112,   472,
      14,   408,    -1,   330,     9,   472,    -1,   123,   112,   472,
      -1,   123,   332,   469,    -1,   332,   469,    14,   492,    -1,
     112,   184,   474,    -1,   193,   333,   194,    -1,    72,   403,
     406,    -1,    72,   256,    -1,    71,   345,    -1,   388,    -1,
     383,    -1,   193,   345,   194,    -1,   335,     9,   345,    -1,
     345,    -1,   335,    -1,    -1,    27,    -1,    27,   345,    -1,
      27,   345,   136,   345,    -1,   193,   337,   194,    -1,   443,
      14,   337,    -1,   137,   193,   456,   194,    14,   337,    -1,
      29,   345,    -1,   443,    14,   340,    -1,    28,   345,    -1,
     443,    14,   342,    -1,   137,   193,   456,   194,    14,   342,
      -1,   346,    -1,   443,    -1,   333,    -1,   447,    -1,   446,
      -1,   137,   193,   456,   194,    14,   345,    -1,   443,    14,
     345,    -1,   443,    14,    38,   443,    -1,   443,    14,    38,
      72,   403,   406,    -1,   443,    26,   345,    -1,   443,    25,
     345,    -1,   443,    24,   345,    -1,   443,    23,   345,    -1,
     443,    22,   345,    -1,   443,    21,   345,    -1,   443,    20,
     345,    -1,   443,    19,   345,    -1,   443,    18,   345,    -1,
     443,    17,   345,    -1,   443,    16,   345,    -1,   443,    15,
     345,    -1,   443,    68,    -1,    68,   443,    -1,   443,    67,
      -1,    67,   443,    -1,   345,    34,   345,    -1,   345,    35,
     345,    -1,   345,    10,   345,    -1,   345,    12,   345,    -1,
     345,    11,   345,    -1,   345,    36,   345,    -1,   345,    38,
     345,    -1,   345,    37,   345,    -1,   345,    52,   345,    -1,
     345,    50,   345,    -1,   345,    51,   345,    -1,   345,    53,
     345,    -1,   345,    54,   345,    -1,   345,    69,   345,    -1,
     345,    55,   345,    -1,   345,    30,   345,    -1,   345,    49,
     345,    -1,   345,    48,   345,    -1,    50,   345,    -1,    51,
     345,    -1,    56,   345,    -1,    58,   345,    -1,   345,    40,
     345,    -1,   345,    39,   345,    -1,   345,    42,   345,    -1,
     345,    41,   345,    -1,   345,    43,   345,    -1,   345,    47,
     345,    -1,   345,    44,   345,    -1,   345,    46,   345,    -1,
     345,    45,   345,    -1,   345,    57,   403,    -1,   193,   346,
     194,    -1,   345,    31,   345,    32,   345,    -1,   345,    31,
      32,   345,    -1,   345,    33,   345,    -1,   466,    -1,    66,
     345,    -1,    65,   345,    -1,    64,   345,    -1,    63,   345,
      -1,    62,   345,    -1,    61,   345,    -1,    60,   345,    -1,
      73,   404,    -1,    59,   345,    -1,   411,    -1,   364,    -1,
     371,    -1,   374,    -1,   377,    -1,   363,    -1,   199,   405,
     199,    -1,    13,   345,    -1,   385,    -1,   117,   193,   387,
     414,   194,    -1,    -1,    -1,   243,   242,   193,   349,   286,
     194,   479,   347,   479,   196,   225,   197,    -1,    -1,   324,
     243,   242,   193,   350,   286,   194,   479,   347,   479,   196,
     225,   197,    -1,    -1,   189,    83,   352,   357,    -1,    -1,
     189,   190,   353,   286,   191,   479,   357,    -1,    -1,   189,
     196,   354,   225,   197,    -1,    -1,    83,   355,   357,    -1,
      -1,   190,   356,   286,   191,   479,   357,    -1,     8,   345,
      -1,     8,   342,    -1,     8,   196,   225,   197,    -1,    91,
      -1,   468,    -1,   359,     9,   358,   136,   345,    -1,   358,
     136,   345,    -1,   360,     9,   358,   136,   408,    -1,   358,
     136,   408,    -1,   359,   413,    -1,    -1,   360,   413,    -1,
      -1,   183,   193,   361,   194,    -1,   138,   193,   457,   194,
      -1,    70,   457,   200,    -1,   366,   413,    -1,    -1,   366,
       9,   345,   136,   345,    -1,   345,   136,   345,    -1,   366,
       9,   345,   136,    38,   443,    -1,   345,   136,    38,   443,
      -1,   368,   413,    -1,    -1,   368,     9,   408,   136,   408,
      -1,   408,   136,   408,    -1,   370,   413,    -1,    -1,   370,
       9,   419,   136,   419,    -1,   419,   136,   419,    -1,   139,
      70,   365,   200,    -1,   139,    70,   367,   200,    -1,   139,
      70,   369,   200,    -1,   140,    70,   380,   200,    -1,   140,
      70,   381,   200,    -1,   140,    70,   382,   200,    -1,   141,
      70,   380,   200,    -1,   141,    70,   381,   200,    -1,   141,
      70,   382,   200,    -1,   335,   413,    -1,    -1,   409,   413,
      -1,    -1,   420,   413,    -1,    -1,   399,   196,   459,   197,
      -1,   399,   196,   461,   197,    -1,   385,    70,   453,   200,
      -1,   386,    70,   453,   200,    -1,   364,    -1,   371,    -1,
     374,    -1,   377,    -1,   468,    -1,   446,    -1,    91,    -1,
     193,   346,   194,    -1,    81,    -1,   387,     9,    83,    -1,
     387,     9,    38,    83,    -1,    83,    -1,    38,    83,    -1,
     177,   163,   389,   178,    -1,   391,    54,    -1,   391,   178,
     392,   177,    54,   390,    -1,    -1,   163,    -1,   391,   393,
      14,   394,    -1,    -1,   392,   395,    -1,    -1,   163,    -1,
     164,    -1,   196,   345,   197,    -1,   164,    -1,   196,   345,
     197,    -1,   388,    -1,   397,    -1,   396,    32,   397,    -1,
     396,    51,   397,    -1,   210,    -1,    73,    -1,   111,    -1,
     112,    -1,   113,    -1,    27,    -1,    29,    -1,    28,    -1,
     114,    -1,   115,    -1,   176,    -1,   116,    -1,    74,    -1,
      75,    -1,    77,    -1,    76,    -1,    94,    -1,    95,    -1,
      93,    -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,
     100,    -1,   101,    -1,    57,    -1,   102,    -1,   104,    -1,
     105,    -1,   106,    -1,   107,    -1,   108,    -1,   110,    -1,
     109,    -1,    92,    -1,    13,    -1,   130,    -1,   131,    -1,
     132,    -1,   133,    -1,    72,    -1,    71,    -1,   125,    -1,
       5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,
     159,    -1,   117,    -1,   118,    -1,   127,    -1,   128,    -1,
     129,    -1,   124,    -1,   123,    -1,   122,    -1,   121,    -1,
     120,    -1,   119,    -1,   189,    -1,   126,    -1,   137,    -1,
     138,    -1,    10,    -1,    12,    -1,    11,    -1,   143,    -1,
     145,    -1,   144,    -1,   146,    -1,   147,    -1,   161,    -1,
     160,    -1,   188,    -1,   171,    -1,   174,    -1,   173,    -1,
     184,    -1,   186,    -1,   183,    -1,   222,   193,   288,   194,
      -1,   223,    -1,   163,    -1,   399,    -1,   407,    -1,   124,
      -1,   451,    -1,   193,   346,   194,    -1,   400,    -1,   401,
     158,   452,    -1,   400,    -1,   449,    -1,   402,   158,   452,
      -1,   399,    -1,   124,    -1,   454,    -1,   193,   194,    -1,
     334,    -1,    -1,    -1,    90,    -1,   463,    -1,   193,   288,
     194,    -1,    -1,    78,    -1,    79,    -1,    80,    -1,    91,
      -1,   146,    -1,   147,    -1,   161,    -1,   143,    -1,   174,
      -1,   144,    -1,   145,    -1,   160,    -1,   188,    -1,   154,
      90,   155,    -1,   154,   155,    -1,   407,    -1,   221,    -1,
     138,   193,   412,   194,    -1,    70,   412,   200,    -1,   183,
     193,   362,   194,    -1,   372,    -1,   375,    -1,   378,    -1,
     410,    -1,   384,    -1,   193,   408,   194,    -1,   408,    34,
     408,    -1,   408,    35,   408,    -1,   408,    10,   408,    -1,
     408,    12,   408,    -1,   408,    11,   408,    -1,   408,    36,
     408,    -1,   408,    38,   408,    -1,   408,    37,   408,    -1,
     408,    52,   408,    -1,   408,    50,   408,    -1,   408,    51,
     408,    -1,   408,    53,   408,    -1,   408,    54,   408,    -1,
     408,    55,   408,    -1,   408,    49,   408,    -1,   408,    48,
     408,    -1,   408,    69,   408,    -1,    56,   408,    -1,    58,
     408,    -1,    50,   408,    -1,    51,   408,    -1,   408,    40,
     408,    -1,   408,    39,   408,    -1,   408,    42,   408,    -1,
     408,    41,   408,    -1,   408,    43,   408,    -1,   408,    47,
     408,    -1,   408,    44,   408,    -1,   408,    46,   408,    -1,
     408,    45,   408,    -1,   408,    31,   408,    32,   408,    -1,
     408,    31,    32,   408,    -1,   409,     9,   408,    -1,   408,
      -1,   223,   158,   211,    -1,   163,   158,   211,    -1,   163,
     158,   130,    -1,   223,   158,   130,    -1,   221,    -1,    82,
      -1,   468,    -1,   407,    -1,   201,   463,   201,    -1,   202,
     463,   202,    -1,   154,   463,   155,    -1,   415,   413,    -1,
      -1,     9,    -1,    -1,     9,    -1,    -1,   415,     9,   408,
     136,   408,    -1,   415,     9,   408,    -1,   408,   136,   408,
      -1,   408,    -1,    78,    -1,    79,    -1,    80,    -1,   154,
      90,   155,    -1,   154,   155,    -1,    78,    -1,    79,    -1,
      80,    -1,   210,    -1,    91,    -1,    91,    52,   418,    -1,
     416,    -1,   418,    -1,   210,    -1,    50,   417,    -1,    51,
     417,    -1,   138,   193,   421,   194,    -1,    70,   421,   200,
      -1,   183,   193,   424,   194,    -1,   373,    -1,   376,    -1,
     379,    -1,   420,     9,   419,    -1,   419,    -1,   422,   413,
      -1,    -1,   422,     9,   419,   136,   419,    -1,   422,     9,
     419,    -1,   419,   136,   419,    -1,   419,    -1,   423,     9,
     419,    -1,   419,    -1,   425,   413,    -1,    -1,   425,     9,
     358,   136,   419,    -1,   358,   136,   419,    -1,   423,   413,
      -1,    -1,   193,   426,   194,    -1,    -1,   428,     9,   210,
     427,    -1,   210,   427,    -1,    -1,   430,   428,   413,    -1,
      49,   429,    48,    -1,   431,    -1,    -1,   134,    -1,   135,
      -1,   210,    -1,   163,    -1,   196,   345,   197,    -1,   434,
      -1,   452,    -1,   210,    -1,   196,   345,   197,    -1,   436,
      -1,   452,    -1,    70,   453,   200,    -1,   196,   345,   197,
      -1,   444,   438,    -1,   193,   333,   194,   438,    -1,   455,
     438,    -1,   193,   333,   194,   438,    -1,   193,   333,   194,
     433,   435,    -1,   193,   346,   194,   433,   435,    -1,   193,
     333,   194,   433,   434,    -1,   193,   346,   194,   433,   434,
      -1,   450,    -1,   398,    -1,   448,    -1,   449,    -1,   439,
      -1,   441,    -1,   443,   433,   435,    -1,   402,   158,   452,
      -1,   445,   193,   288,   194,    -1,   446,   193,   288,   194,
      -1,   193,   443,   194,    -1,   398,    -1,   448,    -1,   449,
      -1,   439,    -1,   443,   433,   435,    -1,   442,    -1,   445,
     193,   288,   194,    -1,   193,   443,   194,    -1,   402,   158,
     452,    -1,   450,    -1,   439,    -1,   398,    -1,   364,    -1,
     407,    -1,   193,   443,   194,    -1,   193,   346,   194,    -1,
     446,   193,   288,   194,    -1,   445,   193,   288,   194,    -1,
     193,   447,   194,    -1,   348,    -1,   351,    -1,   443,   433,
     437,   475,   193,   288,   194,    -1,   193,   333,   194,   433,
     437,   475,   193,   288,   194,    -1,   402,   158,   212,   475,
     193,   288,   194,    -1,   402,   158,   452,   193,   288,   194,
      -1,   402,   158,   196,   345,   197,   193,   288,   194,    -1,
     451,    -1,   451,    70,   453,   200,    -1,   451,   196,   345,
     197,    -1,   452,    -1,    83,    -1,    84,    -1,   198,   196,
     345,   197,    -1,   198,   452,    -1,   345,    -1,    -1,   450,
      -1,   440,    -1,   441,    -1,   454,   433,   435,    -1,   401,
     158,   450,    -1,   193,   443,   194,    -1,    -1,   440,    -1,
     442,    -1,   454,   433,   434,    -1,   193,   443,   194,    -1,
     456,     9,    -1,   456,     9,   443,    -1,   456,     9,   137,
     193,   456,   194,    -1,    -1,   443,    -1,   137,   193,   456,
     194,    -1,   458,   413,    -1,    -1,   458,     9,   345,   136,
     345,    -1,   458,     9,   345,    -1,   345,   136,   345,    -1,
     345,    -1,   458,     9,   345,   136,    38,   443,    -1,   458,
       9,    38,   443,    -1,   345,   136,    38,   443,    -1,    38,
     443,    -1,   460,   413,    -1,    -1,   460,     9,   345,   136,
     345,    -1,   460,     9,   345,    -1,   345,   136,   345,    -1,
     345,    -1,   462,   413,    -1,    -1,   462,     9,   408,   136,
     408,    -1,   462,     9,   408,    -1,   408,   136,   408,    -1,
     408,    -1,   463,   464,    -1,   463,    90,    -1,   464,    -1,
      90,   464,    -1,    83,    -1,    83,    70,   465,   200,    -1,
      83,   433,   210,    -1,   156,   345,   197,    -1,   156,    82,
      70,   345,   200,   197,    -1,   157,   443,   197,    -1,   210,
      -1,    85,    -1,    83,    -1,   127,   193,   335,   194,    -1,
     128,   193,   443,   194,    -1,   128,   193,   346,   194,    -1,
     128,   193,   447,   194,    -1,   128,   193,   446,   194,    -1,
     128,   193,   333,   194,    -1,     7,   345,    -1,     6,   345,
      -1,     5,   193,   345,   194,    -1,     4,   345,    -1,     3,
     345,    -1,   443,    -1,   467,     9,   443,    -1,   402,   158,
     211,    -1,   402,   158,   130,    -1,    -1,   102,   492,    -1,
     184,   474,    14,   492,   195,    -1,   431,   184,   474,    14,
     492,   195,    -1,   186,   474,   469,    14,   492,   195,    -1,
     431,   186,   474,   469,    14,   492,   195,    -1,   212,    -1,
     492,   212,    -1,   211,    -1,   492,   211,    -1,   212,    -1,
     212,   179,   481,   180,    -1,   210,    -1,   210,   179,   481,
     180,    -1,   179,   477,   180,    -1,    -1,   492,    -1,   476,
       9,   492,    -1,   476,   413,    -1,   476,     9,   172,    -1,
     477,    -1,   172,    -1,    -1,    -1,    32,   492,    -1,   102,
     492,    -1,   103,   492,    -1,   483,   413,    -1,   480,    -1,
     482,   480,    -1,   483,     9,   484,   210,    -1,   484,   210,
      -1,   483,     9,   484,   210,   482,    -1,   484,   210,   482,
      -1,    50,    -1,    51,    -1,    -1,    91,   136,   492,    -1,
      31,    91,   136,   492,    -1,   223,   158,   210,   136,   492,
      -1,   486,     9,   485,    -1,   485,    -1,   486,   413,    -1,
      -1,   183,   193,   487,   194,    -1,   223,    -1,   210,   158,
     490,    -1,   210,   475,    -1,   179,   492,   413,   180,    -1,
     179,   492,     9,   492,   180,    -1,    31,   492,    -1,    59,
     492,    -1,   223,    -1,   138,    -1,   142,    -1,   488,    -1,
     489,   158,   490,    -1,   138,   491,    -1,   163,    -1,   193,
     111,   193,   478,   194,    32,   492,   194,    -1,   193,   492,
       9,   476,   413,   194,    -1,   492,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   735,   735,   735,   744,   746,   749,   750,   751,   752,
     753,   754,   755,   758,   760,   760,   762,   762,   764,   766,
     769,   772,   776,   780,   784,   789,   790,   791,   792,   793,
     794,   795,   796,   797,   798,   802,   803,   804,   805,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   828,   829,   830,   831,   832,   833,   834,   835,   836,
     837,   838,   839,   840,   841,   842,   843,   844,   845,   846,
     847,   848,   849,   850,   851,   852,   853,   854,   855,   856,
     857,   858,   859,   860,   861,   862,   863,   867,   871,   872,
     876,   877,   882,   884,   889,   894,   895,   896,   898,   903,
     905,   910,   915,   917,   919,   924,   925,   929,   930,   932,
     936,   943,   950,   954,   960,   962,   965,   966,   967,   968,
     971,   972,   976,   981,   981,   987,   987,   994,   993,   999,
     999,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1022,  1020,  1029,  1027,
    1034,  1044,  1038,  1048,  1046,  1050,  1051,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1074,
    1074,  1079,  1085,  1089,  1089,  1097,  1098,  1102,  1103,  1107,
    1113,  1111,  1126,  1123,  1139,  1136,  1153,  1152,  1161,  1159,
    1171,  1170,  1189,  1187,  1206,  1205,  1214,  1212,  1223,  1223,
    1230,  1229,  1241,  1239,  1252,  1253,  1257,  1260,  1263,  1264,
    1265,  1268,  1269,  1272,  1274,  1277,  1278,  1281,  1282,  1285,
    1286,  1290,  1291,  1296,  1297,  1300,  1301,  1302,  1306,  1307,
    1311,  1312,  1316,  1317,  1321,  1322,  1327,  1328,  1334,  1335,
    1336,  1337,  1340,  1343,  1345,  1348,  1349,  1353,  1355,  1358,
    1361,  1364,  1365,  1368,  1369,  1373,  1379,  1385,  1392,  1394,
    1399,  1404,  1410,  1414,  1418,  1422,  1427,  1432,  1437,  1442,
    1448,  1457,  1462,  1467,  1473,  1475,  1479,  1483,  1488,  1492,
    1495,  1498,  1502,  1506,  1510,  1514,  1519,  1527,  1529,  1532,
    1533,  1534,  1535,  1537,  1539,  1544,  1545,  1548,  1549,  1550,
    1554,  1555,  1557,  1558,  1562,  1564,  1567,  1571,  1577,  1579,
    1582,  1582,  1586,  1585,  1589,  1591,  1594,  1597,  1595,  1611,
    1607,  1621,  1623,  1625,  1627,  1629,  1631,  1633,  1637,  1638,
    1639,  1642,  1648,  1652,  1658,  1661,  1666,  1668,  1673,  1678,
    1682,  1683,  1687,  1688,  1690,  1692,  1698,  1699,  1701,  1705,
    1706,  1711,  1715,  1716,  1720,  1721,  1725,  1727,  1733,  1738,
    1739,  1741,  1745,  1746,  1747,  1748,  1752,  1753,  1754,  1755,
    1756,  1757,  1759,  1764,  1767,  1768,  1772,  1773,  1777,  1778,
    1781,  1782,  1785,  1786,  1789,  1790,  1794,  1795,  1796,  1797,
    1798,  1799,  1800,  1804,  1805,  1808,  1809,  1810,  1813,  1815,
    1817,  1818,  1821,  1823,  1827,  1829,  1833,  1837,  1841,  1846,
    1847,  1849,  1850,  1851,  1852,  1855,  1859,  1860,  1864,  1865,
    1869,  1870,  1871,  1872,  1876,  1880,  1885,  1889,  1893,  1897,
    1901,  1906,  1907,  1908,  1909,  1910,  1914,  1916,  1917,  1918,
    1921,  1922,  1923,  1924,  1925,  1926,  1927,  1928,  1929,  1930,
    1931,  1932,  1933,  1934,  1935,  1936,  1937,  1938,  1939,  1940,
    1941,  1942,  1943,  1944,  1945,  1946,  1947,  1948,  1949,  1950,
    1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,  1959,  1960,
    1961,  1962,  1963,  1964,  1966,  1967,  1969,  1970,  1972,  1973,
    1974,  1975,  1976,  1977,  1978,  1979,  1980,  1981,  1982,  1983,
    1984,  1985,  1986,  1987,  1988,  1989,  1990,  1991,  1992,  1993,
    1994,  1998,  2002,  2007,  2006,  2021,  2019,  2037,  2036,  2055,
    2054,  2073,  2072,  2090,  2090,  2105,  2105,  2123,  2124,  2125,
    2130,  2132,  2136,  2140,  2146,  2150,  2156,  2158,  2162,  2164,
    2168,  2172,  2173,  2177,  2179,  2183,  2185,  2186,  2189,  2193,
    2195,  2199,  2202,  2207,  2209,  2213,  2216,  2221,  2225,  2229,
    2233,  2237,  2241,  2245,  2249,  2253,  2257,  2259,  2263,  2265,
    2269,  2271,  2275,  2282,  2289,  2291,  2296,  2297,  2298,  2299,
    2300,  2301,  2302,  2304,  2305,  2309,  2310,  2311,  2312,  2316,
    2322,  2331,  2344,  2345,  2348,  2351,  2354,  2355,  2358,  2362,
    2365,  2368,  2375,  2376,  2380,  2381,  2383,  2388,  2389,  2390,
    2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2400,
    2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,  2410,
    2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,  2419,  2420,
    2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,  2429,  2430,
    2431,  2432,  2433,  2434,  2435,  2436,  2437,  2438,  2439,  2440,
    2441,  2442,  2443,  2444,  2445,  2446,  2447,  2448,  2449,  2450,
    2451,  2452,  2453,  2454,  2455,  2456,  2457,  2458,  2459,  2460,
    2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,  2472,  2477,
    2478,  2482,  2483,  2484,  2485,  2487,  2491,  2492,  2503,  2504,
    2506,  2518,  2519,  2520,  2524,  2525,  2526,  2530,  2531,  2532,
    2535,  2537,  2541,  2542,  2543,  2544,  2546,  2547,  2548,  2549,
    2550,  2551,  2552,  2553,  2554,  2555,  2558,  2563,  2564,  2565,
    2567,  2568,  2570,  2571,  2572,  2573,  2574,  2575,  2576,  2578,
    2580,  2582,  2584,  2586,  2587,  2588,  2589,  2590,  2591,  2592,
    2593,  2594,  2595,  2596,  2597,  2598,  2599,  2600,  2601,  2602,
    2604,  2606,  2608,  2610,  2611,  2614,  2615,  2619,  2623,  2625,
    2629,  2630,  2634,  2637,  2640,  2643,  2649,  2650,  2651,  2652,
    2653,  2654,  2655,  2660,  2662,  2666,  2667,  2670,  2671,  2675,
    2678,  2680,  2682,  2686,  2687,  2688,  2689,  2692,  2696,  2697,
    2698,  2699,  2703,  2705,  2712,  2713,  2714,  2715,  2716,  2717,
    2719,  2720,  2722,  2723,  2724,  2728,  2730,  2734,  2736,  2739,
    2742,  2744,  2746,  2749,  2751,  2755,  2757,  2760,  2763,  2769,
    2771,  2774,  2775,  2780,  2783,  2787,  2787,  2792,  2795,  2796,
    2800,  2801,  2805,  2806,  2807,  2811,  2816,  2821,  2822,  2826,
    2831,  2836,  2837,  2841,  2842,  2847,  2849,  2854,  2865,  2879,
    2891,  2906,  2907,  2908,  2909,  2910,  2911,  2912,  2922,  2931,
    2933,  2935,  2939,  2940,  2941,  2942,  2943,  2959,  2960,  2962,
    2964,  2971,  2972,  2973,  2974,  2975,  2976,  2977,  2978,  2980,
    2985,  2989,  2990,  2994,  2997,  3004,  3008,  3017,  3024,  3032,
    3034,  3035,  3039,  3040,  3041,  3043,  3048,  3049,  3060,  3061,
    3062,  3063,  3074,  3077,  3080,  3081,  3082,  3083,  3094,  3098,
    3099,  3100,  3102,  3103,  3104,  3108,  3110,  3113,  3115,  3116,
    3117,  3118,  3121,  3123,  3124,  3128,  3130,  3133,  3135,  3136,
    3137,  3141,  3143,  3146,  3149,  3151,  3153,  3157,  3158,  3160,
    3161,  3167,  3168,  3170,  3180,  3182,  3184,  3187,  3188,  3189,
    3193,  3194,  3195,  3196,  3197,  3198,  3199,  3200,  3201,  3202,
    3203,  3207,  3208,  3212,  3214,  3222,  3224,  3228,  3232,  3237,
    3241,  3249,  3250,  3254,  3255,  3261,  3262,  3271,  3272,  3280,
    3283,  3287,  3290,  3295,  3300,  3302,  3303,  3304,  3308,  3309,
    3313,  3314,  3317,  3322,  3323,  3327,  3330,  3332,  3336,  3342,
    3343,  3344,  3348,  3352,  3362,  3370,  3372,  3376,  3378,  3383,
    3389,  3392,  3397,  3402,  3404,  3411,  3414,  3417,  3418,  3421,
    3424,  3425,  3430,  3432,  3436,  3442,  3452,  3453
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
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_VEC", "T_KEYSET",
  "T_CALLABLE", "T_CLASS_C", "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE",
  "T_COMMENT", "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO",
  "T_CLOSE_TAG", "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
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
  "static_vec_literal", "static_vec_literal_ae", "keyset_literal",
  "static_keyset_literal", "static_keyset_literal_ae", "vec_ks_expr_list",
  "static_vec_ks_expr_list", "static_vec_ks_expr_list_ae",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
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
  "hh_non_empty_constraint_list", "hh_non_empty_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_access_type_start", "hh_access_type",
  "array_typelist", "hh_type", "hh_type_opt", 0
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
     426,   427,   428,    40,    41,    59,   123,   125,    36,    96,
      93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   203,   205,   204,   206,   206,   207,   207,   207,   207,
     207,   207,   207,   207,   208,   207,   209,   207,   207,   207,
     207,   207,   207,   207,   207,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   212,   212,
     213,   213,   214,   214,   215,   216,   216,   216,   216,   217,
     217,   218,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   223,   224,   224,   225,   225,   226,   226,   226,   226,
     227,   227,   227,   228,   227,   229,   227,   230,   227,   231,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   232,   227,   233,   227,
     227,   234,   227,   235,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   237,
     236,   238,   238,   240,   239,   241,   241,   242,   242,   243,
     245,   244,   246,   244,   247,   244,   249,   248,   250,   248,
     252,   251,   253,   251,   254,   251,   255,   251,   257,   256,
     259,   258,   260,   258,   261,   261,   262,   263,   264,   264,
     264,   264,   264,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   277,   277,
     277,   277,   278,   278,   278,   279,   279,   280,   280,   281,
     281,   282,   282,   283,   283,   284,   284,   284,   284,   284,
     284,   284,   284,   285,   285,   285,   285,   285,   285,   285,
     285,   286,   286,   286,   286,   286,   286,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   289,
     289,   289,   289,   289,   289,   290,   290,   291,   291,   291,
     292,   292,   292,   292,   293,   293,   294,   295,   296,   296,
     298,   297,   299,   297,   297,   297,   297,   300,   297,   301,
     297,   297,   297,   297,   297,   297,   297,   297,   302,   302,
     302,   303,   304,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   309,   309,   309,   309,   309,   310,
     310,   311,   312,   312,   313,   313,   314,   314,   315,   316,
     316,   316,   317,   317,   317,   317,   318,   318,   318,   318,
     318,   318,   318,   319,   319,   319,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   325,   325,
     325,   325,   325,   326,   326,   327,   327,   327,   328,   328,
     328,   328,   329,   329,   330,   330,   331,   331,   332,   333,
     333,   333,   333,   333,   333,   334,   335,   335,   336,   336,
     337,   337,   337,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   345,   345,   345,   345,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   346,   346,
     346,   347,   347,   349,   348,   350,   348,   352,   351,   353,
     351,   354,   351,   355,   351,   356,   351,   357,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   361,   362,   362,
     363,   364,   364,   365,   365,   366,   366,   366,   366,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   380,   381,   381,
     382,   382,   383,   384,   385,   385,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   387,   387,   387,   387,   388,
     389,   389,   390,   390,   391,   391,   392,   392,   393,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   398,   399,
     399,   400,   400,   400,   400,   400,   401,   401,   402,   402,
     402,   403,   403,   403,   404,   404,   404,   405,   405,   405,
     406,   406,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     409,   409,   410,   410,   410,   410,   411,   411,   411,   411,
     411,   411,   411,   412,   412,   413,   413,   414,   414,   415,
     415,   415,   415,   416,   416,   416,   416,   416,   417,   417,
     417,   417,   418,   418,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   420,   420,   421,   421,   422,
     422,   422,   422,   423,   423,   424,   424,   425,   425,   426,
     426,   427,   427,   428,   428,   430,   429,   431,   432,   432,
     433,   433,   434,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     446,   447,   447,   448,   448,   449,   449,   449,   450,   451,
     451,   451,   452,   452,   452,   452,   453,   453,   454,   454,
     454,   454,   454,   454,   455,   455,   455,   455,   455,   456,
     456,   456,   456,   456,   456,   457,   457,   458,   458,   458,
     458,   458,   458,   458,   458,   459,   459,   460,   460,   460,
     460,   461,   461,   462,   462,   462,   462,   463,   463,   463,
     463,   464,   464,   464,   464,   464,   464,   465,   465,   465,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   467,   467,   468,   468,   469,   469,   470,   470,   470,
     470,   471,   471,   472,   472,   473,   473,   474,   474,   475,
     475,   476,   476,   477,   478,   478,   478,   478,   479,   479,
     480,   480,   481,   482,   482,   483,   483,   483,   483,   484,
     484,   484,   485,   485,   485,   486,   486,   487,   487,   488,
     489,   490,   490,   491,   491,   492,   492,   492,   492,   492,
     492,   492,   492,   492,   492,   492,   493,   493
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     3,     1,     2,     1,     2,     3,     4,     3,
       1,     2,     1,     2,     2,     1,     3,     1,     3,     2,
       2,     2,     5,     4,     2,     0,     1,     1,     1,     1,
       3,     5,     8,     0,     4,     0,     6,     0,    10,     0,
       4,     2,     3,     2,     3,     2,     3,     3,     3,     3,
       3,     3,     5,     1,     1,     1,     0,     9,     0,    10,
       5,     0,    13,     0,     5,     3,     3,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     3,     2,     2,     0,
       4,     9,     0,     0,     4,     2,     0,     1,     0,     1,
       0,     9,     0,    10,     0,    11,     0,     9,     0,    10,
       0,     8,     0,     9,     0,     7,     0,     8,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       3,     3,     2,     2,     0,     2,     0,     2,     0,     1,
       3,     1,     3,     2,     0,     1,     2,     4,     1,     4,
       1,     4,     1,     4,     1,     4,     3,     5,     3,     4,
       4,     5,     5,     4,     0,     1,     1,     4,     0,     5,
       0,     2,     0,     3,     0,     7,     8,     6,     2,     5,
       6,     4,     0,     4,     5,     7,     6,     6,     7,     9,
       8,     6,     7,     5,     2,     4,     5,     3,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     2,     3,     4,     4,     3,     1,     1,     2,     4,
       3,     5,     1,     3,     2,     0,     2,     3,     2,     0,
       0,     4,     0,     5,     2,     2,     2,     0,    10,     0,
      11,     3,     3,     3,     4,     4,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       2,     1,     1,     5,     6,     1,     1,     4,     1,     1,
       3,     2,     2,     0,     2,     0,     1,     3,     1,     1,
       1,     1,     3,     4,     4,     4,     1,     1,     2,     2,
       2,     3,     3,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     1,     3,     5,
       1,     3,     5,     4,     3,     3,     3,     4,     3,     3,
       3,     2,     2,     1,     1,     3,     3,     1,     1,     0,
       1,     2,     4,     3,     3,     6,     2,     3,     2,     3,
       6,     1,     1,     1,     1,     1,     6,     3,     4,     6,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    12,     0,    13,     0,     4,     0,
       7,     0,     5,     0,     3,     0,     6,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     2,     0,     5,     3,     6,     4,     2,
       0,     5,     3,     2,     0,     5,     3,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     0,     2,     0,
       2,     0,     4,     4,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     4,     1,     2,     4,
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
       3,     4,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     1,     3,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     2,     2,     4,
       3,     4,     1,     1,     1,     3,     1,     2,     0,     5,
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
       2,     2,     2,     1,     2,     4,     2,     5,     3,     1,
       1,     0,     3,     4,     5,     3,     1,     2,     0,     4,
       1,     3,     2,     4,     5,     2,     2,     1,     1,     1,
       1,     3,     2,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   430,     0,     0,   845,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   936,
       0,   924,   716,     0,   722,   723,   724,    25,   787,   912,
     913,   154,   155,   725,     0,   135,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   189,     0,     0,     0,     0,
       0,     0,   396,   397,   398,   401,   400,   399,     0,     0,
       0,     0,   218,     0,     0,     0,    32,    33,    34,   729,
     731,   732,   726,   727,     0,     0,     0,   733,   728,     0,
     700,    27,    28,    29,    31,    30,     0,   730,     0,     0,
       0,     0,   734,   402,   535,     0,   153,   125,     0,   717,
       0,     0,     4,   115,   117,   786,     0,   699,     0,     6,
     188,     7,     9,     8,    10,     0,     0,   394,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   441,   901,
     902,   517,   513,   514,   515,   516,   424,   520,     0,   423,
     872,   701,   708,     0,   789,   512,   393,   875,   876,   887,
     442,     0,     0,   445,   444,   873,   874,   871,   908,   911,
     502,   788,    11,   401,   400,   399,     0,     0,    31,     0,
     115,   188,     0,   980,   442,   979,     0,   977,   976,   519,
       0,   431,   438,   436,     0,     0,   484,   485,   486,   487,
     511,   509,   508,   507,   506,   505,   504,   503,    25,   912,
     725,   703,    32,    33,    34,     0,     0,  1000,   894,   701,
       0,   702,   465,     0,   463,     0,   940,     0,   796,   422,
     712,   208,     0,  1000,   421,   711,   706,     0,   721,   702,
     919,   920,   926,   918,   713,     0,     0,   715,   510,     0,
       0,     0,     0,   427,     0,   133,   429,     0,     0,   139,
     141,     0,     0,   143,     0,    75,    74,    69,    68,    60,
      61,    52,    72,    83,    84,     0,    55,     0,    67,    59,
      65,    86,    78,    77,    50,    73,    93,    94,    51,    89,
      48,    90,    49,    91,    47,    95,    82,    87,    92,    79,
      80,    54,    81,    85,    46,    76,    62,    96,    70,    63,
      53,    45,    44,    43,    42,    41,    40,    64,    97,    99,
      57,    38,    39,    66,  1038,  1039,    58,  1043,    37,    56,
      88,     0,     0,   115,    98,   991,  1037,     0,  1040,     0,
       0,   145,     0,     0,     0,   179,     0,     0,     0,     0,
       0,     0,   798,     0,   103,   105,   307,     0,     0,   306,
       0,   222,     0,   219,   312,     0,     0,     0,     0,     0,
     997,   204,   216,   932,   936,   554,   577,   577,     0,   961,
       0,   736,     0,     0,     0,   959,     0,    16,     0,   119,
     196,   210,   217,   605,   547,     0,   985,   527,   529,   531,
     849,   430,   443,     0,     0,   441,   442,   444,     0,     0,
     915,   718,     0,   719,     0,     0,     0,   178,     0,     0,
     121,   298,     0,    24,   187,     0,   215,   200,   214,   399,
     402,   188,   395,   168,   169,   170,   171,   172,   174,   175,
     177,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   924,
       0,   167,   917,   917,   946,     0,     0,     0,     0,     0,
       0,     0,     0,   392,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   464,   462,   850,
     851,     0,   917,     0,   863,   298,   298,   917,     0,   932,
       0,   188,     0,     0,   147,     0,   847,   842,   796,     0,
     443,   441,     0,   944,     0,   552,   795,   935,   721,   443,
     441,   442,   121,     0,   298,   420,     0,   865,   714,     0,
     125,   258,     0,   534,     0,   150,     0,     0,   428,     0,
       0,     0,     0,     0,   142,   166,   144,  1038,  1039,  1035,
    1036,     0,  1042,  1028,     0,     0,     0,     0,    71,    36,
      58,    35,   992,   173,   176,   146,   125,     0,   163,   165,
       0,     0,     0,     0,   106,     0,   797,   104,    18,     0,
     100,     0,   308,     0,   148,   221,   220,     0,     0,   149,
     981,     0,     0,   443,   441,   442,   445,   444,     0,  1021,
     228,     0,   933,     0,     0,     0,     0,   796,   796,     0,
       0,   151,     0,     0,   735,   960,   787,     0,     0,   958,
     792,   957,   118,     5,    13,    14,     0,   226,     0,     0,
     540,     0,     0,   796,     0,     0,   709,   704,   541,     0,
       0,     0,     0,   849,   125,     0,   798,   848,  1047,   419,
     433,   498,   881,   900,   130,   124,   126,   127,   128,   129,
     393,     0,   518,   790,   791,   116,   796,     0,  1001,     0,
       0,     0,   798,   299,     0,   523,   190,   224,     0,   468,
     470,   469,   481,     0,     0,   501,   466,   467,   471,   473,
     472,   489,   488,   491,   490,   492,   494,   496,   495,   493,
     483,   482,   475,   476,   474,   477,   478,   480,   497,   479,
     916,     0,     0,   950,     0,   796,   984,     0,   983,  1000,
     878,   206,   198,   212,     0,   985,   202,   188,     0,   434,
     437,   439,   447,   461,   460,   459,   458,   457,   456,   455,
     454,   453,   452,   451,   450,   853,     0,   852,   855,   877,
     859,  1000,   856,     0,     0,     0,     0,     0,     0,     0,
       0,   978,   432,   840,   844,   795,   846,     0,   705,     0,
     939,     0,   938,   224,     0,   705,   923,   922,   908,   911,
       0,     0,   852,   855,   921,   856,   425,   260,   262,   125,
     538,   537,   426,     0,   125,   242,   134,   429,     0,     0,
       0,     0,     0,   254,   254,   140,   796,     0,     0,     0,
    1026,   796,     0,  1007,     0,     0,     0,     0,     0,   794,
       0,    32,    33,    34,   700,     0,     0,   738,   699,   742,
     743,   744,   746,     0,   737,   123,   745,  1000,  1041,     0,
       0,     0,     0,    19,     0,    20,     0,   101,     0,     0,
       0,   112,   798,     0,   110,   105,   102,   107,     0,   305,
     313,   310,     0,     0,   970,   975,   972,   971,   974,   973,
      12,  1019,  1020,     0,   796,     0,     0,     0,   932,   929,
       0,   551,     0,   567,   795,   553,   795,   576,   570,   573,
     969,   968,   967,     0,   963,     0,   964,   966,     0,     5,
       0,     0,     0,   599,   600,   608,   607,     0,   441,     0,
     795,   546,   550,     0,     0,   986,     0,   528,     0,     0,
    1008,   849,   284,  1046,     0,     0,   864,     0,   914,   795,
    1003,   999,   300,   301,   698,   797,   297,     0,   849,     0,
       0,   226,   525,   192,   500,     0,   584,   585,     0,   582,
     795,   945,     0,     0,   298,   228,     0,   226,     0,     0,
     224,     0,   924,   448,     0,     0,   861,   862,   879,   880,
     909,   910,     0,     0,     0,   828,   803,   804,   805,   812,
       0,    32,    33,    34,     0,     0,   816,   822,   823,   824,
     814,   815,   834,   796,     0,   842,   943,   942,     0,   226,
       0,   866,   720,     0,   264,     0,     0,   131,     0,     0,
       0,     0,     0,     0,     0,   234,   235,   246,     0,   125,
     244,   160,   254,     0,   254,     0,   795,     0,     0,     0,
       0,   795,  1027,  1029,  1006,   796,  1005,     0,   796,   767,
     768,   765,   766,   802,     0,   796,   794,   560,   579,   579,
       0,   549,     0,     0,   952,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1032,   180,     0,   183,   164,     0,     0,
     108,   113,   114,   106,   797,   111,     0,   309,     0,   982,
     152,   998,  1021,  1012,  1016,   227,   229,   319,     0,     0,
     930,     0,     0,   556,     0,   962,     0,    17,     0,   985,
     225,   319,     0,     0,   705,   543,     0,   710,   987,     0,
    1008,   532,     0,     0,  1047,     0,   289,   287,   855,   867,
    1000,   855,   868,  1002,     0,     0,   302,   122,     0,   849,
     223,     0,   849,     0,   499,   949,   948,     0,   298,     0,
       0,     0,     0,     0,     0,   226,   194,   721,   854,   298,
       0,   808,   809,   810,   811,   817,   818,   832,     0,   796,
       0,   828,   564,   581,   581,     0,   807,   836,   795,   839,
     841,   843,     0,   937,     0,   854,     0,     0,     0,     0,
     261,   539,   136,     0,   429,   234,   236,   932,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   248,     0,  1033,
       0,  1022,     0,  1025,   795,     0,     0,     0,   740,   795,
     793,     0,     0,   796,     0,     0,   781,   796,     0,   784,
     783,     0,   796,     0,   747,   785,   782,   956,     0,   796,
     750,   752,   751,     0,     0,   748,   749,   753,   755,   754,
     770,   769,   772,   771,   773,   775,   777,   776,   774,   763,
     762,   757,   758,   756,   759,   760,   761,   764,  1031,     0,
     125,     0,     0,   109,    21,   311,     0,     0,     0,  1013,
    1018,     0,   393,   934,   932,   435,   440,   446,   558,     0,
       0,    15,     0,   393,   611,     0,     0,   613,   606,   609,
       0,   604,     0,   989,     0,  1009,   536,     0,   290,     0,
       0,   285,     0,   304,   303,  1008,     0,   319,     0,   849,
       0,   298,     0,   906,   319,   985,   319,   988,     0,     0,
       0,   449,     0,     0,   820,   795,   827,   813,     0,     0,
     796,     0,     0,   826,   796,     0,   806,     0,     0,   796,
     833,   941,   319,     0,   125,     0,   257,   243,     0,     0,
       0,   233,   156,   247,     0,     0,   250,     0,   255,   256,
     125,   249,  1034,  1023,     0,  1004,     0,  1045,   801,   800,
     739,   568,   795,   559,     0,   571,   795,   578,   574,     0,
     795,   548,   741,     0,   583,   795,   951,   779,     0,     0,
       0,    22,    23,  1015,  1010,  1011,  1014,   230,     0,     0,
       0,   400,   391,     0,     0,     0,   205,   318,   320,     0,
     390,     0,     0,     0,   985,   393,     0,     0,   555,   965,
     315,   211,   602,     0,     0,   542,   530,     0,   293,   283,
       0,   286,   292,   298,   522,  1008,   393,  1008,     0,   947,
       0,   905,   393,     0,   393,   990,   319,   849,   903,   831,
     830,   819,   569,   795,   563,     0,   572,   795,   580,   575,
       0,   821,   795,   835,   393,   125,   263,   132,   137,   158,
     237,     0,   245,   251,   125,   253,  1024,     0,     0,     0,
     562,   780,   545,     0,   955,   954,   778,   125,   184,  1017,
       0,     0,     0,   993,     0,     0,     0,   231,     0,   985,
       0,   356,   352,   358,   700,    31,     0,   346,     0,   351,
     355,   368,     0,   366,   371,     0,   370,     0,   369,     0,
     188,   322,     0,   324,     0,   325,   326,     0,     0,   931,
     557,     0,   603,   601,   612,   610,   294,     0,     0,   281,
     291,     0,     0,  1008,     0,   201,   522,  1008,   907,   207,
     315,   213,   393,     0,     0,     0,   566,   825,   838,     0,
     209,   259,     0,     0,   125,   240,   157,   252,  1044,   799,
       0,     0,     0,     0,     0,     0,   418,     0,   994,     0,
     336,   340,   415,   416,   350,     0,     0,     0,   331,   664,
     663,   660,   662,   661,   681,   683,   682,   652,   622,   624,
     623,   642,   658,   657,   618,   629,   630,   632,   631,   651,
     635,   633,   634,   636,   637,   638,   639,   640,   641,   643,
     644,   645,   646,   647,   648,   650,   649,   619,   620,   621,
     625,   626,   628,   666,   667,   676,   675,   674,   673,   672,
     671,   659,   678,   668,   669,   670,   653,   654,   655,   656,
     679,   680,   684,   686,   685,   687,   688,   665,   690,   689,
     692,   694,   693,   627,   697,   695,   696,   691,   677,   617,
     363,   614,     0,   332,   384,   385,   383,   376,     0,   377,
     333,   410,     0,     0,     0,     0,   414,     0,   188,   197,
     314,     0,     0,     0,   282,   296,   904,     0,     0,   386,
     125,   191,  1008,     0,     0,   203,  1008,   829,     0,     0,
     125,   238,   138,   159,     0,   561,   544,   953,   182,   334,
     335,   413,   232,     0,   796,   796,     0,   359,   347,     0,
       0,     0,   365,   367,     0,     0,   372,   379,   380,   378,
       0,     0,   321,   995,     0,     0,     0,   417,     0,   316,
       0,   295,     0,   597,   798,   125,     0,     0,   193,   199,
       0,   565,   837,     0,     0,   161,   337,   115,     0,   338,
     339,     0,   795,     0,   795,   361,   357,   362,   615,   616,
       0,   348,   381,   382,   374,   375,   373,   411,   408,  1021,
     327,   323,   412,     0,   317,   598,   797,     0,     0,   387,
     125,   195,     0,   241,     0,   186,     0,   393,     0,   353,
     360,   364,     0,     0,   849,   329,     0,   595,   521,   524,
       0,   239,     0,     0,   162,   344,     0,   392,   354,   409,
     996,     0,   798,   404,   849,   596,   526,     0,   185,     0,
       0,   343,  1008,   849,   268,   405,   406,   407,  1047,   403,
       0,     0,     0,   342,     0,   404,     0,  1008,     0,   341,
     388,   125,   328,  1047,     0,   273,   271,     0,   125,     0,
       0,   274,     0,     0,   269,   330,     0,   389,     0,   277,
     267,     0,   270,   276,   181,   278,     0,     0,   265,   275,
       0,   266,   280,   279
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   112,   909,   633,   180,  1523,   729,
     351,   352,   353,   354,   862,   863,   864,   114,   115,   116,
     117,   118,   408,   665,   666,   547,   254,  1592,   553,  1501,
    1593,  1835,   851,   346,   576,  1795,  1097,  1290,  1854,   425,
     181,   667,   949,  1163,  1350,   122,   636,   966,   668,   687,
     970,   610,   965,   234,   528,   669,   637,   967,   427,   371,
     391,   125,   951,   912,   887,  1115,  1526,  1219,  1025,  1742,
    1596,   806,  1031,   552,   815,  1033,  1390,   798,  1014,  1017,
    1208,  1861,  1862,   655,   656,   681,   682,   358,   359,   365,
    1561,  1720,  1721,  1302,  1437,  1549,  1714,  1844,  1864,  1753,
    1799,  1800,  1801,  1536,  1537,  1538,  1539,  1755,  1756,  1762,
    1811,  1542,  1543,  1547,  1707,  1708,  1709,  1731,  1892,  1438,
    1439,   182,   127,  1878,  1879,  1712,  1441,  1442,  1443,  1444,
     128,   247,   548,   549,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,  1573,   139,   948,  1162,   140,   652,
     653,   654,   251,   400,   543,   642,   643,  1252,   644,  1253,
     141,   142,   616,   617,  1242,  1243,  1359,  1360,   143,   839,
     997,   144,   840,   998,   145,   841,   999,   619,  1245,  1362,
     146,   842,   147,   148,  1784,   149,   638,  1563,   639,  1132,
     917,  1321,  1318,  1700,  1701,   150,   151,   152,   237,   153,
     238,   248,   412,   535,   154,  1053,  1247,   846,   155,  1054,
     940,   587,  1055,  1000,  1185,  1001,  1187,  1364,  1188,  1189,
    1003,  1368,  1369,  1004,   774,   518,   194,   195,   670,   658,
     501,  1148,  1149,   760,   761,   936,   157,   240,   158,   159,
     184,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     721,   244,   245,   613,   227,   228,   724,   725,  1258,  1259,
     384,   385,   903,   170,   601,   171,   651,   172,   337,  1722,
    1774,   372,   420,   676,   677,  1047,  1143,  1299,   883,  1300,
     884,   885,   820,   821,   822,   338,   339,   848,   562,  1525,
     934
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1592
static const yytype_int16 yypact[] =
{
   -1592,   170, -1592, -1592,  5612, 13612, 13612,   -20, 13612, 13612,
   13612, 11012, 13612, 13612, -1592, 13612, 13612, 13612, 13612, 13612,
   13612, 13612, 13612, 13612, 13612, 13612, 13612, 16749, 16749, 11212,
   13612, 17357,     4,     7, -1592, -1592, -1592,   150, -1592,   176,
   -1592, -1592, -1592,   153, 13612, -1592,     7,   144,   174,   190,
   -1592,     7, 11412,  4337, 11612, -1592, 14655,  4941,   197, 13612,
    3483,    62, -1592, -1592, -1592,    55,    47,    35,   202,   207,
     227,   234, -1592,  4337,   240,   242,   177,   396,   405, -1592,
   -1592, -1592, -1592, -1592, 13612,   563,   712, -1592, -1592,  4337,
   -1592, -1592, -1592, -1592,  4337, -1592,  4337, -1592,   308,   285,
    4337,  4337, -1592,   409, -1592, 11812, -1592, -1592,   430,   505,
     520,   520, -1592,   488,   370,   557,   355, -1592,    81, -1592,
     502, -1592, -1592, -1592, -1592,  2361,   589, -1592, -1592,   358,
     364,   377,   385,   397,   407,   412,   425,  5116, -1592, -1592,
   -1592, -1592,    77,   534,   555,   560, -1592,   562,   585, -1592,
     137,   470, -1592,   501,    -3, -1592,  3081,   146, -1592, -1592,
    2956,    86,   476,    91, -1592,    94,   169,   499,   201, -1592,
   -1592,   625, -1592, -1592, -1592,   540,   509,   544, -1592, 13612,
   -1592,   502,   589, 17840,  2977, 17840, 13612, 17840, 17840, 14013,
     528, 16915, 14013, 17840,   680,  4337,   666,   666,   103,   666,
     666,   666,   666,   666,   666,   666,   666,   666, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592,    60, 13612,   558, -1592, -1592,
     587,   556,   401,   570,   401, 16749, 16963,   588,   747, -1592,
     540, -1592, 13612,   558, -1592,   627, -1592,   647,   583, -1592,
     152, -1592, -1592, -1592,   401,    86, 12012, -1592, -1592, 13612,
    9012,   801,    90, 17840, 10012, -1592, 13612, 13612,  4337, -1592,
   -1592, 10996,   619, -1592, 11396, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592,  4401, -1592,  4401, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592,    88,    96,   544, -1592, -1592, -1592,
   -1592,   623,  1379,    99, -1592, -1592,   659,   806, -1592,   672,
   15405, -1592,   636,   640, 11996, -1592,    12, 13596,  3547,  3547,
    4337,   642,   827,   646, -1592,    64, -1592, 16345,   102, -1592,
     714, -1592,   715, -1592,   833,   105, 16749, 13612, 13612,   660,
     677, -1592, -1592, 16446, 11212, 13612, 13612, 13612,   107,   283,
     275, -1592, 13812, 16749,   634, -1592,  4337, -1592,   218,   370,
   -1592, -1592, -1592, -1592, 17455,   844,   757, -1592, -1592, -1592,
      58, 13612,   667,   669, 17840,   679,  2149,   682,  5812, 13612,
   -1592,   618,   661,   574,   618,   466,   485, -1592,  4337,  4401,
     673, 10212, 14655, -1592, -1592,  2457, -1592, -1592, -1592, -1592,
   -1592,   502, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, 13612, 13612, 13612, 13612, 12212, 13612, 13612, 13612, 13612,
   13612, 13612, 13612, 13612, 13612, 13612, 13612, 13612, 13612, 13612,
   13612, 13612, 13612, 13612, 13612, 13612, 13612, 13612, 13612, 17553,
   13612, -1592, 13612, 13612, 13612, 14012,  4337,  4337,  4337,  4337,
    4337,  2361,   760,   960,  4735, 13612, 13612, 13612, 13612, 13612,
   13612, 13612, 13612, 13612, 13612, 13612, 13612, -1592, -1592, -1592,
   -1592,   974, 13612, 13612, -1592, 10212, 10212, 13612, 13612, 16446,
     686,   502, 12412, 13796, -1592, 13612, -1592,   691,   876,   731,
     694,   696, 14157,   401, 12612, -1592, 12812, -1592,   583,   698,
     700,  2177, -1592,    65, 10212, -1592,  1602, -1592, -1592, 15345,
   -1592, -1592, 10412, -1592, 13612, -1592,   802,  9212,   889,   704,
   17719,   890,   121,    57, -1592, -1592, -1592,   724, -1592, -1592,
   -1592,  4401, -1592,  2782,   713,   896, 16244,  4337, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592,   720, -1592, -1592,
     711,   721,   719,   723,    78,  4671,  4207, -1592, -1592,  4337,
    4337, 13612,   401,    62, -1592, -1592, -1592, 16244,   843, -1592,
     401,   122,   124,   716,   734,  2279,   147,   735,   736,   597,
     807,   739,   401,   126,   740, 17019,   742,   934,   935,   745,
     749, -1592,  2039,  4337, -1592, -1592,   881,  2441,    28, -1592,
   -1592, -1592,   370, -1592, -1592, -1592,   920,   821,   778,   365,
     799, 13612,   824,   953,   770,   809, -1592,   154, -1592,  4401,
    4401,   955,   801,    58, -1592,   781,   964, -1592,  4401,    69,
   -1592,   463,   164, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
    1083,  2735, -1592, -1592, -1592, -1592,   966,   797, -1592, 16749,
   13612,   784,   970, 17840,   967, -1592, -1592,   848,  3078, 11597,
   17978, 14013, 14480, 13612, 17792, 14654, 12591, 12990, 13389, 12188,
   14997, 15169, 15169, 15169, 15169,  2084,  2084,  2084,  2084,  2084,
    1289,  1289,   768,   768,   768,   103,   103,   103, -1592,   666,
   17840,   783,   787, 17067,   793,   975,     3, 13612,   212,   558,
     319, -1592, -1592, -1592,   977,   757, -1592,   502, 16547, -1592,
   -1592, -1592, 14013, 14013, 14013, 14013, 14013, 14013, 14013, 14013,
   14013, 14013, 14013, 14013, 14013, -1592, 13612,   352, -1592,   185,
   -1592,   558,   400,   795,  2807,   808,   810,   796,  3091,   127,
     812, -1592, 17840,  5042, -1592,  4337, -1592,    69,   442, 16749,
   17840, 16749, 17123,   848,    69,   401,   186, -1592,   154,   842,
     813, 13612, -1592,   188, -1592, -1592, -1592,  8812,   564, -1592,
   -1592, 17840, 17840,     7, -1592, -1592, -1592, 13612,   899, 16123,
   16244,  4337,  9412,   811,   817, -1592,   999,   922,   879,   858,
   -1592,  1008,   825,  3194,  4401, 16244, 16244, 16244, 16244, 16244,
     829,   948,   958,   959,   873,   839, 16244,   448,   875, -1592,
   -1592, -1592, -1592,   838, -1592, 17934, -1592,   220, -1592,  6012,
    4328,   841,  4207, -1592,  4207, -1592,  4337,  4337,  4207,  4207,
    4337, -1592,  1029,   845, -1592,   104, -1592, -1592,  3492, -1592,
   17934,  1026, 16749,   852, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592,   863,  1041,  4337,  4328,   855, 16446, 16648,
    1046, -1592, 13012, -1592, 13612, -1592, 13612, -1592, -1592, -1592,
   -1592, -1592, -1592,   864, -1592, 13612, -1592, -1592,  5186, -1592,
    4401,  4328,   872, -1592, -1592, -1592, -1592,  1056,   878, 13612,
   17455, -1592, -1592, 14012,   880, -1592,  4401, -1592,   883,  6212,
    1054,    44, -1592, -1592,   108,   974, -1592,  1602, -1592,  4401,
   -1592, -1592,   401, 17840, -1592, 10612, -1592, 16244,    54,   895,
    4328,   821, -1592, -1592, 14654, 13612, -1592, -1592, 13612, -1592,
   13612, -1592,  3900,   902, 10212,   807,  1057,   821,  4401,  1086,
     848,  4337, 17553,   401,  3992,   909, -1592, -1592,   180,   910,
   -1592, -1592,  1087,  3204,  3204,  5042, -1592, -1592, -1592,  1052,
     912,  1040,  1042,  1047,   266,   925, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592,  1113,   929,   691,   401,   401, 13212,   821,
    1602, -1592, -1592,  4069,   576,     7, 10012, -1592,  6412,   930,
    6612,   931, 16123, 16749,   936,   992,   401, 17934,  1116, -1592,
   -1592, -1592, -1592,   579, -1592,   367,  4401,   951,   998,  4401,
    4337,  2782, -1592, -1592, -1592,  1127, -1592,   947,   966,   814,
     814,  1076,  1076,  4206,   961,  1150, 16244, 16244, 16244, 16244,
   15550, 17455,  1771, 15695, 16244, 16244, 16244, 16244, 16012, 16244,
   16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244,
   16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244, 16244,
   16244, 16244,  4337, -1592, -1592,  1077, -1592, -1592,   965,   968,
   -1592, -1592, -1592,   262,  4671, -1592,   969, -1592, 16244,   401,
   -1592, -1592,   101, -1592,   571,  1158, -1592, -1592,   132,   980,
     401, 10812, 16749, 17840, 17171, -1592,  2217, -1592,  5412,   757,
    1158, -1592,   210,   -10, -1592, 17840,  1048,   984, -1592,   983,
    1054, -1592,  4401,   801,  4401,    46,  1168,  1100,   189, -1592,
     558,   238, -1592, -1592, 16749, 13612, 17840, 17934,   993,    54,
   -1592,   994,    54,   995, 14654, 17840, 17227,   996, 10212,   997,
    1001,  4401,  1002,  1000,  4401,   821, -1592,   583,   445, 10212,
   13612, -1592, -1592, -1592, -1592, -1592, -1592,  1058,  1010,  1183,
    1109,  5042,  5042,  5042,  5042,  1053, -1592, 17455,  5042, -1592,
   -1592, -1592, 16749, 17840,  1005, -1592,     7,  1180,  1138, 10012,
   -1592, -1592, -1592,  1021, 13612,   992,   401, 16446, 16123,  1023,
   16244,  6812,   674,  1031, 13612,    59,   494, -1592,  1038, -1592,
    4401, -1592,  1096, -1592,  3538,  1203,  1043, 16244, -1592, 16244,
   -1592,  1050,  1036,  1231,  4620,  1045, 17934,  1232,  1063, -1592,
   -1592,  1106,  1239,  1072, -1592, -1592, -1592, 16317,  1060,  1258,
   10192, 10592, 11192, 16244, 17888, 12791, 13190, 14152, 14823, 15396,
   15543, 15543, 15543, 15543,  3026,  3026,  3026,  3026,  3026,  1170,
    1170,   814,   814,   814,  1076,  1076,  1076,  1076, -1592,  1075,
   -1592,  1078,  1081, -1592, -1592, 17934,  4337,  4401,  4401, -1592,
     571,  4328,   645, -1592, 16446, -1592, -1592, 14013,   401, 13412,
    1074, -1592,  1088,   800, -1592,   191, 13612, -1592, -1592, -1592,
   13612, -1592, 13612, -1592,   801, -1592, -1592,   145,  1268,  1200,
   13612, -1592,  1092,   401, 17840,  1054,  1094, -1592,  1098,    54,
   13612, 10212,  1099, -1592, -1592,   757, -1592, -1592,  1095,  1101,
    1102, -1592,  1105,  5042, -1592,  5042, -1592, -1592,  1111,  1103,
    1285,  1164,  1107, -1592,  1297,  1110, -1592,  1173,  1118,  1305,
   -1592,   401, -1592,  1281, -1592,  1121, -1592, -1592,  1124,  1128,
     134, -1592, -1592, 17934,  1129,  1131, -1592,  4570, -1592, -1592,
   -1592, -1592, -1592, -1592,  4401, -1592,  4401, -1592, 17934, 17275,
   -1592, -1592, 16244, -1592, 16244, -1592, 16244, -1592, -1592, 16244,
   17455, -1592, -1592, 16244, -1592, 16244, -1592, 12392, 16244,  1135,
    7012, -1592, -1592,   571, -1592, -1592, -1592, -1592,   546, 14829,
    4328,  1220, -1592,  2518,  1166,  3474, -1592, -1592, -1592,   760,
   15936,   110,   111,  1152,   757,   960,   135, 16749, 17840, -1592,
   -1592, -1592,  1185,  4284,  4348, 17840, -1592,    70,  1335,  1267,
   13612, -1592, 17840, 10212,  1245,  1054,  1247,  1054,  1162, 17840,
    1169, -1592,  1304,  1177,  1854, -1592, -1592,    54, -1592, -1592,
    1229, -1592, -1592,  5042, -1592,  5042, -1592,  5042, -1592, -1592,
    5042, -1592, 17455, -1592,  1904, -1592,  8812, -1592, -1592, -1592,
   -1592,  9612, -1592, -1592, -1592,  8812, -1592,  1181, 16244, 17330,
   17934, 17934, 17934,  1238, 17934, 17378, 12392, -1592, -1592,   571,
    4328,  4328,  4337, -1592,  1362, 15840,    83, -1592, 14829,   757,
    4066, -1592,  1201, -1592,   112,  1187,   114, -1592, 15178, -1592,
   -1592, -1592,   115, -1592, -1592,  2263, -1592,  1184, -1592,  1298,
     502, -1592, 15004, -1592, 15004, -1592, -1592,  1370,   760, -1592,
     401, 14307, -1592, -1592, -1592, -1592,  1371,  1303, 13612, -1592,
   17840,  1193,  1195,  1054,   486, -1592,  1245,  1054, -1592, -1592,
   -1592, -1592,  1992,  1196,  5042,  1253, -1592, -1592, -1592,  1256,
   -1592,  8812,  9812,  9612, -1592, -1592, -1592,  8812, -1592, 17934,
   16244, 16244, 16244,  7212,  1198,  1199, -1592, 16244, -1592,  4328,
   -1592, -1592, -1592, -1592, -1592,  4401,  1320,  2518, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
     347, -1592,  1166, -1592, -1592, -1592, -1592, -1592,    79,   372,
   -1592,  1381,   116, 15405,  1298,  1382, -1592,  4401,   502, -1592,
   -1592,  1206,  1388, 13612, -1592, 17840, -1592,   129,  1207, -1592,
   -1592, -1592,  1054,   486, 14481, -1592,  1054, -1592,  5042,  5042,
   -1592, -1592, -1592, -1592,  7412, 17934, 17934, 17934, -1592, -1592,
   -1592, 17934, -1592,  1090,  1395,  1396,  1209, -1592, -1592, 16244,
   15178, 15178,  1348, -1592,  2263,  2263,   474, -1592, -1592, -1592,
   16244,  1325, -1592,  1230,  1221,   117, 16244, -1592,  4337, -1592,
   16244, 17840,  1339, -1592,  1421, -1592,  7612,  1237, -1592, -1592,
     486, -1592, -1592,  7812,  1242,  1319, -1592,  1337,  1282, -1592,
   -1592,  1340,  4401,  1261,  1320, -1592, -1592, 17934, -1592, -1592,
    1273, -1592,  1409, -1592, -1592, -1592, -1592, 17934,  1433,   597,
   -1592, -1592, 17934,  1255, 17934, -1592,   138,  1259,  8012, -1592,
   -1592, -1592,  1254, -1592,  1257,  1278,  4337,   960,  1276, -1592,
   -1592, -1592, 16244,  1279,    63, -1592,  1378, -1592, -1592, -1592,
    8212, -1592,  4328,   841, -1592,  1295,  4337,   675, -1592, 17934,
   -1592,  1277,  1453,   663,    63, -1592, -1592,  1390, -1592,  4328,
    1280, -1592,  1054,    68, -1592, -1592, -1592, -1592,  4401, -1592,
    1283,  1284,   119, -1592,   565,   663,   160,  1054,  1287, -1592,
   -1592, -1592, -1592,  4401,    92,  1462,  1401,   565, -1592,  8412,
     163,  1465,  1402, 13612, -1592, -1592,  8612, -1592,   292,  1472,
    1404, 13612, -1592, 17840, -1592,  1478,  1412, 13612, -1592, 17840,
   13612, -1592, 17840, 17840
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1592, -1592, -1592,  -558, -1592, -1592, -1592,   136,    48,   -54,
     341, -1592,  -263,  -514, -1592, -1592,   392,   233,  1498, -1592,
    2755, -1592,  -242, -1592,    38, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592,  -356, -1592, -1592,  -159,
     161,    19, -1592, -1592, -1592, -1592, -1592, -1592,    29, -1592,
   -1592, -1592, -1592, -1592, -1592,    30, -1592, -1592,  1019,  1027,
    1024,   -99,  -701,  -871,   539,   595,  -359,   298,  -944, -1592,
     -77, -1592, -1592, -1592, -1592,  -733,   140, -1592, -1592, -1592,
   -1592,  -342, -1592,  -612, -1592,  -439, -1592, -1592,   937, -1592,
     -57, -1592, -1592, -1069, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592,   -93, -1592,    -2, -1592, -1592, -1592,
   -1592, -1592,  -171, -1592,    97, -1023, -1592, -1591,  -364, -1592,
    -124,   133,  -117,  -351, -1592,  -179, -1592, -1592, -1592,   106,
     -22,     5,    25,  -720,   -69, -1592, -1592,    20, -1592,   -12,
   -1592, -1592,    -5,   -45,   -40, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592,  -578,  -859, -1592, -1592, -1592, -1592,
   -1592,  1866, -1592, -1592, -1592, -1592, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592, -1592, -1592, -1592,  1163,   487,   356,
   -1592, -1592, -1592, -1592, -1592,   419, -1592, -1592, -1592, -1592,
   -1592, -1592, -1592, -1592,  -988, -1592,  2370,    37, -1592,   908,
    -405, -1592, -1592,  -478,  3627,  2669, -1592, -1592, -1592,   496,
      -9,  -629, -1592, -1592,   573,   368,  -460, -1592,   369, -1592,
   -1592, -1592, -1592, -1592,   550, -1592, -1592, -1592,   118,  -903,
     -30,  -442,  -438, -1592,   626,  -113, -1592, -1592,    39,    42,
     489, -1592, -1592,   297,   -21, -1592,  -357,    27,  -365,    51,
    -294, -1592, -1592,  -465,  1190, -1592, -1592, -1592, -1592, -1592,
     717,   683, -1592, -1592, -1592,  -354,  -700, -1592,  1144, -1323,
   -1592,   -70,  -187,   -23,   744, -1592, -1039, -1224,  -251,   151,
   -1592,   457,   529, -1592, -1592, -1592, -1592,   483, -1592,  1791,
   -1101
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1031
static const yytype_int16 yytable[] =
{
     183,   185,   335,   187,   188,   189,   191,   192,   193,   432,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   510,   121,   226,   229,   392,   932,  1144,   647,
     395,   396,   482,   123,   124,   969,   403,   646,   250,   253,
     648,   928,   119,  1327,   769,   343,   532,   261,   504,   264,
     783,   255,   344,   946,   347,   428,   259,   481,   243,   758,
     405,  1136,  1313,   759,   718,   432,   765,   766,   236,   252,
     241,   861,   866,   242,   927,   908,  1426,   342,  1215,   253,
    1161,  1035,  1009,   402,   407,   581,   583,  1021,  1764,   813,
     422,  1388,  1609,    14,   793,   790,  1172,   -71,   794,   544,
     404,  1324,   -71,    14,   334,   -36,  1524,    14,   -35,   378,
     -36,   593,    14,   -35,   598,  1765,   544,    14,   364,  1552,
    1554,  -349,   156,  1617,  1702,  1771,  1771,   577,  1609,  1328,
     811,   872,   537,   544,   405,   889,   889,   126,  1204,   502,
     113,   889,  1788,   889,   889,   356,  1145,  -586,   209,    40,
     519,   881,   882,  1566,  1319,  -702,   502,   402,   407,   410,
     469,  -591,   499,   500,  -883,   120,   589,  1782,   788,   362,
       3,   521,   470,   186,   404,  1901,  1846,   363,   360,   722,
     856,   513,   -99,  1457,  -533,   361,  1320,   530,   578,   262,
    -895,  1146,   333,  -703,   520,   407,   -99,   246,  1894,  1831,
     249,  1908,  1251,   499,   500,  1612,   589,  -882,   763,   370,
     529,   404,  1783,   767,   536,   381,  -885,  -591,  1329,   527,
    -594,  1847,  -925,  -592,   507,   907,   590,   404,  1458,  1715,
     390,  1716,   370,  1105,  -889,  -797,   370,   370,  -797,  -884,
     857,   539,  1567,  1895,   539,  1452,  1909,   375,  -288,  -288,
    -888,   253,   550,   814,  1389,  -886,  -928,  -272,  -927,  -869,
     357,   370,  -797,   108,  1902,   503,   418,   561,  1466,  1175,
    -894,   507,   688,  1766,  1381,  1472,   423,  1474,  1610,  1611,
    1147,  -795,   503,   -71,   506,   545,   572,   431,   541,   483,
    -883,   -36,   546,   355,   -35,  1426,  1464,   594,   797,  1222,
     599,  1226,   621,  1494,  1349,  1553,  1555,  -349,  -870,  1618,
    1703,  1772,  1821,  1002,  1889,   812,   873,  1459,   874,   388,
     890,   982,   389,   604,   223,   223,  1303,  -709,  1500,  1559,
    -893,   517,  1896,  -882,   849,  1910,  1158,   256,  1367,  -892,
     506,   878,  -885,   511,  1101,  1102,   603,   607,  -925,   623,
     508,  1128,   770,   622,   393,   686,  1195,  -896,   379,  -704,
    -889,  1759,   253,   404,   856,  -884,   432,   257,   335,   226,
     615,   253,   253,  -899,  1314,  1915,  -888,   627,  1092,  1760,
     418,  -886,  -928,   258,  -927,  -869,   113,  1315,   334,  -890,
     113,   -98,   602,   345,   551,   366,   191,   508,  1761,   419,
     367,   618,   618,  1767,   671,   -98,  1316,  1582,   392,   734,
     735,   428,   929,   634,   635,   739,   683,   499,   500,   914,
     368,  1196,  1768,  1118,   418,  1769,  1574,   369,  1576,  1312,
     624,   382,   383,   373,  -870,   374,   689,   690,   691,   692,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,  1916,   719,   376,   720,   720,   723,
     334,   393,   741,  1224,  1225,   377,   571,  -710,   394,   742,
     743,   744,   745,   746,   747,   748,   749,   750,   751,   752,
     753,   754,   397,   160,  1378,  1151,   243,   720,   764,  1152,
     683,   683,   720,   768,   740,  1814,   236,   742,   241,   776,
     772,   242,   964,   209,    40,  -890,   222,   224,   657,   780,
     417,   782,   223,   728,  1815,  1169,   730,  1816,   915,   683,
     800,  -857,   418,  -593,  1728,   499,   500,   801,  1733,   802,
     424,   126,   963,   916,   113,  -857,   482,  1336,   421,   379,
    1338,  1513,   762,   433,   675,   647,   629,  1018,   333,   434,
     787,   370,  1020,   646,  1227,  1326,   648,  1177,   379,   120,
     334,   481,   435,   730,   975,   629,   499,   500,   971,  -860,
     436,   355,   355,   584,   789,   805,   868,   795,   379,  1098,
     861,  1099,   437,  -860,   406,   411,   918,   499,   500,   398,
    1224,  1225,   438,   379,  -587,   399, -1000,   439,   895,   897,
     414,   571,   370,   732,   370,   370,   370,   370,   953,   632,
     440,  -705,   382,   383,  -858,  -588,   409,   419,   108,   935,
    -589,   937,   472,  1589,   921,  -897,   404,   757,  -858,  1015,
    1016,   382,   383,   737, -1000,  1473,   379,   881,   882,  1428,
     532,  1206,  1207,   380,   223,   473,  -897,   379,   571,   475,
    1093,   382,   383,   223,   629,   606,   474,   673,   406,   505,
     223,  1011,   792,  1297,  1298,   943,   382,   383,  1520,  1521,
     223,  1729,  1730,   113,  1223,  1224,  1225,   674,   954,   580,
     582,  1391,  -891,  1787,    14,  -590,   647,  1790,  -703,  1351,
      55,   379,   509,   847,   646,   406,   386,   648,    62,    63,
      64,   173,   174,   429,   523, -1000,   961,   379,   381,   382,
     383,   531,   962,   514,   629,   867,   675,  1468,   516,  1342,
     382,   383,  1361,  1363,  1363,   470,   419,   419,  1370,   160,
    1352,  1812,  1813,   160,  1557,   522,  1456,   935,   937,  -895,
   -1000,   974,  1380, -1000,  1010,   937,   526,  1429,   902,   904,
    1890,  1891,  1430,   506,    62,    63,    64,   173,  1431,   429,
    1432,   657,  1808,  1809,   382,   383,   534,  1886,   430,  1385,
    1224,  1225,  1875,  1876,  1877,  -701,  1013,  1221,   525,   630,
     382,   383,  1900,   208,    62,    63,    64,   173,   174,   429,
    1045,  1048,   253,   483,  1428,   533,   223,  1037,  1019,   542,
    1433,  1434,  1042,  1435,   555,    50,   563, -1030,   865,   865,
     566,   466,   467,   468,   370,   469,   413,   415,   416,  1613,
     567,   573,   647,  1884,   430,   574,   586,   470,   585,  1446,
     646,   588,  1436,   648,   595,   596,   592,   597,  1897,    14,
    1030,   212,   213,   214,   608,   600,   609,   605,   649,   650,
     672,   659,   612,   660,   430,  1583,  -120,  1088,  1089,  1090,
    1871,    55,   628,   661,   386,  1113,   663,    91,    92,   685,
      93,   178,    95,  1091,   773,   775,   624,  1123,   777,  1124,
     778,   802,   784,  1479,   785,  1480,   803,   160,   544,   807,
    1126,  1176,  1470,   561,   810,   824,   823,   852,   387,   996,
     875,  1005,  1429,   850,  1135,   854,   853,  1430,   855,    62,
      63,    64,   173,  1431,   429,  1432,   871,   121,   876,   879,
     126,   880,   888,   113,   891,   220,   220,   123,   124,   886,
    1156,  1863,   893,   894,   896,   898,   119,  1028,   113,   899,
    1164,   905,   910,  1165,   911,  1166,   913,  -725,   120,   683,
     919,  1863,   920,  1332,   922,  1433,  1434,   923,  1435,   926,
    1885,   728,   930,   931,  1137,   939,   223,   941,   944,   945,
     950,   947,   126,   956,   960,   113,   762,   957,   795,   430,
     959,   968,  1100,   675,  1199,   976,   980,  1451,   612,   243,
    -707,  1022,   978,  1203,   979,   952,  1032,  1012,  1036,   236,
     120,   241,  1034,  1038,   242,  1039,  1040,  1041,  1057,  1043,
    1209,  1114,  1056,  1585,  1571,  1586,   156,  1587,  1058,  1059,
    1588,  1060,  1061,  1063,  1064,   223,   160,  1096,  1104,  1236,
    1108,   126,  1106,  1111,   113,   647,  1240,  1110,  1420,   657,
    1112,  1117,  1305,   646,  1210,   208,   648,   209,    40,   571,
    1121,   795,   126,   625,  1125,   113,   657,   631,  1131,   120,
    1133,   757,  1134,   792,  1140,  1138,   223,    50,   223,    62,
      63,    64,   173,   174,   429,   865,  1142,   865,  1159,  1171,
     120,   865,   865,  1103,   625,  1168,   631,   625,   631,   631,
    1174,  1180,  1179,  -898,  1190,  1191,   223,   370,  1250,  1306,
    1192,  1256,  1193,   212,   213,   214,  1307,  1194,  1197,  1184,
    1184,   996,  1198,  1200,  1737,  1212,  1214,   647,  1218,  1217,
    1220,  1229,  1496,   220,  1230,   646,  1234,   755,   648,    91,
      92,  1235,    93,   178,    95,  1091,   792,   121,  1505,   430,
    1334,   126,   113,   126,   113,  1827,   113,   123,   124,  1239,
    1289,  1238,  1291,   683,  1294,  1292,   119,  1301,   942,   223,
     756,   208,   108,  1304,   683,  1307,  1232,   964,  1323,   120,
    1356,   120,  1330,  1331,  1322,   223,   223,  1335,  1339,  1341,
    1337,  1343,  1355,    50,  1353,  1347,   571,  1344,  1346,   571,
     989,  1372,    62,    63,    64,    65,    66,   429,  1366,   253,
    1354,  1373,  1374,    72,   476,  1375,  1377,  1382,  1392,  1387,
    1085,  1086,  1087,  1088,  1089,  1090,  1386,   973,   847,   212,
     213,   214,  1394,  1874,  1403,  1396,  1401,  1397,  1407,  1091,
    1402,  1406,  1409,  1411,  1400,  1405,   156,  1376,  1410,   177,
    1416,  1428,    89,  1591,   478,    91,    92,  1414,    93,   178,
      95,   126,  1597,  1408,   113,   220,  1412,  1415,  1006,  1419,
    1007,  1449,   430,  1421,   220,  1603,  1422,   657,  1791,  1792,
     657,   220,  1460,  1461,  1450,  1463,   160,  1796,  1465,   120,
    1475,   220,  1467,  1471,  1483,  1477,    14,  1476,  1026,  1478,
    1485,   160,   645,  1482,  1448,  1481,  1487,  1486,  1428,  1490,
    1489,  1453,  1491,  1495,  1492,  1454,  1497,  1455,  1498,   223,
     223,  1558,  1499,   432,  1502,  1462,  1503,   996,   996,   996,
     996,  1517,  1528,  1541,   996,  1469,   683,   865,   160,   463,
     464,   465,   466,   467,   468,   113,   469,  1556,  1562,  1568,
    1569,  1484,  1744,    14,   126,  1488,  1577,   113,   470,  1429,
    1493,  1109,  1572,  1578,  1430,  1584,    62,    63,    64,   173,
    1431,   429,  1432,  1580,  1601,  1598,  1607,   612,  1120,  1710,
    1615,  1711,   120,  1616,  1717,  1723,  1724,  1726,  1727,  1738,
    1736,  1713,  1739,  1749,  1750,  1770,  1776,   160,    34,    35,
      36,  1779,  1780,  1785,  1802,  1804,  1806,  1810,  1818,  1819,
     275,   210,  1433,  1434,  1820,  1435,  1429,   220,   160,   223,
    1445,  1430,  1825,    62,    63,    64,   173,  1431,   429,  1432,
    1826,  1445,  1423,  1830,  1834,  1440,   430,  1833,   277,  -345,
    1836,  1839,  1837,  1841,  1575,  1765,  1440,  1842,  1845,  1851,
    1852,   223,  1606,  1848,  1853,  1570,  1858,   657,   683,  1860,
     208,  1865,  1873,    79,    80,    81,    82,    83,  1869,  1433,
    1434,  1872,  1435,  1881,   215,  1883,  1903,  1887,  1888,  1911,
      87,    88,    50,  1898,  1904,  1912,  1917,  1918,  1786,   996,
     564,   996,  1920,   430,    97,  1921,  1293,  1868,  1793,   223,
     736,  1579,   733,   731,  1170,   160,  1130,   160,   102,   160,
    1882,  1026,  1216,  1379,   223,   223,  1743,   557,   212,   213,
     214,   558,  1880,  1734,  1758,   217,   217,  1504,  1614,   233,
     869,  1763,  1548,  1905,  1893,  1775,  1732,  1529,   177,  1595,
     620,    89,   327,  1828,    91,    92,  1248,    93,   178,    95,
    1365,  1317,  1241,   126,   233,  1201,   113,  1186,  1357,  1778,
    1358,  1150,   331,  1725,   614,   333,   684,  1046,  1843,  1296,
    1233,  1546,   332,  1608,  1519,  1288,     0,     0,   483,     0,
       0,   120,     0,     0,  1445,     0,     0,   220,  1850,     0,
    1445,     0,  1445,     0,     0,   657,     0,     0,     0,  1440,
    1550,   223,     0,     0,     0,  1440,     0,  1440,     0,     0,
       0,  1308,  1445,     0,     0,     0,     0,   160,     0,   996,
       0,   996,     0,   996,     0,     0,   996,  1440,     0,   126,
    1741,  1595,   113,     0,     0,     0,     0,   113,   126,     0,
       0,   113,     0,  1333,     0,     0,   220,     0,     0,  1899,
       0,     0,     0,     0,     0,     0,  1906,   120,   370,  1773,
       0,   571,     0,     0,   333,     0,   120,     0,     0,     0,
       0,     0,     0,     0,  1699,     0,     0,     0,     0,     0,
       0,  1706,     0,   208,     0,   209,    40,   220,   333,   220,
     333,  1371,     0,     0,     0,     0,     0,   333,   160,     0,
    1445,     0,     0,     0,     0,    50,   612,  1026,  1823,     0,
     160,     0,     0,  1856,     0,  1440,     0,   220,  1781,  1718,
     996,     0,     0,   217,   126,     0,     0,   113,   113,   113,
     126,     0,     0,   113,     0,     0,   126,     0,     0,   113,
     432,   212,   213,   214,   223,  1803,  1805,     0,     0,     0,
       0,     0,   120,     0,     0,     0,     0,     0,   120,     0,
       0,   334,     0,     0,   120,   755,     0,    91,    92,     0,
      93,   178,    95,   233,     0,   233,     0,     0,     0,     0,
     220,  1065,  1066,  1067,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   612,     0,     0,   220,   220,   791,     0,
     108,     0,  1068,     0,     0,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,   645,     0,
     233,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1091,     0,     0,     0,     0,     0,     0,   340,     0,   571,
       0,     0,     0,     0,     0,   217,     0,     0,  1428,     0,
       0,     0,     0,     0,   217,     0,     0,     0,     0,     0,
     333,   217,     0,     0,   996,   996,     0,   126,     0,     0,
     113,   217,     0,     0,     0,     0,     0,     0,     0,  1797,
       0,     0,   233,   218,   218,     0,  1699,  1699,  1913,     0,
    1706,  1706,     0,    14,     0,   120,  1919,     0,  1428,   160,
       0,     0,  1922,     0,   370,  1923,     0,   233,     0,   126,
     233,     0,   113,     0,     0,     0,   126,     0,     0,   113,
     220,   220,     0,     0,     0,     0,  1560,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   120,     0,     0,
       0,     0,     0,    14,   120,     0,     0,     0,     0,     0,
       0,   126,   657,     0,   113,  1254,  1429,   233,     0,   645,
    1857,  1430,  1855,    62,    63,    64,   173,  1431,   429,  1432,
       0,     0,   657,   126,     0,   160,   113,     0,     0,   120,
     160,   657,  1870,     0,   160,     0,  1428,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   217,     0,     0,
       0,   120,     0,     0,     0,     0,  1429,     0,     0,  1433,
    1434,  1430,  1435,    62,    63,    64,   173,  1431,   429,  1432,
     220,     0,   126,     0,     0,   113,     0,     0,     0,   126,
       0,    14,   113,   430,     0,     0,     0,     0,     0,     0,
       0,  1581,     0,     0,     0,     0,     0,     0,     0,   233,
     120,   233,   220,     0,   837,     0,   559,   120,   560,  1433,
    1434,     0,  1435,     0,     0,     0,     0,     0,     0,     0,
     160,   160,   160,     0,     0,     0,   160,     0,     0,     0,
       0,   218,   160,   430,     0,   837,     0,     0,     0,     0,
       0,  1590,     0,     0,  1429,   645,     0,     0,     0,  1430,
     220,    62,    63,    64,   173,  1431,   429,  1432,     0,     0,
     208,     0,   900,   565,   901,   220,   220, -1031, -1031, -1031,
   -1031, -1031,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,    50,     0,     0,     0,     0,   233,   233,     0,
       0,     0,     0,   470,     0,     0,   233,  1433,  1434,     0,
    1435,     0,     0,   512,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   217,   212,   213,
     214,   430,     0,     0,     0,     0,     0,     0,     0,  1735,
       0,   512,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,    91,    92,     0,    93,   178,    95,
     678,     0,   220,   340,     0,     0,   497,   498,     0,     0,
       0,     0,     0,   218,     0,     0,     0,   441,   442,   443,
       0,     0,   218,   160,     0,     0,   217,     0,     0,   218,
       0,     0,     0,     0,   497,   498,     0,   444,   445,   218,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,   160,     0,   217,     0,   217,
       0,     0,   160,   499,   500,     0,   470,     0,     0,     0,
       0,     0,     0,   512,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   217,   837,     0,
       0,   499,   500,     0,     0,     0,     0,   160,   645,     0,
       0,   233,   233,   837,   837,   837,   837,   837,     0,     0,
       0,     0,     0,     0,   837,     0,     0,     0,     0,   160,
       0,     0,     0,   662,   208,     0,   497,   498,   233,     0,
       0,     0,   816,     0,     0,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
     217,   786,     0,     0,     0,   218,     0,     0,     0,     0,
       0,     0,     0,     0,   233,     0,   217,   217,   160,     0,
       0,     0,     0,     0,     0,   160,     0,   219,   219,     0,
     645,   235,   212,   213,   214,     0,     0,     0,   233,   233,
       0,     0,     0,   499,   500,     0,     0,  1310,   233,     0,
       0,     0,     0,     0,   233,     0,  1704,     0,    91,    92,
    1705,    93,   178,    95,     0,     0,     0,   233,     0,     0,
     924,   925,   208,     0,     0,   837,     0,     0,   233,   933,
       0,   441,   442,   443,     0,     0,  1545,     0,     0,     0,
       0,     0,     0,     0,    50,     0,   233,     0,     0,     0,
     233,   444,   445,   877,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
     212,   213,   214,     0,     0,     0,     0,     0,     0,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,   217,     0,     0,   426,     0,    91,    92,     0,    93,
     178,    95,     0,     0,   233,     0,     0,   233,   208,   233,
       0,     0,     0,     0,     0,   218,     0,     0,     0,  1530,
       0,     0,     0,     0,   837,   837,   837,   837,     0,   233,
      50,     0,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
       0,     0,     0,     0,     0,   219,   212,   213,   214,   208,
       0,     0,     0,     0,   218,     0,   837,     0,     0,     0,
       0,     0,     0,     0,   678,   678,     0,     0,     0,     0,
     217,    50,    91,    92,     0,    93,   178,    95,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   906,     0,
     233,     0,   233,  1531,     0,   218,     0,   218,     0,     0,
     685,     0,   217,     0,     0,     0,  1532,   212,   213,   214,
    1533,     0,     0,     0,     0,     0,     0,     0,     0,   233,
       0,     0,   233,     0,     0,   218,     0,   177,     0,     0,
      89,  1534,     0,    91,    92,     0,    93,  1535,    95,     0,
       0,     0,     0,     0,     0,   233,     0,     0,     0,     0,
     217,  1129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,   217,  1139,   837,     0,
       0,     0,     0,     0,     0,     0,     0,   219,   233,     0,
    1153,     0,   233,     0,     0,   837,   219,   837,   218,     0,
       0,     0,     0,   219,     0,   441,   442,   443,     0,     0,
       0,     0,     0,   219,   218,   218,     0,     0,     0,  1173,
       0,   837,     0,     0,   219,   444,   445,     0,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,     0,   233,   233,     0,     0,   233,
       0,     0,   217,     0,   470,     0,     0,     0,     0,     0,
       0,   336,     0,   817,     0,     0,     0,   441,   442,   443,
       0,     0,     0,     0,     0,     0,     0,  1228,     0,     0,
    1231,     0,     0,     0,     0,     0,     0,   444,   445,   235,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   208,   469,     0,     0,     0,     0,     0,
       0,     0,     0,   818,     0,     0,   470,     0,     0,   219,
       0,     0,     0,     0,     0,    50,     0,     0,   218,   218,
       0,     0,   233,     0,   233,     0,     0,     0,     0,     0,
     837,     0,   837,     0,   837,     0,     0,   837,   233,     0,
       0,   837,     0,   837,     0,     0,   837,     0,     0,     0,
       0,   212,   213,   214,     0,     0,     0,   233,   233,     0,
       0,   233,   938,  1325,     0,   933,   843,     0,   233,     0,
       0,   177,     0,     0,    89,   217,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1345,     0,     0,  1348,     0,   843,     0,     0,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,     0,     0,     0,     0,   218,     0,
     233,   512,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   977,     0,   837,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   233,   233,
     218,  1393,     0,   497,   498,  1153,   233,     0,   233,     0,
     336,     0,   336,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   497,   498,     0,     0,     0,   219,
     233,     0,   233,     0,     0,     0,     0,     0,     0,   233,
       0,     0,     0,     0,     0,     0,     0,     0,   218, -1031,
   -1031, -1031, -1031, -1031,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,     0,   218,   218,     0,     0,   336,  1424,  1425,
     499,   500,     0,     0,     0,  1091,     0,     0,   837,   837,
     837,   441,   442,   443,     0,   837,     0,   233,   219,     0,
       0,   499,   500,   233,     0,   233,     0,     0,     0,     0,
       0,   444,   445,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,   219,
       0,   219,     0,     0,     0,     0,     0,     0,     0,   208,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     218,     0,     0,     0,   336,     0,     0,   336,     0,   219,
     843,    50,     0,     0,     0,  1506,     0,  1507,     0,     0,
       0,     0,     0,     0,     0,   843,   843,   843,   843,   843,
      62,    63,    64,    65,    66,   429,   843,     0,     0,     0,
       0,    72,   476,     0,     0,   233,     0,   212,   213,   214,
    1095,     0,     0,     0,     0,   275,     0,     0,     0,     0,
       0,  1551,   233,     0,     0,   845,     0,     0,     0,     0,
       0,     0,   219,    91,    92,     0,    93,   178,    95,     0,
     477,   233,   478,   277,     0,     0,  1116,   837,   219,   219,
       0,     0,     0,     0,     0,   479,   870,   480,   837,     0,
     430,   952,     0,     0,   837,   208,     0,     0,   837,     0,
       0,  1116,  1181,  1182,  1183,   208,     0,     0,   981,     0,
     219,     0,     0,     0,     0,     0,     0,    50,     0,     0,
     233,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,   218,     0,     0,   336,   843,   819,     0,
    1160,   838,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   557,   212,   213,   214,   558,     0,     0,     0,
     837,     0,   235,   212,   213,   214,     0,     0,     0,     0,
     233,     0,   838,   177,     0,     0,    89,   327,     0,    91,
      92,     0,    93,   178,    95,     0,  1044,   233,     0,    91,
      92,     0,    93,   178,    95,     0,   233,   331,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   332,     0,     0,
       0,   233,   219,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   336,   336,  1754,     0,     0,     0,
       0,     0,     0,   336,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   843,   843,   843,   843,
       0,   219,     0,     0,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,   843,   843,   843,   843,   843,   843,   843,   843,
     843,   843,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   843,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,     0,  1049,  1050,  1051,  1052,     0,     0,
       0,     0,   441,   442,   443,  1062,     0,     0,  1777,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,   445,   219,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,   470,     0,     0,   208,   838,     0,   219,     0,   275,
       0,     0,   219,     0,     0,     0,     0,    50,   336,   336,
     838,   838,   838,   838,   838,     0,    50,   219,   219,     0,
     843,   838,     0,  1838,   348,   349,     0,   277,     0,     0,
       0,     0,  1544,     0,     0,     0,     0,   843,     0,   843,
       0,     0,     0,   212,   213,   214,  1157,     0,     0,   208,
       0,     0,   212,   213,   214,     0,     0,     0,   208,     0,
       0,     0,     0,   843,     0,     0,     0,     0,     0,    91,
      92,    50,    93,   178,    95,   350,     0,     0,    91,    92,
      50,    93,   178,    95,   221,   221,     0,     0,   239,     0,
       0,     0,     0,     0,     0,   336,     0,  1545,     0,   933,
       0,  1427,     0,     0,   219,     0,   557,   212,   213,   214,
     558,   336,     0,     0,   933,     0,   212,   213,   214,  1107,
       0,     0,     0,     0,   336,     0,     0,   177,     0,     0,
      89,   327,   838,    91,    92,     0,    93,   178,    95,   350,
    1395,     0,    91,    92,     0,    93,   178,    95,     0,     0,
       0,   331,     0,   336,     0,     0,  1244,  1246,  1246,     0,
       0,   332,     0,  1257,  1260,  1261,  1262,  1264,  1265,  1266,
    1267,  1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,  1276,
    1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,
    1287,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   843,     0,   843,     0,   843,  1295,     0,   843,
     219,     0,     0,   843,     0,   843,     0,     0,   843,     0,
       0,   336,     0,     0,   336,     0,   819,     0,     0,     0,
    1527,     0,     0,  1540,     0,     0,     0,     0,     0,     0,
       0,   838,   838,   838,   838,     0,     0,   219,     0,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,     0,     0,     0,
       0,     0,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,   838,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   843,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1383,
    1604,  1605,     0,     0,     0,     0,     0,   336,     0,   336,
    1540,     0,     0,     0,     0,     0,  1398,     0,  1399,     0,
     441,   442,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   336,     0,     0,   336,
     444,   445,  1417,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   470,
     843,   843,   843,     0,     0,   838,     0,   843,     0,  1752,
       0,     0,     0,     0,   221,   336,     0,  1540,     0,   336,
       0,     0,   838,   221,   838,     0,     0,     0,     0,     0,
     221,     0,   441,   442,   443,     0,     0,     0,     0,     0,
     221,     0,     0,     0,     0,     0,     0,     0,   838,     0,
       0,   239,   444,   445,     0,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
       0,     0,   336,   336,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1509,     0,  1510,     0,  1511,     0,     0,  1512,   441,
     442,   443,  1514,     0,  1515,     0,     0,  1516,     0,     0,
       0,     0,     0,     0,     0,     0,   239,  1167,     0,   444,
     445,     0,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,     0,     0,   843,
       0,     0,     0,     0,     0,     0,   221,     0,   470,     0,
     843,     0,     0,     0,     0,     0,   843,   208,     0,   336,
     843,   336,     0,     0,     0,     0,     0,   838,     0,   838,
       0,   838,     0,     0,   838,     0,     0,     0,   838,    50,
     838,     0,     0,   838,     0,     0,     0,  1599,     0,     0,
       0,     0,     0,     0,   336,     0,     0,     0,     0,  1178,
       0,  1531,     0,   844,     0,   336,     0,     0,     0,     0,
       0,     0,     0,     0,  1532,   212,   213,   214,  1533,     0,
       0,     0,   843,     0,     0,     0,  1065,  1066,  1067,     0,
       0,     0,  1867,     0,   844,   177,     0,     0,    89,    90,
       0,    91,    92,     0,    93,  1535,    95,  1068,     0,  1527,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,     0,   838,     0,     0,  1205,     0,     0,  1745,
    1746,  1747,     0,     0,     0,  1091,  1751,     0,     0,     0,
       0,     0,     0,   336,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,   441,   442,   443,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   221,   336,     0,   336,
      50,     0,     0,     0,   444,   445,   336,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,  1237,     0,     0,     0,   212,   213,   214,     0,
       0,     0,     0,   470,     0,   838,   838,   838,   441,   442,
     443,     0,   838,     0,     0,   221,     0,     0,     0,   860,
     336,     0,    91,    92,     0,    93,   178,    95,   444,   445,
       0,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,     0,   469,   221,     0,   221,   208,
       0,     0,     0,     0,     0,     0,     0,   470,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1807,     0,
       0,    50,   275,     0,     0,     0,   221,   844,     0,  1817,
      50,     0,     0,     0,     0,  1822,     0,     0,     0,  1824,
       0,     0,   844,   844,   844,   844,   844,     0,     0,     0,
     277,     0,     0,   844,     0,     0,     0,   212,   213,   214,
       0,     0,   336,     0,     0,     0,   212,   213,   214,     0,
       0,  1564,   208,     0,     0,     0,     0,   177,     0,   336,
      89,    90,     0,    91,    92,     0,    93,   178,    95,   221,
       0,     0,    91,    92,    50,    93,   178,    95,  1798,     0,
       0,  1859,     0,     0,   838,   221,   221,     0,     0,     0,
       0,     0,     0,     0,     0,   838,     0,     0,     0,     0,
       0,   838,     0,     0,     0,   838,     0,     0,     0,   557,
     212,   213,   214,   558,     0,  1565,     0,   239,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   336,     0,     0,
     177,     0,     0,    89,   327,     0,    91,    92,     0,    93,
     178,    95,     0,     0,   844,     0,     0,     0,     0,     0,
     441,   442,   443,     0,   331,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   332,     0,     0,   838,     0,   239,
     444,   445,  1388,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
    1065,  1066,  1067,   336,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     0,     0,     0,     0,     0,   336,   221,
     221,  1068,     0,     0,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,     0,     0,     0,     0,
       0,     0,     0,   844,   844,   844,   844,     0,   239,  1091,
       0,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,   844,
     844,   844,   844,   844,   844,   844,   844,   844,   844,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   844,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   221,
       0,     0,   208,     0,     0,     0,  1404,     0,     0,     0,
       0,     0,   401,    12,    13,  1389,     0,     0,     0,     0,
       0,     0,     0,   738,    50,     0,     0,     0,     0,     0,
       0,   221,   858,   859,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
     212,   213,   214,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,   239,     0,    43,     0,     0,   221,
       0,     0,     0,   860,     0,     0,    91,    92,    50,    93,
     178,    95,     0,     0,   221,   221,    55,   844,     0,     0,
       0,     0,     0,     0,    62,    63,    64,   173,   174,   175,
       0,     0,    69,    70,   844,     0,   844,     0,     0,     0,
       0,     0,   176,    75,    76,    77,    78,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
     844,     0,     0,     0,   177,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   178,    95,     0,     0,     0,    97,
       0,     0,    98,     0,     0,     0,     0,     0,    99,     0,
       0,     0,     0,   102,   103,   104,     0,     0,   105,     0,
       0,   221,     0,   108,   109,     0,   110,   111,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,   844,
       0,   844,    43,   844,     0,     0,   844,   239,     0,     0,
     844,     0,   844,     0,    50,   844,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,   173,   174,   175,     0,     0,    69,    70,
       0,     0,     0,     0,   221,     0,     0,     0,   176,    75,
      76,    77,    78,     0,    79,    80,    81,    82,    83,     0,
       0,     0,   983,   984,     0,    85,     0,     0,     0,     0,
     177,    87,    88,    89,    90,     0,    91,    92,     0,    93,
     178,    95,   985,     0,     0,    97,     0,     0,    98,   239,
     986,   987,   988,   208,    99,     0,   441,   442,   443,   102,
     103,   104,     0,   989,   179,   844,   341,     0,     0,   108,
     109,     0,   110,   111,     0,    50,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
     990,   991,   992,   993,     0,   470,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,   994,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,    91,    92,     0,
      93,   178,    95,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   995,     0,   844,   844,   844,
       0,     0,     0,     0,   844,    14,    15,    16,     0,     0,
       0,     0,    17,  1757,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,   471,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,    82,    83,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
       0,    91,    92,     0,    93,    94,    95,    96,     0,     0,
      97,     0,     0,    98,     0,     0,     0,     0,     0,    99,
     100,     0,   101,     0,   102,   103,   104,     0,     0,   105,
       0,   106,   107,  1127,   108,   109,   844,   110,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   844,     0,     0,
       0,     0,     0,   844,     0,     0,     0,   844,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,  1840,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,   844,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,     0,    91,    92,     0,
      93,    94,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,   100,     0,   101,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1311,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,     0,    91,    92,     0,
      93,    94,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,   100,     0,   101,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,   664,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1094,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1141,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1211,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,  1213,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,  1384,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1518,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1748,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,  1794,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1829,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,  1832,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1849,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1866,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1907,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,  1914,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,    96,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,   804,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1029,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1594,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1740,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,   173,   174,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,  1066,  1067,   105,     0,   106,   107,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,  1068,     0,    10,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,     0,
     679,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1091,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,   680,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
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
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,  1067,   179,     0,     0,   799,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,  1068,     0,    10,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,     0,
    1154,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1091,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,  1155,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   401,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   441,   442,   443,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   470,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     190,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,   554,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,  1068,     0,    10,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,     0,
     225,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1091,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,   441,   442,   443,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   470,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,   556,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,   260,   442,   443,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   444,   445,     0,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,   470,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,   263,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   401,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   105,   441,   442,   443,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   470,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,   575,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,   538,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   693,   469,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   470,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,     0,
     738,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1091,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
     779,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     470,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,     0,     0,
     781,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1091,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,     0,     0,
    1122,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,     0,     0,     0,     0,
    1202,     0,     0,     0,     0,     0,     0,     0,     0,  1091,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,     0,     0,     0,
    1447,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,   441,   442,   443,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   470,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
       0,   579,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,   441,   442,   443,     0,
     108,   109,     0,   110,   111,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   444,   445,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,     0,   469,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   470,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   626,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,    62,    63,    64,   173,   174,   175,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   176,
      75,    76,    77,    78,     0,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,    85,     0,     0,     0,
       0,   177,    87,    88,    89,    90,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,    98,
     771,     0,     0,     0,     0,    99,     0,     0,     0,     0,
     102,   103,   104,     0,     0,   179,     0,     0,     0,     0,
     108,   109,     0,   110,   111,   265,   266,     0,   267,   268,
       0,     0,   269,   270,   271,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   273,
       0,   274,     0,   444,   445,     0,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   276,
     469,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   470,   278,   279,   280,   281,   282,   283,   284,
       0,     0,     0,   208,     0,   209,    40,     0,     0,     0,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,    50,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,     0,
       0,     0,   726,   320,   321,   322,     0,     0,     0,   323,
     568,   212,   213,   214,   569,     0,     0,     0,     0,     0,
     265,   266,     0,   267,   268,     0,     0,   269,   270,   271,
     272,   570,     0,     0,     0,     0,     0,    91,    92,     0,
      93,   178,    95,   328,   273,   329,   274,     0,   330,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,   727,     0,
     108,     0,     0,     0,   276,     0,     0,     0,     0,     0,
       0,  1091,     0,     0,     0,     0,     0,     0,   278,   279,
     280,   281,   282,   283,   284,     0,     0,     0,   208,     0,
     209,    40,     0,     0,     0,     0,     0,     0,     0,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
      50,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,     0,     0,     0,   319,   320,   321,
     322,     0,     0,     0,   323,   568,   212,   213,   214,   569,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     265,   266,     0,   267,   268,     0,   570,   269,   270,   271,
     272,     0,    91,    92,     0,    93,   178,    95,   328,     0,
     329,     0,     0,   330,   273,     0,   274,     0,   275,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   727,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,   276,     0,   277,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   278,   279,
     280,   281,   282,   283,   284,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
      50,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,     0,     0,     0,     0,   320,   321,
     322,     0,     0,     0,   323,   324,   212,   213,   214,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   326,     0,     0,    89,
     327,     0,    91,    92,     0,    93,   178,    95,   328,     0,
     329,     0,     0,   330,   265,   266,     0,   267,   268,     0,
     331,   269,   270,   271,   272,     0,     0,     0,     0,     0,
     332,     0,     0,     0,  1719,     0,     0,     0,   273,     0,
     274,   445,   275,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,   276,     0,
     277,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,   278,   279,   280,   281,   282,   283,   284,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,    50,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,     0,     0,
       0,     0,   320,   321,   322,     0,     0,     0,   323,   324,
     212,   213,   214,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     326,     0,     0,    89,   327,     0,    91,    92,     0,    93,
     178,    95,   328,     0,   329,     0,     0,   330,   265,   266,
       0,   267,   268,     0,   331,   269,   270,   271,   272,     0,
       0,     0,     0,     0,   332,     0,     0,     0,  1789,     0,
       0,     0,   273,     0,   274,     0,   275,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
       0,   469,   276,     0,   277,     0,     0,     0,     0,     0,
       0,     0,     0,   470,     0,     0,   278,   279,   280,   281,
     282,   283,   284,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,    50,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,     0,     0,     0,   319,   320,   321,   322,     0,
       0,     0,   323,   324,   212,   213,   214,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,     0,     0,    89,   327,     0,
      91,    92,     0,    93,   178,    95,   328,     0,   329,     0,
       0,   330,   265,   266,     0,   267,   268,     0,   331,   269,
     270,   271,   272,     0,     0,     0,     0,     0,   332,     0,
       0,     0,     0,     0,     0,     0,   273,     0,   274,     0,
     275,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,
       0,     0,     0,     0,     0,     0,   276,     0,   277,     0,
       0,     0,  1091,     0,     0,     0,     0,     0,     0,     0,
     278,   279,   280,   281,   282,   283,   284,     0,     0,     0,
     208,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,    50,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,     0,     0,     0,     0,
     320,   321,   322,     0,     0,     0,   323,   324,   212,   213,
     214,   325,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   326,     0,
       0,    89,   327,     0,    91,    92,     0,    93,   178,    95,
     328,     0,   329,     0,     0,   330,     0,   265,   266,     0,
     267,   268,   331,  1522,   269,   270,   271,   272,     0,     0,
       0,     0,   332,     0,     0,     0,     0,     0,     0,     0,
       0,   273,     0,   274,     0,   275,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,     0,     0,     0,     0,     0,
       0,   276,     0,   277,     0,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,   278,   279,   280,   281,   282,
     283,   284,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,    50,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,     0,     0,     0,     0,   320,   321,   322,     0,     0,
       0,   323,   324,   212,   213,   214,   325,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,     0,     0,    89,   327,     0,    91,
      92,     0,    93,   178,    95,   328,     0,   329,     0,     0,
     330,  1619,  1620,  1621,  1622,  1623,     0,   331,  1624,  1625,
    1626,  1627,     0,     0,     0,     0,     0,   332,     0,     0,
       0,     0,     0,     0,     0,  1628,  1629,  1630, -1031, -1031,
   -1031, -1031,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,  1631,     0,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1632,
    1633,  1634,  1635,  1636,  1637,  1638,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1639,  1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,
    1649,    50,  1650,  1651,  1652,  1653,  1654,  1655,  1656,  1657,
    1658,  1659,  1660,  1661,  1662,  1663,  1664,  1665,  1666,  1667,
    1668,  1669,  1670,  1671,  1672,  1673,  1674,  1675,  1676,  1677,
    1678,  1679,     0,     0,     0,  1680,  1681,   212,   213,   214,
       0,  1682,  1683,  1684,  1685,  1686,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1687,  1688,  1689,
       0,     0,     0,    91,    92,     0,    93,   178,    95,  1690,
       0,  1691,  1692,     0,  1693,   441,   442,   443,     0,     0,
       0,  1694,  1695,     0,  1696,     0,  1697,  1698,     0,     0,
       0,     0,     0,     0,     0,   444,   445,     0,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,     0,     0,     0,     0,     0,   265,   266,
       0,   267,   268,     0,   470,   269,   270,   271,   272,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   273,     0,   274,  1074,  1075,  1076,  1077,  1078,
    1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   276,     0,     0,  1091,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   278,   279,   280,   281,
     282,   283,   284,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,    50,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,     0,     0,     0,   319,   320,   321,   322,   796,
       0,     0,   323,   568,   212,   213,   214,   569,     0,     0,
       0,     0,     0,   265,   266,     0,   267,   268,     0,     0,
     269,   270,   271,   272,   570,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   178,    95,   328,   273,   329,   274,
       0,   330, -1031, -1031, -1031, -1031,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,     0,
       0,     0,     0,     0,     0,     0,     0,   276,     0,     0,
       0,     0,  1091,     0,     0,     0,     0,     0,     0,     0,
       0,   278,   279,   280,   281,   282,   283,   284,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,    50,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,     0,     0,     0,
    1249,   320,   321,   322,     0,     0,     0,   323,   568,   212,
     213,   214,   569,     0,     0,     0,     0,     0,   265,   266,
       0,   267,   268,     0,     0,   269,   270,   271,   272,   570,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   178,
      95,   328,   273,   329,   274,     0,   330,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   276,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   278,   279,   280,   281,
     282,   283,   284,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,    50,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,     0,     0,     0,  1255,   320,   321,   322,     0,
       0,     0,   323,   568,   212,   213,   214,   569,     0,     0,
       0,     0,     0,   265,   266,     0,   267,   268,     0,     0,
     269,   270,   271,   272,   570,     0,     0,     0,     0,     0,
      91,    92,     0,    93,   178,    95,   328,   273,   329,   274,
       0,   330,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   276,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   278,   279,   280,   281,   282,   283,   284,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,    50,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   275,     0,     0,
       0,   320,   321,   322,     0,     0,     0,   323,   568,   212,
     213,   214,   569,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   277,     0,     0,     0,   570,
       0,     0,     0,     0,     0,    91,    92,     0,    93,   178,
      95,   328,     0,   329,     0,     0,   330,   208,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,  1263,     0,     0,  -392,     0,     0,
       0,     0,     0,     0,     0,    62,    63,    64,   173,   174,
     429,     0,   825,   826,     0,     0,     0,     0,   827,     0,
     828,     0,     0,     0,   557,   212,   213,   214,   558,     0,
       0,     0,   829,     0,     0,     0,     0,     0,     0,     0,
      34,    35,    36,   208,     0,   177,     0,     0,    89,   327,
       0,    91,    92,   210,    93,   178,    95,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,   331,
       0,     0,     0,     0,     0,   430,     0,     0,     0,   332,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     830,   831,   832,   833,     0,    79,    80,    81,    82,    83,
       0,  1023,     0,     0,     0,     0,   215,     0,     0,     0,
       0,   177,    87,    88,    89,   834,     0,    91,    92,     0,
      93,   178,    95,     0,     0,     0,    97,     0,     0,     0,
       0,     0,     0,    29,     0,   835,     0,     0,     0,     0,
     102,    34,    35,    36,   208,   836,   209,    40,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1024,    75,   212,   213,   214,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   177,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   178,    95,   825,   826,     0,    97,     0,     0,
     827,     0,   828,     0,     0,     0,     0,     0,     0,     0,
       0,   102,     0,     0,   829,     0,   216,     0,     0,     0,
       0,   108,    34,    35,    36,   208,     0,  1065,  1066,  1067,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,  1068,     0,
       0,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,  1089,  1090,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   830,   831,   832,   833,  1091,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   177,    87,    88,    89,   834,     0,    91,
      92,     0,    93,   178,    95,    29,     0,     0,    97,     0,
       0,     0,     0,    34,    35,    36,   208,   835,   209,    40,
       0,     0,   102,     0,     0,     0,   210,   836,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,  1413,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   212,   213,   214,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   177,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   178,    95,    29,     0,     0,    97,
       0,     0,     0,     0,    34,    35,    36,   208,     0,   209,
      40,     0,     0,   102,     0,     0,     0,   210,   216,     0,
       0,   591,     0,   108,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   611,    75,   212,   213,   214,     0,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
     215,     0,     0,     0,     0,   177,    87,    88,    89,    90,
       0,    91,    92,     0,    93,   178,    95,    29,     0,   972,
      97,     0,     0,     0,     0,    34,    35,    36,   208,     0,
     209,    40,     0,     0,   102,     0,     0,     0,   210,   216,
       0,     0,     0,     0,   108,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,   212,   213,   214,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   177,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   178,    95,    29,     0,
       0,    97,     0,     0,     0,     0,    34,    35,    36,   208,
       0,   209,    40,     0,     0,   102,     0,     0,     0,   210,
     216,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1119,    75,   212,   213,   214,
       0,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,   177,    87,    88,
      89,    90,     0,    91,    92,     0,    93,   178,    95,    29,
       0,     0,    97,     0,     0,     0,     0,    34,    35,    36,
     208,     0,   209,    40,     0,     0,   102,     0,     0,     0,
     210,   216,     0,     0,     0,     0,   108,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   212,   213,
     214,     0,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,   177,    87,
      88,    89,    90,     0,    91,    92,     0,    93,   178,    95,
       0,     0,     0,    97,     0,   441,   442,   443,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   102,     0,     0,
       0,     0,   216,     0,     0,   444,   445,   108,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,     0,   469,   441,   442,   443,     0,     0,     0,     0,
       0,     0,     0,     0,   470,     0,     0,     0,     0,     0,
       0,     0,     0,   444,   445,     0,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,     0,
     469,     0,     0,     0,     0,     0,     0,     0,     0,   441,
     442,   443,   470,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   444,
     445,   515,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,   441,   442,   443,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,   444,   445,   524,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,     0,     0,     0,     0,     0,
       0,     0,     0,   441,   442,   443,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   444,   445,   892,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,     0,
     469,   441,   442,   443,     0,     0,     0,     0,     0,     0,
       0,     0,   470,     0,     0,     0,     0,     0,     0,     0,
       0,   444,   445,   958,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,   441,   442,   443,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   444,   445,  1008,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,     0,   469,  1065,  1066,  1067,     0,     0,
       0,     0,     0,     0,     0,     0,   470,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1068,  1309,     0,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1065,  1066,  1067,     0,  1091,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1068,     0,  1340,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,     0,     0,  1065,  1066,
    1067,     0,     0,     0,     0,     0,     0,     0,     0,  1091,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1068,
       0,  1508,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,     0,    34,    35,    36,   208,     0,
     209,    40,     0,     0,     0,     0,     0,  1091,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,  1600,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   230,     0,     0,     0,     0,     0,   231,     0,     0,
       0,     0,     0,     0,     0,     0,   212,   213,   214,     0,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,   215,     0,     0,  1602,     0,   177,    87,    88,    89,
      90,     0,    91,    92,     0,    93,   178,    95,     0,     0,
       0,    97,     0,    34,    35,    36,   208,     0,   209,    40,
       0,     0,     0,     0,     0,   102,   640,     0,     0,     0,
     232,     0,     0,     0,     0,   108,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,   213,   214,     0,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   177,    87,    88,    89,    90,     0,
      91,    92,     0,    93,   178,    95,     0,     0,     0,    97,
       0,    34,    35,    36,   208,     0,   209,    40,     0,     0,
       0,     0,     0,   102,   210,     0,     0,     0,   641,     0,
       0,     0,     0,   108,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   230,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   212,   213,   214,     0,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   177,    87,    88,    89,    90,     0,    91,    92,
       0,    93,   178,    95,     0,     0,     0,    97,     0,   441,
     442,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   102,     0,     0,     0,     0,   232,   808,     0,   444,
     445,   108,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   441,   442,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   809,   444,   445,   955,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,     0,   469,
     441,   442,   443,     0,     0,     0,     0,     0,     0,     0,
       0,   470,     0,     0,     0,     0,     0,     0,     0,     0,
     444,   445,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,     0,   469,  1065,  1066,
    1067,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1068,
    1418,     0,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1065,  1066,  1067,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1091,     0,     0,
       0,     0,     0,     0,     0,  1068,     0,     0,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
     443,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1091,     0,     0,     0,     0,   444,   445,
       0,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,     0,   469,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   470
};

static const yytype_int16 yycheck[] =
{
       5,     6,    56,     8,     9,    10,    11,    12,    13,   126,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   181,     4,    29,    30,    96,   656,   931,   394,
     100,   101,   156,     4,     4,   735,   105,   394,    33,    44,
     394,   653,     4,  1144,   509,    57,   233,    52,   161,    54,
     528,    46,    57,   682,    59,   125,    51,   156,    31,   501,
     105,   920,  1131,   501,   469,   182,   505,   506,    31,    44,
      31,   585,   586,    31,   652,   633,  1300,    57,  1022,    84,
     951,   814,   783,   105,   105,   348,   349,   807,     9,    32,
       9,    32,     9,    49,   536,   534,   967,     9,   536,     9,
     105,  1140,    14,    49,    56,     9,  1429,    49,     9,    84,
      14,     9,    49,    14,     9,    36,     9,    49,    83,     9,
       9,     9,     4,     9,     9,     9,     9,   115,     9,    83,
       9,     9,   245,     9,   179,     9,     9,     4,  1009,    70,
       4,     9,  1733,     9,     9,    83,    38,    70,    83,    84,
      90,    50,    51,    83,   164,   158,    70,   179,   179,   108,
      57,    70,   134,   135,    70,     4,   102,    38,   533,   122,
       0,   216,    69,   193,   179,    83,    38,   130,   123,   473,
     102,   186,   179,    38,     8,   130,   196,   232,   176,    53,
     193,    83,    56,   158,   216,   216,   193,   193,    38,  1790,
     193,    38,  1061,   134,   135,  1528,   102,    70,   502,    73,
     232,   216,    83,   507,   244,   155,    70,    70,   172,   228,
      70,    83,    70,    70,    70,   197,   162,   232,    83,  1552,
      94,  1554,    96,   862,    70,   191,   100,   101,   194,    70,
     162,   246,   172,    83,   249,    54,    83,    70,   194,   191,
      70,   256,   257,   196,   195,    70,    70,   194,    70,    70,
     198,   125,   194,   198,   172,   196,   162,   179,  1337,   970,
     193,    70,   431,   194,  1218,  1344,   195,  1346,   195,   196,
     172,   180,   196,   195,   193,   195,   340,   126,   250,   156,
     196,   195,   254,    60,   195,  1519,  1335,   195,   540,  1032,
     195,  1034,   195,  1372,  1175,   195,   195,   195,    70,   195,
     195,   195,   195,   773,   195,   194,   194,   172,   194,    86,
     194,   194,    89,   368,    27,    28,   194,   158,   194,   194,
     193,   195,   172,   196,   576,   172,   948,   193,  1197,   193,
     193,   194,   196,   182,   858,   859,   368,   368,   196,   379,
     196,   909,   511,    70,   163,   425,    90,   193,    83,   158,
     196,    14,   367,   368,   102,   196,   483,   193,   422,   374,
     375,   376,   377,   193,   164,    83,   196,   382,   158,    32,
     162,   196,   196,   193,   196,   196,   250,   177,   340,    70,
     254,   179,   367,   196,   258,   193,   401,   196,    51,   179,
     193,   376,   377,    31,   409,   193,   196,  1476,   478,   479,
     480,   481,   654,   195,   196,   484,   421,   134,   135,    54,
     193,   155,    50,   888,   162,    53,  1465,   193,  1467,  1129,
     155,   156,   157,   193,   196,   193,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   172,   470,    70,   472,   473,   474,
     422,   163,   484,   106,   107,    70,   340,   158,   193,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,    83,     4,  1214,   937,   469,   502,   503,   937,
     505,   506,   507,   508,   484,    31,   469,   512,   469,   518,
     515,   469,   193,    83,    84,   196,    27,    28,   400,   524,
      32,   526,   225,   475,    50,   964,   475,    53,   163,   534,
     542,   179,   162,    70,  1573,   134,   135,   542,  1577,   544,
      38,   408,   729,   178,   408,   193,   670,  1159,   193,    83,
    1162,  1410,   501,   195,   418,   920,    90,   799,   422,   195,
     533,   425,   804,   920,   197,  1143,   920,   972,    83,   408,
     522,   670,   195,   522,   761,    90,   134,   135,   737,   179,
     195,   348,   349,   350,   533,   547,   591,   536,    83,   852,
    1104,   854,   195,   193,   105,    90,   641,   134,   135,   190,
     106,   107,   195,    83,    70,   196,   158,   195,   617,   618,
      90,   475,   476,   477,   478,   479,   480,   481,   688,   386,
     195,   158,   156,   157,   179,    70,   196,   179,   198,   659,
      70,   661,    70,  1492,   643,   193,   641,   501,   193,    75,
      76,   156,   157,   482,   196,  1345,    83,    50,    51,     4,
     837,    75,    76,    90,   357,    70,   193,    83,   522,   158,
     847,   156,   157,   366,    90,   368,   196,   201,   179,   193,
     373,   784,   536,   102,   103,   680,   156,   157,   132,   133,
     383,   195,   196,   547,   105,   106,   107,   202,   693,   348,
     349,   197,   193,  1732,    49,    70,  1061,  1736,   158,  1177,
     111,    83,   193,   567,  1061,   216,   162,  1061,   119,   120,
     121,   122,   123,   124,   225,   158,   725,    83,   155,   156,
     157,   232,   727,   195,    90,   589,   590,  1339,    48,  1168,
     156,   157,  1192,  1193,  1194,    69,   179,   179,  1198,   250,
    1179,  1764,  1765,   254,  1444,   158,  1324,   777,   778,   193,
     193,   756,  1217,   196,   784,   785,     9,   112,   622,   623,
     195,   196,   117,   193,   119,   120,   121,   122,   123,   124,
     125,   653,  1760,  1761,   156,   157,   193,  1878,   189,   105,
     106,   107,   119,   120,   121,   158,   791,  1029,   200,   155,
     156,   157,  1893,    81,   119,   120,   121,   122,   123,   124,
     823,   824,   807,   670,     4,   158,   509,   816,   803,     8,
     165,   166,   821,   168,   195,   103,   193,   158,   585,   586,
      14,    53,    54,    55,   688,    57,   109,   110,   111,  1529,
     158,   195,  1197,  1872,   189,   195,     9,    69,   196,  1304,
    1197,   195,   197,  1197,   130,   130,   357,    14,  1887,    49,
     812,   139,   140,   141,   194,   366,   179,   368,    14,   102,
     199,   194,   373,   194,   189,  1477,   193,    53,    54,    55,
     195,   111,   383,   194,   162,   884,   194,   165,   166,   193,
     168,   169,   170,    69,   193,     9,   155,   892,   194,   894,
     194,   896,   194,  1353,   194,  1355,    94,   408,     9,   195,
     905,   971,  1341,   179,    14,     9,   193,   196,   196,   773,
     194,   775,   112,   193,   919,   196,   195,   117,   195,   119,
     120,   121,   122,   123,   124,   125,    83,   908,   194,   194,
     797,   195,   193,   797,   194,    27,    28,   908,   908,   132,
     945,  1844,   200,     9,     9,   200,   908,   811,   812,   200,
     955,    70,    32,   958,   133,   960,   178,   158,   797,   964,
     136,  1864,     9,  1150,   194,   165,   166,   158,   168,    14,
    1873,   923,   191,     9,   923,     9,   679,   180,   194,     9,
     132,    14,   849,   200,     9,   849,   935,   200,   937,   189,
     197,    14,   856,   857,  1003,   200,   200,   197,   509,   972,
     158,   102,   194,  1008,   194,   193,   195,   194,     9,   972,
     849,   972,   195,    91,   972,   136,   158,     9,    70,   194,
    1015,   885,   193,  1483,  1463,  1485,   908,  1487,    70,    70,
    1490,   158,   193,   158,   196,   738,   547,   196,     9,  1048,
      14,   908,   197,   180,   908,  1410,  1055,   195,  1290,   931,
       9,   196,  1121,  1410,  1016,    81,  1410,    83,    84,   923,
      14,  1010,   929,   380,   200,   929,   948,   384,   196,   908,
      14,   935,   194,   937,   191,   195,   779,   103,   781,   119,
     120,   121,   122,   123,   124,   852,    32,   854,   193,    32,
     929,   858,   859,   860,   411,   193,   413,   414,   415,   416,
      14,    14,   193,   193,    52,   193,   809,   971,  1060,  1121,
      70,  1063,    70,   139,   140,   141,  1121,    70,   193,   983,
     984,   985,     9,   194,  1584,   195,   195,  1492,   136,   193,
      14,   180,  1374,   225,   136,  1492,     9,   163,  1492,   165,
     166,   194,   168,   169,   170,    69,  1010,  1128,  1390,   189,
    1155,  1018,  1016,  1020,  1018,  1784,  1020,  1128,  1128,     9,
      83,   200,   197,  1168,   195,   197,  1128,     9,   679,   872,
     196,    81,   198,   193,  1179,  1180,  1040,   193,   195,  1018,
    1189,  1020,    14,    83,   136,   888,   889,   194,   193,   193,
     196,   194,     9,   103,   136,   195,  1060,   196,   196,  1063,
      91,   196,   119,   120,   121,   122,   123,   124,   155,  1214,
     200,  1206,    32,   130,   131,    77,   195,   194,   180,  1224,
      50,    51,    52,    53,    54,    55,   195,   738,  1092,   139,
     140,   141,   136,  1862,  1243,    32,   200,   194,  1247,    69,
       9,     9,   136,  1252,   194,   200,  1128,  1209,     9,   159,
    1259,     4,   162,  1495,   171,   165,   166,   197,   168,   169,
     170,  1128,  1504,   200,  1128,   357,   194,     9,   779,   194,
     781,   197,   189,   195,   366,  1517,   195,  1159,  1738,  1739,
    1162,   373,    14,    83,   196,   193,   797,   197,   194,  1128,
     195,   383,   194,   194,     9,   193,    49,   196,   809,   194,
     136,   812,   394,   200,  1309,   194,     9,   200,     4,   136,
     200,  1316,   194,    32,     9,  1320,   195,  1322,   194,  1022,
    1023,  1445,   194,  1440,   195,  1330,   195,  1191,  1192,  1193,
    1194,   196,   112,   167,  1198,  1340,  1341,  1104,   849,    50,
      51,    52,    53,    54,    55,  1209,    57,   195,   163,    14,
      83,  1360,  1594,    49,  1221,  1364,   194,  1221,    69,   112,
    1369,   872,   117,   194,   117,   136,   119,   120,   121,   122,
     123,   124,   125,   196,   136,   194,    14,   888,   889,   195,
     179,    83,  1221,   196,    14,    14,    83,   194,   193,   136,
     194,  1550,   136,   195,   195,    14,    14,   908,    78,    79,
      80,   195,    14,   196,     9,     9,   197,    59,    83,   179,
      31,    91,   165,   166,   193,   168,   112,   509,   929,  1122,
    1302,   117,    83,   119,   120,   121,   122,   123,   124,   125,
       9,  1313,  1296,   196,   115,  1302,   189,   195,    59,   102,
     158,   180,   102,   170,   197,    36,  1313,    14,   193,   195,
     193,  1154,  1522,   194,   176,  1460,   180,  1339,  1463,   180,
      81,    83,     9,   143,   144,   145,   146,   147,   173,   165,
     166,   194,   168,    83,   154,   195,    14,   194,   194,    14,
     160,   161,   103,   196,    83,    83,    14,    83,  1730,  1353,
     111,  1355,    14,   189,   174,    83,  1104,  1853,  1740,  1202,
     481,   197,   478,   476,   965,  1016,   911,  1018,   188,  1020,
    1869,  1022,  1023,  1215,  1217,  1218,  1593,   138,   139,   140,
     141,   142,  1864,  1580,  1617,    27,    28,  1387,  1530,    31,
     593,  1702,  1435,  1897,  1885,  1714,  1576,  1431,   159,  1501,
     377,   162,   163,  1785,   165,   166,  1059,   168,   169,   170,
    1194,  1132,  1056,  1420,    56,  1005,  1420,   984,  1190,  1718,
    1191,   935,   183,  1568,   374,  1429,   422,   823,  1819,  1112,
    1041,  1435,   193,  1525,  1423,  1092,    -1,    -1,  1445,    -1,
      -1,  1420,    -1,    -1,  1466,    -1,    -1,   679,  1830,    -1,
    1472,    -1,  1474,    -1,    -1,  1477,    -1,    -1,    -1,  1466,
    1439,  1304,    -1,    -1,    -1,  1472,    -1,  1474,    -1,    -1,
      -1,  1122,  1494,    -1,    -1,    -1,    -1,  1128,    -1,  1483,
      -1,  1485,    -1,  1487,    -1,    -1,  1490,  1494,    -1,  1496,
    1592,  1593,  1496,    -1,    -1,    -1,    -1,  1501,  1505,    -1,
      -1,  1505,    -1,  1154,    -1,    -1,   738,    -1,    -1,  1891,
      -1,    -1,    -1,    -1,    -1,    -1,  1898,  1496,  1522,  1713,
      -1,  1525,    -1,    -1,  1528,    -1,  1505,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1538,    -1,    -1,    -1,    -1,    -1,
      -1,  1545,    -1,    81,    -1,    83,    84,   779,  1552,   781,
    1554,  1202,    -1,    -1,    -1,    -1,    -1,  1561,  1209,    -1,
    1582,    -1,    -1,    -1,    -1,   103,  1217,  1218,  1778,    -1,
    1221,    -1,    -1,  1837,    -1,  1582,    -1,   809,  1723,  1558,
    1584,    -1,    -1,   225,  1591,    -1,    -1,  1591,  1592,  1593,
    1597,    -1,    -1,  1597,    -1,    -1,  1603,    -1,    -1,  1603,
    1857,   139,   140,   141,  1447,  1754,  1755,    -1,    -1,    -1,
      -1,    -1,  1591,    -1,    -1,    -1,    -1,    -1,  1597,    -1,
      -1,  1713,    -1,    -1,  1603,   163,    -1,   165,   166,    -1,
     168,   169,   170,   275,    -1,   277,    -1,    -1,    -1,    -1,
     872,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1304,    -1,    -1,   888,   889,   196,    -1,
     198,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,   920,    -1,
     332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,  1713,
      -1,    -1,    -1,    -1,    -1,   357,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,   366,    -1,    -1,    -1,    -1,    -1,
    1734,   373,    -1,    -1,  1738,  1739,    -1,  1744,    -1,    -1,
    1744,   383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1753,
      -1,    -1,   394,    27,    28,    -1,  1760,  1761,  1903,    -1,
    1764,  1765,    -1,    49,    -1,  1744,  1911,    -1,     4,  1420,
      -1,    -1,  1917,    -1,  1778,  1920,    -1,   419,    -1,  1786,
     422,    -1,  1786,    -1,    -1,    -1,  1793,    -1,    -1,  1793,
    1022,  1023,    -1,    -1,    -1,    -1,  1447,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1786,    -1,    -1,
      -1,    -1,    -1,    49,  1793,    -1,    -1,    -1,    -1,    -1,
      -1,  1828,  1844,    -1,  1828,   194,   112,   469,    -1,  1061,
    1837,   117,  1836,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,  1864,  1850,    -1,  1496,  1850,    -1,    -1,  1828,
    1501,  1873,  1856,    -1,  1505,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   509,    -1,    -1,
      -1,  1850,    -1,    -1,    -1,    -1,   112,    -1,    -1,   165,
     166,   117,   168,   119,   120,   121,   122,   123,   124,   125,
    1122,    -1,  1899,    -1,    -1,  1899,    -1,    -1,    -1,  1906,
      -1,    49,  1906,   189,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   561,
    1899,   563,  1154,    -1,   566,    -1,   275,  1906,   277,   165,
     166,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1591,  1592,  1593,    -1,    -1,    -1,  1597,    -1,    -1,    -1,
      -1,   225,  1603,   189,    -1,   597,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,   112,  1197,    -1,    -1,    -1,   117,
    1202,   119,   120,   121,   122,   123,   124,   125,    -1,    -1,
      81,    -1,    83,   332,    85,  1217,  1218,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   103,    -1,    -1,    -1,    -1,   649,   650,    -1,
      -1,    -1,    -1,    69,    -1,    -1,   658,   165,   166,    -1,
     168,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   679,   139,   140,
     141,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   165,   166,    -1,   168,   169,   170,
     419,    -1,  1304,   422,    -1,    -1,    67,    68,    -1,    -1,
      -1,    -1,    -1,   357,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,   366,  1744,    -1,    -1,   738,    -1,    -1,   373,
      -1,    -1,    -1,    -1,    67,    68,    -1,    30,    31,   383,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,  1786,    -1,   779,    -1,   781,
      -1,    -1,  1793,   134,   135,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   809,   810,    -1,
      -1,   134,   135,    -1,    -1,    -1,    -1,  1828,  1410,    -1,
      -1,   823,   824,   825,   826,   827,   828,   829,    -1,    -1,
      -1,    -1,    -1,    -1,   836,    -1,    -1,    -1,    -1,  1850,
      -1,    -1,    -1,   194,    81,    -1,    67,    68,   850,    -1,
      -1,    -1,   561,    -1,    -1,  1447,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
     872,   194,    -1,    -1,    -1,   509,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   886,    -1,   888,   889,  1899,    -1,
      -1,    -1,    -1,    -1,    -1,  1906,    -1,    27,    28,    -1,
    1492,    31,   139,   140,   141,    -1,    -1,    -1,   910,   911,
      -1,    -1,    -1,   134,   135,    -1,    -1,   200,   920,    -1,
      -1,    -1,    -1,    -1,   926,    -1,   163,    -1,   165,   166,
     167,   168,   169,   170,    -1,    -1,    -1,   939,    -1,    -1,
     649,   650,    81,    -1,    -1,   947,    -1,    -1,   950,   658,
      -1,    10,    11,    12,    -1,    -1,   193,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,   968,    -1,    -1,    -1,
     972,    30,    31,   194,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1022,  1023,    -1,    -1,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,  1036,    -1,    -1,  1039,    81,  1041,
      -1,    -1,    -1,    -1,    -1,   679,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,  1056,  1057,  1058,  1059,    -1,  1061,
     103,    -1,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,
      -1,    -1,    -1,    -1,    -1,   225,   139,   140,   141,    81,
      -1,    -1,    -1,    -1,   738,    -1,  1108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   823,   824,    -1,    -1,    -1,    -1,
    1122,   103,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,
    1142,    -1,  1144,   125,    -1,   779,    -1,   781,    -1,    -1,
     193,    -1,  1154,    -1,    -1,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1171,
      -1,    -1,  1174,    -1,    -1,   809,    -1,   159,    -1,    -1,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    -1,
      -1,    -1,    -1,    -1,    -1,  1197,    -1,    -1,    -1,    -1,
    1202,   910,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1217,  1218,   926,  1220,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   357,  1230,    -1,
     939,    -1,  1234,    -1,    -1,  1237,   366,  1239,   872,    -1,
      -1,    -1,    -1,   373,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   383,   888,   889,    -1,    -1,    -1,   968,
      -1,  1263,    -1,    -1,   394,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,  1297,  1298,    -1,    -1,  1301,
      -1,    -1,  1304,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    31,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1036,    -1,    -1,
    1039,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   469,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    81,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    69,    -1,    -1,   509,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,  1022,  1023,
      -1,    -1,  1394,    -1,  1396,    -1,    -1,    -1,    -1,    -1,
    1402,    -1,  1404,    -1,  1406,    -1,    -1,  1409,  1410,    -1,
      -1,  1413,    -1,  1415,    -1,    -1,  1418,    -1,    -1,    -1,
      -1,   139,   140,   141,    -1,    -1,    -1,  1429,  1430,    -1,
      -1,  1433,   197,  1142,    -1,  1144,   566,    -1,  1440,    -1,
      -1,   159,    -1,    -1,   162,  1447,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1171,    -1,    -1,  1174,    -1,   597,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1122,    -1,
    1492,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   197,    -1,  1508,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1520,  1521,
    1154,  1230,    -1,    67,    68,  1234,  1528,    -1,  1530,    -1,
     275,    -1,   277,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,   679,
    1552,    -1,  1554,    -1,    -1,    -1,    -1,    -1,    -1,  1561,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1202,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,  1217,  1218,    -1,    -1,   332,  1297,  1298,
     134,   135,    -1,    -1,    -1,    69,    -1,    -1,  1600,  1601,
    1602,    10,    11,    12,    -1,  1607,    -1,  1609,   738,    -1,
      -1,   134,   135,  1615,    -1,  1617,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,   779,
      -1,   781,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1304,    -1,    -1,    -1,   419,    -1,    -1,   422,    -1,   809,
     810,   103,    -1,    -1,    -1,  1394,    -1,  1396,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   825,   826,   827,   828,   829,
     119,   120,   121,   122,   123,   124,   836,    -1,    -1,    -1,
      -1,   130,   131,    -1,    -1,  1717,    -1,   139,   140,   141,
     850,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,  1440,  1734,    -1,    -1,   566,    -1,    -1,    -1,    -1,
      -1,    -1,   872,   165,   166,    -1,   168,   169,   170,    -1,
     169,  1753,   171,    59,    -1,    -1,   886,  1759,   888,   889,
      -1,    -1,    -1,    -1,    -1,   184,   597,   186,  1770,    -1,
     189,   193,    -1,    -1,  1776,    81,    -1,    -1,  1780,    -1,
      -1,   911,    78,    79,    80,    81,    -1,    -1,   197,    -1,
     920,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
    1802,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,  1447,    -1,    -1,   561,   947,   563,    -1,
     950,   566,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
    1842,    -1,   972,   139,   140,   141,    -1,    -1,    -1,    -1,
    1852,    -1,   597,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    -1,   172,  1869,    -1,   165,
     166,    -1,   168,   169,   170,    -1,  1878,   183,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,  1893,  1022,  1023,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   649,   650,  1615,    -1,    -1,    -1,
      -1,    -1,    -1,   658,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1056,  1057,  1058,  1059,
      -1,  1061,    -1,    -1,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,
    1090,  1091,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1108,   810,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1122,    -1,   825,   826,   827,   828,    -1,    -1,
      -1,    -1,    10,    11,    12,   836,    -1,    -1,  1717,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,  1154,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    81,   810,    -1,  1197,    -1,    31,
      -1,    -1,  1202,    -1,    -1,    -1,    -1,   103,   823,   824,
     825,   826,   827,   828,   829,    -1,   103,  1217,  1218,    -1,
    1220,   836,    -1,  1802,   111,   112,    -1,    59,    -1,    -1,
      -1,    -1,   128,    -1,    -1,    -1,    -1,  1237,    -1,  1239,
      -1,    -1,    -1,   139,   140,   141,   947,    -1,    -1,    81,
      -1,    -1,   139,   140,   141,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,  1263,    -1,    -1,    -1,    -1,    -1,   165,
     166,   103,   168,   169,   170,   162,    -1,    -1,   165,   166,
     103,   168,   169,   170,    27,    28,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,   910,    -1,   193,    -1,  1878,
      -1,  1301,    -1,    -1,  1304,    -1,   138,   139,   140,   141,
     142,   926,    -1,    -1,  1893,    -1,   139,   140,   141,   197,
      -1,    -1,    -1,    -1,   939,    -1,    -1,   159,    -1,    -1,
     162,   163,   947,   165,   166,    -1,   168,   169,   170,   162,
     172,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   183,    -1,   968,    -1,    -1,  1057,  1058,  1059,    -1,
      -1,   193,    -1,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,
    1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,
    1091,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1402,    -1,  1404,    -1,  1406,  1108,    -1,  1409,
    1410,    -1,    -1,  1413,    -1,  1415,    -1,    -1,  1418,    -1,
      -1,  1036,    -1,    -1,  1039,    -1,  1041,    -1,    -1,    -1,
    1430,    -1,    -1,  1433,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1056,  1057,  1058,  1059,    -1,    -1,  1447,    -1,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1087,  1088,  1089,  1090,  1091,    -1,    -1,    -1,
      -1,    -1,   225,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1492,  1108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1508,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,
    1520,  1521,    -1,    -1,    -1,    -1,    -1,  1142,    -1,  1144,
    1530,    -1,    -1,    -1,    -1,    -1,  1237,    -1,  1239,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1171,    -1,    -1,  1174,
      30,    31,  1263,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
    1600,  1601,  1602,    -1,    -1,  1220,    -1,  1607,    -1,  1609,
      -1,    -1,    -1,    -1,   357,  1230,    -1,  1617,    -1,  1234,
      -1,    -1,  1237,   366,  1239,    -1,    -1,    -1,    -1,    -1,
     373,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
     383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1263,    -1,
      -1,   394,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,  1297,  1298,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1402,    -1,  1404,    -1,  1406,    -1,    -1,  1409,    10,
      11,    12,  1413,    -1,  1415,    -1,    -1,  1418,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   469,   197,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,  1759,
      -1,    -1,    -1,    -1,    -1,    -1,   509,    -1,    69,    -1,
    1770,    -1,    -1,    -1,    -1,    -1,  1776,    81,    -1,  1394,
    1780,  1396,    -1,    -1,    -1,    -1,    -1,  1402,    -1,  1404,
      -1,  1406,    -1,    -1,  1409,    -1,    -1,    -1,  1413,   103,
    1415,    -1,    -1,  1418,    -1,    -1,    -1,  1508,    -1,    -1,
      -1,    -1,    -1,    -1,  1429,    -1,    -1,    -1,    -1,   197,
      -1,   125,    -1,   566,    -1,  1440,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,  1842,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,  1852,    -1,   597,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    31,    -1,  1869,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,  1508,    -1,    -1,   197,    -1,    -1,  1600,
    1601,  1602,    -1,    -1,    -1,    69,  1607,    -1,    -1,    -1,
      -1,    -1,    -1,  1528,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   679,  1552,    -1,  1554,
     103,    -1,    -1,    -1,    30,    31,  1561,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   136,    -1,    -1,    -1,   139,   140,   141,    -1,
      -1,    -1,    -1,    69,    -1,  1600,  1601,  1602,    10,    11,
      12,    -1,  1607,    -1,    -1,   738,    -1,    -1,    -1,   162,
    1615,    -1,   165,   166,    -1,   168,   169,   170,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,   779,    -1,   781,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1759,    -1,
      -1,   103,    31,    -1,    -1,    -1,   809,   810,    -1,  1770,
     103,    -1,    -1,    -1,    -1,  1776,    -1,    -1,    -1,  1780,
      -1,    -1,   825,   826,   827,   828,   829,    -1,    -1,    -1,
      59,    -1,    -1,   836,    -1,    -1,    -1,   139,   140,   141,
      -1,    -1,  1717,    -1,    -1,    -1,   139,   140,   141,    -1,
      -1,   197,    81,    -1,    -1,    -1,    -1,   159,    -1,  1734,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,   872,
      -1,    -1,   165,   166,   103,   168,   169,   170,  1753,    -1,
      -1,  1842,    -1,    -1,  1759,   888,   889,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1770,    -1,    -1,    -1,    -1,
      -1,  1776,    -1,    -1,    -1,  1780,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,   197,    -1,   920,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1802,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    -1,    -1,   947,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,   183,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,  1842,    -1,   972,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      10,    11,    12,  1878,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1893,  1022,
    1023,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1056,  1057,  1058,  1059,    -1,  1061,    69,
      -1,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,
    1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1108,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,  1122,
      -1,    -1,    81,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,   195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,   103,    -1,    -1,    -1,    -1,    -1,
      -1,  1154,   111,   112,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
     139,   140,   141,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,  1197,    -1,    91,    -1,    -1,  1202,
      -1,    -1,    -1,   162,    -1,    -1,   165,   166,   103,   168,
     169,   170,    -1,    -1,  1217,  1218,   111,  1220,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,  1237,    -1,  1239,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
    1263,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,   188,   189,   190,    -1,    -1,   193,    -1,
      -1,  1304,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,  1402,
      -1,  1404,    91,  1406,    -1,    -1,  1409,  1410,    -1,    -1,
    1413,    -1,  1415,    -1,   103,  1418,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,  1447,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    50,    51,    -1,   154,    -1,    -1,    -1,    -1,
     159,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,    70,    -1,    -1,   174,    -1,    -1,   177,  1492,
      78,    79,    80,    81,   183,    -1,    10,    11,    12,   188,
     189,   190,    -1,    91,   193,  1508,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,   103,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,    -1,    69,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,   154,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,    -1,  1600,  1601,  1602,
      -1,    -1,    -1,    -1,  1607,    49,    50,    51,    -1,    -1,
      -1,    -1,    56,  1616,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   195,   126,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,   171,    -1,    -1,
     174,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
     184,    -1,   186,    -1,   188,   189,   190,    -1,    -1,   193,
      -1,   195,   196,   197,   198,   199,  1759,   201,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1770,    -1,    -1,
      -1,    -1,    -1,  1776,    -1,    -1,    -1,  1780,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,  1804,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,  1842,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,   184,    -1,   186,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,   184,    -1,   186,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    97,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,   197,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,   171,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    11,    12,   193,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,   172,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    12,   193,    -1,    -1,   196,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,   172,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    10,    11,    12,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     108,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,   195,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    10,    11,    12,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,   195,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,    11,    12,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,   195,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    10,    11,    12,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,   195,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,   194,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    10,    11,    12,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
      -1,   195,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    10,    11,    12,    -1,
     198,   199,    -1,   201,   202,     3,     4,     5,     6,     7,
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
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,   177,
     194,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     188,   189,   190,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,   201,   202,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   159,    -1,    -1,    -1,    -1,    -1,   165,   166,    -1,
     168,   169,   170,   171,    27,   173,    29,    -1,   176,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   196,    -1,
     198,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   159,    10,    11,    12,
      13,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,    -1,    -1,   176,    27,    -1,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,   171,    -1,
     173,    -1,    -1,   176,     3,     4,    -1,     6,     7,    -1,
     183,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   197,    -1,    -1,    -1,    27,    -1,
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
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,   165,   166,    -1,   168,
     169,   170,   171,    -1,   173,    -1,    -1,   176,     3,     4,
      -1,     6,     7,    -1,   183,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   197,    -1,
      -1,    -1,    27,    -1,    29,    -1,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,   171,    -1,   173,    -1,
      -1,   176,     3,     4,    -1,     6,     7,    -1,   183,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      31,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
     171,    -1,   173,    -1,    -1,   176,    -1,     3,     4,    -1,
       6,     7,   183,   184,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    31,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    59,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,   171,    -1,   173,    -1,    -1,
     176,     3,     4,     5,     6,     7,    -1,   183,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,   166,    -1,   168,   169,   170,   171,
      -1,   173,   174,    -1,   176,    10,    11,    12,    -1,    -1,
      -1,   183,   184,    -1,   186,    -1,   188,   189,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    69,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,   130,   131,   132,   133,   194,
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    27,   173,    29,
      -1,   176,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    27,   173,    29,    -1,   176,    -1,    -1,    -1,
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
      -1,    -1,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   159,    -1,    -1,    -1,    -1,    -1,
     165,   166,    -1,   168,   169,   170,   171,    27,   173,    29,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    31,    -1,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,   165,   166,    -1,   168,   169,
     170,   171,    -1,   173,    -1,    -1,   176,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    32,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,   159,    -1,    -1,   162,   163,
      -1,   165,   166,    91,   168,   169,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,   183,
      -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,    -1,   143,   144,   145,   146,   147,
      -1,    38,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,    -1,   165,   166,    -1,
     168,   169,   170,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,   183,    -1,    -1,    -1,    -1,
     188,    78,    79,    80,    81,   193,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    50,    51,    -1,   174,    -1,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    70,    -1,   193,    -1,    -1,    -1,
      -1,   198,    78,    79,    80,    81,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,    69,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,   165,
     166,    -1,   168,   169,   170,    70,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,   183,    83,    84,
      -1,    -1,   188,    -1,    -1,    -1,    91,   193,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    70,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,    -1,
      -1,   196,    -1,   198,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
      -1,   165,   166,    -1,   168,   169,   170,    70,    -1,    72,
     174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,   193,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    70,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   188,    -1,    -1,    -1,    91,
     193,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,   161,
     162,   163,    -1,   165,   166,    -1,   168,   169,   170,    70,
      -1,    -1,   174,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   188,    -1,    -1,    -1,
      91,   193,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,    -1,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,    -1,   165,   166,    -1,   168,   169,   170,
      -1,    -1,    -1,   174,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    30,    31,   198,    33,    34,
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
      -1,    30,    31,   136,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,   136,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   136,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,    -1,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,   136,    -1,   159,   160,   161,   162,
     163,    -1,   165,   166,    -1,   168,   169,   170,    -1,    -1,
      -1,   174,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   188,    91,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,    -1,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,    -1,
     165,   166,    -1,   168,   169,   170,    -1,    -1,    -1,   174,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   188,    91,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,   198,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,    -1,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,    -1,   165,   166,
      -1,   168,   169,   170,    -1,    -1,    -1,   174,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,   193,    28,    -1,    30,
      31,   198,    33,    34,    35,    36,    37,    38,    39,    40,
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
       0,   204,   205,     0,   206,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    49,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   141,   143,
     144,   145,   146,   147,   151,   154,   159,   160,   161,   162,
     163,   165,   166,   168,   169,   170,   171,   174,   177,   183,
     184,   186,   188,   189,   190,   193,   195,   196,   198,   199,
     201,   202,   207,   210,   220,   221,   222,   223,   224,   227,
     243,   244,   248,   251,   258,   264,   324,   325,   333,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   348,
     351,   363,   364,   371,   374,   377,   383,   385,   386,   388,
     398,   399,   400,   402,   407,   411,   431,   439,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     466,   468,   470,   122,   123,   124,   137,   159,   169,   193,
     210,   243,   324,   345,   443,   345,   193,   345,   345,   345,
     108,   345,   345,   345,   429,   430,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,    81,    83,
      91,   124,   139,   140,   141,   154,   193,   221,   364,   399,
     402,   407,   443,   446,   443,    38,   345,   457,   458,   345,
     124,   130,   193,   221,   256,   399,   400,   401,   403,   407,
     440,   441,   442,   450,   454,   455,   193,   334,   404,   193,
     334,   355,   335,   345,   229,   334,   193,   193,   193,   334,
     195,   345,   210,   195,   345,     3,     4,     6,     7,    10,
      11,    12,    13,    27,    29,    31,    57,    59,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   130,
     131,   132,   133,   137,   138,   142,   159,   163,   171,   173,
     176,   183,   193,   210,   211,   212,   223,   471,   488,   489,
     492,   195,   340,   342,   345,   196,   236,   345,   111,   112,
     162,   213,   214,   215,   216,   220,    83,   198,   290,   291,
     123,   130,   122,   130,    83,   292,   193,   193,   193,   193,
     210,   262,   474,   193,   193,    70,    70,    70,   335,    83,
      90,   155,   156,   157,   463,   464,   162,   196,   220,   220,
     210,   263,   474,   163,   193,   474,   474,    83,   190,   196,
     356,    27,   333,   337,   345,   346,   443,   447,   225,   196,
     452,    90,   405,   463,    90,   463,   463,    32,   162,   179,
     475,   193,     9,   195,    38,   242,   163,   261,   474,   124,
     189,   243,   325,   195,   195,   195,   195,   195,   195,   195,
     195,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   195,    70,    70,   196,   158,   131,   169,   171,   184,
     186,   264,   323,   324,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    67,    68,   134,
     135,   433,    70,   196,   438,   193,   193,    70,   196,   193,
     242,   243,    14,   345,   195,   136,    48,   210,   428,    90,
     333,   346,   158,   443,   136,   200,     9,   413,   257,   333,
     346,   443,   475,   158,   193,   406,   433,   438,   194,   345,
      32,   227,     8,   357,     9,   195,   227,   228,   335,   336,
     345,   210,   276,   231,   195,   195,   195,   138,   142,   492,
     492,   179,   491,   193,   111,   492,    14,   158,   138,   142,
     159,   210,   212,   195,   195,   195,   237,   115,   176,   195,
     213,   215,   213,   215,   220,   196,     9,   414,   195,   102,
     162,   196,   443,     9,   195,   130,   130,    14,     9,   195,
     443,   467,   335,   333,   346,   443,   446,   447,   194,   179,
     254,   137,   443,   456,   457,   345,   365,   366,   335,   380,
     380,   195,    70,   433,   155,   464,    82,   345,   443,    90,
     155,   464,   220,   209,   195,   196,   249,   259,   389,   391,
      91,   193,   358,   359,   361,   402,   449,   451,   468,    14,
     102,   469,   352,   353,   354,   286,   287,   431,   432,   194,
     194,   194,   194,   194,   197,   226,   227,   244,   251,   258,
     431,   345,   199,   201,   202,   210,   476,   477,   492,    38,
     172,   288,   289,   345,   471,   193,   474,   252,   242,   345,
     345,   345,   345,    32,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   403,   345,
     345,   453,   453,   345,   459,   460,   130,   196,   211,   212,
     452,   262,   210,   263,   474,   474,   261,   243,    38,   337,
     340,   342,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   163,   196,   210,   434,   435,
     436,   437,   452,   453,   345,   288,   288,   453,   345,   456,
     242,   194,   345,   193,   427,     9,   413,   194,   194,    38,
     345,    38,   345,   406,   194,   194,   194,   450,   451,   452,
     288,   196,   210,   434,   435,   452,   194,   225,   280,   196,
     342,   345,   345,    94,    32,   227,   274,   195,    28,   102,
      14,     9,   194,    32,   196,   277,   492,    31,    91,   223,
     485,   486,   487,   193,     9,    50,    51,    56,    58,    70,
     138,   139,   140,   141,   163,   183,   193,   221,   223,   372,
     375,   378,   384,   399,   407,   408,   410,   210,   490,   225,
     193,   235,   196,   195,   196,   195,   102,   162,   111,   112,
     162,   216,   217,   218,   219,   220,   216,   210,   345,   291,
     408,    83,     9,   194,   194,   194,   194,   194,   194,   194,
     195,    50,    51,   481,   483,   484,   132,   267,   193,     9,
     194,   194,   136,   200,     9,   413,     9,   413,   200,   200,
      83,    85,   210,   465,   210,    70,   197,   197,   206,   208,
      32,   133,   266,   178,    54,   163,   178,   393,   346,   136,
       9,   413,   194,   158,   492,   492,    14,   357,   286,   225,
     191,     9,   414,   492,   493,   433,   438,   433,   197,     9,
     413,   180,   443,   345,   194,     9,   414,    14,   349,   245,
     132,   265,   193,   474,   345,    32,   200,   200,   136,   197,
       9,   413,   345,   475,   193,   255,   250,   260,    14,   469,
     253,   242,    72,   443,   345,   475,   200,   197,   194,   194,
     200,   197,   194,    50,    51,    70,    78,    79,    80,    91,
     138,   139,   140,   141,   154,   183,   210,   373,   376,   379,
     416,   418,   419,   423,   426,   210,   443,   443,   136,   265,
     433,   438,   194,   345,   281,    75,    76,   282,   225,   334,
     225,   336,   102,    38,   137,   271,   443,   408,   210,    32,
     227,   275,   195,   278,   195,   278,     9,   413,    91,   136,
     158,     9,   413,   194,   172,   476,   477,   478,   476,   408,
     408,   408,   408,   408,   412,   415,   193,    70,    70,    70,
     158,   193,   408,   158,   196,    10,    11,    12,    31,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    69,   158,   475,   197,   399,   196,   239,   215,   215,
     210,   216,   216,   220,     9,   414,   197,   197,    14,   443,
     195,   180,     9,   413,   210,   268,   399,   196,   456,   137,
     443,    14,    38,   345,   345,   200,   345,   197,   206,   492,
     268,   196,   392,    14,   194,   345,   358,   452,   195,   492,
     191,   197,    32,   479,   432,    38,    83,   172,   434,   435,
     437,   434,   435,   492,    38,   172,   345,   408,   286,   193,
     399,   266,   350,   246,   345,   345,   345,   197,   193,   288,
     267,    32,   266,   492,    14,   265,   474,   403,   197,   193,
      14,    78,    79,    80,   210,   417,   417,   419,   421,   422,
      52,   193,    70,    70,    70,    90,   155,   193,     9,   413,
     194,   427,    38,   345,   266,   197,    75,    76,   283,   334,
     227,   197,   195,    95,   195,   271,   443,   193,   136,   270,
      14,   225,   278,   105,   106,   107,   278,   197,   492,   180,
     136,   492,   210,   485,     9,   194,   413,   136,   200,     9,
     413,   412,   367,   368,   408,   381,   408,   409,   381,   130,
     211,   358,   360,   362,   194,   130,   211,   408,   461,   462,
     408,   408,   408,    32,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   490,    83,
     240,   197,   197,   219,   195,   408,   484,   102,   103,   480,
     482,     9,   296,   194,   193,   337,   342,   345,   443,   136,
     200,   197,   469,   296,   164,   177,   196,   388,   395,   164,
     196,   394,   136,   195,   479,   492,   357,   493,    83,   172,
      14,    83,   475,   443,   345,   194,   286,   196,   286,   193,
     136,   193,   288,   194,   196,   492,   196,   195,   492,   266,
     247,   406,   288,   136,   200,     9,   413,   418,   421,   369,
     370,   419,   382,   419,   420,   382,   155,   358,   424,   425,
     419,   443,   196,   334,    32,    77,   227,   195,   336,   270,
     456,   271,   194,   408,   101,   105,   195,   345,    32,   195,
     279,   197,   180,   492,   136,   172,    32,   194,   408,   408,
     194,   200,     9,   413,   136,   200,     9,   413,   200,   136,
       9,   413,   194,   136,   197,     9,   413,   408,    32,   194,
     225,   195,   195,   210,   492,   492,   480,   399,     4,   112,
     117,   123,   125,   165,   166,   168,   197,   297,   322,   323,
     324,   329,   330,   331,   332,   431,   456,    38,   345,   197,
     196,   197,    54,   345,   345,   345,   357,    38,    83,   172,
      14,    83,   345,   193,   479,   194,   296,   194,   286,   345,
     288,   194,   296,   469,   296,   195,   196,   193,   194,   419,
     419,   194,   200,     9,   413,   136,   200,     9,   413,   200,
     136,   194,     9,   413,   296,    32,   225,   195,   194,   194,
     194,   232,   195,   195,   279,   225,   492,   492,   136,   408,
     408,   408,   408,   358,   408,   408,   408,   196,   197,   482,
     132,   133,   184,   211,   472,   492,   269,   399,   112,   332,
      31,   125,   138,   142,   163,   169,   306,   307,   308,   309,
     399,   167,   314,   315,   128,   193,   210,   316,   317,   298,
     243,   492,     9,   195,     9,   195,   195,   469,   323,   194,
     443,   293,   163,   390,   197,   197,    83,   172,    14,    83,
     345,   288,   117,   347,   479,   197,   479,   194,   194,   197,
     196,   197,   296,   286,   136,   419,   419,   419,   419,   358,
     197,   225,   230,   233,    32,   227,   273,   225,   194,   408,
     136,   136,   136,   225,   399,   399,   474,    14,   211,     9,
     195,   196,   472,   469,   309,   179,   196,     9,   195,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      29,    57,    71,    72,    73,    74,    75,    76,    77,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     137,   138,   143,   144,   145,   146,   147,   159,   160,   161,
     171,   173,   174,   176,   183,   184,   186,   188,   189,   210,
     396,   397,     9,   195,   163,   167,   210,   317,   318,   319,
     195,    83,   328,   242,   299,   472,   472,    14,   243,   197,
     294,   295,   472,    14,    83,   345,   194,   193,   479,   195,
     196,   320,   347,   479,   293,   197,   194,   419,   136,   136,
      32,   227,   272,   273,   225,   408,   408,   408,   197,   195,
     195,   408,   399,   302,   492,   310,   311,   407,   307,    14,
      32,    51,   312,   315,     9,    36,   194,    31,    50,    53,
      14,     9,   195,   212,   473,   328,    14,   492,   242,   195,
      14,   345,    38,    83,   387,   196,   225,   479,   320,   197,
     479,   419,   419,   225,    99,   238,   197,   210,   223,   303,
     304,   305,     9,   413,     9,   413,   197,   408,   397,   397,
      59,   313,   318,   318,    31,    50,    53,   408,    83,   179,
     193,   195,   408,   474,   408,    83,     9,   414,   225,   197,
     196,   320,    97,   195,   115,   234,   158,   102,   492,   180,
     407,   170,    14,   481,   300,   193,    38,    83,   194,   197,
     225,   195,   193,   176,   241,   210,   323,   324,   180,   408,
     180,   284,   285,   432,   301,    83,   197,   399,   239,   173,
     210,   195,   194,     9,   414,   119,   120,   121,   326,   327,
     284,    83,   269,   195,   479,   432,   493,   194,   194,   195,
     195,   196,   321,   326,    38,    83,   172,   479,   196,   225,
     493,    83,   172,    14,    83,   321,   225,   197,    38,    83,
     172,    14,    83,   345,   197,    83,   172,    14,    83,   345,
      14,    83,   345,   345
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
#line 735 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
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

  case 202:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
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

  case 204:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval).reset();;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { (yyval).reset();;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval).reset();;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval).reset();;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { (yyval).reset();;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval).reset();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval).reset();;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval).reset();;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval).reset();;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval).reset();;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval).reset();;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onYieldFrom((yyval),&(yyvsp[(2) - (2)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PIPE);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[(2) - (12)]),(yyvsp[(5) - (12)]),(yyvsp[(8) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(7) - (12)]),&(yyvsp[(9) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->finishStatement((yyvsp[(12) - (13)]), (yyvsp[(12) - (13)])); (yyvsp[(12) - (13)]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[(1) - (13)]),
                                           (yyvsp[(3) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(12) - (13)]),(yyvsp[(8) - (13)]),&(yyvsp[(10) - (13)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
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

  case 531:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
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

  case 533:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onDict((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onVec((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onKeyset((yyval), (yyvsp[(3) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval).reset();;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval).reset();;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
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

  case 601:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
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

  case 602:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval).reset();;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval).reset();;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval).reset();;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval).reset();;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval).reset();;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval).reset();;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { (yyval).reset();;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval).reset();;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 867:

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

  case 868:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
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

  case 869:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
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

  case 870:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
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

  case 871:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
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
#line 2930 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
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

  case 887:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onPipeVariable((yyval));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { (yyval).reset();;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
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
#line 3076 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
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

  case 928:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { (yyval).reset();;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
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
#line 3142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
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

  case 964:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3184 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3238 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3255 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3261 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3282 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3287 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3290 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3309 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1; ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3335 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    {;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    {;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    {;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3350 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3355 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3371 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3372 "hphp.y"
    { ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3377 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3378 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1030:

/* Line 1455 of yacc.c  */
#line 3389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1031:

/* Line 1455 of yacc.c  */
#line 3394 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1032:

/* Line 1455 of yacc.c  */
#line 3398 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1033:

/* Line 1455 of yacc.c  */
#line 3403 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 1034:

/* Line 1455 of yacc.c  */
#line 3405 "hphp.y"
    { _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); (yyval) = (yyvsp[(2) - (5)]);;}
    break;

  case 1035:

/* Line 1455 of yacc.c  */
#line 3411 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1036:

/* Line 1455 of yacc.c  */
#line 3414 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1037:

/* Line 1455 of yacc.c  */
#line 3417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1038:

/* Line 1455 of yacc.c  */
#line 3418 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1039:

/* Line 1455 of yacc.c  */
#line 3421 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1040:

/* Line 1455 of yacc.c  */
#line 3424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1041:

/* Line 1455 of yacc.c  */
#line 3427 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1042:

/* Line 1455 of yacc.c  */
#line 3430 "hphp.y"
    { (yyvsp[(1) - (2)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1043:

/* Line 1455 of yacc.c  */
#line 3432 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1044:

/* Line 1455 of yacc.c  */
#line 3438 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1045:

/* Line 1455 of yacc.c  */
#line 3444 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1046:

/* Line 1455 of yacc.c  */
#line 3452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1047:

/* Line 1455 of yacc.c  */
#line 3453 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14519 "hphp.7.tab.cpp"
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
#line 3456 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

