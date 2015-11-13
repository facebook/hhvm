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
     T_COALESCE = 282,
     T_BOOLEAN_OR = 283,
     T_BOOLEAN_AND = 284,
     T_IS_NOT_IDENTICAL = 285,
     T_IS_IDENTICAL = 286,
     T_IS_NOT_EQUAL = 287,
     T_IS_EQUAL = 288,
     T_SPACESHIP = 289,
     T_IS_GREATER_OR_EQUAL = 290,
     T_IS_SMALLER_OR_EQUAL = 291,
     T_SR = 292,
     T_SL = 293,
     T_INSTANCEOF = 294,
     T_UNSET_CAST = 295,
     T_BOOL_CAST = 296,
     T_OBJECT_CAST = 297,
     T_ARRAY_CAST = 298,
     T_STRING_CAST = 299,
     T_DOUBLE_CAST = 300,
     T_INT_CAST = 301,
     T_DEC = 302,
     T_INC = 303,
     T_POW = 304,
     T_CLONE = 305,
     T_NEW = 306,
     T_EXIT = 307,
     T_IF = 308,
     T_ELSEIF = 309,
     T_ELSE = 310,
     T_ENDIF = 311,
     T_LNUMBER = 312,
     T_DNUMBER = 313,
     T_ONUMBER = 314,
     T_STRING = 315,
     T_STRING_VARNAME = 316,
     T_VARIABLE = 317,
     T_NUM_STRING = 318,
     T_INLINE_HTML = 319,
     T_HASHBANG = 320,
     T_CHARACTER = 321,
     T_BAD_CHARACTER = 322,
     T_ENCAPSED_AND_WHITESPACE = 323,
     T_CONSTANT_ENCAPSED_STRING = 324,
     T_ECHO = 325,
     T_DO = 326,
     T_WHILE = 327,
     T_ENDWHILE = 328,
     T_FOR = 329,
     T_ENDFOR = 330,
     T_FOREACH = 331,
     T_ENDFOREACH = 332,
     T_DECLARE = 333,
     T_ENDDECLARE = 334,
     T_AS = 335,
     T_SUPER = 336,
     T_SWITCH = 337,
     T_ENDSWITCH = 338,
     T_CASE = 339,
     T_DEFAULT = 340,
     T_BREAK = 341,
     T_GOTO = 342,
     T_CONTINUE = 343,
     T_FUNCTION = 344,
     T_CONST = 345,
     T_RETURN = 346,
     T_TRY = 347,
     T_CATCH = 348,
     T_THROW = 349,
     T_USE = 350,
     T_GLOBAL = 351,
     T_PUBLIC = 352,
     T_PROTECTED = 353,
     T_PRIVATE = 354,
     T_FINAL = 355,
     T_ABSTRACT = 356,
     T_STATIC = 357,
     T_VAR = 358,
     T_UNSET = 359,
     T_ISSET = 360,
     T_EMPTY = 361,
     T_HALT_COMPILER = 362,
     T_CLASS = 363,
     T_INTERFACE = 364,
     T_EXTENDS = 365,
     T_IMPLEMENTS = 366,
     T_OBJECT_OPERATOR = 367,
     T_NULLSAFE_OBJECT_OPERATOR = 368,
     T_DOUBLE_ARROW = 369,
     T_LIST = 370,
     T_ARRAY = 371,
     T_CALLABLE = 372,
     T_CLASS_C = 373,
     T_METHOD_C = 374,
     T_FUNC_C = 375,
     T_LINE = 376,
     T_FILE = 377,
     T_COMMENT = 378,
     T_DOC_COMMENT = 379,
     T_OPEN_TAG = 380,
     T_OPEN_TAG_WITH_ECHO = 381,
     T_CLOSE_TAG = 382,
     T_WHITESPACE = 383,
     T_START_HEREDOC = 384,
     T_END_HEREDOC = 385,
     T_DOLLAR_OPEN_CURLY_BRACES = 386,
     T_CURLY_OPEN = 387,
     T_DOUBLE_COLON = 388,
     T_NAMESPACE = 389,
     T_NS_C = 390,
     T_DIR = 391,
     T_NS_SEPARATOR = 392,
     T_XHP_LABEL = 393,
     T_XHP_TEXT = 394,
     T_XHP_ATTRIBUTE = 395,
     T_XHP_CATEGORY = 396,
     T_XHP_CATEGORY_LABEL = 397,
     T_XHP_CHILDREN = 398,
     T_ENUM = 399,
     T_XHP_REQUIRED = 400,
     T_TRAIT = 401,
     T_ELLIPSIS = 402,
     T_INSTEADOF = 403,
     T_TRAIT_C = 404,
     T_HH_ERROR = 405,
     T_FINALLY = 406,
     T_XHP_TAG_LT = 407,
     T_XHP_TAG_GT = 408,
     T_TYPELIST_LT = 409,
     T_TYPELIST_GT = 410,
     T_UNRESOLVED_LT = 411,
     T_COLLECTION = 412,
     T_SHAPE = 413,
     T_TYPE = 414,
     T_UNRESOLVED_TYPE = 415,
     T_NEWTYPE = 416,
     T_UNRESOLVED_NEWTYPE = 417,
     T_COMPILER_HALT_OFFSET = 418,
     T_ASYNC = 419,
     T_LAMBDA_OP = 420,
     T_LAMBDA_CP = 421,
     T_UNRESOLVED_OP = 422
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
#line 873 "hphp.5.tab.cpp"

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
#define YYLAST   16619

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  267
/* YYNRULES -- Number of rules.  */
#define YYNRULES  985
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1809

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   422

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,   195,     2,   192,    53,    36,   196,
     187,   188,    51,    48,     9,    49,    50,    52,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   189,
      41,    14,    42,    29,    57,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,   194,    35,     2,   193,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   190,    34,   191,    56,     2,     2,     2,
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
      27,    28,    31,    32,    33,    37,    38,    39,    40,    43,
      44,    45,    46,    47,    55,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    69,    70,    71,    72,    73,
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
     184,   185,   186
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
     217,   219,   221,   223,   225,   227,   229,   232,   236,   240,
     242,   245,   247,   250,   254,   259,   263,   265,   268,   270,
     273,   276,   278,   282,   284,   288,   291,   294,   297,   303,
     308,   311,   312,   314,   316,   318,   320,   324,   330,   339,
     340,   345,   346,   353,   354,   365,   366,   371,   374,   378,
     381,   385,   388,   392,   396,   400,   404,   408,   412,   418,
     420,   422,   424,   425,   435,   436,   447,   453,   454,   468,
     469,   475,   479,   483,   486,   489,   492,   495,   498,   501,
     505,   508,   511,   512,   517,   527,   528,   529,   534,   537,
     538,   540,   541,   543,   544,   554,   555,   566,   567,   579,
     580,   590,   591,   602,   603,   612,   613,   623,   624,   632,
     633,   642,   643,   652,   653,   661,   662,   671,   673,   675,
     677,   679,   681,   684,   688,   692,   695,   698,   699,   702,
     703,   706,   707,   709,   713,   715,   719,   722,   723,   725,
     728,   733,   735,   740,   742,   747,   749,   754,   756,   761,
     765,   771,   775,   780,   785,   791,   797,   802,   803,   805,
     807,   812,   813,   819,   820,   823,   824,   828,   829,   837,
     846,   853,   856,   862,   869,   874,   875,   880,   886,   894,
     901,   908,   916,   926,   935,   942,   950,   956,   959,   964,
     970,   974,   975,   979,   984,   991,   997,  1003,  1010,  1019,
    1027,  1030,  1031,  1033,  1036,  1039,  1043,  1048,  1053,  1057,
    1059,  1061,  1064,  1069,  1073,  1079,  1081,  1085,  1088,  1089,
    1092,  1096,  1099,  1100,  1101,  1106,  1107,  1113,  1116,  1119,
    1122,  1123,  1134,  1135,  1147,  1151,  1155,  1159,  1164,  1169,
    1173,  1179,  1182,  1185,  1186,  1193,  1199,  1204,  1208,  1210,
    1212,  1216,  1221,  1223,  1226,  1228,  1230,  1235,  1242,  1244,
    1246,  1251,  1253,  1255,  1259,  1262,  1265,  1266,  1269,  1270,
    1272,  1276,  1278,  1280,  1282,  1284,  1288,  1293,  1298,  1303,
    1305,  1307,  1310,  1313,  1316,  1320,  1324,  1326,  1328,  1330,
    1332,  1336,  1338,  1342,  1344,  1346,  1348,  1349,  1351,  1354,
    1356,  1358,  1360,  1362,  1364,  1366,  1368,  1370,  1371,  1373,
    1375,  1377,  1381,  1387,  1389,  1393,  1399,  1404,  1408,  1412,
    1416,  1421,  1425,  1429,  1433,  1436,  1439,  1441,  1443,  1447,
    1451,  1453,  1455,  1456,  1458,  1461,  1466,  1470,  1474,  1481,
    1484,  1488,  1495,  1497,  1499,  1501,  1503,  1505,  1512,  1516,
    1521,  1528,  1532,  1536,  1540,  1544,  1548,  1552,  1556,  1560,
    1564,  1568,  1572,  1576,  1579,  1582,  1585,  1588,  1592,  1596,
    1600,  1604,  1608,  1612,  1616,  1620,  1624,  1628,  1632,  1636,
    1640,  1644,  1648,  1652,  1656,  1659,  1662,  1665,  1668,  1672,
    1676,  1680,  1684,  1688,  1692,  1696,  1700,  1704,  1708,  1712,
    1718,  1723,  1727,  1729,  1732,  1735,  1738,  1741,  1744,  1747,
    1750,  1753,  1756,  1758,  1760,  1762,  1766,  1769,  1771,  1777,
    1778,  1779,  1791,  1792,  1805,  1806,  1811,  1812,  1820,  1821,
    1827,  1828,  1832,  1833,  1840,  1843,  1846,  1851,  1853,  1855,
    1861,  1865,  1871,  1875,  1878,  1879,  1882,  1883,  1888,  1893,
    1897,  1902,  1907,  1912,  1917,  1919,  1921,  1923,  1925,  1929,
    1933,  1938,  1940,  1943,  1948,  1951,  1958,  1959,  1961,  1966,
    1967,  1970,  1971,  1973,  1975,  1979,  1981,  1985,  1987,  1989,
    1993,  1997,  1999,  2001,  2003,  2005,  2007,  2009,  2011,  2013,
    2015,  2017,  2019,  2021,  2023,  2025,  2027,  2029,  2031,  2033,
    2035,  2037,  2039,  2041,  2043,  2045,  2047,  2049,  2051,  2053,
    2055,  2057,  2059,  2061,  2063,  2065,  2067,  2069,  2071,  2073,
    2075,  2077,  2079,  2081,  2083,  2085,  2087,  2089,  2091,  2093,
    2095,  2097,  2099,  2101,  2103,  2105,  2107,  2109,  2111,  2113,
    2115,  2117,  2119,  2121,  2123,  2125,  2127,  2129,  2131,  2133,
    2135,  2137,  2139,  2141,  2143,  2145,  2147,  2149,  2151,  2153,
    2155,  2157,  2162,  2164,  2166,  2168,  2170,  2172,  2174,  2178,
    2180,  2184,  2186,  2188,  2192,  2194,  2196,  2198,  2201,  2203,
    2204,  2205,  2207,  2209,  2213,  2214,  2216,  2218,  2220,  2222,
    2224,  2226,  2228,  2230,  2232,  2234,  2236,  2238,  2240,  2244,
    2247,  2249,  2251,  2256,  2260,  2265,  2267,  2269,  2273,  2277,
    2281,  2285,  2289,  2293,  2297,  2301,  2305,  2309,  2313,  2317,
    2321,  2325,  2329,  2333,  2337,  2341,  2344,  2347,  2350,  2353,
    2357,  2361,  2365,  2369,  2373,  2377,  2381,  2385,  2389,  2395,
    2400,  2404,  2408,  2412,  2414,  2416,  2418,  2420,  2424,  2428,
    2432,  2435,  2436,  2438,  2439,  2441,  2442,  2448,  2452,  2456,
    2458,  2460,  2462,  2464,  2468,  2471,  2473,  2475,  2477,  2479,
    2481,  2485,  2487,  2489,  2491,  2494,  2497,  2502,  2506,  2511,
    2514,  2515,  2521,  2525,  2529,  2531,  2535,  2537,  2540,  2541,
    2547,  2551,  2554,  2555,  2559,  2560,  2565,  2568,  2569,  2573,
    2577,  2579,  2580,  2582,  2584,  2586,  2588,  2592,  2594,  2596,
    2598,  2602,  2604,  2606,  2610,  2614,  2617,  2622,  2625,  2630,
    2636,  2642,  2648,  2654,  2656,  2658,  2660,  2662,  2664,  2666,
    2670,  2674,  2679,  2684,  2688,  2690,  2692,  2694,  2696,  2700,
    2702,  2707,  2711,  2713,  2715,  2717,  2719,  2721,  2725,  2729,
    2734,  2739,  2743,  2745,  2747,  2755,  2765,  2773,  2780,  2789,
    2791,  2794,  2799,  2804,  2806,  2808,  2813,  2815,  2816,  2818,
    2821,  2823,  2825,  2827,  2831,  2835,  2839,  2840,  2842,  2844,
    2848,  2852,  2855,  2859,  2866,  2867,  2869,  2874,  2877,  2878,
    2884,  2888,  2892,  2894,  2901,  2906,  2911,  2914,  2917,  2918,
    2924,  2928,  2932,  2934,  2937,  2938,  2944,  2948,  2952,  2954,
    2957,  2960,  2962,  2965,  2967,  2972,  2976,  2980,  2987,  2991,
    2993,  2995,  2997,  3002,  3007,  3012,  3017,  3022,  3027,  3030,
    3033,  3038,  3041,  3044,  3046,  3050,  3054,  3058,  3059,  3062,
    3068,  3075,  3082,  3090,  3092,  3095,  3097,  3100,  3102,  3107,
    3109,  3114,  3118,  3119,  3121,  3125,  3128,  3132,  3134,  3136,
    3137,  3138,  3141,  3144,  3147,  3152,  3155,  3161,  3165,  3167,
    3169,  3170,  3174,  3179,  3185,  3189,  3191,  3194,  3195,  3200,
    3202,  3206,  3209,  3212,  3215,  3217,  3219,  3221,  3223,  3227,
    3232,  3239,  3241,  3250,  3257,  3259
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   221,    -1,   238,    -1,   245,    -1,   242,    -1,   252,
      -1,   443,    -1,   126,   187,   188,   189,    -1,   153,   214,
     189,    -1,    -1,   153,   214,   190,   202,   200,   191,    -1,
      -1,   153,   190,   203,   200,   191,    -1,   114,   209,   189,
      -1,   114,   108,   209,   189,    -1,   114,   109,   209,   189,
      -1,   114,   207,   190,   212,   191,   189,    -1,   114,   108,
     207,   190,   209,   191,   189,    -1,   114,   109,   207,   190,
     209,   191,   189,    -1,   218,   189,    -1,    79,    -1,   100,
      -1,   159,    -1,   160,    -1,   162,    -1,   164,    -1,   163,
      -1,   204,    -1,   136,    -1,   165,    -1,   129,    -1,   130,
      -1,   121,    -1,   120,    -1,   119,    -1,   118,    -1,   117,
      -1,   116,    -1,   109,    -1,    98,    -1,    94,    -1,    96,
      -1,    75,    -1,    92,    -1,    12,    -1,   115,    -1,   106,
      -1,    55,    -1,   167,    -1,   128,    -1,   153,    -1,    70,
      -1,    10,    -1,    11,    -1,   111,    -1,   114,    -1,   122,
      -1,    71,    -1,   134,    -1,    69,    -1,     7,    -1,     6,
      -1,   113,    -1,   135,    -1,    13,    -1,    89,    -1,     4,
      -1,     3,    -1,   110,    -1,    74,    -1,    73,    -1,   104,
      -1,   105,    -1,   107,    -1,   101,    -1,    27,    -1,   108,
      -1,    72,    -1,   102,    -1,   170,    -1,    93,    -1,    95,
      -1,    97,    -1,   103,    -1,    90,    -1,    91,    -1,    99,
      -1,   112,    -1,   123,    -1,   205,    -1,   127,    -1,   214,
     156,    -1,   156,   214,   156,    -1,   208,     9,   210,    -1,
     210,    -1,   208,   387,    -1,   214,    -1,   156,   214,    -1,
     214,    99,   204,    -1,   156,   214,    99,   204,    -1,   211,
       9,   213,    -1,   213,    -1,   211,   387,    -1,   210,    -1,
     108,   210,    -1,   109,   210,    -1,   204,    -1,   214,   156,
     204,    -1,   214,    -1,   153,   156,   214,    -1,   156,   214,
      -1,   215,   448,    -1,   215,   448,    -1,   218,     9,   444,
      14,   382,    -1,   109,   444,    14,   382,    -1,   219,   220,
      -1,    -1,   221,    -1,   238,    -1,   245,    -1,   252,    -1,
     190,   219,   191,    -1,    72,   328,   221,   274,   276,    -1,
      72,   328,    30,   219,   275,   277,    75,   189,    -1,    -1,
      91,   328,   222,   268,    -1,    -1,    90,   223,   221,    91,
     328,   189,    -1,    -1,    93,   187,   330,   189,   330,   189,
     330,   188,   224,   266,    -1,    -1,   101,   328,   225,   271,
      -1,   105,   189,    -1,   105,   337,   189,    -1,   107,   189,
      -1,   107,   337,   189,    -1,   110,   189,    -1,   110,   337,
     189,    -1,    27,   105,   189,    -1,   115,   284,   189,    -1,
     121,   286,   189,    -1,    89,   329,   189,    -1,   145,   329,
     189,    -1,   123,   187,   440,   188,   189,    -1,   189,    -1,
      83,    -1,    84,    -1,    -1,    95,   187,   337,    99,   265,
     264,   188,   226,   267,    -1,    -1,    95,   187,   337,    28,
      99,   265,   264,   188,   227,   267,    -1,    97,   187,   270,
     188,   269,    -1,    -1,   111,   230,   112,   187,   373,    81,
     188,   190,   219,   191,   232,   228,   235,    -1,    -1,   111,
     230,   170,   229,   233,    -1,   113,   337,   189,    -1,   106,
     204,   189,    -1,   337,   189,    -1,   331,   189,    -1,   332,
     189,    -1,   333,   189,    -1,   334,   189,    -1,   335,   189,
      -1,   110,   334,   189,    -1,   336,   189,    -1,   204,    30,
      -1,    -1,   190,   231,   219,   191,    -1,   232,   112,   187,
     373,    81,   188,   190,   219,   191,    -1,    -1,    -1,   190,
     234,   219,   191,    -1,   170,   233,    -1,    -1,    36,    -1,
      -1,   108,    -1,    -1,   237,   236,   447,   239,   187,   280,
     188,   452,   314,    -1,    -1,   318,   237,   236,   447,   240,
     187,   280,   188,   452,   314,    -1,    -1,   403,   317,   237,
     236,   447,   241,   187,   280,   188,   452,   314,    -1,    -1,
     163,   204,   243,    30,   462,   442,   190,   287,   191,    -1,
      -1,   403,   163,   204,   244,    30,   462,   442,   190,   287,
     191,    -1,    -1,   258,   255,   246,   259,   260,   190,   290,
     191,    -1,    -1,   403,   258,   255,   247,   259,   260,   190,
     290,   191,    -1,    -1,   128,   256,   248,   261,   190,   290,
     191,    -1,    -1,   403,   128,   256,   249,   261,   190,   290,
     191,    -1,    -1,   127,   251,   380,   259,   260,   190,   290,
     191,    -1,    -1,   165,   257,   253,   260,   190,   290,   191,
      -1,    -1,   403,   165,   257,   254,   260,   190,   290,   191,
      -1,   447,    -1,   157,    -1,   447,    -1,   447,    -1,   127,
      -1,   120,   127,    -1,   120,   119,   127,    -1,   119,   120,
     127,    -1,   119,   127,    -1,   129,   373,    -1,    -1,   130,
     262,    -1,    -1,   129,   262,    -1,    -1,   373,    -1,   262,
       9,   373,    -1,   373,    -1,   263,     9,   373,    -1,   133,
     265,    -1,    -1,   415,    -1,    36,   415,    -1,   134,   187,
     429,   188,    -1,   221,    -1,    30,   219,    94,   189,    -1,
     221,    -1,    30,   219,    96,   189,    -1,   221,    -1,    30,
     219,    92,   189,    -1,   221,    -1,    30,   219,    98,   189,
      -1,   204,    14,   382,    -1,   270,     9,   204,    14,   382,
      -1,   190,   272,   191,    -1,   190,   189,   272,   191,    -1,
      30,   272,   102,   189,    -1,    30,   189,   272,   102,   189,
      -1,   272,   103,   337,   273,   219,    -1,   272,   104,   273,
     219,    -1,    -1,    30,    -1,   189,    -1,   274,    73,   328,
     221,    -1,    -1,   275,    73,   328,    30,   219,    -1,    -1,
      74,   221,    -1,    -1,    74,    30,   219,    -1,    -1,   279,
       9,   404,   320,   463,   166,    81,    -1,   279,     9,   404,
     320,   463,    36,   166,    81,    -1,   279,     9,   404,   320,
     463,   166,    -1,   279,   387,    -1,   404,   320,   463,   166,
      81,    -1,   404,   320,   463,    36,   166,    81,    -1,   404,
     320,   463,   166,    -1,    -1,   404,   320,   463,    81,    -1,
     404,   320,   463,    36,    81,    -1,   404,   320,   463,    36,
      81,    14,   337,    -1,   404,   320,   463,    81,    14,   337,
      -1,   279,     9,   404,   320,   463,    81,    -1,   279,     9,
     404,   320,   463,    36,    81,    -1,   279,     9,   404,   320,
     463,    36,    81,    14,   337,    -1,   279,     9,   404,   320,
     463,    81,    14,   337,    -1,   281,     9,   404,   463,   166,
      81,    -1,   281,     9,   404,   463,    36,   166,    81,    -1,
     281,     9,   404,   463,   166,    -1,   281,   387,    -1,   404,
     463,   166,    81,    -1,   404,   463,    36,   166,    81,    -1,
     404,   463,   166,    -1,    -1,   404,   463,    81,    -1,   404,
     463,    36,    81,    -1,   404,   463,    36,    81,    14,   337,
      -1,   404,   463,    81,    14,   337,    -1,   281,     9,   404,
     463,    81,    -1,   281,     9,   404,   463,    36,    81,    -1,
     281,     9,   404,   463,    36,    81,    14,   337,    -1,   281,
       9,   404,   463,    81,    14,   337,    -1,   283,   387,    -1,
      -1,   337,    -1,    36,   415,    -1,   166,   337,    -1,   283,
       9,   337,    -1,   283,     9,   166,   337,    -1,   283,     9,
      36,   415,    -1,   284,     9,   285,    -1,   285,    -1,    81,
      -1,   192,   415,    -1,   192,   190,   337,   191,    -1,   286,
       9,    81,    -1,   286,     9,    81,    14,   382,    -1,    81,
      -1,    81,    14,   382,    -1,   287,   288,    -1,    -1,   289,
     189,    -1,   445,    14,   382,    -1,   290,   291,    -1,    -1,
      -1,   316,   292,   322,   189,    -1,    -1,   318,   462,   293,
     322,   189,    -1,   323,   189,    -1,   324,   189,    -1,   325,
     189,    -1,    -1,   317,   237,   236,   446,   187,   294,   278,
     188,   452,   315,    -1,    -1,   403,   317,   237,   236,   447,
     187,   295,   278,   188,   452,   315,    -1,   159,   300,   189,
      -1,   160,   308,   189,    -1,   162,   310,   189,    -1,     4,
     129,   373,   189,    -1,     4,   130,   373,   189,    -1,   114,
     263,   189,    -1,   114,   263,   190,   296,   191,    -1,   296,
     297,    -1,   296,   298,    -1,    -1,   217,   152,   204,   167,
     263,   189,    -1,   299,    99,   317,   204,   189,    -1,   299,
      99,   318,   189,    -1,   217,   152,   204,    -1,   204,    -1,
     301,    -1,   300,     9,   301,    -1,   302,   370,   306,   307,
      -1,   157,    -1,    29,   303,    -1,   303,    -1,   135,    -1,
     135,   173,   462,   174,    -1,   135,   173,   462,     9,   462,
     174,    -1,   373,    -1,   122,    -1,   163,   190,   305,   191,
      -1,   136,    -1,   381,    -1,   304,     9,   381,    -1,   304,
     386,    -1,    14,   382,    -1,    -1,    57,   164,    -1,    -1,
     309,    -1,   308,     9,   309,    -1,   161,    -1,   311,    -1,
     204,    -1,   125,    -1,   187,   312,   188,    -1,   187,   312,
     188,    51,    -1,   187,   312,   188,    29,    -1,   187,   312,
     188,    48,    -1,   311,    -1,   313,    -1,   313,    51,    -1,
     313,    29,    -1,   313,    48,    -1,   312,     9,   312,    -1,
     312,    34,   312,    -1,   204,    -1,   157,    -1,   161,    -1,
     189,    -1,   190,   219,   191,    -1,   189,    -1,   190,   219,
     191,    -1,   318,    -1,   122,    -1,   318,    -1,    -1,   319,
      -1,   318,   319,    -1,   116,    -1,   117,    -1,   118,    -1,
     121,    -1,   120,    -1,   119,    -1,   183,    -1,   321,    -1,
      -1,   116,    -1,   117,    -1,   118,    -1,   322,     9,    81,
      -1,   322,     9,    81,    14,   382,    -1,    81,    -1,    81,
      14,   382,    -1,   323,     9,   445,    14,   382,    -1,   109,
     445,    14,   382,    -1,   324,     9,   445,    -1,   120,   109,
     445,    -1,   120,   326,   442,    -1,   326,   442,    14,   462,
      -1,   109,   178,   447,    -1,   187,   327,   188,    -1,    70,
     377,   380,    -1,    70,   250,    -1,    69,   337,    -1,   362,
      -1,   357,    -1,   187,   337,   188,    -1,   329,     9,   337,
      -1,   337,    -1,   329,    -1,    -1,    27,    -1,    27,   337,
      -1,    27,   337,   133,   337,    -1,   187,   331,   188,    -1,
     415,    14,   331,    -1,   134,   187,   429,   188,    14,   331,
      -1,    28,   337,    -1,   415,    14,   334,    -1,   134,   187,
     429,   188,    14,   334,    -1,   338,    -1,   415,    -1,   327,
      -1,   419,    -1,   418,    -1,   134,   187,   429,   188,    14,
     337,    -1,   415,    14,   337,    -1,   415,    14,    36,   415,
      -1,   415,    14,    36,    70,   377,   380,    -1,   415,    26,
     337,    -1,   415,    25,   337,    -1,   415,    24,   337,    -1,
     415,    23,   337,    -1,   415,    22,   337,    -1,   415,    21,
     337,    -1,   415,    20,   337,    -1,   415,    19,   337,    -1,
     415,    18,   337,    -1,   415,    17,   337,    -1,   415,    16,
     337,    -1,   415,    15,   337,    -1,   415,    66,    -1,    66,
     415,    -1,   415,    65,    -1,    65,   415,    -1,   337,    32,
     337,    -1,   337,    33,   337,    -1,   337,    10,   337,    -1,
     337,    12,   337,    -1,   337,    11,   337,    -1,   337,    34,
     337,    -1,   337,    36,   337,    -1,   337,    35,   337,    -1,
     337,    50,   337,    -1,   337,    48,   337,    -1,   337,    49,
     337,    -1,   337,    51,   337,    -1,   337,    52,   337,    -1,
     337,    67,   337,    -1,   337,    53,   337,    -1,   337,    47,
     337,    -1,   337,    46,   337,    -1,    48,   337,    -1,    49,
     337,    -1,    54,   337,    -1,    56,   337,    -1,   337,    38,
     337,    -1,   337,    37,   337,    -1,   337,    40,   337,    -1,
     337,    39,   337,    -1,   337,    41,   337,    -1,   337,    45,
     337,    -1,   337,    42,   337,    -1,   337,    44,   337,    -1,
     337,    43,   337,    -1,   337,    55,   377,    -1,   187,   338,
     188,    -1,   337,    29,   337,    30,   337,    -1,   337,    29,
      30,   337,    -1,   337,    31,   337,    -1,   439,    -1,    64,
     337,    -1,    63,   337,    -1,    62,   337,    -1,    61,   337,
      -1,    60,   337,    -1,    59,   337,    -1,    58,   337,    -1,
      71,   378,    -1,    57,   337,    -1,   384,    -1,   356,    -1,
     355,    -1,   193,   379,   193,    -1,    13,   337,    -1,   359,
      -1,   114,   187,   361,   387,   188,    -1,    -1,    -1,   237,
     236,   187,   341,   280,   188,   452,   339,   190,   219,   191,
      -1,    -1,   318,   237,   236,   187,   342,   280,   188,   452,
     339,   190,   219,   191,    -1,    -1,   183,    81,   344,   349,
      -1,    -1,   183,   184,   345,   280,   185,   452,   349,    -1,
      -1,   183,   190,   346,   219,   191,    -1,    -1,    81,   347,
     349,    -1,    -1,   184,   348,   280,   185,   452,   349,    -1,
       8,   337,    -1,     8,   334,    -1,     8,   190,   219,   191,
      -1,    88,    -1,   441,    -1,   351,     9,   350,   133,   337,
      -1,   350,   133,   337,    -1,   352,     9,   350,   133,   382,
      -1,   350,   133,   382,    -1,   351,   386,    -1,    -1,   352,
     386,    -1,    -1,   177,   187,   353,   188,    -1,   135,   187,
     430,   188,    -1,    68,   430,   194,    -1,   373,   190,   432,
     191,    -1,   373,   190,   434,   191,    -1,   359,    68,   425,
     194,    -1,   360,    68,   425,   194,    -1,   356,    -1,   441,
      -1,   418,    -1,    88,    -1,   187,   338,   188,    -1,   361,
       9,    81,    -1,   361,     9,    36,    81,    -1,    81,    -1,
      36,    81,    -1,   171,   157,   363,   172,    -1,   365,    52,
      -1,   365,   172,   366,   171,    52,   364,    -1,    -1,   157,
      -1,   365,   367,    14,   368,    -1,    -1,   366,   369,    -1,
      -1,   157,    -1,   158,    -1,   190,   337,   191,    -1,   158,
      -1,   190,   337,   191,    -1,   362,    -1,   371,    -1,   370,
      30,   371,    -1,   370,    49,   371,    -1,   204,    -1,    71,
      -1,   108,    -1,   109,    -1,   110,    -1,    27,    -1,    28,
      -1,   111,    -1,   112,    -1,   170,    -1,   113,    -1,    72,
      -1,    73,    -1,    75,    -1,    74,    -1,    91,    -1,    92,
      -1,    90,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    97,    -1,    98,    -1,    55,    -1,    99,    -1,   101,
      -1,   102,    -1,   103,    -1,   104,    -1,   105,    -1,   107,
      -1,   106,    -1,    89,    -1,    13,    -1,   127,    -1,   128,
      -1,   129,    -1,   130,    -1,    70,    -1,    69,    -1,   122,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   153,    -1,   114,    -1,   115,    -1,   124,    -1,   125,
      -1,   126,    -1,   121,    -1,   120,    -1,   119,    -1,   118,
      -1,   117,    -1,   116,    -1,   183,    -1,   123,    -1,   134,
      -1,   135,    -1,    10,    -1,    12,    -1,    11,    -1,   137,
      -1,   139,    -1,   138,    -1,   140,    -1,   141,    -1,   155,
      -1,   154,    -1,   182,    -1,   165,    -1,   168,    -1,   167,
      -1,   178,    -1,   180,    -1,   177,    -1,   216,   187,   282,
     188,    -1,   217,    -1,   157,    -1,   373,    -1,   381,    -1,
     121,    -1,   423,    -1,   187,   338,   188,    -1,   374,    -1,
     375,   152,   422,    -1,   374,    -1,   421,    -1,   376,   152,
     422,    -1,   373,    -1,   121,    -1,   427,    -1,   187,   188,
      -1,   328,    -1,    -1,    -1,    87,    -1,   436,    -1,   187,
     282,   188,    -1,    -1,    76,    -1,    77,    -1,    78,    -1,
      88,    -1,   140,    -1,   141,    -1,   155,    -1,   137,    -1,
     168,    -1,   138,    -1,   139,    -1,   154,    -1,   182,    -1,
     148,    87,   149,    -1,   148,   149,    -1,   381,    -1,   215,
      -1,   135,   187,   385,   188,    -1,    68,   385,   194,    -1,
     177,   187,   354,   188,    -1,   383,    -1,   358,    -1,   187,
     382,   188,    -1,   382,    32,   382,    -1,   382,    33,   382,
      -1,   382,    10,   382,    -1,   382,    12,   382,    -1,   382,
      11,   382,    -1,   382,    34,   382,    -1,   382,    36,   382,
      -1,   382,    35,   382,    -1,   382,    50,   382,    -1,   382,
      48,   382,    -1,   382,    49,   382,    -1,   382,    51,   382,
      -1,   382,    52,   382,    -1,   382,    53,   382,    -1,   382,
      47,   382,    -1,   382,    46,   382,    -1,   382,    67,   382,
      -1,    54,   382,    -1,    56,   382,    -1,    48,   382,    -1,
      49,   382,    -1,   382,    38,   382,    -1,   382,    37,   382,
      -1,   382,    40,   382,    -1,   382,    39,   382,    -1,   382,
      41,   382,    -1,   382,    45,   382,    -1,   382,    42,   382,
      -1,   382,    44,   382,    -1,   382,    43,   382,    -1,   382,
      29,   382,    30,   382,    -1,   382,    29,    30,   382,    -1,
     217,   152,   205,    -1,   157,   152,   205,    -1,   217,   152,
     127,    -1,   215,    -1,    80,    -1,   441,    -1,   381,    -1,
     195,   436,   195,    -1,   196,   436,   196,    -1,   148,   436,
     149,    -1,   388,   386,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   388,     9,   382,   133,   382,    -1,   388,     9,
     382,    -1,   382,   133,   382,    -1,   382,    -1,    76,    -1,
      77,    -1,    78,    -1,   148,    87,   149,    -1,   148,   149,
      -1,    76,    -1,    77,    -1,    78,    -1,   204,    -1,    88,
      -1,    88,    50,   391,    -1,   389,    -1,   391,    -1,   204,
      -1,    48,   390,    -1,    49,   390,    -1,   135,   187,   393,
     188,    -1,    68,   393,   194,    -1,   177,   187,   396,   188,
      -1,   394,   386,    -1,    -1,   394,     9,   392,   133,   392,
      -1,   394,     9,   392,    -1,   392,   133,   392,    -1,   392,
      -1,   395,     9,   392,    -1,   392,    -1,   397,   386,    -1,
      -1,   397,     9,   350,   133,   392,    -1,   350,   133,   392,
      -1,   395,   386,    -1,    -1,   187,   398,   188,    -1,    -1,
     400,     9,   204,   399,    -1,   204,   399,    -1,    -1,   402,
     400,   386,    -1,    47,   401,    46,    -1,   403,    -1,    -1,
     131,    -1,   132,    -1,   204,    -1,   157,    -1,   190,   337,
     191,    -1,   406,    -1,   422,    -1,   204,    -1,   190,   337,
     191,    -1,   408,    -1,   422,    -1,    68,   425,   194,    -1,
     190,   337,   191,    -1,   416,   410,    -1,   187,   327,   188,
     410,    -1,   428,   410,    -1,   187,   327,   188,   410,    -1,
     187,   327,   188,   405,   407,    -1,   187,   338,   188,   405,
     407,    -1,   187,   327,   188,   405,   406,    -1,   187,   338,
     188,   405,   406,    -1,   422,    -1,   372,    -1,   420,    -1,
     421,    -1,   411,    -1,   413,    -1,   415,   405,   407,    -1,
     376,   152,   422,    -1,   417,   187,   282,   188,    -1,   418,
     187,   282,   188,    -1,   187,   415,   188,    -1,   372,    -1,
     420,    -1,   421,    -1,   411,    -1,   415,   405,   406,    -1,
     414,    -1,   417,   187,   282,   188,    -1,   187,   415,   188,
      -1,   422,    -1,   411,    -1,   372,    -1,   356,    -1,   381,
      -1,   187,   415,   188,    -1,   187,   338,   188,    -1,   418,
     187,   282,   188,    -1,   417,   187,   282,   188,    -1,   187,
     419,   188,    -1,   340,    -1,   343,    -1,   415,   405,   409,
     448,   187,   282,   188,    -1,   187,   327,   188,   405,   409,
     448,   187,   282,   188,    -1,   376,   152,   206,   448,   187,
     282,   188,    -1,   376,   152,   422,   187,   282,   188,    -1,
     376,   152,   190,   337,   191,   187,   282,   188,    -1,   423,
      -1,   426,   423,    -1,   423,    68,   425,   194,    -1,   423,
     190,   337,   191,    -1,   424,    -1,    81,    -1,   192,   190,
     337,   191,    -1,   337,    -1,    -1,   192,    -1,   426,   192,
      -1,   422,    -1,   412,    -1,   413,    -1,   427,   405,   407,
      -1,   375,   152,   422,    -1,   187,   415,   188,    -1,    -1,
     412,    -1,   414,    -1,   427,   405,   406,    -1,   187,   415,
     188,    -1,   429,     9,    -1,   429,     9,   415,    -1,   429,
       9,   134,   187,   429,   188,    -1,    -1,   415,    -1,   134,
     187,   429,   188,    -1,   431,   386,    -1,    -1,   431,     9,
     337,   133,   337,    -1,   431,     9,   337,    -1,   337,   133,
     337,    -1,   337,    -1,   431,     9,   337,   133,    36,   415,
      -1,   431,     9,    36,   415,    -1,   337,   133,    36,   415,
      -1,    36,   415,    -1,   433,   386,    -1,    -1,   433,     9,
     337,   133,   337,    -1,   433,     9,   337,    -1,   337,   133,
     337,    -1,   337,    -1,   435,   386,    -1,    -1,   435,     9,
     382,   133,   382,    -1,   435,     9,   382,    -1,   382,   133,
     382,    -1,   382,    -1,   436,   437,    -1,   436,    87,    -1,
     437,    -1,    87,   437,    -1,    81,    -1,    81,    68,   438,
     194,    -1,    81,   405,   204,    -1,   150,   337,   191,    -1,
     150,    80,    68,   337,   194,   191,    -1,   151,   415,   191,
      -1,   204,    -1,    82,    -1,    81,    -1,   124,   187,   329,
     188,    -1,   125,   187,   415,   188,    -1,   125,   187,   338,
     188,    -1,   125,   187,   419,   188,    -1,   125,   187,   418,
     188,    -1,   125,   187,   327,   188,    -1,     7,   337,    -1,
       6,   337,    -1,     5,   187,   337,   188,    -1,     4,   337,
      -1,     3,   337,    -1,   415,    -1,   440,     9,   415,    -1,
     376,   152,   205,    -1,   376,   152,   127,    -1,    -1,    99,
     462,    -1,   178,   447,    14,   462,   189,    -1,   403,   178,
     447,    14,   462,   189,    -1,   180,   447,   442,    14,   462,
     189,    -1,   403,   180,   447,   442,    14,   462,   189,    -1,
     206,    -1,   462,   206,    -1,   205,    -1,   462,   205,    -1,
     206,    -1,   206,   173,   454,   174,    -1,   204,    -1,   204,
     173,   454,   174,    -1,   173,   450,   174,    -1,    -1,   462,
      -1,   449,     9,   462,    -1,   449,   386,    -1,   449,     9,
     166,    -1,   450,    -1,   166,    -1,    -1,    -1,    30,   462,
      -1,    99,   462,    -1,   100,   462,    -1,   454,     9,   455,
     204,    -1,   455,   204,    -1,   454,     9,   455,   204,   453,
      -1,   455,   204,   453,    -1,    48,    -1,    49,    -1,    -1,
      88,   133,   462,    -1,    29,    88,   133,   462,    -1,   217,
     152,   204,   133,   462,    -1,   457,     9,   456,    -1,   456,
      -1,   457,   386,    -1,    -1,   177,   187,   458,   188,    -1,
     217,    -1,   204,   152,   461,    -1,   204,   448,    -1,    29,
     462,    -1,    57,   462,    -1,   217,    -1,   135,    -1,   136,
      -1,   459,    -1,   460,   152,   461,    -1,   135,   173,   462,
     174,    -1,   135,   173,   462,     9,   462,   174,    -1,   157,
      -1,   187,   108,   187,   451,   188,    30,   462,   188,    -1,
     187,   462,     9,   449,   386,   188,    -1,   462,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   727,   727,   727,   736,   738,   741,   742,   743,   744,
     745,   746,   747,   750,   752,   752,   754,   754,   756,   758,
     761,   764,   768,   772,   776,   781,   782,   783,   784,   785,
     786,   787,   791,   792,   793,   794,   795,   796,   797,   798,
     799,   800,   801,   802,   803,   804,   805,   806,   807,   808,
     809,   810,   811,   812,   813,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   824,   825,   826,   827,   828,
     829,   830,   831,   832,   833,   834,   835,   836,   837,   838,
     839,   840,   841,   842,   843,   844,   845,   846,   847,   848,
     849,   850,   851,   855,   859,   860,   864,   865,   870,   872,
     877,   882,   883,   884,   886,   891,   893,   898,   903,   905,
     907,   912,   913,   917,   918,   920,   924,   931,   938,   942,
     948,   950,   953,   954,   955,   956,   959,   960,   964,   969,
     969,   975,   975,   982,   981,   987,   987,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1010,  1008,  1017,  1015,  1022,  1030,  1024,  1034,
    1032,  1036,  1037,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
    1048,  1049,  1057,  1057,  1062,  1068,  1072,  1072,  1080,  1081,
    1085,  1086,  1090,  1096,  1094,  1109,  1106,  1122,  1119,  1136,
    1135,  1144,  1142,  1154,  1153,  1172,  1170,  1189,  1188,  1197,
    1195,  1206,  1206,  1213,  1212,  1224,  1222,  1235,  1236,  1240,
    1243,  1246,  1247,  1248,  1251,  1252,  1255,  1257,  1260,  1261,
    1264,  1265,  1268,  1269,  1273,  1274,  1279,  1280,  1283,  1284,
    1285,  1289,  1290,  1294,  1295,  1299,  1300,  1304,  1305,  1310,
    1311,  1316,  1317,  1318,  1319,  1322,  1325,  1327,  1330,  1331,
    1335,  1337,  1340,  1343,  1346,  1347,  1350,  1351,  1355,  1361,
    1367,  1374,  1376,  1381,  1386,  1392,  1396,  1400,  1404,  1409,
    1414,  1419,  1424,  1430,  1439,  1444,  1449,  1455,  1457,  1461,
    1465,  1470,  1474,  1477,  1480,  1484,  1488,  1492,  1496,  1501,
    1509,  1511,  1514,  1515,  1516,  1517,  1519,  1521,  1526,  1527,
    1530,  1531,  1532,  1536,  1537,  1539,  1540,  1544,  1546,  1549,
    1553,  1559,  1561,  1564,  1564,  1568,  1567,  1571,  1573,  1576,
    1579,  1577,  1593,  1589,  1603,  1605,  1607,  1609,  1611,  1613,
    1615,  1619,  1620,  1621,  1624,  1630,  1634,  1640,  1643,  1648,
    1650,  1655,  1660,  1664,  1665,  1669,  1670,  1672,  1674,  1680,
    1681,  1683,  1687,  1688,  1693,  1697,  1698,  1702,  1703,  1707,
    1709,  1715,  1720,  1721,  1723,  1727,  1728,  1729,  1730,  1734,
    1735,  1736,  1737,  1738,  1739,  1741,  1746,  1749,  1750,  1754,
    1755,  1759,  1760,  1763,  1764,  1767,  1768,  1771,  1772,  1776,
    1777,  1778,  1779,  1780,  1781,  1782,  1786,  1787,  1790,  1791,
    1792,  1795,  1797,  1799,  1800,  1803,  1805,  1809,  1811,  1815,
    1819,  1823,  1828,  1829,  1831,  1832,  1833,  1834,  1837,  1841,
    1842,  1846,  1847,  1851,  1852,  1853,  1854,  1858,  1862,  1867,
    1871,  1875,  1880,  1881,  1882,  1883,  1884,  1888,  1890,  1891,
    1892,  1895,  1896,  1897,  1898,  1899,  1900,  1901,  1902,  1903,
    1904,  1905,  1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,
    1914,  1915,  1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,
    1924,  1925,  1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1937,  1939,  1940,  1942,  1943,  1945,  1946,
    1947,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,
    1957,  1958,  1959,  1960,  1961,  1962,  1963,  1964,  1968,  1972,
    1977,  1976,  1991,  1989,  2007,  2006,  2025,  2024,  2043,  2042,
    2060,  2060,  2075,  2075,  2093,  2094,  2095,  2100,  2102,  2106,
    2110,  2116,  2120,  2126,  2128,  2132,  2134,  2138,  2142,  2143,
    2147,  2154,  2161,  2163,  2168,  2169,  2170,  2171,  2173,  2177,
    2178,  2179,  2180,  2184,  2190,  2199,  2212,  2213,  2216,  2219,
    2222,  2223,  2226,  2230,  2233,  2236,  2243,  2244,  2248,  2249,
    2251,  2255,  2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,
    2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,
    2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,
    2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,
    2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,
    2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,
    2314,  2315,  2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,
    2324,  2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,  2333,
    2334,  2338,  2343,  2344,  2348,  2349,  2350,  2351,  2353,  2357,
    2358,  2369,  2370,  2372,  2384,  2385,  2386,  2390,  2391,  2392,
    2396,  2397,  2398,  2401,  2403,  2407,  2408,  2409,  2410,  2412,
    2413,  2414,  2415,  2416,  2417,  2418,  2419,  2420,  2421,  2424,
    2429,  2430,  2431,  2433,  2434,  2436,  2437,  2438,  2439,  2441,
    2443,  2445,  2447,  2449,  2450,  2451,  2452,  2453,  2454,  2455,
    2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,  2465,
    2467,  2469,  2471,  2473,  2474,  2477,  2478,  2482,  2486,  2488,
    2492,  2495,  2498,  2504,  2505,  2506,  2507,  2508,  2509,  2510,
    2515,  2517,  2521,  2522,  2525,  2526,  2530,  2533,  2535,  2537,
    2541,  2542,  2543,  2544,  2547,  2551,  2552,  2553,  2554,  2558,
    2560,  2567,  2568,  2569,  2570,  2571,  2572,  2574,  2575,  2580,
    2582,  2585,  2588,  2590,  2592,  2595,  2597,  2601,  2603,  2606,
    2609,  2615,  2617,  2620,  2621,  2626,  2629,  2633,  2633,  2638,
    2641,  2642,  2646,  2647,  2651,  2652,  2653,  2657,  2659,  2667,
    2668,  2672,  2674,  2682,  2683,  2687,  2688,  2693,  2695,  2700,
    2711,  2725,  2737,  2752,  2753,  2754,  2755,  2756,  2757,  2758,
    2768,  2777,  2779,  2781,  2785,  2786,  2787,  2788,  2789,  2805,
    2806,  2808,  2817,  2818,  2819,  2820,  2821,  2822,  2823,  2824,
    2826,  2831,  2835,  2836,  2840,  2843,  2850,  2854,  2863,  2870,
    2872,  2878,  2880,  2881,  2885,  2886,  2893,  2894,  2899,  2900,
    2905,  2906,  2907,  2908,  2919,  2922,  2925,  2926,  2927,  2928,
    2939,  2943,  2944,  2945,  2947,  2948,  2949,  2953,  2955,  2958,
    2960,  2961,  2962,  2963,  2966,  2968,  2969,  2973,  2975,  2978,
    2980,  2981,  2982,  2986,  2988,  2991,  2994,  2996,  2998,  3002,
    3003,  3005,  3006,  3012,  3013,  3015,  3025,  3027,  3029,  3032,
    3033,  3034,  3038,  3039,  3040,  3041,  3042,  3043,  3044,  3045,
    3046,  3047,  3048,  3052,  3053,  3057,  3059,  3067,  3069,  3073,
    3077,  3082,  3086,  3094,  3095,  3099,  3100,  3106,  3107,  3116,
    3117,  3125,  3128,  3132,  3135,  3140,  3145,  3147,  3148,  3149,
    3153,  3154,  3158,  3159,  3162,  3165,  3167,  3171,  3177,  3178,
    3179,  3183,  3187,  3197,  3205,  3207,  3211,  3213,  3218,  3224,
    3227,  3232,  3240,  3243,  3246,  3247,  3250,  3253,  3254,  3259,
    3262,  3266,  3270,  3276,  3286,  3287
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
  "T_MINUS_EQUAL", "T_PLUS_EQUAL", "T_YIELD", "T_AWAIT", "'?'", "':'",
  "\"??\"", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'", "'^'", "'&'",
  "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL", "T_IS_EQUAL",
  "'<'", "'>'", "T_SPACESHIP", "T_IS_GREATER_OR_EQUAL",
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
  "yield_list_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "expr", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@30", "$@31", "lambda_expression", "$@32",
  "$@33", "$@34", "$@35", "$@36", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "lexical_var_list", "xhp_tag",
  "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes", "xhp_children",
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
  "hh_opt_return_type", "hh_constraint", "hh_typevar_list",
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
     273,   274,   275,   276,   277,   278,   279,   280,   281,    63,
      58,   282,   283,   284,   124,    94,    38,   285,   286,   287,
     288,    60,    62,   289,   290,   291,   292,   293,    43,    45,
      46,    42,    47,    37,    33,   294,   126,    64,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,    91,   305,
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
     416,   417,   418,   419,   420,   421,   422,    40,    41,    59,
     123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   197,   199,   198,   200,   200,   201,   201,   201,   201,
     201,   201,   201,   201,   202,   201,   203,   201,   201,   201,
     201,   201,   201,   201,   201,   204,   204,   204,   204,   204,
     204,   204,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   206,   206,   207,   207,   208,   208,
     209,   210,   210,   210,   210,   211,   211,   212,   213,   213,
     213,   214,   214,   215,   215,   215,   216,   217,   218,   218,
     219,   219,   220,   220,   220,   220,   221,   221,   221,   222,
     221,   223,   221,   224,   221,   225,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   226,   221,   227,   221,   221,   228,   221,   229,
     221,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   231,   230,   232,   232,   234,   233,   235,   235,
     236,   236,   237,   239,   238,   240,   238,   241,   238,   243,
     242,   244,   242,   246,   245,   247,   245,   248,   245,   249,
     245,   251,   250,   253,   252,   254,   252,   255,   255,   256,
     257,   258,   258,   258,   258,   258,   259,   259,   260,   260,
     261,   261,   262,   262,   263,   263,   264,   264,   265,   265,
     265,   266,   266,   267,   267,   268,   268,   269,   269,   270,
     270,   271,   271,   271,   271,   272,   272,   272,   273,   273,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     278,   278,   278,   278,   278,   278,   279,   279,   279,   279,
     279,   279,   279,   279,   280,   280,   280,   280,   280,   280,
     280,   280,   281,   281,   281,   281,   281,   281,   281,   281,
     282,   282,   283,   283,   283,   283,   283,   283,   284,   284,
     285,   285,   285,   286,   286,   286,   286,   287,   287,   288,
     289,   290,   290,   292,   291,   293,   291,   291,   291,   291,
     294,   291,   295,   291,   291,   291,   291,   291,   291,   291,
     291,   296,   296,   296,   297,   298,   298,   299,   299,   300,
     300,   301,   301,   302,   302,   303,   303,   303,   303,   303,
     303,   303,   304,   304,   305,   306,   306,   307,   307,   308,
     308,   309,   310,   310,   310,   311,   311,   311,   311,   312,
     312,   312,   312,   312,   312,   312,   313,   313,   313,   314,
     314,   315,   315,   316,   316,   317,   317,   318,   318,   319,
     319,   319,   319,   319,   319,   319,   320,   320,   321,   321,
     321,   322,   322,   322,   322,   323,   323,   324,   324,   325,
     325,   326,   327,   327,   327,   327,   327,   327,   328,   329,
     329,   330,   330,   331,   331,   331,   331,   332,   333,   334,
     335,   336,   337,   337,   337,   337,   337,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   338,   338,
     338,   338,   338,   338,   338,   338,   338,   338,   339,   339,
     341,   340,   342,   340,   344,   343,   345,   343,   346,   343,
     347,   343,   348,   343,   349,   349,   349,   350,   350,   351,
     351,   352,   352,   353,   353,   354,   354,   355,   356,   356,
     357,   358,   359,   359,   360,   360,   360,   360,   360,   361,
     361,   361,   361,   362,   363,   363,   364,   364,   365,   365,
     366,   366,   367,   368,   368,   369,   369,   369,   370,   370,
     370,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   371,   371,   371,   371,   371,   371,   371,   371,   371,
     371,   372,   373,   373,   374,   374,   374,   374,   374,   375,
     375,   376,   376,   376,   377,   377,   377,   378,   378,   378,
     379,   379,   379,   380,   380,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     383,   383,   383,   384,   384,   384,   384,   384,   384,   384,
     385,   385,   386,   386,   387,   387,   388,   388,   388,   388,
     389,   389,   389,   389,   389,   390,   390,   390,   390,   391,
     391,   392,   392,   392,   392,   392,   392,   392,   392,   393,
     393,   394,   394,   394,   394,   395,   395,   396,   396,   397,
     397,   398,   398,   399,   399,   400,   400,   402,   401,   403,
     404,   404,   405,   405,   406,   406,   406,   407,   407,   408,
     408,   409,   409,   410,   410,   411,   411,   412,   412,   413,
     413,   414,   414,   415,   415,   415,   415,   415,   415,   415,
     415,   415,   415,   415,   416,   416,   416,   416,   416,   416,
     416,   416,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   418,   419,   419,   420,   420,   421,   421,   421,   422,
     422,   423,   423,   423,   424,   424,   425,   425,   426,   426,
     427,   427,   427,   427,   427,   427,   428,   428,   428,   428,
     428,   429,   429,   429,   429,   429,   429,   430,   430,   431,
     431,   431,   431,   431,   431,   431,   431,   432,   432,   433,
     433,   433,   433,   434,   434,   435,   435,   435,   435,   436,
     436,   436,   436,   437,   437,   437,   437,   437,   437,   438,
     438,   438,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   440,   440,   441,   441,   442,   442,   443,
     443,   443,   443,   444,   444,   445,   445,   446,   446,   447,
     447,   448,   448,   449,   449,   450,   451,   451,   451,   451,
     452,   452,   453,   453,   454,   454,   454,   454,   455,   455,
     455,   456,   456,   456,   457,   457,   458,   458,   459,   460,
     461,   461,   462,   462,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   462,   463,   463
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
       1,     1,     1,     1,     1,     1,     2,     3,     3,     1,
       2,     1,     2,     3,     4,     3,     1,     2,     1,     2,
       2,     1,     3,     1,     3,     2,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     3,     5,     1,
       1,     1,     0,     9,     0,    10,     5,     0,    13,     0,
       5,     3,     3,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     0,     4,     9,     0,     0,     4,     2,     0,
       1,     0,     1,     0,     9,     0,    10,     0,    11,     0,
       9,     0,    10,     0,     8,     0,     9,     0,     7,     0,
       8,     0,     8,     0,     7,     0,     8,     1,     1,     1,
       1,     1,     2,     3,     3,     2,     2,     0,     2,     0,
       2,     0,     1,     3,     1,     3,     2,     0,     1,     2,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     5,     4,     0,     1,     1,
       4,     0,     5,     0,     2,     0,     3,     0,     7,     8,
       6,     2,     5,     6,     4,     0,     4,     5,     7,     6,
       6,     7,     9,     8,     6,     7,     5,     2,     4,     5,
       3,     0,     3,     4,     6,     5,     5,     6,     8,     7,
       2,     0,     1,     2,     2,     3,     4,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     2,
       3,     2,     0,     0,     4,     0,     5,     2,     2,     2,
       0,    10,     0,    11,     3,     3,     3,     4,     4,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     2,     1,     1,     4,     6,     1,     1,
       4,     1,     1,     3,     2,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     3,
       4,     3,     3,     3,     2,     2,     1,     1,     3,     3,
       1,     1,     0,     1,     2,     4,     3,     3,     6,     2,
       3,     6,     1,     1,     1,     1,     1,     6,     3,     4,
       6,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     3,     2,     1,     5,     0,
       0,    11,     0,    12,     0,     4,     0,     7,     0,     5,
       0,     3,     0,     6,     2,     2,     4,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       4,     4,     4,     4,     1,     1,     1,     1,     3,     3,
       4,     1,     2,     4,     2,     6,     0,     1,     4,     0,
       2,     0,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     3,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     4,     3,     4,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       3,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       2,     0,     1,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     2,     2,     4,     3,     4,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     3,     3,     2,     4,     2,     4,     5,
       5,     5,     5,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     4,     1,     0,     1,     2,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     3,     1,     1,     0,
       0,     2,     2,     2,     4,     2,     5,     3,     1,     1,
       0,     3,     4,     5,     3,     1,     2,     0,     4,     1,
       3,     2,     2,     2,     1,     1,     1,     1,     3,     4,
       6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   423,     0,   787,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   878,     0,
     866,   669,     0,   675,   676,   677,    25,   734,   854,   150,
     151,   678,     0,   131,     0,     0,     0,     0,    26,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,     0,
     389,   390,   391,   394,   393,   392,     0,     0,     0,     0,
     211,     0,     0,     0,   682,   684,   685,   679,   680,     0,
       0,     0,   686,   681,     0,   653,    27,    28,    29,    31,
      30,     0,   683,     0,     0,     0,     0,   687,   395,   522,
       0,   149,   121,   858,   670,     0,     0,     4,   111,   113,
     733,     0,   652,     0,     6,   181,     7,     9,     8,    10,
       0,     0,   387,   434,     0,     0,     0,     0,     0,     0,
       0,   432,   842,   843,   504,   503,   417,   507,     0,   416,
     814,   654,   661,     0,   736,   502,   386,   817,   818,   829,
     433,     0,     0,   436,   435,   815,   816,   813,   849,   853,
       0,   492,   735,    11,   394,   393,   392,     0,     0,    31,
       0,   111,   181,     0,   922,   433,   921,     0,   919,   918,
     506,     0,   424,   429,     0,     0,   474,   475,   476,   477,
     501,   499,   498,   497,   496,   495,   494,   493,   854,   678,
     656,     0,     0,   942,   835,   654,     0,   655,   456,     0,
     454,     0,   882,     0,   743,   415,   665,   201,     0,   942,
     414,   664,   659,     0,   674,   655,   861,   862,   868,   860,
     666,     0,     0,   668,   500,     0,     0,     0,     0,   420,
       0,   129,   422,     0,     0,   135,   137,     0,     0,   139,
       0,    72,    71,    66,    65,    57,    58,    49,    69,    80,
       0,    52,     0,    64,    56,    62,    82,    75,    74,    47,
      70,    89,    90,    48,    85,    45,    86,    46,    87,    44,
      91,    79,    83,    88,    76,    77,    51,    78,    81,    43,
      73,    59,    92,    67,    60,    50,    42,    41,    40,    39,
      38,    37,    61,    93,    95,    54,    35,    36,    63,   975,
     976,    55,   981,    34,    53,    84,     0,     0,   111,    94,
     933,   974,     0,   977,     0,     0,   141,     0,     0,   172,
       0,     0,     0,     0,     0,     0,   745,     0,    99,   101,
     300,     0,     0,   299,     0,   215,     0,   212,   305,     0,
       0,     0,     0,     0,   939,   197,   209,   874,   878,     0,
     903,     0,   689,     0,     0,     0,   901,     0,    16,     0,
     115,   189,   203,   210,   559,   534,     0,   927,   514,   516,
     518,   791,   423,   434,     0,     0,   432,   433,   435,     0,
       0,   671,     0,   672,     0,     0,     0,   171,     0,     0,
     117,   291,     0,    24,   180,     0,   208,   193,   207,   392,
     395,   181,   388,   164,   165,   166,   167,   168,   170,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   866,     0,   163,   857,
     857,   888,     0,     0,     0,     0,     0,     0,     0,     0,
     385,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,   453,   792,   793,     0,   857,
       0,   805,   291,   291,   857,     0,   859,   850,   874,     0,
     181,     0,     0,   143,     0,   789,   784,   743,     0,   434,
     432,     0,   886,     0,   539,   742,   877,   674,   434,   432,
     433,   117,     0,   291,   413,     0,   807,   667,     0,   121,
     251,     0,   521,     0,   146,     0,     0,   421,     0,     0,
       0,     0,     0,   138,   162,   140,   975,   976,   972,   973,
       0,   967,     0,     0,     0,     0,    68,    33,    55,    32,
     934,   169,   142,   121,     0,   159,   161,     0,     0,     0,
       0,   102,     0,   744,   100,    18,     0,    96,     0,   301,
       0,   144,   214,   213,     0,     0,   145,   923,     0,     0,
     434,   432,   433,   436,   435,     0,   960,   221,     0,   875,
       0,     0,   147,     0,     0,   688,   902,   734,     0,     0,
     900,   739,   899,   114,     5,    13,    14,     0,   219,     0,
       0,   527,     0,     0,     0,   743,     0,     0,   662,   657,
     528,     0,     0,     0,     0,   791,   121,     0,   745,   790,
     985,   412,   426,   488,   823,   841,   126,   120,   122,   123,
     124,   125,   386,     0,   505,   737,   738,   112,   743,     0,
     943,     0,     0,     0,   745,   292,     0,   510,   183,   217,
       0,   459,   461,   460,     0,     0,   491,   457,   458,   462,
     464,   463,   479,   478,   481,   480,   482,   484,   486,   485,
     483,   473,   472,   466,   467,   465,   468,   469,   471,   487,
     470,   856,     0,     0,   892,     0,   743,   926,     0,   925,
     942,   820,   849,   199,   191,   205,     0,   927,   195,   181,
       0,   427,   430,   438,   452,   451,   450,   449,   448,   447,
     446,   445,   444,   443,   442,   441,   795,     0,   794,   797,
     819,   801,   942,   798,     0,     0,     0,     0,     0,     0,
       0,     0,   920,   425,   782,   786,   742,   788,     0,   658,
       0,   881,     0,   880,   217,     0,   658,   865,   864,     0,
       0,   794,   797,   863,   798,   418,   253,   255,   121,   525,
     524,   419,     0,   121,   235,   130,   422,     0,     0,     0,
       0,     0,   247,   247,   136,     0,     0,     0,     0,   965,
     743,     0,   949,     0,     0,     0,     0,     0,   741,     0,
     653,     0,     0,   691,   652,   696,     0,   690,   119,   695,
     942,   978,     0,     0,     0,     0,    19,     0,    20,     0,
      97,     0,     0,     0,   108,   745,     0,   106,   101,    98,
     103,     0,   298,   306,   303,     0,     0,   912,   917,   914,
     913,   916,   915,    12,   958,   959,     0,     0,     0,     0,
     874,   871,     0,   538,   911,   910,   909,     0,   905,     0,
     906,   908,     0,     5,     0,     0,     0,   553,   554,   562,
     561,     0,   432,     0,   742,   533,   537,     0,     0,   928,
       0,   515,     0,     0,   950,   791,   277,   984,     0,     0,
     806,     0,   855,   742,   945,   941,   293,   294,   651,   744,
     290,     0,   791,     0,     0,   219,   512,   185,   490,     0,
     542,   543,     0,   540,   742,   887,     0,     0,   291,   221,
       0,   219,     0,     0,   217,     0,   866,   439,     0,     0,
     803,   804,   821,   822,   851,   852,     0,     0,     0,   770,
     750,   751,   752,   759,     0,     0,     0,   763,   761,   762,
     776,   743,     0,   784,   885,   884,     0,   219,     0,   808,
     673,     0,   257,     0,     0,   127,     0,     0,     0,     0,
       0,     0,     0,   227,   228,   239,     0,   121,   237,   156,
     247,     0,   247,     0,     0,   979,     0,     0,     0,   742,
     966,   968,   948,   743,   947,     0,   743,   717,   718,   715,
     716,   749,     0,   743,   741,     0,   536,     0,     0,   894,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   971,   173,
       0,   176,   160,     0,     0,   104,   109,   110,   102,   744,
     107,     0,   302,     0,   924,   148,   960,   940,   955,   220,
     222,   312,     0,     0,   872,     0,   904,     0,    17,     0,
     927,   218,   312,     0,     0,   658,   530,     0,   663,   929,
       0,   950,   519,     0,     0,   985,     0,   282,   280,   797,
     809,   942,   797,   810,   944,     0,     0,   295,   118,     0,
     791,   216,     0,   791,     0,   489,   891,   890,     0,   291,
       0,     0,     0,     0,     0,     0,   219,   187,   674,   796,
     291,     0,   755,   756,   757,   758,   764,   765,   774,     0,
     743,     0,   770,     0,   754,   778,   742,   781,   783,   785,
       0,   879,     0,   796,     0,     0,     0,     0,   254,   526,
     132,     0,   422,   227,   229,   874,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,     0,     0,   961,     0,
     964,   742,     0,     0,     0,   693,   742,   740,     0,   731,
       0,   743,     0,   697,   732,   730,   898,     0,   743,   700,
     702,   701,     0,     0,   698,   699,   703,   705,   704,   720,
     719,   722,   721,   723,   725,   727,   726,   724,   713,   712,
     707,   708,   706,   709,   710,   711,   714,   970,     0,   121,
       0,     0,   105,    21,   304,     0,     0,     0,   957,     0,
     386,   876,   874,   428,   431,   437,     0,    15,     0,   386,
     565,     0,     0,   567,   560,   563,     0,   558,     0,   931,
       0,   951,   523,     0,   283,     0,     0,   278,     0,   297,
     296,   950,     0,   312,     0,   791,     0,   291,     0,   847,
     312,   927,   312,   930,     0,     0,     0,   440,     0,     0,
     767,   742,   769,   760,     0,   753,     0,     0,   743,   775,
     883,   312,     0,   121,     0,   250,   236,     0,     0,     0,
     226,   152,   240,     0,     0,   243,     0,   248,   249,   121,
     242,   980,   962,     0,   946,     0,   983,   748,   747,   692,
       0,   742,   535,   694,     0,   541,   742,   893,   729,     0,
       0,     0,    22,    23,   954,   952,   953,   223,     0,     0,
       0,   393,   384,     0,     0,     0,   198,   311,   313,     0,
     383,     0,     0,     0,   927,   386,     0,   907,   308,   204,
     556,     0,     0,   529,   517,     0,   286,   276,     0,   279,
     285,   291,   509,   950,   386,   950,     0,   889,     0,   846,
     386,     0,   386,   932,   312,   791,   844,   773,   772,   766,
       0,   768,   742,   777,   386,   121,   256,   128,   133,   154,
     230,     0,   238,   244,   121,   246,   963,     0,     0,   532,
       0,   897,   896,   728,   121,   177,   956,     0,     0,     0,
     935,     0,     0,     0,   224,     0,   927,     0,   349,   345,
     351,   653,    31,     0,   339,     0,   344,   348,   361,     0,
     359,   364,     0,   363,     0,   362,     0,   181,   315,     0,
     317,     0,   318,   319,     0,     0,   873,     0,   557,   555,
     566,   564,   287,     0,     0,   274,   284,     0,     0,     0,
       0,   194,   509,   950,   848,   200,   308,   206,   386,     0,
       0,   780,     0,   202,   252,     0,     0,   121,   233,   153,
     245,   982,   746,     0,     0,     0,     0,     0,   411,     0,
     936,     0,   329,   333,   408,   409,   343,     0,     0,     0,
     324,   617,   616,   613,   615,   614,   634,   636,   635,   605,
     576,   577,   595,   611,   610,   572,   582,   583,   585,   584,
     604,   588,   586,   587,   589,   590,   591,   592,   593,   594,
     596,   597,   598,   599,   600,   601,   603,   602,   573,   574,
     575,   578,   579,   581,   619,   620,   629,   628,   627,   626,
     625,   624,   612,   631,   621,   622,   623,   606,   607,   608,
     609,   632,   633,   637,   639,   638,   640,   641,   618,   643,
     642,   645,   647,   646,   580,   650,   648,   649,   644,   630,
     571,   356,   568,     0,   325,   377,   378,   376,   369,     0,
     370,   326,   403,     0,     0,     0,     0,   407,     0,   181,
     190,   307,     0,     0,     0,   275,   289,   845,     0,   121,
     379,   121,   184,     0,     0,     0,   196,   950,   771,     0,
     121,   231,   134,   155,     0,   531,   895,   175,   327,   328,
     406,   225,     0,     0,   743,     0,   352,   340,     0,     0,
       0,   358,   360,     0,     0,   365,   372,   373,   371,     0,
       0,   314,   937,     0,     0,     0,   410,     0,   309,     0,
     288,     0,   551,   745,     0,     0,   121,   186,   192,     0,
     779,     0,     0,   157,   330,   111,     0,   331,   332,     0,
       0,   346,   742,   354,   350,   355,   569,   570,     0,   341,
     374,   375,   367,   368,   366,   404,   401,   960,   320,   316,
     405,     0,   310,   552,   744,     0,   511,   380,     0,   188,
       0,   234,     0,   179,     0,   386,     0,   353,   357,     0,
       0,   791,   322,     0,   549,   508,   513,   232,     0,     0,
     158,   337,     0,   385,   347,   402,   938,     0,   745,   397,
     791,   550,     0,   178,     0,     0,   336,   950,   791,   261,
     398,   399,   400,   985,   396,     0,     0,     0,   335,     0,
     397,     0,   950,     0,   334,   381,   121,   321,   985,     0,
     266,   264,     0,   121,     0,     0,   267,     0,     0,   262,
     323,     0,   382,     0,   270,   260,     0,   263,   269,   174,
     271,     0,     0,   258,   268,     0,   259,   273,   272
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   863,   604,   171,  1420,   700,
     335,   336,   337,   338,   825,   826,   827,   109,   110,   111,
     112,   113,   389,   637,   638,   526,   240,  1485,   532,  1401,
    1486,  1723,   814,   330,   553,  1683,  1042,  1219,  1740,   405,
     172,   639,   903,  1104,  1276,   117,   607,   920,   640,   659,
     924,   587,   919,   220,   507,   641,   608,   921,   407,   355,
     372,   120,   905,   866,   849,  1059,  1423,  1157,   973,  1632,
    1489,   775,   979,   531,   784,   981,  1309,   767,   962,   965,
    1146,  1747,  1748,   627,   628,   653,   654,   342,   343,   349,
    1457,  1611,  1612,  1230,  1347,  1446,  1605,  1731,  1750,  1642,
    1687,  1688,  1689,  1433,  1434,  1435,  1436,  1644,  1645,  1651,
    1699,  1439,  1440,  1444,  1598,  1599,  1600,  1622,  1777,  1348,
    1349,   173,   122,  1763,  1764,  1603,  1351,  1352,  1353,  1354,
     123,   233,   527,   528,   124,   125,   126,   127,   128,   129,
     130,   131,  1469,   132,   902,  1103,   133,   624,   625,   626,
     237,   381,   522,   614,   615,  1181,   616,  1182,   134,   135,
     136,   805,   137,   138,  1673,   139,   609,  1459,   610,  1073,
     871,  1247,  1244,  1591,  1592,   140,   141,   142,   223,   143,
     224,   234,   392,   514,   144,  1001,   809,   145,  1002,   894,
     564,  1003,   948,  1126,   949,  1128,  1129,  1130,   951,  1287,
    1288,   952,   745,   497,   184,   185,   642,   630,   478,  1089,
    1090,   731,   732,   890,   147,   226,   148,   149,   175,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   692,   160,
     230,   231,   590,   213,   214,   695,   696,  1187,  1188,   365,
     366,   857,   161,   578,   162,   623,   163,   322,  1613,  1663,
     356,   400,   648,   649,   995,  1084,  1228,   846,   847,   789,
     790,   791,   323,   324,   811,  1422,   888
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1449
static const yytype_int16 yypact[] =
{
   -1449,   181, -1449, -1449,  5142, 12708, 12708,   -14, 12708, 12708,
   12708, 10574, 12708, -1449, 12708, 12708, 12708, 12708, 12708, 12708,
   12708, 12708, 12708, 12708, 12708, 12708, 15333, 15333, 10768, 12708,
   15727,     7,    12, -1449, -1449, -1449, -1449, -1449,   199, -1449,
   -1449,   142, 12708, -1449,    12,    32,    57,   184, -1449,    12,
   10962,   736, 11156, -1449, 13458,  9604,   217, 12708,  1239,    34,
   -1449, -1449, -1449,   252,    44,    66,   186,   223,   226,   294,
   -1449,   736,   316,   341, -1449, -1449, -1449, -1449, -1449, 12708,
     654,   760, -1449, -1449,   736, -1449, -1449, -1449, -1449,   736,
   -1449,   736, -1449,   198,   347,   736,   736, -1449,   323, -1449,
   11350, -1449, -1449,   357,   553,   579,   579, -1449,   557,   458,
     396,   448, -1449,    82, -1449,   211, -1449, -1449, -1449, -1449,
    1085,   672, -1449, -1449,   449,   465,   485,   501,   506,   508,
    2727, -1449, -1449, -1449, -1449,   193, -1449,   641,   653, -1449,
     155,   475, -1449,   561,     1, -1449,  2571,   172, -1449, -1449,
    3107,   148,   536,   195, -1449,   154,   222,   588,   324, -1449,
      37, -1449,   676, -1449, -1449, -1449,   596,   590,   584, -1449,
   12708, -1449,   211,   672, 16192,  3688, 16192, 12708, 16192, 16192,
   13905,   606,  4466, 13905,   740,   736,   731,   731,   570,   731,
     731,   731,   731,   731,   731,   731,   731,   731, -1449, -1449,
   -1449,    63, 12708,   635, -1449, -1449,   659,   626,   453,   643,
     453, 15333, 14822,   638,   837, -1449,   596, -1449, 12708,   635,
   -1449,   698, -1449,   705,   677, -1449,   167, -1449, -1449, -1449,
     453,   148, 11544, -1449, -1449, 12708,  8440,   894,    88, 16192,
    9410, -1449, 12708, 12708,   736, -1449, -1449, 14065,   714, -1449,
   14111, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
    2821, -1449,  2821, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,    87,
      90,   584, -1449, -1449, -1449, -1449,   718,   610,    91, -1449,
   -1449,   754,   897, -1449,   756, 14378, -1449,   723, 14157, -1449,
      15, 14224,  1379,  1379,   736,   724,   912,   741, -1449,    55,
   -1449, 15050,   102, -1449,   806, -1449,   821, -1449,   935,   104,
   15333, 12708, 12708,   766,   784, -1449, -1449,  4107, 10768,   105,
      70,   462, -1449, 12902, 15333,   778, -1449,   736, -1449,   355,
     458, -1449, -1449, -1449, -1449, 15820,   945,   862, -1449, -1449,
   -1449,    65, 12708,   781,   782, 16192,   785,  2168,   786,  5336,
   12708,   511,   773,   871,   511,   507,   455, -1449,   736,  2821,
     789,  9798, 13458, -1449, -1449,   923, -1449, -1449, -1449, -1449,
   -1449,   211, -1449, -1449, -1449, -1449, -1449, -1449, -1449, 12708,
   12708, 12708, 11738, 12708, 12708, 12708, 12708, 12708, 12708, 12708,
   12708, 12708, 12708, 12708, 12708, 12708, 12708, 12708, 12708, 12708,
   12708, 12708, 12708, 12708, 12708, 12708, 15913, 12708, -1449, 12708,
   12708, 12708,  4537,   736,   736,   736,   736,   736,  1085,   872,
    1171,  4298, 12708, 12708, 12708, 12708, 12708, 12708, 12708, 12708,
   12708, 12708, 12708, 12708, -1449, -1449, -1449, -1449,   949, 12708,
   12708, -1449,  9798,  9798, 12708, 12708,   357,   175,  4107,   792,
     211, 11932, 14270, -1449, 12708, -1449,   794,   973,   834,   798,
     801, 13041,   453, 12126, -1449, 12320, -1449,   677,   805,   813,
    2658, -1449,    83,  9798, -1449,  1502, -1449, -1449, 14316, -1449,
   -1449,  9992, -1449, 12708, -1449,   913,  8634,   979,   817, 16075,
     997,    85,    62, -1449, -1449, -1449,   839, -1449, -1449, -1449,
    2821,   520,   826,  1010, 14957,   736, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449,   833, -1449, -1449,   835,   842,   836,
     845,    61,  1307,  1511, -1449, -1449,   736,   736, 12708,   453,
      34, -1449, -1449, -1449, 14957,   943, -1449,   453,    94,   107,
     849,   850,  2872,   379,   856,   858,   529,   924,   868,   453,
     123,   873, -1449,   619,   736, -1449, -1449,   991,  2522,   318,
   -1449, -1449, -1449,   458, -1449, -1449, -1449,  1033,   940,   899,
     348,   920, 12708,   357,   951,  1064,   900,   941, -1449,   175,
   -1449,  2821,  2821,  1082,   894,    65, -1449,   915,  1098, -1449,
    2821,    74, -1449,   469,   173, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449,  1260,  3032, -1449, -1449, -1449, -1449,  1105,   948,
   -1449, 15333, 12708,   931,  1114, 16192,  1111, -1449, -1449,   998,
     939, 11141, 16367, 13905, 12708, 16146, 16446, 16517,  9778, 10747,
   12104, 12491, 13175, 13175, 13175, 13175,  3426,  3426,  3426,  3426,
    3426,  1648,  1648,   990,   990,   990,   570,   570,   570, -1449,
     731, 16192,   934,   936, 15032,   946,  1123,    13, 12708,   183,
     635,    27,   175, -1449, -1449, -1449,  1120,   862, -1449,   211,
   15147, -1449, -1449, 13905, 13905, 13905, 13905, 13905, 13905, 13905,
   13905, 13905, 13905, 13905, 13905, 13905, -1449, 12708,   445,   307,
   -1449, -1449,   635,   463,   952,  3257,   954,   955,   956,  3339,
     124,   958, -1449, 16192,  3864, -1449,   736, -1449,    74,   267,
   15333, 16192, 15333, 15495,   998,    74,   453,   322,   999,   966,
   12708, -1449,   335, -1449, -1449, -1449,  8246,   569, -1449, -1449,
   16192, 16192,    12, -1449, -1449, -1449, 12708,  1059, 14840, 14957,
     736,  8828,   970,   971, -1449,   135,  1073,  1029,  1013, -1449,
    1157,   981,  2148,  2821, 14957, 14957, 14957, 14957, 14957,   980,
    1019,   985, 14957,   437,  1022, -1449,   986, -1449, 16282, -1449,
     476, -1449,  5530,  1038,   987,  1511, -1449,  1511, -1449,   736,
     736,  1511,  1511,   736, -1449,  1169,   989, -1449,    69, -1449,
   -1449,  3593, -1449, 16282,  1167, 15333,   994, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449,   139,   736,  1038,   996,
    4107, 15240,  1170, -1449, -1449, -1449, -1449,   993, -1449, 12708,
   -1449, -1449,  4754, -1449,  2821,  1038,  1000, -1449, -1449, -1449,
   -1449,  1175,  1004, 12708, 15820, -1449, -1449,  4537,  1007, -1449,
    2821, -1449,  1008,  5724,  1173,    60, -1449, -1449,   168,   949,
   -1449,  1502, -1449,  2821, -1449, -1449,   453, 16192, -1449, 10186,
   -1449, 14957,    96,  1012,  1038,   940, -1449, -1449, 16446, 12708,
   -1449, -1449, 12708, -1449, 12708, -1449,  3763,  1017,  9798,   924,
    1177,   940,  2821,  1191,   998,   736, 15913,   453, 10559,  1021,
   -1449, -1449,   283,  1023, -1449, -1449,  1201,  1135,  1135,  3864,
   -1449, -1449, -1449,  1166,  1031,    72,  1032, -1449, -1449, -1449,
   -1449,  1211,  1035,   794,   453,   453, 12514,   940,  1502, -1449,
   -1449, 10947,   652,    12,  9410, -1449,  5918,  1040,  6112,  1043,
   14840, 15333,  1037,  1094,   453, 16282,  1219, -1449, -1449, -1449,
   -1449,   648, -1449,    42,  2821, -1449,  1103,  2821,   736,   520,
   -1449, -1449, -1449,  1229, -1449,  1051,  1105,   800,   800,  1179,
    1179, 15600,  1046,  1249, 14957, 14646, 15820,  3166, 14512, 14957,
   14957, 14957, 14957, 14747, 14957, 14957, 14957, 14957, 14957, 14957,
   14957, 14957, 14957, 14957, 14957, 14957, 14957, 14957, 14957, 14957,
   14957, 14957, 14957, 14957, 14957, 14957, 14957,   736, -1449, -1449,
    1178, -1449, -1449,  1075,  1076, -1449, -1449, -1449,   253,  1307,
   -1449,  1079, -1449, 14957,   453, -1449,   529, -1449,   634,  1262,
   -1449, -1449,   127,  1086,   453, 10380, -1449,  2389, -1449,  4948,
     862,  1262, -1449,   403,   413, -1449, 16192,  1136,  1087, -1449,
    1089,  1173, -1449,  2821,   894,  2821,    54,  1261,  1195,   340,
   -1449,   635,   375, -1449, -1449, 15333, 12708, 16192, 16282,  1092,
      96, -1449,  1093,    96,  1097, 16446, 16192, 15541,  1113,  9798,
    1119,  1112,  2821,  1118,  1096,  2821,   940, -1449,   677,   490,
    9798, 12708, -1449, -1449, -1449, -1449, -1449, -1449,  1149,  1107,
    1300,  1222,  3864,  1164, -1449, 15820,  3864, -1449, -1449, -1449,
   15333, 16192,  1124, -1449,    12,  1285,  1241,  9410, -1449, -1449,
   -1449,  1134, 12708,  1094,   453,  4107, 14840,  1138, 14957,  6306,
     759,  1139, 12708,   144,    48, -1449,  1153,  2821, -1449,  1196,
   -1449,  3380,  1303,  1142, 14957, -1449, 14957, -1449,  1146, -1449,
    1202,  1331,  1154, -1449, -1449, -1449, 15644,  1150,  1334, 16325,
   16409,  3800, 14957, 16238, 16552, 10166, 11911, 12298, 14377, 14513,
   14513, 14513, 14513,  1725,  1725,  1725,  1725,  1725,  1670,  1670,
     800,   800,   800,  1179,  1179,  1179,  1179, -1449,  1156, -1449,
    1160,  1162, -1449, -1449, 16282,   736,  2821,  2821, -1449,  1038,
     137, -1449,  4107, -1449, -1449, 13905,  1161, -1449,  1163,   640,
   -1449,   326, 12708, -1449, -1449, -1449, 12708, -1449, 12708, -1449,
     894, -1449, -1449,   188,  1342,  1277, 12708, -1449,  1172,   453,
   16192,  1173,  1176, -1449,  1180,    96, 12708,  9798,  1181, -1449,
   -1449,   862, -1449, -1449,  1183,  1184,  1197, -1449,  1182,  3864,
   -1449,  3864, -1449, -1449,  1204, -1449,  1227,  1205,  1356, -1449,
     453, -1449,  1353, -1449,  1200, -1449, -1449,  1206,  1208,   130,
   -1449, -1449, 16282,  1215,  1216, -1449, 12887, -1449, -1449, -1449,
   -1449, -1449, -1449,  2821, -1449,  2821, -1449, 16282, 15702, -1449,
   14957, 15820, -1449, -1449, 14957, -1449, 14957, -1449, 16482, 14957,
    1210,  6500, -1449, -1449,   634, -1449, -1449, -1449,   608, 13597,
    1038,  1276, -1449,  1850,  1245,   685, -1449, -1449, -1449,   872,
    1986,   111,   112,  1221,   862,  1171,   131, -1449, -1449, -1449,
    1233, 11529, 12693, 16192, -1449,    76,  1394,  1332, 12708, -1449,
   16192,  9798,  1304,  1173,  1315,  1173,  1231, 16192,  1232, -1449,
    1568,  1236,  1622, -1449, -1449,    96, -1449, -1449,  1295, -1449,
    3864, -1449, 15820, -1449,  1880, -1449,  8246, -1449, -1449, -1449,
   -1449,  9022, -1449, -1449, -1449,  8246, -1449,  1242, 14957, 16282,
    1305, 16282, 15746, 16482, -1449, -1449, -1449,  1038,  1038,   736,
   -1449,  1426, 14646,    78, -1449, 13597,   862,  2215, -1449,  1268,
   -1449,   114,  1255,   115, -1449, 13904, -1449, -1449, -1449,   116,
   -1449, -1449,   905, -1449,  1257, -1449,  1366,   211, -1449, 13736,
   -1449, 13736, -1449, -1449,  1435,   872, -1449, 13180, -1449, -1449,
   -1449, -1449,  1437,  1372, 12708, -1449, 16192,  1266,  1269,  1265,
     632, -1449,  1304,  1173, -1449, -1449, -1449, -1449,  1936,  1272,
    3864, -1449,  1324, -1449,  8246,  9216,  9022, -1449, -1449, -1449,
    8246, -1449, 16282, 14957, 14957,  6694,  1275,  1284, -1449, 14957,
   -1449,  1038, -1449, -1449, -1449, -1449, -1449,  2821,  2786,  1850,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449,   147, -1449,  1245, -1449, -1449, -1449, -1449, -1449,    97,
     597, -1449,  1447,   117, 14378,  1366,  1451, -1449,  2821,   211,
   -1449, -1449,  1287,  1464, 12708, -1449, 16192, -1449,   134, -1449,
   -1449, -1449, -1449,  1291,   632, 13319, -1449,  1173, -1449,  3864,
   -1449, -1449, -1449, -1449,  6888, 16282, 16282, -1449, -1449, -1449,
   16282, -1449,   552,   153,  1473,  1293, -1449, -1449, 14957, 13904,
   13904,  1428, -1449,   905,   905,   688, -1449, -1449, -1449, 14957,
    1406, -1449,  1317,  1306,   119, 14957, -1449,   736, -1449, 14957,
   16192,  1414, -1449,  1490,  7082,  7276, -1449, -1449, -1449,   632,
   -1449,  7470,  1312,  1390, -1449,  1405,  1361, -1449, -1449,  1409,
    2821, -1449,  2786, -1449, -1449, 16282, -1449, -1449,  1341, -1449,
    1480, -1449, -1449, -1449, -1449, 16282,  1501,   529, -1449, -1449,
   16282,  1329, 16282, -1449,   446,  1336, -1449, -1449,  7664, -1449,
    1328, -1449,  1335,  1355,   736,  1171,  1354, -1449, -1449, 14957,
     158,   133, -1449,  1446, -1449, -1449, -1449, -1449,  1038,   987,
   -1449,  1364,   736,   708, -1449, 16282, -1449,  1345,  1527,   753,
     133, -1449,  1456, -1449,  1038,  1351, -1449,  1173,   151, -1449,
   -1449, -1449, -1449,  2821, -1449,  1357,  1362,   121, -1449,   645,
     753,   325,  1173,  1359, -1449, -1449, -1449, -1449,  2821,   287,
    1530,  1470,   645, -1449,  7858,   330,  1538,  1474, 12708, -1449,
   -1449,  8052, -1449,   303,  1540,  1475, 12708, -1449, 16192, -1449,
    1544,  1478, 12708, -1449, 16192, 12708, -1449, 16192, 16192
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1449, -1449, -1449,  -520, -1449, -1449, -1449,   484,   -11,   -33,
     505, -1449,  -253,  -496, -1449, -1449,   513,    -3,  1493, -1449,
    2564, -1449,  -451, -1449,    26, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449, -1449, -1449,  -179, -1449, -1449,  -141,
     113,    21, -1449, -1449, -1449, -1449, -1449, -1449,    25, -1449,
   -1449, -1449, -1449, -1449, -1449,    28, -1449, -1449,  1106,  1110,
    1121,   -84,  -617,  -828,   646,   702,  -185,   420,  -884, -1449,
      89, -1449, -1449, -1449, -1449,  -693,   271, -1449, -1449, -1449,
   -1449,  -172, -1449,  -579, -1449,  -424, -1449, -1449,  1009, -1449,
     109, -1449, -1449,  -989, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449,    73, -1449,   159, -1449, -1449, -1449,
   -1449, -1449,    -4, -1449,   243,  -813, -1449, -1448,  -191, -1449,
    -138,   152,  -119,  -178, -1449,   -10, -1449, -1449, -1449,   255,
     -15,     4,    30,  -701,   -67, -1449, -1449,    10, -1449, -1449,
      -5,   -36,   122, -1449, -1449, -1449, -1449, -1449, -1449, -1449,
   -1449, -1449,  -551,  -811, -1449, -1449, -1449, -1449, -1449,  1503,
   -1449, -1449, -1449, -1449, -1449,   526, -1449, -1449, -1449, -1449,
   -1449, -1449, -1449, -1449,  -769, -1449,  2005,     9, -1449,   182,
    -376, -1449, -1449,  -463,  3076,  3121, -1449, -1449,   600,   -25,
    -593, -1449, -1449,   662,   470,  -684,   473, -1449, -1449, -1449,
   -1449, -1449,   657, -1449, -1449, -1449,   106,  -857,  -132,  -407,
    -396, -1449,   717,  -109, -1449, -1449,    39,    46,   165, -1449,
   -1449,   929,   -12, -1449,  -334,   163,   118, -1449,  -301, -1449,
   -1449, -1449,  -448,  1254, -1449, -1449, -1449, -1449, -1449,   772,
     861, -1449, -1449, -1449,  -318,  -673, -1449,  1212,  -757, -1449,
     -69,  -181,    92,   815, -1449, -1025,   279,   -90,   562,   633,
   -1449, -1449, -1449, -1449,   586,  1095, -1036
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -970
static const yytype_int16 yytable[] =
{
     174,   176,   412,   178,   179,   180,   182,   183,   459,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   320,   373,   212,   215,   116,   376,   377,  1085,   118,
     114,   489,   119,   384,   923,   886,   236,   239,   511,   222,
     740,   618,   481,   319,   754,   247,   882,   250,   241,  1253,
     328,   408,   331,   245,   412,   339,  1250,   620,   736,   737,
     950,   900,   458,  1077,   386,   327,   824,   829,   766,   227,
     689,   729,   238,   881,   239,   969,   228,  1102,   369,   558,
     560,   370,   730,  1239,   862,   383,  1153,  1501,   388,   759,
     983,   402,   782,  1113,   780,   385,   -68,   523,   515,   -33,
     -32,   -68,   812,   835,   -33,   -32,  1653,    13,   762,   359,
     146,   570,    13,   575,   523,   340,   523,   115,   198,   763,
    1449,  1451,   516,  -342,  1509,  1593,  1660,   554,  1660,  1142,
    1501,  1654,   851,   851,   386,  1254,   851,   957,   593,   851,
     851,  1338,   479,    13,   984,  1162,  1163,   348,  1056,   693,
     498,  1162,  1163,  -655,   566,   383,   121,  1462,   388,  1133,
     819,  1648,  1690,   346,   198,   385,   500,  1056,   566,   150,
    1671,   347,   492,   177,  1307,   883,  1677,  1649,   734,  -663,
      13,     3,   509,   738,    13,   555,   -95,   499,  -836,   506,
     388,   208,   210,   229,   232,  1180,  1650,   385,    13,   235,
     -95,   476,   477,   508,  1086,   476,   477,  -520,   206,   206,
    -547,   567,   362,   385,   918,  1672,   479,   820,  -656,   242,
    1255,  1134,  -825,  -824,  1365,   398,   341,   518,   594,   486,
     518,  1719,  1050,  1165,   411,  -867,  1372,   239,   529,  1310,
    -827,  -831,  1463,   484,   243,  -744,  1339,   404,  -744,  1087,
    -281,  1340,   783,    60,    61,    62,   164,  1341,   409,  1342,
     540,  -544,   520,  -546,   480,   387,   525,  1502,  1503,  1366,
     660,   403,  1300,   781,  1374,   103,   -68,   524,   487,   -33,
     -32,  1380,   836,  1382,  -281,  1655,   490,  1160,  1275,  1164,
    -826,   571,   550,   576,   592,   837,  1343,  1344,   460,  1345,
    1450,  1452,  1394,  -342,  1510,  1594,  1661,  1116,  1709,   985,
    1774,   852,   936,  1057,   319,  1231,   581,   966,  1400,  1456,
     410,  -265,   968,  1099,  1286,  1046,  1047,  1691,  1346,   339,
     339,   561,  1746,  1308,  1088,   387,   658,   580,   480,  -744,
     584,   412,  -834,  1069,  -825,  -824,   239,   385,  1470,   741,
    1472,  -830,   819,   212,  1367,   374,   -94,  -867,   598,  -833,
    -837,  1779,  -827,  -831,   603,   485,  1793,   387,  1786,   320,
     -94,   244,   344,   350,  -662,  -828,   502,   182,  1360,   345,
    -835,   579,   483,   510,  1800,   643,   373,   706,   707,   408,
    -870,   319,   484,   206,   711,  1478,   655,  1238,   476,   477,
     868,   150,  1062,  -869,   378,   150,  1780,   329,  -811,   398,
     351,  1794,  -826,   352,   661,   662,   663,   665,   666,   667,
     668,   669,   670,   671,   672,   673,   674,   675,   676,   677,
     678,   679,   680,   681,   682,   683,   684,   685,   686,   687,
     688,   699,   690,  -812,   691,   691,   694,  -546,  1624,   476,
     477,  1297,  1289,  1787,  -838,   222,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   723,   724,   725,  1801,
    -840,   712,   747,  -830,   691,   735,  -657,   655,   655,   691,
     739,   353,  1733,   374,  1092,   227,   713,   629,   108,   743,
     319,  1781,   228,   619,  1110,  1093,  1795,  -828,   751,   889,
     753,   891,   115,   357,   459,   869,   569,   379,   655,   861,
    1410,   398,  -870,   380,   485,   577,   770,   582,   771,   917,
     870,  1262,   589,   206,  1264,  -869,  1159,  1734,   358,   599,
    -811,   769,   206,  1252,   375,   248,   360,  -548,   318,   206,
     618,   121,   600,   360,   605,   606,   206,   390,  -942,   786,
    1118,   929,   774,   824,   150,   354,   620,   617,   458,   828,
     828,  1240,  1043,   831,  1044,  -812,   483,   841,   925,   399,
     702,  1245,   709,   371,  1241,   354,   872,   844,   845,   354,
     354,  1482,  1421,  -942,   476,   477,  -942,   397,   360,  -942,
     875,   907,   360,  1242,   600,  1387,   702,  1388,  1381,    36,
     476,   477,  1679,  1246,   354,   363,   364,   385,   787,   229,
     399,   595,   363,   364,   398,   701,   889,   891,  -799,   702,
      48,  -658,   511,   958,   891,   446,  1656,  -942,  1037,  1038,
     702,    36,  -799,   702,   360,   401,  -802,   447,   413,   260,
     391,   733,   963,   964,  1338,  1657,   959,   897,  1658,   399,
    -802,   646,    48,   589,   414,  1277,  -838,   363,   364,   908,
     360,   363,   364,  -800,   701,   451,   394,   262,  1504,   496,
     206,   915,   618,   168,   415,   758,    84,  -800,   764,    86,
      87,  1454,    88,   169,    90,  1268,  1376,    13,   620,    36,
     416,   150,  1606,   916,  1607,   417,  1278,   418,    36,  1364,
     854,   855,   645,   363,   364,   168,  1481,  1299,    84,   449,
      48,    86,    87,   452,    88,   169,    90,  1702,   542,    48,
     108,   450,   928,   482,   108,  1144,  1145,  1771,   530,   363,
     364,   629,  1769,  1226,  1227,   360,  1703,  1417,  1418,  1704,
     367,   361,  1785,  1684,  -545,   536,   537,  1782,  -656,  1339,
    1161,  1162,  1163,  1505,  1340,   961,    60,    61,    62,   164,
    1341,   409,  1342,   168,    36,   990,    84,   312,  1331,    86,
      87,   239,    88,   169,    90,  -832,   967,   488,    86,    87,
      53,    88,   169,    90,  1356,    48,   495,   316,    60,    61,
      62,   164,   165,   409,   460,   493,  1628,   317,   447,  1343,
    1344,   618,  1345,   362,   363,   364,  1479,   978,   399,   549,
    1441,   501,   828,  -836,   828,    36,   896,   620,   828,   828,
    1048,  1620,  1621,   410,    60,    61,    62,   164,   165,   409,
     483,  1359,   504,   206,  1775,  1776,    48,   557,   559,    36,
    1700,  1701,  1396,  1378,    86,    87,   505,    88,   169,    90,
    -654,  1033,  1034,  1035,  1067,   410,  1117,   512,  1405,   360,
      48,  1304,  1162,  1163,   513,   600,   699,  1036,  1076,  1760,
    1761,  1762,  1442,   108,  1749,   927,   393,   395,   396,   115,
    1696,  1697,   647,   116,   993,   996,   318,   118,   114,   354,
     119,   410,   206,  1749,  1097,    86,    87,  1756,    88,   169,
      90,  1770,   521,   534,  1105,   541,  -969,  1106,   545,  1107,
    1258,   544,   551,   655,   562,   954,   367,   955,   121,    86,
      87,   563,    88,   169,    90,   115,  1137,   601,   363,   364,
     565,   150,   206,   572,   206,   222,   549,   354,   704,   354,
     354,   354,   354,   974,  1484,  1680,   150,  1467,   573,   574,
     368,  1141,   360,  1490,   585,   209,   209,   586,   600,   621,
     206,   622,   728,  1495,   121,   227,   644,  1147,   146,   631,
     632,  1173,   228,   633,   635,   115,  -116,   150,  1177,   657,
      53,   744,   746,   595,    36,   549,   748,   618,   523,   749,
    1148,   629,   619,   755,  1179,   702,   115,  1185,  1233,   761,
    1054,   756,    36,   620,   772,    48,   776,   702,   629,   702,
     108,   779,   540,   792,   121,   589,  1064,   206,    36,   793,
     813,   363,   364,    48,   834,   815,   817,   150,    36,   810,
     198,   816,   206,   206,   818,   121,  1634,   838,   839,    48,
    1078,   443,   444,   445,   842,   446,   828,   843,   150,    48,
     830,   647,   733,   848,   764,   850,   617,   447,   618,   859,
    1235,   853,  1595,   864,    86,    87,  1596,    88,   169,    90,
     865,   867,  -678,   874,   620,  1234,   702,   856,   858,   115,
    1715,   115,    86,    87,   873,    88,   169,    90,   876,   229,
     116,  1260,  1442,   877,   118,   114,   880,   119,    86,    87,
     884,    88,   169,    90,   655,  1282,   726,   885,    86,    87,
     657,    88,   169,    90,   893,   655,  1235,    36,   121,   898,
     121,   764,   895,   899,   619,   901,   906,   904,   910,   150,
     911,   150,   914,   150,   922,   974,  1154,   913,    48,   727,
     209,   103,   932,   933,   354,   906,   930,   239,  1292,   325,
     934,  -660,   206,   206,   960,  1759,  1322,  1306,   970,   980,
     982,   986,   987,  1327,    36,   988,   989,  1004,  1674,   991,
    1675,  1005,  1006,  1295,  1008,   146,  1009,  1041,  1049,  1681,
    1051,  1053,   115,  1055,  1065,    48,  1061,  1066,   617,  1074,
    1072,   168,  1075,  1081,    84,    85,  1079,    86,    87,  1100,
      88,   169,    90,  1083,  1109,  1115,   629,  1112,  1120,   629,
    -839,  1122,  1123,  1124,    36,  1121,  1131,  1455,  1132,  1135,
    1136,   121,   596,  1138,  1155,  1718,   602,  1156,   947,  1150,
     953,   412,  1152,  1158,   150,    48,  1167,  1361,  1171,  1172,
    1175,  1362,   406,  1363,    86,    87,  1036,    88,   169,    90,
     108,  1370,   596,   619,   602,   596,   602,   602,  1176,  1218,
    1259,  1377,   655,  1393,   976,   108,  1220,  1221,  1223,  1248,
     209,  1229,   115,  1232,   918,  1256,  1257,   206,  1249,   209,
    1261,   583,  1279,  1263,  1265,  1273,   209,    60,    61,    62,
     164,   165,   409,   209,    86,    87,   108,    88,   169,    90,
    1267,  1280,  1270,  1045,   647,  1290,  1604,  1269,  1272,  1281,
     943,   121,   150,  1285,  1291,  1293,  1294,   617,    36,  1338,
     589,   974,   206,  1296,   150,  1784,  1301,  1311,  1305,  1313,
    1316,  1058,  1791,  1315,  1319,  1320,  1355,   206,   206,    48,
    1321,  1325,  1323,  1326,  1330,  1355,   108,   332,   333,  1332,
    1498,  1333,  1357,  1358,   410,   538,  1368,   539,  1369,  1371,
    1390,   549,    13,  1466,  1373,  1392,   655,   108,  1375,  1379,
    1386,   629,  1383,   728,  1384,   761,    60,    61,    62,    63,
      64,   409,  1350,  1395,  1385,  1425,    36,    70,   453,  1397,
    1458,  1350,  1389,  1391,  1398,   334,  1399,   589,    86,    87,
    1414,    88,   169,    90,  1402,  1403,  1438,    48,  1464,   354,
    1453,  1500,   543,  1465,   206,   821,   822,   209,  1468,  1473,
    1474,  1125,  1125,   947,  1339,   455,  1476,  1488,  1480,  1340,
    1491,    60,    61,    62,   164,  1341,   409,  1342,  1493,   619,
    1499,  1507,   761,   410,   115,  1508,  1601,  1602,   108,  1608,
     108,  1614,   108,  1615,  1617,  1619,  1618,  1629,    36,  1616,
    1627,  1659,  1447,   823,  1638,  1665,    86,    87,  1667,    88,
     169,    90,  1169,  1639,  1343,  1344,  1668,  1345,  1669,    48,
    1355,  1676,  1692,   121,  1694,  1698,  1355,  1706,  1355,   549,
    1707,   629,   549,  1708,   650,  1713,   150,   325,   410,  1714,
    1355,  1721,  1722,   617,  -338,  1728,  1471,   460,  1725,   115,
     619,  1631,  1488,  1724,  1654,  1729,  1732,  1737,   115,   203,
     203,   810,  1738,   219,  1735,  1739,  1350,  1751,  1744,   204,
     204,  1754,  1350,  1757,  1350,   334,  1758,  1766,    86,    87,
    1768,    88,   169,    90,  1788,  1772,  1350,   219,   121,  1783,
    1773,  1789,  1796,   108,  1802,  1797,  1803,   121,  1805,  1806,
    1753,   150,  1222,   703,   708,  1111,   150,  1071,  1609,  1767,
     150,  1662,  1338,  1298,   617,  1633,   705,  1404,  1765,   832,
     209,    36,  1647,   198,  1355,  1625,  1506,  1742,  1445,  1652,
      36,  1790,  1778,   319,  1623,  1664,  1426,   115,  1711,  1243,
    1127,  1283,    48,   115,  1178,  1284,  1091,   994,   115,  1670,
    1139,    48,   591,  1416,   656,    13,   947,  1730,  1225,  1693,
     947,     0,  1170,  1217,   412,     0,  1338,     0,     0,     0,
    1350,   108,     0,     0,     0,   785,   121,     0,     0,   209,
       0,     0,   121,   108,     0,     0,     0,   121,     0,   150,
     150,   150,     0,     0,     0,   150,     0,     0,     0,   726,
     150,    86,    87,     0,    88,   169,    90,   823,     0,    13,
      86,    87,     0,    88,   169,    90,     0,  1339,     0,   209,
       0,   209,  1340,     0,    60,    61,    62,   164,  1341,   409,
    1342,     0,   760,     0,   103,     0,   440,   441,   442,   443,
     444,   445,     0,   446,   203,     0,     0,   209,     0,  1334,
       0,     0,     0,     0,   204,   447,   878,   879,  1030,  1031,
    1032,  1033,  1034,  1035,     0,   887,     0,  1343,  1344,     0,
    1345,  1339,     0,     0,     0,     0,  1340,  1036,    60,    61,
      62,   164,  1341,   409,  1342,     0,     0,   115,     0,     0,
       0,   410,     0,   219,     0,   219,     0,     0,     0,  1475,
       0,     0,     0,   947,   209,   947,  -970,  -970,  -970,  -970,
    -970,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,   209,
     209,  1343,  1344,  1798,  1345,     0,   121,   115,   115,     0,
       0,  1804,  1036,     0,   115,     0,     0,  1807,     0,   150,
    1808,     0,     0,     0,     0,   410,     0,     0,     0,     0,
     219,     0,     0,  1477,     0,   108,     0,     0,     0,     0,
       0,     0,     0,   318,     0,     0,   121,   121,     0,  1443,
       0,   115,     0,   121,   203,     0,     0,   629,     0,   150,
     150,     0,     0,   203,   204,     0,   150,     0,     0,     0,
     203,     0,     0,   204,     0,     0,   629,   203,     0,     0,
     204,     0,     0,     0,   629,     0,     0,   204,   219,     0,
     121,     0,     0,     0,   947,     0,     0,  1743,     0,  1427,
     108,     0,     0,   150,  1338,   108,     0,   650,   650,   108,
       0,     0,   219,     0,     0,   219,     0,   115,     0,   209,
     209,     0,     0,   354,   115,     0,   549,     0,     0,   318,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1590,
       0,     0,     0,     0,     0,     0,  1597,    13,     0,    36,
       0,     0,     0,   318,     0,   318,   121,     0,     0,   219,
    1338,   318,     0,   121,     0,     0,     0,     0,     0,   150,
      48,     0,     0,     0,     0,     0,   150,     0,     0,  1070,
       0,     0,     0,     0,   947,     0,     0,     0,   108,   108,
     108,     0,  1428,     0,   108,  1080,     0,     0,     0,   108,
       0,   203,     0,    13,     0,  1429,  1430,     0,  1094,  1339,
       0,   204,     0,     0,  1340,     0,    60,    61,    62,   164,
    1341,   409,  1342,   168,     0,     0,    84,  1431,     0,    86,
      87,     0,    88,  1432,    90,   260,     0,  1114,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,   205,   205,   219,   219,   221,     0,   803,     0,  1343,
    1344,     0,  1345,   262,     0,  1339,     0,     0,     0,     0,
    1340,     0,    60,    61,    62,   164,  1341,   409,  1342,     0,
       0,     0,     0,   410,     0,    36,     0,   803,     0,   209,
       0,  1483,     0,     0,     0,     0,     0,     0,     0,  1166,
       0,     0,  1168,     0,   209,   209,    48,     0,   549,     0,
       0,     0,     0,     0,  -385,  1343,  1344,     0,  1345,     0,
       0,     0,    60,    61,    62,   164,   165,   409,     0,   318,
       0,     0,     0,   947,   219,   219,     0,     0,   108,   410,
       0,   536,   537,   219,     0,     0,  1685,  1626,     0,     0,
       0,     0,     0,  1590,  1590,     0,     0,  1597,  1597,   168,
       0,     0,    84,   312,   203,    86,    87,     0,    88,   169,
      90,   354,     0,     0,   204,     0,     0,     0,   108,   108,
       0,   209,     0,   316,     0,   108,     0,     0,     0,   410,
       0,     0,     0,   317,     0,     0,     0,   260,  1251,     0,
     887,     0,   491,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,   108,   203,     0,   262,     0,  1271,  1741,     0,
    1274,     0,     0,   204,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1755,    36,     0,     0,
       0,     0,     0,   474,   475,     0,     0,     0,     0,     0,
       0,     0,     0,   203,     0,   203,     0,     0,    48,     0,
       0,     0,     0,   204,     0,   204,     0,     0,     0,     0,
       0,     0,  1312,     0,     0,     0,  1094,     0,   108,     0,
       0,   203,   803,     0,     0,   108,     0,     0,     0,     0,
       0,   204,     0,   536,   537,   219,   219,   803,   803,   803,
     803,   803,     0,     0,    36,   803,     0,     0,     0,   476,
     477,   168,     0,     0,    84,   312,   219,    86,    87,     0,
      88,   169,    90,     0,   992,    48,     0,     0,     0,     0,
       0,  1335,  1336,     0,     0,   316,     0,     0,   203,     0,
       0,     0,     0,     0,     0,   317,     0,  1428,   204,     0,
       0,   219,     0,   203,   203,     0,   205,     0,     0,     0,
    1429,  1430,     0,   204,   204,   205,   634,   219,   219,     0,
       0,     0,   205,     0,     0,     0,     0,   219,   168,   205,
       0,    84,    85,   219,    86,    87,     0,    88,  1432,    90,
     205,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,     0,   803,     0,     0,   219,     0,   419,
     420,   421,     0,     0,     0,     0,     0,     0,  1406,     0,
    1407,     0,     0,     0,     0,   219,     0,     0,   422,   219,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,  1448,     0,     0,     0,     0,
       0,   221,     0,     0,     0,     0,   447,     0,     0,     0,
       0,     0,     0,   203,   203,     0,     0,     0,     0,     0,
       0,     0,     0,   204,   204,     0,     0,   219,     0,     0,
     219,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,     0,     0,   803,     0,   219,
       0,     0,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
       0,     0,   419,   420,   421,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   803,     0,     0,   806,
       0,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   219,   446,   219,   806,
       0,     0,     0,  1236,     0,     0,     0,     0,   203,   447,
       0,     0,     0,     0,     0,     0,     0,     0,   204,     0,
       0,     0,  1643,     0,     0,   219,     0,     0,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
       0,     0,     0,   203,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,     0,     0,     0,     0,   203,   203,
       0,   803,     0,     0,     0,     0,   205,     0,   204,   204,
     219,     0,     0,     0,   219,     0,     0,   803,     0,   803,
       0,     0,   491,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   803,     0,    60,    61,    62,
      63,    64,   409,     0,     0,     0,     0,     0,    70,   453,
       0,     0,     0,  1666,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   860,     0,   205,     0,     0,     0,   219,
     219,     0,   219,   474,   475,   203,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   204,   455,   419,   420,   421,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,   457,     0,     0,   410,   205,   422,   205,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,   205,   806,  1726,     0,     0,     0,   476,
     477,     0,     0,     0,   447,     0,     0,     0,     0,   806,
     806,   806,   806,   806,     0,     0,   219,   806,   219,     0,
       0,     0,     0,   803,   219,     0,     0,   803,  1040,   803,
       0,     0,   803,     0,   321,     0,   321,     0,     0,     0,
       0,     0,   219,   219,     0,     0,   219,     0,     0,     0,
     205,     0,     0,   219,     0,     0,   757,     0,     0,     0,
     260,     0,     0,  1060,     0,   205,   205,     0,   887,     0,
       0,     0,    33,    34,    35,     0,     0,     0,     0,     0,
    1060,     0,     0,   887,   199,     0,     0,     0,   262,   205,
       0,   321,     0,     0,     0,   219,   491,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,     0,
      36,   803,     0,     0,     0,     0,   806,     0,     0,  1101,
     219,   219,     0,     0,     0,     0,   448,     0,   219,     0,
     219,    48,     0,    74,    75,    76,    77,    78,     0,     0,
       0,   221,     0,     0,   201,     0,     0,   474,   475,     0,
      82,    83,   219,     0,   219,     0,     0,     0,     0,     0,
     219,     0,     0,     0,    92,     0,   536,   537,     0,     0,
       0,     0,     0,   321,     0,     0,   321,     0,    97,     0,
       0,     0,     0,     0,   168,   205,   205,    84,   312,     0,
      86,    87,     0,    88,   169,    90,   803,   803,     0,     0,
       0,     0,   803,     0,   219,     0,     0,     0,   316,     0,
     219,     0,   219,   476,   477,     0,     0,     0,   317,   806,
       0,   205,     0,     0,   806,   806,   806,   806,   806,   806,
     806,   806,   806,   806,   806,   806,   806,   806,   806,   806,
     806,   806,   806,   806,   806,   806,   806,   806,   806,   806,
     806,   806,   419,   420,   421,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   806,     0,
     840,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,     0,   446,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   447,
     205,   219,   207,   207,   321,   788,   225,     0,   804,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
       0,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,     0,   219,     0,     0,   804,     0,
     205,   803,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,   803,     0,     0,     0,     0,     0,   803,     0,
     205,   205,   803,   806,     0,     0,     0,     0,     0,     0,
       0,     0,   474,   475,     0,     0,  1010,  1011,  1012,   806,
       0,   806,     0,   219,     0,   321,   321,     0,     0,     0,
       0,     0,     0,     0,   321,  1013,     0,   806,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
       0,     0,   803,   892,     0,     0,     0,     0,     0,     0,
       0,   219,     0,  1036,  1337,     0,     0,   205,   476,   477,
       0,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   419,   420,   421,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   422,   207,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,   806,   205,     0,     0,   806,
       0,   806,     0,     0,   806,     0,     0,     0,     0,     0,
       0,     0,     0,   804,     0,  1424,     0,     0,  1437,   419,
     420,   421,     0,     0,  1183,     0,   321,   321,   804,   804,
     804,   804,   804,     0,     0,     0,   804,     0,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,   447,     0,     0,   260,
       0,     0,     0,   806,     0,     0,     0,   207,     0,     0,
       0,     0,  1496,  1497,     0,     0,   207,     0,   321,     0,
       0,     0,  1437,   207,     0,     0,     0,   262,     0,     0,
     207,     0,     0,     0,   321,     0,     0,     0,   931,     0,
       0,   225,     0,     0,     0,     0,     0,   321,     0,    36,
       0,     0,     0,     0,     0,   804,     0,  -970,  -970,  -970,
    -970,  -970,   438,   439,   440,   441,   442,   443,   444,   445,
      48,   446,     0,     0,     0,     0,   321,     0,     0,     0,
       0,     0,     0,   447,     0,     0,     0,     0,   806,   806,
       0,     0,     0,     0,   806,     0,  1641,     0,     0,     0,
       0,     0,     0,     0,  1437,   536,   537,     0,     0,     0,
       0,     0,   225,     0,     0,     0,     0,     0,     0,     0,
     935,     0,     0,   168,     0,     0,    84,   312,     0,    86,
      87,     0,    88,   169,    90,     0,  1314,     0,   321,     0,
       0,   321,     0,   788,     0,     0,     0,   316,     0,     0,
       0,     0,     0,     0,   207,     0,     0,   317,   804,     0,
       0,     0,     0,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,     0,     0,   419,   420,   421,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,     0,     0,
     807,     0,   422,     0,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   321,   446,   321,
     807,     0,     0,   806,     0,     0,     0,     0,     0,     0,
     447,     0,     0,     0,   806,   808,     0,     0,     0,     0,
     806,     0,     0,     0,   806,     0,   321,     0,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   833,     0,     0,     0,     0,
       0,     0,   491,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,     0,     0,     0,     0,     0,
       0,     0,   804,     0,     0,     0,     0,   207,     0,     0,
       0,   321,     0,     0,   806,   321,     0,     0,   804,     0,
     804,     0,     0,  1752,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   474,   475,     0,   804,     0,     0,  1424,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   419,   420,   421,     0,     0,     0,     0,
       0,     0,     0,     0,  1052,     0,   207,     0,     0,     0,
     321,   321,   422,     0,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,     0,   446,   476,
     477,     0,     0,     0,     0,     0,   207,     0,   207,  1013,
     447,     0,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,   207,   807,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1036,     0,     0,
     807,   807,   807,   807,   807,     0,     0,   321,   807,   321,
       0,     0,     0,     0,   804,     0,     0,     0,   804,     0,
     804,     0,     0,   804,     0,     0,     0,     0,     0,     0,
     975,     0,     0,   321,     0,     0,     0,     0,     0,     0,
       0,   207,   937,   938,   321,   997,   998,   999,  1000,     0,
       0,     0,     0,  1007,     0,     0,   207,   207,     0,     0,
       0,     0,   939,     0,     0,     0,     0,     0,     0,     0,
     940,   941,   942,    36,     0,     0,     0,     0,     0,     0,
     225,     0,   943,     0,  1108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,   804,     0,     0,     0,     0,   807,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   944,
       0,     0,   225,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   945,   321,     0,   321,     0,     0,     0,     0,
       0,   321,  1098,    86,    87,     0,    88,   169,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   946,     0,     0,     0,     0,   207,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,   804,     0,
       0,     0,     0,   804,     0,     0,     0,     0,     0,     0,
       0,   321,     0,     0,     0,     0,     0,     0,     0,     0,
     807,     0,   225,     0,     0,   807,   807,   807,   807,   807,
     807,   807,   807,   807,   807,   807,   807,   807,   807,   807,
     807,   807,   807,   807,   807,   807,   807,   807,   807,   807,
     807,   807,   807,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   807,
    1186,  1189,  1190,  1191,  1193,  1194,  1195,  1196,  1197,  1198,
    1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,  1207,  1208,
    1209,  1210,  1211,  1212,  1213,  1214,  1215,  1216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,   321,     0,  1224,    28,     0,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,     0,   198,   321,
       0,     0,     0,     0,     0,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1686,    48,     0,     0,
       0,   225,   804,     0,     0,     0,   207,     0,     0,     0,
       0,     0,     0,   804,     0,     0,     0,     0,   200,   804,
       0,   207,   207,   804,   807,     0,     0,     0,     0,     0,
       0,   588,    73,     0,    74,    75,    76,    77,    78,     0,
     807,     0,   807,     0,   321,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,   807,    88,
     169,    90,     0,     0,     0,    92,     0,     0,     0,  1302,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,     0,   804,   202,  1317,     0,  1318,     0,   103,
       0,     5,     6,     7,     8,     9,     0,     0,   207,     0,
       0,    10,     0,  1328,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   382,    12,   321,     0,     0,
       0,     0,     0,     0,   710,     0,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   807,   225,    48,     0,
     807,     0,   807,     0,     0,   807,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,  1409,     0,     0,     0,  1411,    80,  1412,     0,     0,
    1413,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,   225,    93,
       0,     0,     0,     0,     0,    94,   419,   420,   421,     0,
      97,    98,    99,     0,   807,   100,     0,     0,     0,     0,
     103,   104,     0,   105,   106,   422,     0,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,     0,     0,     0,     0,     0,     0,     0,  1492,
       0,     0,     0,   447,     0,     0,     0,     0,     0,     0,
     251,   252,     0,   253,   254,     0,     0,   255,   256,   257,
     258,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,   807,
     807,     0,     0,     0,     0,   807,     0,     0,     0,     0,
       0,     0,     0,     0,  1646,     0,     0,     0,     0,     0,
       0,     0,   261,     0,     0,     0,     0,     0,     0,   494,
       0,     0,     0,     0,     0,     0,   263,   264,   265,   266,
     267,   268,   269,     0,  1635,  1636,    36,     0,   198,     0,
    1640,     0,     0,     0,     0,     0,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,    48,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,     0,     0,     0,   697,   305,   306,   307,     0,     0,
       0,   308,   546,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     548,     0,     0,     0,     0,     0,    86,    87,     0,    88,
     169,    90,   313,     0,   314,     0,     0,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   807,     0,     0,   698,     0,   103,
       0,     0,     0,     0,     0,   807,     0,     0,     0,     0,
       0,   807,     0,     0,     0,   807,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1727,  1695,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1705,    11,    12,     0,     0,     0,  1710,     0,     0,     0,
    1712,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,   807,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
    1745,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,    81,    82,    83,
      84,    85,     0,    86,    87,     0,    88,    89,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,    95,     0,    96,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1068,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,    81,    82,    83,    84,    85,     0,    86,    87,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,    95,     0,    96,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1237,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,    81,    82,    83,    84,    85,
       0,    86,    87,     0,    88,    89,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
      95,     0,    96,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,   636,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1039,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1082,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1149,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1151,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,  1303,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1415,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1637,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,  1682,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1716,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1717,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,  1720,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1736,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1792,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1799,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     519,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   164,
     165,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   773,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   164,   165,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   977,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   164,   165,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1487,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   164,   165,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1630,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   164,   165,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   164,
     165,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,   326,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,     0,   446,   651,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   447,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,   652,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   167,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,   768,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
       0,     0,  1095,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1036,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   164,   165,   166,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     167,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,  1096,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   382,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   164,
     165,   166,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   167,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   419,
     420,   421,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   447,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,   181,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
    1119,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,   419,   420,   421,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   422,     0,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   447,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   167,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,  1143,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,   246,   420,   421,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   447,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   164,   165,   166,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     167,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,   249,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   164,
     165,   166,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   167,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   419,
     420,   421,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   447,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
    1460,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,   517,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   664,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1035,     0,     0,     0,   710,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1036,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   167,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,     0,   446,
       0,     0,   750,     0,     0,     0,     0,     0,     0,     0,
       0,   447,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   164,   165,   166,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     167,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,     0,     0,     0,     0,   752,     0,     0,     0,
       0,     0,     0,     0,     0,  1036,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   164,
     165,   166,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   167,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   170,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
    1140,     0,     0,     0,     0,     0,     0,     0,   447,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,   419,   420,   421,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   422,     0,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,     0,   446,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     447,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,  1461,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,   419,   420,   421,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   422,  1307,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   447,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   597,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   167,    73,     0,    74,
      75,    76,    77,    78,   251,   252,     0,   253,   254,     0,
      80,   255,   256,   257,   258,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,   259,     0,
      92,     0,     0,    93,     0,     0,  1308,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,     0,     0,   103,   104,   261,   105,   106,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,   198,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,   304,   305,
     306,   307,     0,     0,     0,   308,   546,   547,     0,     0,
       0,     0,     0,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,   548,     0,     0,     0,     0,     0,
      86,    87,     0,    88,   169,    90,   313,   259,   314,   260,
       0,   315,  -970,  -970,  -970,  -970,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,   698,     0,   103,     0,   261,     0,   262,     0,     0,
       0,     0,   447,     0,     0,     0,     0,     0,     0,   263,
     264,   265,   266,   267,   268,   269,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
      48,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,     0,     0,     0,     0,   305,   306,
     307,     0,     0,     0,   308,   309,   310,     0,     0,     0,
       0,     0,   251,   252,     0,   253,   254,     0,     0,   255,
     256,   257,   258,   311,     0,     0,    84,   312,     0,    86,
      87,     0,    88,   169,    90,   313,   259,   314,   260,     0,
     315,     0,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   317,     0,     0,
       0,  1610,     0,     0,   261,     0,   262,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,   264,
     265,   266,   267,   268,   269,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,    48,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,     0,     0,     0,     0,   305,   306,   307,
       0,     0,     0,   308,   309,   310,     0,     0,     0,     0,
       0,   251,   252,     0,   253,   254,     0,     0,   255,   256,
     257,   258,   311,     0,     0,    84,   312,     0,    86,    87,
       0,    88,   169,    90,   313,   259,   314,   260,     0,   315,
       0,     0,     0,     0,     0,     0,   316,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   317,     0,     0,     0,
    1678,     0,     0,   261,     0,   262,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   263,   264,   265,
     266,   267,   268,   269,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,    48,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,     0,     0,     0,   304,   305,   306,   307,     0,
       0,     0,   308,   309,   310,     0,     0,     0,     0,     0,
     251,   252,     0,   253,   254,     0,     0,   255,   256,   257,
     258,   311,     0,     0,    84,   312,     0,    86,    87,     0,
      88,   169,    90,   313,   259,   314,   260,     0,   315,     0,
       0,     0,     0,     0,     0,   316,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   317,     0,     0,     0,     0,
       0,     0,   261,     0,   262,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   263,   264,   265,   266,
     267,   268,   269,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,    48,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,     0,     0,     0,     0,   305,   306,   307,     0,     0,
       0,   308,   309,   310,     0,     0,     0,     0,     0,   251,
     252,     0,   253,   254,     0,     0,   255,   256,   257,   258,
     311,     0,     0,    84,   312,     0,    86,    87,     0,    88,
     169,    90,   313,   259,   314,   260,     0,   315,     0,     0,
       0,     0,     0,     0,   316,  1419,     0,     0,     0,     0,
       0,     0,     0,     0,   317,     0,     0,     0,     0,     0,
       0,   261,     0,   262,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   264,   265,   266,   267,
     268,   269,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    48,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
       0,     0,     0,     0,   305,   306,   307,     0,     0,     0,
     308,   309,   310,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   311,
       0,     0,    84,   312,     0,    86,    87,     0,    88,   169,
      90,   313,     0,   314,     0,     0,   315,  1511,  1512,  1513,
    1514,  1515,     0,   316,  1516,  1517,  1518,  1519,     0,     0,
       0,     0,     0,   317,     0,     0,     0,     0,     0,     0,
       0,  1520,  1521,     0,   422,     0,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,  1522,
     446,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   447,  1523,  1524,  1525,  1526,  1527,  1528,  1529,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1530,  1531,  1532,  1533,  1534,  1535,  1536,
    1537,  1538,  1539,  1540,    48,  1541,  1542,  1543,  1544,  1545,
    1546,  1547,  1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,
    1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,
    1566,  1567,  1568,  1569,  1570,     0,     0,     0,  1571,  1572,
       0,  1573,  1574,  1575,  1576,  1577,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1578,  1579,  1580,
       0,     0,     0,    86,    87,     0,    88,   169,    90,  1581,
       0,  1582,  1583,     0,  1584,   419,   420,   421,     0,     0,
       0,  1585,  1586,     0,  1587,     0,  1588,  1589,     0,     0,
       0,     0,     0,     0,   422,     0,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,   419,   420,   421,     0,     0,     0,     0,     0,     0,
       0,     0,   447,     0,     0,     0,     0,     0,     0,     0,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,   419,   420,   421,
       0,     0,     0,     0,     0,     0,     0,     0,   447,     0,
       0,     0,     0,     0,     0,     0,   422,     0,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   419,   420,   421,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   422,   533,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,     0,   446,
     419,   420,   421,     0,     0,     0,     0,     0,     0,     0,
       0,   447,     0,     0,     0,     0,     0,     0,     0,   422,
     535,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,     0,   446,   419,   420,   421,     0,
       0,     0,     0,     0,     0,     0,     0,   447,     0,     0,
       0,     0,     0,     0,     0,   422,   552,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   251,   252,   447,   253,   254,     0,     0,   255,   256,
     257,   258,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   259,     0,     0,     0,     0,
       0,     0,     0,   556,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,     0,     0,   261,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1036,     0,     0,   263,   264,   265,
     266,   267,   268,   269,     0,     0,     0,    36,   742,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,    48,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,     0,     0,   765,   304,   305,   306,   307,     0,
       0,     0,   308,   546,   547,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,     0,     0,     0,     0,
       0,   548,     0,     0,     0,     0,     0,    86,    87,   259,
      88,   169,    90,   313,     0,   314,     0,     0,   315,     0,
    -970,  -970,  -970,  -970,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1036,   263,   264,   265,   266,   267,   268,   269,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,    48,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,     0,     0,     0,  1184,
     305,   306,   307,     0,     0,     0,   308,   546,   547,   251,
     252,     0,   253,   254,     0,     0,   255,   256,   257,   258,
       0,     0,     0,     0,     0,   548,     0,     0,     0,     0,
       0,    86,    87,   259,    88,   169,    90,   313,     0,   314,
       0,     0,   315,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   264,   265,   266,   267,
     268,   269,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    48,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
       0,     0,     0,     0,   305,   306,   307,  1192,     0,     0,
     308,   546,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   794,   795,     0,     0,   548,
       0,   796,     0,   797,     0,    86,    87,     0,    88,   169,
      90,   313,     0,   314,     0,   798,   315,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,     0,     0,     0,
       0,     0,   419,   420,   421,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   971,   446,     0,     0,
       0,     0,   799,     0,    74,    75,    76,    77,    78,   447,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,   800,     0,    86,    87,    28,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,   801,     0,     0,     0,   199,    97,
       0,     0,     0,     0,   802,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   972,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,   794,   795,     0,    92,     0,
       0,   796,     0,   797,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,   798,     0,   202,     0,     0,
       0,     0,   103,    33,    34,    35,    36,     0,     0,     0,
       0,     0,   419,   420,   421,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,     0,   446,     0,     0,
       0,     0,   799,     0,    74,    75,    76,    77,    78,   447,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,   800,     0,    86,    87,    28,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,   801,     0,     0,     0,   199,    97,
       0,     0,     0,     0,   802,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   912,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    28,     0,   926,    92,     0,
       0,     0,     0,    33,    34,    35,    36,     0,   198,     0,
       0,     0,    97,     0,     0,   199,     0,   202,     0,     0,
     568,     0,   103,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,    28,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,     0,     0,     0,     0,   199,    97,
       0,     0,     0,     0,   202,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1063,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,    28,    88,   169,    90,     0,     0,     0,    92,    33,
      34,    35,    36,     0,   198,     0,     0,     0,     0,     0,
       0,   199,    97,     0,     0,     0,     0,   202,     0,     0,
       0,     0,   103,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   201,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,    92,     0,     0,     0,   419,   420,   421,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     202,     0,     0,     0,   422,   103,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,   419,   420,   421,     0,     0,     0,     0,     0,     0,
       0,     0,   447,     0,     0,     0,     0,     0,     0,     0,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   447,     0,
    1010,  1011,  1012,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   956,  1013,
       0,     0,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,  1010,  1011,  1012,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1036,     0,     0,
       0,     0,     0,  1013,  1266,     0,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1036,  1010,  1011,  1012,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1013,     0,  1174,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1010,  1011,  1012,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1036,
       0,     0,     0,     0,     0,  1013,     0,  1324,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
       0,     0,     0,    33,    34,    35,    36,     0,   198,     0,
       0,     0,     0,  1036,     0,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,  1408,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,     0,   217,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,  1494,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,     0,     0,     0,     0,   611,    97,
       0,     0,     0,     0,   218,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,    33,
      34,    35,    36,     0,   198,     0,     0,     0,     0,     0,
       0,   199,    97,     0,     0,     0,     0,   612,     0,     0,
       0,     0,   613,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   201,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,    92,     0,     0,     0,   419,   420,   421,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     218,     0,     0,   777,   422,   103,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   447,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   419,   420,   421,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   778,   422,   909,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,   419,   420,   421,     0,     0,     0,     0,     0,
       0,     0,     0,   447,     0,     0,     0,     0,     0,     0,
       0,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,     0,   446,  1010,  1011,
    1012,     0,     0,     0,     0,     0,     0,     0,     0,   447,
       0,     0,     0,     0,     0,     0,     0,  1013,  1329,     0,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1010,  1011,  1012,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1036,     0,     0,     0,     0,
       0,  1013,     0,     0,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,  1011,  1012,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1036,
       0,     0,     0,     0,  1013,     0,     0,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,   421,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1036,     0,     0,     0,   422,     0,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,  1012,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,     0,     0,     0,  1013,     0,
       0,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1036,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   447,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1036,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,  1034,  1035,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1036
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,   146,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,    91,    28,    29,     4,    95,    96,   885,     4,
       4,   172,     4,   100,   707,   628,    32,    42,   219,    30,
     488,   375,   151,    54,   507,    50,   625,    52,    44,  1085,
      55,   120,    57,    49,   173,    58,  1081,   375,   482,   483,
     744,   654,   146,   874,   100,    55,   562,   563,   519,    30,
     446,   478,    42,   624,    79,   776,    30,   905,    81,   332,
     333,    84,   478,  1072,   604,   100,   970,     9,   100,   513,
     783,     9,    30,   921,     9,   100,     9,     9,   230,     9,
       9,    14,   553,     9,    14,    14,     9,    47,   515,    79,
       4,     9,    47,     9,     9,    81,     9,     4,    81,   515,
       9,     9,   231,     9,     9,     9,     9,   112,     9,   957,
       9,    34,     9,     9,   170,    81,     9,   754,    68,     9,
       9,     4,    68,    47,     9,   103,   104,    81,     9,   450,
      87,   103,   104,   152,    99,   170,     4,    81,   170,    87,
      99,    14,     9,   119,    81,   170,   202,     9,    99,     4,
      36,   127,   177,   187,    30,   626,  1624,    30,   479,   152,
      47,     0,   218,   484,    47,   170,   173,   202,   187,   214,
     202,    26,    27,    30,   187,  1006,    49,   202,    47,   187,
     187,   131,   132,   218,    36,   131,   132,     8,    26,    27,
      68,   156,   149,   218,   187,    81,    68,   156,   152,   187,
     166,   149,    68,    68,    36,   156,   192,   232,   360,   192,
     235,  1679,   825,   191,   121,    68,  1261,   242,   243,   191,
      68,    68,   166,    68,   187,   185,   109,    36,   188,    81,
     185,   114,   190,   116,   117,   118,   119,   120,   121,   122,
     173,    68,   236,    68,   190,   100,   240,   189,   190,    81,
     411,   189,  1156,   188,  1263,   192,   189,   189,   160,   189,
     189,  1270,   188,  1272,   188,   188,   173,   980,  1116,   982,
      68,   189,   325,   189,   189,   188,   159,   160,   146,   162,
     189,   189,  1291,   189,   189,   189,   189,   924,   189,   174,
     189,   188,   188,   174,   325,   188,   352,   768,   188,   188,
     183,   188,   773,   902,  1135,   821,   822,   174,   191,   332,
     333,   334,   174,   189,   166,   170,   405,   352,   190,   188,
     352,   460,   187,   863,   190,   190,   351,   352,  1373,   490,
    1375,    68,    99,   358,   166,   157,   173,   190,   363,   187,
     187,    36,   190,   190,   367,   190,    36,   202,    81,   402,
     187,   187,   120,   187,   152,    68,   211,   382,    52,   127,
     187,   351,   187,   218,    81,   390,   455,   456,   457,   458,
      68,   402,    68,   211,   461,  1384,   401,  1070,   131,   132,
      52,   236,   850,    68,    81,   240,    81,   190,    68,   156,
     187,    81,   190,   187,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   452,   447,    68,   449,   450,   451,    68,  1473,   131,
     132,  1152,  1136,   166,   187,   446,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   166,
     187,   461,   497,   190,   479,   480,   152,   482,   483,   484,
     485,   187,    36,   157,   891,   446,   491,   381,     4,   494,
     501,   166,   446,   375,   918,   891,   166,   190,   503,   631,
     505,   633,   389,   187,   642,   157,   341,   184,   513,   191,
    1321,   156,   190,   190,   190,   350,   521,   352,   523,   700,
     172,  1100,   357,   341,  1103,   190,   977,    81,   187,   364,
     190,   521,   350,  1084,   187,    51,    81,    68,    54,   357,
     874,   389,    87,    81,   189,   190,   364,   190,   152,    29,
     926,   732,   526,  1049,   389,    71,   874,   375,   642,   562,
     563,   158,   815,   568,   817,   190,   187,   188,   709,   173,
     452,   158,   459,    89,   171,    91,   612,    48,    49,    95,
      96,  1392,  1339,   187,   131,   132,   190,    30,    81,   152,
     615,   660,    81,   190,    87,  1279,   478,  1281,  1271,    79,
     131,   132,  1627,   190,   120,   150,   151,   612,    88,   446,
     173,   149,   150,   151,   156,   452,   748,   749,   173,   501,
     100,   152,   803,   755,   756,    55,    29,   190,   152,   810,
     512,    79,   187,   515,    81,   187,   173,    67,   189,    29,
      87,   478,    73,    74,     4,    48,   755,   652,    51,   173,
     187,   196,   100,   488,   189,  1118,   187,   150,   151,   664,
      81,   150,   151,   173,   501,   190,    87,    57,  1425,   185,
     488,   696,  1006,   153,   189,   512,   156,   187,   515,   159,
     160,  1354,   162,   163,   164,  1109,  1265,    47,  1006,    79,
     189,   526,  1449,   698,  1451,   189,  1120,   189,    79,  1250,
      81,    82,   195,   150,   151,   153,  1390,  1155,   156,    68,
     100,   159,   160,   152,   162,   163,   164,    29,   108,   100,
     236,    68,   727,   187,   240,    73,    74,  1763,   244,   150,
     151,   625,  1757,    99,   100,    81,    48,   129,   130,    51,
     156,    87,  1778,   191,    68,   135,   136,  1772,   152,   109,
     102,   103,   104,  1426,   114,   760,   116,   117,   118,   119,
     120,   121,   122,   153,    79,   790,   156,   157,  1219,   159,
     160,   776,   162,   163,   164,   187,   772,   187,   159,   160,
     108,   162,   163,   164,  1232,   100,    46,   177,   116,   117,
     118,   119,   120,   121,   642,   189,  1480,   187,    67,   159,
     160,  1135,   162,   149,   150,   151,  1385,   781,   173,   325,
     125,   152,   815,   187,   817,    79,   651,  1135,   821,   822,
     823,   189,   190,   183,   116,   117,   118,   119,   120,   121,
     187,   191,   194,   651,   189,   190,   100,   332,   333,    79,
    1653,  1654,  1293,  1267,   159,   160,     9,   162,   163,   164,
     152,    51,    52,    53,   859,   183,   925,   152,  1309,    81,
     100,   102,   103,   104,   187,    87,   877,    67,   873,   116,
     117,   118,   187,   389,  1731,   710,   104,   105,   106,   766,
    1649,  1650,   398,   862,   792,   793,   402,   862,   862,   405,
     862,   183,   710,  1750,   899,   159,   160,   189,   162,   163,
     164,  1758,     8,   189,   909,   187,   152,   912,   152,   914,
    1091,    14,   189,   918,   190,   750,   156,   752,   766,   159,
     160,     9,   162,   163,   164,   812,   951,   149,   150,   151,
     189,   766,   750,   127,   752,   926,   452,   453,   454,   455,
     456,   457,   458,   778,  1395,  1629,   781,  1371,   127,    14,
     190,   956,    81,  1404,   188,    26,    27,   173,    87,    14,
     778,    99,   478,  1414,   812,   926,   193,   963,   862,   188,
     188,   996,   926,   188,   188,   862,   187,   812,  1003,   187,
     108,   187,     9,   149,    79,   501,   188,  1321,     9,   188,
     964,   885,   874,   188,  1005,   877,   883,  1008,  1065,   515,
     835,   188,    79,  1321,    91,   100,   189,   889,   902,   891,
     526,    14,   173,   187,   862,   850,   851,   835,    79,     9,
     187,   150,   151,   100,    81,   190,   190,   862,    79,   545,
      81,   189,   850,   851,   189,   883,  1487,   188,   188,   100,
     877,    51,    52,    53,   188,    55,  1049,   189,   883,   100,
     566,   567,   889,   129,   891,   187,   874,    67,  1392,    68,
    1065,   188,   157,    30,   159,   160,   161,   162,   163,   164,
     130,   172,   152,     9,  1392,  1065,   958,   593,   594,   966,
    1673,   968,   159,   160,   133,   162,   163,   164,   188,   926,
    1069,  1096,   187,   152,  1069,  1069,    14,  1069,   159,   160,
     185,   162,   163,   164,  1109,  1130,   157,     9,   159,   160,
     187,   162,   163,   164,     9,  1120,  1121,    79,   966,   188,
     968,   958,   174,     9,  1006,    14,   187,   129,   194,   964,
     194,   966,     9,   968,    14,   970,   971,   191,   100,   190,
     211,   192,   188,   188,   660,   187,   194,  1152,  1144,    54,
     194,   152,   970,   971,   188,  1748,  1181,  1162,    99,   189,
     189,    88,   133,  1188,    79,   152,     9,   187,  1619,   188,
    1621,   152,   187,  1147,   152,  1069,   190,   190,     9,  1630,
     191,    14,  1069,   189,    14,   100,   190,   194,  1006,    14,
     190,   153,   188,   185,   156,   157,   189,   159,   160,   187,
     162,   163,   164,    30,   187,    14,  1100,    30,   187,  1103,
     187,    76,    77,    78,    79,    14,    50,  1355,   187,   187,
       9,  1069,   361,   188,   187,  1676,   365,   133,   744,   189,
     746,  1350,   189,    14,  1069,   100,   133,  1242,     9,   188,
     194,  1246,   157,  1248,   159,   160,    67,   162,   163,   164,
     766,  1256,   391,  1135,   393,   394,   395,   396,     9,    81,
    1095,  1266,  1267,  1288,   780,   781,   191,   191,   189,   133,
     341,     9,  1159,   187,   187,    14,    81,  1095,   189,   350,
     188,   352,   133,   190,   187,   189,   357,   116,   117,   118,
     119,   120,   121,   364,   159,   160,   812,   162,   163,   164,
     187,   194,   190,   819,   820,  1140,  1447,   188,   190,     9,
      88,  1159,  1147,   149,   190,    30,    75,  1135,    79,     4,
    1155,  1156,  1140,   189,  1159,  1776,   188,   174,   189,   133,
     188,   847,  1783,    30,   188,   133,  1230,  1155,  1156,   100,
       9,   191,   188,     9,   188,  1239,   862,   108,   109,   189,
    1419,   189,   191,   190,   183,   260,    14,   262,    81,   187,
     133,   877,    47,  1368,   188,     9,  1371,   883,   188,   188,
     188,  1265,   189,   889,   190,   891,   116,   117,   118,   119,
     120,   121,  1230,    30,   187,   109,    79,   127,   128,   189,
     157,  1239,   188,   188,   188,   156,   188,  1232,   159,   160,
     190,   162,   163,   164,   189,   189,   161,   100,    14,   925,
     189,  1422,   317,    81,  1232,   108,   109,   488,   114,   188,
     188,   937,   938,   939,   109,   165,   190,  1401,   133,   114,
     188,   116,   117,   118,   119,   120,   121,   122,   133,  1321,
      14,   173,   958,   183,  1331,   190,   189,    81,   964,    14,
     966,    14,   968,    81,   188,   190,   187,   133,    79,  1464,
     188,    14,  1349,   156,   189,    14,   159,   160,  1609,   162,
     163,   164,   988,   189,   159,   160,   189,   162,    14,   100,
    1374,   190,     9,  1331,   191,    57,  1380,    81,  1382,  1005,
     173,  1385,  1008,   187,   399,    81,  1331,   402,   183,     9,
    1394,   189,   112,  1321,    99,   164,   191,  1355,    99,  1396,
    1392,  1485,  1486,   152,    34,    14,   187,   189,  1405,    26,
      27,  1037,   187,    30,   188,   170,  1374,    81,   174,    26,
      27,   167,  1380,   188,  1382,   156,     9,    81,   159,   160,
     189,   162,   163,   164,    14,   188,  1394,    54,  1396,   190,
     188,    81,    14,  1069,    14,    81,    81,  1405,    14,    81,
    1739,  1396,  1049,   453,   458,   919,  1401,   865,  1455,  1754,
    1405,  1604,     4,  1153,  1392,  1486,   455,  1306,  1750,   570,
     651,    79,  1509,    81,  1478,  1476,  1427,  1725,  1345,  1593,
      79,  1782,  1770,  1604,  1472,  1605,  1341,  1484,  1667,  1073,
     938,  1131,   100,  1490,  1004,  1132,   889,   792,  1495,  1614,
     953,   100,   358,  1334,   402,    47,  1132,  1707,  1056,  1644,
    1136,    -1,   989,  1037,  1743,    -1,     4,    -1,    -1,    -1,
    1478,  1147,    -1,    -1,    -1,   540,  1484,    -1,    -1,   710,
      -1,    -1,  1490,  1159,    -1,    -1,    -1,  1495,    -1,  1484,
    1485,  1486,    -1,    -1,    -1,  1490,    -1,    -1,    -1,   157,
    1495,   159,   160,    -1,   162,   163,   164,   156,    -1,    47,
     159,   160,    -1,   162,   163,   164,    -1,   109,    -1,   750,
      -1,   752,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,   190,    -1,   192,    -1,    48,    49,    50,    51,
      52,    53,    -1,    55,   211,    -1,    -1,   778,    -1,  1225,
      -1,    -1,    -1,    -1,   211,    67,   621,   622,    48,    49,
      50,    51,    52,    53,    -1,   630,    -1,   159,   160,    -1,
     162,   109,    -1,    -1,    -1,    -1,   114,    67,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,  1634,    -1,    -1,
      -1,   183,    -1,   260,    -1,   262,    -1,    -1,    -1,   191,
      -1,    -1,    -1,  1279,   835,  1281,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,   850,
     851,   159,   160,  1788,   162,    -1,  1634,  1674,  1675,    -1,
      -1,  1796,    67,    -1,  1681,    -1,    -1,  1802,    -1,  1634,
    1805,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     317,    -1,    -1,   191,    -1,  1331,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1339,    -1,    -1,  1674,  1675,    -1,  1345,
      -1,  1718,    -1,  1681,   341,    -1,    -1,  1731,    -1,  1674,
    1675,    -1,    -1,   350,   341,    -1,  1681,    -1,    -1,    -1,
     357,    -1,    -1,   350,    -1,    -1,  1750,   364,    -1,    -1,
     357,    -1,    -1,    -1,  1758,    -1,    -1,   364,   375,    -1,
    1718,    -1,    -1,    -1,  1390,    -1,    -1,  1725,    -1,    29,
    1396,    -1,    -1,  1718,     4,  1401,    -1,   792,   793,  1405,
      -1,    -1,   399,    -1,    -1,   402,    -1,  1784,    -1,   970,
     971,    -1,    -1,  1419,  1791,    -1,  1422,    -1,    -1,  1425,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1435,
      -1,    -1,    -1,    -1,    -1,    -1,  1442,    47,    -1,    79,
      -1,    -1,    -1,  1449,    -1,  1451,  1784,    -1,    -1,   446,
       4,  1457,    -1,  1791,    -1,    -1,    -1,    -1,    -1,  1784,
     100,    -1,    -1,    -1,    -1,    -1,  1791,    -1,    -1,   864,
      -1,    -1,    -1,    -1,  1480,    -1,    -1,    -1,  1484,  1485,
    1486,    -1,   122,    -1,  1490,   880,    -1,    -1,    -1,  1495,
      -1,   488,    -1,    47,    -1,   135,   136,    -1,   893,   109,
      -1,   488,    -1,    -1,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    29,    -1,   922,    -1,    -1,
      -1,    -1,    -1,    -1,  1095,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    27,   540,   541,    30,    -1,   544,    -1,   159,
     160,    -1,   162,    57,    -1,   109,    -1,    -1,    -1,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   183,    -1,    79,    -1,   574,    -1,  1140,
      -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   984,
      -1,    -1,   987,    -1,  1155,  1156,   100,    -1,  1604,    -1,
      -1,    -1,    -1,    -1,   108,   159,   160,    -1,   162,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,  1625,
      -1,    -1,    -1,  1629,   621,   622,    -1,    -1,  1634,   183,
      -1,   135,   136,   630,    -1,    -1,  1642,   191,    -1,    -1,
      -1,    -1,    -1,  1649,  1650,    -1,    -1,  1653,  1654,   153,
      -1,    -1,   156,   157,   651,   159,   160,    -1,   162,   163,
     164,  1667,    -1,    -1,   651,    -1,    -1,    -1,  1674,  1675,
      -1,  1232,    -1,   177,    -1,  1681,    -1,    -1,    -1,   183,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    29,  1083,    -1,
    1085,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1718,   710,    -1,    57,    -1,  1112,  1724,    -1,
    1115,    -1,    -1,   710,    -1,    -1,   211,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1742,    79,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   750,    -1,   752,    -1,    -1,   100,    -1,
      -1,    -1,    -1,   750,    -1,   752,    -1,    -1,    -1,    -1,
      -1,    -1,  1167,    -1,    -1,    -1,  1171,    -1,  1784,    -1,
      -1,   778,   779,    -1,    -1,  1791,    -1,    -1,    -1,    -1,
      -1,   778,    -1,   135,   136,   792,   793,   794,   795,   796,
     797,   798,    -1,    -1,    79,   802,    -1,    -1,    -1,   131,
     132,   153,    -1,    -1,   156,   157,   813,   159,   160,    -1,
     162,   163,   164,    -1,   166,   100,    -1,    -1,    -1,    -1,
      -1,  1226,  1227,    -1,    -1,   177,    -1,    -1,   835,    -1,
      -1,    -1,    -1,    -1,    -1,   187,    -1,   122,   835,    -1,
      -1,   848,    -1,   850,   851,    -1,   341,    -1,    -1,    -1,
     135,   136,    -1,   850,   851,   350,   188,   864,   865,    -1,
      -1,    -1,   357,    -1,    -1,    -1,    -1,   874,   153,   364,
      -1,   156,   157,   880,   159,   160,    -1,   162,   163,   164,
     375,    -1,    -1,    -1,    -1,    -1,   893,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   901,    -1,    -1,   904,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1313,    -1,
    1315,    -1,    -1,    -1,    -1,   922,    -1,    -1,    29,   926,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,  1350,    -1,    -1,    -1,    -1,
      -1,   446,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,   970,   971,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   970,   971,    -1,    -1,   984,    -1,    -1,
     987,    -1,   989,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   488,    -1,    -1,    -1,  1004,    -1,  1006,
      -1,    -1,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1053,    -1,    -1,   544,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,  1083,    55,  1085,   574,
      -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,  1095,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1095,    -1,
      -1,    -1,  1507,    -1,    -1,  1112,    -1,    -1,  1115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1135,    -1,
      -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,  1155,  1156,
      -1,  1158,    -1,    -1,    -1,    -1,   651,    -1,  1155,  1156,
    1167,    -1,    -1,    -1,  1171,    -1,    -1,  1174,    -1,  1176,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1192,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,    -1,    -1,  1608,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   191,    -1,   710,    -1,    -1,    -1,  1226,
    1227,    -1,  1229,    65,    66,  1232,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,  1232,   165,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,
      -1,   180,    -1,    -1,   183,   750,    29,   752,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,   778,   779,  1690,    -1,    -1,    -1,   131,
     132,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,   794,
     795,   796,   797,   798,    -1,    -1,  1313,   802,  1315,    -1,
      -1,    -1,    -1,  1320,  1321,    -1,    -1,  1324,   813,  1326,
      -1,    -1,  1329,    -1,   260,    -1,   262,    -1,    -1,    -1,
      -1,    -1,  1339,  1340,    -1,    -1,  1343,    -1,    -1,    -1,
     835,    -1,    -1,  1350,    -1,    -1,   188,    -1,    -1,    -1,
      29,    -1,    -1,   848,    -1,   850,   851,    -1,  1763,    -1,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
     865,    -1,    -1,  1778,    88,    -1,    -1,    -1,    57,   874,
      -1,   317,    -1,    -1,    -1,  1392,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      79,  1408,    -1,    -1,    -1,    -1,   901,    -1,    -1,   904,
    1417,  1418,    -1,    -1,    -1,    -1,   189,    -1,  1425,    -1,
    1427,   100,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   926,    -1,    -1,   148,    -1,    -1,    65,    66,    -1,
     154,   155,  1449,    -1,  1451,    -1,    -1,    -1,    -1,    -1,
    1457,    -1,    -1,    -1,   168,    -1,   135,   136,    -1,    -1,
      -1,    -1,    -1,   399,    -1,    -1,   402,    -1,   182,    -1,
      -1,    -1,    -1,    -1,   153,   970,   971,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,  1493,  1494,    -1,    -1,
      -1,    -1,  1499,    -1,  1501,    -1,    -1,    -1,   177,    -1,
    1507,    -1,  1509,   131,   132,    -1,    -1,    -1,   187,  1004,
      -1,  1006,    -1,    -1,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
    1035,  1036,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1053,    -1,
     188,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
    1095,  1608,    26,    27,   540,   541,    30,    -1,   544,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1625,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,  1642,    -1,    -1,   574,    -1,
    1135,  1648,    -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,
      -1,    -1,  1659,    -1,    -1,    -1,    -1,    -1,  1665,    -1,
    1155,  1156,  1669,  1158,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    -1,    10,    11,    12,  1174,
      -1,  1176,    -1,  1690,    -1,   621,   622,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   630,    29,    -1,  1192,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,  1729,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1738,    -1,    67,  1229,    -1,    -1,  1232,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1754,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1763,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1778,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   211,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,  1320,  1321,    -1,    -1,  1324,
      -1,  1326,    -1,    -1,  1329,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   779,    -1,  1340,    -1,    -1,  1343,    10,
      11,    12,    -1,    -1,   188,    -1,   792,   793,   794,   795,
     796,   797,   798,    -1,    -1,    -1,   802,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,  1392,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    29,
      -1,    -1,    -1,  1408,    -1,    -1,    -1,   341,    -1,    -1,
      -1,    -1,  1417,  1418,    -1,    -1,   350,    -1,   864,    -1,
      -1,    -1,  1427,   357,    -1,    -1,    -1,    57,    -1,    -1,
     364,    -1,    -1,    -1,   880,    -1,    -1,    -1,   191,    -1,
      -1,   375,    -1,    -1,    -1,    -1,    -1,   893,    -1,    79,
      -1,    -1,    -1,    -1,    -1,   901,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
     100,    55,    -1,    -1,    -1,    -1,   922,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,  1493,  1494,
      -1,    -1,    -1,    -1,  1499,    -1,  1501,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1509,   135,   136,    -1,    -1,    -1,
      -1,    -1,   446,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     191,    -1,    -1,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,   166,    -1,   984,    -1,
      -1,   987,    -1,   989,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,    -1,   488,    -1,    -1,   187,  1004,    -1,
      -1,    -1,    -1,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1036,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1053,    -1,    -1,
     544,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,  1083,    55,  1085,
     574,    -1,    -1,  1648,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,  1659,   544,    -1,    -1,    -1,    -1,
    1665,    -1,    -1,    -1,  1669,    -1,  1112,    -1,    -1,  1115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   574,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1158,    -1,    -1,    -1,    -1,   651,    -1,    -1,
      -1,  1167,    -1,    -1,  1729,  1171,    -1,    -1,  1174,    -1,
    1176,    -1,    -1,  1738,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,  1192,    -1,    -1,  1754,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   191,    -1,   710,    -1,    -1,    -1,
    1226,  1227,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,   131,
     132,    -1,    -1,    -1,    -1,    -1,   750,    -1,   752,    29,
      67,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,   778,   779,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
     794,   795,   796,   797,   798,    -1,    -1,  1313,   802,  1315,
      -1,    -1,    -1,    -1,  1320,    -1,    -1,    -1,  1324,    -1,
    1326,    -1,    -1,  1329,    -1,    -1,    -1,    -1,    -1,    -1,
     779,    -1,    -1,  1339,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   835,    48,    49,  1350,   794,   795,   796,   797,    -1,
      -1,    -1,    -1,   802,    -1,    -1,   850,   851,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
     874,    -1,    88,    -1,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1408,    -1,    -1,    -1,    -1,   901,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1425,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
      -1,    -1,   926,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,  1449,    -1,  1451,    -1,    -1,    -1,    -1,
      -1,  1457,   901,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   970,   971,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1493,  1494,    -1,
      -1,    -1,    -1,  1499,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1507,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1004,    -1,  1006,    -1,    -1,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1053,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1095,  1608,    -1,  1053,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,  1625,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1642,   100,    -1,    -1,
      -1,  1135,  1648,    -1,    -1,    -1,  1140,    -1,    -1,    -1,
      -1,    -1,    -1,  1659,    -1,    -1,    -1,    -1,   121,  1665,
      -1,  1155,  1156,  1669,  1158,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
    1174,    -1,  1176,    -1,  1690,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,  1192,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,    -1,  1158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    -1,  1729,   187,  1174,    -1,  1176,    -1,   192,
      -1,     3,     4,     5,     6,     7,    -1,    -1,  1232,    -1,
      -1,    13,    -1,  1192,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,  1763,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1778,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1320,  1321,   100,    -1,
    1324,    -1,  1326,    -1,    -1,  1329,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,  1320,    -1,    -1,    -1,  1324,   148,  1326,    -1,    -1,
    1329,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,  1392,   171,
      -1,    -1,    -1,    -1,    -1,   177,    10,    11,    12,    -1,
     182,   183,   184,    -1,  1408,   187,    -1,    -1,    -1,    -1,
     192,   193,    -1,   195,   196,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1408,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,  1493,
    1494,    -1,    -1,    -1,    -1,  1499,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1508,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    74,    75,    -1,  1493,  1494,    79,    -1,    81,    -1,
    1499,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,    -1,
      -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,    -1,   159,   160,    -1,   162,
     163,   164,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1648,    -1,    -1,   190,    -1,   192,
      -1,    -1,    -1,    -1,    -1,  1659,    -1,    -1,    -1,    -1,
      -1,  1665,    -1,    -1,    -1,  1669,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,  1692,  1648,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1659,    27,    28,    -1,    -1,    -1,  1665,    -1,    -1,    -1,
    1669,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    49,    -1,  1729,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
    1729,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,   191,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
      -1,    -1,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,   190,   191,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    -1,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,   190,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,   110,   111,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,   190,   191,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,
     110,   111,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
     190,   191,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   110,   111,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,   191,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
      -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,   111,
      -1,   113,    -1,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,   190,   191,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,
     108,    -1,   110,   111,    -1,   113,    -1,   115,   116,   117,
     118,   119,   120,   121,    -1,   123,   124,   125,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,   190,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    98,    -1,   100,   101,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,   110,   111,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,
     110,   111,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
     190,   191,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   110,   111,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,   191,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    96,    97,    -1,    -1,   100,   101,
      -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,   111,
      -1,   113,    -1,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,   190,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,
     108,    -1,   110,   111,    -1,   113,    -1,   115,   116,   117,
     118,   119,   120,   121,    -1,   123,   124,   125,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,   190,   191,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,   110,   111,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,   190,   191,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,
     110,   111,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
     190,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   110,   111,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,   191,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
      -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,   111,
      -1,   113,    -1,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,   190,   191,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,
     108,    -1,   110,   111,    -1,   113,    -1,   115,   116,   117,
     118,   119,   120,   121,    -1,   123,   124,   125,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,   190,   191,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,   110,   111,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,
     110,   111,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
     190,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   110,   111,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
      -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,   111,
      -1,   113,    -1,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,   190,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,
     108,    -1,   110,   111,    -1,   113,    -1,   115,   116,   117,
     118,   119,   120,   121,    -1,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,   190,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,
      -1,   105,   106,   107,   108,    -1,   110,   111,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,
     110,   111,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
     190,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,    -1,   190,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    10,
      11,    12,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    67,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,   105,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     191,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,    10,    11,    12,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    67,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,   191,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,   189,    11,    12,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    67,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    10,
      11,    12,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    67,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     191,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,   188,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,    10,    11,    12,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      67,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
      -1,    -1,    -1,    -1,   191,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,    10,    11,    12,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    67,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,
     138,   139,   140,   141,     3,     4,    -1,     6,     7,    -1,
     148,    10,    11,    12,    13,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    27,    -1,
     168,    -1,    -1,   171,    -1,    -1,   189,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,    55,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   153,    -1,    -1,    -1,    -1,    -1,
     159,   160,    -1,   162,   163,   164,   165,    27,   167,    29,
      -1,   170,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,   190,    -1,   192,    -1,    55,    -1,    57,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,    -1,    -1,    -1,   128,   129,
     130,    -1,    -1,    -1,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    27,   167,    29,    -1,
     170,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,   191,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,    -1,   128,   129,   130,
      -1,    -1,    -1,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   153,    -1,    -1,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    27,   167,    29,    -1,   170,
      -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
     191,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    27,   167,    29,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,    -1,   128,   129,   130,    -1,    -1,
      -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,   165,    27,   167,    29,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,   177,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      -1,    -1,    -1,    -1,   128,   129,   130,    -1,    -1,    -1,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,   167,    -1,    -1,   170,     3,     4,     5,
       6,     7,    -1,   177,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    55,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,   154,   155,
      -1,    -1,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,    10,    11,    12,    -1,    -1,
      -1,   177,   178,    -1,   180,    -1,   182,   183,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   189,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     189,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   189,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    67,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   189,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,   188,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,   188,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,    -1,    -1,    -1,   159,   160,    27,
     162,   163,   164,   165,    -1,   167,    -1,    -1,   170,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,   127,
     128,   129,   130,    -1,    -1,    -1,   134,   135,   136,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,
      -1,   159,   160,    27,   162,   163,   164,   165,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      -1,    -1,    -1,    -1,   128,   129,   130,    30,    -1,    -1,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,   153,
      -1,    54,    -1,    56,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,   167,    -1,    68,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    36,    55,    -1,    -1,
      -1,    -1,   135,    -1,   137,   138,   139,   140,   141,    67,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    68,   162,
     163,   164,    -1,    -1,    -1,   168,    76,    77,    78,    79,
      -1,    81,    -1,    -1,   177,    -1,    -1,    -1,    88,   182,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    48,    49,    -1,   168,    -1,
      -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    68,    -1,   187,    -1,    -1,
      -1,    -1,   192,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,   135,    -1,   137,   138,   139,   140,   141,    67,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    68,   162,
     163,   164,    -1,    -1,    -1,   168,    76,    77,    78,    79,
      -1,    81,    -1,    -1,   177,    -1,    -1,    -1,    88,   182,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    68,    -1,    70,   168,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,   182,    -1,    -1,    88,    -1,   187,    -1,    -1,
     190,    -1,   192,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    68,   162,
     163,   164,    -1,    -1,    -1,   168,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,   182,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    68,   162,   163,   164,    -1,    -1,    -1,   168,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,   182,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    29,   192,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    29,   133,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,   133,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    29,    -1,   133,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    -1,    67,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,   133,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,   182,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,   182,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    28,    29,   192,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    12,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    29,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   198,   199,     0,   200,     3,     4,     5,     6,     7,
      13,    27,    28,    47,    48,    49,    54,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    68,    69,
      70,    71,    72,    76,    77,    78,    79,    80,    81,    83,
      84,    88,    89,    90,    91,    93,    95,    97,   100,   101,
     105,   106,   107,   108,   109,   110,   111,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   123,   124,   125,   126,
     127,   128,   134,   135,   137,   138,   139,   140,   141,   145,
     148,   153,   154,   155,   156,   157,   159,   160,   162,   163,
     164,   165,   168,   171,   177,   178,   180,   182,   183,   184,
     187,   189,   190,   192,   193,   195,   196,   201,   204,   214,
     215,   216,   217,   218,   221,   237,   238,   242,   245,   252,
     258,   318,   319,   327,   331,   332,   333,   334,   335,   336,
     337,   338,   340,   343,   355,   356,   357,   359,   360,   362,
     372,   373,   374,   376,   381,   384,   403,   411,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     426,   439,   441,   443,   119,   120,   121,   134,   153,   163,
     187,   204,   237,   318,   337,   415,   337,   187,   337,   337,
     337,   105,   337,   337,   401,   402,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,    81,    88,
     121,   148,   187,   215,   356,   373,   376,   381,   415,   418,
     415,    36,   337,   430,   431,   337,   121,   127,   187,   215,
     250,   373,   374,   375,   377,   381,   412,   413,   414,   422,
     427,   428,   187,   328,   378,   187,   328,   347,   329,   337,
     223,   328,   187,   187,   187,   328,   189,   337,   204,   189,
     337,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    55,    57,    69,    70,    71,    72,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   127,   128,   129,   130,   134,   135,
     136,   153,   157,   165,   167,   170,   177,   187,   204,   205,
     206,   217,   444,   459,   460,   462,   189,   334,   337,   190,
     230,   337,   108,   109,   156,   207,   208,   209,   210,   214,
      81,   192,   284,   285,   120,   127,   119,   127,    81,   286,
     187,   187,   187,   187,   204,   256,   447,   187,   187,   329,
      81,    87,   149,   150,   151,   436,   437,   156,   190,   214,
     214,   204,   257,   447,   157,   187,   447,   447,    81,   184,
     190,   348,    27,   327,   331,   337,   338,   415,   419,   219,
     190,    87,   379,   436,    87,   436,   436,    30,   156,   173,
     448,   187,     9,   189,    36,   236,   157,   255,   447,   121,
     183,   237,   319,   189,   189,   189,   189,   189,   189,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    55,    67,   189,    68,
      68,   190,   152,   128,   163,   165,   178,   180,   258,   317,
     318,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    65,    66,   131,   132,   405,    68,
     190,   410,   187,   187,    68,   190,   192,   423,   187,   236,
     237,    14,   337,   189,   133,    46,   204,   400,    87,   327,
     338,   152,   415,   133,   194,     9,   386,   251,   327,   338,
     415,   448,   152,   187,   380,   405,   410,   188,   337,    30,
     221,     8,   349,     9,   189,   221,   222,   329,   330,   337,
     204,   270,   225,   189,   189,   189,   135,   136,   462,   462,
     173,   187,   108,   462,    14,   152,   135,   136,   153,   204,
     206,   189,   189,   231,   112,   170,   189,   207,   209,   207,
     209,   214,   190,     9,   387,   189,    99,   156,   190,   415,
       9,   189,   127,   127,    14,     9,   189,   415,   440,   329,
     327,   338,   415,   418,   419,   188,   173,   248,   134,   415,
     429,   430,   189,    68,   405,   149,   437,    80,   337,   415,
      87,   149,   437,   214,   203,   189,   190,   243,   253,   363,
     365,    88,   187,   192,   350,   351,   353,   376,   421,   423,
     441,    14,    99,   442,   344,   345,   346,   280,   281,   403,
     404,   188,   188,   188,   188,   188,   191,   220,   221,   238,
     245,   252,   403,   337,   193,   195,   196,   204,   449,   450,
     462,    36,   166,   282,   283,   337,   444,   187,   447,   246,
     236,   337,   337,   337,    30,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   377,
     337,   337,   425,   425,   337,   432,   433,   127,   190,   205,
     206,   422,   423,   256,   204,   257,   447,   447,   255,   237,
      36,   331,   334,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   157,   190,   204,   406,
     407,   408,   409,   422,   425,   337,   282,   282,   425,   337,
     429,   236,   188,   337,   187,   399,     9,   386,   188,   188,
      36,   337,    36,   337,   380,   188,   188,   188,   422,   282,
     190,   204,   406,   407,   422,   188,   219,   274,   190,   334,
     337,   337,    91,    30,   221,   268,   189,    28,    99,    14,
       9,   188,    30,   190,   271,   462,    29,    88,   217,   456,
     457,   458,   187,     9,    48,    49,    54,    56,    68,   135,
     157,   177,   187,   215,   217,   358,   373,   381,   382,   383,
     204,   461,   219,   187,   229,   190,   189,   190,   189,    99,
     156,   108,   109,   156,   210,   211,   212,   213,   214,   210,
     204,   337,   285,   382,    81,     9,   188,   188,   188,   188,
     188,   188,   188,   189,    48,    49,   454,   455,   129,   261,
     187,     9,   188,   188,    81,    82,   204,   438,   204,    68,
     191,   191,   200,   202,    30,   130,   260,   172,    52,   157,
     172,   367,   338,   133,     9,   386,   188,   152,   462,   462,
      14,   349,   280,   219,   185,     9,   387,   462,   463,   405,
     410,   405,   191,     9,   386,   174,   415,   337,   188,     9,
     387,    14,   341,   239,   129,   259,   187,   447,   337,    30,
     194,   194,   133,   191,     9,   386,   337,   448,   187,   249,
     244,   254,    14,   442,   247,   236,    70,   415,   337,   448,
     194,   191,   188,   188,   194,   191,   188,    48,    49,    68,
      76,    77,    78,    88,   135,   148,   177,   204,   389,   391,
     392,   395,   398,   204,   415,   415,   133,   259,   405,   410,
     188,   337,   275,    73,    74,   276,   219,   328,   219,   330,
      99,    36,   134,   265,   415,   382,   204,    30,   221,   269,
     189,   272,   189,   272,     9,   174,    88,   133,   152,     9,
     386,   188,   166,   449,   450,   451,   449,   382,   382,   382,
     382,   382,   385,   388,   187,   152,   187,   382,   152,   190,
      10,    11,    12,    29,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    67,   152,   448,   191,
     373,   190,   233,   209,   209,   204,   210,   210,   214,     9,
     387,   191,   191,    14,   415,   189,     9,   174,   204,   262,
     373,   190,   429,   134,   415,    14,   194,   337,   191,   200,
     462,   262,   190,   366,    14,   188,   337,   350,   422,   189,
     462,   185,   191,    30,   452,   404,    36,    81,   166,   406,
     407,   409,   406,   407,   462,    36,   166,   337,   382,   280,
     187,   373,   260,   342,   240,   337,   337,   337,   191,   187,
     282,   261,    30,   260,   462,    14,   259,   447,   377,   191,
     187,    14,    76,    77,    78,   204,   390,   390,   392,   393,
     394,    50,   187,    87,   149,   187,     9,   386,   188,   399,
      36,   337,   260,   191,    73,    74,   277,   328,   221,   191,
     189,    92,   189,   265,   415,   187,   133,   264,    14,   219,
     272,   102,   103,   104,   272,   191,   462,   133,   462,   204,
     456,     9,   188,   386,   133,   194,     9,   386,   385,   205,
     350,   352,   354,   188,   127,   205,   382,   434,   435,   382,
     382,   382,    30,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   461,    81,   234,
     191,   191,   213,   189,   382,   455,    99,   100,   453,     9,
     290,   188,   187,   331,   334,   337,   194,   191,   442,   290,
     158,   171,   190,   362,   369,   158,   190,   368,   133,   189,
     452,   462,   349,   463,    81,   166,    14,    81,   448,   415,
     337,   188,   280,   190,   280,   187,   133,   187,   282,   188,
     190,   462,   190,   189,   462,   260,   241,   380,   282,   133,
     194,     9,   386,   391,   393,   149,   350,   396,   397,   392,
     415,   190,   328,    30,    75,   221,   189,   330,   264,   429,
     265,   188,   382,    98,   102,   189,   337,    30,   189,   273,
     191,   174,   462,   133,   166,    30,   188,   382,   382,   188,
     133,     9,   386,   188,   133,   191,     9,   386,   382,    30,
     188,   219,   189,   189,   204,   462,   462,   373,     4,   109,
     114,   120,   122,   159,   160,   162,   191,   291,   316,   317,
     318,   323,   324,   325,   326,   403,   429,   191,   190,   191,
      52,   337,   337,   337,   349,    36,    81,   166,    14,    81,
     337,   187,   452,   188,   290,   188,   280,   337,   282,   188,
     290,   442,   290,   189,   190,   187,   188,   392,   392,   188,
     133,   188,     9,   386,   290,    30,   219,   189,   188,   188,
     188,   226,   189,   189,   273,   219,   462,   462,   133,   382,
     350,   382,   382,   382,   190,   191,   453,   129,   130,   178,
     205,   445,   462,   263,   373,   109,   326,    29,   122,   135,
     136,   157,   163,   300,   301,   302,   303,   373,   161,   308,
     309,   125,   187,   204,   310,   311,   292,   237,   462,     9,
     189,     9,   189,   189,   442,   317,   188,   287,   157,   364,
     191,   191,    81,   166,    14,    81,   337,   282,   114,   339,
     452,   191,   452,   188,   188,   191,   190,   191,   290,   280,
     133,   392,   350,   191,   219,   224,   227,    30,   221,   267,
     219,   188,   382,   133,   133,   219,   373,   373,   447,    14,
     205,     9,   189,   190,   445,   442,   303,   173,   190,     9,
     189,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    55,    69,    70,    71,    72,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   134,   135,   137,   138,   139,   140,   141,   153,   154,
     155,   165,   167,   168,   170,   177,   178,   180,   182,   183,
     204,   370,   371,     9,   189,   157,   161,   204,   311,   312,
     313,   189,    81,   322,   236,   293,   445,   445,    14,   237,
     191,   288,   289,   445,    14,    81,   337,   188,   187,   190,
     189,   190,   314,   339,   452,   287,   191,   188,   392,   133,
      30,   221,   266,   267,   219,   382,   382,   191,   189,   189,
     382,   373,   296,   462,   304,   305,   381,   301,    14,    30,
      49,   306,   309,     9,    34,   188,    29,    48,    51,    14,
       9,   189,   206,   446,   322,    14,   462,   236,   189,    14,
     337,    36,    81,   361,   219,   219,   190,   314,   191,   452,
     392,   219,    96,   232,   191,   204,   217,   297,   298,   299,
       9,   174,     9,   386,   191,   382,   371,   371,    57,   307,
     312,   312,    29,    48,    51,   382,    81,   173,   187,   189,
     382,   447,   382,    81,     9,   387,   191,   191,   219,   314,
      94,   189,   112,   228,   152,    99,   462,   381,   164,    14,
     454,   294,   187,    36,    81,   188,   191,   189,   187,   170,
     235,   204,   317,   318,   174,   382,   174,   278,   279,   404,
     295,    81,   373,   233,   167,   204,   189,   188,     9,   387,
     116,   117,   118,   320,   321,   278,    81,   263,   189,   452,
     404,   463,   188,   188,   189,   189,   190,   315,   320,    36,
      81,   166,   452,   190,   219,   463,    81,   166,    14,    81,
     315,   219,   191,    36,    81,   166,    14,    81,   337,   191,
      81,   166,    14,    81,   337,    14,    81,   337,   337
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
#line 727 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 730 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 737 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onUse((yyvsp[(2) - (3)]), &Parser::useClass);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(); (yyval).reset();;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 866 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
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
#line 989 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1140 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1144 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
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

  case 195:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
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

  case 197:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval).reset();;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval).reset();;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval).reset();;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval).reset();;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval).reset();;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
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

  case 516:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
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

  case 518:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
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

  case 520:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
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

  case 555:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
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

  case 556:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval).reset();;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval).reset();;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { (yyval).reset();;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval).reset();;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
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

  case 810:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
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

  case 811:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
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

  case 812:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
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

  case 813:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
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

  case 820:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
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

  case 829:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { (yyval).reset();;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval)++;;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
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

  case 864:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
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
#line 2939 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval).reset();;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2975 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
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

  case 906:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3170 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    {;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    {;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3190 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3201 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3213 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3272 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3287 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13700 "hphp.5.tab.cpp"
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
#line 3290 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

