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
#define YYLAST   16539

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  262
/* YYNRULES -- Number of rules.  */
#define YYNRULES  973
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1777

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
      56,    61,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   138,
     140,   142,   144,   146,   148,   150,   152,   154,   156,   158,
     160,   162,   164,   166,   168,   170,   172,   174,   176,   178,
     180,   182,   184,   186,   188,   190,   192,   194,   196,   198,
     200,   202,   204,   206,   210,   212,   214,   217,   221,   226,
     228,   232,   234,   238,   241,   244,   247,   253,   258,   261,
     262,   264,   266,   268,   270,   274,   280,   289,   290,   295,
     296,   303,   304,   315,   316,   321,   324,   328,   331,   335,
     338,   342,   346,   350,   354,   358,   362,   368,   370,   372,
     374,   375,   385,   386,   397,   403,   404,   418,   419,   425,
     429,   433,   436,   439,   442,   445,   448,   451,   455,   458,
     461,   462,   467,   477,   478,   479,   484,   487,   488,   490,
     491,   493,   494,   504,   505,   516,   517,   529,   530,   540,
     541,   552,   553,   562,   563,   573,   574,   582,   583,   592,
     593,   602,   603,   611,   612,   621,   623,   625,   627,   629,
     631,   634,   638,   642,   645,   648,   649,   652,   653,   656,
     657,   659,   663,   665,   669,   672,   673,   675,   678,   683,
     685,   690,   692,   697,   699,   704,   706,   711,   715,   721,
     725,   730,   735,   741,   747,   752,   753,   755,   757,   762,
     763,   769,   770,   773,   774,   778,   779,   787,   796,   803,
     806,   812,   819,   824,   825,   830,   836,   844,   851,   858,
     866,   876,   885,   892,   900,   906,   909,   914,   920,   924,
     925,   929,   934,   941,   947,   953,   960,   969,   977,   980,
     981,   983,   986,   989,   993,   998,  1003,  1007,  1009,  1011,
    1014,  1019,  1023,  1029,  1031,  1035,  1038,  1039,  1042,  1046,
    1049,  1050,  1051,  1056,  1057,  1063,  1066,  1069,  1072,  1073,
    1084,  1085,  1097,  1101,  1105,  1109,  1114,  1119,  1123,  1129,
    1132,  1135,  1136,  1143,  1149,  1154,  1158,  1160,  1162,  1166,
    1171,  1173,  1176,  1178,  1180,  1185,  1192,  1194,  1196,  1201,
    1203,  1205,  1209,  1212,  1215,  1216,  1219,  1220,  1222,  1226,
    1228,  1230,  1232,  1234,  1238,  1243,  1248,  1253,  1255,  1257,
    1260,  1263,  1266,  1270,  1274,  1276,  1278,  1280,  1282,  1286,
    1288,  1292,  1294,  1296,  1298,  1299,  1301,  1304,  1306,  1308,
    1310,  1312,  1314,  1316,  1318,  1320,  1321,  1323,  1325,  1327,
    1331,  1337,  1339,  1343,  1349,  1354,  1358,  1362,  1366,  1371,
    1375,  1379,  1383,  1386,  1389,  1391,  1393,  1397,  1401,  1403,
    1405,  1406,  1408,  1411,  1416,  1420,  1424,  1431,  1434,  1438,
    1445,  1447,  1449,  1451,  1453,  1455,  1462,  1466,  1471,  1478,
    1482,  1486,  1490,  1494,  1498,  1502,  1506,  1510,  1514,  1518,
    1522,  1526,  1529,  1532,  1535,  1538,  1542,  1546,  1550,  1554,
    1558,  1562,  1566,  1570,  1574,  1578,  1582,  1586,  1590,  1594,
    1598,  1602,  1606,  1609,  1612,  1615,  1618,  1622,  1626,  1630,
    1634,  1638,  1642,  1646,  1650,  1654,  1658,  1662,  1668,  1673,
    1677,  1679,  1682,  1685,  1688,  1691,  1694,  1697,  1700,  1703,
    1706,  1708,  1710,  1712,  1716,  1719,  1721,  1727,  1728,  1729,
    1741,  1742,  1755,  1756,  1761,  1762,  1770,  1771,  1777,  1778,
    1782,  1783,  1790,  1793,  1796,  1801,  1803,  1805,  1811,  1815,
    1821,  1825,  1828,  1829,  1832,  1833,  1838,  1843,  1847,  1852,
    1857,  1862,  1867,  1869,  1871,  1873,  1875,  1879,  1883,  1888,
    1890,  1893,  1898,  1901,  1908,  1909,  1911,  1916,  1917,  1920,
    1921,  1923,  1925,  1929,  1931,  1935,  1937,  1939,  1943,  1947,
    1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,
    1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,
    1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,
    2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,
    2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,
    2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,
    2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,
    2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,
    2112,  2114,  2116,  2118,  2120,  2122,  2124,  2128,  2130,  2134,
    2136,  2138,  2142,  2144,  2146,  2148,  2151,  2153,  2154,  2155,
    2157,  2159,  2163,  2164,  2166,  2168,  2170,  2172,  2174,  2176,
    2178,  2180,  2182,  2184,  2186,  2188,  2190,  2194,  2197,  2199,
    2201,  2206,  2210,  2215,  2217,  2219,  2223,  2227,  2231,  2235,
    2239,  2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,
    2279,  2283,  2287,  2291,  2294,  2297,  2300,  2303,  2307,  2311,
    2315,  2319,  2323,  2327,  2331,  2335,  2339,  2345,  2350,  2354,
    2358,  2362,  2364,  2366,  2368,  2370,  2374,  2378,  2382,  2385,
    2386,  2388,  2389,  2391,  2392,  2398,  2402,  2406,  2408,  2410,
    2412,  2414,  2418,  2421,  2423,  2425,  2427,  2429,  2431,  2435,
    2437,  2439,  2441,  2444,  2447,  2452,  2456,  2461,  2464,  2465,
    2471,  2475,  2479,  2481,  2485,  2487,  2490,  2491,  2497,  2501,
    2504,  2505,  2509,  2510,  2515,  2518,  2519,  2523,  2527,  2529,
    2530,  2532,  2534,  2536,  2538,  2542,  2544,  2546,  2548,  2552,
    2554,  2556,  2560,  2564,  2567,  2572,  2575,  2580,  2586,  2592,
    2598,  2604,  2606,  2608,  2610,  2612,  2614,  2616,  2620,  2624,
    2629,  2634,  2638,  2640,  2642,  2644,  2646,  2650,  2652,  2657,
    2661,  2663,  2665,  2667,  2669,  2671,  2675,  2679,  2684,  2689,
    2693,  2695,  2697,  2705,  2715,  2723,  2730,  2739,  2741,  2744,
    2749,  2754,  2756,  2758,  2763,  2765,  2766,  2768,  2771,  2773,
    2775,  2777,  2781,  2785,  2789,  2790,  2792,  2794,  2798,  2802,
    2805,  2809,  2816,  2817,  2819,  2824,  2827,  2828,  2834,  2838,
    2842,  2844,  2851,  2856,  2861,  2864,  2867,  2868,  2874,  2878,
    2882,  2884,  2887,  2888,  2894,  2898,  2902,  2904,  2907,  2910,
    2912,  2915,  2917,  2922,  2926,  2930,  2937,  2941,  2943,  2945,
    2947,  2952,  2957,  2962,  2967,  2972,  2977,  2980,  2983,  2988,
    2991,  2994,  2996,  3000,  3004,  3008,  3009,  3012,  3018,  3025,
    3032,  3040,  3042,  3045,  3047,  3050,  3052,  3057,  3059,  3064,
    3068,  3069,  3071,  3075,  3078,  3082,  3084,  3086,  3087,  3088,
    3091,  3094,  3097,  3102,  3105,  3111,  3115,  3117,  3119,  3120,
    3124,  3129,  3135,  3139,  3141,  3144,  3145,  3150,  3152,  3156,
    3159,  3162,  3165,  3167,  3169,  3171,  3173,  3177,  3182,  3189,
    3191,  3200,  3207,  3209
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   216,    -1,   233,    -1,   240,    -1,   237,    -1,   247,
      -1,   438,    -1,   126,   187,   188,   189,    -1,   153,   209,
     189,    -1,    -1,   153,   209,   190,   202,   200,   191,    -1,
      -1,   153,   190,   203,   200,   191,    -1,   114,   207,   189,
      -1,   114,   108,   207,   189,    -1,   114,   109,   207,   189,
      -1,   213,   189,    -1,    79,    -1,   100,    -1,   159,    -1,
     160,    -1,   162,    -1,   164,    -1,   163,    -1,   204,    -1,
     136,    -1,   165,    -1,   129,    -1,   130,    -1,   121,    -1,
     120,    -1,   119,    -1,   118,    -1,   117,    -1,   116,    -1,
     109,    -1,    98,    -1,    94,    -1,    96,    -1,    75,    -1,
      92,    -1,    12,    -1,   115,    -1,   106,    -1,    55,    -1,
     167,    -1,   128,    -1,   153,    -1,    70,    -1,    10,    -1,
      11,    -1,   111,    -1,   114,    -1,   122,    -1,    71,    -1,
     134,    -1,    69,    -1,     7,    -1,     6,    -1,   113,    -1,
     135,    -1,    13,    -1,    89,    -1,     4,    -1,     3,    -1,
     110,    -1,    74,    -1,    73,    -1,   104,    -1,   105,    -1,
     107,    -1,   101,    -1,    27,    -1,   108,    -1,    72,    -1,
     102,    -1,   170,    -1,    93,    -1,    95,    -1,    97,    -1,
     103,    -1,    90,    -1,    91,    -1,    99,    -1,   112,    -1,
     123,    -1,   205,    -1,   127,    -1,   207,     9,   208,    -1,
     208,    -1,   209,    -1,   156,   209,    -1,   209,    99,   204,
      -1,   156,   209,    99,   204,    -1,   204,    -1,   209,   156,
     204,    -1,   209,    -1,   153,   156,   209,    -1,   156,   209,
      -1,   210,   443,    -1,   210,   443,    -1,   213,     9,   439,
      14,   377,    -1,   109,   439,    14,   377,    -1,   214,   215,
      -1,    -1,   216,    -1,   233,    -1,   240,    -1,   247,    -1,
     190,   214,   191,    -1,    72,   323,   216,   269,   271,    -1,
      72,   323,    30,   214,   270,   272,    75,   189,    -1,    -1,
      91,   323,   217,   263,    -1,    -1,    90,   218,   216,    91,
     323,   189,    -1,    -1,    93,   187,   325,   189,   325,   189,
     325,   188,   219,   261,    -1,    -1,   101,   323,   220,   266,
      -1,   105,   189,    -1,   105,   332,   189,    -1,   107,   189,
      -1,   107,   332,   189,    -1,   110,   189,    -1,   110,   332,
     189,    -1,    27,   105,   189,    -1,   115,   279,   189,    -1,
     121,   281,   189,    -1,    89,   324,   189,    -1,   145,   324,
     189,    -1,   123,   187,   435,   188,   189,    -1,   189,    -1,
      83,    -1,    84,    -1,    -1,    95,   187,   332,    99,   260,
     259,   188,   221,   262,    -1,    -1,    95,   187,   332,    28,
      99,   260,   259,   188,   222,   262,    -1,    97,   187,   265,
     188,   264,    -1,    -1,   111,   225,   112,   187,   368,    81,
     188,   190,   214,   191,   227,   223,   230,    -1,    -1,   111,
     225,   170,   224,   228,    -1,   113,   332,   189,    -1,   106,
     204,   189,    -1,   332,   189,    -1,   326,   189,    -1,   327,
     189,    -1,   328,   189,    -1,   329,   189,    -1,   330,   189,
      -1,   110,   329,   189,    -1,   331,   189,    -1,   204,    30,
      -1,    -1,   190,   226,   214,   191,    -1,   227,   112,   187,
     368,    81,   188,   190,   214,   191,    -1,    -1,    -1,   190,
     229,   214,   191,    -1,   170,   228,    -1,    -1,    36,    -1,
      -1,   108,    -1,    -1,   232,   231,   442,   234,   187,   275,
     188,   447,   309,    -1,    -1,   313,   232,   231,   442,   235,
     187,   275,   188,   447,   309,    -1,    -1,   398,   312,   232,
     231,   442,   236,   187,   275,   188,   447,   309,    -1,    -1,
     163,   204,   238,    30,   457,   437,   190,   282,   191,    -1,
      -1,   398,   163,   204,   239,    30,   457,   437,   190,   282,
     191,    -1,    -1,   253,   250,   241,   254,   255,   190,   285,
     191,    -1,    -1,   398,   253,   250,   242,   254,   255,   190,
     285,   191,    -1,    -1,   128,   251,   243,   256,   190,   285,
     191,    -1,    -1,   398,   128,   251,   244,   256,   190,   285,
     191,    -1,    -1,   127,   246,   375,   254,   255,   190,   285,
     191,    -1,    -1,   165,   252,   248,   255,   190,   285,   191,
      -1,    -1,   398,   165,   252,   249,   255,   190,   285,   191,
      -1,   442,    -1,   157,    -1,   442,    -1,   442,    -1,   127,
      -1,   120,   127,    -1,   120,   119,   127,    -1,   119,   120,
     127,    -1,   119,   127,    -1,   129,   368,    -1,    -1,   130,
     257,    -1,    -1,   129,   257,    -1,    -1,   368,    -1,   257,
       9,   368,    -1,   368,    -1,   258,     9,   368,    -1,   133,
     260,    -1,    -1,   410,    -1,    36,   410,    -1,   134,   187,
     424,   188,    -1,   216,    -1,    30,   214,    94,   189,    -1,
     216,    -1,    30,   214,    96,   189,    -1,   216,    -1,    30,
     214,    92,   189,    -1,   216,    -1,    30,   214,    98,   189,
      -1,   204,    14,   377,    -1,   265,     9,   204,    14,   377,
      -1,   190,   267,   191,    -1,   190,   189,   267,   191,    -1,
      30,   267,   102,   189,    -1,    30,   189,   267,   102,   189,
      -1,   267,   103,   332,   268,   214,    -1,   267,   104,   268,
     214,    -1,    -1,    30,    -1,   189,    -1,   269,    73,   323,
     216,    -1,    -1,   270,    73,   323,    30,   214,    -1,    -1,
      74,   216,    -1,    -1,    74,    30,   214,    -1,    -1,   274,
       9,   399,   315,   458,   166,    81,    -1,   274,     9,   399,
     315,   458,    36,   166,    81,    -1,   274,     9,   399,   315,
     458,   166,    -1,   274,   382,    -1,   399,   315,   458,   166,
      81,    -1,   399,   315,   458,    36,   166,    81,    -1,   399,
     315,   458,   166,    -1,    -1,   399,   315,   458,    81,    -1,
     399,   315,   458,    36,    81,    -1,   399,   315,   458,    36,
      81,    14,   332,    -1,   399,   315,   458,    81,    14,   332,
      -1,   274,     9,   399,   315,   458,    81,    -1,   274,     9,
     399,   315,   458,    36,    81,    -1,   274,     9,   399,   315,
     458,    36,    81,    14,   332,    -1,   274,     9,   399,   315,
     458,    81,    14,   332,    -1,   276,     9,   399,   458,   166,
      81,    -1,   276,     9,   399,   458,    36,   166,    81,    -1,
     276,     9,   399,   458,   166,    -1,   276,   382,    -1,   399,
     458,   166,    81,    -1,   399,   458,    36,   166,    81,    -1,
     399,   458,   166,    -1,    -1,   399,   458,    81,    -1,   399,
     458,    36,    81,    -1,   399,   458,    36,    81,    14,   332,
      -1,   399,   458,    81,    14,   332,    -1,   276,     9,   399,
     458,    81,    -1,   276,     9,   399,   458,    36,    81,    -1,
     276,     9,   399,   458,    36,    81,    14,   332,    -1,   276,
       9,   399,   458,    81,    14,   332,    -1,   278,   382,    -1,
      -1,   332,    -1,    36,   410,    -1,   166,   332,    -1,   278,
       9,   332,    -1,   278,     9,   166,   332,    -1,   278,     9,
      36,   410,    -1,   279,     9,   280,    -1,   280,    -1,    81,
      -1,   192,   410,    -1,   192,   190,   332,   191,    -1,   281,
       9,    81,    -1,   281,     9,    81,    14,   377,    -1,    81,
      -1,    81,    14,   377,    -1,   282,   283,    -1,    -1,   284,
     189,    -1,   440,    14,   377,    -1,   285,   286,    -1,    -1,
      -1,   311,   287,   317,   189,    -1,    -1,   313,   457,   288,
     317,   189,    -1,   318,   189,    -1,   319,   189,    -1,   320,
     189,    -1,    -1,   312,   232,   231,   441,   187,   289,   273,
     188,   447,   310,    -1,    -1,   398,   312,   232,   231,   442,
     187,   290,   273,   188,   447,   310,    -1,   159,   295,   189,
      -1,   160,   303,   189,    -1,   162,   305,   189,    -1,     4,
     129,   368,   189,    -1,     4,   130,   368,   189,    -1,   114,
     258,   189,    -1,   114,   258,   190,   291,   191,    -1,   291,
     292,    -1,   291,   293,    -1,    -1,   212,   152,   204,   167,
     258,   189,    -1,   294,    99,   312,   204,   189,    -1,   294,
      99,   313,   189,    -1,   212,   152,   204,    -1,   204,    -1,
     296,    -1,   295,     9,   296,    -1,   297,   365,   301,   302,
      -1,   157,    -1,    29,   298,    -1,   298,    -1,   135,    -1,
     135,   173,   457,   174,    -1,   135,   173,   457,     9,   457,
     174,    -1,   368,    -1,   122,    -1,   163,   190,   300,   191,
      -1,   136,    -1,   376,    -1,   299,     9,   376,    -1,   299,
     381,    -1,    14,   377,    -1,    -1,    57,   164,    -1,    -1,
     304,    -1,   303,     9,   304,    -1,   161,    -1,   306,    -1,
     204,    -1,   125,    -1,   187,   307,   188,    -1,   187,   307,
     188,    51,    -1,   187,   307,   188,    29,    -1,   187,   307,
     188,    48,    -1,   306,    -1,   308,    -1,   308,    51,    -1,
     308,    29,    -1,   308,    48,    -1,   307,     9,   307,    -1,
     307,    34,   307,    -1,   204,    -1,   157,    -1,   161,    -1,
     189,    -1,   190,   214,   191,    -1,   189,    -1,   190,   214,
     191,    -1,   313,    -1,   122,    -1,   313,    -1,    -1,   314,
      -1,   313,   314,    -1,   116,    -1,   117,    -1,   118,    -1,
     121,    -1,   120,    -1,   119,    -1,   183,    -1,   316,    -1,
      -1,   116,    -1,   117,    -1,   118,    -1,   317,     9,    81,
      -1,   317,     9,    81,    14,   377,    -1,    81,    -1,    81,
      14,   377,    -1,   318,     9,   440,    14,   377,    -1,   109,
     440,    14,   377,    -1,   319,     9,   440,    -1,   120,   109,
     440,    -1,   120,   321,   437,    -1,   321,   437,    14,   457,
      -1,   109,   178,   442,    -1,   187,   322,   188,    -1,    70,
     372,   375,    -1,    70,   245,    -1,    69,   332,    -1,   357,
      -1,   352,    -1,   187,   332,   188,    -1,   324,     9,   332,
      -1,   332,    -1,   324,    -1,    -1,    27,    -1,    27,   332,
      -1,    27,   332,   133,   332,    -1,   187,   326,   188,    -1,
     410,    14,   326,    -1,   134,   187,   424,   188,    14,   326,
      -1,    28,   332,    -1,   410,    14,   329,    -1,   134,   187,
     424,   188,    14,   329,    -1,   333,    -1,   410,    -1,   322,
      -1,   414,    -1,   413,    -1,   134,   187,   424,   188,    14,
     332,    -1,   410,    14,   332,    -1,   410,    14,    36,   410,
      -1,   410,    14,    36,    70,   372,   375,    -1,   410,    26,
     332,    -1,   410,    25,   332,    -1,   410,    24,   332,    -1,
     410,    23,   332,    -1,   410,    22,   332,    -1,   410,    21,
     332,    -1,   410,    20,   332,    -1,   410,    19,   332,    -1,
     410,    18,   332,    -1,   410,    17,   332,    -1,   410,    16,
     332,    -1,   410,    15,   332,    -1,   410,    66,    -1,    66,
     410,    -1,   410,    65,    -1,    65,   410,    -1,   332,    32,
     332,    -1,   332,    33,   332,    -1,   332,    10,   332,    -1,
     332,    12,   332,    -1,   332,    11,   332,    -1,   332,    34,
     332,    -1,   332,    36,   332,    -1,   332,    35,   332,    -1,
     332,    50,   332,    -1,   332,    48,   332,    -1,   332,    49,
     332,    -1,   332,    51,   332,    -1,   332,    52,   332,    -1,
     332,    67,   332,    -1,   332,    53,   332,    -1,   332,    47,
     332,    -1,   332,    46,   332,    -1,    48,   332,    -1,    49,
     332,    -1,    54,   332,    -1,    56,   332,    -1,   332,    38,
     332,    -1,   332,    37,   332,    -1,   332,    40,   332,    -1,
     332,    39,   332,    -1,   332,    41,   332,    -1,   332,    45,
     332,    -1,   332,    42,   332,    -1,   332,    44,   332,    -1,
     332,    43,   332,    -1,   332,    55,   372,    -1,   187,   333,
     188,    -1,   332,    29,   332,    30,   332,    -1,   332,    29,
      30,   332,    -1,   332,    31,   332,    -1,   434,    -1,    64,
     332,    -1,    63,   332,    -1,    62,   332,    -1,    61,   332,
      -1,    60,   332,    -1,    59,   332,    -1,    58,   332,    -1,
      71,   373,    -1,    57,   332,    -1,   379,    -1,   351,    -1,
     350,    -1,   193,   374,   193,    -1,    13,   332,    -1,   354,
      -1,   114,   187,   356,   382,   188,    -1,    -1,    -1,   232,
     231,   187,   336,   275,   188,   447,   334,   190,   214,   191,
      -1,    -1,   313,   232,   231,   187,   337,   275,   188,   447,
     334,   190,   214,   191,    -1,    -1,   183,    81,   339,   344,
      -1,    -1,   183,   184,   340,   275,   185,   447,   344,    -1,
      -1,   183,   190,   341,   214,   191,    -1,    -1,    81,   342,
     344,    -1,    -1,   184,   343,   275,   185,   447,   344,    -1,
       8,   332,    -1,     8,   329,    -1,     8,   190,   214,   191,
      -1,    88,    -1,   436,    -1,   346,     9,   345,   133,   332,
      -1,   345,   133,   332,    -1,   347,     9,   345,   133,   377,
      -1,   345,   133,   377,    -1,   346,   381,    -1,    -1,   347,
     381,    -1,    -1,   177,   187,   348,   188,    -1,   135,   187,
     425,   188,    -1,    68,   425,   194,    -1,   368,   190,   427,
     191,    -1,   368,   190,   429,   191,    -1,   354,    68,   420,
     194,    -1,   355,    68,   420,   194,    -1,   351,    -1,   436,
      -1,   413,    -1,    88,    -1,   187,   333,   188,    -1,   356,
       9,    81,    -1,   356,     9,    36,    81,    -1,    81,    -1,
      36,    81,    -1,   171,   157,   358,   172,    -1,   360,    52,
      -1,   360,   172,   361,   171,    52,   359,    -1,    -1,   157,
      -1,   360,   362,    14,   363,    -1,    -1,   361,   364,    -1,
      -1,   157,    -1,   158,    -1,   190,   332,   191,    -1,   158,
      -1,   190,   332,   191,    -1,   357,    -1,   366,    -1,   365,
      30,   366,    -1,   365,    49,   366,    -1,   204,    -1,    71,
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
      -1,   178,    -1,   180,    -1,   177,    -1,   211,   187,   277,
     188,    -1,   212,    -1,   157,    -1,   368,    -1,   376,    -1,
     121,    -1,   418,    -1,   187,   333,   188,    -1,   369,    -1,
     370,   152,   417,    -1,   369,    -1,   416,    -1,   371,   152,
     417,    -1,   368,    -1,   121,    -1,   422,    -1,   187,   188,
      -1,   323,    -1,    -1,    -1,    87,    -1,   431,    -1,   187,
     277,   188,    -1,    -1,    76,    -1,    77,    -1,    78,    -1,
      88,    -1,   140,    -1,   141,    -1,   155,    -1,   137,    -1,
     168,    -1,   138,    -1,   139,    -1,   154,    -1,   182,    -1,
     148,    87,   149,    -1,   148,   149,    -1,   376,    -1,   210,
      -1,   135,   187,   380,   188,    -1,    68,   380,   194,    -1,
     177,   187,   349,   188,    -1,   378,    -1,   353,    -1,   187,
     377,   188,    -1,   377,    32,   377,    -1,   377,    33,   377,
      -1,   377,    10,   377,    -1,   377,    12,   377,    -1,   377,
      11,   377,    -1,   377,    34,   377,    -1,   377,    36,   377,
      -1,   377,    35,   377,    -1,   377,    50,   377,    -1,   377,
      48,   377,    -1,   377,    49,   377,    -1,   377,    51,   377,
      -1,   377,    52,   377,    -1,   377,    53,   377,    -1,   377,
      47,   377,    -1,   377,    46,   377,    -1,   377,    67,   377,
      -1,    54,   377,    -1,    56,   377,    -1,    48,   377,    -1,
      49,   377,    -1,   377,    38,   377,    -1,   377,    37,   377,
      -1,   377,    40,   377,    -1,   377,    39,   377,    -1,   377,
      41,   377,    -1,   377,    45,   377,    -1,   377,    42,   377,
      -1,   377,    44,   377,    -1,   377,    43,   377,    -1,   377,
      29,   377,    30,   377,    -1,   377,    29,    30,   377,    -1,
     212,   152,   205,    -1,   157,   152,   205,    -1,   212,   152,
     127,    -1,   210,    -1,    80,    -1,   436,    -1,   376,    -1,
     195,   431,   195,    -1,   196,   431,   196,    -1,   148,   431,
     149,    -1,   383,   381,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   383,     9,   377,   133,   377,    -1,   383,     9,
     377,    -1,   377,   133,   377,    -1,   377,    -1,    76,    -1,
      77,    -1,    78,    -1,   148,    87,   149,    -1,   148,   149,
      -1,    76,    -1,    77,    -1,    78,    -1,   204,    -1,    88,
      -1,    88,    50,   386,    -1,   384,    -1,   386,    -1,   204,
      -1,    48,   385,    -1,    49,   385,    -1,   135,   187,   388,
     188,    -1,    68,   388,   194,    -1,   177,   187,   391,   188,
      -1,   389,   381,    -1,    -1,   389,     9,   387,   133,   387,
      -1,   389,     9,   387,    -1,   387,   133,   387,    -1,   387,
      -1,   390,     9,   387,    -1,   387,    -1,   392,   381,    -1,
      -1,   392,     9,   345,   133,   387,    -1,   345,   133,   387,
      -1,   390,   381,    -1,    -1,   187,   393,   188,    -1,    -1,
     395,     9,   204,   394,    -1,   204,   394,    -1,    -1,   397,
     395,   381,    -1,    47,   396,    46,    -1,   398,    -1,    -1,
     131,    -1,   132,    -1,   204,    -1,   157,    -1,   190,   332,
     191,    -1,   401,    -1,   417,    -1,   204,    -1,   190,   332,
     191,    -1,   403,    -1,   417,    -1,    68,   420,   194,    -1,
     190,   332,   191,    -1,   411,   405,    -1,   187,   322,   188,
     405,    -1,   423,   405,    -1,   187,   322,   188,   405,    -1,
     187,   322,   188,   400,   402,    -1,   187,   333,   188,   400,
     402,    -1,   187,   322,   188,   400,   401,    -1,   187,   333,
     188,   400,   401,    -1,   417,    -1,   367,    -1,   415,    -1,
     416,    -1,   406,    -1,   408,    -1,   410,   400,   402,    -1,
     371,   152,   417,    -1,   412,   187,   277,   188,    -1,   413,
     187,   277,   188,    -1,   187,   410,   188,    -1,   367,    -1,
     415,    -1,   416,    -1,   406,    -1,   410,   400,   401,    -1,
     409,    -1,   412,   187,   277,   188,    -1,   187,   410,   188,
      -1,   417,    -1,   406,    -1,   367,    -1,   351,    -1,   376,
      -1,   187,   410,   188,    -1,   187,   333,   188,    -1,   413,
     187,   277,   188,    -1,   412,   187,   277,   188,    -1,   187,
     414,   188,    -1,   335,    -1,   338,    -1,   410,   400,   404,
     443,   187,   277,   188,    -1,   187,   322,   188,   400,   404,
     443,   187,   277,   188,    -1,   371,   152,   206,   443,   187,
     277,   188,    -1,   371,   152,   417,   187,   277,   188,    -1,
     371,   152,   190,   332,   191,   187,   277,   188,    -1,   418,
      -1,   421,   418,    -1,   418,    68,   420,   194,    -1,   418,
     190,   332,   191,    -1,   419,    -1,    81,    -1,   192,   190,
     332,   191,    -1,   332,    -1,    -1,   192,    -1,   421,   192,
      -1,   417,    -1,   407,    -1,   408,    -1,   422,   400,   402,
      -1,   370,   152,   417,    -1,   187,   410,   188,    -1,    -1,
     407,    -1,   409,    -1,   422,   400,   401,    -1,   187,   410,
     188,    -1,   424,     9,    -1,   424,     9,   410,    -1,   424,
       9,   134,   187,   424,   188,    -1,    -1,   410,    -1,   134,
     187,   424,   188,    -1,   426,   381,    -1,    -1,   426,     9,
     332,   133,   332,    -1,   426,     9,   332,    -1,   332,   133,
     332,    -1,   332,    -1,   426,     9,   332,   133,    36,   410,
      -1,   426,     9,    36,   410,    -1,   332,   133,    36,   410,
      -1,    36,   410,    -1,   428,   381,    -1,    -1,   428,     9,
     332,   133,   332,    -1,   428,     9,   332,    -1,   332,   133,
     332,    -1,   332,    -1,   430,   381,    -1,    -1,   430,     9,
     377,   133,   377,    -1,   430,     9,   377,    -1,   377,   133,
     377,    -1,   377,    -1,   431,   432,    -1,   431,    87,    -1,
     432,    -1,    87,   432,    -1,    81,    -1,    81,    68,   433,
     194,    -1,    81,   400,   204,    -1,   150,   332,   191,    -1,
     150,    80,    68,   332,   194,   191,    -1,   151,   410,   191,
      -1,   204,    -1,    82,    -1,    81,    -1,   124,   187,   324,
     188,    -1,   125,   187,   410,   188,    -1,   125,   187,   333,
     188,    -1,   125,   187,   414,   188,    -1,   125,   187,   413,
     188,    -1,   125,   187,   322,   188,    -1,     7,   332,    -1,
       6,   332,    -1,     5,   187,   332,   188,    -1,     4,   332,
      -1,     3,   332,    -1,   410,    -1,   435,     9,   410,    -1,
     371,   152,   205,    -1,   371,   152,   127,    -1,    -1,    99,
     457,    -1,   178,   442,    14,   457,   189,    -1,   398,   178,
     442,    14,   457,   189,    -1,   180,   442,   437,    14,   457,
     189,    -1,   398,   180,   442,   437,    14,   457,   189,    -1,
     206,    -1,   457,   206,    -1,   205,    -1,   457,   205,    -1,
     206,    -1,   206,   173,   449,   174,    -1,   204,    -1,   204,
     173,   449,   174,    -1,   173,   445,   174,    -1,    -1,   457,
      -1,   444,     9,   457,    -1,   444,   381,    -1,   444,     9,
     166,    -1,   445,    -1,   166,    -1,    -1,    -1,    30,   457,
      -1,    99,   457,    -1,   100,   457,    -1,   449,     9,   450,
     204,    -1,   450,   204,    -1,   449,     9,   450,   204,   448,
      -1,   450,   204,   448,    -1,    48,    -1,    49,    -1,    -1,
      88,   133,   457,    -1,    29,    88,   133,   457,    -1,   212,
     152,   204,   133,   457,    -1,   452,     9,   451,    -1,   451,
      -1,   452,   381,    -1,    -1,   177,   187,   453,   188,    -1,
     212,    -1,   204,   152,   456,    -1,   204,   443,    -1,    29,
     457,    -1,    57,   457,    -1,   212,    -1,   135,    -1,   136,
      -1,   454,    -1,   455,   152,   456,    -1,   135,   173,   457,
     174,    -1,   135,   173,   457,     9,   457,   174,    -1,   157,
      -1,   187,   108,   187,   446,   188,    30,   457,   188,    -1,
     187,   457,     9,   444,   381,   188,    -1,   457,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   727,   727,   727,   736,   738,   741,   742,   743,   744,
     745,   746,   747,   750,   752,   752,   754,   754,   756,   758,
     761,   764,   769,   770,   771,   772,   773,   774,   775,   779,
     780,   781,   782,   783,   784,   785,   786,   787,   788,   789,
     790,   791,   792,   793,   794,   795,   796,   797,   798,   799,
     800,   801,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   829,
     830,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     843,   847,   848,   852,   854,   859,   860,   861,   863,   868,
     869,   873,   874,   876,   880,   887,   894,   898,   904,   906,
     909,   910,   911,   912,   915,   916,   920,   925,   925,   931,
     931,   938,   937,   943,   943,   948,   949,   950,   951,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     966,   964,   973,   971,   978,   986,   980,   990,   988,   992,
     993,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1013,  1013,  1018,  1024,  1028,  1028,  1036,  1037,  1041,  1042,
    1046,  1052,  1050,  1065,  1062,  1078,  1075,  1092,  1091,  1100,
    1098,  1110,  1109,  1128,  1126,  1145,  1144,  1153,  1151,  1162,
    1162,  1169,  1168,  1180,  1178,  1191,  1192,  1196,  1199,  1202,
    1203,  1204,  1207,  1208,  1211,  1213,  1216,  1217,  1220,  1221,
    1224,  1225,  1229,  1230,  1235,  1236,  1239,  1240,  1241,  1245,
    1246,  1250,  1251,  1255,  1256,  1260,  1261,  1266,  1267,  1272,
    1273,  1274,  1275,  1278,  1281,  1283,  1286,  1287,  1291,  1293,
    1296,  1299,  1302,  1303,  1306,  1307,  1311,  1317,  1323,  1330,
    1332,  1337,  1342,  1348,  1352,  1356,  1360,  1365,  1370,  1375,
    1380,  1386,  1395,  1400,  1405,  1411,  1413,  1417,  1421,  1426,
    1430,  1433,  1436,  1440,  1444,  1448,  1452,  1457,  1465,  1467,
    1470,  1471,  1472,  1473,  1475,  1477,  1482,  1483,  1486,  1487,
    1488,  1492,  1493,  1495,  1496,  1500,  1502,  1505,  1509,  1515,
    1517,  1520,  1520,  1524,  1523,  1527,  1529,  1532,  1535,  1533,
    1549,  1545,  1559,  1561,  1563,  1565,  1567,  1569,  1571,  1575,
    1576,  1577,  1580,  1586,  1590,  1596,  1599,  1604,  1606,  1611,
    1616,  1620,  1621,  1625,  1626,  1628,  1630,  1636,  1637,  1639,
    1643,  1644,  1649,  1653,  1654,  1658,  1659,  1663,  1665,  1671,
    1676,  1677,  1679,  1683,  1684,  1685,  1686,  1690,  1691,  1692,
    1693,  1694,  1695,  1697,  1702,  1705,  1706,  1710,  1711,  1715,
    1716,  1719,  1720,  1723,  1724,  1727,  1728,  1732,  1733,  1734,
    1735,  1736,  1737,  1738,  1742,  1743,  1746,  1747,  1748,  1751,
    1753,  1755,  1756,  1759,  1761,  1765,  1767,  1771,  1775,  1779,
    1784,  1785,  1787,  1788,  1789,  1790,  1793,  1797,  1798,  1802,
    1803,  1807,  1808,  1809,  1810,  1814,  1818,  1823,  1827,  1831,
    1836,  1837,  1838,  1839,  1840,  1844,  1846,  1847,  1848,  1851,
    1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,  1861,
    1862,  1863,  1864,  1865,  1866,  1867,  1868,  1869,  1870,  1871,
    1872,  1873,  1874,  1875,  1876,  1877,  1878,  1879,  1880,  1881,
    1882,  1883,  1884,  1885,  1886,  1887,  1888,  1889,  1890,  1891,
    1892,  1893,  1895,  1896,  1898,  1899,  1901,  1902,  1903,  1904,
    1905,  1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,
    1915,  1916,  1917,  1918,  1919,  1920,  1924,  1928,  1933,  1932,
    1947,  1945,  1963,  1962,  1981,  1980,  1999,  1998,  2016,  2016,
    2031,  2031,  2049,  2050,  2051,  2056,  2058,  2062,  2066,  2072,
    2076,  2082,  2084,  2088,  2090,  2094,  2098,  2099,  2103,  2110,
    2117,  2119,  2124,  2125,  2126,  2127,  2129,  2133,  2134,  2135,
    2136,  2140,  2146,  2155,  2168,  2169,  2172,  2175,  2178,  2179,
    2182,  2186,  2189,  2192,  2199,  2200,  2204,  2205,  2207,  2211,
    2212,  2213,  2214,  2215,  2216,  2217,  2218,  2219,  2220,  2221,
    2222,  2223,  2224,  2225,  2226,  2227,  2228,  2229,  2230,  2231,
    2232,  2233,  2234,  2235,  2236,  2237,  2238,  2239,  2240,  2241,
    2242,  2243,  2244,  2245,  2246,  2247,  2248,  2249,  2250,  2251,
    2252,  2253,  2254,  2255,  2256,  2257,  2258,  2259,  2260,  2261,
    2262,  2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,
    2272,  2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,
    2282,  2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,  2294,
    2299,  2300,  2304,  2305,  2306,  2307,  2309,  2313,  2314,  2325,
    2326,  2328,  2340,  2341,  2342,  2346,  2347,  2348,  2352,  2353,
    2354,  2357,  2359,  2363,  2364,  2365,  2366,  2368,  2369,  2370,
    2371,  2372,  2373,  2374,  2375,  2376,  2377,  2380,  2385,  2386,
    2387,  2389,  2390,  2392,  2393,  2394,  2395,  2397,  2399,  2401,
    2403,  2405,  2406,  2407,  2408,  2409,  2410,  2411,  2412,  2413,
    2414,  2415,  2416,  2417,  2418,  2419,  2420,  2421,  2423,  2425,
    2427,  2429,  2430,  2433,  2434,  2438,  2442,  2444,  2448,  2451,
    2454,  2460,  2461,  2462,  2463,  2464,  2465,  2466,  2471,  2473,
    2477,  2478,  2481,  2482,  2486,  2489,  2491,  2493,  2497,  2498,
    2499,  2500,  2503,  2507,  2508,  2509,  2510,  2514,  2516,  2523,
    2524,  2525,  2526,  2527,  2528,  2530,  2531,  2536,  2538,  2541,
    2544,  2546,  2548,  2551,  2553,  2557,  2559,  2562,  2565,  2571,
    2573,  2576,  2577,  2582,  2585,  2589,  2589,  2594,  2597,  2598,
    2602,  2603,  2607,  2608,  2609,  2613,  2615,  2623,  2624,  2628,
    2630,  2638,  2639,  2643,  2644,  2649,  2651,  2656,  2667,  2681,
    2693,  2708,  2709,  2710,  2711,  2712,  2713,  2714,  2724,  2733,
    2735,  2737,  2741,  2742,  2743,  2744,  2745,  2761,  2762,  2764,
    2773,  2774,  2775,  2776,  2777,  2778,  2779,  2780,  2782,  2787,
    2791,  2792,  2796,  2799,  2806,  2810,  2819,  2826,  2828,  2834,
    2836,  2837,  2841,  2842,  2849,  2850,  2855,  2856,  2861,  2862,
    2863,  2864,  2875,  2878,  2881,  2882,  2883,  2884,  2895,  2899,
    2900,  2901,  2903,  2904,  2905,  2909,  2911,  2914,  2916,  2917,
    2918,  2919,  2922,  2924,  2925,  2929,  2931,  2934,  2936,  2937,
    2938,  2942,  2944,  2947,  2950,  2952,  2954,  2958,  2959,  2961,
    2962,  2968,  2969,  2971,  2981,  2983,  2985,  2988,  2989,  2990,
    2994,  2995,  2996,  2997,  2998,  2999,  3000,  3001,  3002,  3003,
    3004,  3008,  3009,  3013,  3015,  3023,  3025,  3029,  3033,  3038,
    3042,  3050,  3051,  3055,  3056,  3062,  3063,  3072,  3073,  3081,
    3084,  3088,  3091,  3096,  3101,  3103,  3104,  3105,  3109,  3110,
    3114,  3115,  3118,  3121,  3123,  3127,  3133,  3134,  3135,  3139,
    3143,  3153,  3161,  3163,  3167,  3169,  3174,  3180,  3183,  3188,
    3196,  3199,  3202,  3203,  3206,  3209,  3210,  3215,  3218,  3222,
    3226,  3232,  3242,  3243
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
  "ident_for_class_const", "ident", "use_declarations", "use_declaration",
  "namespace_name", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "try_statement_list", "$@12",
  "additional_catches", "finally_statement_list", "$@13",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@14", "$@15", "$@16",
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
     201,   201,   204,   204,   204,   204,   204,   204,   204,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   206,   206,   207,   207,   208,   208,   208,   208,   209,
     209,   210,   210,   210,   211,   212,   213,   213,   214,   214,
     215,   215,   215,   215,   216,   216,   216,   217,   216,   218,
     216,   219,   216,   220,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     221,   216,   222,   216,   216,   223,   216,   224,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     226,   225,   227,   227,   229,   228,   230,   230,   231,   231,
     232,   234,   233,   235,   233,   236,   233,   238,   237,   239,
     237,   241,   240,   242,   240,   243,   240,   244,   240,   246,
     245,   248,   247,   249,   247,   250,   250,   251,   252,   253,
     253,   253,   253,   253,   254,   254,   255,   255,   256,   256,
     257,   257,   258,   258,   259,   259,   260,   260,   260,   261,
     261,   262,   262,   263,   263,   264,   264,   265,   265,   266,
     266,   266,   266,   267,   267,   267,   268,   268,   269,   269,
     270,   270,   271,   271,   272,   272,   273,   273,   273,   273,
     273,   273,   273,   273,   274,   274,   274,   274,   274,   274,
     274,   274,   275,   275,   275,   275,   275,   275,   275,   275,
     276,   276,   276,   276,   276,   276,   276,   276,   277,   277,
     278,   278,   278,   278,   278,   278,   279,   279,   280,   280,
     280,   281,   281,   281,   281,   282,   282,   283,   284,   285,
     285,   287,   286,   288,   286,   286,   286,   286,   289,   286,
     290,   286,   286,   286,   286,   286,   286,   286,   286,   291,
     291,   291,   292,   293,   293,   294,   294,   295,   295,   296,
     296,   297,   297,   298,   298,   298,   298,   298,   298,   298,
     299,   299,   300,   301,   301,   302,   302,   303,   303,   304,
     305,   305,   305,   306,   306,   306,   306,   307,   307,   307,
     307,   307,   307,   307,   308,   308,   308,   309,   309,   310,
     310,   311,   311,   312,   312,   313,   313,   314,   314,   314,
     314,   314,   314,   314,   315,   315,   316,   316,   316,   317,
     317,   317,   317,   318,   318,   319,   319,   320,   320,   321,
     322,   322,   322,   322,   322,   322,   323,   324,   324,   325,
     325,   326,   326,   326,   326,   327,   328,   329,   330,   331,
     332,   332,   332,   332,   332,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   334,   334,   336,   335,
     337,   335,   339,   338,   340,   338,   341,   338,   342,   338,
     343,   338,   344,   344,   344,   345,   345,   346,   346,   347,
     347,   348,   348,   349,   349,   350,   351,   351,   352,   353,
     354,   354,   355,   355,   355,   355,   355,   356,   356,   356,
     356,   357,   358,   358,   359,   359,   360,   360,   361,   361,
     362,   363,   363,   364,   364,   364,   365,   365,   365,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   366,   366,   366,   366,   366,   366,   367,
     368,   368,   369,   369,   369,   369,   369,   370,   370,   371,
     371,   371,   372,   372,   372,   373,   373,   373,   374,   374,
     374,   375,   375,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   378,   378,
     378,   379,   379,   379,   379,   379,   379,   379,   380,   380,
     381,   381,   382,   382,   383,   383,   383,   383,   384,   384,
     384,   384,   384,   385,   385,   385,   385,   386,   386,   387,
     387,   387,   387,   387,   387,   387,   387,   388,   388,   389,
     389,   389,   389,   390,   390,   391,   391,   392,   392,   393,
     393,   394,   394,   395,   395,   397,   396,   398,   399,   399,
     400,   400,   401,   401,   401,   402,   402,   403,   403,   404,
     404,   405,   405,   406,   406,   407,   407,   408,   408,   409,
     409,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   411,   411,   411,   411,   411,   411,   411,   411,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   413,
     414,   414,   415,   415,   416,   416,   416,   417,   417,   418,
     418,   418,   419,   419,   420,   420,   421,   421,   422,   422,
     422,   422,   422,   422,   423,   423,   423,   423,   423,   424,
     424,   424,   424,   424,   424,   425,   425,   426,   426,   426,
     426,   426,   426,   426,   426,   427,   427,   428,   428,   428,
     428,   429,   429,   430,   430,   430,   430,   431,   431,   431,
     431,   432,   432,   432,   432,   432,   432,   433,   433,   433,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   435,   435,   436,   436,   437,   437,   438,   438,   438,
     438,   439,   439,   440,   440,   441,   441,   442,   442,   443,
     443,   444,   444,   445,   446,   446,   446,   446,   447,   447,
     448,   448,   449,   449,   449,   449,   450,   450,   450,   451,
     451,   451,   452,   452,   453,   453,   454,   455,   456,   456,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   457,
     457,   457,   458,   458
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     2,     3,     4,     1,
       3,     1,     3,     2,     2,     2,     5,     4,     2,     0,
       1,     1,     1,     1,     3,     5,     8,     0,     4,     0,
       6,     0,    10,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       0,     4,     9,     0,     0,     4,     2,     0,     1,     0,
       1,     0,     9,     0,    10,     0,    11,     0,     9,     0,
      10,     0,     8,     0,     9,     0,     7,     0,     8,     0,
       8,     0,     7,     0,     8,     1,     1,     1,     1,     1,
       2,     3,     3,     2,     2,     0,     2,     0,     2,     0,
       1,     3,     1,     3,     2,     0,     1,     2,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     5,     4,     0,     1,     1,     4,     0,
       5,     0,     2,     0,     3,     0,     7,     8,     6,     2,
       5,     6,     4,     0,     4,     5,     7,     6,     6,     7,
       9,     8,     6,     7,     5,     2,     4,     5,     3,     0,
       3,     4,     6,     5,     5,     6,     8,     7,     2,     0,
       1,     2,     2,     3,     4,     4,     3,     1,     1,     2,
       4,     3,     5,     1,     3,     2,     0,     2,     3,     2,
       0,     0,     4,     0,     5,     2,     2,     2,     0,    10,
       0,    11,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     2,     1,     1,     4,     6,     1,     1,     4,     1,
       1,     3,     2,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     3,     3,     3,     4,     3,
       3,     3,     2,     2,     1,     1,     3,     3,     1,     1,
       0,     1,     2,     4,     3,     3,     6,     2,     3,     6,
       1,     1,     1,     1,     1,     6,     3,     4,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     3,     2,     1,     5,     0,     0,    11,
       0,    12,     0,     4,     0,     7,     0,     5,     0,     3,
       0,     6,     2,     2,     4,     1,     1,     5,     3,     5,
       3,     2,     0,     2,     0,     4,     4,     3,     4,     4,
       4,     4,     1,     1,     1,     1,     3,     3,     4,     1,
       2,     4,     2,     6,     0,     1,     4,     0,     2,     0,
       1,     1,     3,     1,     3,     1,     1,     3,     3,     1,
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
       4,     1,     1,     4,     1,     0,     1,     2,     1,     1,
       1,     3,     3,     3,     0,     1,     1,     3,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     6,
       7,     1,     2,     1,     2,     1,     4,     1,     4,     3,
       0,     1,     3,     2,     3,     1,     1,     0,     0,     2,
       2,     2,     4,     2,     5,     3,     1,     1,     0,     3,
       4,     5,     3,     1,     2,     0,     4,     1,     3,     2,
       2,     2,     1,     1,     1,     1,     3,     4,     6,     1,
       8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   411,     0,   775,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   866,     0,
     854,   657,     0,   663,   664,   665,    22,   722,   842,   138,
     139,   666,     0,   119,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   170,     0,     0,     0,     0,     0,     0,
     377,   378,   379,   382,   381,   380,     0,     0,     0,     0,
     199,     0,     0,     0,   670,   672,   673,   667,   668,     0,
       0,     0,   674,   669,     0,   641,    24,    25,    26,    28,
      27,     0,   671,     0,     0,     0,     0,   675,   383,   510,
       0,   137,   109,   846,   658,     0,     0,     4,    99,   101,
     721,     0,   640,     0,     6,   169,     7,     9,     8,    10,
       0,     0,   375,   422,     0,     0,     0,     0,     0,     0,
       0,   420,   830,   831,   492,   491,   405,   495,     0,   404,
     802,   642,   649,     0,   724,   490,   374,   805,   806,   817,
     421,     0,     0,   424,   423,   803,   804,   801,   837,   841,
       0,   480,   723,    11,   382,   381,   380,     0,     0,    28,
       0,    99,   169,     0,   910,   421,   909,     0,   907,   906,
     494,     0,   412,   417,     0,     0,   462,   463,   464,   465,
     489,   487,   486,   485,   484,   483,   482,   481,   842,   666,
     644,     0,     0,   930,   823,   642,     0,   643,   444,     0,
     442,     0,   870,     0,   731,   403,   653,   189,     0,   930,
     402,   652,   647,     0,   662,   643,   849,   850,   856,   848,
     654,     0,     0,   656,   488,     0,     0,     0,     0,   408,
       0,   117,   410,     0,     0,   123,   125,     0,     0,   127,
       0,    69,    68,    63,    62,    54,    55,    46,    66,    77,
       0,    49,     0,    61,    53,    59,    79,    72,    71,    44,
      67,    86,    87,    45,    82,    42,    83,    43,    84,    41,
      88,    76,    80,    85,    73,    74,    48,    75,    78,    40,
      70,    56,    89,    64,    57,    47,    39,    38,    37,    36,
      35,    34,    58,    90,    92,    51,    32,    33,    60,   963,
     964,    52,   969,    31,    50,    81,     0,     0,    99,    91,
     921,   962,     0,   965,     0,     0,   129,     0,     0,   160,
       0,     0,     0,     0,     0,     0,    94,    95,   288,     0,
       0,   287,     0,   203,     0,   200,   293,     0,     0,     0,
       0,     0,   927,   185,   197,   862,   866,     0,   891,     0,
     677,     0,     0,     0,   889,     0,    16,     0,   103,   177,
     191,   198,   547,   522,     0,   915,   502,   504,   506,   779,
     411,   422,     0,     0,   420,   421,   423,     0,     0,   659,
       0,   660,     0,     0,     0,   159,     0,     0,   105,   279,
       0,    21,   168,     0,   196,   181,   195,   380,   383,   169,
     376,   152,   153,   154,   155,   156,   158,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   854,     0,   151,   845,   845,   876,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,   441,   780,   781,     0,   845,     0,   793,
     279,   279,   845,     0,   847,   838,   862,     0,   169,     0,
       0,   131,     0,   777,   772,   731,     0,   422,   420,     0,
     874,     0,   527,   730,   865,   662,   422,   420,   421,   105,
       0,   279,   401,     0,   795,   655,     0,   109,   239,     0,
     509,     0,   134,     0,     0,   409,     0,     0,     0,     0,
       0,   126,   150,   128,   963,   964,   960,   961,     0,   955,
       0,     0,     0,     0,    65,    30,    52,    29,   922,   157,
     130,   109,     0,   147,   149,     0,     0,    96,     0,    18,
       0,     0,   289,     0,   132,   202,   201,     0,     0,   133,
     911,     0,     0,   422,   420,   421,   424,   423,     0,   948,
     209,     0,   863,     0,     0,   135,     0,     0,   676,   890,
     722,     0,     0,   888,   727,   887,   102,     5,    13,    14,
       0,   207,     0,     0,   515,     0,     0,     0,   731,     0,
       0,   650,   645,   516,     0,     0,     0,     0,   779,   109,
       0,   733,   778,   973,   400,   414,   476,   811,   829,   114,
     108,   110,   111,   112,   113,   374,     0,   493,   725,   726,
     100,   731,     0,   931,     0,     0,     0,   733,   280,     0,
     498,   171,   205,     0,   447,   449,   448,     0,     0,   479,
     445,   446,   450,   452,   451,   467,   466,   469,   468,   470,
     472,   474,   473,   471,   461,   460,   454,   455,   453,   456,
     457,   459,   475,   458,   844,     0,     0,   880,     0,   731,
     914,     0,   913,   930,   808,   837,   187,   179,   193,     0,
     915,   183,   169,     0,   415,   418,   426,   440,   439,   438,
     437,   436,   435,   434,   433,   432,   431,   430,   429,   783,
       0,   782,   785,   807,   789,   930,   786,     0,     0,     0,
       0,     0,     0,     0,     0,   908,   413,   770,   774,   730,
     776,     0,   646,     0,   869,     0,   868,   205,     0,   646,
     853,   852,     0,     0,   782,   785,   851,   786,   406,   241,
     243,   109,   513,   512,   407,     0,   109,   223,   118,   410,
       0,     0,     0,     0,     0,   235,   235,   124,     0,     0,
       0,     0,   953,   731,     0,   937,     0,     0,     0,     0,
       0,   729,     0,   641,     0,     0,   679,   640,   684,     0,
     678,   107,   683,   930,   966,     0,     0,     0,    19,    20,
       0,    93,    97,     0,   286,   294,   291,     0,     0,   900,
     905,   902,   901,   904,   903,    12,   946,   947,     0,     0,
       0,     0,   862,   859,     0,   526,   899,   898,   897,     0,
     893,     0,   894,   896,     0,     5,     0,     0,     0,   541,
     542,   550,   549,     0,   420,     0,   730,   521,   525,     0,
       0,   916,     0,   503,     0,     0,   938,   779,   265,   972,
       0,     0,   794,     0,   843,   730,   933,   929,   281,   282,
     639,   732,   278,     0,   779,     0,     0,   207,   500,   173,
     478,     0,   530,   531,     0,   528,   730,   875,     0,     0,
     279,   209,     0,   207,     0,     0,   205,     0,   854,   427,
       0,     0,   791,   792,   809,   810,   839,   840,     0,     0,
       0,   758,   738,   739,   740,   747,     0,     0,     0,   751,
     749,   750,   764,   731,     0,   772,   873,   872,     0,   207,
       0,   796,   661,     0,   245,     0,     0,   115,     0,     0,
       0,     0,     0,     0,     0,   215,   216,   227,     0,   109,
     225,   144,   235,     0,   235,     0,     0,   967,     0,     0,
       0,   730,   954,   956,   936,   731,   935,     0,   731,   705,
     706,   703,   704,   737,     0,   731,   729,     0,   524,     0,
       0,   882,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     959,   161,     0,   164,   148,    98,   290,     0,   912,   136,
     948,   928,   943,   208,   210,   300,     0,     0,   860,     0,
     892,     0,    17,     0,   915,   206,   300,     0,     0,   646,
     518,     0,   651,   917,     0,   938,   507,     0,     0,   973,
       0,   270,   268,   785,   797,   930,   785,   798,   932,     0,
       0,   283,   106,     0,   779,   204,     0,   779,     0,   477,
     879,   878,     0,   279,     0,     0,     0,     0,     0,     0,
     207,   175,   662,   784,   279,     0,   743,   744,   745,   746,
     752,   753,   762,     0,   731,     0,   758,     0,   742,   766,
     730,   769,   771,   773,     0,   867,     0,   784,     0,     0,
       0,     0,   242,   514,   120,     0,   410,   215,   217,   862,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,     0,   949,     0,   952,   730,     0,     0,     0,   681,
     730,   728,     0,   719,     0,   731,     0,   685,   720,   718,
     886,     0,   731,   688,   690,   689,     0,     0,   686,   687,
     691,   693,   692,   708,   707,   710,   709,   711,   713,   715,
     714,   712,   701,   700,   695,   696,   694,   697,   698,   699,
     702,   958,     0,   109,   292,     0,     0,     0,   945,     0,
     374,   864,   862,   416,   419,   425,     0,    15,     0,   374,
     553,     0,     0,   555,   548,   551,     0,   546,     0,   919,
       0,   939,   511,     0,   271,     0,     0,   266,     0,   285,
     284,   938,     0,   300,     0,   779,     0,   279,     0,   835,
     300,   915,   300,   918,     0,     0,     0,   428,     0,     0,
     755,   730,   757,   748,     0,   741,     0,     0,   731,   763,
     871,   300,     0,   109,     0,   238,   224,     0,     0,     0,
     214,   140,   228,     0,     0,   231,     0,   236,   237,   109,
     230,   968,   950,     0,   934,     0,   971,   736,   735,   680,
       0,   730,   523,   682,     0,   529,   730,   881,   717,     0,
       0,     0,   942,   940,   941,   211,     0,     0,     0,   381,
     372,     0,     0,     0,   186,   299,   301,     0,   371,     0,
       0,     0,   915,   374,     0,   895,   296,   192,   544,     0,
       0,   517,   505,     0,   274,   264,     0,   267,   273,   279,
     497,   938,   374,   938,     0,   877,     0,   834,   374,     0,
     374,   920,   300,   779,   832,   761,   760,   754,     0,   756,
     730,   765,   374,   109,   244,   116,   121,   142,   218,     0,
     226,   232,   109,   234,   951,     0,     0,   520,     0,   885,
     884,   716,   109,   165,   944,     0,     0,     0,   923,     0,
       0,     0,   212,     0,   915,     0,   337,   333,   339,   641,
      28,     0,   327,     0,   332,   336,   349,     0,   347,   352,
       0,   351,     0,   350,     0,   169,   303,     0,   305,     0,
     306,   307,     0,     0,   861,     0,   545,   543,   554,   552,
     275,     0,     0,   262,   272,     0,     0,     0,     0,   182,
     497,   938,   836,   188,   296,   194,   374,     0,     0,   768,
       0,   190,   240,     0,     0,   109,   221,   141,   233,   970,
     734,     0,     0,     0,     0,     0,   399,     0,   924,     0,
     317,   321,   396,   397,   331,     0,     0,     0,   312,   605,
     604,   601,   603,   602,   622,   624,   623,   593,   564,   565,
     583,   599,   598,   560,   570,   571,   573,   572,   592,   576,
     574,   575,   577,   578,   579,   580,   581,   582,   584,   585,
     586,   587,   588,   589,   591,   590,   561,   562,   563,   566,
     567,   569,   607,   608,   617,   616,   615,   614,   613,   612,
     600,   619,   609,   610,   611,   594,   595,   596,   597,   620,
     621,   625,   627,   626,   628,   629,   606,   631,   630,   633,
     635,   634,   568,   638,   636,   637,   632,   618,   559,   344,
     556,     0,   313,   365,   366,   364,   357,     0,   358,   314,
     391,     0,     0,     0,     0,   395,     0,   169,   178,   295,
       0,     0,     0,   263,   277,   833,     0,   109,   367,   109,
     172,     0,     0,     0,   184,   938,   759,     0,   109,   219,
     122,   143,     0,   519,   883,   163,   315,   316,   394,   213,
       0,     0,   731,     0,   340,   328,     0,     0,     0,   346,
     348,     0,     0,   353,   360,   361,   359,     0,     0,   302,
     925,     0,     0,     0,   398,     0,   297,     0,   276,     0,
     539,   733,     0,     0,   109,   174,   180,     0,   767,     0,
       0,   145,   318,    99,     0,   319,   320,     0,     0,   334,
     730,   342,   338,   343,   557,   558,     0,   329,   362,   363,
     355,   356,   354,   392,   389,   948,   308,   304,   393,     0,
     298,   540,   732,     0,   499,   368,     0,   176,     0,   222,
       0,   167,     0,   374,     0,   341,   345,     0,     0,   779,
     310,     0,   537,   496,   501,   220,     0,     0,   146,   325,
       0,   373,   335,   390,   926,     0,   733,   385,   779,   538,
       0,   166,     0,     0,   324,   938,   779,   249,   386,   387,
     388,   973,   384,     0,     0,     0,   323,     0,   385,     0,
     938,     0,   322,   369,   109,   309,   973,     0,   254,   252,
       0,   109,     0,     0,   255,     0,     0,   250,   311,     0,
     370,     0,   258,   248,     0,   251,   257,   162,   259,     0,
       0,   246,   256,     0,   247,   261,   260
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   845,   597,   171,  1388,   693,
     335,   336,   109,   110,   111,   112,   113,   387,   630,   631,
     524,   240,  1453,   530,  1369,  1454,  1691,   807,   330,   551,
    1651,  1024,  1193,  1708,   403,   172,   632,   885,  1078,  1246,
     117,   600,   902,   633,   652,   906,   580,   901,   220,   505,
     634,   601,   903,   405,   353,   370,   120,   887,   848,   831,
    1033,  1391,  1131,   955,  1600,  1457,   768,   961,   529,   777,
     963,  1279,   760,   944,   947,  1120,  1715,  1716,   620,   621,
     646,   647,   340,   341,   347,  1425,  1579,  1580,  1200,  1315,
    1414,  1573,  1699,  1718,  1610,  1655,  1656,  1657,  1401,  1402,
    1403,  1404,  1612,  1613,  1619,  1667,  1407,  1408,  1412,  1566,
    1567,  1568,  1590,  1745,  1316,  1317,   173,   122,  1731,  1732,
    1571,  1319,  1320,  1321,  1322,   123,   233,   525,   526,   124,
     125,   126,   127,   128,   129,   130,   131,  1437,   132,   884,
    1077,   133,   617,   618,   619,   237,   379,   520,   607,   608,
    1155,   609,  1156,   134,   135,   136,   798,   137,   138,  1641,
     139,   602,  1427,   603,  1047,   853,  1217,  1214,  1559,  1560,
     140,   141,   142,   223,   143,   224,   234,   390,   512,   144,
     983,   802,   145,   984,   876,   868,   985,   930,  1100,   931,
    1102,  1103,  1104,   933,  1257,  1258,   934,   738,   495,   184,
     185,   635,   623,   476,  1063,  1064,   724,   725,   872,   147,
     226,   148,   149,   175,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   685,   160,   230,   231,   583,   213,   214,
     688,   689,  1161,  1162,   363,   364,   839,   161,   571,   162,
     616,   163,   322,  1581,  1631,   354,   398,   641,   642,   977,
    1058,  1198,   828,   829,   782,   783,   784,   323,   324,   804,
    1390,   870
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1238
static const yytype_int16 yypact[] =
{
   -1238,   186, -1238, -1238,  5464, 13030, 13030,    12, 13030, 13030,
   13030, 10896, 13030, -1238, 13030, 13030, 13030, 13030, 13030, 13030,
   13030, 13030, 13030, 13030, 13030, 13030, 15413, 15413, 11090, 13030,
    3495,    22,   180, -1238, -1238, -1238, -1238, -1238,   198, -1238,
   -1238,   294, 13030, -1238,   180,   182,   185,   224, -1238,   180,
   11284,  1114, 11478, -1238, 13780,  9926,   195, 13030,  2008,   317,
   -1238, -1238, -1238,    55,   411,    62,   314,   356,   358,   366,
   -1238,  1114,   379,   392, -1238, -1238, -1238, -1238, -1238, 13030,
     568,   695, -1238, -1238,  1114, -1238, -1238, -1238, -1238,  1114,
   -1238,  1114, -1238,   380,   394,  1114,  1114, -1238,   418, -1238,
   11672, -1238, -1238,   402,   483,   678,   678, -1238,   579,   456,
     511,   427, -1238,    77, -1238,   587, -1238, -1238, -1238, -1238,
     848,   683, -1238, -1238,   439,   450,   453,   486,   496,   508,
    2714, -1238, -1238, -1238, -1238,   264, -1238,   636,   648, -1238,
      20,   537, -1238,   584,     0, -1238,  2119,   156, -1238, -1238,
    2510,    98,   551,   281, -1238,   158,    32,   559,    82, -1238,
     318, -1238,   690, -1238, -1238, -1238,   598,   588,   605, -1238,
   13030, -1238,   587,   683, 16240,  3338, 16240, 13030, 16240, 16240,
   14227,   581, 14809, 14227,   720,  1114,   712,   712,   124,   712,
     712,   712,   712,   712,   712,   712,   712,   712, -1238, -1238,
   -1238,    71, 13030,   617, -1238, -1238,   632,   606,   247,   609,
     247, 15413, 15019,   613,   796, -1238,   598, -1238, 13030,   617,
   -1238,   658, -1238,   660,   626, -1238,   163, -1238, -1238, -1238,
     247,    98, 11866, -1238, -1238, 13030,  8762,   811,    80, 16240,
    9732, -1238, 13030, 13030,  1114, -1238, -1238,  4981,   634, -1238,
   10881, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
    4184, -1238,  4184, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,    94,
      95,   605, -1238, -1238, -1238, -1238,   639,  4147,   103, -1238,
   -1238,   669,   813, -1238,   688, 14365, -1238,   659, 11269, -1238,
      28, 11851,  1666,  1666,  1114,    92, -1238,   432, -1238, 15037,
      93, -1238,   725, -1238,   726, -1238,   833,   105, 15413, 13030,
   13030,   672,   691, -1238, -1238, 15130, 11090,   109,    86,   446,
   -1238, 13224, 15413,   666, -1238,  1114, -1238,     6,   456, -1238,
   -1238, -1238, -1238, 15868,   851,   769, -1238, -1238, -1238,   102,
   13030,   684,   685, 16240,   686,  2447,   687,  5658, 13030,   257,
     676,   681,   257,   455,   448, -1238,  1114,  4184,   692, 10120,
   13780, -1238, -1238,  1332, -1238, -1238, -1238, -1238, -1238,   587,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, 13030, 13030, 13030,
   12060, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030,
   13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030,
   13030, 13030, 13030, 13030, 15961, 13030, -1238, 13030, 13030, 13030,
    4582,  1114,  1114,  1114,  1114,  1114,   848,   772,  1127,  4812,
   13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030, 13030,
   13030, 13030, -1238, -1238, -1238, -1238,  1244, 13030, 13030, -1238,
   10120, 10120, 13030, 13030,   402,   167, 15130,   694,   587, 12254,
   13015, -1238, 13030, -1238,   696,   868,   733,   699,   700, 13363,
     247, 12448, -1238, 12642, -1238,   626,   702,   705,  2546, -1238,
     319, 10120, -1238,  1258, -1238, -1238, 13209, -1238, -1238, 10314,
   -1238, 13030, -1238,   805,  8956,   889,   711, 16123,   891,    67,
      81, -1238, -1238, -1238,   730, -1238, -1238, -1238,  4184,   543,
     727,   898, 14944,  1114, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238,   731, -1238, -1238,   112,   114,   433,  1666, -1238,
    1114, 13030,   247,   317, -1238, -1238, -1238, 14944,   831, -1238,
     247,    90,   107,   729,   742,  3036,   316,   744,   746,   526,
     790,   749,   247,   133,   752, -1238,  1188,  1114, -1238, -1238,
     873,  2368,   386, -1238, -1238, -1238,   456, -1238, -1238, -1238,
     912,   821,   781,   336,   802, 13030,   402,   823,   952,   774,
     812, -1238,   167, -1238,  4184,  4184,   954,   811,   102, -1238,
     787,   967, -1238,  4184,   381, -1238,   472,   171, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238,  1045,  2660, -1238, -1238, -1238,
   -1238,   969,   806, -1238, 15413, 13030,   791,   974, 16240,   971,
   -1238, -1238,   859,  1448,  4489, 16328, 14227, 13030, 16194, 16401,
    3857, 10100, 11069, 12426,  2543, 12619, 12619, 12619, 12619,  3392,
    3392,  3392,  3392,  3392,  1242,  1242,   782,   782,   782,   124,
     124,   124, -1238,   712, 16240,   795,   797, 15575,   808,   991,
     -10, 13030,    21,   617,     5,   167, -1238, -1238, -1238,   987,
     769, -1238,   587, 15227, -1238, -1238, 14227, 14227, 14227, 14227,
   14227, 14227, 14227, 14227, 14227, 14227, 14227, 14227, 14227, -1238,
   13030,   216,   174, -1238, -1238,   617,   361,   809,  2778,   814,
     818,   819,  2885,   136,   827, -1238, 16240,  2193, -1238,  1114,
   -1238,   381,    42, 15413, 16240, 15413, 15621,   859,   381,   247,
     175,   864,   832, 13030, -1238,   190, -1238, -1238, -1238,  8568,
     552, -1238, -1238, 16240, 16240,   180, -1238, -1238, -1238, 13030,
     922, 14827, 14944,  1114,  9150,   837,   838, -1238,    58,   940,
     899,   879, -1238,  1032,   858,  2811,  4184, 14944, 14944, 14944,
   14944, 14944,   860,   897,   863, 14944,     3,   901, -1238,   867,
   -1238,  4356, -1238,   368, -1238,  5852,  1962,   869, -1238, -1238,
    1114, -1238, -1238,  2975, -1238,  4356,  1044, 15413,   878, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,    96,  1114,
    1962,   881, 15130, 15320,  1054, -1238, -1238, -1238, -1238,   882,
   -1238, 13030, -1238, -1238,  5076, -1238,  4184,  1962,   883, -1238,
   -1238, -1238, -1238,  1061,   893, 13030, 15868, -1238, -1238,  4582,
     888, -1238,  4184, -1238,   894,  6046,  1050,    40, -1238, -1238,
     120,  1244, -1238,  1258, -1238,  4184, -1238, -1238,   247, 16240,
   -1238, 10508, -1238, 14944,   141,   900,  1962,   821, -1238, -1238,
   16401, 13030, -1238, -1238, 13030, -1238, 13030, -1238,  3339,   905,
   10120,   790,  1058,   821,  4184,  1081,   859,  1114, 15961,   247,
    3508,   909, -1238, -1238,   173,   913, -1238, -1238,  1085,  1776,
    1776,  2193, -1238, -1238, -1238,  1051,   916,    74,   919, -1238,
   -1238, -1238, -1238,  1099,   926,   696,   247,   247, 12836,   821,
    1258, -1238, -1238,  3781,   572,   180,  9732, -1238,  6240,   928,
    6434,   929, 14827, 15413,   932,   989,   247,  4356,  1118, -1238,
   -1238, -1238, -1238,   570, -1238,    30,  4184, -1238,  1000,  4184,
    1114,   543, -1238, -1238, -1238,  1128, -1238,   950,   969,   682,
     682,  1076,  1076, 15726,   951,  1137, 14944, 14633, 15868,  2176,
   14499, 14944, 14944, 14944, 14944, 14734, 14944, 14944, 14944, 14944,
   14944, 14944, 14944, 14944, 14944, 14944, 14944, 14944, 14944, 14944,
   14944, 14944, 14944, 14944, 14944, 14944, 14944, 14944, 14944,  1114,
   -1238, -1238,  1066, -1238, -1238, -1238, -1238, 14944,   247, -1238,
     526, -1238,   610,  1139, -1238, -1238,   137,   963,   247, 10702,
   -1238,  2102, -1238,  5270,   769,  1139, -1238,   498,    10, -1238,
   16240,  1020,   970, -1238,   978,  1050, -1238,  4184,   811,  4184,
      45,  1142,  1077,   205, -1238,   617,   291, -1238, -1238, 15413,
   13030, 16240,  4356,   981,   141, -1238,   980,   141,   972, 16401,
   16240, 15680,   988, 10120,   990,   984,  4184,   992,   994,  4184,
     821, -1238,   626,   396, 10120, 13030, -1238, -1238, -1238, -1238,
   -1238, -1238,  1043,   983,  1171,  1093,  2193,  1037, -1238, 15868,
    2193, -1238, -1238, -1238, 15413, 16240,  1005, -1238,   180,  1167,
    1126,  9732, -1238, -1238, -1238,  1015, 13030,   989,   247, 15130,
   14827,  1017, 14944,  6628,   577,  1023, 13030,   108,   420, -1238,
    1035,  4184, -1238,  1083, -1238,  3832,  1189,  1030, 14944, -1238,
   14944, -1238,  1036, -1238,  1090,  1216,  1039, -1238, -1238, -1238,
   15785,  1038,  1221, 11463,  3439, 16364, 14944, 16286, 16472, 10488,
   12233,  4717,  3198, 12813, 12813, 12813, 12813,  2132,  2132,  2132,
    2132,  2132,  1433,  1433,   682,   682,   682,  1076,  1076,  1076,
    1076, -1238,  1046, -1238,  4356,  1114,  4184,  4184, -1238,  1962,
     131, -1238, 15130, -1238, -1238, 14227,  1042, -1238,  1047,   623,
   -1238,   350, 13030, -1238, -1238, -1238, 13030, -1238, 13030, -1238,
     811, -1238, -1238,   123,  1224,  1158, 13030, -1238,  1064,   247,
   16240,  1050,  1053, -1238,  1070,   141, 13030, 10120,  1078, -1238,
   -1238,   769, -1238, -1238,  1065,  1075,  1092, -1238,  1084,  2193,
   -1238,  2193, -1238, -1238,  1094, -1238,  1147,  1095,  1275, -1238,
     247, -1238,  1255, -1238,  1111, -1238, -1238,  1101,  1116,   138,
   -1238, -1238,  4356,  1122,  1123, -1238,  4119, -1238, -1238, -1238,
   -1238, -1238, -1238,  4184, -1238,  4184, -1238,  4356, 15829, -1238,
   14944, 15868, -1238, -1238, 14944, -1238, 14944, -1238, 16437, 14944,
    1115,  6822,   610, -1238, -1238, -1238,   591, 13919,  1962,  1198,
   -1238,  1100,  1155,  1478, -1238, -1238, -1238,   772,  3018,   115,
     116,  1129,   769,  1127,   139, -1238, -1238, -1238,  1162,  3911,
    4020, 16240, -1238,   325,  1306,  1240, 13030, -1238, 16240, 10120,
    1212,  1050,  1536,  1050,  1140, 16240,  1145, -1238,  1787,  1150,
    1845, -1238, -1238,   141, -1238, -1238,  1194, -1238,  2193, -1238,
   15868, -1238,  1863, -1238,  8568, -1238, -1238, -1238, -1238,  9344,
   -1238, -1238, -1238,  8568, -1238,  1153, 14944,  4356,  1197,  4356,
   15887, 16437, -1238, -1238, -1238,  1962,  1962,  1114, -1238,  1321,
   14633,    75, -1238, 13919,   769,  1935, -1238,  1169, -1238,   118,
    1156,   119, -1238, 14226, -1238, -1238, -1238,   121, -1238, -1238,
    1028, -1238,  1154, -1238,  1264,   587, -1238, 14058, -1238, 14058,
   -1238, -1238,  1335,   772, -1238, 13502, -1238, -1238, -1238, -1238,
    1339,  1274, 13030, -1238, 16240,  1174,  1170,  1176,   534, -1238,
    1212,  1050, -1238, -1238, -1238, -1238,  1878,  1180,  2193, -1238,
    1236, -1238,  8568,  9538,  9344, -1238, -1238, -1238,  8568, -1238,
    4356, 14944, 14944,  7016,  1183,  1184, -1238, 14944, -1238,  1962,
   -1238, -1238, -1238, -1238, -1238,  4184,   986,  1100, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,   599,
   -1238,  1155, -1238, -1238, -1238, -1238, -1238,    88,   154, -1238,
    1360,   122, 14365,  1264,  1363, -1238,  4184,   587, -1238, -1238,
    1191,  1367, 13030, -1238, 16240, -1238,   134, -1238, -1238, -1238,
   -1238,  1193,   534, 13641, -1238,  1050, -1238,  2193, -1238, -1238,
   -1238, -1238,  7210,  4356,  4356, -1238, -1238, -1238,  4356, -1238,
     892,   135,  1375,  1195, -1238, -1238, 14944, 14226, 14226,  1336,
   -1238,  1028,  1028,   323, -1238, -1238, -1238, 14944,  1308, -1238,
    1222,  1207,   127, 14944, -1238,  1114, -1238, 14944, 16240,  1315,
   -1238,  1388,  7404,  7598, -1238, -1238, -1238,   534, -1238,  7792,
    1210,  1288, -1238,  1310,  1253, -1238, -1238,  1313,  4184, -1238,
     986, -1238, -1238,  4356, -1238, -1238,  1249, -1238,  1385, -1238,
   -1238, -1238, -1238,  4356,  1410,   526, -1238, -1238,  4356,  1238,
    4356, -1238,   492,  1243, -1238, -1238,  7986, -1238,  1237, -1238,
    1246,  1268,  1114,  1127,  1265, -1238, -1238, 14944,   144,   142,
   -1238,  1359, -1238, -1238, -1238, -1238,  1962,   869, -1238,  1280,
    1114,   574, -1238,  4356, -1238,  1266,  1432,   596,   142, -1238,
    1371, -1238,  1962,  1267, -1238,  1050,   143, -1238, -1238, -1238,
   -1238,  4184, -1238,  1270,  1271,   128, -1238,   583,   596,   176,
    1050,  1263, -1238, -1238, -1238, -1238,  4184,   416,  1441,  1380,
     583, -1238,  8180,   301,  1450,  1381, 13030, -1238, -1238,  8374,
   -1238,   441,  1451,  1387, 13030, -1238, 16240, -1238,  1455,  1389,
   13030, -1238, 16240, 13030, -1238, 16240, 16240
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1238, -1238, -1238,  -525, -1238, -1238, -1238,   165,    52,   -46,
     454,   915,     1,  1488, -1238,  2722, -1238,  -438, -1238,    23,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,
   -1238,  -236, -1238, -1238,  -147,   437,    18, -1238, -1238, -1238,
   -1238, -1238, -1238,    26, -1238, -1238, -1238, -1238, -1238, -1238,
      27, -1238, -1238,  1018,  1024,  1026,  -102,  -606,  -810,   586,
     641,  -233,   371,  -856, -1238,    47, -1238, -1238, -1238, -1238,
    -718,   226, -1238, -1238, -1238, -1238,  -215, -1238,  -582, -1238,
    -417, -1238, -1238,   942, -1238,    63, -1238, -1238,  -965, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,    36,
   -1238,   117, -1238, -1238, -1238, -1238, -1238,   -52, -1238,   203,
    -833, -1238, -1237,  -230, -1238,  -120,    87,  -119,  -221, -1238,
     -49, -1238, -1238, -1238,   212,   -38,     2,    19,  -679,   -62,
   -1238, -1238,   -14, -1238, -1238,    -5,   -51,    91, -1238, -1238,
   -1238, -1238, -1238, -1238, -1238, -1238, -1238,  -569,  -803, -1238,
   -1238, -1238, -1238, -1238,   960, -1238, -1238, -1238, -1238, -1238,
     475, -1238, -1238, -1238, -1238, -1238, -1238, -1238, -1238,  -809,
   -1238,  2300,    35, -1238,   611,  -389, -1238, -1238,  -462,  3434,
    3481, -1238, -1238,   544,  -122,  -618, -1238, -1238,   612,   430,
    -671,   431, -1238, -1238, -1238, -1238, -1238,   601, -1238, -1238,
   -1238,    17,  -789,  -155,  -403,  -393, -1238,   670,  -116, -1238,
   -1238,    38,    41,   558, -1238, -1238,   754,   -31, -1238,  -331,
      50,   505, -1238,  -101, -1238, -1238, -1238,  -446,  1187, -1238,
   -1238, -1238, -1238, -1238,   625,   604, -1238, -1238, -1238,  -317,
    -640, -1238,  1144,  -757, -1238,   -63,  -149,    57,   761, -1238,
    -895,   245,  -126,   521,   582, -1238, -1238, -1238, -1238,   535,
      53, -1020
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -958
static const yytype_int16 yytable[] =
{
     174,   176,   410,   178,   179,   180,   182,   183,   320,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   146,   116,   212,   215,   487,   457,   114,   371,   882,
     118,   119,   374,   375,   236,   479,   864,   239,   382,  1223,
     733,   327,   611,   747,   456,   247,   241,   250,   863,   384,
     328,   245,   331,  1051,   410,   682,   613,   406,   965,   337,
     905,   238,   381,   729,   730,   222,   932,   966,   227,   386,
     509,   228,   844,   722,   239,   513,   773,  1076,  1059,   759,
     229,  1209,   367,   723,  1469,   368,   400,    13,  -812,   521,
     951,   121,   504,  1087,   752,   383,  1127,  1621,   357,   817,
    -814,   558,   563,   -65,   -30,  1030,   319,   325,   -65,   -30,
     755,   775,   -29,   805,   568,   514,   521,   -29,   521,   384,
     756,   558,  1622,   558,  1417,  1419,  1224,  -330,  1477,  1116,
    1561,  1628,   381,  1136,  1137,  1306,  1628,  1469,  1277,   386,
     552,   939,   833,   346,  1658,   833,   833,   833,   833,    13,
     482,   498,  -643,  1030,   586,  -930,  1060,  -651,   496,  1333,
    1220,  1107,   396,   -92,   497,   383,   477,   507,  1215,   108,
    1639,   386,   490,   474,   475,   342,   397,   -92,    13,   444,
     506,   865,   343,  1624,  -650,  1154,     3,  -824,    13,    13,
      13,   445,   900,  -930,   -91,   598,   599,   383,   553,   177,
    1216,  1061,  1625,   587,  1334,  1626,  -508,  -822,   -91,   232,
    -812,  1225,  1747,   383,  -644,  1640,   248,   474,   475,   318,
     360,  1139,  -814,  1108,  -815,  -732,  -813,   516,  -732,  -826,
     516,  -855,   967,   458,  -645,   482,   352,   239,   527,  -819,
    1307,  -818,  -816,  -858,  1134,  1308,  1138,    60,    61,    62,
     164,  1309,   407,  1310,   369,   774,   352,  1748,  -857,   518,
     352,   352,   653,   523,  1470,  1471,   401,   538,  1342,   522,
    1031,   776,   483,  -799,  1270,  1348,  1623,  1350,   818,   548,
    1245,   559,   564,   -65,   -30,   352,  1062,  -269,   478,  1335,
    1311,  1312,   -29,  1313,   569,   819,  1362,  1278,   585,   574,
    1090,   808,  1073,   809,  1418,  1420,  1256,  -330,  1478,  1659,
    1562,  1629,   573,   536,   408,   537,  1677,  1742,  1714,   577,
    1043,   834,  1314,   948,   918,  1201,  1368,  1424,   950,  -269,
    -253,  -732,  -532,   337,   337,   557,  1340,  1761,   358,   410,
     651,   734,  1749,  -821,   239,   383,  -815,   686,  -813,  -534,
     494,   212,  1670,  -855,   320,  1645,   591,   483,  -825,  -800,
    -828,  -819,  -535,  -818,  -816,  -858,   596,   235,   572,   242,
     541,  1671,   243,   740,  1672,   182,   727,   319,   474,   475,
    -857,   731,  1762,   636,  -534,   329,  1036,  1446,   850,  -787,
     371,   699,   700,   406,   648,  -799,   622,   704,   338,   198,
     198,   108,  1328,  -787,  1208,   108,  1430,   361,   362,   528,
    1687,   244,   654,   655,   656,   658,   659,   660,   661,   662,
     663,   664,   665,   666,   667,   668,   669,   670,   671,   672,
     673,   674,   675,   676,   677,   678,   679,   680,   681,  1259,
     683,   115,   684,   684,   687,   705,  1438,  1267,  1440,   477,
     643,  -823,   319,   325,   706,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   718,  1763,   481,   871,
    1066,   873,   684,   728,   121,   648,   648,   684,   732,   222,
    1067,  -800,   227,  1084,   706,   228,   857,   736,  1378,  1222,
     547,  1431,  1232,   851,   229,  1234,   744,  1754,   746,   376,
     694,   348,   692,   481,   823,   762,   648,   372,   852,   339,
     484,   103,   474,   475,   763,   457,   764,   474,   475,  1092,
    1019,  1133,  1768,  1136,  1137,   611,   726,   358,  1701,   358,
     344,   560,   810,   456,  -790,   593,   358,   372,   345,   613,
    -536,   397,   593,   349,   899,   350,  1592,   767,  -790,   694,
    1389,   319,   108,   351,   854,   907,   813,  1450,   409,   337,
     751,   640,   150,   757,   358,   318,   355,   897,   352,  -788,
     389,   478,   779,  1702,   826,   827,   911,   843,  1355,   356,
    1356,   373,  1755,  -788,   208,   210,   871,   873,   396,   396,
     889,   778,   388,   940,   873,   588,   361,   362,   361,   362,
     383,  1349,   377,   474,   475,   361,   362,  1769,   378,   395,
     488,  1280,   396,  1616,   399,   547,   352,   697,   352,   352,
     352,   352,    36,   402,  -646,   945,   946,  1306,   411,  1617,
    1247,   780,   941,   361,   362,   622,  1472,   206,   206,   412,
     879,   721,   413,    48,   639,  1118,  1119,   509,  1618,   358,
     638,  1332,   890,  1344,  1020,   359,  1210,   611,   385,  -826,
    1574,   972,  1575,  -930,   547,   485,  1238,   860,   861,  1211,
      13,   613,  1135,  1136,  1137,   414,   869,  1248,   754,  1274,
    1136,  1137,  1422,  1269,   397,   415,   898,  1449,  1212,   108,
      60,    61,    62,   164,   165,   407,   168,   416,  -930,    84,
    1647,  -930,    86,    87,   447,    88,   169,    90,   803,  1196,
    1197,  1739,  1728,  1729,  1730,   910,   448,   360,   361,   362,
    1385,  1386,   458,  1588,  1589,   812,  1753,   449,   385,   391,
     393,   394,  1307,  1015,  1016,  1017,   450,  1308,   480,    60,
      61,    62,   164,  1309,   407,  1310,  -820,   358,   943,  1018,
    -644,   838,   840,   593,  1473,  1301,  1324,   408,  -533,   358,
     385,   365,   358,  1724,   239,   392,   493,   949,   593,   500,
     491,  1447,  1743,  1744,    36,   486,   508,  1596,   611,   445,
     209,   209,  1311,  1312,   499,  1313,   555,   556,  1668,  1669,
     397,    53,   613,  -824,   150,    48,   481,   960,   150,    60,
      61,    62,   164,   165,   407,   503,   408,   502,  1664,  1665,
    -642,  1111,   510,   511,  1327,   594,   361,   362,   352,   519,
    1346,  -957,   206,   532,   115,  1364,   539,   542,   361,   362,
    1737,   361,   362,   441,   442,   443,  1041,   444,   643,   643,
     543,  1373,   975,   978,  1091,  1750,   121,   567,   549,   445,
    1050,   365,   565,   566,    86,    87,  1147,    88,   169,    90,
     578,   146,   116,  1151,   579,   614,   408,   114,   615,   637,
     118,   119,   624,   625,   626,   628,  1071,   739,   612,  -104,
      53,   650,   588,   737,   622,   366,  1079,   741,   742,  1080,
     748,  1081,   121,   749,   702,   648,   765,   562,   521,  1044,
     769,   622,   929,   538,   935,   772,   570,   786,   575,  1052,
    1717,   692,   816,   582,   785,  1054,  1228,   820,   806,   830,
     592,   726,  1435,   757,   108,  1452,  1648,    36,  1068,  1717,
     821,   121,   824,  1115,  1458,   825,   832,  1738,   958,   108,
     835,   841,   846,   222,  1463,   150,   227,  1121,    48,   228,
     206,   847,   121,   849,  -666,   695,   855,  1088,   229,   206,
     611,   856,   858,   589,   859,   209,   206,   595,   862,  1122,
     108,    36,   866,   206,   613,  1025,   867,  1203,   875,   880,
     877,   695,  1252,   881,   610,   883,   204,   204,   886,   892,
     757,   893,    48,   589,  1032,   595,   589,   595,   595,   895,
     896,   904,   914,   912,   695,   404,   915,    86,    87,   108,
      88,   169,    90,   916,   888,   695,  -648,  1602,   695,  1140,
     942,   952,  1142,  1683,   547,  1204,   962,   964,   968,   611,
     108,   970,   969,  1292,  1205,   121,   721,   121,   754,  1153,
    1297,   971,  1159,   613,   582,   168,   973,   986,    84,   987,
     988,    86,    87,   990,    88,   169,    90,   991,  1027,  1023,
     146,   116,    33,    34,    35,  1230,   114,  1029,  1039,   118,
     119,  1035,   352,  1046,   199,  1048,  1040,  1053,   648,  1055,
    1057,  1049,   150,  1652,  1099,  1099,   929,  1074,  1086,   648,
    1205,   622,  1083,   209,   622,  1089,  1094,   206,  1727,  1095,
    -827,  1105,   209,  1106,   576,   754,  1109,    36,  1110,   209,
    1221,   108,   869,   108,  1112,   108,   209,  1124,  1126,  1129,
    1262,   239,  1130,    74,    75,    76,    77,    78,    48,  1395,
     121,  1276,  1132,  1141,   201,  1143,  1361,  1145,  1146,  1241,
      82,    83,  1244,  1018,  1265,  1149,  1150,  1192,  1199,  1642,
    1202,  1643,   547,  1218,    92,   547,  1226,   900,  1227,  1235,
    1649,    60,    61,    62,    63,    64,   407,  1219,    97,  1231,
    1233,   204,    70,   451,  1240,  1237,  1249,  1250,  1239,    36,
    1251,   925,  1242,  1243,   803,  1563,  1255,    86,    87,  1564,
      88,   169,    90,    36,  1282,  1261,   115,  1263,  1068,   410,
      48,  1264,   878,  1423,  1266,  1271,  1686,  1329,   108,  1281,
     453,  1330,  1275,  1331,    48,  1410,  1283,  1323,  1286,  1285,
     121,  1338,  1396,  1290,  1289,  1291,  1323,  1293,   408,  1295,
    1296,  1345,   648,  1325,  1300,  1397,  1398,  1326,  1336,  1337,
     209,  1341,   115,    60,    61,    62,   164,   165,   407,  1303,
    1304,  1339,   622,   168,  1351,   206,    84,  1399,  1343,    86,
      87,   909,    88,  1400,    90,  1352,  1347,    36,  1572,   836,
     837,   929,  1354,    86,    87,   929,    88,   169,    90,  1353,
    1358,   115,  1357,  1359,  1360,  1363,   108,  1318,    48,  1366,
     438,   439,   440,   441,   442,   443,  1318,   444,   108,   204,
    1365,   936,   115,   937,  1367,  1382,  1752,  1393,   204,   445,
     408,  1370,  1371,  1759,   206,   204,  1406,   150,  1421,  1426,
    1432,  1433,   204,    36,  1466,   198,  1436,  1448,  1441,   956,
    1461,  1434,   150,  1442,   648,  1467,  1374,    36,  1375,   198,
    1444,  1459,  1475,  1569,    48,  1570,  1476,    86,    87,  1576,
      88,   169,    90,  1582,   206,  1583,   206,  1586,    48,  1323,
    1302,   612,  1585,   150,   695,  1323,  1587,  1323,  1595,  1597,
     622,  1416,  1606,  1607,  1627,  1028,   695,  1633,   695,  1323,
    1636,  1637,   206,  1644,  1660,   115,  1662,   115,   121,  1674,
     582,  1038,  1456,  1666,  1676,  1675,  1681,  1682,   209,  1689,
    1690,   719,   150,    86,    87,  1692,    88,   169,    90,  -326,
     458,    36,  1693,  1696,   929,   719,   929,    86,    87,  1622,
      88,   169,    90,   150,  1697,  1700,  1705,  1584,   206,  1318,
    1635,  1703,    48,  1706,   720,  1318,   103,  1318,  1707,  1712,
    1719,  1726,  1468,   206,   206,   695,   204,  1722,   753,  1318,
     103,   121,  1734,  1751,  1725,  1756,  1736,   209,  1740,  1741,
     121,  1757,  1765,  1323,  1764,  1770,   108,   610,  1771,  1773,
    1774,  1721,   318,   811,   701,   696,  1599,  1456,  1411,   698,
     115,  1012,  1013,  1014,  1015,  1016,  1017,  1085,  1045,  1735,
    1661,    86,    87,   612,    88,   169,    90,   209,  1268,   209,
    1018,  1601,  1372,  1733,   150,   814,   150,  1593,   150,  1620,
     956,  1128,  1474,  1615,   203,   203,  1413,  1746,   219,   650,
    1758,  1394,  1213,   929,  1632,   209,  1630,    36,  1611,   108,
    1152,  1591,  1101,  1318,   108,  1253,  1113,  1254,   108,   121,
    1306,  1065,   219,   584,   649,   121,   976,  1384,    48,  1698,
     121,  1195,   352,  1144,  1191,   547,     0,    36,   318,     0,
       0,     0,     0,   206,   206,     0,     0,     0,  1558,     0,
     115,   209,  1679,  1710,     0,  1565,     0,  1638,    48,     0,
       0,     0,   318,    13,   318,     0,   209,   209,     0,     0,
     318,     0,   410,     0,     0,     0,     0,     0,     0,   610,
       0,   150,     0,  1409,   204,     0,     0,    86,    87,     0,
      88,   169,    90,   929,   612,     0,     0,   108,   108,   108,
       0,     0,     0,   108,   319,     0,     0,  1229,   108,  1634,
       0,     0,     0,     0,     0,   888,     0,    86,    87,     0,
      88,   169,    90,     0,     0,  1307,     0,     0,     0,     0,
    1308,     0,    60,    61,    62,   164,  1309,   407,  1310,     0,
       0,     0,     0,   204,     0,  1410,     0,     0,     0,     0,
       0,     0,  1260,     0,     0,     0,     0,     0,     0,   150,
     206,     0,     0,     0,     0,     0,     0,   582,   956,   121,
       0,   150,     0,     0,     0,  1311,  1312,     0,  1313,   203,
       0,     0,     0,   204,     0,   204,   209,   209,     0,     0,
       0,  1694,     0,     0,     0,     0,   622,     0,     0,   408,
     610,     0,     0,     0,     0,   206,     0,  1439,     0,   121,
     121,   204,     0,     0,     0,   622,   121,   547,   115,     0,
     206,   206,     0,   622,     0,    36,     0,     0,   219,     0,
     219,  1766,     0,     0,  1415,     0,     0,     0,   318,  1772,
     582,     0,   929,     0,     0,  1775,    48,   108,  1776,     0,
       0,     0,     0,   121,     0,  1653,     0,   204,     0,     0,
    1711,     0,  1558,  1558,   869,     0,  1565,  1565,     0,     0,
       0,  1306,   204,   204,     0,     0,   612,     0,     0,   869,
     352,   115,     0,     0,     0,   219,     0,   108,   108,     0,
     115,     0,     0,   206,   108,     0,     0,     0,     0,     0,
       0,     0,   334,   209,     0,    86,    87,   203,    88,   169,
      90,     0,     0,     0,    13,     0,   203,     0,     0,   121,
       0,     0,     0,   203,     0,     0,   121,     0,     0,  1306,
     203,   108,  1096,  1097,  1098,    36,     0,  1709,     0,   150,
    1577,   219,     0,     0,     0,   612,     0,  1306,   209,     0,
       0,     0,     0,     0,     0,  1723,    48,     0,     0,     0,
       0,     0,  1306,   209,   209,   219,     0,     0,   219,   115,
       0,     0,    13,     0,     0,   115,  1307,     0,     0,     0,
     115,  1308,   610,    60,    61,    62,   164,  1309,   407,  1310,
      13,     0,   204,   204,     0,     0,     0,   108,     0,     0,
       0,     0,   150,     0,   108,    13,     0,   150,     0,     0,
       0,   150,   219,     0,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,     0,     0,  1311,  1312,     0,  1313,
       0,     0,     0,     0,  1307,     0,   209,     0,     0,  1308,
       0,    60,    61,    62,   164,  1309,   407,  1310,     0,     0,
     408,   610,  1307,     0,   203,     0,     0,  1308,  1443,    60,
      61,    62,   164,  1309,   407,  1310,     0,  1307,     0,     0,
       0,     0,  1308,     0,    60,    61,    62,   164,  1309,   407,
    1310,     0,     0,     0,  1311,  1312,     0,  1313,     0,     0,
     150,   150,   150,     0,    36,     0,   150,     0,     0,     0,
       0,   150,  1311,  1312,     0,  1313,   219,   219,   408,   204,
     796,     0,     0,     0,     0,    48,  1445,  1311,  1312,   115,
    1313,    36,     0,     0,     0,     0,   408,     0,     0,     0,
       0,     0,     0,     0,  1451,   796,     0,  1396,     0,     0,
       0,   408,    48,     0,     0,     0,     0,     0,     0,  1594,
    1397,  1398,     0,     0,   204,     0,     0,     0,     0,   115,
     115,     0,     0,     0,     0,     0,   115,    36,   168,   204,
     204,    84,    85,     0,    86,    87,     0,    88,  1400,    90,
       0,     0,   219,   219,     0,     0,     0,     0,    48,     0,
       0,   219,   417,   418,   419,   168,   332,   333,    84,    85,
       0,    86,    87,   115,    88,   169,    90,     0,     0,     0,
       0,   420,   203,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,     0,     0,
     150,     0,   204,     0,   334,     0,     0,    86,    87,   445,
      88,   169,    90,  -958,  -958,  -958,  -958,  -958,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,   992,   993,   994,   115,
       0,   203,     0,     0,     0,     0,   115,     0,     0,  1018,
     150,   150,     0,     0,     0,   995,     0,   150,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
       0,   203,     0,   203,     0,    60,    61,    62,    63,    64,
     407,   919,   920,  1018,   150,     0,    70,   451,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
     796,   921,     0,     0,     0,     0,     0,     0,     0,   922,
     923,   924,    36,   219,   219,   796,   796,   796,   796,   796,
       0,   925,   452,   796,   453,     0,     0,     0,     0,     0,
       0,     0,     0,    48,   219,     0,  1206,   454,     0,   455,
       0,     0,   408,     0,     0,   203,     0,     0,     0,     0,
     150,     0,     0,     0,     0,     0,     0,   150,   219,     0,
     203,   203,     0,     0,     0,     0,   205,   205,   926,     0,
     221,     0,     0,     0,   219,   219,     0,     0,     0,     0,
       0,   927,     0,     0,   219,     0,     0,     0,     0,     0,
     219,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,     0,     0,   219,  1157,     0,     0,     0,     0,     0,
     928,   796,     0,     0,   219,     0,     0,     0,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,   219,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   445,     0,     0,     0,     0,
     203,   203,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,   219,     0,   219,
       0,   489,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   796,     0,   219,     0,     0,   796,
     796,   796,   796,   796,   796,   796,   796,   796,   796,   796,
     796,   796,   796,   796,   796,   796,   796,   796,   796,   796,
     796,   796,   796,   796,   796,   796,   796,     0,     0,     0,
       0,   205,   472,   473,     0,   796,     0,     0,     0,     0,
       0,     0,     0,     0,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   219,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   203,     0,   842,
     489,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,     0,   219,   472,   473,   219,   474,   475,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   219,   444,     0,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
     445,   472,   473,     0,     0,     0,     0,   203,   203,     0,
     796,     0,     0,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,   219,     0,   627,   796,     0,   796,   205,
       0,   474,   475,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,     0,   796,   205,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
     417,   418,   419,   205,     0,     0,     0,   474,   475,     0,
       0,     0,     0,     0,   219,   219,     0,   219,     0,   420,
     203,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,   417,   418,   419,   445,     0,     0,
       0,     0,     0,     0,   750,     0,     0,     0,     0,     0,
       0,     0,     0,   420,   221,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,   219,     0,   219,     0,     0,   321,     0,   796,   219,
       0,   445,   796,     0,   796,     0,   205,   796,   417,   418,
     419,     0,     0,     0,     0,   219,   219,     0,     0,   219,
       0,     0,     0,     0,     0,     0,   219,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
     260,     0,   799,     0,     0,   445,     0,     0,   219,     0,
       0,   874,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   796,     0,     0,   799,   262,     0,
       0,     0,     0,   219,   219,     0,     0,     0,     0,     0,
       0,   219,     0,   219,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,   417,   418,   419,     0,     0,
       0,     0,     0,   446,     0,   219,     0,   219,     0,     0,
       0,    48,     0,   219,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,   205,     0,   534,   535,     0,   796,
     796,     0,   445,     0,     0,   796,     0,   219,     0,     0,
       0,     0,     0,   219,   168,   219,     0,    84,   312,   913,
      86,    87,     0,    88,   169,    90,     0,   974,     0,     0,
       0,     0,   321,     0,   321,   417,   418,   419,   316,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   317,     0,
       0,     0,     0,   205,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,   445,   205,     0,   205,     0,   260,     0,     0,
     489,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,     0,   219,     0,     0,     0,     0,     0,
       0,   205,   799,     0,     0,   262,   917,     0,     0,     0,
       0,   219,     0,     0,     0,     0,     0,   799,   799,   799,
     799,   799,     0,     0,     0,   799,     0,    36,   219,     0,
       0,   472,   473,     0,   796,     0,  1022,     0,     0,     0,
       0,     0,     0,     0,     0,   796,     0,   205,    48,   321,
       0,   796,   321,     0,     0,   796,  -373,     0,     0,     0,
    1034,     0,   205,   205,    60,    61,    62,   164,   165,   407,
       0,     0,     0,     0,     0,     0,   219,  1034,     0,     0,
       0,     0,     0,   534,   535,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,   474,   475,     0,
       0,   168,     0,     0,    84,   312,     0,    86,    87,     0,
      88,   169,    90,   799,     0,   796,  1075,     0,     0,     0,
       0,     0,     0,     0,   219,   316,     0,     0,     0,     0,
       0,   408,     0,     0,     0,   317,     0,     0,   221,     0,
     219,     0,     0,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,   822,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,   205,   205,     0,     0,     0,     0,     0,     0,
     321,   781,     0,     0,   797,  1018,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   799,     0,   205,   797,
       0,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,     0,
       0,     0,     0,     0,     0,     0,     0,   799,     0,     0,
       0,     0,     0,     0,     0,     0,   321,   321,     0,     0,
       0,     0,     0,     0,     0,   321,     0,     0,     0,   417,
     418,   419,   489,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,     0,     0,     0,   420,   205,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,     0,
       0,     0,     0,   472,   473,     0,   445,     0,     0,   205,
       0,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   205,
     205,     0,   799,  -958,  -958,  -958,  -958,  -958,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,   799,     0,
     799,   994,     0,     0,     0,     0,     0,     0,     0,   445,
     207,   207,     0,     0,   225,     0,   799,     0,   995,   474,
     475,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,     0,   797,     0,     0,     0,     0,  1305,
       0,     0,   205,     0,     0,     0,  1018,   321,   321,   797,
     797,   797,   797,   797,     0,     0,     0,   797,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1082,     0,     0,     0,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,   321,     0,
       0,    33,    34,    35,    36,   445,   198,     0,     0,     0,
       0,     0,     0,   199,   321,     0,     0,     0,     0,     0,
     799,   205,     0,     0,   799,    48,   799,   321,     0,   799,
       0,     0,     0,     0,     0,   797,     0,     0,  1392,     0,
       0,  1405,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,   217,     0,     0,     0,   321,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   201,     0,   207,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
     205,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   799,    97,     0,     0,
       0,     0,   218,     0,     0,  1464,  1465,   103,   321,     0,
       0,   321,     0,   781,     0,  1405,     0,     0,     0,  1093,
       0,     0,     0,     0,     0,     0,     0,     0,   797,     0,
       0,     0,     0,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,     0,     0,     0,     0,     0,     0,     0,     0,   797,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   799,   799,     0,     0,     0,     0,   799,     0,  1609,
       0,     0,     0,   207,     0,     0,     0,  1405,     0,   321,
       0,   321,   207,     0,     0,     0,     0,     0,     0,   207,
       0,   417,   418,   419,     0,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   225,   321,     0,
     420,   321,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
       0,     0,     0,     0,   797,     0,     0,     0,     0,     0,
       0,   260,     0,   321,     0,     0,     0,   321,     0,     0,
     797,     0,   797,     0,     0,     0,     0,     0,   225,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   797,   262,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,    36,   444,     0,     0,     0,   799,     0,   321,   321,
     207,   417,   418,   419,   445,     0,     0,   799,     0,     0,
       0,     0,    48,   799,     0,     0,     0,   799,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,   534,   535,     0,
       0,     0,  1117,     0,     0,     0,   800,     0,   445,     0,
       0,     0,     0,     0,     0,   168,     0,     0,    84,   312,
       0,    86,    87,     0,    88,   169,    90,   799,  1284,     0,
       0,   800,     0,     0,     0,   321,  1720,   321,     0,   316,
       0,     0,   797,     0,     0,     0,   797,     0,   797,   317,
       0,   797,  1392,   801,     0,     0,     0,     0,     0,   321,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
     321,     0,     0,     0,     0,     0,     0,     0,   815,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,   207,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   797,     0,
       0,     0,  1428,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   321,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   417,
     418,   419,     0,     0,     0,     0,     0,   207,     0,   321,
       0,   321,     0,     0,     0,     0,     0,   321,   420,  1277,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,   260,   207,     0,   207,
       0,     0,     0,   797,   797,     0,   445,     0,     0,   797,
       0,     0,     0,     0,     0,     0,     0,   321,     0,     0,
       0,     0,     0,     0,   262,   207,   800,     0,     0,     0,
       0,  1429,     0,   260,     0,     0,     0,     0,     0,     0,
       0,   800,   800,   800,   800,   800,    36,     0,     0,   800,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,     0,     0,     0,     0,     0,    48,     0,     0,
       0,   207,     0,   957,     0,   540,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,   207,   207,   979,   980,
     981,   982,     0,     0,     0,     0,   989,     0,     0,     0,
       0,     0,   534,   535,    48,     0,     0,     0,     0,     0,
     225,     0,     0,     0,     0,     0,     0,     0,   321,     0,
     168,     0,     0,    84,   312,     0,    86,    87,  1278,    88,
     169,    90,     0,     0,     0,   321,     0,   800,     0,   534,
     535,     0,     0,     0,   316,     0,     0,     0,     0,     0,
       0,     0,  1654,     0,   317,     0,     0,   168,   797,     0,
      84,   312,   225,    86,    87,     0,    88,   169,    90,   797,
       0,     0,     0,     0,     0,   797,     0,     0,     0,   797,
       0,   316,     0,     0,  1072,     0,   992,   993,   994,     0,
       0,   317,     0,     0,     0,     0,     0,     0,     0,     0,
     321,     0,     0,     0,     0,   995,   207,   207,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   797,
     800,     0,   225,  1018,     0,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   321,     0,     0,     0,     0,     0,     0,
       0,   800,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,  1160,  1163,  1164,  1165,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,
     418,   419,     0,   207,     0,     0,     0,     0,  1194,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   225,   444,     0,     0,     0,   207,     0,
       0,     0,     0,     0,     0,     0,   445,     0,     0,     0,
       0,     0,     0,   207,   207,     0,   800,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   800,     0,   800,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,     0,     0,     0,     0,
     800,     0,     0,     0,     0,     0,     0,     0,     0,   259,
       0,     0,     0,  1272,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1287,
       0,  1288,     0,     0,     0,     0,   207,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1298,     0,     0,
       0,   263,   264,   265,   266,   267,   268,   269,     0,     0,
       0,    36,     0,   198,     0,     0,     0,     0,     0,     0,
       0,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,    48,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,     0,     0,     0,   690,
     305,   306,   307,     0,     0,     0,   308,   544,   545,     0,
       0,     0,     0,     0,   800,   225,     0,     0,   800,     0,
     800,     0,     0,   800,     0,   546,     0,     0,     0,     0,
       0,    86,    87,     0,    88,   169,    90,   313,     0,   314,
       0,     0,   315,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1377,   691,     0,   103,  1379,     0,  1380,     0,     0,
    1381,     0,     0,     0,  1018,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     800,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   380,
      12,     0,     0,     0,     0,     0,     0,     0,   703,     0,
       0,     0,     0,     0,     0,     0,     0,  1460,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,   800,   800,     0,     0,     0,
      41,   800,     0,     0,     0,     0,     0,     0,     0,     0,
    1614,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,  1603,  1604,     0,     0,   167,    73,  1608,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,   417,   418,   419,    97,    98,    99,     0,     0,   100,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
     800,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   800,     0,     0,     0,     0,     0,   800,     0,     0,
       0,   800,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,  1695,     0,     0,  1663,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,  1673,     0,
       0,     0,     0,     0,  1678,     0,     0,     0,  1680,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,   800,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
     531,    46,     0,    47,     0,     0,    48,    49,  1713,     0,
       0,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,    81,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,    95,     0,    96,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1042,   103,   104,
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
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,    81,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,    95,     0,
      96,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1207,   103,   104,     0,   105,   106,     5,     6,     7,
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
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,    81,    82,    83,
      84,    85,     0,    86,    87,     0,    88,    89,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,    95,     0,    96,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,     0,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,   629,
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
       0,   101,   102,  1021,   103,   104,     0,   105,   106,     5,
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
      99,     0,     0,   100,     0,   101,   102,  1056,   103,   104,
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
     102,  1123,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,  1125,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,  1273,     0,    48,    49,
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
       0,   101,   102,  1383,   103,   104,     0,   105,   106,     5,
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
      99,     0,     0,   100,     0,   101,   102,  1605,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,  1650,    47,     0,     0,
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
       0,   100,     0,   101,   102,  1684,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1685,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,  1688,    46,     0,    47,
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
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1704,   103,   104,
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
     102,  1760,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,   100,     0,   101,   102,  1767,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   517,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,     0,   766,     0,     0,     0,
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
     959,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,    12,     0,  1455,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,  1598,     0,
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
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
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
      99,     0,     0,   170,     0,   326,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,   644,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,    14,    15,
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
      87,     0,    88,   169,    90,     0,   645,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   170,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,   170,     0,     0,   761,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,     0,     0,  1069,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1018,    14,    15,     0,     0,
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
      88,   169,    90,     0,  1070,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   380,
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
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   417,   418,   419,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   445,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,   181,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   164,   165,   166,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     167,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
     533,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,   211,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   445,     0,    14,    15,
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
       0,     0,    97,    98,    99,     0,     0,   170,     0,   417,
     418,   419,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   445,     0,    16,     0,
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
       0,     0,    92,     0,     0,    93,     0,     0,   550,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,   246,   993,   994,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   995,     0,     0,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
    1018,     0,    16,     0,    17,    18,    19,    20,    21,    22,
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
      97,    98,    99,     0,     0,   170,     0,   249,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   380,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   417,   418,   419,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   445,     0,
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
     554,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,   515,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     657,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    97,    98,    99,     0,     0,   170,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,     0,     0,     0,
     703,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1018,     0,    14,    15,     0,     0,     0,     0,    16,     0,
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
       0,   170,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,   743,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,    14,    15,     0,     0,
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
       0,     0,     0,     0,     0,    10,  -958,  -958,  -958,  -958,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,   745,     0,
       0,     0,     0,     0,     0,     0,   445,     0,     0,     0,
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
    -958,  -958,  -958,  -958,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,     0,     0,     0,
       0,     0,  1114,     0,     0,     0,     0,     0,     0,     0,
    1018,     0,     0,     0,    14,    15,     0,     0,     0,     0,
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
      99,     0,     0,   170,     0,   417,   418,   419,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   445,     0,    16,     0,    17,    18,    19,    20,
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
       0,    93,     0,   735,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   170,     0,   417,
     418,   419,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   445,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,   590,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,   251,   252,     0,   253,
     254,     0,    80,   255,   256,   257,   258,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
     259,     0,    92,     0,     0,    93,     0,   758,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,     0,     0,     0,   103,   104,   261,   105,
     106,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,    36,     0,   198,     0,     0,     0,     0,     0,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,    48,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,     0,     0,     0,
     304,   305,   306,   307,     0,     0,     0,   308,   544,   545,
       0,     0,     0,     0,     0,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,   546,     0,     0,     0,
       0,     0,    86,    87,     0,    88,   169,    90,   313,   259,
     314,   260,     0,   315,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   691,     0,   103,     0,   261,     0,   262,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   263,   264,   265,   266,   267,   268,   269,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,    48,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,     0,     0,     0,     0,
     305,   306,   307,     0,     0,     0,   308,   309,   310,     0,
       0,     0,     0,     0,   251,   252,     0,   253,   254,     0,
       0,   255,   256,   257,   258,   311,     0,     0,    84,   312,
       0,    86,    87,     0,    88,   169,    90,   313,   259,   314,
     260,     0,   315,     0,     0,     0,     0,     0,     0,   316,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   317,
       0,     0,     0,  1578,     0,     0,   261,     0,   262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,     0,   305,
     306,   307,     0,     0,     0,   308,   309,   310,     0,     0,
       0,     0,     0,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,   311,     0,     0,    84,   312,     0,
      86,    87,     0,    88,   169,    90,   313,   259,   314,   260,
       0,   315,     0,     0,     0,     0,     0,     0,   316,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   317,     0,
       0,     0,  1646,     0,     0,   261,     0,   262,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   263,
     264,   265,   266,   267,   268,   269,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
      48,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,     0,     0,     0,   304,   305,   306,
     307,     0,     0,     0,   308,   309,   310,     0,     0,     0,
       0,     0,   251,   252,     0,   253,   254,     0,     0,   255,
     256,   257,   258,   311,     0,     0,    84,   312,     0,    86,
      87,     0,    88,   169,    90,   313,   259,   314,   260,     0,
     315,     0,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   317,     0,     0,
       0,     0,     0,     0,   261,     0,   262,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   316,  1387,     0,     0,
       0,     0,     0,     0,     0,     0,   317,     0,     0,     0,
       0,     0,     0,   261,     0,   262,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   263,   264,   265,
     266,   267,   268,   269,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,    48,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,     0,     0,     0,     0,   305,   306,   307,     0,
       0,     0,   308,   309,   310,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   311,     0,     0,    84,   312,     0,    86,    87,     0,
      88,   169,    90,   313,     0,   314,     0,     0,   315,  1479,
    1480,  1481,  1482,  1483,     0,   316,  1484,  1485,  1486,  1487,
       0,     0,     0,     0,     0,   317,     0,     0,     0,     0,
       0,     0,     0,  1488,  1489,     0,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,  1490,   444,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   445,  1491,  1492,  1493,  1494,  1495,
    1496,  1497,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1498,  1499,  1500,  1501,  1502,
    1503,  1504,  1505,  1506,  1507,  1508,    48,  1509,  1510,  1511,
    1512,  1513,  1514,  1515,  1516,  1517,  1518,  1519,  1520,  1521,
    1522,  1523,  1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,
    1532,  1533,  1534,  1535,  1536,  1537,  1538,     0,     0,     0,
    1539,  1540,     0,  1541,  1542,  1543,  1544,  1545,   251,   252,
       0,   253,   254,     0,     0,   255,   256,   257,   258,  1546,
    1547,  1548,     0,     0,     0,    86,    87,     0,    88,   169,
      90,  1549,   259,  1550,  1551,     0,  1552,     0,     0,     0,
       0,     0,     0,  1553,  1554,     0,  1555,     0,  1556,  1557,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,   264,   265,   266,   267,   268,
     269,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,    48,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,     0,
       0,     0,   304,   305,   306,   307,     0,     0,     0,   308,
     544,   545,   251,   252,     0,   253,   254,     0,     0,   255,
     256,   257,   258,     0,     0,     0,     0,     0,   546,     0,
       0,     0,     0,     0,    86,    87,   259,    88,   169,    90,
     313,     0,   314,     0,     0,   315,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,   264,
     265,   266,   267,   268,   269,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,    48,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,     0,     0,     0,  1158,   305,   306,   307,
       0,     0,     0,   308,   544,   545,   251,   252,     0,   253,
     254,     0,     0,   255,   256,   257,   258,     0,     0,     0,
       0,     0,   546,     0,     0,     0,     0,     0,    86,    87,
     259,    88,   169,    90,   313,     0,   314,     0,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   261,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,    48,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,     0,     0,     0,
       0,   305,   306,   307,  1166,     0,     0,   308,   544,   545,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   787,   788,     0,     0,   546,     0,   789,     0,
     790,     0,    86,    87,     0,    88,   169,    90,   313,     0,
     314,     0,   791,   315,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,     0,     0,     0,     0,     0,   417,
     418,   419,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   953,   444,     0,     0,     0,     0,   792,
       0,    74,    75,    76,    77,    78,   445,     0,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   168,    82,    83,
      84,   793,     0,    86,    87,    28,    88,   169,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   198,     0,
       0,   794,     0,     0,     0,   199,    97,     0,     0,     0,
       0,   795,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   492,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   954,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,   787,   788,     0,    92,     0,     0,   789,     0,
     790,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,   791,     0,   202,     0,     0,     0,     0,   103,
      33,    34,    35,    36,     0,     0,     0,     0,     0,   417,
     418,   419,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,   792,
       0,    74,    75,    76,    77,    78,   445,     0,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   168,    82,    83,
      84,   793,     0,    86,    87,    28,    88,   169,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   198,     0,
       0,   794,     0,     0,     0,   199,    97,     0,     0,     0,
       0,   795,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   501,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,    28,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,     0,     0,     0,     0,   199,    97,
       0,     0,     0,     0,   202,     0,     0,   561,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   581,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    28,     0,   908,    92,     0,
       0,     0,     0,    33,    34,    35,    36,     0,   198,     0,
       0,     0,    97,     0,     0,   199,     0,   202,     0,     0,
       0,     0,   103,     0,     0,     0,     0,    48,     0,     0,
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
       0,     0,     0,     0,  1037,    73,     0,    74,    75,    76,
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
       0,    92,     0,     0,     0,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     202,     0,     0,     0,   420,   103,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   417,   418,   419,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   894,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,   992,   993,   994,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,   938,   995,     0,     0,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1018,     0,   992,   993,   994,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1236,   995,     0,     0,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,   992,
     993,   994,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1018,     0,     0,     0,     0,     0,   995,  1148,
       0,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1018,   992,   993,   994,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   995,     0,  1294,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,     0,     0,     0,    33,    34,    35,    36,     0,   198,
       0,     0,     0,     0,  1018,     0,   604,     0,     0,     0,
       0,     0,  1376,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   201,     0,     0,     0,
    1462,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,    33,    34,    35,
      36,     0,   198,     0,     0,     0,     0,     0,     0,   199,
      97,     0,     0,     0,     0,   605,     0,     0,     0,     0,
     606,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   201,
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,     0,     0,     0,    92,
       0,     0,     0,   417,   418,   419,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   218,     0,
       0,   770,   420,   103,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   771,   420,   891,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,   445,     0,     0,     0,     0,     0,     0,     0,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,   992,   993,   994,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,   995,  1299,     0,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1018,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   995,     0,   445,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1018,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1018,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1018
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,    54,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     4,     4,    28,    29,   172,   146,     4,    91,   647,
       4,     4,    95,    96,    32,   151,   618,    42,   100,  1059,
     486,    55,   373,   505,   146,    50,    44,    52,   617,   100,
      55,    49,    57,   856,   173,   444,   373,   120,   776,    58,
     700,    42,   100,   480,   481,    30,   737,     9,    30,   100,
     219,    30,   597,   476,    79,   230,     9,   887,   867,   517,
      30,  1046,    81,   476,     9,    84,     9,    47,    68,     9,
     769,     4,   214,   903,   511,   100,   952,     9,    79,     9,
      68,     9,     9,     9,     9,     9,    54,    54,    14,    14,
     513,    30,     9,   551,     9,   231,     9,    14,     9,   170,
     513,     9,    34,     9,     9,     9,    81,     9,     9,   939,
       9,     9,   170,   103,   104,     4,     9,     9,    30,   170,
     112,   747,     9,    81,     9,     9,     9,     9,     9,    47,
      68,   202,   152,     9,    68,   152,    36,   152,    87,    36,
    1055,    87,   156,   173,   202,   170,    68,   218,   158,     4,
      36,   202,   177,   131,   132,   120,   173,   187,    47,    55,
     218,   619,   127,    29,   152,   988,     0,   187,    47,    47,
      47,    67,   187,   190,   173,   189,   190,   202,   170,   187,
     190,    81,    48,   358,    81,    51,     8,   187,   187,   187,
     190,   166,    36,   218,   152,    81,    51,   131,   132,    54,
     149,   191,   190,   149,    68,   185,    68,   232,   188,   187,
     235,    68,   174,   146,   152,    68,    71,   242,   243,    68,
     109,    68,    68,    68,   962,   114,   964,   116,   117,   118,
     119,   120,   121,   122,    89,   188,    91,    81,    68,   236,
      95,    96,   409,   240,   189,   190,   189,   173,  1233,   189,
     174,   190,   190,    68,  1130,  1240,   188,  1242,   188,   325,
    1090,   189,   189,   189,   189,   120,   166,   185,   190,   166,
     159,   160,   189,   162,   189,   188,  1261,   189,   189,   350,
     906,   189,   884,   189,   189,   189,  1109,   189,   189,   174,
     189,   189,   350,   260,   183,   262,   189,   189,   174,   350,
     845,   188,   191,   761,   188,   188,   188,   188,   766,   188,
     188,   188,    68,   332,   333,   334,  1231,    36,    81,   458,
     403,   488,   166,   187,   349,   350,   190,   448,   190,    68,
     185,   356,    29,   190,   400,  1592,   361,   190,   187,    68,
     187,   190,    68,   190,   190,   190,   365,   187,   349,   187,
     317,    48,   187,   495,    51,   380,   477,   325,   131,   132,
     190,   482,    81,   388,    68,   190,   832,  1352,    52,   173,
     453,   454,   455,   456,   399,   190,   379,   459,    81,    81,
      81,   236,    52,   187,  1044,   240,    81,   150,   151,   244,
    1647,   187,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,  1110,
     445,     4,   447,   448,   449,   459,  1341,  1126,  1343,    68,
     397,   187,   400,   400,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   166,   187,   624,
     873,   626,   477,   478,   387,   480,   481,   482,   483,   444,
     873,   190,   444,   900,   489,   444,   608,   492,  1291,  1058,
     325,   166,  1074,   157,   444,  1077,   501,    81,   503,    81,
     450,   187,   450,   187,   188,   519,   511,   157,   172,   192,
     192,   192,   131,   132,   519,   635,   521,   131,   132,   908,
     152,   959,    81,   103,   104,   856,   476,    81,    36,    81,
     119,    99,    99,   635,   173,    87,    81,   157,   127,   856,
      68,   173,    87,   187,   693,   187,  1441,   524,   187,   499,
    1307,   499,   387,   187,   605,   702,   561,  1360,   121,   558,
     510,   396,     4,   513,    81,   400,   187,   689,   403,   173,
      87,   190,    29,    81,    48,    49,   725,   191,  1249,   187,
    1251,   187,   166,   187,    26,    27,   741,   742,   156,   156,
     653,   538,   190,   748,   749,   149,   150,   151,   150,   151,
     605,  1241,   184,   131,   132,   150,   151,   166,   190,    30,
     173,   191,   156,    14,   187,   450,   451,   452,   453,   454,
     455,   456,    79,    36,   152,    73,    74,     4,   189,    30,
    1092,    88,   748,   150,   151,   618,  1393,    26,    27,   189,
     645,   476,   189,   100,   196,    73,    74,   796,    49,    81,
     195,  1220,   657,  1235,   803,    87,   158,   988,   100,   187,
    1417,   783,  1419,   152,   499,   160,  1083,   614,   615,   171,
      47,   988,   102,   103,   104,   189,   623,  1094,   513,   102,
     103,   104,  1322,  1129,   173,   189,   691,  1358,   190,   524,
     116,   117,   118,   119,   120,   121,   153,   189,   187,   156,
    1595,   190,   159,   160,    68,   162,   163,   164,   543,    99,
     100,  1731,   116,   117,   118,   720,    68,   149,   150,   151,
     129,   130,   635,   189,   190,   560,  1746,   190,   170,   104,
     105,   106,   109,    51,    52,    53,   152,   114,   187,   116,
     117,   118,   119,   120,   121,   122,   187,    81,   753,    67,
     152,   586,   587,    87,  1394,  1193,  1202,   183,    68,    81,
     202,   156,    81,   189,   769,    87,    46,   765,    87,   211,
     189,  1353,   189,   190,    79,   187,   218,  1448,  1109,    67,
      26,    27,   159,   160,   152,   162,   332,   333,  1621,  1622,
     173,   108,  1109,   187,   236,   100,   187,   774,   240,   116,
     117,   118,   119,   120,   121,     9,   183,   194,  1617,  1618,
     152,   933,   152,   187,   191,   149,   150,   151,   653,     8,
    1237,   152,   211,   189,   387,  1263,   187,    14,   150,   151,
    1725,   150,   151,    51,    52,    53,   841,    55,   785,   786,
     152,  1279,   785,   786,   907,  1740,   759,    14,   189,    67,
     855,   156,   127,   127,   159,   160,   978,   162,   163,   164,
     188,   844,   844,   985,   173,    14,   183,   844,    99,   193,
     844,   844,   188,   188,   188,   188,   881,     9,   373,   187,
     108,   187,   149,   187,   867,   190,   891,   188,   188,   894,
     188,   896,   805,   188,   457,   900,    91,   339,     9,   846,
     189,   884,   737,   173,   739,    14,   348,     9,   350,   859,
    1699,   859,    81,   355,   187,   862,  1065,   188,   187,   129,
     362,   871,  1339,   873,   759,  1363,  1597,    79,   875,  1718,
     188,   844,   188,   938,  1372,   189,   187,  1726,   773,   774,
     188,    68,    30,   908,  1382,   387,   908,   945,   100,   908,
     339,   130,   865,   172,   152,   450,   133,   904,   908,   348,
    1291,     9,   188,   359,   152,   211,   355,   363,    14,   946,
     805,    79,   185,   362,  1291,   810,     9,  1039,     9,   188,
     174,   476,  1104,     9,   373,    14,    26,    27,   129,   194,
     940,   194,   100,   389,   829,   391,   392,   393,   394,   191,
       9,    14,   188,   194,   499,   157,   188,   159,   160,   844,
     162,   163,   164,   194,   187,   510,   152,  1455,   513,   966,
     188,    99,   969,  1641,   859,  1039,   189,   189,    88,  1360,
     865,   152,   133,  1155,  1039,   948,   871,   950,   873,   987,
    1162,     9,   990,  1360,   486,   153,   188,   187,   156,   152,
     187,   159,   160,   152,   162,   163,   164,   190,    14,   190,
    1043,  1043,    76,    77,    78,  1070,  1043,   189,    14,  1043,
    1043,   190,   907,   190,    88,    14,   194,   189,  1083,   185,
      30,   188,   524,   191,   919,   920,   921,   187,    30,  1094,
    1095,  1074,   187,   339,  1077,    14,   187,   486,  1716,    14,
     187,    50,   348,   187,   350,   940,   187,    79,     9,   355,
    1057,   946,  1059,   948,   188,   950,   362,   189,   189,   187,
    1118,  1126,   133,   137,   138,   139,   140,   141,   100,    29,
    1043,  1136,    14,   133,   148,   970,  1258,     9,   188,  1086,
     154,   155,  1089,    67,  1121,   194,     9,    81,     9,  1587,
     187,  1589,   987,   133,   168,   990,    14,   187,    81,   187,
    1598,   116,   117,   118,   119,   120,   121,   189,   182,   188,
     190,   211,   127,   128,   190,   187,   133,   194,   188,    79,
       9,    88,   190,   189,  1019,   157,   149,   159,   160,   161,
     162,   163,   164,    79,  1141,   190,   759,    30,  1145,  1318,
     100,    75,   644,  1323,   189,   188,  1644,  1212,  1043,   174,
     165,  1216,   189,  1218,   100,   187,   133,  1200,   188,    30,
    1133,  1226,   122,   133,   188,     9,  1209,   188,   183,   191,
       9,  1236,  1237,   191,   188,   135,   136,   190,    14,    81,
     486,   188,   805,   116,   117,   118,   119,   120,   121,  1196,
    1197,   187,  1235,   153,   189,   644,   156,   157,   188,   159,
     160,   703,   162,   163,   164,   190,   188,    79,  1415,    81,
      82,  1106,   188,   159,   160,  1110,   162,   163,   164,   187,
     133,   844,   188,   188,     9,    30,  1121,  1200,   100,   188,
      48,    49,    50,    51,    52,    53,  1209,    55,  1133,   339,
     189,   743,   865,   745,   188,   190,  1744,   109,   348,    67,
     183,   189,   189,  1751,   703,   355,   161,   759,   189,   157,
      14,    81,   362,    79,  1387,    81,   114,   133,   188,   771,
     133,  1336,   774,   188,  1339,    14,  1283,    79,  1285,    81,
     190,   188,   173,   189,   100,    81,   190,   159,   160,    14,
     162,   163,   164,    14,   743,    81,   745,   187,   100,  1342,
    1195,   856,   188,   805,   859,  1348,   190,  1350,   188,   133,
    1353,  1318,   189,   189,    14,   817,   871,    14,   873,  1362,
     189,    14,   771,   190,     9,   948,   191,   950,  1301,    81,
     832,   833,  1369,    57,   187,   173,    81,     9,   644,   189,
     112,   157,   844,   159,   160,   152,   162,   163,   164,    99,
    1323,    79,    99,   164,  1249,   157,  1251,   159,   160,    34,
     162,   163,   164,   865,    14,   187,   189,  1432,   817,  1342,
    1577,   188,   100,   187,   190,  1348,   192,  1350,   170,   174,
      81,     9,  1390,   832,   833,   940,   486,   167,   190,  1362,
     192,  1364,    81,   190,   188,    14,   189,   703,   188,   188,
    1373,    81,    81,  1446,    14,    14,  1301,   856,    81,    14,
      81,  1707,  1307,   558,   456,   451,  1453,  1454,  1313,   453,
    1043,    48,    49,    50,    51,    52,    53,   901,   847,  1722,
    1612,   159,   160,   988,   162,   163,   164,   743,  1127,   745,
      67,  1454,  1276,  1718,   946,   563,   948,  1444,   950,  1561,
     952,   953,  1395,  1477,    26,    27,  1313,  1738,    30,   187,
    1750,  1309,  1047,  1358,  1573,   771,  1572,    79,  1475,  1364,
     986,  1440,   920,  1446,  1369,  1105,   935,  1106,  1373,  1452,
       4,   871,    54,   356,   400,  1458,   785,  1302,   100,  1675,
    1463,  1030,  1387,   971,  1019,  1390,    -1,    79,  1393,    -1,
      -1,    -1,    -1,   952,   953,    -1,    -1,    -1,  1403,    -1,
    1133,   817,  1635,  1693,    -1,  1410,    -1,  1582,   100,    -1,
      -1,    -1,  1417,    47,  1419,    -1,   832,   833,    -1,    -1,
    1425,    -1,  1711,    -1,    -1,    -1,    -1,    -1,    -1,   988,
      -1,  1043,    -1,   125,   644,    -1,    -1,   159,   160,    -1,
     162,   163,   164,  1448,  1109,    -1,    -1,  1452,  1453,  1454,
      -1,    -1,    -1,  1458,  1572,    -1,    -1,  1069,  1463,  1576,
      -1,    -1,    -1,    -1,    -1,   187,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,   109,    -1,    -1,    -1,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   703,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,  1114,    -1,    -1,    -1,    -1,    -1,    -1,  1121,
    1069,    -1,    -1,    -1,    -1,    -1,    -1,  1129,  1130,  1602,
      -1,  1133,    -1,    -1,    -1,   159,   160,    -1,   162,   211,
      -1,    -1,    -1,   743,    -1,   745,   952,   953,    -1,    -1,
      -1,  1658,    -1,    -1,    -1,    -1,  1699,    -1,    -1,   183,
    1109,    -1,    -1,    -1,    -1,  1114,    -1,   191,    -1,  1642,
    1643,   771,    -1,    -1,    -1,  1718,  1649,  1572,  1301,    -1,
    1129,  1130,    -1,  1726,    -1,    79,    -1,    -1,   260,    -1,
     262,  1756,    -1,    -1,  1317,    -1,    -1,    -1,  1593,  1764,
    1202,    -1,  1597,    -1,    -1,  1770,   100,  1602,  1773,    -1,
      -1,    -1,    -1,  1686,    -1,  1610,    -1,   817,    -1,    -1,
    1693,    -1,  1617,  1618,  1731,    -1,  1621,  1622,    -1,    -1,
      -1,     4,   832,   833,    -1,    -1,  1291,    -1,    -1,  1746,
    1635,  1364,    -1,    -1,    -1,   317,    -1,  1642,  1643,    -1,
    1373,    -1,    -1,  1202,  1649,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   156,  1069,    -1,   159,   160,   339,   162,   163,
     164,    -1,    -1,    -1,    47,    -1,   348,    -1,    -1,  1752,
      -1,    -1,    -1,   355,    -1,    -1,  1759,    -1,    -1,     4,
     362,  1686,    76,    77,    78,    79,    -1,  1692,    -1,  1301,
    1423,   373,    -1,    -1,    -1,  1360,    -1,     4,  1114,    -1,
      -1,    -1,    -1,    -1,    -1,  1710,   100,    -1,    -1,    -1,
      -1,    -1,     4,  1129,  1130,   397,    -1,    -1,   400,  1452,
      -1,    -1,    47,    -1,    -1,  1458,   109,    -1,    -1,    -1,
    1463,   114,  1291,   116,   117,   118,   119,   120,   121,   122,
      47,    -1,   952,   953,    -1,    -1,    -1,  1752,    -1,    -1,
      -1,    -1,  1364,    -1,  1759,    47,    -1,  1369,    -1,    -1,
      -1,  1373,   444,    -1,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,    -1,    -1,   159,   160,    -1,   162,
      -1,    -1,    -1,    -1,   109,    -1,  1202,    -1,    -1,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
     183,  1360,   109,    -1,   486,    -1,    -1,   114,   191,   116,
     117,   118,   119,   120,   121,   122,    -1,   109,    -1,    -1,
      -1,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,   159,   160,    -1,   162,    -1,    -1,
    1452,  1453,  1454,    -1,    79,    -1,  1458,    -1,    -1,    -1,
      -1,  1463,   159,   160,    -1,   162,   538,   539,   183,  1069,
     542,    -1,    -1,    -1,    -1,   100,   191,   159,   160,  1602,
     162,    79,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   191,   567,    -1,   122,    -1,    -1,
      -1,   183,   100,    -1,    -1,    -1,    -1,    -1,    -1,   191,
     135,   136,    -1,    -1,  1114,    -1,    -1,    -1,    -1,  1642,
    1643,    -1,    -1,    -1,    -1,    -1,  1649,    79,   153,  1129,
    1130,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,   614,   615,    -1,    -1,    -1,    -1,   100,    -1,
      -1,   623,    10,    11,    12,   153,   108,   109,   156,   157,
      -1,   159,   160,  1686,   162,   163,   164,    -1,    -1,    -1,
      -1,    29,   644,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
    1602,    -1,  1202,    -1,   156,    -1,    -1,   159,   160,    67,
     162,   163,   164,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    10,    11,    12,  1752,
      -1,   703,    -1,    -1,    -1,    -1,  1759,    -1,    -1,    67,
    1642,  1643,    -1,    -1,    -1,    29,    -1,  1649,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,   743,    -1,   745,    -1,   116,   117,   118,   119,   120,
     121,    48,    49,    67,  1686,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   771,
     772,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,   785,   786,   787,   788,   789,   790,   791,
      -1,    88,   163,   795,   165,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,   806,    -1,   194,   178,    -1,   180,
      -1,    -1,   183,    -1,    -1,   817,    -1,    -1,    -1,    -1,
    1752,    -1,    -1,    -1,    -1,    -1,    -1,  1759,   830,    -1,
     832,   833,    -1,    -1,    -1,    -1,    26,    27,   135,    -1,
      30,    -1,    -1,    -1,   846,   847,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,   856,    -1,    -1,    -1,    -1,    -1,
     862,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,    -1,    -1,   875,   188,    -1,    -1,    -1,    -1,    -1,
     177,   883,    -1,    -1,   886,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   904,    -1,    -1,    -1,   908,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
     952,   953,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   966,    -1,    -1,   969,    -1,   971,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   986,    -1,   988,    -1,    -1,   991,
     992,   993,   994,   995,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,    -1,    -1,    -1,
      -1,   211,    65,    66,    -1,  1027,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1057,    -1,  1059,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1069,    -1,   191,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1086,    65,    66,  1089,   131,   132,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,  1109,    55,    -1,
      -1,    -1,  1114,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    65,    66,    -1,    -1,    -1,    -1,  1129,  1130,    -1,
    1132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1141,
      -1,    -1,    -1,  1145,    -1,   188,  1148,    -1,  1150,   339,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   348,    -1,
      -1,    -1,    -1,    -1,  1166,   355,    -1,    -1,    -1,    -1,
      -1,    -1,   362,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,   373,    -1,    -1,    -1,   131,   132,    -1,
      -1,    -1,    -1,    -1,  1196,  1197,    -1,  1199,    -1,    29,
    1202,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    67,    -1,    -1,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   444,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,  1283,    -1,  1285,    -1,    -1,    54,    -1,  1290,  1291,
      -1,    67,  1294,    -1,  1296,    -1,   486,  1299,    10,    11,
      12,    -1,    -1,    -1,    -1,  1307,  1308,    -1,    -1,  1311,
      -1,    -1,    -1,    -1,    -1,    -1,  1318,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,   542,    -1,    -1,    67,    -1,    -1,  1360,    -1,
      -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1376,    -1,    -1,   567,    57,    -1,
      -1,    -1,    -1,  1385,  1386,    -1,    -1,    -1,    -1,    -1,
      -1,  1393,    -1,  1395,    -1,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   189,    -1,  1417,    -1,  1419,    -1,    -1,
      -1,   100,    -1,  1425,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,   644,    -1,   135,   136,    -1,  1461,
    1462,    -1,    67,    -1,    -1,  1467,    -1,  1469,    -1,    -1,
      -1,    -1,    -1,  1475,   153,  1477,    -1,   156,   157,   191,
     159,   160,    -1,   162,   163,   164,    -1,   166,    -1,    -1,
      -1,    -1,   260,    -1,   262,    10,    11,    12,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   703,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   317,
      -1,    -1,    67,   743,    -1,   745,    -1,    29,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1576,    -1,    -1,    -1,    -1,    -1,
      -1,   771,   772,    -1,    -1,    57,   191,    -1,    -1,    -1,
      -1,  1593,    -1,    -1,    -1,    -1,    -1,   787,   788,   789,
     790,   791,    -1,    -1,    -1,   795,    -1,    79,  1610,    -1,
      -1,    65,    66,    -1,  1616,    -1,   806,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1627,    -1,   817,   100,   397,
      -1,  1633,   400,    -1,    -1,  1637,   108,    -1,    -1,    -1,
     830,    -1,   832,   833,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,  1658,   847,    -1,    -1,
      -1,    -1,    -1,   135,   136,    -1,   856,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   191,   131,   132,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   883,    -1,  1697,   886,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1706,   177,    -1,    -1,    -1,    -1,
      -1,   183,    -1,    -1,    -1,   187,    -1,    -1,   908,    -1,
    1722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1731,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1746,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,   952,   953,    -1,    -1,    -1,    -1,    -1,    -1,
     538,   539,    -1,    -1,   542,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   986,    -1,   988,   567,
      -1,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1027,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   614,   615,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   623,    -1,    -1,    -1,    10,
      11,    12,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    29,  1069,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    67,    -1,    -1,  1109,
      -1,    -1,    -1,    -1,  1114,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1129,
    1130,    -1,  1132,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,  1148,    -1,
    1150,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      26,    27,    -1,    -1,    30,    -1,  1166,    -1,    29,   131,
     132,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,   772,    -1,    -1,    -1,    -1,  1199,
      -1,    -1,  1202,    -1,    -1,    -1,    67,   785,   786,   787,
     788,   789,   790,   791,    -1,    -1,    -1,   795,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,   846,    -1,
      -1,    76,    77,    78,    79,    67,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,   862,    -1,    -1,    -1,    -1,    -1,
    1290,  1291,    -1,    -1,  1294,   100,  1296,   875,    -1,  1299,
      -1,    -1,    -1,    -1,    -1,   883,    -1,    -1,  1308,    -1,
      -1,  1311,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,   127,    -1,    -1,    -1,   904,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,   211,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
    1360,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1376,   182,    -1,    -1,
      -1,    -1,   187,    -1,    -1,  1385,  1386,   192,   966,    -1,
      -1,   969,    -1,   971,    -1,  1395,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   986,    -1,
      -1,    -1,    -1,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1027,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1461,  1462,    -1,    -1,    -1,    -1,  1467,    -1,  1469,
      -1,    -1,    -1,   339,    -1,    -1,    -1,  1477,    -1,  1057,
      -1,  1059,   348,    -1,    -1,    -1,    -1,    -1,    -1,   355,
      -1,    10,    11,    12,    -1,    -1,   362,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   373,  1086,    -1,
      29,  1089,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,  1132,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,  1141,    -1,    -1,    -1,  1145,    -1,    -1,
    1148,    -1,  1150,    -1,    -1,    -1,    -1,    -1,   444,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1166,    57,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    79,    55,    -1,    -1,    -1,  1616,    -1,  1196,  1197,
     486,    10,    11,    12,    67,    -1,    -1,  1627,    -1,    -1,
      -1,    -1,   100,  1633,    -1,    -1,    -1,  1637,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,   135,   136,    -1,
      -1,    -1,   191,    -1,    -1,    -1,   542,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,  1697,   166,    -1,
      -1,   567,    -1,    -1,    -1,  1283,  1706,  1285,    -1,   177,
      -1,    -1,  1290,    -1,    -1,    -1,  1294,    -1,  1296,   187,
      -1,  1299,  1722,   542,    -1,    -1,    -1,    -1,    -1,  1307,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   567,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,   644,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1376,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1393,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,   703,    -1,  1417,
      -1,  1419,    -1,    -1,    -1,    -1,    -1,  1425,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    29,   743,    -1,   745,
      -1,    -1,    -1,  1461,  1462,    -1,    67,    -1,    -1,  1467,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1475,    -1,    -1,
      -1,    -1,    -1,    -1,    57,   771,   772,    -1,    -1,    -1,
      -1,   191,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   787,   788,   789,   790,   791,    79,    -1,    -1,   795,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,   817,    -1,   772,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,   832,   833,   787,   788,
     789,   790,    -1,    -1,    -1,    -1,   795,    -1,    -1,    -1,
      -1,    -1,   135,   136,   100,    -1,    -1,    -1,    -1,    -1,
     856,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1576,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   189,   162,
     163,   164,    -1,    -1,    -1,  1593,    -1,   883,    -1,   135,
     136,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1610,    -1,   187,    -1,    -1,   153,  1616,    -1,
     156,   157,   908,   159,   160,    -1,   162,   163,   164,  1627,
      -1,    -1,    -1,    -1,    -1,  1633,    -1,    -1,    -1,  1637,
      -1,   177,    -1,    -1,   883,    -1,    10,    11,    12,    -1,
      -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1658,    -1,    -1,    -1,    -1,    29,   952,   953,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1697,
     986,    -1,   988,    67,    -1,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1731,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1027,    -1,    -1,    -1,    -1,    -1,    -1,  1746,    -1,
      -1,    -1,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
      11,    12,    -1,  1069,    -1,    -1,    -1,    -1,  1027,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,  1109,    55,    -1,    -1,    -1,  1114,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,  1129,  1130,    -1,  1132,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1148,    -1,  1150,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
    1166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,  1132,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1148,
      -1,  1150,    -1,    -1,    -1,    -1,  1202,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1166,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,   127,
     128,   129,   130,    -1,    -1,    -1,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,  1290,  1291,    -1,    -1,  1294,    -1,
    1296,    -1,    -1,  1299,    -1,   153,    -1,    -1,    -1,    -1,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,   167,
      -1,    -1,   170,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,  1290,   190,    -1,   192,  1294,    -1,  1296,    -1,    -1,
    1299,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1360,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1376,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1376,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    81,    -1,  1461,  1462,    -1,    -1,    -1,
      88,  1467,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1476,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,  1461,  1462,    -1,    -1,   134,   135,  1467,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    10,    11,    12,   182,   183,   184,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
    1616,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1627,    -1,    -1,    -1,    -1,    -1,  1633,    -1,    -1,
      -1,  1637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,  1660,    -1,    -1,  1616,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,  1627,    -1,
      -1,    -1,    -1,    -1,  1633,    -1,    -1,    -1,  1637,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,  1697,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,
     189,    95,    -1,    97,    -1,    -1,   100,   101,  1697,    -1,
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,    -1,   182,   183,
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
     100,   101,    -1,    -1,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
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
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,    -1,   192,   193,    -1,   195,
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
      -1,    -1,    88,    89,    90,    91,    92,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,
     106,   107,   108,    -1,   110,   111,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,   145,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,   190,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    98,    -1,   100,   101,
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
      90,    91,    -1,    93,    -1,    95,    96,    97,    -1,    -1,
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
      88,    89,    90,    91,    -1,    93,    94,    95,    -1,    97,
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
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    13,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    48,    49,
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
     160,    -1,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
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
      -1,   187,    -1,    -1,   190,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
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
      -1,    10,    11,    12,   192,   193,    -1,   195,   196,     3,
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
      -1,   105,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
     189,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    48,    49,
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
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,   189,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,   189,    11,    12,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,   189,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    10,    11,    12,   192,   193,    -1,   195,   196,     3,
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
     189,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,   188,    -1,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
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
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    48,    49,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
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
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
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
     184,    -1,    -1,   187,    -1,    10,    11,    12,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    67,    -1,    54,    -1,    56,    57,    58,    59,
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
      -1,   171,    -1,   188,    -1,    -1,    -1,   177,    -1,    -1,
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
      -1,   137,   138,   139,   140,   141,     3,     4,    -1,     6,
       7,    -1,   148,    10,    11,    12,    13,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      27,    -1,   168,    -1,    -1,   171,    -1,   188,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    55,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,   128,   129,   130,    -1,    -1,    -1,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   153,    -1,    -1,    -1,
      -1,    -1,   159,   160,    -1,   162,   163,   164,   165,    27,
     167,    29,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   190,    -1,   192,    -1,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,    -1,
     128,   129,   130,    -1,    -1,    -1,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   153,    -1,    -1,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    27,   167,
      29,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,   191,    -1,    -1,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,    -1,   128,
     129,   130,    -1,    -1,    -1,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   153,    -1,    -1,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    27,   167,    29,
      -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,   191,    -1,    -1,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,    -1,    -1,   127,   128,   129,
     130,    -1,    -1,    -1,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    27,   167,    29,    -1,
     170,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,    -1,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,   167,    -1,    -1,   170,     3,
       4,     5,     6,     7,    -1,   177,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    55,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   153,
     154,   155,    -1,    -1,    -1,   159,   160,    -1,   162,   163,
     164,   165,    27,   167,   168,    -1,   170,    -1,    -1,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,   128,   129,   130,    -1,    -1,    -1,   134,
     135,   136,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   153,    -1,
      -1,    -1,    -1,    -1,   159,   160,    27,   162,   163,   164,
     165,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,   127,   128,   129,   130,
      -1,    -1,    -1,   134,   135,   136,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,    -1,    -1,    -1,   159,   160,
      27,   162,   163,   164,   165,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
      -1,   128,   129,   130,    30,    -1,    -1,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,   153,    -1,    54,    -1,
      56,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
     167,    -1,    68,   170,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    36,    55,    -1,    -1,    -1,    -1,   135,
      -1,   137,   138,   139,   140,   141,    67,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    68,   162,   163,   164,    -1,
      -1,    -1,   168,    76,    77,    78,    79,    -1,    81,    -1,
      -1,   177,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    48,    49,    -1,   168,    -1,    -1,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
      -1,    -1,    68,    -1,   187,    -1,    -1,    -1,    -1,   192,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,   135,
      -1,   137,   138,   139,   140,   141,    67,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    68,   162,   163,   164,    -1,
      -1,    -1,   168,    76,    77,    78,    79,    -1,    81,    -1,
      -1,   177,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    68,   162,
     163,   164,    -1,    -1,    -1,   168,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,   182,
      -1,    -1,    -1,    -1,   187,    -1,    -1,   190,    -1,   192,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    68,    -1,    70,   168,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,   182,    -1,    -1,    88,    -1,   187,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   100,    -1,    -1,
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
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    29,    -1,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    29,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    29,   133,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,   133,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    -1,    67,    -1,    88,    -1,    -1,    -1,
      -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
     133,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
     182,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    28,    29,   192,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    67,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     187,   189,   190,   192,   193,   195,   196,   201,   204,   209,
     210,   211,   212,   213,   216,   232,   233,   237,   240,   247,
     253,   313,   314,   322,   326,   327,   328,   329,   330,   331,
     332,   333,   335,   338,   350,   351,   352,   354,   355,   357,
     367,   368,   369,   371,   376,   379,   398,   406,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     421,   434,   436,   438,   119,   120,   121,   134,   153,   163,
     187,   204,   232,   313,   332,   410,   332,   187,   332,   332,
     332,   105,   332,   332,   396,   397,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,    81,    88,
     121,   148,   187,   210,   351,   368,   371,   376,   410,   413,
     410,    36,   332,   425,   426,   332,   121,   127,   187,   210,
     245,   368,   369,   370,   372,   376,   407,   408,   409,   417,
     422,   423,   187,   323,   373,   187,   323,   342,   324,   332,
     218,   323,   187,   187,   187,   323,   189,   332,   204,   189,
     332,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    55,    57,    69,    70,    71,    72,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   127,   128,   129,   130,   134,   135,
     136,   153,   157,   165,   167,   170,   177,   187,   204,   205,
     206,   212,   439,   454,   455,   457,   189,   329,   332,   190,
     225,   332,   108,   109,   156,   207,   208,   209,    81,   192,
     279,   280,   120,   127,   119,   127,    81,   281,   187,   187,
     187,   187,   204,   251,   442,   187,   187,   324,    81,    87,
     149,   150,   151,   431,   432,   156,   190,   209,   209,   204,
     252,   442,   157,   187,   442,   442,    81,   184,   190,   343,
      27,   322,   326,   332,   333,   410,   414,   214,   190,    87,
     374,   431,    87,   431,   431,    30,   156,   173,   443,   187,
       9,   189,    36,   231,   157,   250,   442,   121,   183,   232,
     314,   189,   189,   189,   189,   189,   189,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    55,    67,   189,    68,    68,   190,
     152,   128,   163,   165,   178,   180,   253,   312,   313,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    65,    66,   131,   132,   400,    68,   190,   405,
     187,   187,    68,   190,   192,   418,   187,   231,   232,    14,
     332,   189,   133,    46,   204,   395,    87,   322,   333,   152,
     410,   133,   194,     9,   381,   246,   322,   333,   410,   443,
     152,   187,   375,   400,   405,   188,   332,    30,   216,     8,
     344,     9,   189,   216,   217,   324,   325,   332,   204,   265,
     220,   189,   189,   189,   135,   136,   457,   457,   173,   187,
     108,   457,    14,   152,   135,   136,   153,   204,   206,   189,
     189,   226,   112,   170,   189,   207,   207,   209,     9,   189,
      99,   190,   410,     9,   189,   127,   127,    14,     9,   189,
     410,   435,   324,   322,   333,   410,   413,   414,   188,   173,
     243,   134,   410,   424,   425,   189,    68,   400,   149,   432,
      80,   332,   410,    87,   149,   432,   209,   203,   189,   190,
     238,   248,   358,   360,    88,   187,   192,   345,   346,   348,
     371,   416,   418,   436,    14,    99,   437,   339,   340,   341,
     275,   276,   398,   399,   188,   188,   188,   188,   188,   191,
     215,   216,   233,   240,   247,   398,   332,   193,   195,   196,
     204,   444,   445,   457,    36,   166,   277,   278,   332,   439,
     187,   442,   241,   231,   332,   332,   332,    30,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   372,   332,   332,   420,   420,   332,   427,   428,
     127,   190,   205,   206,   417,   418,   251,   204,   252,   442,
     442,   250,   232,    36,   326,   329,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   157,
     190,   204,   401,   402,   403,   404,   417,   420,   332,   277,
     277,   420,   332,   424,   231,   188,   332,   187,   394,     9,
     381,   188,   188,    36,   332,    36,   332,   375,   188,   188,
     188,   417,   277,   190,   204,   401,   402,   417,   188,   214,
     269,   190,   329,   332,   332,    91,    30,   216,   263,   189,
      28,    99,    14,     9,   188,    30,   190,   266,   457,    29,
      88,   212,   451,   452,   453,   187,     9,    48,    49,    54,
      56,    68,   135,   157,   177,   187,   210,   212,   353,   368,
     376,   377,   378,   204,   456,   214,   187,   224,   189,   189,
      99,   208,   204,   332,   280,   377,    81,     9,   188,   188,
     188,   188,   188,   188,   188,   189,    48,    49,   449,   450,
     129,   256,   187,     9,   188,   188,    81,    82,   204,   433,
     204,    68,   191,   191,   200,   202,    30,   130,   255,   172,
      52,   157,   172,   362,   333,   133,     9,   381,   188,   152,
     457,   457,    14,   344,   275,   214,   185,     9,   382,   457,
     458,   400,   405,   400,   191,     9,   381,   174,   410,   332,
     188,     9,   382,    14,   336,   234,   129,   254,   187,   442,
     332,    30,   194,   194,   133,   191,     9,   381,   332,   443,
     187,   244,   239,   249,    14,   437,   242,   231,    70,   410,
     332,   443,   194,   191,   188,   188,   194,   191,   188,    48,
      49,    68,    76,    77,    78,    88,   135,   148,   177,   204,
     384,   386,   387,   390,   393,   204,   410,   410,   133,   254,
     400,   405,   188,   332,   270,    73,    74,   271,   214,   323,
     214,   325,    99,    36,   134,   260,   410,   377,   204,    30,
     216,   264,   189,   267,   189,   267,     9,   174,    88,   133,
     152,     9,   381,   188,   166,   444,   445,   446,   444,   377,
     377,   377,   377,   377,   380,   383,   187,   152,   187,   377,
     152,   190,    10,    11,    12,    29,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    67,   152,
     443,   191,   368,   190,   228,   204,   191,    14,   410,   189,
       9,   174,   204,   257,   368,   190,   424,   134,   410,    14,
     194,   332,   191,   200,   457,   257,   190,   361,    14,   188,
     332,   345,   417,   189,   457,   185,   191,    30,   447,   399,
      36,    81,   166,   401,   402,   404,   401,   402,   457,    36,
     166,   332,   377,   275,   187,   368,   255,   337,   235,   332,
     332,   332,   191,   187,   277,   256,    30,   255,   457,    14,
     254,   442,   372,   191,   187,    14,    76,    77,    78,   204,
     385,   385,   387,   388,   389,    50,   187,    87,   149,   187,
       9,   381,   188,   394,    36,   332,   255,   191,    73,    74,
     272,   323,   216,   191,   189,    92,   189,   260,   410,   187,
     133,   259,    14,   214,   267,   102,   103,   104,   267,   191,
     457,   133,   457,   204,   451,     9,   188,   381,   133,   194,
       9,   381,   380,   205,   345,   347,   349,   188,   127,   205,
     377,   429,   430,   377,   377,   377,    30,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   456,    81,   229,   377,   450,    99,   100,   448,     9,
     285,   188,   187,   326,   329,   332,   194,   191,   437,   285,
     158,   171,   190,   357,   364,   158,   190,   363,   133,   189,
     447,   457,   344,   458,    81,   166,    14,    81,   443,   410,
     332,   188,   275,   190,   275,   187,   133,   187,   277,   188,
     190,   457,   190,   189,   457,   255,   236,   375,   277,   133,
     194,     9,   381,   386,   388,   149,   345,   391,   392,   387,
     410,   190,   323,    30,    75,   216,   189,   325,   259,   424,
     260,   188,   377,    98,   102,   189,   332,    30,   189,   268,
     191,   174,   457,   133,   166,    30,   188,   377,   377,   188,
     133,     9,   381,   188,   133,   191,     9,   381,   377,    30,
     188,   214,   204,   457,   457,   368,     4,   109,   114,   120,
     122,   159,   160,   162,   191,   286,   311,   312,   313,   318,
     319,   320,   321,   398,   424,   191,   190,   191,    52,   332,
     332,   332,   344,    36,    81,   166,    14,    81,   332,   187,
     447,   188,   285,   188,   275,   332,   277,   188,   285,   437,
     285,   189,   190,   187,   188,   387,   387,   188,   133,   188,
       9,   381,   285,    30,   214,   189,   188,   188,   188,   221,
     189,   189,   268,   214,   457,   457,   133,   377,   345,   377,
     377,   377,   190,   191,   448,   129,   130,   178,   205,   440,
     457,   258,   368,   109,   321,    29,   122,   135,   136,   157,
     163,   295,   296,   297,   298,   368,   161,   303,   304,   125,
     187,   204,   305,   306,   287,   232,   457,     9,   189,     9,
     189,   189,   437,   312,   188,   282,   157,   359,   191,   191,
      81,   166,    14,    81,   332,   277,   114,   334,   447,   191,
     447,   188,   188,   191,   190,   191,   285,   275,   133,   387,
     345,   191,   214,   219,   222,    30,   216,   262,   214,   188,
     377,   133,   133,   214,   368,   368,   442,    14,   205,     9,
     189,   190,   440,   437,   298,   173,   190,     9,   189,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      55,    69,    70,    71,    72,    73,    74,    75,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   134,
     135,   137,   138,   139,   140,   141,   153,   154,   155,   165,
     167,   168,   170,   177,   178,   180,   182,   183,   204,   365,
     366,     9,   189,   157,   161,   204,   306,   307,   308,   189,
      81,   317,   231,   288,   440,   440,    14,   232,   191,   283,
     284,   440,    14,    81,   332,   188,   187,   190,   189,   190,
     309,   334,   447,   282,   191,   188,   387,   133,    30,   216,
     261,   262,   214,   377,   377,   191,   189,   189,   377,   368,
     291,   457,   299,   300,   376,   296,    14,    30,    49,   301,
     304,     9,    34,   188,    29,    48,    51,    14,     9,   189,
     206,   441,   317,    14,   457,   231,   189,    14,   332,    36,
      81,   356,   214,   214,   190,   309,   191,   447,   387,   214,
      96,   227,   191,   204,   212,   292,   293,   294,     9,   174,
       9,   381,   191,   377,   366,   366,    57,   302,   307,   307,
      29,    48,    51,   377,    81,   173,   187,   189,   377,   442,
     377,    81,     9,   382,   191,   191,   214,   309,    94,   189,
     112,   223,   152,    99,   457,   376,   164,    14,   449,   289,
     187,    36,    81,   188,   191,   189,   187,   170,   230,   204,
     312,   313,   174,   377,   174,   273,   274,   399,   290,    81,
     368,   228,   167,   204,   189,   188,     9,   382,   116,   117,
     118,   315,   316,   273,    81,   258,   189,   447,   399,   458,
     188,   188,   189,   189,   190,   310,   315,    36,    81,   166,
     447,   190,   214,   458,    81,   166,    14,    81,   310,   214,
     191,    36,    81,   166,    14,    81,   332,   191,    81,   166,
      14,    81,   332,    14,    81,   332,   332
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
#line 764 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 853 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 859 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { (yyval).reset();;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1110 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
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

  case 183:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
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

  case 185:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { (yyval).reset();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval).reset();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval).reset();;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval).reset();;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval).reset();;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval).reset();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval).reset();;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval).reset();;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { (yyval).reset();;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { (yyval).reset();;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { (yyval).reset();;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval).reset();;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval).reset();;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval).reset();;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval).reset();;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval).reset();;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
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

  case 504:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
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

  case 506:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
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

  case 508:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
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

  case 543:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
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

  case 544:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval).reset();;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval).reset();;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval).reset();;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval).reset();;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval).reset();;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval).reset();;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval).reset();;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval).reset();;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval).reset();;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval).reset();;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { (yyval).reset();;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval).reset();;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
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

  case 798:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
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

  case 799:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
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

  case 800:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
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

  case 801:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
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

  case 808:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
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

  case 817:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval).reset();;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { (yyval)++;;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
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
#line 2877 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
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

  case 858:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval).reset();;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
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

  case 894:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    {;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    {;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    {;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13570 "hphp.5.tab.cpp"
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
#line 3246 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

